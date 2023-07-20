/*
 *  Ex_14 - Physical Computing
 *  Revisit Input & Output
 *
 *  LED
 *  -- GP12
 *  Switch
 *  -- GP7
 *
 */

#define ONBOARD_LED 12    // LED GP12
#define ONBOARD_SW  7    // Switch GP7

uint8_t current_State = 0;

uint8_t current_LED = LOW;
uint8_t current_SW = HIGH;
uint8_t previous_SW = HIGH;
unsigned long lastTransition = 0;

void setup() {
   pinMode(ONBOARD_LED, OUTPUT);
   pinMode(ONBOARD_SW, INPUT_PULLUP);
   digitalWrite(ONBOARD_LED, current_LED);    // turn the LED off by making the voltage LOW   
}

void loop() {

    current_SW = digitalRead(ONBOARD_SW); // Switch is asserted Low (Active Low)    
    current_State = ((previous_SW == HIGH) && (current_SW == LOW)) ? ((current_State < 3) ? current_State + 1 : 0) : current_State;
      
    switch (current_State) {
      case 0: case 1: case 2:
          if (lastTransition + (current_State == 0? 1000: current_State == 1? 500: 100) < millis()) {
              current_LED = ((current_LED == HIGH) ? LOW: HIGH);
              lastTransition = millis();
          }
          break;
      case 3:
          current_LED = HIGH;
          lastTransition = millis();
          break;   
      default:
          current_State = 0;
    }
      
    previous_SW = current_SW;
    digitalWrite(ONBOARD_LED, current_LED); 
}
                    
                
