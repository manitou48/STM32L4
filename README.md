##  80mhz STM32L476RE dragonfly sketches, and some butterfly tests

Some simple proof-of-concept sketches (alpha tests).

Files | Description
---|---
adcdma | timed ADC samples with DMA
adc_internal | ADC internal channels temperature, VBAT, VREF (DEPRECATED)
adc_test | ADC timing and averaging
crc.ino	|   CRC 32 example
dacdma  | timer clocks circular DMA to DAC
driftrtc.ino | check frequency of RTC crystal or LSI
IRtest  | IR transmit-receive test IRremote
isrperf.ino  | ISR overhead in cycles
mem2mem.ino  | DMA memcpy()
rng.ino      | hardware RNG
rtcalarm.ino  | RTC ALARM A every n seconds, ISR update
rtcsetget.ino | set RTC from epoch seconds, compiler TIME DATE
rtcwkup.ino  | RTC periodic WKUP ISR
sysinfo.ino      | hardware registers for clocks, peripherals
timers.ino   | timer callback

### alpha testing Notes & Experiences

As this is alpha testing, performance and function will change with time.

* LEDs are pulled HIGH, so write LOW to turn on

* delay() uses sleep (WFE)

* pin 44 (39 butterfly), user button, is pulled LOW, push for HIGH

* uses SerialUSB (Serial is redefined), you need Serial.begin(baudrate) and while(!Serial) is helpful

* HAL is only used for USB, rest of core is Thomas's creation 

* CPU clock 80mhz, AHBCLK 80mhz  APB1CLK 40mhz  APB2CLK 40mhz (max SPI 20mhz)

* PWM 488Hz, ADC 60 ADC_CLK ticks (1.5us), CRC 213 mbs (510 mbs -O3), RNG 37.2 mbs

* lower power tests: @80mhz, powered at 3.3v (no USB) power LED on, run (15ma), sleep/WFE (6ma), stop (0.9ma), standby (0.43ma), shutdown (0.34ma)

* to burn new bootloader, hold USER button and then push/release RESET button


-------reference --------------

https://www.tindie.com/products/onehorse/dragonfly-stm32l4-development-board/
https://www.tindie.com/products/TleraCorp/dragonfly-stm32l476-development-board/?pt=full_prod_search dragonfly
https://www.tindie.com/products/TleraCorp/butterfly-stm32l433-development-board/ butterfly

https://github.com/kriswiner/Dragonfly/

https://github.com/GrumpyOldPizza/arduino-STM32L4

http://www.stm32duino.com/viewtopic.php?t=896&start=10 early discussion

and http://www.stm32duino.com/viewtopic.php?t=1092

MBED STM32L4 critters:
* https://developer.mbed.org/platforms/ST-Discovery-L476VG/ i've tested it a bit
* https://developer.mbed.org/platforms/ST-Nucleo-L476RG/    some testing

------------------------------------

Some anecdotal performance comparisons:

