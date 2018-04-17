#include "copyfile.h"

/*************�̳߳رر�����*********************************/
//ѹջ����
void handler(void *arg)
{
	printf("[%u] is ended.\n",
		(unsigned)pthread_self());

	//�����ȡ��ʱ������״̬������ȡ��ǰִ�и����̽�������ִ��ȡ����Ӧ��
	pthread_mutex_unlock((pthread_mutex_t *)arg);
}

//�����̳߳�
void *routine(void *arg)
{
	//�̳߳ر�������
	thread_pool *pool = (thread_pool *)arg;
	struct task *p;//����ָ��Ҫ���ѵ�����ڵ�

	while(1)
	{
		//��ֹ�߳��ڴ�������ʱ����ȡ����pthread_cond_wait()��ȡ����
		pthread_cleanup_push(handler, (void *)&pool->lock);
		
		//������������ʱ����������
		pthread_mutex_lock(&pool->lock);
		//================================================//

		// 1, ���������û�����񣬶����̳߳�δ���رգ��߳̾ͽ�����������˯�ߵȴ�
		while(pool->waiting_tasks == 0 && !pool->shutdown)
		{
			//�Զ��������������������еȴ�
			pthread_cond_wait(&pool->cond, &pool->lock);
		}
		//ע�⣬���ﲻ��ʹ��if���жϣ���Ϊ�߳������󣬻���Ҫѯ����û������

		// 2, ���������û�����񣬶����̳߳عرգ����˳����ˣ�
		if(pool->waiting_tasks == 0 && pool->shutdown == true)
		{
			//����
			pthread_mutex_unlock(&pool->lock);
			
			//����
			pthread_exit(NULL); // CANNOT use 'break';
		}

		// 3, �������̴߳�������
		//ȡ������ڵ�p
		p = pool->task_list->next;
		pool->task_list->next = p->next;
		
		//������������1
		pool->waiting_tasks--;

		//================================================//
		//��������������ȡ�ýڵ㣬������
		pthread_mutex_unlock(&pool->lock);
		
		//��ջ����ִ������
		pthread_cleanup_pop(0);
		//================================================//
		
		//���������ڼ䣬����Ӧ�κε�ȡ���ź�
		//����߳��ڴ��������ڼ��յ�ȡ�������������������Ӧȡ��
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		
		//ִ��������
		(p->do_task)(p->arg);
		//printf("quxiao!\n");
		//���������񣬿���Ӧȡ������
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		

		//�ͷŴ����������ڵ��ڴ�
		free(p);
	}

	pthread_exit(NULL);
}
//��ʼ������
bool init_pool(thread_pool *pool, unsigned int threads_number)
{
	pthread_mutex_init(&pool->lock, NULL);//��ʼ����
	pthread_cond_init(&pool->cond, NULL);//��ʼ����������

	pool->shutdown = false;//��ʼ����־
	pool->task_list = malloc(sizeof(struct task));//Ϊ��������ͷ�ڵ������ڴ�
	pool->tids = malloc(sizeof(pthread_t) * MAX_ACTIVE_THREADS);//�������߳�TID���ڴ�

	//�����ж�
	if(pool->task_list == NULL || pool->tids == NULL)
	{
		perror("allocate memory error");
		return false;
	}

	//��������ڵ�ָ�����ʼ��
	pool->task_list->next = NULL;

	pool->max_waiting_tasks = MAX_WAITING_TASKS;//�����������ʼ��
	pool->waiting_tasks = 0;//�ȴ������������
	pool->active_threads = threads_number;//��ʼ���̸߳���

	int i;
	//��ʼ�������̣߳��ʹ����������߳�
	for(i=0; i<pool->active_threads; i++)
	{
		//��ͨ�����̣߳�����routine
		if(pthread_create(&((pool->tids)[i]), NULL,
					routine, (void *)pool) != 0)
		{
			perror("create threads error");
			return false;
		}
	}

	return true;
}
//����������
bool add_task(thread_pool *pool,
	      void *(*do_task)(void *arg), void *arg)
{
	//����������ڵ���ڴ�
	struct task *new_task = malloc(sizeof(struct task));
	if(new_task == NULL)
	{
		perror("allocate memory error");
		return false;
	}
	
	//��ʼ��������ڵ��Ա
	new_task->do_task = do_task;
	new_task->arg = arg;
	new_task->next = NULL;

	//ֻҪ��������������Ҫ����
	//����ýڵ㵽��������β��
	//============ LOCK =============//
	pthread_mutex_lock(&pool->lock);
	//===============================//
	
	//��������Ѿ��ﵽ���ֵ������������񣬽������ˣ�
	if(pool->waiting_tasks >= MAX_WAITING_TASKS)
	{
		//����
		pthread_mutex_unlock(&pool->lock);

		//���������Ϣ
		fprintf(stderr, "too many tasks.\n");
		
		//�ͷŸ����������ڵ���ڴ�
		free(new_task);

		return false;
	}
	
	//Ѱ����β�Ľڵ�tmp
	struct task *tmp = pool->task_list;
	while(tmp->next != NULL)
		tmp = tmp->next;

	//�����Ľڵ��ָ����ָ��������ڵ�
	tmp->next = new_task;
	
	//������������1
	pool->waiting_tasks++;

	//�����ϣ�������ϣ�������
	//=========== UNLOCK ============//
	pthread_mutex_unlock(&pool->lock);
	//===============================//

	//�����һ�����񣬻�������һ��������������˯�ߵ��߳�������������
	pthread_cond_signal(&pool->cond);
	return true;
}
//�̳߳����ٺ���
bool destroy_pool(thread_pool *pool)
{
	// 1, activate all threads
	
	//�̳߳صĹرձ�־Ϊ��
	pool->shutdown = true;
	
	//�������е��̣߳������������ж��̵߳ı�־Ϊtrue��ȫ���˳�
	pthread_cond_broadcast(&pool->cond);

	// 2, wait for their exiting
	int i;
//	printf("active threads:%d\n",pool->active_threads);
	//�ȴ��߳��˳�
	for(i=0; i<pool->active_threads; i++)
	{
		//�ȴ�����
		errno = pthread_join(pool->tids[i], NULL);
		//�߳��˳�ʧ��
		if(errno != 0)
		{
			printf("join tids[%d] error: %s\n",
					i, strerror(errno));
		}
		//�߳��˳��ɹ�
		else
		{
		//	printf("[%u] is joined\n", (unsigned)pool->tids[i]);
		}
	}

	// 3, free memories
	//�ͷ����е��ڴ�
	free(pool->task_list);
	free(pool->tids);
	free(pool);

	return true;
}

/**************************************************************/

void *copy(void *arg)//�����ļ�����
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
	
	if(argc !=4)//�ж������������
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
	
	//��ʼ���̳߳�
	thread_pool *pool = malloc(sizeof(thread_pool));
	init_pool(pool,pthread_num);
	
	//��Ӹ�������
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
	//����
	destroy_pool(pool);
	sleep(1);
	
	thread_pool c_pool;//������ӡ�Ѹ��Ƶ�Ŀ¼����
	printf("\nAll file/dir has been copy!\n");
	printf("There are %d dir\n",c_pool.dirent_count);
	
	return 0;
}


