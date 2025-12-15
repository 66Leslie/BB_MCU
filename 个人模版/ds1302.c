/* instruments:read the figure3 and table3 in ds1302.pdf*/		
#include "STC15F2K60S2.H"
#include <intrins.h>

 sbit SCK = P1^7;
 sbit SDA = P2^3;
 sbit RST = P1^3;
//
void Write_Ds1302(unsigned  char temp) 
{
	unsigned char i;
	for (i=0;i<8;i++)     	
	{ 
		SCK = 0;
		SDA = temp&0x01;
		temp>>=1; 
		SCK=1;
	}
}   

//
void Write_Ds1302_Byte( unsigned char address,unsigned char dat )     
{
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1; 	_nop_();  
 	Write_Ds1302(address);	
 	Write_Ds1302(dat);		
 	RST=0; 
}

//
unsigned char Read_Ds1302_Byte ( unsigned char address )
{
 	unsigned char i,temp=0x00;
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1;	_nop_();
 	Write_Ds1302(address);
 	for (i=0;i<8;i++) 	
 	{		
		SCK=0;
		temp>>=1;	
 		if(SDA)
 		temp|=0x80;	
 		SCK=1;
	} 
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
	SCK=1;	_nop_();
	SDA=0;	_nop_();
	SDA=1;	_nop_();
	return (temp);			
}
void Set_RTC(unsigned char* putRTC)
{
	unsigned char temp;
	Write_Ds1302_Byte(0x8e,0x00);
	
	temp = ((putRTC[0]/10)<<4) + putRTC[0]%10;
	Write_Ds1302_Byte(0x84,temp);//hour
	
	temp = ((putRTC[1]/10)<<4) + putRTC[1]%10;
	Write_Ds1302_Byte(0x82,temp);//min
	
	temp = ((putRTC[2]/10)<<4) + putRTC[2]%10;
	Write_Ds1302_Byte(0x80,temp);
	
	Write_Ds1302_Byte(0x8e,0x80);
}

void Read_RTC(unsigned char* putRTC){

	unsigned char temp;
	temp = Read_Ds1302_Byte(0x85);						// 读取时
	putRTC[0] = (temp>>4)*10+(temp&0x0f);//&0xf高四位清零

	temp = Read_Ds1302_Byte(0x83);						// 读取分
	putRTC[1] = (temp>>4)*10+(temp&0x0f);

	temp = Read_Ds1302_Byte(0x81);						// 读取秒
	putRTC[2] = (temp>>4)*10+(temp&0x0f);
}
