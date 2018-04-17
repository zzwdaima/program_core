#include<reg51.h> 
#include<intrins.h>
#include<string.h>

#define MotorTabNum 4

unsigned char T0_NUM;
typedef unsigned int u16;	  //对int类型进行声明定义
typedef unsigned char u8;	  //对char类型进行声明定义

/*-----------------------------------------------------------------------------*/
unsigned int wifi_vcc=0;
unsigned int wifi_turn=0;
unsigned int wifi_speed_up=0;
unsigned int wifi_speed_down=0;	

unsigned char cut_get[10];
unsigned int count_a=0;
unsigned int count_b=0;
unsigned int count_c=0;
unsigned char LED_Buffer[16],com_dat = 0;      //从串口接收的数据

/*------------------------------------------------------------------------------*/



sbit D1 = P0^0;		 //LED
sbit D2 = P0^1;
sbit D3 = P0^2;
sbit D4 = P0^3;

sbit IRIN=P3^2;		//红外线接口

sbit K1 = P3^3;		//光控接口
sbit K2 = P3^6;		 //K2
sbit K5 = P3^4;		//按键开关接口
sbit K4 = P3^5;		//K4
sbit K3 = P3^7;     //K5

sbit MotorEn = P1^1;     // 使能
sbit FX = P1^0;          // 方向
sbit CLK = P1^2;         // 脉冲


sbit SMG1 = P1^4;					//数码管第一位定义
sbit SMG2 = P1^5;					//数码管第二位定义
sbit SMG3 = P1^6;					//数码管第三位定义
sbit SMG4 = P1^7;					//数码管第四位定义

u8 IrValue[6];                       //红外线接收数组
u8 Time;                             //红外线时间信号
u8 K;

int table1[]={0x3f,0x06,0x5b,0x4f,
             0x66,0x6d,0x7d,0x07,
			 0x7f,0x6f,0x40};	   //共阴极数码管段值0~9

int table2[]={0x3f,0x06,0x5b,0x4f,0x66};				   //0~4

int n ;


unsigned char g_MotorSt = 0;     // 启停
unsigned char g_MotorDir = 0;    // 正反


unsigned char MotorTab[5] = {12, 10, 8, 6, 4};//加减速与这里的速度设定顺序有关


signed char g_MotorNum = 0;

//--------------调用已写函数-------------------//
void delayms(xms);
void mDelay(unsigned int DelayTime);		//延时函数
void T0_Init();

void KeyScan(void);
void Display(void);                  //数码管显示函数

void delay();
void IrInit();

void wifi_DelayUs2x(unsigned char t);
void wifi_DelayMs(unsigned char t);
void wifi_Delay(unsigned int del);
void Com_Init(void);
void Uart1Sends(unsigned char *str);
void esp8266_init();
void delaywifi(unsigned int ttt);
void wifi_duel();

// 主程序  main()
void main(void)
{   
	unsigned char i;
	com_dat = 0;
		
	wifi_Delay(100);
	wifi_Delay(300);

	
	IrInit();
	T0_Init();	     // 定时器0 初始化

	Com_Init();
	esp8266_init();

	MotorEn = 0;     // L297 使能(只有使能，才能正常工作，否则停止)
	FX = 0;			  //0正转，1反转
	
	while(1)
	{
		wifi_duel();
		Display();
		KeyScan();	      // 按键
	}

}

/* void main()
{
    com_dat = 0;
		
	wifi_Delay(100);
	wifi_Delay(300);
	Com_Init();
	esp8266_init();
	while(1)
	{
		for (count_a;count_a<16;count_a++)
		{
			if(LED_Buffer[count_a]==':')
			{
				count_a++;
				count_b = count_a+4;
				for(count_a;count_a<count_b;count_a++)
				{
					cut_get[count_c] = LED_Buffer[count_a];
					count_c++;
				}
			}
			
		}
		
		q = cut_get;
		
		if(strcmp(cut_get,"open")== 0) LED = 0;
		if(strcmp(cut_get,"clos")== 0) LED = 1;
	
		com_dat = 0;
		count_a=0;
		count_b=0;
		count_c=0;
		wifi_Delay(100);

	}
} */

