#include "myhead.h"
#include "jpeglib.h"
#include "jconfig.h"
#include "jerror.h"

 int show_pic(int x,int y,char *file_name)//在任意位置显示图片，图片规格小于800*480,暂未进行越界处理
{
	int fd;
	//打开开发板显示屏
	fd = open("/dev/fb0",O_RDWR);
	
	int *data = NULL;
	//对显示屏进行内存映射
	data = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	
	struct jpeg_decompress_struct info;
	struct jpeg_error_mgr erro;
	//使用jpeg库函数创建
	jpeg_create_decompress(&info);
	info.err = jpeg_std_error(&erro);
	
	FILE *pic = fopen(file_name,"r+");//打开要展示的图片
	//将图片传入数据函数中，函数将能获取图片的各种信息
	jpeg_stdio_src(&info,pic);
	jpeg_read_header(&info,true);
	jpeg_start_decompress(&info);
	//申请和长相同的空间占图片宽*3的字节
	char *buf = calloc(1,info.output_width*3);
	int new_buf[info.output_width];
	
	printf("图片宽是:%d 高是:%d\n",info.output_width,info.output_height);

	int i;
	//将图片扫描进内存映射中
	while(info.output_scanline < info.output_height)//判断扫描的屏幕高度是否达到屏幕自身的高度
	{	
		//将图片一次性读取出来存入buf中
		jpeg_read_scanlines(&info,(JSAMPARRAY)&buf,1);
		
		for(i=0;i<info.output_width;i++)
		{
			new_buf[i] = buf[3*i+2]|buf[3*i+1]<<8|buf[3*i]<<16;//jpeg格式是不会将原图片上下左右颠倒，因此只需要把颜色位调整就好
		}
		//将完整的一行像素全部映射到屏幕内存中
		memcpy(data+(y+info.output_scanline-1)*800+x,new_buf,info.output_width*4);
	}
	
	jpeg_finish_decompress(&info);//清空函数中的图片数据
	jpeg_destroy_decompress(&info);//释放处理函数的内存
	
	munmap(data,800*480*4);//释放映射内存空间
	
	fclose(pic);
	close(fd);
	
	return 0;
}

int send_file(char *ip_addr)//传输文件
{	
	int addrsize = sizeof(struct sockaddr_in);
	int ret;
	
	int newclient;
	//配置服务器的结构体
	struct sockaddr_in bindaddr;
	struct sockaddr_in other_addr;
	bindaddr.sin_family = AF_INET;//TCP使用AF,udp使用AP
	bindaddr.sin_addr.s_addr = inet_addr(ip_addr);//绑定ip地址
	bindaddr.sin_port = htons(10000);//绑定端口号
	
	//申请套接字
	int server = socket(AF_INET,SOCK_STREAM,0);
	if(server == -1)
	{
		perror("create failed!\n");
		return -1;
	}
	
	int on = 1;
	//解除端口号的引用，在程序结束的时候会自动释放端口号，在下次再使用时不会提示端口号已使用
	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	//绑定套接字	
	ret = bind(server,(struct sockaddr *)&bindaddr,addrsize);
	if(ret == -1)
	{
		perror("bind error!\n");
		return -1;
	}
	
	ret = listen(server,6);
	if(ret == -1)
	{
		perror("listen failed!\n");
		return -1;
	}
	
	//fd_set myset;
	//接受到其他IP地址发来的信息时产生新的套接字进行数据传输，即与其他IP地址进行连接
	newclient = accept(server,(struct sockaddr *)&other_addr,&addrsize);
	if(newclient == -1)
	{
		perror("accept failed!\n");
		return -1;
	}
	printf("Connecting IP is:%s\nPort is:%hu\n",inet_ntoa(other_addr.sin_addr),ntohs(other_addr.sin_port));

	
	char buf[1024];
	char file_name[1024];
	char flag[10];
	
	printf("insert the file you want send!\n");
	scanf("%s",file_name);
	//使用send函数将文件名提前送到客户端
	send(newclient,file_name,1024,0);
	//使用recv阻塞，等待客户端接收文件名后给予回应
	recv(newclient,flag,10,0);
	if(strcmp(flag,"OK")==0)
	{
		printf("Target is ready!\n");
		//定义文件描述符，打开文件
		FILE *fp = fopen(file_name, "r");  
		if(NULL == fp)  
		{  
			printf("File:%s Not Found\n", file_name);  
		}  
		else  
		{  
			bzero(buf, 1024);  
			int length = 0; 

			// 每读取一段数据，便将其发送给客户端，循环直到文件读完为止  
			while((length = fread(buf,sizeof(char),1024,fp)) > 0)  
			{  
				if(send(newclient,buf,length,0) < 0) //send的返回值为传输字符数，若=-1则表示传输结束
				{  
					printf("Send File:%s Failed./n", file_name);  
					break;  
				}  
				bzero(buf, 1024);  //清空buf待下次再次接收
			}  
			
			// 关闭文件  
			fclose(fp);  
			printf("File:%s Transfer Successful!\n", file_name);  
		}  
			
		printf("file has been sent!\n");
	}
	close(server);
	close(newclient);
	
	
	return 0;
}

