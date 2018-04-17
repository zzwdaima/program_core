#include "copyfile.h"

/*************线程池必备函数*********************************/
//压栈例程
void handler(void *arg)
{
	printf("[%u] is ended.\n",
		(unsigned)pthread_self());

	//如果被取消时是上锁状态，即在取消前执行该例程解锁，再执行取消响应！
	pthread_mutex_unlock((pthread_mutex_t *)arg);
}

//消费线程池
void *routine(void *arg)
{
	//线程池变量传参
	thread_pool *pool = (thread_pool *)arg;
	struct task *p;//用于指向将要消费的任务节点

	while(1)
	{
		//防止线程在处理任务时，被取消，pthread_cond_wait()是取消点
		pthread_cleanup_push(handler, (void *)&pool->lock);
		
		//访问任务链表时，首先上锁
		pthread_mutex_lock(&pool->lock);
		//================================================//

		// 1, 任务队列中没有任务，而且线程池未被关闭，线程就进入条件变量睡眠等待
		while(pool->waiting_tasks == 0 && !pool->shutdown)
		{
			//自动解锁，进入条件变量中等待
			pthread_cond_wait(&pool->cond, &pool->lock);
		}
		//注意，这里不能使用if来判断，因为线程醒来后，还是要询问有没有任务！

		// 2, 任务队列中没有任务，而且线程池关闭，即退出走人！
		if(pool->waiting_tasks == 0 && pool->shutdown == true)
		{
			//解锁
			pthread_mutex_unlock(&pool->lock);
			
			//走人
			pthread_exit(NULL); // CANNOT use 'break';
		}

		// 3, 有任务，线程处理它！
		//取出任务节点p
		p = pool->task_list->next;
		pool->task_list->next = p->next;
		
		//任务数量减少1
		pool->waiting_tasks--;

		//================================================//
		//访问完任务链表，取得节点，解锁！
		pthread_mutex_unlock(&pool->lock);
		
		//弹栈，不执行例程
		pthread_cleanup_pop(0);
		//================================================//
		
		//在任务处理期间，不响应任何的取消信号
		//如果线程在处理任务期间收到取消请求，先完成任务，再响应取消
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		
		//执行任务函数
		(p->do_task)(p->arg);
		//printf("quxiao!\n");
		//处理完任务，可响应取消请求
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		

		//释放处理完的任务节点内存
		free(p);
	}

	pthread_exit(NULL);
}
//初始化函数
bool init_pool(thread_pool *pool, unsigned int threads_number)
{
	pthread_mutex_init(&pool->lock, NULL);//初始化锁
	pthread_cond_init(&pool->cond, NULL);//初始化条件变量

	pool->shutdown = false;//初始化标志
	pool->task_list = malloc(sizeof(struct task));//为任务链表头节点申请内存
	pool->tids = malloc(sizeof(pthread_t) * MAX_ACTIVE_THREADS);//申请存放线程TID号内存

	//错误判断
	if(pool->task_list == NULL || pool->tids == NULL)
	{
		perror("allocate memory error");
		return false;
	}

	//任务链表节点指针域初始化
	pool->task_list->next = NULL;

	pool->max_waiting_tasks = MAX_WAITING_TASKS;//最大任务数初始化
	pool->waiting_tasks = 0;//等待处理的任务数
	pool->active_threads = threads_number;//初始化线程个数

	int i;
	//初始化几条线程，就创建多少条线程
	for(i=0; i<pool->active_threads; i++)
	{
		//普通属性线程，例程routine
		if(pthread_create(&((pool->tids)[i]), NULL,
					routine, (void *)pool) != 0)
		{
			perror("create threads error");
			return false;
		}
	}

	return true;
}
//增加任务函数
bool add_task(thread_pool *pool,
	      void *(*do_task)(void *arg), void *arg)
{
	//申请新任务节点的内存
	struct task *new_task = malloc(sizeof(struct task));
	if(new_task == NULL)
	{
		perror("allocate memory error");
		return false;
	}
	
	//初始化新任务节点成员
	new_task->do_task = do_task;
	new_task->arg = arg;
	new_task->next = NULL;

	//只要访问任务链表，就要上锁
	//插入该节点到任务链表尾部
	//============ LOCK =============//
	pthread_mutex_lock(&pool->lock);
	//===============================//
	
	//如果任务已经达到最大值，则不能添加任务，解锁走人！
	if(pool->waiting_tasks >= MAX_WAITING_TASKS)
	{
		//解锁
		pthread_mutex_unlock(&pool->lock);

		//输出错误信息
		fprintf(stderr, "too many tasks.\n");
		
		//释放刚申请的任务节点的内存
		free(new_task);

		return false;
	}
	
	//寻找最尾的节点tmp
	struct task *tmp = pool->task_list;
	while(tmp->next != NULL)
		tmp = tmp->next;

	//让最后的节点的指针域指向新任务节点
	tmp->next = new_task;
	
	//任务数量增加1
	pool->waiting_tasks++;

	//添加完毕，访问完毕，解锁！
	//=========== UNLOCK ============//
	pthread_mutex_unlock(&pool->lock);
	//===============================//

	//添加了一个任务，唤醒其中一条在条件变量中睡眠的线程起来处理任务。
	pthread_cond_signal(&pool->cond);
	return true;
}
//线程池销毁函数
bool destroy_pool(thread_pool *pool)
{
	// 1, activate all threads
	
	//线程池的关闭标志为真
	pool->shutdown = true;
	
	//唤醒所有的线程，让他们醒来判断线程的标志为true，全部退出
	pthread_cond_broadcast(&pool->cond);

	// 2, wait for their exiting
	int i;
//	printf("active threads:%d\n",pool->active_threads);
	//等待线程退出
	for(i=0; i<pool->active_threads; i++)
	{
		//等待回收
		errno = pthread_join(pool->tids[i], NULL);
		//线程退出失败
		if(errno != 0)
		{
			printf("join tids[%d] error: %s\n",
					i, strerror(errno));
		}
		//线程退出成功
		else
		{
		//	printf("[%u] is joined\n", (unsigned)pool->tids[i]);
		}
	}

	// 3, free memories
	//释放所有的内存
	free(pool->task_list);
	free(pool->tids);
	free(pool);

	return true;
}

