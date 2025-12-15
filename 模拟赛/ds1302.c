#include <ds1302.h>
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

void Set_RTC()
{

	unsigned char temp;
	Write_Ds1302_Byte(0x8e,0x00);//写操作前，清零第bit7 WP
	
	temp = ((putRTC[0]/10)<<4)+putRTC[0]%10;//hour
	// 23/10 = 2(0'b0010) << 4 = 0001 0000 + 3(0'b0011) = 0001 0011
	Write_Ds1302_Byte(0x84,temp);
	temp = ((putRTC[1]/10)<<4)+putRTC[1]%10;//minutes
	Write_Ds1302_Byte(0x82,temp);
	temp = ((putRTC[2]/10)<<4)+putRTC[2]%10;//seconds
	Write_Ds1302_Byte(0x80,temp);
	
	Write_Ds1302_Byte(0x8e, 0x80);//写保护bit7 = 1
}

void Read_RTC(){

	unsigned char temp;
	temp = Read_Ds1302_Byte(0x85);						// 读取时
	putRTC[0] = (temp>>4)*10+(temp&0x0f);//&0xf高四位清零
	temp = Read_Ds1302_Byte(0x83);						// 读取分
	putRTC[1] = (temp>>4)*10+(temp&0x0f);
	temp = Read_Ds1302_Byte(0x81);						// 读取秒
	putRTC[2] = (temp>>4)*10+(temp&0x0f);
	
}