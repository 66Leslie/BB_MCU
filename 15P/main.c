#include "STC15F2K60S2.H"
#include <stdio.h>

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;

uint8_t ucLed; //led???????
unsigned long ulms;
uint8_t  uiKey_Dly,ucKey_Old;    //???ˮ????????????��???
int  uiFreq;
uint8_t  pucSeg_Buf[9],pucSeg_Code[8];				//???SEG?????????"0.123"?????0xC0 & 0x7f,0xf9,0xa4,0xb0
uint8_t  ucSeg_Pos;   					//0-7 ???8??SEG
uint16_t   uiSeg_Dly;    //????????��ms
uint8_t putRTC[3] = {23,59,50};
uint8_t ucMenu;
bit ucMenu1;
uint8_t ucDac;
int uiPf;
int uiAdj;
int uiFreqMax;
uint8_t  putRTCMax[3];
void System_Init(void);
void Key_Proc(void);
void Seg_Proc(void);

void PCF8591_DAC(unsigned char ucData);  			//DAC???
void Set_RTC(unsigned char* putRTC);				//ds1302, ??????????????putRtc??????
void Read_RTC(unsigned char* putRTC);				//ds1302, ?????????????putRtc??????
//-------????????��?????????????????????????????????????????????????????--------
//unsigned int rd_temp(void); 						//????ds18b20,????????
//unsigned char PCF8591_ADC(unsigned char adc_ch);	//ADC????


//void EEPROM_Write(unsigned char* pucBuf, unsigned char addr, unsigned char num);  	//????????pucBuf??num??????????EEPROM????addr??
//void EEPROM_Read(unsigned char* pucBuf, unsigned char addr, unsigned char num);		//??EEPROM????addr??????num???????????????��????pucBuf??
//unsigned char Wave_Recv(void); 					// ??????????????????��cm
//void Freq_dect(void);								//??????????????��??????uiFreq??
//void Uart_SendString(unsigned char *pucStr);		//????????????

int main(void)
{
	System_Init();	
	Set_RTC(putRTC);
	while(1)
	{
		Key_Proc();  //??��????????
		Seg_Proc(); //????SEG?????????
	}	
}


void led(uint8_t ucLed) // xxxx xxxx
{
	 P0 = ~ucLed; //0-LED OFF,1-ON
	 P2 = P2 & 0x1F | 0x80; //LE = 1 P2 = 100x xxxx
	 P2 = P2 & 0x1F; //LE = 0; P2 = 000x xxxx	 	
}
void Buzz_Relay(uint8_t buzz,uint8_t relay)
{
	P0 = 0x00;
	if(buzz)
		P0 = P0 | 0x40;		//1-> BUZZ ON; 0100 0000
	if(relay)
		P0 = P0 | 0x10;		//1-> relay ON; 0001 0000
	P2 = P2 & 0x1F | 0xA0; //LE = 1 P2 = 101 0 0000
	P2 = P2 & 0x1F;	
}	

void Timer1Init(void)		//1????@12.000MHz
{
	AUXR &= 0xBF;		//????????12T??
	TMOD &= 0x0F;		//??????????
	TL1 = 0x18;		//?????????
	TH1 = 0xFC;		//?????????
	TF1 = 0;		//???TF1???
	TR1 = 1;		//?????1??????
	
	ET1 = 1;		//enable Timer 1
}
void Timer0Init(void)		//Counter
{
	TMOD  = (TMOD&0xF0) | 0x05;		//??????????
	TL0 = 0;		//?????????
	TH0 = 0;		//?????????
	TF0 = 0;		//???TF1???
	TR0 = 0;		//?????1??????
}
void System_Init(void)
{
	//???LED??BUZZ??Relay
	led(0x00);
	Buzz_Relay(0,0);
	Set_RTC(putRTC);
	Timer1Init();
	Timer0Init();
	Set_RTC(putRTC);
	ucMenu = 0;
	uiPf = 2000;
	uiFreqMax = 0;
	uiAdj = 0;
	EA = 1; 
}

