#include "STC15F2K60S2.H"
#include "intrins.h"
sbit Tx = P1^0;
sbit Rx = P1^1;
void Delay12us(void);
void ultrasound_init(){
    unsigned char i;
    for(i = 0;i < 8;i++){
        Tx = 1;
        Delay12us();
        Tx = 0;
        Delay12us();
    }
}
unsigned char ultrasound_data(){
    CMOD = 0x00;//1Mhz
	ultrasound_init();
    CF = 0;
    CH = 0;
    CL = 0;
    CR = 1;
    while((Rx == 1)&&(CF ==0));
    CR = 0;
    if(CF){CF = 0;return 0;}
    else{
        return (CH<<8|CL)*0.017;
    }
}
void Delay12us(void)	//@12.000MHz
{
	unsigned char data i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}
