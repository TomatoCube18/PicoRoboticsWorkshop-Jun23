/* 
 *  Ex_14 - Physical Computing
 *  Revisit Input & Output
 *  
 *  LED 
 *  -- GP12
 *  Switch
 *  -- GP27 or GP28
 *  
 */

#define ONBOARD_LED 12    // LED GP12
#define ONBOARD_SW  28    // End-Stop Switch GP27 or GP28

int current_LED = LOW;

void setup() {
   Serial.begin(115200); //set up serial library baud rate to 115200
   pinMode(ONBOARD_LED, OUTPUT);
   pinMode(ONBOARD_SW, INPUT);

   digitalWrite(ONBOARD_LED, current_LED);    // turn the LED off by making the voltage LOW   
}

void loop() {
   current_LED = (current_LED == HIGH) ? LOW: HIGH;
   digitalWrite(ONBOARD_LED, current_LED);
   
   if (digitalRead(ONBOARD_SW) == LOW) {  // Switch is asserted Low (Active Low)
      delay(100);
   }
   else {
      delay(500);
   }
}
                    
                