int send_voice_file(char *ip_addr)//发送语音信息，其实相当于录音后再发送一个语音文件，只是文件名已经固定
{	
	int addrsize = sizeof(struct sockaddr_in);
	int ret;
	
	int newclient;
	//配置服务器的结构体
	struct sockaddr_in bindaddr;
	struct sockaddr_in other_addr;
	bindaddr.sin_family = AF_INET;//TCP使用AF,udp使用AP
	bindaddr.sin_addr.s_addr = inet_addr(ip_addr);//绑定ip地址
	bindaddr.sin_port = htons(10000);//绑定端口号
	//申请套接字
	int server = socket(AF_INET,SOCK_STREAM,0);
	if(server == -1)
	{
		perror("create failed!\n");
		return -1;
	}
	
	int on = 1;
	//解除端口号的引用，在程序结束的时候会自动释放端口号，在下次再使用时不会提示端口号已使用
	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	//绑定套接字	
	ret = bind(server,(struct sockaddr *)&bindaddr,addrsize);
	if(ret == -1)
	{
		perror("bind error!\n");
		return -1;
	}
	
	ret = listen(server,6);
	if(ret == -1)
	{
		perror("listen failed!\n");
		return -1;
	}
	
	fd_set myset;
	//接受到其他IP地址发来的信息时产生新的套接字进行数据传输，即与其他IP地址进行连接
	newclient = accept(server,(struct sockaddr *)&other_addr,&addrsize);
	if(newclient == -1)
	{
		perror("accept failed!\n");
		return -1;
	}
	printf("Connecting IP is:%s\nPort is:%hu\n",inet_ntoa(other_addr.sin_addr),ntohs(other_addr.sin_port));

	
	char buf[1024];
	char file_name[1024];
	char flag[10];
	
	strcpy(file_name,"voice.wav");
	//使用send函数将文件名提前送到客户端
	send(newclient,file_name,1024,0);
	//使用recv阻塞，等待客户端接收文件名后给予回应
	recv(newclient,flag,10,0);
	if(strcmp(flag,"OK")==0)
	{
		printf("Target is ready!\n");
		//定义文件描述符，打开文件
		FILE *fp = fopen(file_name, "r");  
		if(NULL == fp)  
		{  
			printf("File:%s Not Found\n", file_name);  
		}  
		else  
		{  
			bzero(buf, 1024);  
			int length = 0; 

			// 每读取一段数据，便将其发送给客户端，循环直到文件读完为止  
			while((length = fread(buf,sizeof(char),1024,fp)) > 0)  
			{  
				if(send(newclient,buf,length,0) < 0)  //send的返回值为传输字符数，若=-1则表示传输结束
				{  
					printf("Send File:%s Failed./n", file_name);  
					break;  
				}  
				bzero(buf, 1024); //清空buf待下次再次接收 
			}  

			// 关闭文件  
			fclose(fp);  
			printf("File:%s Transfer Successful!\n", file_name);  
		}  
			
		printf("file has been sent!\n");
	}
	close(server);
	close(newclient);
	
	
	return 0;
}

int ctrl = 0;
int open_voice()
{
	if(ctrl == 0)
	{
		system("aplay voice.wav &");
		ctrl = 1;
		return 0;
	}
	if(ctrl == 1)
	{
		system("killall -9 aplay");
		printf("aplay is end!\n");
		ctrl = 0;
		return 0;
	}
	
}