/********定时器初始化********************************/
 void T0_Init()
{
	TMOD = 0x01;
	TH0 = 0xFF;  // 1ms
	TL0 = 0xA3;
	EA = 1;
	ET0 = 1;
//	TR0 = 1; 

} 

/********定时器中断*****************/
void T0_time() interrupt 1
{
	TR0 = 0;
    TH0 = 0xFF;  // 1ms
	TL0 = 0xA3;
	T0_NUM++;
	if(T0_NUM >= MotorTab[g_MotorNum])	// 增加按键g_MotorNum++   减小按键g_MotorNum--
	{
		T0_NUM = 0;
		CLK=CLK^0x01;	       //   输出脉冲
	}
	TR0 = 1;
}	 


//-----按键控制---------------------
void KeyScan(void)
{
	if(K1 == 0||K5 == 0||K == 0x45||wifi_vcc==1)
	{
	     	
		  // 软件延时消抖
		if(K1 == 0||K5 == 0||K == 0x45||wifi_vcc==1)
		{
	     
			Display();
	        D1 = 0 ;
			g_MotorSt = g_MotorSt ^0x01;
			MotorEn = 1;
			TR0 = 1;
			 
	    }
	}
	else
	{
	
	  D1 = 1 ;
	  MotorEn = 0;
	  TR0 = 1; 
	}

	if(K2 == 0||wifi_turn == 1)		   //正反转
	{
	
	   // 软件延时消抖
		if(K2 == 0||wifi_turn == 1)
		{		  
			while(!K2)
			{Display();}
			wifi_turn = 0;

		    D2 = ~D2 ;	 //默认正转的时候灯灭，反转时灯亮
			g_MotorDir = g_MotorDir ^ 0x01;
			FX ^= 1;
		
		}
	}

	if(K3 == 0||wifi_speed_up == 1)  // 加速
	{
	    D3 = 0; 
		delayms(50);   // 软件延时消抖
		D3 = 1;
		if(K3 == 0||wifi_speed_up == 1)
		{
		    while(!K3)
			{Display();}
			wifi_speed_up = 0;
			
			g_MotorNum++;
			if(g_MotorNum > MotorTabNum)
			{	g_MotorNum = MotorTabNum; }
		}
	}



	if(K4 == 0||wifi_speed_down == 1)  // 减速
	{
		D4 = 0;
		delayms(50);   // 软件延时消抖
		D4 = 1;
		if(K4 == 0||wifi_speed_down == 1)
		{
		    while(!K4)
			{Display();}
			wifi_speed_down = 0;

			g_MotorNum--;
			if(g_MotorNum < 0)
			{	g_MotorNum = 0;	}
		}
	}




}

/******************************************************************************
 * 名称： Set_Display()
 * 功能： 数码管显示函数
 * 输入： 无
 * 输出： 无
 * 说明：
******************************************************************************/
void Display(void)
{ 
	unsigned char b1, b2;
	b1=g_MotorNum/10;		
    b2=g_MotorNum%10;
	if(FX == 0)
		P2=0x71;//显示F 正转
    else
		P2=0x79;   //显示E  反转
	SMG1=0;							 
    delayms(1);
    SMG1=1;

	P2=0x40;
    SMG2=0;							 //显示--
    delayms(1);
    SMG2=1;

	P2=table1[b1];
	SMG3=0;							 //显示十位
	delayms(1);
	SMG3=1;

	P2=table2[b2];
    SMG4=0;
    delayms(1);						 //显示个位
    SMG4=1;
}

/********延时函数***********************************************************/
void delayms(xms)
{
	 unsigned int x,y;
	 for(x=xms;x>0;x--)
	 	for(y=110;y>0;y--);
}

/*******************************************************************************
* 函 数 名         : delay
* 函数功能		   : 延时函数，i=1时，大约延时10us
*******************************************************************************/
void delay(u16 i)
{
	while(i--);	
}

/*******************************************************************************
* 函数名         : IrInit()
* 函数功能		   : 初始化红外线接收
* 输入           : 无
* 输出         	 : 无
*******************************************************************************/

void IrInit()
{
    
	IT0=1;//下降沿触发
	EX0=1;//打开中断0允许

	IRIN=1;//初始化端口
	
}

