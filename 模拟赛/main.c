#include "STC15F2K60S2.H"
#include <Delay.h>
#include <intrins.h>
#include <key.h>
#include <seg.h>
#include <base.h>
#include <iic.h>
#include <stdio.h>
#include <ds1302.h>
typedef unsigned char u8;
typedef unsigned int  u16;
u8 key = 0;
u8 Seg_pos = 1;
u8 Seg_Buff[8] = {0};
u8 uwTick;
u8 ucMenu;
u16 save01[3] = {0,0,0};
u16 save02[3] = {0,0,0};
u16 save03[3] = {0,0,0};
unsigned int putRTC[3] = {23,59,50};
u16 save_pos = 0;
u8 V_light,V_rp;
unsigned int trg = 0;
void Seg_Proc();
unsigned  char k;
unsigned int first01,first02,first03;
void Timer0Init(void)		//1毫秒@12.000MHz
{
	TMOD &= 0xF0;		//设置定时器模式,定时器0是低4位， GATE C/T M1 M0
	TMOD |= 0x01;		//设置定时器模式
	TL0 = 0x18;			//设置定时初值低八位(65535-goal)/256
	TH0 = 0xFC;			//设置定时初值高八位(65535-goal)%256
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0 = 1;				//定时中断使能
	EA  = 1;				//使能全局中断
}

void Timer0Server() interrupt 1
{	//定时器达到TH才会进来此中断
	//由于单片机计数完一次（低八位=高八位）后默认从0启动
	TL0 = 0x18;		//设置定时初值,定时器低八位
	TH0 = 0xFC;		//设置定时初值,定时器高八位
	uwTick++;
	if(++Seg_pos == 8) Seg_pos = 0;
	if((trg!=0)&&(++trg) == 3000) trg = 0;
	if(ucMenu == 1 && (Seg_pos == 1 ||Seg_pos == 5))
	Seg_Disp(Seg_pos,Seg_Buff[Seg_pos],0);
	else Seg_Disp(Seg_pos,Seg_Buff[Seg_pos],1);
}
int main(){
	Set_RTC();
	Timer0Init();
	for(k=0;k<3;k++)save01[k]=save02[k]=save03[k]=0;
	while(1)
	{	
		Seg_Proc();
	}  
}

void Key_Proc(){
	
	
}
void Seg_Proc(){
	if(trg)return;//此时如果在触发状态，不继续判断
	key = key_judge();
	if(key == 4) ucMenu += (ucMenu==2)?-2:1;
	if(ucMenu==2){
		if(key==5)save_pos+=save_pos==2?-2:1;
		if(key==8)for(k=0;k<3;k++)save01[k]=save02[k]=save03[k]=0;}
	V_rp   = Ad_Read(0x41);//AIN1光敏电阻
	V_light= Ad_Read(0x43);//AIN3滑动变阻器
	Read_RTC();
	if(trg) return;
	else{	
		if(V_light >= V_rp) trg = 0;
		else{
			for(k=0;k<3;k++) save03[k]=save02[k];
			sprintf(Seg_Buff,"CC%2d%2d%2d",putRTC[0],putRTC[1],putRTC[2]);
			for(k=0;k<3;k++) save02[k]=save01[k];
			for(k=0;k<3;k++) save01[k]=putRTC[k];
			first01=save01[0];
			first02=save01[0];
			first03=save01[0];
			LED(8);
			trg= 1;
		}
	}
	if(trg) return;
	switch(ucMenu)
	{
		case 0: {sprintf(Seg_Buff,"%02d-%02d-%02d",putRTC[0],putRTC[1],putRTC[2]);LED(1);}break;
		case 1: {sprintf(Seg_Buff,"P%3dU%3d",(unsigned int)(V_light*1.961),(unsigned int)(V_rp*1.961));LED(2);}break;
		case 2: {
				switch(save_pos){
				case 0: {if((save01[0]+save01[1]+save01[2])==0)sprintf(Seg_Buff,"A1------");
								 else sprintf(Seg_Buff,"A1%02d%02d%02d",putRTC[0],save01[1],save01[2]);};break;
				case 1: {if((save02[0]+save02[1]+save02[2])==0)sprintf(Seg_Buff,"A2------");
								 else sprintf(Seg_Buff,"A2%02d%02d%02d",putRTC[0],save02[1],save02[2]);};break;
				case 2: {if((save03[0]+save03[1]+save03[2])==0)sprintf(Seg_Buff,"A3------");
								 else sprintf(Seg_Buff,"A3%02d%02d%02d",putRTC[0],save03[1],save03[2]);};break;}
				LED(3);
				break;
				}
	}
}
