#include<reg51.h> 
#include<intrins.h>
#include<string.h>

#define MotorTabNum 4

unsigned char T0_NUM;
typedef unsigned int u16;	  //��int���ͽ�����������
typedef unsigned char u8;	  //��char���ͽ�����������

/*-----------------------------------------------------------------------------*/
unsigned int wifi_vcc=0;
unsigned int wifi_turn=0;
unsigned int wifi_speed_up=0;
unsigned int wifi_speed_down=0;	

unsigned char cut_get[10];
unsigned int count_a=0;
unsigned int count_b=0;
unsigned int count_c=0;
unsigned char LED_Buffer[16],com_dat = 0;      //�Ӵ��ڽ��յ�����

/*------------------------------------------------------------------------------*/



sbit D1 = P0^0;		 //LED
sbit D2 = P0^1;
sbit D3 = P0^2;
sbit D4 = P0^3;

sbit IRIN=P3^2;		//�����߽ӿ�

sbit K1 = P3^3;		//��ؽӿ�
sbit K2 = P3^6;		 //K2
sbit K5 = P3^4;		//�������ؽӿ�
sbit K4 = P3^5;		//K4
sbit K3 = P3^7;     //K5

sbit MotorEn = P1^1;     // ʹ��
sbit FX = P1^0;          // ����
sbit CLK = P1^2;         // ����


sbit SMG1 = P1^4;					//����ܵ�һλ����
sbit SMG2 = P1^5;					//����ܵڶ�λ����
sbit SMG3 = P1^6;					//����ܵ���λ����
sbit SMG4 = P1^7;					//����ܵ���λ����

u8 IrValue[6];                       //�����߽�������
u8 Time;                             //������ʱ���ź�
u8 K;

int table1[]={0x3f,0x06,0x5b,0x4f,
             0x66,0x6d,0x7d,0x07,
			 0x7f,0x6f,0x40};	   //����������ܶ�ֵ0~9

int table2[]={0x3f,0x06,0x5b,0x4f,0x66};				   //0~4

int n ;


unsigned char g_MotorSt = 0;     // ��ͣ
unsigned char g_MotorDir = 0;    // ����


unsigned char MotorTab[5] = {12, 10, 8, 6, 4};//�Ӽ�����������ٶ��趨˳���й�


signed char g_MotorNum = 0;

//--------------������д����-------------------//
void delayms(xms);
void mDelay(unsigned int DelayTime);		//��ʱ����
void T0_Init();

void KeyScan(void);
void Display(void);                  //�������ʾ����

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

