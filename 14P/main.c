#include "STC15F2K60S2.H"
#include <stdio.h>
#include <intrins.h>
typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
//variables_new
unsigned int freq;
unsigned int wet;
float t_cel;
int light,light_ref;
uint8_t ucMenu,ucMenu1;
uint8_t t_ref;
uint8_t count_trg;
bit flag_trg;
uint16_t t_max,wet_max;
uint8_t save_h,save_m;
bit flag_warn;
float t_cel_old,wet_old;
unsigned char ucled;
unsigned long key_long_ms;
float t_average,wet_average;
unsigned long ulms_trg;
//variables in model
unsigned long ulms;
uint8_t  uiKey_Dly,ucKey_Old;    //保存按键延时以及上一次的键值
unsigned char putRTC[3] = {23,59,50};
uint8_t  pucSeg_Buf[12],pucSeg_Code[8];				//存放SEG的显示内容"0.123"和编码0xC0 & 0x7f,0xf9,0xa4,0xb0
uint8_t  ucSeg_Pos;   					//0-7 对应8个SEG
uint16_t   uiSeg_Dly;    //刷新时间，单位ms
bit cal_flag;
//hardware function
void System_Init(void);
void Key_Proc(void);
void Seg_Proc(void);
void Seg_Tran(unsigned char  *pucSeg_Buf,unsigned char  *pucSeg_Code);
void Seg_Disp(unsigned char  *pucSeg_Code,unsigned char  ucSeg_Pos);
void led(uint8_t ucled);
void f_det(void);
void Set_RTC(unsigned char* putRTC);				//ds1302, 设置时间，时间存放在putRTC数组中
void Read_RTC(unsigned char* putRTC);				//ds1302, 读取时间，时间存放在putRTC数组中
float Read_temperature();
unsigned char PCF8591_ADC(unsigned char adc_ch);
void Delay750ms();
int main(void)
{
	System_Init();
	while(1)
	{		
		Key_Proc();  //执行按键的操作
		Seg_Proc(); //控制SEG显示的内容
	}	
}
void Seg_Proc(void)
{
	switch(uiSeg_Dly){
		case 1:{
			if(ucMenu == 0) ucled = ucled&0xf8|0x01;
			else if(ucMenu == 1)ucled = ucled&0xf8|0x02;
			else if(ucMenu == 2)ucled = ucled&0xf8|0x00;
			if(count_trg>1){
				if(t_cel>t_cel_old&&wet>wet_old)ucled = ucled|0x20;
				else ucled = ucled&0xdf;
			}
			if(wet == 0 && count_trg>0) ucled = ucled|0x10;
			else ucled = ucled & 0xef;
			if(flag_trg) ucled |=  0x04;
		}break;
		case 22:light = PCF8591_ADC(0x01);
		default:break;
	}
	if(uiSeg_Dly) return;  //控制刷新时间 每500ms
	if(flag_trg == 1 && cal_flag == 1){//触发界面
		t_cel = Read_temperature();
		if(t_cel > 4095) t_cel = 0;
		if (freq > 2000 || freq < 200) wet = 0;
		else wet = 10 + (freq-200)*80/1800.0;
		t_cel_old = t_cel;
		wet_old = wet;
		save_h = putRTC[0];
		save_m = putRTC[1];
		//报警灯
		if(t_cel > t_ref) flag_warn=1;
		else{
			ucled = ucled & 0xf7;
			flag_warn=0;
		}	
		//参数计算
		if(wet){
			count_trg++;
			if(t_cel>t_max) t_max = t_cel;
			t_average = (t_average*(count_trg-1)+t_cel)/count_trg;
			if(wet>wet_max) wet_max =wet;
			wet_average=(wet_average*(count_trg-1)+wet)/count_trg;
			cal_flag = 0;
		}
	}
	Read_RTC(putRTC);
	if(flag_trg){
		if(!wet) sprintf(pucSeg_Buf,"E %02.0f-AA",t_cel);  
		else sprintf(pucSeg_Buf,"E %02.0f-%02d",t_cel,wet); 
	}else{
		switch(ucMenu)
		{
		case 0://界面切换，时间，回显，参数
			sprintf(pucSeg_Buf,"%02d-%02d-%02d",(unsigned int)putRTC[0],(unsigned int)putRTC[1],(unsigned int)putRTC[2]);
			break;
		case 1:
		if(count_trg ==0){
			if(ucMenu1 == 0)sprintf(pucSeg_Buf,"C       ");//温度
			else if(ucMenu1==1) sprintf(pucSeg_Buf,"H       ");//湿度
			else sprintf(pucSeg_Buf,"F00     ");//时间
		}else{
			if(ucMenu1 == 0)sprintf(pucSeg_Buf,"C %02d-%3.1f",t_max,t_average);//温度
			else if(ucMenu1==1) sprintf(pucSeg_Buf,"H %02d-%3.1f",wet_max,wet_average);//湿度
			else sprintf(pucSeg_Buf,"F%02d%02d-%02d",(unsigned int)count_trg,(unsigned int)save_h,(unsigned int)save_m);//时间
		}break;
		case 2:
			sprintf(pucSeg_Buf,"P     %2d",(int)t_ref);break;
		default:break;
		}
	}
	Seg_Tran(pucSeg_Buf,pucSeg_Code);
}
void T1_ISR(void) interrupt 3
{
	ulms++;
	if(ulms % 250 == 0){
		f_det();
	}
	if(ucKey_Old == 9) key_long_ms++;
	else key_long_ms =0;
	if(ulms - ulms_trg > 3000) flag_trg =0;
	if(++uiKey_Dly == 15) uiKey_Dly = 0;  //0-19
	if(++uiSeg_Dly == 99) uiSeg_Dly = 0; //0-499
	Seg_Disp(pucSeg_Code,ucSeg_Pos);     //SEG动态扫描，1ms
	if(++ucSeg_Pos == 8) ucSeg_Pos = 0; //0-7 
	if(ulms%100 == 0){
		if(flag_warn)ucled = ucled  ^ 0x08;
		if(flag_trg == 0){
			if(light_ref - light > 10)
			{
				ulms_trg = ulms;
				cal_flag = 1;
				flag_trg = 1;
			}
			light_ref = light;
		}
	}
	led(ucled);
}
void f_det(void){
	TR0 = 0;
	if(TF0 == 1) freq = 65535; //溢出
	else freq =((TH0<<8) + TL0)*4; //未溢出
	TH0 = 0;
	TL0 = 0;
	TR0 = 1;
}
/*----------------------led and buzz--------------------------*/
void led(uint8_t ucLed) // xxxx xxxx
{
	 P0 = ~ucLed; //0-LED OFF,1-ON
	 P2 = P2 & 0x1F | 0x80; //LE = 1 P2 = 100x xxxx
	 P2 = P2 & 0x1F; //LE = 0; P2 = 000x xxxx	 	
}
//init
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
void System_Init(void)
{

	//关闭LED、BUZZ、Relay
	led(0x00);
	ucled = 0x00;
	while (Read_temperature()==85);
	Delay750ms();
	Set_RTC(putRTC);
	Timer0Init();
	Timer1Init();
	ucMenu = 0;
	ucMenu1 = 0;
	ulms = 0;
	ulms_trg = 0;
	count_trg = 0;
	t_average =0;
	t_ref = 30;
	t_max=0;
	t_cel = 0;
	wet_average =0;
	wet_max =0;
	flag_trg =0;
	save_h =0;
	save_m = 0;
	key_long_ms =0;
	light = 0;
	light_ref=0;
	flag_warn =0;
}
//0,4-19 - >s4-s19
/*----------------------key--------------------------*/
unsigned char Key_Read()
{
	unsigned char temp = 0;
	P44 = 0;P42 = 1;P35 = 1;
	if(P33 == 0) temp = 4;
	if(P32 == 0) temp = 5;
	if(P31 == 0) temp = 6;
	if(P30 == 0) temp = 7;
	P44 = 1;P42 = 0;P35 = 1;
	if(P33 == 0) temp = 8;
	if(P32 == 0) temp = 9;
	if(P31 == 0) temp = 10;
	if(P30 == 0) temp = 11;
	P44 = 1;P42 = 1;P35 = 0;
	if(P33 == 0) temp = 12;
	if(P32 == 0) temp = 13;
	if(P31 == 0) temp = 14;
	if(P30 == 0) temp = 15;
	P44 = 1;P42 = 1;P35 = 1;
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
	 if(flag_trg) return;
	 if(ucKey_Down == 4) // s4
	 {
		if((ucMenu+1)>=3) ucMenu=0;
		else ucMenu++;
		if(ucMenu == 1) ucMenu1 =0;
	 }
	 if(ucKey_Down == 5 && ucMenu == 1) // s4
	 {
		if((ucMenu1+1)==3) ucMenu1=0;
		else ucMenu1++;
	 }
	 if(ucMenu == 2 && (ucKey_Down == 8 ||ucKey_Down == 9)) // s8
	 {
		 if(ucKey_Down ==8) t_ref += t_ref>98?0:1;
		 else t_ref -= t_ref<1?0:1;
	 }
	 if(key_long_ms>=2000 && ucKey_Val == 0){
		count_trg = 0;
		t_average = 0;
		t_max =0;
		wet_average = 0;
		wet_max =0;
		save_h =0;
		save_m =0;
	 }
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
void Delay750ms()		//@12.000MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 35;
	j = 51;
	k = 182;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

