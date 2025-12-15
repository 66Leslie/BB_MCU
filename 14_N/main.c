#include "STC15F2K60S2.H"
#include "stdio.h"
typedef unsigned char u8;

#define slow_seg 99
#define slow_key 15
#define slow_wave 250
#define slow_cel 450
#define slow_dac 50
/*----------variabels----------*/
bit menu0,menu1;

u8 seg_code[8];
u8 seg_pos;
u8 seg_buf[15];
u8 seg_dly,key_dly,wave_dly,dac_dly;
u8 ucMenu;
u8 menu2;
u8 dist;
u8 dist_ref;cel_ref;
u8 save[6];
char tune;
unsigned int cel_dly,v_mat;
unsigned long ulms;
float dac_low;
float cel;

/*----------functions*/
void Timer2_Init();

void Seg_disp(unsigned char *seg_code,unsigned char seg_pos);
void Seg_tran(unsigned char *seg_buf,unsigned char *seg_code);
unsigned char Key_read(void);
unsigned char wave_receive();
float rd_temperature();
void dac_out(unsigned char val);


void sys_init(){
    while(rd_temperature()==85);
    Timer2_Init();
    seg_pos=seg_dly=key_dly=ulms = 0;
    cel = 0;
    tune = 0;
    cel_ref = 30;
    dist_ref = 40;
    dac_low = 1.0;
    v_mat = 340;
    EA = 1;
}
void wave(){
    unsigned char temp;
    if(wave_dly < slow_wave) return;
    else wave_dly = 0;
    temp = wave_receive();
    temp = temp*v_mat/340.0 + tune;
    if(temp > 90) dist = 90;
    else if(temp < 10) dist = 10;
    else dist = temp;
}
void celsius(){
    float temp;
    if(cel_dly<slow_cel) return;
    else cel_dly = 0;
    temp = rd_temperature();
    if(temp>80) cel = 80;
    else if(temp<0) cel = 0;
    else cel = temp;
}
void dac(){
    unsigned char temp;
    if(dac_dly<slow_dac)return;
    else dac_dly = 0;
    if(dist>90) temp = 5;
    else if(dist<10)temp = dac_low;
    else temp = dac_low+(dist-10)*1.0*(5-dac_low)/80.0;

    dac_out(temp*51);
}
void Seg_proc(){
    if(seg_dly < slow_seg)return;
    else seg_dly = 0;

    switch(ucMenu){
        case 0:
            if(!menu0)sprintf(seg_buf,"%4.1f-%4d",cel,(unsigned int)dist);//cm
            else sprintf(seg_buf,"%4.1f-%5.2f",cel,dist/100.0);break;//m
        case 1:
            if(!menu1)sprintf(seg_buf,"P1    %2d",(unsigned int)dist_ref);
            else sprintf(seg_buf,"P2    %2d",(unsigned int)cel_ref);
        case 2:
            if(menu2 == 0) sprintf(seg_buf,"F1   %3d",(int)tune);
            if(menu2 == 1) sprintf(seg_buf,"F2  %4d",v_mat);
            if(menu2 == 2) sprintf(seg_buf,"F3    %3.1f",dac_low);
        default:break;
    }
    Seg_tran(seg_buf,seg_code);
}
void Key_proc(){
    u8 key_val,key_down;
    static unsigned char key_old; 
    static unsigned long dist_record;  

    if(key_dly < slow_key)return;
    else key_dly = 0;

    if(ulms - dist_record < 6000)return;

    key_val = Key_read();
    key_down = key_val & (key_val ^ key_old);
    key_old = key_val;
    switch(key_down){
        case 4:
            if(++ucMenu==3)ucMenu=0;
            if(ucMenu==0)menu0 = 0;
            if(ucMenu==1)menu1 = 0;
            if(ucMenu==2)menu2 = 0;
            break;
        case 5:
            if(ucMenu==0)menu0 ^= 1;
            if(ucMenu==1)menu1 ^= 1;
            if(ucMenu==2)menu2 += menu2>1?-2:1;
            break;
        case 8:
            if(ucMenu==0){
                dist_record = ulms;
            }//to be defined
            if(ucMenu==1){
                if(menu1)cel_ref+=10;//1
                else dist_ref+=10;//0
            } 
            if(ucMenu==2){
                if(menu2==0)tune+=5;
                if(menu2==1)v_mat+=10;
                if(menu2==2)dac_low+=0.1;
            }
            break;
        case 9:
            if(ucMenu==0){

            }//to be defined
            if(ucMenu==1){
                if(menu1)cel_ref-=10;//1
                else dist_ref-=10;//0
            } 
            if(ucMenu==2){
                if(menu2==0)tune-=5;
                if(menu2==1)v_mat-=10;
                if(menu2==2)dac_low-=0.1;
            }break;
        default:break;
    }

    if(dist_ref>90)dist_ref=90;
    if(dist_ref<10)dist_ref=10;
    if(tune>90)tune=90;
    if(tune<-90)tune=-90;
    if(v_mat>9990)v_mat=9990;
    if(v_mat<10)v_mat=10;
    if(dac_low>2)dac_low=2;
    if(dac_low<0.1)dac_low=0.1;

}
void main(){
    sys_init();
    while(1){
        wave();
        dac();
        celsius();
        Seg_proc();
        Key_proc();
    }
}
void Timer2_Isr(void) interrupt 12
{
    ulms++;
    if(seg_dly<slow_seg) seg_dly++;
    if(key_dly<slow_key) key_dly++;      
    if(wave_dly<slow_wave)wave_dly++;
    if(cel_dly<slow_cel)cel_dly++;
    if(dac_dly<slow_dac)dac_dly++;
    Seg_disp(seg_code,seg_pos);
    if(++seg_pos==8) seg_pos = 0;
}

