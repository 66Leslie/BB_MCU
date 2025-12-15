#include "STC15F2K60S2.H"
#include <stdio.h>

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
/*----------------------sys variables--------------------------*/
uint8_t  pucSeg_Buf[15],pucSeg_Code[8];	//存放SEG的显示内容"0.123"和编码0xC0 & 0x7f,0xf9,0xa4,0xb0
uint8_t  ucSeg_Pos;   //0-7 对应8个SEG
uint16_t   uiSeg_Dly; //刷新时间，单位ms
unsigned long ulms;
uint8_t  uiKey_Dly,ucKey_Old;    //保存按键延时以及上一次的键值
uint8_t ucMenu,ucled;
/*----------------------user variables--------------------------*/
unsigned char time[3] = {23,59,50};
/*----------------------explanded functions---------------------*/
void Set_Rtc(unsigned char* ucRtc);
void Read_Rtc(unsigned char* ucRtc);
void led(unsigned char ucled);
void Timer0Init(void);
void Timer1Init(void);
/*----------------------sys functions--------------------------*/
void Buzz_Relay(uint8_t buzz,uint8_t relay);
void System_Init(void);
void Key_Proc(void);
void Seg_Proc(void);
void Seg_Tran(unsigned char  *pucSeg_Buf,unsigned char  *pucSeg_Code);
void Seg_Disp(unsigned char  *pucSeg_Code,unsigned char  ucSeg_Pos);
// void f_det(void);
// float ReadTemperature(void);
// unsigned char PCF8591_ADC(unsigned char adc_ch);

int main(void)
{
	System_Init();
	while(1)
	{
		Key_Proc();  //执行按键的操作
		Seg_Proc(); //控制SEG显示的内容
	}	
}
/*---------------proc---------------*/
void Seg_Proc(void)
{
	if(uiSeg_Dly) return;  //控制刷新时间 每500ms
	Read_Rtc(time);
	switch(ucMenu)
	{
	case 0://界面切换，时间，回显，参数
		sprintf(pucSeg_Buf,"%02d-%02d-%02d",
			(unsigned int)time[0],(unsigned int)time[1],(unsigned int)time[2]);
		break;
	}
	Seg_Tran(pucSeg_Buf,pucSeg_Code);
}
void Key_proc(void){
    u8 key_val,key_down = 0;
    if(key_dly) return;
    key_val = Key_read();
    key_down = key_val & (key_val ^ key_old);
    key_old = key_val;
    if(key_down == 4){
        printf(Seg_buf);
        printf("   %4.2fC",t_cel);
    }
}
void System_Init(void)
{
	//关闭LED、BUZZ、Relay
	led(0x00);
	ucled = 0x00;
	Buzz_Relay(0,0);
	Set_Rtc(time);
	Timer0Init();
	Timer1Init();
	ucMenu = 0;
	//add your code here  根据初始状态要求来设置变量初值
	
	EA = 1; //开全局中断
}
/*---------------uart---------------*/
void Uart1_Init(void)	//9600bps@12.000MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR &= 0xFB;		//定时器时钟12T模式
	T2L = 0xE6;			//设置定时初始值
	T2H = 0xFF;			//设置定时初始值
	AUXR |= 0x10;		//定时器2开始计时
    ES = 1;
}
extern char putchar(unsigned char dat){
    SBUF = dat;
    while(TI == 0);//wait until data sent
    TI = 0;//clear the send flag
    return dat;
}
void Uart_Proc(void)
{
	if (Uart_R_index == 0)return;
	if (Sys_Tick >= 10)
	{
		Sys_Tick = Uart_Flag = 0;
		memset(Uart_R, 0, Uart_R_index);
		Uart_R_index = 0; 
	}
}
void Uart1Server() interrupt 4
{
	if (RI == 1) 
	{
		Uart_Flag = 1;				
		Sys_Tick = 0;				
		Uart_R[Uart_R_index] = SBUF;
		Uart_R_index++;				
		RI = 0;						
	}
	if (Uart_R_index > 10)
		Uart_R_index = 0;
}
/*----------------------1ms interrupt--------------------------*/
void T1_ISR(void) interrupt 3
{
	ulms++;

	if(++uiKey_Dly == 10) uiKey_Dly = 0;  //0-19
	if(++uiSeg_Dly == 152) uiSeg_Dly = 0; //0-499

	Seg_Disp(pucSeg_Code,ucSeg_Pos);     //SEG动态扫描，1ms
	if(++ucSeg_Pos == 8) ucSeg_Pos = 0; //0-7 

	led(ucled);
}

