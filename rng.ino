// random RNG
#define REPS 50

uint32_t random(){
    while((RNG->SR & RNG_SR_DRDY) == 0); // wait
    return RNG->DR;
}

void setup() {
  Serial.begin(9600);
  RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;   // enable RNG
  RNG->CR = RNG_CR_RNGEN;

}

void loop() {
  uint32_t t, r;
  int i;

  t=micros();
  for(i=0;i<REPS;i++) r=random();
  t=micros()-t;
  float bps = REPS*32.e6/t;
  Serial.println(bps,2);
  Serial.println(r,HEX);

  delay(2000);

}