// ������  main()
void main(void)
{   
	unsigned char i;
	com_dat = 0;
		
	wifi_Delay(100);
	wifi_Delay(300);

	
	IrInit();
	T0_Init();	     // ��ʱ��0 ��ʼ��

	Com_Init();
	esp8266_init();

	MotorEn = 0;     // L297 ʹ��(ֻ��ʹ�ܣ�������������������ֹͣ)
	FX = 0;			  //0��ת��1��ת
	
	while(1)
	{
		wifi_duel();
		Display();
		KeyScan();	      // ����
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

/********��ʱ����ʼ��********************************/
 void T0_Init()
{
	TMOD = 0x01;
	TH0 = 0xFF;  // 1ms
	TL0 = 0xA3;
	EA = 1;
	ET0 = 1;
//	TR0 = 1; 

} 

/********��ʱ���ж�*****************/
void T0_time() interrupt 1
{
	TR0 = 0;
    TH0 = 0xFF;  // 1ms
	TL0 = 0xA3;
	T0_NUM++;
	if(T0_NUM >= MotorTab[g_MotorNum])	// ���Ӱ���g_MotorNum++   ��С����g_MotorNum--
	{
		T0_NUM = 0;
		CLK=CLK^0x01;	       //   �������
	}
	TR0 = 1;
}	 


//-----��������---------------------
void KeyScan(void)
{
	if(K1 == 0||K5 == 0||K == 0x45||wifi_vcc==1)
	{
	     	
		  // �����ʱ����
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

	if(K2 == 0||wifi_turn == 1)		   //����ת
	{
	
	   // �����ʱ����
		if(K2 == 0||wifi_turn == 1)
		{		  
			while(!K2)
			{Display();}
			wifi_turn = 0;

		    D2 = ~D2 ;	 //Ĭ����ת��ʱ����𣬷�תʱ����
			g_MotorDir = g_MotorDir ^ 0x01;
			FX ^= 1;
		
		}
	}

	if(K3 == 0||wifi_speed_up == 1)  // ����
	{
	    D3 = 0; 
		delayms(50);   // �����ʱ����
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



	if(K4 == 0||wifi_speed_down == 1)  // ����
	{
		D4 = 0;
		delayms(50);   // �����ʱ����
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
 * ���ƣ� Set_Display()
 * ���ܣ� �������ʾ����
 * ���룺 ��
 * ����� ��
 * ˵����
******************************************************************************/
void Display(void)
{ 
	unsigned char b1, b2;
	b1=g_MotorNum/10;		
    b2=g_MotorNum%10;
	if(FX == 0)
		P2=0x71;//��ʾF ��ת
    else
		P2=0x79;   //��ʾE  ��ת
	SMG1=0;							 
    delayms(1);
    SMG1=1;

	P2=0x40;
    SMG2=0;							 //��ʾ--
    delayms(1);
    SMG2=1;

	P2=table1[b1];
	SMG3=0;							 //��ʾʮλ
	delayms(1);
	SMG3=1;

	P2=table2[b2];
    SMG4=0;
    delayms(1);						 //��ʾ��λ
    SMG4=1;
}

/********��ʱ����***********************************************************/
void delayms(xms)
{
	 unsigned int x,y;
	 for(x=xms;x>0;x--)
	 	for(y=110;y>0;y--);
}

/*******************************************************************************
* �� �� ��         : delay
* ��������		   : ��ʱ������i=1ʱ����Լ��ʱ10us
*******************************************************************************/
void delay(u16 i)
{
	while(i--);	
}

/*******************************************************************************
* ������         : IrInit()
* ��������		   : ��ʼ�������߽���
* ����           : ��
* ���         	 : ��
*******************************************************************************/

void IrInit()
{
    
	IT0=1;//�½��ش���
	EX0=1;//���ж�0����

	IRIN=1;//��ʼ���˿�
	
}

/*******************************************************************************
* ������         : ReadIr()
* ��������		   : ��ȡ������ֵ���жϺ���
* ����           : ��
* ���         	 : ��
*******************************************************************************/

void ReadIr() interrupt 0
{
	u8 j,k;
	u16 err;
	Time=0;					 
	delay(700);	//7ms
	if(IRIN==0)		//ȷ���Ƿ���Ľ��յ���ȷ���ź�
	{	 
		
		err=1000;				//1000*10us=10ms,����˵�����յ�������ź�
		/*������������Ϊ����ѭ���������һ������Ϊ�ٵ�ʱ������ѭ������ó�������ʱ
		�������������*/	
		while((IRIN==0)&&(err>0))	//�ȴ�ǰ��9ms�ĵ͵�ƽ��ȥ  		
		{			
			delay(1);
			err--;
		} 
		if(IRIN==1)			//�����ȷ�ȵ�9ms�͵�ƽ
		{
			err=500;
			while((IRIN==1)&&(err>0))		 //�ȴ�4.5ms����ʼ�ߵ�ƽ��ȥ
			{
				delay(1);
				err--;
			}
			for(k=0;k<4;k++)		//����4������
			{				
				for(j=0;j<8;j++)	//����һ������
				{

					err=60;		
					while((IRIN==0)&&(err>0))//�ȴ��ź�ǰ���560us�͵�ƽ��ȥ
					{
						delay(1);
						err--;
					}
					err=500;
					while((IRIN==1)&&(err>0))	 //����ߵ�ƽ��ʱ�䳤�ȡ�
					{
						delay(10);	 //0.1ms
						Time++;
						err--;
						if(Time>30)
						{
							return;
						}
					}
					IrValue[k]>>=1;	 //k��ʾ�ڼ�������
					if(Time>=8)			//����ߵ�ƽ���ִ���565us����ô��1
					{
						IrValue[k]|=0x80;
					}
					Time=0;		//����ʱ��Ҫ���¸�ֵ							
				}
			}
		}
		K = IrValue[2];		 //�������߷��͵����ݸ�ֵ��K
//------------��K��ֵ�����ж�-------------------//       
	   
		if(K == 0x47)		  //�ر�
		{
			D1 = 1 ;
	        MotorEn = 0;
	        TR0 = 1; 
		}
		if(K == 0x07)		   //����ת
	    {
	
		      D2 = ~D2 ;	 //Ĭ����ת��ʱ����𣬷�תʱ����
			  g_MotorDir = g_MotorDir ^ 0x01;
			  FX ^= 1;
		
		  
	     }
		if(K == 0x09)  // ����
		{
	      D3 = 0; 
		  delayms(50);   // �����ʱ����
		  D3 = 1;  
		  g_MotorNum++;
		  if(g_MotorNum > MotorTabNum)
		  {  g_MotorNum = MotorTabNum; }
		  
	     }

		 if(K == 0x15)  // ����
	     {
	    	D4 = 0;
	    	delayms(50);   // �����ʱ����
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
 *������ģ��ֻ��Ҫ
 *void delay();
 *void IrInit();
 *void ReadIr();
 *��3���������ʵ�ֳ�ʼ�����Ӷ�����Ӧֵ�����ж�
 ������ң�����ϰ�����ֵ���ã���Ӧλ�ã���
    
      ���أ�45H   mode��46H    ������47H    
	��һ�ף�44H   ���ˣ�40H    �����43H
	    EQ��07H   vol-��15H    vol+��09H
		 0��16H    RPT: 19H    U/SD: 0DH
		 1: 0CH      2: 18H      3: 5EH
		 4: 08H		 5: 1CH	     6: 5AH
		 7: 42H		 8: 52H	     9: 4AH

45��������44�����أ�07��������ת��15�������٣�09��������
 ******************************************************/
 
/*-------------------------------------------------------------------------------*/
/*------------------------------------------------
 uS��ʱ����������������� unsigned char t���޷���ֵ
 unsigned char �Ƕ����޷����ַ���������ֵ�ķ�Χ��
 0~255 ����ʹ�þ���12M����ȷ��ʱ��ʹ�û��,������ʱ
 �������� T=tx2+5 uS 
------------------------------------------------*/
void wifi_DelayUs2x(unsigned char t)
{   
 while(--t);
}
/*------------------------------------------------
 mS��ʱ����������������� unsigned char t���޷���ֵ
 unsigned char �Ƕ����޷����ַ���������ֵ�ķ�Χ��
 0~255 ����ʹ�þ���12M����ȷ��ʱ��ʹ�û��
------------------------------------------------*/
void wifi_DelayMs(unsigned char t)
{
     
 while(t--)
 {
     //������ʱ1mS
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
* ���� : Com_Int()
* ���� : �����ж��Ӻ���
* ���� : ��
* ��� : ��
***********************************************************************/
void Com_Int(void) interrupt 4
{
	ES = 0;
	if(RI == 1)   //��Ӳ�����յ�һ������ʱ��RI����λ
	{
		LED_Buffer[com_dat] = SBUF; 		//�ѴӴ��ڶ��������浽����
		RI = 0; 
		com_dat++;
		if(com_dat == 15)
		{
			com_dat = 0;		//��com_dat = 16ʱ����0����ֹ�������
		}
	}
	
	ES = 1;
}

/********************************************************************
* ���� : Com_Init()
* ���� : ���ڳ�ʼ��������11.0592,������9600��ʹ���˴����ж�
* ���� : ��
* ��� : ��
***********************************************************************/
void Com_Init(void)
{
     TMOD = 0x20;   //��ʱ�������ڶ�ʱ��1�ķ�ʽ2
     PCON = 0x00;   //����Ƶ
     SCON = 0x50;	//���ڹ����ڷ�ʽ1�������������н���
	 
     TH1 = 0xFD;    //���ò����� 9600
     TL1 = 0xFD;						  
	 
     TR1 = 1;		//������ʱ��1	  
	 ES = 1;		//�������ж�	   
	 EA = 1;		//�����ж�
	  
}

 //���п���������char�����飬������ֹ��/0��ֹͣ
void Uart1Sends(unsigned char *str)
{
	while(*str!='\0')
	{
		SBUF=*str;
		while(!TI);//�ȴ���������źţ�TI=1������
		TI=0;
		str++;
	}
}

//��ʱ����
void delaywifi(unsigned int ttt)
{
while(ttt--);
}		

//ESP8266�ϵ��ʼ��
void esp8266_init()
{
	Uart1Sends("AT+CIPMUX=1\r\n");
	delaywifi(50000);
	Uart1Sends("AT+CIPSERVER=1,8080\r\n");//�˿�
}
//��������ַ�������
void wifi_duel()
{
	//����ESP8266���ʱ������룬������Ҫ���д���ѡȡ��Ҫ������
		for (count_a;count_a<16;count_a++)
		{
			if(LED_Buffer[count_a]==':')//��ð��Ϊ��־
			{
				count_a++;
				count_b = count_a+4;//ѡȡð�ź�4λ
				for(count_a;count_a<count_b;count_a++)
				{
					cut_get[count_c] = LED_Buffer[count_a];//�����ݱ��浽�µ�������
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
	    
		strcpy(LED_Buffer,"nullnullnull");//��������������ѭ����ESP8266ֻ����һ�Σ�����պ󣬺����е�forѭ����Խ�磬���Ը�����ֵ����ѭ��
		strcpy(cut_get,"null");//����ͬ��
		com_dat = 0;
		count_a=0;
		count_b=0;
		count_c=0;
	//	wifi_Delay(100);
	  
} 
/*------------------------WIFIģ��51����Ӧ����-------------------------------------*/
/*
	void Com_Int(void) interrupt 4;		//���մ��ڵ�����
	void Com_Init(void);    //���ô��ڵķ��ͣ������ʣ���ʱ����ģ��
	void Uart1Sends(unsigned char *str);	//�򿪴���
	void delaywifi(unsigned int ttt);
	void esp8266_init();   //�ɵ�Ƭ������ָ���ʼ��WIFIģ��
	
 */