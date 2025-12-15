#include <Seg.h>
//段选是0点亮，1熄灭
code unsigned char seg_dula[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0xff, 0x88};
code unsigned char seg_wela[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

unsigned char i;
unsigned char flag;

void Seg_Disp(unsigned char wela, dula,point)
{
	P0 = 0xff;
	P2 = P2 & 0x1f | 0xe0;
	P2 &= 0x1f;
	for(i = 0;i<10;i++)
	{
		if(dula == i ) flag = 1;
		else if(dula == (i+'0')) {flag = 1;dula = i;}
		else;
	}
	if(flag)
	{
		P0 = seg_wela[wela];
		P2 = P2 & 0x1f | 0xc0;
		P2 &= 0x1f;
		P0 = seg_dula[dula];
		if (!point)
		P0 &= 0x7f;
		P2 = P2 & 0x1f | 0xe0;
		P2 &= 0x1f;
	}
	else//非数字的数码管显示
	{
		P0 = seg_wela[wela];
		P2 = P2 & 0x1f | 0xc0;
		P2 &= 0x1f;
		switch(dula){
			 case 'A': P0  = 0x88;break;
			 case 'B': P0  = 0x83;break;	
			 case 'C': P0  = 0xc6;break;
			 case 'D': P0  = 0xa1;break;		
			 case 'E': P0  = 0x86;break;
			 case 'F': P0  = 0x8e;break;	
			 case 'P': P0  = 0x8c;break;
			 case '-': P0  = 0xBF;break;
			 case ' ': P0  = 0xFF;break;	
			 case 'H': P0  = 0x89;break;
			 case 'U': P0  = 0xc1;break;
			 default:  P0  = 0xFF;break;
		}
		if (!point)
		P0 &= 0x7f;
		P2 = P2 & 0x1f | 0xe0;
		P2 &= 0x1f;
	}
	flag = 0;
}