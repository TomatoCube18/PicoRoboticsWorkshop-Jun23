/* 
 *  Ex_03 - Conditional
 *  Simple communication through the Serial monitor.
 *  
 */
String rcvString = "";
float weight = 0;
float height = 0;
uint8_t question = 0; // A variable to keep track which question is the system asking (0 -> Weight , 1 -> Height)

void setup() {
   Serial.begin(115200); //set up serial library baud rate to 115200
   delay(2000);
   Serial.println("What is your weight (in kg) ?");
}

void loop() {
   float tempValue = 0;
   float bmi = 0;
   if(Serial.available()) { //if number of bytes (characters) available for reading
      rcvString = Serial.readString(); // read string until newline character
      tempValue = rcvString.toFloat(); // convert string to float
      
      if (question == 0) {  // Weight entry
          weight = tempValue;
          Serial.println("");
          Serial.print("Weight you enter is ");
          Serial.println(weight);

          Serial.println("");
          Serial.println("What is your height (in meter) ?");
          question = 1;
      }
      else {  // Height entry
          height = tempValue;
          Serial.println("");
          Serial.print("Height you enter is ");
          Serial.println(height);

          if (height > 0 && weight > 0) {
              bmi = weight / (pow (height, 2));
              Serial.println("");
              Serial.print("Your BMI is ");
              Serial.print(bmi);
              Serial.print(" & ");
              if (bmi >= 40) 
                  Serial.println("You are Morbidly Obese");
              else if (bmi >= 35 && bmi < 40)
                  Serial.println("You are Severely Obese");
              else if (bmi >= 30 && bmi < 35)
                  Serial.println("You are Moderately Obese");
              else if (bmi >= 25 && bmi < 30)
                  Serial.println("You are overweight");
              else if (bmi >= 18.5 && bmi < 25)
                  Serial.println("You are Normal, Hurray!!");
              else if (bmi >= 16 && bmi < 18.5)
                  Serial.println("You are Underweight");
              else 
                  Serial.println("You are Severely Underweight");
          }
          else {
              Serial.println("");
              Serial.print("Invalid entry ");
          }
          
          
          Serial.println("");
          Serial.println("What is your weight (in kg) ?");
          question = 0;
      }
   }
}
                    
                
