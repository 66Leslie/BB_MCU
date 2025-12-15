#include "STC15F2K60S2.H"
#include "stdio.h"
#define key_delay 15
#define seg_delay 99
/*-------------variables-------------*/
bit menu_0,menu_2;
unsigned char flag_light[3];
unsigned char seg_buf[15],seg_pos;
unsigned char seg_code[8];
unsigned char seg_dly,key_dly;
unsigned char key_old;
unsigned char Menu;
unsigned char menu_3;
unsigned char ucled;
unsigned char duty;
unsigned char count_relay;

unsigned int freq;
unsigned int wet;
unsigned int dist;

unsigned int freq_ref,wet_ref,dist_ref;

unsigned long ulms;

/*-------------functions-------------*/
void Seg_tran(unsigned char*seg_buf,unsigned char*seg_code);
void Seg_disp(unsigned char* seg_code,unsigned char seg_pos);
void Seg_proc();
void Key_proc();
void freq_detect();
void sys_init();
void Timer1_Init(void);
void Timer0_Init(void);
void Timer2_Init(void);
void dac_out(unsigned char val);

void compare();
void cal();

void relay(bit en);
void motor(bit en);
void pulse(unsigned char duty);
unsigned char ultrasound_data();
unsigned char key_read(void);
unsigned char adc_read(unsigned char ch);
void e2prom_write(unsigned char*str,unsigned char address,unsigned char num);
void e2prom_read(unsigned char*str,unsigned char address,unsigned char num);

void cal(){
	unsigned char adc,dac;
	adc = adc_read(3);
    if(adc) wet = adc*100/255.0;
	if(wet > 100) wet = 100;
	if(wet>80){
		dac = 5.0;
	}else if(wet < wet_ref) dac=1.0;
	else dac = 1.0+(wet-wet_ref)*4.0/(80-wet_ref);
	dac = dac * 255.0 / 5.0;
	dac_out(dac);
    dist = ultrasound_data();
}
void compare(){
    static bit flag;//record the last time
    if(dist > dist_ref){
        if(flag == 0)count_relay++;
        relay(1);
        flag = 1;
        ucled |= 0x20;
    }
    else{
        if(flag == 1)count_relay++;
        relay(0);
        flag = 0;
        ucled &= 0xdf;
    }
    e2prom_write(&count_relay,0,1);

    if(freq>freq_ref){
        duty = 80;
        ucled |= 0x10;
    }
    else{
        duty = 20;
        ucled &= 0xef;
    }

    if(wet>wet_ref) ucled |= 0x08;
    else ucled &= 0xf7;
}
void led(unsigned char ucled){
    //1-->light
    P0 = ~ucled;    
    P2 = P2 & 0x1f | 0x80;
    P2 &= 0x1f;
}
void led_proc(void){
    if(Menu==0) ucled = ucled&0xfe|0x01;
    else ucled = ucled&0xfe;
    if(Menu==1) ucled = ucled&0xfd|0x02;
    else ucled = ucled&0xfd;
    if(Menu==2) ucled = ucled&0xfb|0x04;
    else ucled = ucled&0xfb;

    if(Menu==3){
        if(menu_3==0)flag_light[0]=1;
        else flag_light[0]=0;
        if(menu_3==1)flag_light[1]=1;
        else flag_light[1]=0;
        if(menu_3==2)flag_light[2]=1;
        else flag_light[2]=0;
    }else{
        flag_light[0]=0;
        flag_light[1]=0;
        flag_light[2]=0;
    }
}
void main(){
    sys_init();
    while(1){
        Key_proc();
        Seg_proc();
    }
}
void Seg_proc(){
    if(seg_dly < seg_delay) return;
    else seg_dly = 0;
	cal();
	compare();
	led_proc();
    switch(Menu){
        case 0://freq
            if(menu_0)sprintf(seg_buf,"F %6d",freq);
            else sprintf(seg_buf,"F %7.1f",freq/1000.0);break;
        case 1://wet
            sprintf(seg_buf,"H     %2d",wet);break;
        case 2:
            if(menu_2)sprintf(seg_buf,"A    %3d",dist);
            else sprintf(seg_buf,"A    %4.2f",dist/100.0);break;
        case 3:
            if(menu_3 == 0)sprintf(seg_buf,"P1   %4.1f",freq_ref/1000.0);
            else if(menu_3 == 1)sprintf(seg_buf,"P2    %2d",wet_ref);
            else if(menu_3 == 2)sprintf(seg_buf,"P3    %3.1f",dist_ref/100.0);
            else;break;
        default:break;
    }
    Seg_tran(seg_buf,seg_code);
}
void Timer1_Isr(void) interrupt 3
{
    ulms++;
    if(ulms % 250 == 0) freq_detect(); 
    if(seg_dly < seg_delay) seg_dly++;
    if(key_dly < key_delay) key_dly++;
    if(++seg_pos==8) seg_pos=0;
    Seg_disp(seg_code,seg_pos);
    if((ulms % 100==0) && (Menu==3)){
        if(flag_light[0]) ucled ^= 0x01;
        else ucled &= 0xfe;
        if(flag_light[1]) ucled ^= 0x02;
        else ucled &= 0xfd;
        if(flag_light[2]) ucled ^= 0x04;
        else ucled &= 0xfb;
    }
    led(ucled);
}
void Timer2_Isr(void) interrupt 12
{
    static unsigned char count;
    if(++count==10) count = 0;
    motor(duty > count*10);
}
/*-------------key-------------*/
void Key_proc(){
    unsigned char key_val,key_down,key_up;
    static unsigned long s7_record;
    if(key_dly < key_delay) return;
    else key_dly = 0;
    key_val = key_read();
    key_down = key_val & (key_val ^ key_old);
    if(key_old == 7 && key_val == 0)key_up = 7;
	else key_up = 0;
    key_old = key_val;
    if(key_val == 7 && s7_record == 0) s7_record = ulms;
    
    if(ulms - s7_record > 1000){
        if(key_up == 7)count_relay = 0;
    } 
	if(key_val != 7) s7_record = 0;
    switch(key_down){
        case 4:Menu += Menu<3?1:-3;
			   if(Menu == 3)menu_3=0;
               break;
        case 5:if(Menu==3) menu_3 += menu_3<2?1:-2;break;
        case 6:if(Menu==2)menu_2 = ~menu_2;//distane menu
                if(Menu==3){//param menu
                    switch(menu_3){
                        case 0: freq_ref +=500;break;//hz
                        case 1: wet_ref += 10;break;//%
                        case 2: dist_ref+=10;break;//cm
                    }
                }break;
        case 7:
            if(Menu==0)menu_0=~menu_0;
            if(Menu==3){//param menu
                switch(menu_3){
                    case 0: freq_ref -=500;break;//hz
                    case 1: wet_ref -= 10;break;//%
                    case 2: dist_ref-=10;break;//cm
                    default:break;
                }
            }break;
        default:break;
    }

    if(freq_ref > 12000) freq_ref=1000;
    else if(freq_ref < 1000) freq_ref = 12000;
    if(wet_ref > 60) wet_ref = 10;
    else if(wet_ref<10) wet_ref = 60;
    if(dist_ref > 120) dist_ref = 10;
    else if(dist_ref < 10)dist_ref = 120;

}
unsigned char key_read(){
    unsigned char temp;
    ET1 = 0;
	temp = 0;
    if(!P33) temp = 4;
    if(!P32) temp = 5;
    if(!P31) temp = 6;
    if(!P30) temp = 7;
    ET1 = 1;
    return temp;
}
/*-------------init-------------*/
void sys_init(){
    Timer1_Init();
    Timer0_Init();
    Timer2_Init();
    freq = 0;
    wet = 0;
    dist = 0;
    freq_ref = 9000;
    wet_ref = 40;
    dist_ref = 60;
    seg_dly = 0;
    key_dly = 0;
    menu_0 = 1;
    menu_2 = 1;
    menu_3 = 0;
    Menu = 0;
    ulms = 0;
    EA = 1;
}
void Timer0_Init(void)		//100微秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xf0;			//设置定时器模式
	TMOD |= 0x05;			//设置定时器模式
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x00;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}
void freq_detect(){
    TR0=0;
    if(TF0) freq = 65535;
    else{
        freq = (TH0<<8) | TL0;
        freq *= 4;
    }
    TF0 = 0;
    TL0 = TH0 = 0;
    TR0 = 1;
}
void Timer1_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;			//定时器时钟12T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0x18;				//设置定时初始值
	TH1 = 0xFC;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1 = 1;				//使能定时器1中断
}
void Timer2_Init(void)		//100微秒@12.000MHz
{
	AUXR |= 0x04;			//定时器时钟1T模式
	T2L = 0x50;				//设置定时初始值
	T2H = 0xFB;				//设置定时初始值
	AUXR |= 0x10;			//定时器2开始计时
	IE2 |= 0x04;			//使能定时器2中断
}


