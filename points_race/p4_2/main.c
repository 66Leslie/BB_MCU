#include "STC15F2K60S2.H"
#include <stdio.h>
#include <string.h>
#include <intrins.h>
unsigned char Uart_R[10];
unsigned char Uart_R_index;
unsigned char Sys_Tick;
unsigned char buf[8];
bit Uart_Flag;
void Uart_Proc();
void SendByte(unsigned char dat);
void Uart_SendStr(unsigned char *dat);
void Delay100ms();
void UartInit();

void main(void){
    UartInit();
    EA = 1;
    Uart_SendStr(buf);
    while(1){
        Uart_Proc();
        Uart_SendStr(buf);
    }
}
void Uart_Proc()
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
void Timer1_Isr(void) interrupt 3
{
	if (Uart_Flag)Sys_Tick++;
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

void led(unsigned char ucled){
    P0 = ~ucled;
    P2 = P2 & 0x1f | 0x80;
    P2 &= 0x1f;
}
void Delay100ms()		//@12.000MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 5;
	j = 144;
	k = 71;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}
