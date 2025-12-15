/*主要看table3
温度传感器，每次只能做一件事。跳过内存检查不算，然后执行任务。下一次任务与本次任务
之间需要一次Init。温度转换完成之后是先放在scratchpad暂存区。*/
#include "STC15F2K60S2.H"
sbit DQ = P1 ^ 4;
//
void Delay_OneWire(unsigned int t)  
{
	unsigned char i;
	while(t--){
		for(i=0;i<12;i++);
	}
}

//
void Write_DS18B20(unsigned char dat)
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		DQ = 0;
		DQ = dat&0x01;
		Delay_OneWire(5);
		DQ = 1;
		dat >>= 1;
	}
	Delay_OneWire(5);
}

//
unsigned char Read_DS18B20(void)
{
	unsigned char i;
	unsigned char dat;
  
	for(i=0;i<8;i++)
	{
		DQ = 0;
		dat >>= 1;
		DQ = 1;
		if(DQ)
		{
			dat |= 0x80;
		}	    
		Delay_OneWire(5);
	}
	return dat;
}

//
bit init_ds18b20(void)
{
  	bit initflag = 0;
  	
  	DQ = 1;
  	Delay_OneWire(12);
  	DQ = 0;
  	Delay_OneWire(80);
  	DQ = 1;
  	Delay_OneWire(10); 
    initflag = DQ;     
  	Delay_OneWire(5);
  
  	return initflag;
}
float ReadTemperature(void){
	unsigned char low,high;
	//start t conversion
	init_ds18b20();
	Write_DS18B20(0xcc); // skip rom
	Write_DS18B20(0x44); // convert temperature
	Delay_OneWire(200);
	//start t read
	init_ds18b20();
	Write_DS18B20(0xcc); // skip rom
	Write_DS18B20(0xbe); 
	low = Read_DS18B20(); // LSB first
	high = Read_DS18B20(); // MSB second
	return (float)(high << 8 | low)*0.0625; // 12 bit resolution
}