int talking_msg(char *ip_addr)//发送文字信息
{
	int newclient;
	
	int addrsize = sizeof(struct sockaddr_in);
	int ret;
	//配置服务器的结构体
	struct sockaddr_in bindaddr;
	struct sockaddr_in other_addr;
	bindaddr.sin_family = AF_INET;//TCP使用AF,udp使用AP
	bindaddr.sin_addr.s_addr = inet_addr(ip_addr);//绑定ip地址
	bindaddr.sin_port = htons(10000);//绑定端口号
	//申请套接字
	int server = socket(AF_INET,SOCK_STREAM,0);
	if(server == -1)
	{
		perror("create failed!\n");
		return -1;
	}
	
	int on = 1;
	//解除端口号的引用，在程序结束的时候会自动释放端口号，在下次再使用时不会提示端口号已使用
	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		
	ret = bind(server,(struct sockaddr *)&bindaddr,addrsize);
	if(ret == -1)
	{
		perror("bind error!\n");
		return -1;
	}
	
	ret = listen(server,6);
	if(ret == -1)
	{
		perror("listen failed!\n");
		return -1;
	}
	//使用select多路复用
	fd_set myset;//申请套接字组
	//接受到其他IP地址发来的信息时产生新的套接字进行数据传输，即与其他IP地址进行连接
	newclient = accept(server,(struct sockaddr *)&other_addr,&addrsize);
	if(newclient == -1)
	{
		perror("accept failed!\n");
		return -1;
	}
	printf("Connecting IP is:%s\nPort is:%hu\n",inet_ntoa(other_addr.sin_addr),ntohs(other_addr.sin_port));

	char buf[100];
	
	while(1)
	{	
		FD_ZERO(&myset);//清空套接字组 select特性：每次有一个套接字有反应则会清空其他套接字，只保留有反应的套接字，除了套接字，也可以添加文件描述符
		bzero(buf,100);//清空数据数组
		FD_SET(0,&myset);//添加套接字，0表示键盘的数据流文件描述符
		FD_SET(newclient,&myset);//添加新的套接字
		//配置套接字属性，第一个参数必须为最大套接字+1，其他参数无特殊要求可以为NULL
		ret = select(newclient+1,&myset,NULL,NULL,NULL);
		if(ret == -1)
		{
			perror("select error!\n");
			return -1;
		}
		//使用FD_ISSET函数检测新套接字是否有反应
		if(FD_ISSET(newclient,&myset))
		{
			ret = recv(newclient,buf,100,0);
			if(strncmp(buf,"bye",3)==0)//若接收到bye则自己退出通信
			{
				printf("Tony has leave!\n");
				break;
			}
			if(ret == 0)
			{
				return -1;
			}
			printf("Tony:%s",buf);
		}
		//使用FD_ISSET函数检测键盘是否有输入
		if(FD_ISSET(0,&myset))
		{
			fgets(buf,100,stdin);
			send(newclient,buf,strlen(buf),0);
			if(strncmp(buf,"bye",3)==0)
			{
				break;
			}
			printf("You:%s",buf);
		}
		
		
	}
	
	
	close(server);
	close(newclient);
	
	
	return 0;
}

int main(int argc,char *argv[])
{
	show_pic(108,190,"file.jpg");
	show_pic(320,190,"maikefeng.jpg");
	show_pic(532,190,"laba.jpg");
	show_pic(660,318,"quit.jpg");
	show_pic(660,82,"talking.jpg");
	
	int destop = open("/dev/input/event0",O_RDWR);
	
	if(destop<0)
		printf("open error!\n");
	
	struct input_event buf;
	int x,y;
	
	
	while(1)
	{
		bzero(&buf,sizeof(buf));
		read(destop,&buf,sizeof(buf));
		//读取手指按下显示屏的位置
		if(buf.type==EV_ABS&&buf.code==ABS_X)
			x=buf.value;
		if(buf.type==EV_ABS&&buf.code==ABS_Y)
			y=buf.value;
		//判断手指是否离开显示屏，这里是按下开始录音
		if(buf.type==EV_KEY&&buf.code==BTN_TOUCH&&buf.value==1)
		{
			if(x<448&&x>320&&y<318&&y>190)
			{
				system("arecord -d50 -c1 -r16000 -twav -fS16_LE voice.wav &");
			}
			
		}
		//判断手指是否离开显示屏，只有手指离开显示屏后才有反应，防止因为持续按着导致多次触发
		if(buf.type==EV_KEY&&buf.code==BTN_TOUCH&&buf.value==0)
		{
			
			if(x>108&&x<236&&y<318&&y>190)
			{
				
				printf("Waiting connecting...\n");
				send_file(argv[1]);
			}
			
			if(x<448&&x>320&&y<318&&y>190)
			{
				
				system("killall -9 arecord");
				printf("arecord is end!\n");
				printf("Waiting connecting...\n");
				send_voice_file(argv[1]);
			} 
			
			if(x>660&&x<768&&y>82&&y<190)
			{
				
				printf("Waiting connecting...\n");
				talking_msg(argv[1]);
			}
			
			if(x>532&&x<660&&y<318&&y>190)
			{
				open_voice();
			}
			
			if(x>660&&x<768&&y>318&&y<426)
			{
				printf("program is end!\n");
				break;
			}
		
		}
	}
	close(destop);
	
	return 0;
}
