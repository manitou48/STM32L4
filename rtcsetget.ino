// dragonfly test version pre RTC software

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define RTC_TR_RESERVED_MASK    ((uint32_t)0x007F7F7F)
#define RTC_INIT_MASK           ((uint32_t)0xFFFFFFFF)  

#define BYTE2BCD(byte)      ((byte % 10) | ((byte / 10) << 4))
#define BCD2BYTE(byte)      ((byte & 0x0f) + 10*((byte >> 4) & 0x0f))

struct tm
{
  int tm_sec;      /* Seconds. [0-60] (1 leap second) */
  int tm_min;     /* Minutes. [0-59] */
  int tm_hour;      /* Hours. [0-23] */
  int tm_mday;      /* Day.   [1-31] */
  int tm_mon;     /* Month. [0-11] */
  int tm_year;      /* Year - 1900.  */
  int tm_wday;      /* Day of week. [0-6] */
  int tm_yday;      /* Days in year.[0-365] */
  int tm_isdst;     /* DST.   [-1/0/1]*/
};
static struct tm tm;

#define SECS_PER_MIN  ((time_t)(60UL))
#define SECS_PER_HOUR ((time_t)(3600UL))
#define SECS_PER_DAY  ((time_t)(SECS_PER_HOUR * 24UL))
#define DAYS_PER_WEEK ((time_t)(7UL))
#define SECS_PER_WEEK ((time_t)(SECS_PER_DAY * DAYS_PER_WEEK))
#define SECS_PER_YEAR ((time_t)(SECS_PER_WEEK * 52UL))
#define SECS_YR_2000  ((time_t)(946684800UL)) // the time at the start of y2k


// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0
 
void breakTime(time_t timeInput){
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;

  time = (uint32_t)timeInput;
  tm.tm_sec = time % 60;
  time /= 60; // now it is minutes
  tm.tm_min = time % 60;
  time /= 60; // now it is hours
  tm.tm_hour = time % 24;
  time /= 24; // now it is days
  tm.tm_wday = ((time + 4) % 7) ;  // thursday is day 4
  
  year = 0;  
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm.tm_year = year; // year is offset from 1970 
  
  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0
  
  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }
    
    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  tm.tm_mon = month + 1;  // jan is month 1  
  tm.tm_mday = time + 1;     // day of month
}