//0,4-19 - >s4-s19
uint8_t Key_Read(void)
{
	uint8_t  Key_Value;
    uint16_t  Key_Data; //XXXX XXXX XXXX XXXX
	P44=0;P42=1;P35=1;
	Key_Data = P3 & 0x0F;		//0000 1111
	P44=1;P42=0;P35=1;
	Key_Data = (Key_Data <<4) | (P3 & 0x0F);
	P44=1;P42=1;P35=0; 
	Key_Data = (Key_Data <<4) | (P3 & 0x0F);
	P44=1;P42=1;P35=1; 
	Key_Data = (Key_Data <<4) | (P3 & 0x0F);	
	//1111 1111 1111 1111->0000 0000 0000 0000-??0x0000
	//0111 1111 1111 1111->1000 0000 0000 0000-> 0x8000
	switch(~Key_Data)
	{
		case 0x8000: Key_Value = 4;break;  //1000 0000 0000 0000
		case 0x4000: Key_Value = 5;break;  //0100 0000 0000 0000
		case 0x2000: Key_Value = 6;break;  //0010 0000 0000 0000
		case 0x1000: Key_Value = 7;break;  //0001 0000 0000 0000
		case 0x0800: Key_Value = 8;break; 
		case 0x0400: Key_Value = 9;break; 
		case 0x0200: Key_Value = 10;break; 
		case 0x0100: Key_Value = 11;break; 
		case 0x0080: Key_Value = 12;break; 
		case 0x0040: Key_Value = 13;break; 
		case 0x0020: Key_Value = 14;break; 
		case 0x0010: Key_Value = 15;break; 
		case 0x0008: Key_Value = 16;break; 
		case 0x0004: Key_Value = 17;break; 
		case 0x0002: Key_Value = 18;break; 
		case 0x0001: Key_Value = 19;break; 
		default: Key_Value = 0;
	}
	return Key_Value;
}	


// "T - 28.35"->9???????????'.'  ??21-56-00??->8?????
void Seg_Tran(unsigned char  *pucSeg_Buf,unsigned char  *pucSeg_Code)
{
	 uint8_t  i,j=0,temp;
	 for(i=0;i<8;i++,j++)
	 {	
			switch(pucSeg_Buf[j])
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
			 case 'C': temp = 0xc6;break;
			 case 'D': temp = 0xa1;break;		
			 case 'E': temp = 0x86;break;
			 case 'F': temp = 0x8e;break;	
			 case '-': temp = 0xbf;break;  //1011 1111
			 case ' ': temp = 0xff;break;	
			 case 'H': temp = 0x89;break;	 //H 1000 1001		
			 case 'P': temp = 0x8c;break;
			 case 'L': temp = 0xc7;break;  //L 1100 0111
			 default:temp = 0xff;break;
		 }
		 //"C 28-23.2"
		 if(pucSeg_Buf[j+1] == '.')	
		 {
			  temp &= 0x7F; //0111 1111
			  j++; //0-8
		 }
			pucSeg_Code[i] = temp; //0 -7
	 }
}

//pucSeg_Code: 0xC0--> 0
//ucSeg_Pos : 0- 7
void Seg_Disp(unsigned char  *pucSeg_Code,unsigned char  ucSeg_Pos)
{
	 //???
	 P0 = 0xFF;
	 P2 = P2 & 0x1F | 0xE0; // 1110 0000
	 P2 = P2 & 0x1F;
	
	//?��?COM ucSeg_Pos:0->7
	 P0 = 1<< ucSeg_Pos;    //0000 0001,0000 0010,....1000 0000
   P2 = P2 & 0x1F | 0xC0; // 1100 0000
	 P2 = P2 & 0x1F;
	
	 //??Data
	 P0 = pucSeg_Code[ucSeg_Pos];
	 P2 = P2 & 0x1F | 0xE0; // 1110 0000
	 P2 = P2 & 0x1F;		
	
}

