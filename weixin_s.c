#include "myhead.h"

int recv_file(char *ip_addr)
{
	
	int addrsize=sizeof(struct sockaddr_in);
	int ret;
	
	int server;
	
	//定义ipv4地址结构体变量，用于存放对方的ip和端口号
	struct sockaddr_in server_addr;
	
	//初始化该结构体变量
	bzero(&server_addr,sizeof(server_addr)); //memset()也行
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=inet_addr(ip_addr);
	server_addr.sin_port=htons(10000);
	//创建套接字
	server=socket(AF_INET,SOCK_STREAM,0);
	if(server==-1)
	{
		perror("create sock failed!\n");
		return -1;
	}
	//对服务器发送连接请求
	ret=connect(server,(struct sockaddr *)&server_addr,addrsize);
	if(ret==-1)
	{
		perror("拨号失败!\n");
		return -1;
	}
	//申请套接字数组
	//fd_set myset;
	char buf[1024];
	char file_name[1024];
	
	bzero(file_name,1024);
//	stpcpy(file_name,"test.jpg");
	//阻塞等待接收的文件名
	recv(server,file_name,1024,0);
	//发送接收成功的标志
	send(server,"OK",10,0);

	//在客户端的所在目录创建新的文件
	FILE *fp = fopen(file_name, "w");
	if(NULL == fp)  
	{  
		printf("File:\t%s Can Not Open To Write\n", file_name);  
		exit(1);  
	}  
	
	// 从服务器接收数据到buffer中  
	// 每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止  
	bzero(buf, 1024);  
	int length = 0;  //用来表示接收的字符串有多少字节
	while((length = recv(server,buf,1024,0)) > 0)  
	{  
		if(fwrite(buf,sizeof(char),length,fp)<length)  //若小于接收字节则一直写入直至写完
		{  
			printf("File:\t%s Write Failed\n", file_name);  
			break;  
		}  
		bzero(buf, 1024);  //清空数组用于再次接收
	}  
   
	printf("Receive File: %s From Server IP Successful!\n", file_name);  

	fclose(fp);  
	
	printf("file has been recv\n");
	
	//挂机
	close(server);
	return 0;
}

int talking_msg(char *ip_addr)
{
	int server;
	
	int addrsize=sizeof(struct sockaddr_in);
	int ret;
	char buf[100];
	//定义ipv4地址结构体变量，用于存放对方的ip和端口号
	struct sockaddr_in other_addr;
	//初始化该结构体变量
	bzero(&other_addr,sizeof(other_addr)); //memset()也行
	other_addr.sin_family=AF_INET;
	other_addr.sin_addr.s_addr=inet_addr(ip_addr);
	other_addr.sin_port=htons(10000);
	//申请套接字
	server=socket(AF_INET,SOCK_STREAM,0);
	if(server==-1)
	{
		perror("create sock failed!\n");
		return -1;
	}
	//发送连接请求
	ret=connect(server,(struct sockaddr *)&other_addr,addrsize);
	if(ret==-1)
	{
		perror("拨号失败!\n");
		return 0;
	}
	//创建套接字数组
	fd_set myset;
	
	while(1)
	{
		//添加套接字
		FD_ZERO(&myset);
		bzero(buf,100);
		
		FD_SET(server,&myset);//客户端连接不会产生新的套接字
		FD_SET(0,&myset);//添加套接字，0表示键盘的数据流文件描述符
		//配置套接字属性，第一个参数必须为最大套接字+1，其他参数无特殊要求可以为NULL
		ret = select(server+1,&myset,NULL,NULL,NULL);
		if(ret == -1)
		{
			perror("select failed!\n");
			return -1;
		}
		//使用FD_ISSET函数检测新套接字是否有反应
		if(FD_ISSET(server,&myset))
		{
			ret = recv(server,buf,100,0);//若接收到bye则自己退出通信
			if(strncmp(buf,"bye",3)==0)
			{
				printf("James has leave!\n");
				break;
			}
			if(ret == 0)
			{
				return -1;
			}
			printf("James:%s",buf);
			
		}
		//使用FD_ISSET函数检测键盘是否有输入
		if(FD_ISSET(0,&myset))
		{
			fgets(buf,100,stdin);
			send(server,buf,strlen(buf),0);
			if(strncmp(buf,"bye",3)==0)//若为自己输入bye则自己退出通信
			{
				break;
			}
			printf("You:%s",buf);
		}
	}
	
	//挂机
	close(server);
	
	return 0;
}

int main(int argc,char *argv[])
{
	
	int num;
	printf("\n***********************************\n");
	printf("****   1)Receive the file    ******\n");
	printf("****   2)Talking with James  ******\n");
	printf("****   0)Quit the programe   ******\n");
	printf("***********************************\n");
	while(1)
	{		
		printf("\nInsert your choose:");
		scanf("%d",&num);
		switch(num)
		{
			case 1:
				printf("\nWaiting connecting...\n");
				recv_file(argv[1]);
				break;
			case 2:
				printf("\nWaiting connecting...\n");
				talking_msg(argv[1]);
				break;
			case 0:
				return 0;
			default:
				printf("Please choose again!\n");
				break;
		}
	}
	return 0;
}