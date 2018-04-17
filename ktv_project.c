
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct main_ktv{//定义歌库的节点结构体
	char song_name[50];
	char singer_name[50];
	char date[20];
	char type[20];
	char song_time[10];
	
	struct main_ktv *next;
	struct main_ktv *prev;
}Node,*PLNode;

typedef struct play_ktv{//定义播放列表的节点结构体
	char song_name[50];
	char singer_name[50];
	char date[20];
	char type[20];
	char song_time[10];
	
	struct play_ktv *next;
	struct play_ktv *prev;
}p_Node,*p_PLNode;

PLNode init_head(PLNode head)//初始化头节点
{
	head = (PLNode)malloc(sizeof(Node));
	
	strcpy(head->song_name,"none");
	strcpy(head->singer_name,"none");
	strcpy(head->date,"none");
	strcpy(head->type,"none");
	strcpy(head->song_time,"none");
	
	head->next = NULL;
	head->prev = NULL;
	
	return head;
	
}

p_PLNode init_p_head(p_PLNode p_head)//初始化头节点
{
	p_head = (p_PLNode)malloc(sizeof(p_Node));
	
	strcpy(p_head->song_name,"none");
	strcpy(p_head->singer_name,"none");
	strcpy(p_head->date,"none");
	strcpy(p_head->type,"none");
	strcpy(p_head->song_time,"none");
	
	p_head->next = NULL;
	p_head->prev = NULL;
	
	return p_head;
	
}
/**                    插入                       **/
int add_tail(PLNode head,char *song_name,char *singer_name,\
char *date,char *type,char *song_time)
{
	if(head == NULL)
	{
		printf("head is NULL!\n");
		return -1;
	}
	PLNode node = NULL;
	node = (PLNode)malloc(sizeof(Node));//给节点申请栈空间
	
	PLNode p =NULL;
	//给节点赋值
	strcpy(node->song_name,song_name);
	strcpy(node->singer_name,singer_name);
	strcpy(node->date,date);
	strcpy(node->type,type);
	strcpy(node->song_time,song_time);
	
	for(p=head;p->next!=NULL;p=p->next);
	
	p->next = node;
	node->next = NULL;
	node->prev = p;
	
	return 0;
}

int add_p_tail(p_PLNode p_head,char *song_name,char *singer_name,\
char *date,char *type,char *song_time)
{
	if(p_head == NULL)
	{
		printf("head is NULL!\n");
		return -1;
	}
	p_PLNode node = NULL;
	node = (p_PLNode)malloc(sizeof(p_Node));//给节点申请栈空间
	
	p_PLNode p =NULL;
	//给节点赋值
	strcpy(node->song_name,song_name);
	strcpy(node->singer_name,singer_name);
	strcpy(node->date,date);
	strcpy(node->type,type);
	strcpy(node->song_time,song_time);
	
	for(p=p_head;p->next!=NULL;p=p->next);
	
	p->next = node;
	node->next = NULL;
	node->prev = p;
	
	printf("The song has been added!\n");
	sleep(1);
	return 0;
}	

int add_p_head(p_PLNode p_head,p_PLNode tmp)//定义头插，只能插入第一个节点的后面，不能替换已经播放的第一个节点
{
	if(p_head == NULL)
	{
		printf("head is NULL!\n");
		return -1;
	}
	
	tmp->next = p_head->next->next;//将要插入的节点的后驱指向第一节点的后驱
	if(p_head->next->next!=NULL)//判断第一节点的后驱是否为空
		p_head->next->next->prev = tmp;//将第二节点的前驱指向新插入节点
	p_head->next->next = tmp;//第二节点的后驱指向新插入节点
	tmp->prev = p_head->next; //新插入节点的前驱指向第一节点
	
	return 0;
}	


/**                    展示+查询                       **/

int show_only(PLNode tmp)//显示指定节点内容
{
	if(tmp == NULL)
	{
		printf("Not found the song!\n");
		return -1;
	}
	printf("%-10s   %-10s\t%-5s\t    %-5s\t%-10s\n",tmp->song_name,tmp->singer_name,\
		tmp->date,tmp->type,tmp->song_time);
		usleep(500000);
		
	return 0;	
}