/*******************************************************************************
* 函数名         : ReadIr()
* 函数功能		   : 读取红外数值的中断函数
* 输入           : 无
* 输出         	 : 无
*******************************************************************************/

void ReadIr() interrupt 0
{
	u8 j,k;
	u16 err;
	Time=0;					 
	delay(700);	//7ms
	if(IRIN==0)		//确认是否真的接收到正确的信号
	{	 
		
		err=1000;				//1000*10us=10ms,超过说明接收到错误的信号
		/*当两个条件都为真是循环，如果有一个条件为假的时候跳出循环，免得程序出错的时
		侯，程序死在这里*/	
		while((IRIN==0)&&(err>0))	//等待前面9ms的低电平过去  		
		{			
			delay(1);
			err--;
		} 
		if(IRIN==1)			//如果正确等到9ms低电平
		{
			err=500;
			while((IRIN==1)&&(err>0))		 //等待4.5ms的起始高电平过去
			{
				delay(1);
				err--;
			}
			for(k=0;k<4;k++)		//共有4组数据
			{				
				for(j=0;j<8;j++)	//接收一组数据
				{

					err=60;		
					while((IRIN==0)&&(err>0))//等待信号前面的560us低电平过去
					{
						delay(1);
						err--;
					}
					err=500;
					while((IRIN==1)&&(err>0))	 //计算高电平的时间长度。
					{
						delay(10);	 //0.1ms
						Time++;
						err--;
						if(Time>30)
						{
							return;
						}
					}
					IrValue[k]>>=1;	 //k表示第几组数据
					if(Time>=8)			//如果高电平出现大于565us，那么是1
					{
						IrValue[k]|=0x80;
					}
					Time=0;		//用完时间要重新赋值							
				}
			}
		}
		K = IrValue[2];		 //将红外线发送的数据赋值给K
//------------对K的值进行判断-------------------//       
	   
		if(K == 0x47)		  //关闭
		{
			D1 = 1 ;
	        MotorEn = 0;
	        TR0 = 1; 
		}
		if(K == 0x07)		   //正反转
	    {
	
		      D2 = ~D2 ;	 //默认正转的时候灯灭，反转时灯亮
			  g_MotorDir = g_MotorDir ^ 0x01;
			  FX ^= 1;
		
		  
	     }
		if(K == 0x09)  // 加速
		{
	      D3 = 0; 
		  delayms(50);   // 软件延时消抖
		  D3 = 1;  
		  g_MotorNum++;
		  if(g_MotorNum > MotorTabNum)
		  {  g_MotorNum = MotorTabNum; }
		  
	     }

		 if(K == 0x15)  // 减速
	     {
	    	D4 = 0;
	    	delayms(50);   // 软件延时消抖
    		D4 = 1;
			g_MotorNum--;
	    	if(g_MotorNum < 0)
	    	{  g_MotorNum = 0;	}
          }

//------------------------------------------------//

		if(IrValue[2]!=~IrValue[3])
		{
			return;
		}
	}			
}

/******************************************************
 *红外线模块只需要
 *void delay();
 *void IrInit();
 *void ReadIr();
 *这3个函数便可实现初始化，从而对相应值进行判断
 开发板遥控器上按键键值设置（对应位置）：
    
      开关：45H   mode：46H    静音：47H    
	下一首：44H   快退：40H    快进：43H
	    EQ：07H   vol-：15H    vol+：09H
		 0：16H    RPT: 19H    U/SD: 0DH
		 1: 0CH      2: 18H      3: 5EH
		 4: 08H		 5: 1CH	     6: 5AH
		 7: 42H		 8: 52H	     9: 4AH

45用作开，44用作关，07用作正反转，15用作减速，09用作加速
 ******************************************************/
 
/*-------------------------------------------------------------------------------*/
/*------------------------------------------------
 uS延时函数，含有输入参数 unsigned char t，无返回值
 unsigned char 是定义无符号字符变量，其值的范围是
 0~255 这里使用晶振12M，精确延时请使用汇编,大致延时
 长度如下 T=tx2+5 uS 
------------------------------------------------*/
void wifi_DelayUs2x(unsigned char t)
{   
 while(--t);
}
/*------------------------------------------------
 mS延时函数，含有输入参数 unsigned char t，无返回值
 unsigned char 是定义无符号字符变量，其值的范围是
 0~255 这里使用晶振12M，精确延时请使用汇编
------------------------------------------------*/
void wifi_DelayMs(unsigned char t)
{
     
 while(t--)
 {
     //大致延时1mS
     wifi_DelayUs2x(245);
	 wifi_DelayUs2x(245);
 }
}

