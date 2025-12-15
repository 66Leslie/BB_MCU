#include <STC15F2K60S2.H>
#include <it.h>
#include <seg.h>
#include <key.h>
extern unsigned char uwTick;
extern unsigned char key;
extern unsigned char ucMenu;
extern unsigned int putRTC[3];
extern unsigned int save[3];
extern unsigned char V_light,V_rp;
extern unsigned int trg =0;
/* 定时器模块 */
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
	if((++trg) == 1000) trg = 0;
	
	if(++Seg_pos == 8) Seg_pos = 0;
	if(ucMenu == 0 && (Seg_pos == 1 ||Seg_pos == 5))
		Seg_Disp(Seg_pos,Seg_Buff[Seg_pos],0);
	else Seg_Disp(Seg_pos,Seg_Buff[Seg_pos],1);
	
	if(trg) return;//此时如果在触发状态，不继续判断
	
	if(V_light > V_rp) trg = 0;
	else if(V_light < V_rp){
		trg= 1;
		save[2]	= putRTC[0]*10000 + putRTC[1]*100 + putRTC[2];
	}
	
	if(uwTick % 3 ==0){
		key = key_judge();
		if(key == 4) ucMenu += (ucMenu==2)?-2:1;
	}
}