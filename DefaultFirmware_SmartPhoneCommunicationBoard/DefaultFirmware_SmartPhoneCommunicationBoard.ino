// Line Sensor Test

#define IR_RECEIVE_PIN      6
#define IR_RECV 6

#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#define OLED_RESET -1
#define I2C_ADDRESS 0x3C

#define LINESENI2CADDR 0x17

#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#define ONBOARD_LED 12
#define ONBOARD_SW 7

#define SPK_OUT   22 

#define NEOPIXEL  20 
#define NUMPIXELS 3 

#define ENDSTP_SW_A 27
#define ENDSTP_SW_B 28
#define LDR_PIN A0  // 26 - ADC0

#define SERVO_A 16
#define SERVO_B 17

#define MotorA_1 8
#define MotorA_2 9
#define MotorB_1 10
#define MotorB_2 11


#define I2C1_SDA 18
#define I2C1_SCL 19

#define UART0_TX 0
#define UART0_RX 1


Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL, NEO_GRB + NEO_KHZ800);
SSD1306AsciiWire oled(Wire1);


unsigned long lastOledRefresh = 0;

uint8_t currButtonState = HIGH;
uint8_t prevButtonState = HIGH;
bool isAdvanceRobot = false;


unsigned long currentMillis = 0;

#define longMillis 400
#define shortMillis 200

uint8_t currLDRState_L = HIGH;
uint8_t prevLDRState_L = HIGH;
uint8_t buffLDRState_L = 0xFF;
unsigned long lastLDR_IRQ_L = 0;
unsigned long LowTime_LDR_L = 0;
unsigned long HighTime_LDR_L = 0;

uint8_t currLDRState_R = HIGH;
uint8_t prevLDRState_R = HIGH;
uint8_t buffLDRState_R = 0xFF;
unsigned long lastLDR_IRQ_R = 0;
unsigned long LowTime_LDR_R = 0;
unsigned long HighTime_LDR_R = 0;


// MotorState - 0:Stop, 1:Forward, 2:Backward
uint8_t leftMotorState = 0;
uint8_t rightMotorState = 0;

void setup() {
    byte error;
    int nDevices;

    // USB UART
    Serial.begin(115200);

    pinMode(UART0_TX, INPUT_PULLUP);
    pinMode(UART0_RX, INPUT_PULLUP);
    
    Wire1.setSDA(I2C1_SDA);
    Wire1.setSCL(I2C1_SCL);
    Wire1.begin(); 

    oled.begin( &Adafruit128x32, I2C_ADDRESS);
    oled.setFont(System5x7);
    oled.clear();
    oled.println("Hello TomatoCube!");

    pinMode(IR_RECV, INPUT_PULLUP);

    pinMode(ENDSTP_SW_A, INPUT_PULLUP);
    pinMode(ENDSTP_SW_B, INPUT_PULLUP);
    pinMode(ONBOARD_SW, INPUT_PULLUP);
    pinMode(ONBOARD_LED, OUTPUT);

    pinMode(MotorA_1, OUTPUT);
    pinMode(MotorA_2, OUTPUT);
    pinMode(MotorB_1, OUTPUT);
    pinMode(MotorB_2, OUTPUT);
    digitalWrite(MotorA_1, LOW);
    digitalWrite(MotorA_2, LOW);
    digitalWrite(MotorB_1, LOW);
    digitalWrite(MotorB_2, LOW);
    
    pinMode(SPK_OUT, OUTPUT);
    noTone(SPK_OUT);
    delay(500);

    
    tone(SPK_OUT, 659, 90); // NOTE_E5
    delay(100);
    tone(SPK_OUT, 587, 90); // NOTE_D5
    delay(100);
    tone(SPK_OUT, 370, 90); // NOTE_FS4
    delay(100);
    noTone(SPK_OUT);
    
    for(int i=0; i<NUMPIXELS; i++) { // For each pixel...  
        pixels.setPixelColor(i, pixels.Color((i==0)?150:0, (i==1)?150:0, (i==2)?150:0));
        pixels.show();   // Send the updated pixel colors to the hardware.
        delay(500); // Pause before next pass through loop
    }
  
    prevButtonState = digitalRead(ONBOARD_SW);

//    attachInterrupt(UART0_TX, ldrChange, CHANGE);  //Setting IR detection on FALLING edge.
}

