#include <STC15F2K60S2.H>
extern unsigned char uwTick;
unsigned char cont;
unsigned char key_val;
unsigned char key_down;
unsigned char key_old;
unsigned char key_scan()
{
	//????????
//	if(!P30) return 1;
//	if(!P31) return 2;
//	if(!P32) return 3;
//	if(!P33) return 4; 
	//????? S5 S4 S8 ??? 9 13 14
	P30 = 1,P31 = 1,P32 = 0,P33 = 1;
	if(!P44) return 5;
	P30 = 1,P31 = 1,P32 = 1,P33 = 0;
	if(!P44) return 4;
	if(!P42) return 8;
	return 0;
}
unsigned char key_judge()
{
	if(uwTick - cont < 10) return 0;
	cont = uwTick; 	 
	key_val  = key_scan();
	key_down = key_val & (key_val ^ key_old);
	key_old  = key_val;
	return key_down;
}
/*???????
	P30 = 0,P31 = 1,P32 = 1,P33 = 1;
	if(!P44) return 1;
	if(!P42) return 2;
	if(!P35) return 3;
	if(!P34) return 4;
	P30 = 1,P31 = 0,P32 = 1,P33 = 1;
	if(!P44) return 5;
	if(!P42) return 6;
	if(!P35) return 7;
	if(!P34) return 8;
	P30 = 1,P31 = 1,P32 = 0,P33 = 1;
	if(!P44) return 9;
	if(!P42) return 10;
	if(!P35) return 11;
	if(!P34) return 12;
	P30 = 1,P31 = 1,P32 = 1,P33 = 0;
	if(!P44) return 13;
	if(!P42) return 14;
	if(!P35) return 15;
	if(!P34) return 16;
*/