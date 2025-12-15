#include <base.h>

void HC138(unsigned char n){
	P25 = n % 2;
	n /= 2;
	P26 = n % 2;
	n /= 2;
	P27 = n % 2;
}
void Buzz_Relay(unsigned char a,unsigned char b){
	P06 = a;
	P04 =~b;
}
void LED(unsigned char s){
	HC138(4);
	P0 &= 0xff;
	switch(s){
		case 1: P0 = 0xfe;break;
		case 2: P0 = 0xfd;break;
		case 3: P0 = 0xfb;break;
		case 8: P0 = 0x7f;break;
	}
	HC138(0);
}
void InitPerSys(){
		HC138(5);
		Buzz_Relay(0,0);
		HC138(0);
}