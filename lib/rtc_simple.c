#include "rtc_simple.h"

void rtcInc(TClock *rtc)
{
	if((++rtc->sec) != 60) return;
	rtc->sec = 0;
	if((++rtc->min) != 60) return;
	rtc->min = 0;
	if((++rtc->hr) != 24) return;
	rtc->hr = 0;
}

void rtcDec(TClock *rtc)
{
	if((--rtc->sec) != 255) return;
	rtc->sec = 59;
	if((--rtc->min) != 255) return;
	rtc->min = 59;
	if((--rtc->hr) != 255) return;
	rtc->hr = 9;
}

