#ifndef __IT_H__
#define __IT_H__


extern unsigned char Seg_pos;
extern unsigned char Seg_Buff[8];

void Timer0Init(void);
void Timer0Server();

#endif