/*----------------------led and buzz--------------------------*/
void led(uint8_t ucLed) // xxxx xxxx
{
	 P0 = ~ucLed; //0-LED OFF,1-ON
	 P2 = P2 & 0x1F | 0x80; //LE = 1 P2 = 100x xxxx
	 P2 = P2 & 0x1F; //LE = 0; P2 = 000x xxxx	 	
}
void Buzz_Relay(uint8_t buzz,uint8_t relay)
{
	P0 = 0x00;
	if(buzz)
		P0 = P0 | 0x40;		//1-> BUZZ ON; 0100 0000
	if(relay)
		P0 = P0 | 0x10;		//1-> relay ON; 0001 0000
	P2 = P2 & 0x1F | 0xA0; //LE = 1 P2 = 101 0 0000
	P2 = P2 & 0x1F;	
}	
/*----------------------TimerInit--------------------------*/
void Timer1Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0x18;		//设置定时初值
	TH1 = 0xFC;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 1;		//定时器1开始计时
	ET1 = 1;		//enable Timer 1 interrupt
}
void Timer0Init(void)		//100微秒@12.000MHz
{
	TMOD &= 0xF0;
	TMOD |=	0x05;	//设置定时器模式
	TL0 = 0;		//设置定时初值
	TH0 = 0;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	EA  = 1;
}
/*----------------------key--------------------------*/
unsigned char Key_Read()
{
	unsigned char temp = 0;
	P44 = 0;P42 = 1;P35 = 1;P34 = 1;
	if(P33 == 0) temp = 4;
	if(P32 == 0) temp = 5;
	if(P31 == 0) temp = 6;
	if(P30 == 0) temp = 7;
	P44 = 1;P42 = 0;P35 = 1;P34 = 1;
	if(P33 == 0) temp = 8;
	if(P32 == 0) temp = 9;
	if(P31 == 0) temp = 10;
	if(P30 == 0) temp = 11;
	P44 = 1;P42 = 1;P35 = 0;P34 = 1;
	if(P33 == 0) temp = 12;
	if(P32 == 0) temp = 13;
	if(P31 == 0) temp = 14;
	if(P30 == 0) temp = 15;
	P44 = 1;P42 = 1;P35 = 1;P34 = 0;
	if(P33 == 0) temp = 16;
	if(P32 == 0) temp = 17;
	if(P31 == 0) temp = 18;
	if(P30 == 0) temp = 19;
	return temp;
}
void Key_Proc(void)
{
	 uint8_t ucKey_Down,ucKey_Val;
	 
	 	//防抖
	 if(uiKey_Dly) return; // 0-19
	 
	 ucKey_Val = Key_Read();	 
	 ucKey_Down = ucKey_Val & (ucKey_Val ^ ucKey_Old);	//negedge 	 
	 ucKey_Old = ucKey_Val;
}
/*------------------------------seg-----------------------*/
// "T - 28.35"->9个字符，多了'.'  “21-56-00”->8个字符
void Seg_Tran(unsigned char  *pucSeg_Buf,unsigned char  *pucSeg_Code)
{
	 uint8_t  i,j=0,temp;
	 for(i=0;i<8;i++,j++)
	 {	
			switch(pucSeg_Buf[j])
			{
			 case '0': temp = 0xC0;break;
			 case '1': temp = 0xf9;break;
			 case '2': temp = 0xa4;break;
			 case '3': temp = 0xb0;break;
			 case '4': temp = 0x99;break;
			 case '5': temp = 0x92;break;
			 case '6': temp = 0x82;break;
			 case '7': temp = 0xf8;break;
			 case '8': temp = 0x80;break;
			 case '9': temp = 0x90;break;
			 case 'A': temp = 0x88;break;
			 case 'B': temp = 0x83;break;	
			 case 'C': temp = 0xC6;break;
			 case 'D': temp = 0xa1;break;		
			 case 'E': temp = 0x86;break;
			 case 'F': temp = 0x8e;break;	
			 case '-': temp = 0xBF;break;  //1011 1111
			 case 'P': temp = 0x8c;break;//10001100
			 case ' ': temp = 0xFF;break;	
			 case 'H': temp = 0x89;break;	 //H 1000 1001		
			 default:temp = 0xFF;break;
		 }
		 //"C 28-23.2"
		 if(pucSeg_Buf[j+1] == '.')	
		 {
			  temp &= 0x7F; //0111 1111
			  j++; //0-8
		 }
			pucSeg_Code[i] = temp; //0 -7
	 }
}
//pucSeg_Code: 0xC0--> 0
//ucSeg_Pos : 0- 7
void Seg_Disp(unsigned char  *pucSeg_Code,unsigned char  ucSeg_Pos)
{
	 //消影
	 P0 = 0xFF;
	 P2 = P2 & 0x1F | 0xE0; // 1110 0000
	 P2 = P2 & 0x1F;
	
	//切换COM ucSeg_Pos:0->7
	 P0 = 1<< ucSeg_Pos;    //0000 0001,0000 0010,....1000 0000
     P2 = P2 & 0x1F | 0xC0; // 1100 0000
	 P2 = P2 & 0x1F;
	
	 //送Data
	 P0 = pucSeg_Code[ucSeg_Pos];
	 P2 = P2 & 0x1F | 0xE0; // 1110 0000
	 P2 = P2 & 0x1F;		
	
}
//同时按下按键读取
// uint8_t  Key_Value;
// uint16_t  Key_Data; //XXXX XXXX XXXX XXXX
// P44=0;P42=1;P35=1;P34=1;
// Key_Data = P3 & 0x0F;		//0000 1111
// P44=1;P42=0;P35=1;P34=1;
// Key_Data = (Key_Data <<4) | (P3 & 0x0F);
// P44=1;P42=1;P35=0;P34=1; 
// Key_Data = (Key_Data <<4) | (P3 & 0x0F);
// P44=1;P42=1;P35=1;P34=0; 
// Key_Data = (Key_Data <<4) | (P3 & 0x0F);	
// //1111 1111 1111 1111->0000 0000 0000 0000-??0x0000
// //0111 1111 1111 1111->1000 0000 0000 0000-> 0x8000