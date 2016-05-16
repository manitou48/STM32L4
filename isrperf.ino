// isrperf   jumper 12 to 13
//  interrupt done by line  then port  
//#define FASTISR

#define CYCLES SysTick->VAL

volatile uint32_t ticks,ti,tf;

void fcn() {
    tf=CYCLES;
}
void isr(){
    ti=CYCLES;
#ifdef FASTISR
      EXTI->PR1 =  1<<11; // clear interrupt line 11
#endif
}


void setup() {
  Serial.begin(9600);
  pinMode(13,OUTPUT);   // PC10
  pinMode(12,INPUT);  //  PC11
  Serial.println("isrperf  jumper 12 to 13");
}

void loop() {
      uint32_t t1,t2,t3,t4;
      char str[96];
   

    GPIOC->BSRR = 1<<10; delay(3000); GPIOC->BRR= 1<<10;  //  turn LED on off
    attachInterrupt(12,isr,RISING);
#ifdef FASTISR
    NVIC_SetVector( EXTI15_10_IRQn, (uint32_t)isr);
#endif


    t1=CYCLES;
    t2=CYCLES;
 
    GPIOC->BSRR = 1<<10;  // set bit and fire isr fcn pin 12
    while(ti == 0);  // wait til IRQ fired
    t3=CYCLES;
    fcn();
    t4=CYCLES;
    sprintf(str,"clockres %d  isr in %d  isrout %d fcn in %d  fcnout %d",
       t1-t2,t2-ti,ti-t3,t3-tf,tf-t4);
    Serial.println(str);

    t1=CYCLES;
    digitalWrite(13,LOW);
    t2=CYCLES;
   // GPIOC->BSRR = 1<<(10+16);   // fast clear
     GPIOC->BRR = 1<<10;   // fast clear
    t3=CYCLES;
    sprintf(str,"pin hi %d fast %d",t1-t2,t2-t3);
    Serial.println(str);
    delay(5000);
}
