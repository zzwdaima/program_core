
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct flight{
	char number[10];//航班
	char staddress[10];//起点
	char arraddress[15];//终点
	char date[15];//日期
	char type;//类型
	char stime[10];//时间
	int price;//票价
	
	struct flight *next;
	struct flight *prev;
}Node,*PLNode;

/* struct list_node{
			struct flight info;
			struct list_head list;
}; */
PLNode init_head(PLNode head)//初始化链表头，当前为双向链表
{
	head = (PLNode)malloc(sizeof(Node));
	
	strcpy(head->number,"none");//初始化表头数据为none
	strcpy(head->staddress,"none");
	strcpy(head->arraddress,"none");
	strcpy(head->date,"none");
	head->type = 'N';
	strcpy(head->stime,"none");
	head->price = 0;
	
	head->next = NULL;
	head->prev = NULL;
	
	return head;
	
}
/*****                     插入                   *****/
int add_tail(PLNode head,char *number,char *staddress,\
char *arraddress,char *date,char type,char *stime,int price)
{
	if(head == NULL)
	{
		printf("head is NULL!\n");
		return -1;
	}
	PLNode node = NULL;
	//为节点申请栈空间
	node = (PLNode)malloc(sizeof(Node));
	
	PLNode p =NULL;
	
	//给节点赋值
	strcpy(node->number,number);
	strcpy(node->staddress,staddress);
	strcpy(node->arraddress,arraddress);
	strcpy(node->date,date);
	node->type = type;
	strcpy(node->stime,stime);
	node->price = price;
	
	for(p=head;p->next!=NULL;p=p->next);//将指针指到节点的后驱
	
	p->next = node;//节点后驱为新节点node
	node->next = NULL;//node的后驱为NULL
	node->prev = p;//node的前驱为尾节点
	
	return 0;
}

int add_head(PLNode head,char number[10],char staddress[10],\
char arraddress[15],char date[15],char type,char stime[10],int price)
{
	if(head == NULL)
	{
		printf("head is NULL!\n");
		return -1;
	}
	PLNode node = NULL;
	node = (PLNode)malloc(sizeof(Node));//为节点申请栈空间
	
	//给节点赋值
	strcpy(node->number,number);
	strcpy(node->staddress,staddress);
	strcpy(node->arraddress,arraddress);
	strcpy(node->date,date);
	node->type = type;
	strcpy(node->stime,stime);
	node->price = price;

	node->next = head->next;//将头节点后驱指向新节点后驱
	if(head->next!=NULL)//确定是头节点
		head->next->prev = node;//第二个节点节点的前驱指向新节点
	head->next = node;//头节点的后驱指向新节点
	node->prev = head;//新节点的前驱指向头
	
	return 0;
}	

/*****                   展示查询                   *****/

int show_list_data(PLNode head)
{
	PLNode p = NULL;
	
	for(p=head->next;p!=NULL;p=p->next)//遍历全部节点，并将节点内容打印
	{
		printf("%s  %s --> %s  %s   %c    %s  %d\n",p->number,p->staddress,\
		p->arraddress,p->date,p->type,p->stime,p->price);
		usleep(500000);
	}
	return 0;
}

int show_only(PLNode tmp)//只显示选择的节点内容
{
	if(tmp == NULL)
	{
		printf("Not found the flight!\n");
		return -1;
	}
	printf("%s  %s --> %s  %s   %c    %s  %d\n",tmp->number,tmp->staddress,\
		tmp->arraddress,tmp->date,tmp->type,tmp->stime,tmp->price);
		
	return 0;	
}

int show_stime_node(PLNode head,char *stime)//按照时间来寻找指定节点
{
	PLNode p = NULL;
	PLNode q = NULL;
	
	for(q=head,p=head->next;p!=NULL;q=p,p=p->next)
	{
		if(strcmp(p->stime,stime)==0)
		{
			show_only(p);
		}
		
	}
	
	return 0;
}

int show_type_node(PLNode head,char type)//按照类型来寻找指定节点
{
	PLNode p = NULL;
	PLNode q = NULL;
	
	for(q=head,p=head->next;p!=NULL;q=p,p=p->next)
	{
		if(p->type==type)
		{
			show_only(p);
		}
		
	}
	
	return 0;
}

