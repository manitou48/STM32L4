##  80mhz STM32L476RE dragonfly sketches

Some simple proof-of-concept sketches (alpha tests).

Files | Description
---|---
adc_internal | ADC internal channels temperature, VBAT, VREF (DEPRECATED)
crc.ino	|   CRC 32 example
dacdma  | timer clocks circular DMA to DAC
driftrtc.ino | check frequency of RTC crystal or LSI
IRtest  | IR transmit-receive test IRremote
isrperf.ino  | ISR overhead in cycles
mem2mem.ino  | DMA memcpy()
rng.ino      | hardware RNG
sysinfo.ino      | hardware registers for clocks, peripherals
timers.ino   | timer callback

### alpha testing Notes & Experiences

As this is alpha testing, performance and function will change with time.

* LEDs are pulled HIGH, so write LOW to turn on

* pin 44, user button, is pulled HIGH

* uses SerialUSB (Serial is redefined), you need Serial.begin(baudrate) and while(!Serial) is helpful

* HAL is only used for USB, rest of core is Thomas's creation 

* default pins for Wire are pads on backside, Wire1 uses A4/A5

* CPU clock 80mhz, AHBCLK 80mhz  APB1CLK 40mhz  APB2CLK 40mhz (max SPI 20mhz)

* PWM 488Hz

* to burn new bootloader, hold RESET button and then push/release USER button


-------reference --------------

https://www.tindie.com/products/onehorse/dragonfly-stm32l4-breakout-board/

https://github.com/kriswiner/Dragonfly/

https://github.com/GrumpyOldPizza/arduino-STM32L4

http://www.stm32duino.com/viewtopic.php?t=896&start=10 early discussion