void wifi_Delay(unsigned int del)
{
	unsigned int i,j;
	for(i=0; i<del; i++)
	for(j=0; j<1827; j++)    
	;
}

/********************************************************************
* 名称 : Com_Int()
* 功能 : 串口中断子函数
* 输入 : 无
* 输出 : 无
***********************************************************************/
void Com_Int(void) interrupt 4
{
	ES = 0;
	if(RI == 1)   //当硬件接收到一个数据时，RI会置位
	{
		LED_Buffer[com_dat] = SBUF; 		//把从串口读出的数存到数组
		RI = 0; 
		com_dat++;
		if(com_dat == 15)
		{
			com_dat = 0;		//当com_dat = 16时，清0，防止数组溢出
		}
	}
	
	ES = 1;
}

/********************************************************************
* 名称 : Com_Init()
* 功能 : 串口初始化，晶振11.0592,波特率9600，使能了串口中断
* 输入 : 无
* 输出 : 无
***********************************************************************/
void Com_Init(void)
{
     TMOD = 0x20;   //定时器工作在定时器1的方式2
     PCON = 0x00;   //不倍频
     SCON = 0x50;	//串口工作在方式1，并且启动串行接收
	 
     TH1 = 0xFD;    //设置波特率 9600
     TL1 = 0xFD;						  
	 
     TR1 = 1;		//启动定时器1	  
	 ES = 1;		//开串口中断	   
	 EA = 1;		//开总中断
	  
}

 //串行口连续发送char型数组，遇到终止号/0将停止
void Uart1Sends(unsigned char *str)
{
	while(*str!='\0')
	{
		SBUF=*str;
		while(!TI);//等待发送完成信号（TI=1）出现
		TI=0;
		str++;
	}
}

//延时函数
void delaywifi(unsigned int ttt)
{
while(ttt--);
}		

//ESP8266上电初始化
void esp8266_init()
{
	Uart1Sends("AT+CIPMUX=1\r\n");
	delaywifi(50000);
	Uart1Sends("AT+CIPSERVER=1,8080\r\n");//端口
}
//处理接收字符串函数
void wifi_duel()
{
	//由于ESP8266输出时会带乱码，所以需要进行从中选取需要的数据
		for (count_a;count_a<16;count_a++)
		{
			if(LED_Buffer[count_a]==':')//以冒号为标志
			{
				count_a++;
				count_b = count_a+4;//选取冒号后4位
				for(count_a;count_a<count_b;count_a++)
				{
					cut_get[count_c] = LED_Buffer[count_a];//将数据保存到新的数组中
					count_c++;
				}
			}
			
		}
		
		if(strcmp(cut_get,"open")== 0) 
		{
			wifi_vcc = 1;
		}
		if(strcmp(cut_get,"clos")== 0) 
		{
			wifi_vcc = 0;
		}
		if(strcmp(cut_get,"turn")== 0) 
		{
			wifi_turn = 1;
		}
		if(strcmp(cut_get,"s_up")== 0) 
		{
			wifi_speed_up = 1;
		}
		if(strcmp(cut_get,"s_dn")== 0) 
		{
			wifi_speed_down = 1;
		}
	    
		strcpy(LED_Buffer,"nullnullnull");//由于主函数进行循环而ESP8266只发送一次，但清空后，函数中的for循环会越界，所以赋任意值让其循环
		strcpy(cut_get,"null");//理由同上
		com_dat = 0;
		count_a=0;
		count_b=0;
		count_c=0;
	//	wifi_Delay(100);
	  
} 
/*------------------------WIFI模块51板相应函数-------------------------------------*/
/*
	void Com_Int(void) interrupt 4;		//接收串口的数据
	void Com_Init(void);    //配置串口的发送，波特率，定时器等模块
	void Uart1Sends(unsigned char *str);	//打开串口
	void delaywifi(unsigned int ttt);
	void esp8266_init();   //由单片机发送指令初始化WIFI模块
	
 */