int show_date_node(PLNode head,char *date)//按照日期来寻找指定节点
{
	PLNode p = NULL;
	PLNode q = NULL;
	
	for(q=head,p=head->next;p!=NULL;q=p,p=p->next)
	{
		if(strcmp(p->date,date)==0)
		{
			show_only(p);
		}
		
	}
	
	return 0;
}

PLNode search_number_node(PLNode head,char *number)//按照航号来寻找指定节点
{
	PLNode p = NULL;
	PLNode q = NULL;
	
	for(q=head,p=head->next;p!=NULL;q=p,p=p->next)
	{
		if(strcmp(p->number,number)==0 )
		{
			
			return p;
		}
		
	}
	printf("Not found the flight!\n");
	return NULL;
}
/*****                     删除                   *****/
int delete_number_node(PLNode head,char *number)
{
	PLNode p = NULL;
	PLNode q = NULL;
	
	for(q=head,p=head->next;p!=NULL;q=p,p=p->next)//先找到指定节点
	{
		if(strcmp(p->number,number)==0 )
		{
			sleep(1);
			printf("The flight is cancel!\n");
			//将指定节点的后一节和前一节连接在一起防止断链
			q->next = p->next;
			if(p->next!=NULL)
				p->next->prev = p->prev;
			free(p);//释放该节点内存
			sleep(3);
			system("clear");
			return 0;
		}
		
	}
	printf("Not found the flight!\n");
	return -1;
}

