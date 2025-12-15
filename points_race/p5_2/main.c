#include "STC15F2K60S2.H"
#include <stdio.h>
#include <string.h>
typedef unsigned char u8;
typedef unsigned int u16;
/*----------variales---------*/
u8 Seg_buf[15],Seg_pos;
u8 Seg_code[8];
u8 rtc[3]= {23,59,59};
u8 seg_dly;
u8 Save[10],save_index,sys_tick;
bit Uart_flag;
u8 settime[3];
unsigned long ulms;
unsigned int ulms_trg;
bit trg;

/*----------functions---------*/
void Seg_tran(unsigned char*Seg_buf,unsigned char*Seg_code);
void Seg_disp(unsigned char*Seg_code,unsigned char Seg_pos);
void Set_rtc(unsigned char * rtc);
void Rd_rtc(unsigned char * rtc);
void Timer1_Init(void);
void Uart1_Init(void);
void Seg_proc();
void sys_init();
void Uart_proc();
void buzz(bit en);
void main(void){
    sys_init();
    while(1)
    {
        Seg_proc();
        Uart_proc();
    }
}
void Seg_proc()
{
    u8 i,flag;
    if(seg_dly) return;
    Rd_rtc(rtc);
    sprintf(Seg_buf,"%2d-%2d-%2d",(unsigned int)rtc[0],(unsigned int)rtc[1],(unsigned int)rtc[2]);
    settime[0] = (Save[0]-48)*10 + Save[1]-48;
    settime[1] = (Save[4]-48)*10 + Save[5]-48;
    settime[2] = (Save[8]-48)*10 + Save[9]-48;
    for(i = 0;i < 3;i++){
        if(settime[i] == rtc[i]) flag = 1;
        else{flag = 0;break;};
    }
    if(flag == 1){
        buzz(1);
        ulms_trg = 2000;
    }
	if(ulms_trg == 0) buzz(0);
    printf("%2d",(unsigned int)settime[0]);
    Seg_tran(Seg_buf,Seg_code);   
}
void buzz(bit en){
    P0 = P0 & 0xaf; 
    if(en) P0 = P0 | 0x40;
    P2 = P2 & 0x1f | 0xa0;
    P2 = P2 & 0x1f;
}
void sys_init()
{
    
    Set_rtc(rtc);
    Timer1_Init();
    Uart1_Init();
    ulms = 0;
    ulms_trg = 0;
    EA = 1;

}

extern char putchar(char dat){
    SBUF = dat;
    while(TI == 0);
    TI = 0;
    return dat;
}
void Uart_proc()
{
    if(save_index == 0) return;
    if(sys_tick > 10){
        Uart_flag =  sys_tick = 0;
        memset(Save,0,save_index);
        save_index = 0;
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
void Uart_ISR(void) interrupt 4
{
    if(RI ==  1){
        Uart_flag = 1;
        sys_tick = 0;
        Save[save_index++] = SBUF;
        RI = 0;
    }
    if(save_index >= 10) save_index = 0; 
}
void T1_ISR(void) interrupt 3{
    ulms++;
    if(Uart_flag) sys_tick++;
    if(ulms_trg){
        ulms_trg--;
    }
    if(++seg_dly==199) seg_dly =0;
    if(++Seg_pos==8)Seg_pos=0;
    Seg_disp(Seg_code,Seg_pos);
}

void Timer1_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;			//定时器时钟12T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0x18;				//设置定时初始值
	TH1 = 0xFC;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
    ET1 = 1;
}

void Seg_tran(unsigned char *Seg_buf,unsigned char * Seg_code){
    u8 temp;
    u8 i,j;
    for(i=j=0;j<8;j++){
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
        Seg_code[j] =  temp;
        i++;
    }
}
void Seg_disp(unsigned char*Seg_code,unsigned char Seg_pos){
    P0 = 0xff;
    P2 = P2 & 0x1f | 0xe0;
    P2 = P2 & 0x1f;

    P0 = 1 << Seg_pos;
    P2 = P2 & 0x1f | 0xc0;
    P2 = P2 & 0x1f;

    P0 = Seg_code[Seg_pos];
    P2 = P2 & 0x1f | 0xe0;
    P2 = P2 & 0x1f;
}