void Timer2_Init(void)		//1毫秒@12.000MHz
{
	AUXR |= 0x04;			//定时器时钟1T模式
	T2L = 0x20;				//设置定时初始值
	T2H = 0xD1;				//设置定时初始值
	AUXR |= 0x10;			//定时器2开始计时
	IE2 |= 0x04;			//使能定时器2中断
}
unsigned char Key_read(void){
    u8 temp;
    temp = 0;
    IE2 &= (~0X04);
    P44 = 0;P42 = 1;P35 = 1;P34 = 1;
    if(!P33) temp = 4;
    if(!P32) temp = 5;
    if(!P31) temp = 6;
    if(!P30) temp = 7;
    P44 = 1;P42 = 0;P35 = 1;P34 = 1;
    if(!P33) temp = 8;
    if(!P32) temp = 9;
    if(!P31) temp = 10;
    if(!P30) temp = 11;
    P44 = 1;P42 = 1;P35 = 0;P34 = 1;
    if(!P33) temp = 12;
    if(!P32) temp = 13;
    if(!P31) temp = 14;
    if(!P30) temp = 15;
    P44 = 1;P42 = 1;P35 = 1;P34 = 0;
    if(!P33) temp = 16;
    if(!P32) temp = 17;
    if(!P31) temp = 18;
    if(!P30) temp = 19;
    P3 = 0xff;
    IE2 |= 0X04;
    return temp;
}
void Seg_tran(unsigned char *seg_buf,unsigned char *seg_code){
    u8 i,j;
    u8 temp;
    for(i=j=0;j<8;i++,j++){
        switch(seg_buf[i]){
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
            case '-': temp = 0xbf;break;
            case ' ': temp = 0xff;break;
            default: temp = 0xff;break;
        }
        if(seg_buf[i+1] == '.'){
            temp &= 0x7f;
            i++;
        }
        seg_code[j] = temp;
    }
}
void Seg_disp(unsigned char *seg_code,unsigned char seg_pos){
    P0 = 0xff;
    P2 = P2 & 0x1f | 0xe0;
    P2 = P2 & 0x1f;

    P0 = 1 << seg_pos; 
    P2 = P2 & 0x1f | 0xc0;//1100
    P2 = P2 & 0x1f;

    P0 = seg_code[seg_pos];
    P2 = P2 & 0x1f | 0xe0;
    P2 = P2 & 0x1f;
}