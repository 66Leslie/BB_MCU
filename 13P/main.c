#include "STC15F2K60S2.H"
#include <stdio.h>

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
/*----------------------sys variables--------------------------*/
uint8_t  pucSeg_Buf[15],pucSeg_Code[8];				
uint8_t  ucSeg_Pos;   					
uint16_t   uiSeg_Dly;    //刷新时间，单位ms
unsigned long ulms;
uint8_t  uiKey_Dly,ucKey_Old;    //保存按键延时以及上一次的键值
uint8_t ucMenu,ucled;
bit ucMenu1;
/*----------------------user variables--------------------------*/
unsigned char time[3] = {23,59,50};
float celcius;
int t_ref;
unsigned long buzz_ulms,time_ulms;
bit ctrl;
bit flag_buzz;
/*----------------------explanded functions---------------------*/
void Set_Rtc(unsigned char* ucRtc);
void Read_Rtc(unsigned char* ucRtc);
void led(unsigned char ucled);
void Timer0Init(void);
void Timer1Init(void);
float Read_temperature();
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
		if(ulms%200==0){
			float temp = 0;
			temp = Read_temperature();
			if(temp < 80) celcius = temp;
		}
		if(ctrl==0){
			if(celcius>t_ref){
				Buzz_Relay(0,1);
				flag_buzz = 1;
			}else{
				Buzz_Relay(0,0);
				flag_buzz = 0;
			}
		}
		else{
			if(time[1] == 0 && time[2] == 0){Buzz_Relay(0,1);flag_buzz=1;}
			else{Buzz_Relay(0,0);flag_buzz=0;}
		}
		Key_Proc();  //执行按键的操作
		Seg_Proc(); //控制SEG显示的内容
	}	
}
void Seg_Proc(void)
{
	if(uiSeg_Dly) return;  //控制刷新时间
	Read_Rtc(time);
	switch(ucMenu)
	{
	case 0://界面切换，时间，回显，参数
		sprintf(pucSeg_Buf,"U1   %3.1f",celcius);
		break;
	case 1:
		if(ucMenu1) sprintf(pucSeg_Buf,"U2 %02d-%02d",(unsigned int)time[1],(unsigned int)time[2]);
		else sprintf(pucSeg_Buf,"U2 %02d-%02d",(unsigned int)time[0],(unsigned int)time[1]);
		break;
	case 2:
		sprintf(pucSeg_Buf,"U3    %02d",t_ref);
		break;	
	default:break;
	}
	Seg_Tran(pucSeg_Buf,pucSeg_Code);
}
void T1_ISR(void) interrupt 3
{
	ulms++;
	if(++uiKey_Dly == 10) uiKey_Dly = 0;  //0-19
	if(++uiSeg_Dly == 99) uiSeg_Dly = 0; //0-499

	Seg_Disp(pucSeg_Code,ucSeg_Pos);     //SEG动态扫描，1ms
	if(++ucSeg_Pos == 8) ucSeg_Pos = 0; //0-7 

	if(ulms%100==0){
		if(time[1] == 0 && time[2] == 0){
			ucled |= 0x01;
			time_ulms = ulms;
		}
		if(!ctrl) ucled |= 0x02;
		else ucled &= 0xfd;
		if(flag_buzz)ucled ^= 0x04;
	}
	if(ulms - time_ulms > 5000 && time_ulms>0) ucled &= 0xfe;

	led(ucled);
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
	celcius = 0;
	t_ref = 23;
	ucMenu1=0;
	ulms = 0;
	ctrl = 0;
}
/*----------------------Timer--------------------------*/
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
    uint8_t ucKey_Down, ucKey_Val;

    // 防抖
    if (uiKey_Dly) return; // 0-19

    ucKey_Val = Key_Read();
    ucKey_Down = ucKey_Val & (ucKey_Val ^ ucKey_Old); // negedge
    ucKey_Old = ucKey_Val;
	
	if(ucKey_Down == 12){
		if(ucMenu>=2) ucMenu=0;
		else ucMenu++;
	}else if(ucKey_Down == 16 && ucMenu==2){
		if(t_ref < 99) t_ref++;
	}else if( ucKey_Down ==17 && ucMenu==2){
		if(t_ref > 10) t_ref--;
	}
	else if(ucKey_Down ==13){
		ctrl = ~ctrl;
	}else{
		if (ucMenu == 1 && ucKey_Val == 17) ucMenu1 = 1;
		else ucMenu1 = 0;
	}
}
/*----------------------led and buzz--------------------*/
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
			 case 'U': temp = 0xc1;break;//11000001	
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

