#ifndef RTC_SIMPLE_H
#define RTC_SIMPLE_H

typedef struct TClock {
	unsigned char sec;	
	unsigned char min;
	unsigned char hr;
	unsigned char wday;	
} TClock;

void rtcInc(TClock *rtc);
void rtcDec(TClock *rtc);

#endif
