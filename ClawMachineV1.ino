#include <Servo.h>
#include <LiquidCrystal_74HC595.h> 
#include "stepper.h"
#include "shiftregister.h"
#include "lcdscreen.h"

//Claw Servo
#define servoPin 11
Servo clawServo;

// TMR1 Interrupt Variables
uint32_t wait = 0;
int tmintrvl = 7000;
int clk = 0;
int x = 1023 / 2,y = 1023 / 2;
const float samplerate = 9200.0f;
uint8_t btnPressed = 0;
int clawOpenAngle = 180;
int clawClosedAngle = 10;

enum PlayMode {
  MANUALRTH,
  AUTORTH
};

bool returnHome = false;
PlayMode playMode = AUTORTH;

void setup() {
  //Initialize Output Shift Registers
  initialize_output_register();

  //Initialize Input Register
  initialize_input_register();

  //Initialize Stepper Drivers
  initializeStepperDriver();
  
  //Initialize The LCD Screen
  initialize_lcd_screen();
  
  //Initialize Button LED Status
  buttonledstatus = false;

  //Initialize Timer 2 for Interrupts
  initialize_timer_2();
  
  //Setup X and Y Steppers
  setupStepperResolution(X,FULL);
  setupStepperResolution(Y,FULL);
  
  //Open the claw
  clawServo.attach(servoPin);
  clawServo.write(100);
  delay(250);
  clawServo.detach();

      retractClaw();
}

/*
 * Input Shift Register Initialization
 * Sets up the initial state of the 74HC165 input shift register
 */
void initialize_input_register(){
  //Initialize The Input Shift Register
  pinMode(inputLatchPin, OUTPUT);
  pinMode(inputClockEnablePin, OUTPUT);
  pinMode(inputClockPin, OUTPUT);
  pinMode(inputDataPin, INPUT);
  digitalWrite(inputClockPin, HIGH);
  digitalWrite(inputLatchPin, HIGH);
  delay(100);
}


/*
 * Output Shift Registers Initialization
 * Sets up the initial state of the 74HC595 output shift registers high and low bytes
 */
void initialize_output_register(){
  //Initialize Output Shift Register
  SHIFTREGISTERSTATUSLOW = 0x00;
  SHIFTREGISTERSTATUSHIGH = 0b00000000;
  SHIFTREGISTERSTATUSHIGH |= (1 << 0);
  pinMode(outputDataPin,OUTPUT);
  pinMode(outputLatchPin,OUTPUT);
  pinMode(outputClockPin,OUTPUT);
  digitalWrite(outputDataPin,LOW);
  digitalWrite(outputLatchPin,LOW);
  digitalWrite(outputClockPin,LOW);
  updateShiftRegisterStatus();
}


/*
 * Timer 2 Initialization Routine
 * Sets up the frequency of the TIMER2 module for generating clock pulses for the stepper motors
 */
void initialize_timer_2(){
  //Initialize Timer One 
  noInterrupts(); // disable all interrupts
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  OCR2A = 16000000.0f / samplerate; // compare match register for IRQ with selected samplerate
  TCCR2B |= (1 << WGM12); // CTC mode
  TCCR2B |= (1 << CS10); // no prescaler
  TIMSK2 |= (1 << OCIE1A); // enable timer compare interrupt
  interrupts(); // re-enable all interrupts
}

/*
 * Timer 2 Interrupt Service Routine
 * This ISR is used to move the stepper motors as close to realtime as possible
 */
