#include "STC15F2K60S2.H"
#include <stdio.h>
#include <string.h>
typedef unsigned char u8;
typedef unsigned int u16;
/*variables*/
u8 Seg_buf[15],Seg_pos;
u8 Seg_Code[8];
u8 key_old;
u8 SendBuf[8];
u16 key_dly,seg_dly;
u8 Sys_Tick;
bit Uart_Flag;
u8 Uart_R[10];
u8 Uart_R_index;
unsigned long ulms;
float t_cel;
/*functions*/
void Seg_tran(unsigned char* Seg_buf,unsigned char* Seg_Code);
void Seg_disp(unsigned char* Seg_Code,unsigned char Seg_pos);
unsigned char Key_read();
void sys_init();
float read_temperature();
void Uart1_Init(void);
void Timer1Init(void);
void Key_proc();
void Seg_proc();

void main(void){
    sys_init();
    while(1){
        Key_proc();
        Seg_proc();
        Uart_proc();
    }
}
/*---------------proc---------------*/
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
void Seg_proc(void){
    float temp;
    if(seg_dly) return;
    temp = read_temperature();
    t_cel=temp;
    sprintf(Seg_buf,"   %4.2fC",temp);
    Seg_tran(Seg_buf,Seg_Code);
}

/*---------------init---------------*/
void sys_init(){
    while(read_temperature()==85);
    ulms = 0;
    Timer1Init();
    Uart1_Init();
    key_dly = 0;
    seg_dly = 0;
}
void Timer1Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0x18;		//设置定时初值
	TH1 = 0xFC;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 1;		//定时器1开始计时
    ET1 = 1;
	EA  = 1;
}
/*---------------uart---------------*/
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
extern char putchar(char dat){
    SBUF = dat;
    while(TI == 0);//wait until data sent
    TI = 0;//clear the send flag
    return dat;
}
/*---------------interrupt---------------*/
void T1_ISR(void) interrupt 3{
    ulms++;
    if(Uart_Flag) Sys_Tick++;
    if(++key_dly==15) key_dly = 0;
    if(++seg_dly==199)seg_dly = 0;

    if(++Seg_pos==8) Seg_pos = 0;
    Seg_disp(Seg_Code,Seg_pos);
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
/*---------------seg---------------*/
void Seg_tran(unsigned char* Seg_buf,unsigned char* Seg_Code){
    u8 temp;
    u8 i,j;
    for(i=j=0;j<8;i++,j++){
        switch(Seg_buf[i]){
            case '0':temp = 0xc0;break;
            case '1':temp = 0xf9;break;//11111001
            case '2':temp = 0xa4;break;//10100100
            case '3':temp = 0xb0;break;
            case '4':temp = 0x99;break;
            case '5':temp = 0x92;break;
            case '6':temp = 0x82;break;
            case '7':temp = 0xf8;break;
            case '8':temp = 0x80;break;
            case '9':temp = 0x90;break;
            case 'C':temp = 0xc6;break;
            default:temp = 0xff;break;
        }
        if(Seg_buf[i+1] == '.'){
            temp &= 0x7f;
            i++;
        }
        Seg_Code[j] = temp;
    }
}
void Seg_disp(unsigned char* Seg_Code,unsigned char Seg_pos){
    //Eliminate shadows
    P0 = 0xff;//1 means extinguish and 0 means light
    P2 = P2 & 0x1f | 0xe0;
    P2 = P2 & 0x1f;
    //pos
    P0 = 1 << Seg_pos;
    P2 = P2 & 0x1f | 0xc0;
    P2 = P2 & 0x1f;
    //contents
    P0 = Seg_Code[Seg_pos];
    P2 = P2 & 0x1f | 0xe0;
    P2 = P2 & 0x1f;
}
unsigned char Key_read(void){
	u8 temp = 0;
    if(!P33) temp = 4;
    if(!P32) temp = 5;
    if(!P31) temp = 6;
    if(!P30) temp = 7;
	return temp;
}