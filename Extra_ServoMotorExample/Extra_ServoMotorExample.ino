#include <Servo.h>

Servo myservoA; 
Servo myservoB; 

#define SERVO_A 16
#define SERVO_B 17


void setup() {
      myservoA.attach(SERVO_A, 600, 2400);
      myservoB.attach(SERVO_B, 600, 2400);

      for(int i = 0; i <= 180; i+=10){ // Turn counter clock wise
        myservoA.write(i);              // tell servo to go to position in variable 'pos'
        myservoB.write(i);              // tell servo to go to position in variable 'pos'
        delay(100);
      }

      for(int i = 180; i >= 0; i-=10){ // Turn clock wise
        myservoA.write(i);              // tell servo to go to position in variable 'pos'
        myservoB.write(i);              // tell servo to go to position in variable 'pos'
        delay(100);
      }
}

void loop() {
  
}

