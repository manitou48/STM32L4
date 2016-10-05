// dragonfly test version pre RTC software ALRMA ISR  EXTI line 18
//  can do every minute, or hour, or day, could add in ISR do every 10s
//  can alarm at a fixed time in the future

//  assume RTC is running and set

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define RTC_TR_RESERVED_MASK    ((uint32_t)0x007F7F7F)

volatile uint32_t ticks;

extern "C" void RTC_Alarm_IRQHandler(void) {
	// both A and B
	if (RTC->ISR & RTC_ISR_ALRAF) {
		ticks++;
		RTC->ISR &= (uint32_t)~RTC_ISR_ALRAF;   // clear wakeup
		EXTI->PR1 |= EXTI_PR1_PIF18;
	}
}


uint8_t RTC_Bcd2ToByte(uint8_t Value)
{
  uint32_t tmp = 0;
  tmp = ((uint8_t)(Value & (uint8_t)0xF0) >> (uint8_t)0x4) * 10;
  return (tmp + (Value & (uint8_t)0x0F));
}


uint32_t rtc_ms() {
		uint32_t ms, tmpreg;
		uint8_t hrs,mins,secs,ss;

    ss = RTC->SSR;  // subseconds /256 counting down, rollover issue
		tmpreg = (uint32_t)(RTC->TR & RTC_TR_RESERVED_MASK);
    RTC->DR;   // must read DR after TR 
		hrs = (uint8_t)RTC_Bcd2ToByte((tmpreg & (RTC_TR_HT | RTC_TR_HU)) >> 16);
		mins = (uint8_t)RTC_Bcd2ToByte((tmpreg & (RTC_TR_MNT | RTC_TR_MNU)) >>8);
		secs = (uint8_t)RTC_Bcd2ToByte(tmpreg & (RTC_TR_ST | RTC_TR_SU));
		tmpreg = 3600*hrs + 60*mins + secs;
        ms = 1000*(tmpreg + (255-ss)/256.);
		return ms;
}

void rtc_init() {
	/* Disable the write protection for RTC registers */
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

  __disable_irq();         // no interrupts
  //  enable wakeup
  EXTI->PR1 |= EXTI_PR1_PIF18;    // clear
  EXTI->IMR1 |= EXTI_IMR1_IM18;      // line 18 RTC ALARM EXTI
  EXTI->RTSR1 |= EXTI_RTSR1_RT18;   // rising

  RTC->CR &= ~(RTC_CR_ALRAIE |RTC_CR_ALRAE);   // clear 
  while((RTC->ISR & RTC_ISR_ALRAWF)  == 0);   // wait
   // configure A ALARM 7th second every minute
  RTC->ALRMAR = RTC_ALRMAR_MSK4  | RTC_ALRMAR_MSK3 | RTC_ALRMAR_MSK2 | 7 ;
  RTC->CR |= RTC_CR_ALRAE | RTC_CR_ALRAIE ;   // enable  ALARM A

	//  enable write protection
	RTC->WPR = 0xff;

  __enable_irq();         //Alarm is set, so irqs can be enabled again

  NVIC_DisableIRQ(RTC_Alarm_IRQn);
  NVIC_ClearPendingIRQ(RTC_Alarm_IRQn);
 // NVIC_SetPriority(RTC_Alarm_IRQn,0);
  NVIC_EnableIRQ(RTC_Alarm_IRQn);
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
	rtc_init();
}

void loop() {
  uint32_t ms;
  char str[128];

  ms = rtc_ms();
  sprintf(str,"%d ms %d ticks",ms,ticks);
  Serial.println(str);
  delay(5000); 
}