int delete_list_node(PLNode head)
{
	if(head == NULL)
	{
		printf("head is NULL!\n");
		return -1;
	}
	
	PLNode p = NULL;
	PLNode q = NULL;
	
	for(q=head,p=head;p!=NULL;p=q)//遍历全部节点并全部释放
	{
		
		q = p->next;
		free(p);	
	}
	
	return 0;
}
/*********************************************************/
int Password(char *password);
int the_org_data();
int main()
{
	int sure = 1;//标志位
	int q_sure;//标志位
	int choose;
	int get_choose;
	int get;
	
	char staddress[10];//起点
	char arraddress[15];//终点
	char number[10];//航班
	char date[15];//日期
	char type;//类型
	char stime[10];//时间
	int price;
	char password[20];
	
	system("clear");
	printf("\n    Welcome to the flight System!!\n");
	
	PLNode head = NULL;
	PLNode tmp = NULL;
	
	head = init_head(head);
	the_org_data(head);
	while(sure)
	{
		printf("\n******************************************\n");
		printf("******1 Show the list of flight **********\n");
		printf("******2 Query the flight        **********\n");
		printf("******3 Quick query             **********\n");
		printf("******4 Delete the flight       **********\n");
		printf("******5 Entry the flight        **********\n");
		printf("******0 Quit the system         **********\n");
		printf("******************************************\n");
		printf("choose your choice:");
		scanf("%d",&get_choose);
		switch(get_choose)
		{
			case 1:
				system("clear");
				printf("***************************************************\n");
				printf("NUM   START    FINAL  \tDATE    TYPE   TIME  PRICE\n");
				show_list_data(head);
				printf("\n***************************************************\n");
				break;
			case 2:
				system("clear");
				q_sure = 1;
				while(q_sure)
				{
					printf("\n\n******************************************");
					printf("\n**** What kind of query do you need? *****\n");
					printf("******1 Date of take-off        **********\n");
					printf("******2 Type of the flight      **********\n");
					printf("******3 Time of take-off        **********\n");
					printf("******0 Quit                    **********\n");
					printf("******************************************\n");
					printf("choose your choice:");
					scanf("%d",&choose);
					switch(choose)
					{
						case 1:
							printf("Please insert the Date(eg:2017.1.1):");
							scanf("%s",date);
							system("clear");
							printf("***************************************************\n");
							printf("NUM   START    FINAL  \tDATE    TYPE   TIME  PRICE\n");
							show_date_node(head,date);
							printf("\n***************************************************\n");
			
							break;
						case 2:
							printf("Please insert the TYPE(eg:A):");
							scanf(" %c",&type);//%c前要加空格，为了让scanf在读取字符时在\n之前阻塞。
							system("clear");
							printf("***************************************************\n");
							printf("NUM   START    FINAL  \tDATE    TYPE   TIME  PRICE\n");
							show_type_node(head,type);
							printf("\n***************************************************\n");
							break;
						case 3:
							printf("Please insert the Time(eg:2:30am):");
							scanf("%s",stime);
							system("clear");
							printf("***************************************************\n");
							printf("NUM   START    FINAL  \tDATE    TYPE   TIME  PRICE\n");
							show_stime_node(head,stime);
							printf("\n***************************************************\n");
							break;
						case 0:
							q_sure = 0;
							system("clear");
							break;
					}
				}	
				break;
			case 3:
				printf("Please insert the Date(eg:A001):");
				scanf("%s",number);
				tmp = search_number_node(head,number);
				system("clear");
				printf("***************************************************\n");
				printf("NUM   START    FINAL  \tDATE    TYPE   TIME  PRICE\n");
				show_only(tmp);
				printf("\n***************************************************\n");
				break;
			case 4:
			    while(1)
				{
					printf("Please insert the Password(Insert quit can Quit):");
					scanf("%s",password);
					if(strcmp(password,"quit")==0)
					{
						system("clear");
						break;
					}
					get = Password(password);
					if(get == -1)
					{
						continue;
					}
				
					printf("Please insert the number(eg:A001):");
					scanf("%s",number);
					delete_number_node(head,number);
				
					break;
				}
				break;
			case 5:
				while(1)
				{
					printf("Please insert the Password(Insert quit can Quit):");
					scanf("%s",password);
					if(strcmp(password,"quit")==0)
					{
						system("clear");
						break;
					}
					get = Password(password);
					if(get == -1)
					{
						continue;
					}
				
					printf("Please insert the number(eg:A001):");
					scanf("%s",number);
					printf("Please insert the staddress(eg:义乌):");
					scanf("%s",staddress);
					printf("Please insert the arraddress(eg:北京):");
					scanf("%s",arraddress);
					printf("Please insert the date(eg:2017.1.1):");
					scanf("%s",date);
					printf("Please insert the type(eg:A):");
					scanf(" %c",&type);
					printf("Please insert the time(eg:1:00am):");
					scanf("%s",stime);
					printf("Please insert the price(eg:1000):");
					scanf("%d",&price);
					add_head(head,number,staddress,arraddress,date,type,stime,price);
					
					printf("Data is saving...\n");
					sleep(4);
					printf("Saving success!!\n");
					sleep(2);
					system("clear");
					break;
				}
				break;
			case 0:
				sure = 0;
				delete_list_node(head);
				printf("\nThank you for your use,goodbye! \n");
				sleep(2);
				
				break;
		}
	}
	
	return 0;
}

int the_org_data(PLNode head)//定义初始数据，也可以从文件读取
{
	if(head == NULL)
	{
		printf("head is NULL!\n");
		return -1;
	}
	add_tail(head,"A001","义乌","北京","2017.1.24",'A',"1:30am",3000);
	add_tail(head,"B002","上海","武汉","2017.2.23",'C',"2:15pm",2500);
	add_tail(head,"A003","上海","天津","2017.1.17",'B',"3:20am",3600);
	add_tail(head,"C004","武汉","上海","2017.2.20",'A',"3:10pm",2789);
	add_tail(head,"D005","广州","北京","2017.4.15",'B',"6:20am",3543);
	add_tail(head,"B006","广州","义乌","2017.5.25",'B',"7:20am",3132);
	add_tail(head,"E007","北京","广州","2017.6.30",'C',"6:10pm",2134);
	add_tail(head,"T008","广州","上海","2017.12.7",'C',"3:10am",2230);
	add_tail(head,"A009","新疆","广州","2018.1.20",'B',"5:30am",1999);
	add_tail(head,"C010","海南","北京","2018.2.10",'A',"7:20pm",3450);
	
	return 0;
}
int Password(char *password)//定义管理员密码
{
	char num[20] = "zzwandyhl2k100";
	if(strcmp(num,password)==0 )
	{
		printf("The password is correct\n");
		sleep(1);
		return 0;
	}
	printf("The password is wrong!Please try again.\n");
	return -1;
}