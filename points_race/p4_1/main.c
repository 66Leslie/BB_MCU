#include <STC15F2K60S2.H>
#include <stdio.h>
/*variables*/
unsigned char ucSeg_buf[15],ucSeg_pos;
unsigned char ucSeg_Code[8];
unsigned char Key_old;
unsigned char ucMenu;
unsigned char flag;
unsigned int seg_dly,key_dly;
unsigned int t_ref,t_left;
unsigned long ulms;
unsigned int ulms_cover;
unsigned int ulms_stay;
unsigned char ucled; 
/*functions*/
void Seg_tran(unsigned char* ucSeg_buf,unsigned char* ucSeg_Code);
void Seg_disp(unsigned char* ucSeg_Code,unsigned char ucSeg_pos);
void Timer0Init(void);
void Timer1Init(void);
void Sys_init(void);
void Key_proc(void);
void Seg_proc(void);
unsigned char Key_Read(void);
unsigned char adc(unsigned char ch);
void Relay_Buzz(bit relay,bit buzz);
void led(unsigned char ucled);
void main(){
	Sys_init();
	while(1){
		Key_proc();
		Seg_proc();
	}
}   
void Seg_proc(){
	static unsigned char light;
	switch(seg_dly%21){
		case 9:
			light = adc(0x01);
			if(light<30){
				flag = 1;
				Relay_Buzz(1,0);
				ulms_cover=0;
			}
			else{
				if(flag) ulms_stay = 3000;
				flag = 0;
				Relay_Buzz(0,0);
			}break;
		default:break;
		case 19:
			switch(ucMenu){
				case 0:ucled = 0x01;break;
				case 1:ucled = 0x02;break;
				case 2:ucled = 0x04;break;
				default:break;
			}
	}
	if(flag || ulms_stay) ucMenu = 1;
	if(seg_dly) return;
	switch(ucMenu){
		case 0:
			sprintf(ucSeg_buf,"HELLO   ");break;
		case 1:
			sprintf(ucSeg_buf,"C     %2d",t_left);break;
		case 2:
			sprintf(ucSeg_buf,"E     %2d",t_ref);break;
		default: break;
	}
	Seg_tran(ucSeg_buf,ucSeg_Code);
}
void T1_ISR(void) interrupt 3
{
	ulms++;
	if(++key_dly==13) key_dly=0;
	if(++seg_dly==99) seg_dly=0;
	Seg_disp(ucSeg_Code,ucSeg_pos);
	if(++ucSeg_pos == 8) ucSeg_pos = 0;
	if(flag){
		ulms_cover++;
		if(t_left==0){
			flag = 0;
			Relay_Buzz(0,0);
			ulms_stay = 3000;
		}
		else t_left -= ulms_cover%1000; 
	}else{
		if(ulms_stay) ulms_stay--;
		else ucMenu = 0;
	} 
	led(ucled);
}
void Sys_init(void){
	Timer0Init();
	Timer1Init();
	led(0x00);
	ulms = 0;
	key_dly = 0;
	seg_dly = 0;
	ucMenu = 0;
	t_ref = 5;
	t_left = t_ref;
	ulms_cover = ulms_stay = 0;
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
	EA =1;
}
void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TMOD |= 0x05;
	TL0 = 0x00;		//设置定时初值
	TH0 = 0x00;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	EA  = 1;
}
/*------------------------led----------------------*/
void led(unsigned char ucled){
	P0 = ~ucled;
	P2 = P2 & 0x1f |0x80;
	P2 = P2 & 0x1f;
}
/*--------------------relay&buzz-------------------*/
void Relay_Buzz(bit relay,bit buzz){
	if(relay) P0 |= 0x10;
	if(buzz)  P0 |= 0x40;
	P2 = P2 & 0x1f | 0xa0;
	P2 = P2 & 0x1f;
}
/*----------------------key----------------------*/
void Key_Proc(){
	unsigned char Key_val,Key_down;
	if(key_dly) return;
	Key_val = Key_Read();
	Key_down = Key_val & (Key_val ^ Key_old);//negedge
	Key_old = Key_val;

	if(Key_down == 4){
		switch(ucMenu){
			case 0:ucMenu=2;break;
			case 2:		
				if(t_ref<30)t_ref += 5;
				t_left = t_ref;
				break;
			default:break;
		}
		ulms_stay = 3000;
	}
}
unsigned char Key_Read(){
	unsigned char temp;
	P44=0;P42=1;P35=1;P34=1;
	if(!P33) temp = 4;
	if(!P32) temp = 5;
	if(!P31) temp = 6;
	if(!P30) temp = 7;
	P44=1;P42=0;P35=1;P34=1;
	if(!P33) temp = 8;
	if(!P32) temp = 9;
	if(!P31) temp = 10;
	if(!P30) temp = 11;
	P44=1;P42=1;P35=0;P34=1;
	if(!P33) temp = 12;
	if(!P32) temp = 13;
	if(!P31) temp = 14;
	if(!P30) temp = 15;
	P44=1;P42=1;P35=1;P34=0;
	if(!P33) temp = 16;
	if(!P32) temp = 17;
	if(!P31) temp = 18;
	if(!P30) temp = 19;
	return temp;
}
/*----------------------seg----------------------*/
void Seg_tran(unsigned char* ucSeg_buf,unsigned char* ucSeg_Code){
	unsigned char i,j;
	unsigned char temp;
	for(i=j=0;j<8;i++,j++){
		switch(ucSeg_buf[i])
		{
			case '0': temp = 0xc0;break;
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
			case 'L': temp = 0xc7;break;//0x11000111
			case 'O': temp = 0xc0;break;//0x1100 0000
			default:  temp = 0xff;break;
		}
		if(ucSeg_buf[i+1]=='.'){
			temp &= 0x7f;
			i++;
		}
		ucSeg_Code[j] = temp;
	}
}
void Seg_disp(unsigned char* ucSeg_Code,unsigned char ucSeg_pos){
	//clean the left effects by repeat the last action
	P0 = 0xff;
	P2 = P2 & 0x1f | 0xe0;
	P2 = P2 & 0x1f;
	//choose the postion
	P0 = 0x01 << ucSeg_pos;
	P2 = P2 & 0x1f | 0xc0;
	P2 = P2 & 0x1f;
	//choose the content
	P0 = ucSeg_Code[ucSeg_pos];
	P2 = P2 & 0x1f | 0xe0;
	P2 = P2 & 0x1f;
}