void Seg_tran(unsigned char*seg_buf,unsigned char*seg_code){
    unsigned char i,j;
    unsigned char temp;
    for(i=j=0;j<8;i++,j++)
    {
        switch(seg_buf[i])
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
            case 'H': temp = 0x89;break;
            case 'P': temp = 0x8c;break;
            case ' ': temp = 0xff;break;
            default: temp = 0xff;break;
        }
        if(seg_buf[i+1] == '.')
        {
            temp &= 0x7f;
            i++;
        }
        seg_code[j] = temp;
    }
}
void Seg_disp(unsigned char* seg_code,unsigned char seg_pos){
    P0 = 0xff;
    P2 = P2 & 0x1f | 0xe0;
    P2 = P2 & 0x1f;

    P0 = 1 << seg_pos;
    P2 = P2 & 0x1f | 0xc0;
    P2 = P2 & 0x1f;

    P0 = seg_code[seg_pos];
    P2 = P2 & 0x1f | 0xe0;
    P2 = P2 & 0x1f;
}
void relay(bit en){
    if(en) P0 = P0 & 0xef | 0x10;
    else P0 = P0 & 0xef; 
    P2 = P2 & 0x1f | 0xa0;
    P2 = P2 & 0x1f;
}
void motor(bit en){
    if(en) P0 = P0 & 0xdf | 0x20;
    else P0 = P0 & 0xdf;
    P2 = P2 & 0x1f | 0xa0;
    P2 = P2 & 0x1f;
}