int show_list_data(PLNode head)//显示全部节点内容
{
	PLNode p = NULL;
	
	for(p=head->next;p!=NULL;p=p->next)
	{
		printf("%-10s   %-10s\t%-5s\t    %-5s\t%-10s\n",p->song_name,p->singer_name,\
		p->date,p->type,p->song_time);
		usleep(500000);
	}
	return 0;
}

int show_p_list_data(p_PLNode p_head)//显示播放列表的全部节点内容
{
	p_PLNode p = NULL;
	
	for(p=p_head->next;p!=NULL;p=p->next)
	{
		printf("%-10s   %-10s\t%-5s\t    %-5s\t%-10s\n",p->song_name,p->singer_name,\
		p->date,p->type,p->song_time);
		usleep(500000);
	}
	return 0;
}

int search_singer_node(PLNode head)//查找歌手信息
{
	PLNode p = NULL;
	PLNode q = NULL;
	
	
	for(q=head->next;q!=NULL;q=q->next)
	{
		for(p=head;p!=NULL;p=p->next)
		{
			if(strcmp(p->singer_name,q->singer_name)==0 )
			{
				break;
			}	
			if(p==q->prev)
			{
				printf("\t%s\t\n",q->singer_name);
			}
		}
	}
	
	return 0;
}

PLNode select_singer_name(PLNode head,char *singer_name,char*song_name)//查找歌手名字
{
	PLNode p = NULL;
	PLNode q = NULL;
	
	for(q=head,p=head->next;p!=NULL;q=p,p=p->next)
	{
		if(strcmp(p->singer_name,singer_name)==0&&strcmp(p->song_name,song_name)==0)
		{
			return p;
		}
		
	}
	return NULL;
}

int show_singer_name(PLNode head,char *singer_name)//显示歌手名字下的全部歌曲
{
	PLNode p = NULL;
	PLNode q = NULL;
	
	for(q=head,p=head->next;p!=NULL;q=p,p=p->next)
	{
		if(strcmp(p->singer_name,singer_name)==0 )
		{
			show_only(p);
		}
		
	}
	
	return 0;
}

PLNode search_song_node(PLNode head,char *song_name)//查找歌曲的信息
{
	PLNode p = NULL;
	PLNode q = NULL;
	
	for(q=head,p=head->next;p!=NULL;q=p,p=p->next)
	{
		if(strcmp(p->song_name,song_name)==0 )
		{
			
			return p;
		}
		
	}
	printf("Not found the song!\n");
	return NULL;
}

p_PLNode search_p_song_node(p_PLNode p_head,char *song_name)//查找播放列表歌曲的信息
{
	p_PLNode p = NULL;
	p_PLNode q = NULL;
	
	for(q=p_head,p=p_head->next;p!=NULL;q=p,p=p->next)
	{
		if(strcmp(p->song_name,song_name)==0 )
		{
			
			return p;
		}
		
	}
	printf("Not found the song!\n");
	return NULL;
}

/**                    删除                       **/

p_PLNode delete_p_name_node(p_PLNode p_head,char *song_name)//删除指定播放列表的节点
{
	p_PLNode p = NULL;
	p_PLNode q = NULL;
	
	for(q=p_head->next,p=p_head->next->next;p!=NULL;q=p,p=p->next)
	{
		if(strcmp(p->song_name,song_name)==0 )
		{
			sleep(1);
			
			q->next = p->next;
			if(p->next!=NULL)
				p->next->prev = p->prev;
			
			p->next = NULL;
			p->prev = NULL;
			
			printf("Successful position change!\n");
			sleep(2);
			return p;
		}
		
	}
	return NULL;
}

int delete_list_node(PLNode head)//删除整个歌手链表
{
	if(head == NULL)
	{
		printf("head is NULL!\n");
		return -1;
	}
	
	PLNode p = NULL;
	PLNode q = NULL;
	
	for(q=head,p=head;p!=NULL;p=q)
	{
		
		q = p->next;
		free(p);	
	}
	
	return 0;
}