void ldrChange() {
//    if (isAdvanceRobot) {
        currentMillis = millis();
        buffLDRState_L = (buffLDRState_L<<1) | ((digitalRead(UART0_TX) == HIGH)?1:0);
        if ((prevLDRState_L == HIGH) && ((buffLDRState_L & 0x0F) == 0x00)) {
            LowTime_LDR_L = currentMillis - lastLDR_IRQ_L;
            lastLDR_IRQ_L = currentMillis;  
            prevLDRState_L = LOW; 
        }else if ((prevLDRState_L == LOW) && ((buffLDRState_L & 0x0F) == 0x0F)) {
            HighTime_LDR_L = currentMillis - lastLDR_IRQ_L;
            lastLDR_IRQ_L = currentMillis;
            prevLDRState_L = HIGH;
        }
        
      
        currentMillis = millis();
        buffLDRState_R = (buffLDRState_R<<1) | ((digitalRead(UART0_RX) == HIGH)?1:0);
        if ((prevLDRState_R == HIGH) && ((buffLDRState_R & 0x0F) == 0x00)) {
            LowTime_LDR_R = currentMillis - lastLDR_IRQ_R;
            lastLDR_IRQ_R = currentMillis;  
            prevLDRState_R = LOW; 
        }else if ((prevLDRState_R == LOW) && ((buffLDRState_R & 0x0F) == 0x0F)) {
            HighTime_LDR_R = currentMillis - lastLDR_IRQ_R;
            lastLDR_IRQ_R = currentMillis;
            prevLDRState_R = HIGH;
        }
      
//    }
}

void loop() {

    
    
    currButtonState = digitalRead(ONBOARD_SW);
    if ((prevButtonState == LOW) && (currButtonState == HIGH)) {
        
        isAdvanceRobot = !isAdvanceRobot;
        
    }
    prevButtonState = currButtonState;
    digitalWrite(ONBOARD_LED, !isAdvanceRobot);

    if (millis() > lastOledRefresh + 500) {
        lastOledRefresh = millis();
        if (isAdvanceRobot) {
            oled.clear();
            oled.println("Advance Robot!");
            oled.println("(Left)  (Right)");
            oled.print((leftMotorState == 0)? "  Stop    ": (leftMotorState == 1)?"  Fwd     ":"  Rev     ");
            oled.print((rightMotorState == 0)? "Stop": (rightMotorState == 1)?"Fwd":"Rev");
            
        } else {
            oled.clear();
            oled.println("Basic Robot!");
            oled.println("(Left)  (Right)");
            oled.print((leftMotorState == 0)? "  Stop    ":"  Fwd     ");
            oled.print((rightMotorState == 0)? "Stop":"Fwd");
        }
    }
      
    if (isAdvanceRobot) {

        ldrChange();
        
        if (millis() - lastLDR_IRQ_L > longMillis + shortMillis){
          leftMotorState = 0;
        }
        else if (LowTime_LDR_L + HighTime_LDR_L < longMillis + shortMillis + 10) {
          if (HighTime_LDR_L > LowTime_LDR_L) {
              if ((abs((int)HighTime_LDR_L - longMillis) < 50) && 
                  (abs((int)LowTime_LDR_L - shortMillis) < 50))
                leftMotorState = 2;

          }
          else {
              if ((abs((int)LowTime_LDR_L - longMillis) < 50) && 
                  (abs((int)HighTime_LDR_L - shortMillis) < 50))
                leftMotorState = 1;

          }
        } 

        
        if (millis() - lastLDR_IRQ_R > longMillis + shortMillis){
          rightMotorState = 0;
        }
        else if (LowTime_LDR_R + HighTime_LDR_R < longMillis + shortMillis + 10) {
          if (HighTime_LDR_R > LowTime_LDR_R) {
              if ((abs((int)HighTime_LDR_R - longMillis) < 50) && 
                  (abs((int)LowTime_LDR_R - shortMillis) < 50))
                rightMotorState = 2;
                
          }
          else {
              if ((abs((int)LowTime_LDR_R - longMillis) < 50) && 
                  (abs((int)HighTime_LDR_R - shortMillis) < 50))
                rightMotorState = 1;

                
          }
        } 

//          Serial.print((int)HighTime_LDR_L);
//          Serial.print(" ");
//          Serial.println((int)LowTime_LDR_L);
          analogWrite(MotorA_1, (leftMotorState == 1)? 200: 0);
          analogWrite(MotorA_2, (leftMotorState == 2)? 200: 0);
          analogWrite(MotorB_1, (rightMotorState == 1)? 200: 0);
          analogWrite(MotorB_2, (rightMotorState == 2)? 200: 0);
    
        
      
    } else {
        leftMotorState = (digitalRead(UART0_TX) == HIGH) ? 0: 1;
        rightMotorState = (digitalRead(UART0_RX) == HIGH) ? 0: 1;
//        digitalWrite(MotorA_1, (leftMotorState == 1)? HIGH: LOW);
//        digitalWrite(MotorA_2, LOW);
//        digitalWrite(MotorB_1, (rightMotorState == 1)? HIGH: LOW);
//        digitalWrite(MotorB_2, LOW);
        analogWrite(MotorA_1, (leftMotorState == 1)? 200: 0);
        analogWrite(MotorA_2, 0);
        analogWrite(MotorB_1, (rightMotorState == 1)? 200: 0);
        analogWrite(MotorB_2, 0);
    }
    
}