time_t makeTime(){   
// assemble time elements into time_t 
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  
  int i;
  uint32_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= tm.tm_year*(SECS_PER_DAY * 365);
  for (i = 0; i < tm.tm_year; i++) {
    if (LEAP_YEAR(i)) {
      seconds +=  SECS_PER_DAY;   // add extra days for leap years
    }
  }
  
  // add days for this year, months start from 1
  for (i = 1; i < tm.tm_mon; i++) {
    if ( (i == 2) && LEAP_YEAR(tm.tm_year)) { 
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (tm.tm_mday-1) * SECS_PER_DAY;
  seconds+= tm.tm_hour * SECS_PER_HOUR;
  seconds+= tm.tm_min * SECS_PER_MIN;
  seconds+= tm.tm_sec;
  return (time_t)seconds; 
}

// Unixtimeseconds from 1. Januar 1970  00:00:00
// to 1. Januar 2000   00:00:00 UTC-0
#define SECONDS_FROM_1970_TO_2000 946684800

int conv2d(char* p){
  int v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

uint32_t timefromcompiler(char* date, char* time){
  // looks like 18:49:00 Oct  6 2016
  int _days, _month, _year, _hour, _minute, _second;
  uint32_t _ticks;
  
  //Day
  _days = conv2d(date + 4);
  
  //Month
  switch (date[0]) {
    case 'J': _month = date[1] == 'a' ? 1 : _month = date[2] == 'n' ? 6 : 7; break;
    case 'F': _month = 2; break;
    case 'A': _month = date[2] == 'r' ? 4 : 8; break;
    case 'M': _month = date[2] == 'r' ? 3 : 5; break;
    case 'S': _month = 9; break;
    case 'O': _month = 10; break;
    case 'N': _month = 11; break;
    case 'D': _month = 12; break;
  }
  
  //Year
  _year = conv2d(date + 9);
  
  //Time
  _hour = conv2d(time);
  _minute = conv2d(time + 3);
  _second = conv2d(time + 6);
  
  //Date & Time to Unixtime
  for (int i = 1; i < _month; ++i)
    _days += monthDays[i - 1];
  if (_month > 2 && _year % 4 == 0)
    ++_days;
  _days += 365 * _year + (_year + 3) / 4 - 1;

  _ticks = ((_days * 24 + _hour) * 60 + _minute) * 60 + _second;
  _ticks += SECONDS_FROM_1970_TO_2000;
  
  return(_ticks);
}

uint32_t rtc_secs() {
		uint32_t  hms, ymd;

		hms = (uint32_t)(RTC->TR & RTC_TR_RESERVED_MASK);
    ymd =RTC->DR;   // must read DR after TR 
		tm.tm_hour = BCD2BYTE((hms & (RTC_TR_HT | RTC_TR_HU)) >> 16);
		tm.tm_min = BCD2BYTE((hms & (RTC_TR_MNT | RTC_TR_MNU)) >>8);
		tm.tm_sec = BCD2BYTE((hms & (RTC_TR_ST | RTC_TR_SU)));
		hms = 3600*tm.tm_hour + 60*tm.tm_min + tm.tm_sec;
    tm.tm_year = BCD2BYTE((ymd & (RTC_DR_YT | RTC_DR_YU)) >> 16);
    tm.tm_mon = BCD2BYTE((ymd & (RTC_DR_MT | RTC_DR_MU)) >> 8);
    tm.tm_mday = BCD2BYTE((ymd & (RTC_DR_DT | RTC_DR_DU))) ;
		return hms;
}

void rtc_set(time_t s) {
	// from pyboard core
	/* Disable the write protection for RTC registers */
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	// set init mode
	RTC->ISR = (uint32_t)RTC_INIT_MASK;
	while((RTC->ISR &  RTC_ISR_INITF) == 0);  // ? timeout

#if 0
  // manual
	RTC->DR = 0x162514;     // BCD calendar
  RTC->TR = 0;
#endif
//  tm.tm_year=16;tm.tm_mon=10;tm.tm_mday=6;  tm.tm_hour=13; tm.tm_min=14; tm.tm_sec=15; tm.tm_wday=1;
  breakTime(s);     //  fill tm
  if (tm.tm_wday == 0) tm.tm_wday=7;  // for stm32l4 rtc 
  tm.tm_year -= 30;  
  RTC->DR = BYTE2BCD(tm.tm_year)<<16 | tm.tm_wday<<13 | BYTE2BCD(tm.tm_mon)<<8 | BYTE2BCD(tm.tm_mday);
  RTC->TR = BYTE2BCD(tm.tm_hour)<<16 | BYTE2BCD(tm.tm_min)<<8 | BYTE2BCD(tm.tm_sec);

	// exit init mode
	RTC->ISR &= (uint32_t)~RTC_ISR_INIT;
	//  enable write protection
	RTC->WPR = 0xff;
  delay(1);
}


void setup() {
  time_t s;
  
  Serial.begin(9600);
  while(!Serial);

//  s = 1475785998;     //   date +%s   date --date='@1475785998'
  s = timefromcompiler(__DATE__,__TIME__);
	rtc_set(s);
}

void loop() {
  uint32_t esecs;
  char str[128];

		esecs = rtc_secs();
    sprintf(str,"%02d:%02d:%02d %d/%d/%d %d",tm.tm_hour,tm.tm_min,tm.tm_sec,tm.tm_mon,tm.tm_mday,tm.tm_year, esecs);
    Serial.println(str);

    delay(5000); 
}