* [computational speed](https://github.com/manitou48/DUEZoo/blob/master/perf.txt)  Coremark, linpack, AES, MD5, ...
* [power consumption] (https://github.com/manitou48/DUEZoo/blob/master/power.txt)
* [ISR latency] (https://github.com/manitou48/DUEZoo/blob/master/isrperf.txt)
* [SPI+DMA](https://github.com/manitou48/DUEZoo/blob/master/SPIperf.txt)
* [DMA memcpy](https://github.com/manitou48/DUEZoo/blob/master/mem2mem.txt)
* [random numbers](https://github.com/manitou48/DUEZoo/blob/master/RNGperf.txt)
* [blink size](https://github.com/manitou48/DUEZoo/blob/master/blinksize.txt)
* [crystal drift](https://github.com/manitou48/crystals/blob/master/crystals.txt)


```
    dragonfly tests   @80mhz   0.0.11
      alpha testing -- things will change

                   teensy 3.2  mbed K64F  dragonfly   T3.6 K66
        CRC16 16KB    120MHz     120MHz     80MHz      @180mhz
        bit-bang        9584       7101     16227        5649
        table-driven    2226       1623      2026         612
        hardware         275        275       615         207
		  (dragonfly hardware with -O3 unrolled loop: 257 us)

   memcpy and DMA memory-to-memory
          474.90 mbs  69 us   dma32 copy
          70.93 mbs  462 us   loop copy
          1092.27 mbs  30 us   memcpy
          1927.53 mbs  17 us   memset
          91.28 mbs  359 us   set loop

    IMU float-intensive filters  (no sensor data)  microseconds
                   T3.2     dragonfly   mbed K64F   T3.6 K66
                  @120mhz     @80mhz     @120mhz     @180mhz
       NXP/kalman   3396       535          459        285
       madgwick      203        18            8          7
       mahony        125        13            6          3
   
       fft (float)  8144       689          488        339

   sparkfun mpu9250 sensor/filter rate
	  dragonfly         6328 HZ
      dragonfly isr    45409 
      dragonfly opt    49458    isr + I2C/DMA
	  T3.2@120mhz       2714
	  T3.2@120mhz isr   4397
	  T3.5@120mhz       6640     (K64)
	  T3.5@120mhz isr  77972 

   sdqspi   58mbs 512-byte file read



USB  latency_test
      latency @ 1 bytes: 0.26 ms average, 1.30 maximum
      latency @ 2 bytes: 0.29 ms average, 1.95 maximum
      latency @ 12 bytes: 0.28 ms average, 1.75 maximum
      latency @ 30 bytes: 0.30 ms average, 1.90 maximum
      latency @ 62 bytes: 0.39 ms average, 1.99 maximum
      latency @ 71 bytes: 0.41 ms average, 1.33 maximum
      latency @ 128 bytes: 0.51 ms average, 1.73 maximum
      latency @ 500 bytes: 1.18 ms average, 2.53 maximum
      latency @ 1000 bytes: 2.14 ms average, 2.93 maximum
      latency @ 2000 bytes: 4.13 ms average, 4.49 maximum
      latency @ 4000 bytes: 7.97 ms average, 8.05 maximum
      latency @ 8000 bytes: 15.76 ms average, 15.96 maximum

USBreadbytes
   Average bytes per second = 512424

   mbed TLS
     md5 335 us 3056 KBs
     sha256 869 us 1178 KBs
     aes setkey 10
     encrypt 157
     decrypt 113
     100! us 1979
     93326215443944152681
     entropy us 189
     rng us 2328

speedTest 
F_CPU = 80000000 Hz
1/F_CPU = 0.0125 us
The next tests are runtime compensated for overhead
  nop                       : 0.013 us
  Arduino digitalRead       : 0.276 us
  Arduino digitalWrite      : 0.236 us
  pinMode                   : 4.571 us
  multiply volatile byte    : 0.074 us
  divide volatile byte      : 0.086 us
  multiply volatile integer : 0.061 us
  divide volatile integer   : 0.071 us
  multiply volatile long    : 0.059 us
  multiply single float     : 0.074 us
  divide single float       : 0.238 us
  multiply double float     : 0.751 us
  divide double float       : 12.446 us
  random()                  : 0.821 us
  bitSet() with volatile    : 0.048 us
  analogRead()              : 33.196 us    38.598 0.0.4  38.448 0.0.6
  analogWrite() PWM         : 5.606 us    5.548 0.0.6
  delay(1)                  : 999.996 us
  delay(100)                : 99999.996 us
  delayMicroseconds(1)      : 1.715 us
  delayMicroseconds(5)      : 5.711 us
  delayMicroseconds(100)    : 100.746 us


```

![coremark](https://github.com/manitou48/STM32L4/blob/master/coremark.png)
![coremark power](https://github.com/manitou48/STM32L4/blob/master/coremarka.png)