int delete_p_list_node(p_PLNode p_head)//删除播放列表
{
	if(p_head == NULL)
	{
		printf("head is NULL!\n");
		return -1;
	}
	
	p_PLNode p = NULL;
	p_PLNode q = NULL;
	
	for(q=p_head,p=p_head;p!=NULL;p=q)
	{
		
		q = p->next;
		free(p);	
	}
	
	return 0;
}
/***************************************************/
int the_org_data();
int main()
{
	int sure;
	int choose;
	int get_choose;
	char song_name[50];
	char singer_name[50];
	char p_song_name[50];
	
	system("clear");
	printf("\n    Welcome to the KTV System!!\n");
	
	PLNode head = NULL;
	PLNode tmp = NULL;
	PLNode s_tmp = NULL;
	p_PLNode p_tmp = NULL;
	p_PLNode p_head = NULL;
	
	head = init_head(head);
	p_head = init_p_head(p_head);
	
	the_org_data(head);
	while(sure)
	{
		printf("\n******************************************\n");
		printf("*******       KTV Menus             ******\n");
		printf("******1 Show the list of songs      ******\n");
		printf("******2 According to singer         ******\n");
		printf("******3 According to the title song ******\n");
		printf("******4 Check the song sheet        ******\n");
		printf("******0 Quit the system             ******\n");
		printf("******************************************\n");
		printf("choose your choice:");
		scanf("%d",&get_choose);
		
		switch(get_choose)
		{
			case 1:
				system("clear");
				printf("*****************************************************\n");
				printf("歌名        歌手       出行日期     专辑        时长\n");
				show_list_data(head);
				printf("\n*****************************************************\n");
				break;
			case 2:
				system("clear");
				printf("***************************\n");
				search_singer_node(head);
				printf("***************************\n");
				printf("Please insert the singer's name(eg:周杰伦):");
				scanf("%s",singer_name);
				system("clear");
				printf("******************%s的全部歌曲**********************\n",singer_name);
				printf("歌名        歌手       出行日期     专辑        时长\n");
				show_singer_name(head,singer_name);
				printf("********************************************************\n");
				printf("Please insert the song's name(eg:稻香):");
				scanf("%s",song_name);
				s_tmp = select_singer_name(head,singer_name,song_name);
		
				if(s_tmp == NULL)
				{	
					printf("There is no song here.\n");
					break;
				}
				add_p_tail(p_head,s_tmp->song_name,\
						s_tmp->singer_name,s_tmp->date,s_tmp->type,s_tmp->song_time);
				break;
			case 3:
				printf("Please insert the song's name(eg:稻香):");
				scanf("%s",song_name);
				tmp = search_song_node(head,song_name);
				system("clear");
				printf("***************************************************\n");
				printf("歌名        歌手       出行日期     专辑        时长\n");
				show_only(tmp);
				printf("***************************************************\n");
				if(tmp == NULL)
					break;
				add_p_tail(p_head,tmp->song_name,\
						tmp->singer_name,tmp->date,tmp->type,tmp->song_time);
				break;
			case 4:
			    system("clear");
				printf("*****************************************************\n");
				printf("歌名        歌手       出行日期     专辑        时长\n");
				show_p_list_data(p_head);
				printf("*****************************************************\n");
				printf("********1、Put the song at the beginning  ***********\n");
				printf("********0、Back to the menu               ***********\n");
				printf("*****************************************************\n");
				printf("choose your choice:");
				scanf("%d",&choose);
			
				switch(choose)
					{
						case 1:
							printf("Please insert the song's name(eg:稻香):");
							scanf("%s",p_song_name);
							p_tmp = delete_p_name_node(p_head,p_song_name);
							add_p_head(p_head,p_tmp);
							break;
						case 0:
							
							break;
						default:
							break;
					}
				break;
			case 0:
				sure = 0;
				delete_list_node(head);
				delete_p_list_node(p_head);
				printf("\nThank you for your use,goodbye! \n");
				sleep(2);
				
				break;
			default:
				break;
		}
	}
	return 0;
}

int the_org_data(PLNode head)//添加原始数据
{
	if(head == NULL)
	{
		printf("head is NULL!\n");
		return -1;
	}
	add_tail(head,"我的天空","南征北战","2017.2","青春派","3:54");
	add_tail(head,"告白气球","周杰伦","2017.1","Jay周","3:35");
	add_tail(head,"我的天坑","南征北战","2017.6","热门华语","1:56");
	add_tail(head,"晴天","周杰伦","2001.2","Jay周","4:29");
	add_tail(head,"SweetDevil","初音","2017.3","初音V家","3:56");
	add_tail(head,"稻香","周杰伦","2010.3","Jay周","3:43");
	add_tail(head,"Fade","AlanWalker","2017.5","Fade","4:22");
	add_tail(head,"七 里 香","周杰伦","2014.4","Jay周","4:59");
	

	
	return 0;
}