ISR(TIMER2_COMPA_vect){
  // generate the next clock pulse on accumulator overflow
  wait += tmintrvl; 
  if (wait > 0xffff) {
    if (clk && btnPressed == false) { //On the rising edge of the clock
      
      //Read the limit switch values
      getLimitSwitchValues();
      
      //Move based on wether the limit switches are pressed or not in a given direction
      x = analogRead(A0); 
      y = analogRead(A1);

      if(returnHome == false){
        //X Axis
        if(x > ((1023 / 2) + 200) && bitRead(inputRegisterValues,2) == 0){
          SHIFTREGISTERSTATUSLOW |= (1 << XMOTORDIRBIT);
          SHIFTREGISTERSTATUSLOW |= (1 << XMOTORSTEPBIT);
        }else if(x < ((1023 / 2) - 200) && bitRead(inputRegisterValues,3) == 0){
          SHIFTREGISTERSTATUSLOW &= ~(1 << XMOTORDIRBIT);
          SHIFTREGISTERSTATUSLOW |= (1 << XMOTORSTEPBIT);
        }
  
        //Or Y Axis
        if(y > ((1023 / 2) + 200) && bitRead(inputRegisterValues,0) == 0){
          SHIFTREGISTERSTATUSHIGH &= ~(1 << YMOTORDIRBIT);
          SHIFTREGISTERSTATUSHIGH |= (1 << YMOTORSTEPBIT);
        }else if(y < ((1023 / 2) - 200) && bitRead(inputRegisterValues,1) == 0){
          SHIFTREGISTERSTATUSHIGH |= (1 << YMOTORDIRBIT);
          SHIFTREGISTERSTATUSHIGH |= (1 << YMOTORSTEPBIT);
        }else{
          //Do jack shiznet
        }
      }else{
//        //Return home
//        SHIFTREGISTERSTATUSLOW |= (1 << XMOTORDIRBIT);
//        SHIFTREGISTERSTATUSLOW |= (1 << XMOTORSTEPBIT);
//        SHIFTREGISTERSTATUSHIGH &= ~(1 << YMOTORDIRBIT);
//        SHIFTREGISTERSTATUSHIGH |= (1 << YMOTORSTEPBIT);
      }
      
      //Push the results to the output shift register controlling the steppers
      updateShiftRegisterStatus();

    }else{
      //Otherwise stop the stepper motor
       SHIFTREGISTERSTATUSLOW &= ~(1 << XMOTORSTEPBIT);
       SHIFTREGISTERSTATUSHIGH &= ~(1 << YMOTORSTEPBIT);
       updateShiftRegisterStatus();
    }
    wait -= 0x10000;
    clk = !clk;
  }
}


/*
 * Limit Switch Monitoring
 * Retrieves the input values of the limit switches from the 74HC165 shift register inputs
 */
