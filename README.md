##  80mhz STM32L476RE dragonfly sketches

Some simple proof-of-concept sketches (alpha tests).

Files | Description
---|---
adcdma | timed ADC samples with DMA
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

* delay() uses sleep (WFE)

* pin 44, user button, is pulled HIGH

* uses SerialUSB (Serial is redefined), you need Serial.begin(baudrate) and while(!Serial) is helpful

* HAL is only used for USB, rest of core is Thomas's creation 

* default pins for Wire are pads on backside, Wire1 uses A4/A5

* CPU clock 80mhz, AHBCLK 80mhz  APB1CLK 40mhz  APB2CLK 40mhz (max SPI 20mhz)

* PWM 488Hz, ADC 60 ADC_CLK ticks

* to burn new bootloader, hold USER button and then push/release RESET button


-------reference --------------

https://www.tindie.com/products/onehorse/dragonfly-stm32l4-breakout-board/

https://github.com/kriswiner/Dragonfly/

https://github.com/GrumpyOldPizza/arduino-STM32L4

http://www.stm32duino.com/viewtopic.php?t=896&start=10 early discussion

MBED STM32L4 critters:
* https://developer.mbed.org/platforms/ST-Discovery-L476VG/ i've tested it a bit
* https://developer.mbed.org/platforms/ST-Nucleo-L476RG/

------------------------------------

Some anecdotal performance comparisons:

* [computational speed](https://github.com/manitou48/DUEZoo/blob/master/perf.txt)
* [power consumption] (https://github.com/manitou48/DUEZoo/blob/master/power.txt)
* [ISR latency] (https://github.com/manitou48/DUEZoo/blob/master/isrperf.txt)
* [SPI+DMA](https://github.com/manitou48/DUEZoo/blob/master/SPIperf.txt)
* [DMA memcpy](https://github.com/manitou48/DUEZoo/blob/master/mem2mem.txt)
* [random numbers](https://github.com/manitou48/DUEZoo/blob/master/RNGperf.txt)
* [crystal drift](https://github.com/manitou48/crystals/blob/master/crystals.txt)

