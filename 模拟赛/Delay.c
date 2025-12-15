void Delay1ms()		//@12.000MHz
{
	unsigned char i, j;

	i = 2;
	j = 239;
	do
	{
		while (--j);
	} while (--i);
}
void Delay_ms(unsigned int i){
	while(i--){
	Delay1ms();
	}
}

