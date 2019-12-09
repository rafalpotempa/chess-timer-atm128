#include <ioavr.h>
#include <inavr.h>
#include <stdio.h>
#include <pgmspace.h>
#include "rtc_simple.h"
#include "hd44780.h"
#include "queue.h"
#include "keyb_drv.h"

#define CLOCK_INC	0x03

typedef union TSysRq {
    unsigned char msg;
    struct {
		unsigned char rq_data	: 4;
		unsigned char rq_id 	: 4;
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

static const char __flash sSun[] = "Sun";
static char const __flash sMon[] = "Mon";
static char const __flash sTue[] = "Tue";
static char const __flash sWed[] = "Wed";
static char const __flash sThu[] = "Thu";
static char const __flash sFri[] = "Fri";
static char const __flash sSat[] = "Sat";
static const char __flash sCl[] = "%2d:%02d:%02d %s";

__flash const char __flash* sWDay[] = {
	sSun, sMon, sTue, sWed, sThu, sFri, sSat
 };

//---------------------------------------------------------
// Global variables declaration
//---------------------------------------------------------
TClock rtc;
TQueue event_queue;
unsigned char ev_q_buf[16];
char tmp_buf[4];
TSysRq rq;

//This function is used by printf function to transfer data to STDIO device
int putchar(int ch)
{
	LCDPutChar(ch);
	return ch;
}

void onKeyDown()
{
	LCDGoTo(LINE_1 + 4 + rq.rq_data);
	LCDPutChar('o');
}

void onKeyUp()
{
	LCDGoTo(LINE_1 + 4 + rq.rq_data);
	LCDPutChar(' ');
}

void onClockInc()
{
	LCDGoTo(LINE_0 + 2);
	printf_P(
		sCl,
		rtc.hr,
		rtc.min,
		rtc.sec,
		memcpy_P(tmp_buf, sWDay[rtc.wday], sizeof(sSun)));
}

static void InitDevices()
{
    LCDInit();
    LCDLoadUserCharP(LCDUserChar, 0, sizeof(LCDUserChar));
    LCDClear();
    //T0 start
    TCNT0 = 0;
    OCR0 = 71;

    TCCR0 = (1 << WGM01) | (1 << CS02) | (1 << CS01) | (1 << CS00);
    //dvt = 50;
    //Timers interrupt mask
    TIMSK = (1 << OCIE0);
    __enable_interrupt();	
}

#pragma vector = TIMER0_COMP_vect
__interrupt void ISR_OCR0()
{
static unsigned char pre_dv = 100;
	kbService(&event_queue);
	if(--pre_dv) return;
	pre_dv = 100;
	rtcInc(&rtc);	
	qAdd(&event_queue, MSG(CLOCK_INC, 0));
}

void main()
{	
	InitDevices();
	qInit(&event_queue, ev_q_buf, sizeof(ev_q_buf));
	qAdd(&event_queue, MSG(CLOCK_INC, 0));
	for(;;)
	{
		while(!qGet(&event_queue, &rq.msg));
		switch(rq.rq_id) {
		case KEY_DOWN:
			onKeyDown();
			break;
		case KEY_UP:
			onKeyUp();
			break;
		case CLOCK_INC:
			onClockInc();
			break;
		}
	}
}

