/*

v1.1 - Only print serial stick values when we have new values from the RX

CH1 = Roll
CH2 = Pitch
CH3 = Throttle
CH4 = Yaw
*/

#define PD0 0
#define PD1 1
#define PD2 2
#define PD3 3
#define PD4 4
#define PD5 5
#define PD6 6
#define PD7 7

#define RX_CHANNELS 4

volatile uint16_t rollResult = 127;
volatile uint16_t pitchResult = 127;
volatile uint16_t throttleResult = 127;
volatile uint16_t yawResult = 127;


volatile int8_t updateSerial = false;

volatile int rollTime = 0;
volatile int pitchTime = 0;
volatile int throttleTime = 0;
volatile int yawTime = 0;


void setup() {
  Serial.begin(9600);
  
  // Set inputs
  for (uint8_t x = 2; x < RX_CHANNELS + 2; x++) {
    pinMode(x,INPUT);
  }
  
  // Attach interrupt to pin 2 as rising
  attachInterrupt(0, time_channels, RISING);
  
  // Reuse Timer1 to count the microseconds after pin 2, etc go high
  TCCR1A = 0;
  TIMSK1 = 0;
  TIFR1 = 0;
  TCNT1 = 0;
  TCCR1B = (1<<CS10);
}

void loop() {
  if (updateSerial == true) {
    // FMS PIC 9600 format 
    // 0xF0 + Number of RX channels
    Serial.write(0xF0 + RX_CHANNELS);
    
    // 0x00
    Serial.write((byte)0x00);
    
    // The 4 channels
    Serial.write(rollResult);
    Serial.write(pitchResult);
    Serial.write(throttleResult);
    Serial.write(yawResult);
    
    /*Serial.print("R ");
    Serial.print(rollResult);
    Serial.print(" P ");
    Serial.print(pitchResult);
    Serial.print(" T ");
    Serial.print(throttleResult);
    Serial.print(" Y ");
    Serial.print(yawResult);
    Serial.println(" ");*/
    
    updateSerial = false;
  }

  // Debug: for checking the values
  /*Serial.print("R ");
  Serial.print(rollTime);
  Serial.print(" P ");
  Serial.print(pitchTime);
  Serial.print(" T ");
  Serial.print(throttleTime);
  Serial.print(" Y ");
  Serial.print(yawTime);
  Serial.println(" ");
  delay(100);*/
}

// Check how long each channel is high for
void time_channels(void) {
  TCNT1 = 0; // Reset counter
  
  rollTime = 0;
  pitchTime = 0;
  throttleTime = 0;
  yawTime = 0;
  
  // While any of our 4 channels are high, check each channel and update it with the current timer value
  while ((PIND & ((1<<PD2) | (1<<PD3) | (1<<PD4) | (1<<PD5))))  { 
    if (PIND & (1<<PD2)) {    
      rollTime = TCNT1;
    }
    if (PIND & (1<<PD3)) {    
      pitchTime = TCNT1;
    }
    if (PIND & (1<<PD4)) {    
      throttleTime = TCNT1;
    }
    if (PIND & (1<<PD5)) {    
      yawTime = TCNT1;
    }
  }
  
  // Convert timer values to ~950 - 2000uS times
  rollTime = rollTime / 16;
  pitchTime = pitchTime / 16;
  throttleTime = throttleTime / 16;
  yawTime = yawTime / 16;
  
  // Map the times to 0 to 255
  // Adjust these values so your endpoints correspond close to 0 if you move the stick one way and close to 255 the other way
  rollResult = map(rollTime, 980, 2010, 0, 240); // Roll is picked up wrong by ppjoy, it wraps around so make it smaller
  pitchResult = map(pitchTime, 980, 2020, 0, 250);
  throttleResult = map(throttleTime, 980, 2020, 0, 250);
  yawResult = map(yawTime, 980, 2020, 0, 250);
  
  updateSerial = true;
}