void Key_Proc(void)
{
	 uint8_t ucKey_Down,ucKey_Val;
	 
	 	//????
	 if(uiKey_Dly) return; // 0-19
	 
	 ucKey_Val = Key_Read();	 
	 ucKey_Down = ucKey_Val & (ucKey_Val ^ ucKey_Old);	//negedge 	 
	 ucKey_Old = ucKey_Val;
	
	 if(ucKey_Down == 4) // s4
	 {
		if(++ucMenu == 4) ucMenu = 0;
		if(ucMenu == 1 || ucMenu == 3) ucMenu1  = 0;
	 }
	 if(ucKey_Down == 5 && (ucMenu == 1 || ucMenu == 3)) // s5
	 {
		ucMenu1 = ~ucMenu1;
	 }
	 if(ucKey_Down == 8 && ucMenu == 1)
	 {
		if(ucMenu1 == 0 && uiPf < 10000) uiPf += 1000;
		if(ucMenu1 == 1 && uiAdj  < 900) uiAdj += 100; 
	 }
	 if(ucKey_Down == 9 && ucMenu == 1)
	 {
		if(ucMenu1 == 0 && uiPf > 1000) uiPf -= 1000;
		if(ucMenu1 == 1 && uiAdj > -900) uiAdj -= 100; 
	 }
}

void Seg_Proc(void)
{

	 if(uiSeg_Dly) return;  //?????????? ?500ms
	 Read_RTC(putRTC);
//	 ucDAC = 100;
//	 PCF8591_DAC(ucDAC);//apply dac


	if(uiFreq<=500)
		ucDac = 51;//1V
	else if(uiFreq >= uiPf)
		ucDac = 255;
	else
		ucDac = 51 + (uiFreq-500)*1.0*(255-51)/(uiPf-500);
	PCF8591_DAC(ucDac);//apply dac
	 switch(ucMenu)
	 {
		 case 0:{
			if(uiFreq >= 0) sprintf(pucSeg_Buf,"F  %5d",uiFreq);
			else sprintf(pucSeg_Buf,"F     LL");}break;
		 case 1:if(ucMenu1 ==0)
		 			sprintf(pucSeg_Buf,"P1  %4d",uiPf);
		 		else
					sprintf(pucSeg_Buf,"P2  %4d",uiAdj);
				break;
		 case 2:sprintf(pucSeg_Buf,"%02d-%02d-%02d",(unsigned int)putRTC[0],(unsigned int)putRTC[1],(unsigned int)putRTC[2]);
                break;
		 case 3:if(ucMenu1 ==0)
		 			sprintf(pucSeg_Buf,"HF %05d",uiFreqMax);
				else
		 			sprintf(pucSeg_Buf,"HA%02d%02d%02d",(unsigned int)putRTCMax[0],(unsigned int)putRTCMax[1],(unsigned int)putRTCMax[2]);
				break;
		default:break;
	 }
		 	
	 Seg_Tran(pucSeg_Buf,pucSeg_Code);
}
void F_det(void){
	TR0 = 0;
	if(TF0) 
		uiFreq = 65535;
	else  uiFreq = (TH0<<8) | TL0;
	uiFreq += uiAdj;
	if(uiFreq > uiFreqMax){
		uiFreqMax = uiFreq;
		putRTCMax[0] = putRTC[0];
		putRTCMax[1] = putRTC[1];
		putRTCMax[2] = putRTC[2];
	}
	TH0	= 0;
	TL0 = 0;
	TR0 = 1;
}
//1ms
void T1_ISR(void) interrupt 3
{
	ulms++;
	if(ucMenu == 0)
	{
		if(ulms % 200 == 0)
			ucLed ^= 0x01;
	}else ucLed &= ~0x01;
	if(uiFreq > uiPf)
	{
		if(ulms % 200 == 0) ucLed ^= 0x02;
	}else if (uiFreq < 0) ucLed |= 0x02;
	else ucLed &= ~0x02;
	if(ulms%1000 == 0) F_det();
	 if(++uiKey_Dly == 20) uiKey_Dly = 0;  //0-19
	 if(++uiSeg_Dly == 80) uiSeg_Dly = 0; //0-499
	
	 Seg_Disp(pucSeg_Code,ucSeg_Pos);     //SEG?????�k1ms
	 if(++ucSeg_Pos == 8) ucSeg_Pos = 0; //0-7
	 
	 led(ucLed&0x03);
}