void getLimitSwitchValues(){
  //Latch the values in from the input shift register
  digitalWrite(inputLatchPin, LOW);
  delayMicroseconds(5); 
  digitalWrite(inputLatchPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(inputClockPin, HIGH);
  digitalWrite(inputClockEnablePin, LOW);
  inputRegisterValues = shiftIn(inputDataPin,inputClockPin, MSBFIRST);
  digitalWrite(inputClockEnablePin, HIGH); 
}

/*
 * Limit Switch Debugging Helper Function
 * Displays the values of all 8 input bits of the 74HC164 shift register onto the LCD module for debugging
 */
void writeLimitSwitchValuesToLCD(){
  //Clear the LCD
  lcd.clear();

  //Set the cursor to the top left
  lcd.setCursor(0,0);
  lcd.print("LSB: ");
  lcd.print(bitRead(inputRegisterValues,0));
  lcd.setCursor(0,1);
  //Print the inputs as binary
  lcd.print(bitRead(inputRegisterValues,0));
  lcd.print(bitRead(inputRegisterValues,1));
  lcd.print(bitRead(inputRegisterValues,2));
  lcd.print(bitRead(inputRegisterValues,3));
  lcd.print(bitRead(inputRegisterValues,4));
  lcd.print(bitRead(inputRegisterValues,5));
  lcd.print(bitRead(inputRegisterValues,6));
  lcd.print(bitRead(inputRegisterValues,7));
}


/*
 * Blinks Claw Button LED
 */
void blinkButtonLed(){
  if(buttonledstatus == true){
    SHIFTREGISTERSTATUSHIGH &= ~(1 << 0);
    buttonledstatus = false;
    delay(500);
  }else{
    SHIFTREGISTERSTATUSHIGH |= (1 << 0);
    buttonledstatus = true;
    delay(500);
  }
  updateShiftRegisterStatus();
}

/*
 * Retracts the claw
 */
void retractClaw(){
  //Set the claw retractor stepper motor resolution to full
  setupStepperResolution(CLAW,FULL);

   //  Set the direction bit
  //  DIR | MOTOR DIRECTION
  //  =====================
  //   0  | Clockwise
  //   1  | Counter-clockwise
  //Change the claw direction to counterclockwise
  SHIFTREGISTERSTATUSHIGH &= ~(1 << CLAWMOTORDIRBIT);
  updateShiftRegisterStatus();

  //Set the claw retractor stepper step delay time
  int delayAmount = 10;

  //Set the number of revolutions required
  int revolutions = 4;

  //Set the number of steps for the motor to take
  int numberOfSteps = (CLAWSTEPSPERREV / CLAWMOTORSTEPANGLE) * revolutions; 
  
  //Send step pulses to the claw retractor stepper with a 50% duty cycle
  for(int curStep = 0; curStep < numberOfSteps; curStep++){
    //Send the high pulse
    SHIFTREGISTERSTATUSHIGH |= (1 << CLAWMOTORSTEPBIT);
    updateShiftRegisterStatus(); 

    //Wait the delay amount
    delayMicroseconds(delayAmount);

    //Send the low pulse
    SHIFTREGISTERSTATUSHIGH &= ~(1 << CLAWMOTORSTEPBIT);
    updateShiftRegisterStatus();

    //Wait the delay amount
    delayMicroseconds(delayAmount);
  }
}

/*
 * Drop the claw
 */
void dropClaw(){
  //Set the claw retractor stepper motor resolution to full
  setupStepperResolution(CLAW,FULL);

   //  Set the direction bit
  //  DIR | MOTOR DIRECTION
  //  =====================
  //   0  | Clockwise
  //   1  | Counter-clockwise
  //Change the claw direction to clockwise
  SHIFTREGISTERSTATUSHIGH |= (1 << CLAWMOTORDIRBIT);
  updateShiftRegisterStatus();

  //Set the claw retractor stepper step delay time
  int delayAmount = 10;
  
  //Set the number of revolutions required
  int revolutions = 4;

  //Set the number of steps for the motor to take
  int numberOfSteps = (CLAWSTEPSPERREV / CLAWMOTORSTEPANGLE) * revolutions; 
  
  //Send step pulses to the claw retractor stepper with a 50% duty cycle
  for(int curStep = 0; curStep < numberOfSteps; curStep++){
    //Send the high pulse
    SHIFTREGISTERSTATUSHIGH |= (1 << CLAWMOTORSTEPBIT);
    updateShiftRegisterStatus(); 

    //Wait the delay amount
    delayMicroseconds(delayAmount);

    //Send the low pulse
    SHIFTREGISTERSTATUSHIGH &= ~(1 << CLAWMOTORSTEPBIT);
    updateShiftRegisterStatus();

    //Wait the delay amount
    delayMicroseconds(delayAmount);
  }
}

/*
 * Close The Claw -- Slowly, to drop current draw
 */
void closeClaw(){
  clawServo.attach(servoPin);
  clawServo.write(90);
  delay(60);
  clawServo.write(80);
  delay(60);
  clawServo.write(70);
  delay(60);
  clawServo.write(60);
  delay(60);
  clawServo.write(50);
  delay(60);
  clawServo.write(40);
  delay(60);
  clawServo.write(30);
  delay(60);
  clawServo.write(20);
  delay(60);
  clawServo.write(0);
  delay(60);
  clawServo.detach();
}

/*
 * Open The Claw -- Slowly, to drop current draw
 */
void openClaw(){
  clawServo.attach(servoPin);
  clawServo.write(20);
  delay(60);
  clawServo.write(30);
  delay(60);
  clawServo.write(40);
  delay(60);
  clawServo.write(50);
  delay(60);
  clawServo.write(60);
  delay(60);
  clawServo.write(70);
  delay(60);
  clawServo.write(80);
  delay(60);
  clawServo.write(90);
  delay(60);
  clawServo.write(100);
  delay(60);
  clawServo.detach();
}


/*
 * Manual Return To Home Procedure
 */
void ManualRTHProcedure(){
   //If the claw is in grabbing mode
    if(clawStatus == GRABBING){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Grab Some Candy");

      //Lower the claw
      dropClaw();
      delay(2000);

      //Close The Claw
      closeClaw();
      
      delay(1000);
      //Retract the claw
      retractClaw();

      //Change the claw mode to dropping
      clawStatus = DROPPING;

    }
    //Otherwise if the claw is in dropping mode
    else if(clawStatus == DROPPING){ 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Drop The Candy");

      //Drop the claw
      dropClaw();
      delay(1500);

      //Open the claw
      openClaw();
      
      delay(1000);
      //Retract the claw
      retractClaw();

      //Change the claw mode to grabbing
      clawStatus = GRABBING;
    }
}

/*
 * Auto Return To Home Procedure
 */
void AutoRTHProcedure(){
  dropClaw();
  delay(2000);
  closeClaw();
  delay(1000);
  retractClaw();
  returnHome = true;

   //  Set the direction bit
  //  DIR | MOTOR DIRECTION
  //  =====================
  //   0  | Clockwise
  //   1  | Counter-clockwise
  //Change the x and y direction to clockwise
  SHIFTREGISTERSTATUSLOW &= ~(1 << XMOTORDIRBIT);
  SHIFTREGISTERSTATUSHIGH &= ~(1 << YMOTORDIRBIT);

  //Set the step delay amount
  int delayAmount = 1000; 

  getLimitSwitchValues();


  noInterrupts();
  
  //Move X to Home Limit Switch
  while(bitRead(inputRegisterValues,3) == 0){

    //Send the high pulse
    SHIFTREGISTERSTATUSLOW |= (1 << XMOTORSTEPBIT);
    updateShiftRegisterStatus(); 

    //Wait the delay amount
    delayMicroseconds(delayAmount);

    //Send the low pulse
    SHIFTREGISTERSTATUSLOW &= ~(1 << XMOTORSTEPBIT);
    updateShiftRegisterStatus();

    //Wait the delay amount
    delayMicroseconds(delayAmount);

    getLimitSwitchValues();

  }

  //Move Y to Home Limit Switch
  while(bitRead(inputRegisterValues,0) == 0){
    
    //Send high pulse
    SHIFTREGISTERSTATUSHIGH |= (1 << YMOTORSTEPBIT);
    updateShiftRegisterStatus(); 

    //Wait the delay amount
    delayMicroseconds(delayAmount);

    //Send low pulse
    SHIFTREGISTERSTATUSHIGH &= ~(1 << YMOTORSTEPBIT);
    updateShiftRegisterStatus();
     
    //Wait the delay amount
    delayMicroseconds(delayAmount);

    getLimitSwitchValues();

  }
  
  
  interrupts();

  delay(200);
  openClaw();
  delay(1500);
  returnHome = false;


//  retractClaw();
//  delay(100);
}

/*
 * Main Program Loop
 */
void loop() {
  if(btnPressed){
    btnPressed = false;
    x = 1023 / 2; //Reset the joystick to center
    y = 1023 / 2; //Reset the joystick to center
    SHIFTREGISTERSTATUSHIGH |= (1 << 0);
    buttonledstatus = true;
    updateShiftRegisterStatus();

    if(playMode == MANUALRTH){
      ManualRTHProcedure();
    }else if(playMode == AUTORTH){
      AutoRTHProcedure();
    }
    
  }else{
    btnPressed = bitRead(inputRegisterValues,4);
    
    //Detect X axis direction changes
    if(x > ((1023 / 2) + 200) && clawMovementDirection != FORWARD){
      clawMovementDirection = FORWARD;
    }else if(x < ((1023 / 2) - 200) && clawMovementDirection != BACK){
      clawMovementDirection = BACK;
    }

    //Detect Y axis direction changes
    if(y > ((1023 / 2) + 200) && clawMovementDirection != RIGHT){
      clawMovementDirection = RIGHT;
    }else if(y < ((1023 / 2) - 200) && clawMovementDirection != LEFT){
      clawMovementDirection = LEFT;
    }
    
    //Write to the LCD Screen
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Vigo's Klaw");
    lcd.setCursor(0,1);

    //What's going on
    if(clawMovementDirection == FORWARD){
      lcd.print("Charging Forward");
    }else if(clawMovementDirection == BACK){
      lcd.print("Retreating Back");
    }else if(clawMovementDirection == RIGHT){
      lcd.print("Going Right");
    }else if(clawMovementDirection == LEFT){
      lcd.print("Going Left");
    }else{
      lcd.print("M'Sheen");
    }
    delay(200);
    blinkButtonLed();
    clawMovementDirection = STOPPED;
  }
}


/*
 * Stepper Drivers Initialization Routine
 * Sets the correct outputs for the 74HC595 shift register so that the 
 * stepper motor drivers have the correct initial input on their pins
 */
void initializeStepperDriver(){
  //  MS1 | MS2 | MS3 | STEP RESOLUTION
  //  =================================
  //   0     0     0  | Full Step
  //   1     0     0  | 1/2  Step
  //   0     1     0  | 1/4  Step
  //   1     1     0  | 1/8  Step
  //   1     1     1  | 1/16 Step

  //  DIR | MOTOR DIRECTION
  //  =====================
  //   1  | Clockwise
  //   0  | Counter-clockwise
  
  uint8_t initial_SHIFTREGISTERSTATUSLOW = 0b00000000;
  //                             ||||||||--MOTYMS3
  //                             |||||||--MOTYMS2
  //                             ||||||--MOTYMS1  
  //                             |||||--MOTXDIR Counter-clockwise
  //                             ||||--MOTXSTEP (Start at 0)
  //                             |||--MOTXMS3 FULLSTEP MODE
  //                             ||--MOTXMS2
  //                             --MOTXMS1  
  
  SHIFTREGISTERSTATUSLOW = initial_SHIFTREGISTERSTATUSLOW; //Set the shift register to the initial state
  
  uint8_t initial_SHIFTREGISTERSTATUSHIGH = 0b00000000;
  //                              ||--MOTYDIR Counter-clockwise
  //                              |--MOTYSTEP (Start at 0)
  
  SHIFTREGISTERSTATUSHIGH = initial_SHIFTREGISTERSTATUSHIGH; //Set the shift register to the initial state
}



/*
 * Updates the 74HC595 output shift register pin status
 */
void updateShiftRegisterStatus(){
  digitalWrite(outputLatchPin,LOW);
  shiftOut(outputDataPin,outputClockPin,LSBFIRST,SHIFTREGISTERSTATUSHIGH);
  shiftOut(outputDataPin,outputClockPin,LSBFIRST,SHIFTREGISTERSTATUSLOW);
  digitalWrite(outputLatchPin,HIGH);
}


/*
 * Updates the 74HC595 output shift register pins status in order to 
 * set the correct input conditions on the stepper motor drivers
 * so that the resolution of the selected stepper motor is the indicated one
 */
void setupStepperResolution(WhichMotor which,StepResolution res){
  //  Set the step resolution
  //  MS1 | MS2 | MS3 | STEP RESOLUTION
  //  =================================
  //   0     0     0  | Full Step
  //   1     0     0  | 1/2  Step
  //   0     1     0  | 1/4  Step
  //   1     1     0  | 1/8  Step
  //   1     1     1  | 1/16 Step
  switch(res){
    case FULL:
      if(which == X){
        SHIFTREGISTERSTATUSLOW &= ~(1 << XMS1BIT); //Clear XMS1
        SHIFTREGISTERSTATUSLOW &= ~(1 << XMS2BIT); //Clear XMS2
        SHIFTREGISTERSTATUSLOW &= ~(1 << XMS3BIT); //Clear XMS3
      }else if(which == Y){
        SHIFTREGISTERSTATUSLOW &= ~(1 << YMS1BIT); //Clear YMS1
        SHIFTREGISTERSTATUSLOW &= ~(1 << YMS2BIT); //Clear YMS2
        SHIFTREGISTERSTATUSLOW &= ~(1 << YMS3BIT); //Clear YMS3
      }else if(which == CLAW){
        SHIFTREGISTERSTATUSHIGH &= ~(1 << CLAWMOTORMS1BIT); //Clear CLAW MS1
        SHIFTREGISTERSTATUSHIGH &= ~(1 << CLAWMOTORMS2BIT); //Clear CLAW MS2
        SHIFTREGISTERSTATUSHIGH &= ~(1 << CLAWMOTORMS3BIT); //Clear CLAW MS3
      }
      break;
    case HALF:
      if(which == X){
        SHIFTREGISTERSTATUSLOW |= (1 << XMS1BIT); //Set XMS1
        SHIFTREGISTERSTATUSLOW &= ~(1 << XMS2BIT); //Clear XMS2
        SHIFTREGISTERSTATUSLOW &= ~(1 << XMS3BIT); //Clear XMS3
      }else if(which == Y){
        SHIFTREGISTERSTATUSLOW |= (1 << YMS1BIT); //Set YMS1
        SHIFTREGISTERSTATUSLOW &= ~(1 << YMS2BIT); //Clear YMS2
        SHIFTREGISTERSTATUSLOW &= ~(1 << YMS3BIT); //Clear YMS3
      }else if(which == CLAW){
        SHIFTREGISTERSTATUSHIGH |= (1 << CLAWMOTORMS1BIT); //Set CLAW MS1
        SHIFTREGISTERSTATUSHIGH &= ~(1 << CLAWMOTORMS2BIT); //Clear CLAW MS2
        SHIFTREGISTERSTATUSHIGH &= ~(1 << CLAWMOTORMS3BIT); //Clear CLAW MS3
      }
      break;
    case QUARTER:
      if(which == X){
        SHIFTREGISTERSTATUSLOW &= ~(1 << XMS1BIT); //Clear XMS1
        SHIFTREGISTERSTATUSLOW |= (1 << XMS2BIT); //Set XMS2
        SHIFTREGISTERSTATUSLOW &= ~(1 << XMS3BIT); //Clear XMS3
      }else if(which == Y){
        SHIFTREGISTERSTATUSLOW &= ~(1 << YMS1BIT); //Clear YMS1
        SHIFTREGISTERSTATUSLOW |= (1 << YMS2BIT); //Set YMS2
        SHIFTREGISTERSTATUSLOW &= ~(1 << YMS3BIT); //Clear YMS3
      }else if(which == CLAW){
        SHIFTREGISTERSTATUSHIGH &= ~(1 << CLAWMOTORMS1BIT); //Clear CLAW MS1
        SHIFTREGISTERSTATUSHIGH |= (1 << CLAWMOTORMS2BIT); //Set CLAW MS2
        SHIFTREGISTERSTATUSHIGH &= ~(1 << CLAWMOTORMS3BIT); //Clear CLAW MS3
      }
      break;
    case EIGTH:
      if(which == X){
        SHIFTREGISTERSTATUSLOW |= (1 << XMS1BIT); //Set XMS1
        SHIFTREGISTERSTATUSLOW |= (1 << XMS2BIT); //Set XMS2
        SHIFTREGISTERSTATUSLOW &= ~(1 << XMS3BIT); //Clear XMS3
      }else if(which == Y){
        SHIFTREGISTERSTATUSLOW |= (1 << YMS1BIT); //Set YMS1
        SHIFTREGISTERSTATUSLOW |= (1 << YMS2BIT); //Set YMS2
        SHIFTREGISTERSTATUSLOW &= ~(1 << YMS3BIT); //Clear YMS2
      }else if(which == CLAW){
        SHIFTREGISTERSTATUSHIGH |= (1 << CLAWMOTORMS1BIT); //Set CLAW MS1
        SHIFTREGISTERSTATUSHIGH |= (1 << CLAWMOTORMS2BIT); //Set CLAW MS2
        SHIFTREGISTERSTATUSHIGH &= ~(1 << CLAWMOTORMS3BIT); //Clear CLAW MS3
      }
      break;
    case SIXTEENTH:
      if(which == X){
        SHIFTREGISTERSTATUSLOW |= (1 << XMS1BIT); //Set XMS1
        SHIFTREGISTERSTATUSLOW |= (1 << XMS2BIT); //Set XMS2
        SHIFTREGISTERSTATUSLOW |= (1 << XMS3BIT); //Set XMS3
      }else if(which == Y){
        SHIFTREGISTERSTATUSLOW |= (1 << YMS1BIT); //Set YMS1
        SHIFTREGISTERSTATUSLOW |= (1 << YMS2BIT); //Set YMS2
        SHIFTREGISTERSTATUSLOW |= (1 << YMS3BIT); //Set YMS3
      }else if(which == CLAW){
        SHIFTREGISTERSTATUSHIGH |= (1 << CLAWMOTORMS1BIT); //Set CLAW MS1
        SHIFTREGISTERSTATUSHIGH |= (1 << CLAWMOTORMS2BIT); //Set CLAW MS2
        SHIFTREGISTERSTATUSHIGH |= (1 << CLAWMOTORMS3BIT); //Set CLAW MS3
      }
      break;
  }
}
