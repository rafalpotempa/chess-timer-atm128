#include <ioavr.h>
#include <inavr.h>
#include <stdio.h>
#include <pgmspace.h>
#include "rtc_simple.h"
#include "hd44780.h"
#include "queue.h"
#include "keyb_drv.h"

#define CLOCK_DEC	0x03

#define byte unsigned char

typedef union TSysRq {
    byte msg;
    struct {
		byte rq_data	: 4;
		byte rq_id 	: 4;
    };
} TSysRq;

//---------------------------------------------------------
// Constants declaration
//---------------------------------------------------------

static const char __flash LCDUserChar[] = {
    0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F,
    0x1F, 0x00, 0x10, 0x10, 0x10, 0x10, 0x00, 0x1F,
    0x1F, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x1F,
    0x1F, 0x00, 0x1C, 0x1C, 0x1C, 0x1C, 0x00, 0x1F,
    0x1F, 0x00, 0x1E, 0x1E, 0x1E, 0x1E, 0x00, 0x1F,
    0x1F, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x1F,
    0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03,
    0x18, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x18 };
 
static const char __flash clockMask[] = "%1d:%02d:%02d";

//---------------------------------------------------------
// Global variables declaration
//---------------------------------------------------------
TClock playerOne;
TClock playerTwo;

byte player = 0;

TQueue event_queue;
byte ev_q_buf[16];
char tmp_buf[4];
TSysRq rq;

// Clock values
 

//This function is used by printf function to transfer data to STDIO device
int putchar(int ch)
{
	LCDPutChar(ch);
	return ch;
}

void onKeyDown()
{
  if (player == 0){
        LCDGoTo(LINE_0 + 12);
        LCDPutChar(' ');
    	LCDGoTo(LINE_1 + 3);
	LCDPutChar('o');
        player = 1;
  } else if (player == 1) {
        LCDGoTo(LINE_0 + 12);
        LCDPutChar('o');
    	LCDGoTo(LINE_1 + 3);
	LCDPutChar(' ');
        player = 0;
  }
}

void onKeyUp()
{

}

void onClockDec()
{
	LCDGoTo(LINE_0);
	printf_P(
		clockMask,
		playerOne.hr,
		playerOne.min,
		playerOne.sec);
        
	LCDGoTo(LINE_1+9);
	printf_P(
		clockMask,
		playerTwo.hr,
		playerTwo.min,
		playerTwo.sec);
}

static void InitDevices()
{
    LCDInit();
    LCDLoadUserCharP(LCDUserChar, 0, sizeof(LCDUserChar));
    LCDClear();
    //T0 start
    TCNT0 = 0;
//    OCR0 = 71; // this is real time
//    OCR2 = 71; // this is real time
    OCR0 = 1;    // this is sped up
    OCR2 = 1;    // this is sped up
    TCCR0 = (1 << WGM01) | (1 << CS02) | (1 << CS01) | (1 << CS00);
    //dvt = 50;
    //Timers interrupt mask
    TIMSK = (1 << OCIE0) | (1 << OCIE2);
    __enable_interrupt();	
}

#pragma vector = TIMER0_COMP_vect
__interrupt void ISR_OCR0()
{
static byte pre_dv = 100;
	kbService(&event_queue);
	if(--pre_dv) return;
	pre_dv = 100;
	rtcDec(&playerOne);	
	qAdd(&event_queue, MSG(CLOCK_DEC, 0));
}

#pragma vector = TIMER2_COMP_vect
__interrupt void ISR_OCR2()
{
static byte pre_dv = 100;
	kbService(&event_queue);
	if(--pre_dv) return;
	pre_dv = 100;
	rtcDec(&playerTwo);	
	qAdd(&event_queue, MSG(CLOCK_DEC, 0));
}

void main()
{	
        playerOne.hr = 1;
        playerTwo.hr = 1;
  
	InitDevices();
	qInit(&event_queue, ev_q_buf, sizeof(ev_q_buf));
	qAdd(&event_queue, MSG(CLOCK_DEC, 0));
        
        onKeyDown();
        
	while(1)
	{
		while(!qGet(&event_queue, &rq.msg));
		switch(rq.rq_id) {
		case KEY_DOWN:
			onKeyDown();
                        if (player == 0){
                                TCCR0 = (1 << WGM01) | (0 << CS02) | (0 << CS01) | (0 << CS00);
                                TCCR2 = (1 << WGM21) | (1 << CS22) | (0 << CS21) | (1 << CS20);
                        }
                        else if (player == 1){
                                TCCR0 = (1 << WGM01) | (1 << CS02) | (1 << CS01) | (1 << CS00);
                                TCCR2 = (1 << WGM21) | (0 << CS22) | (0 << CS21) | (0 << CS20);
                        }
			break;
		case KEY_UP:
			onKeyUp();
			break;
		case CLOCK_DEC:
			onClockDec();
			break;
		}
	}
}