/**************************************************************/

void *copy(void *arg)//复制文件任务
{	

	char *src_file = ((FILES *)arg) -> src_file;
	char *dst_file = ((FILES *)arg) -> dst_file;
	
	copy_file(src_file,dst_file);
	printf("Copy is complete!\n");
	
//	pthread_exit(NULL);
}

int main(int argc,char const *argv[])
{
//	pthread_t pth;
//	pthread_create(&pth,NULL,pool_job,NULL);
	
	if(argc !=4)//判断输入参数数量
	{
		printf("The arg need pthread_num,src_file or dir,dst_file or dir!\n");
		return -1;
	}
	
	int pthread_num = atoi(argv[1]);
	
	printf("\n****************************************************\n");
	printf("The pthread number is %d\n",pthread_num);
	printf("The sourse file/dir is %s\n",argv[2]);
	printf("The target file/dir is %s\n",argv[3]);
	printf("*****************************************************\n");
	printf("\n");
	
	FILES *name = malloc(sizeof(FILES));
	strcpy(name->src_file, argv[2]);
	strcpy(name->dst_file, argv[3]);
	
	//初始化线程池
	thread_pool *pool = malloc(sizeof(thread_pool));
	init_pool(pool,pthread_num);
	
	//添加复制任务
	int type;
	type = check_type(argv[2]);
	if(type == 0)
	{
		add_task(pool,copy,name);
	}
	if(type == 1)
	{
		copy_dir(pool,argv[2],argv[3]);
	}
	//销毁
	destroy_pool(pool);
	sleep(1);
	
	thread_pool c_pool;//用来打印已复制的目录个数
	printf("\nAll file/dir has been copy!\n");
	printf("There are %d dir\n",c_pool.dirent_count);
	
	return 0;
}


