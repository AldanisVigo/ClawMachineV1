#ifndef _STEPPER_H_
#define _STEPPER_H_

//X Axis Motor 
#define XMOTORDIRBIT 3
#define XMOTORSTEPBIT 4
#define XMS3BIT 5
#define XMS2BIT 6
#define XMS1BIT 7
#define XMOTORSTEPANGLE 1.8
#define XSTEPSPERREV (360/XMOTORSTEPANGLE)

//Y Axis Motor
#define YMOTORDIRBIT 6
#define YMOTORSTEPBIT 7 
#define YMS3BIT 0
#define YMS2BIT 1
#define YMS1BIT 2
#define YMOTORSTEPANGLE 1.8
#define YSTEPSPERREV (360/YMOTORSTEPANGLE)

//Claw Motor
#define CLAWMOTORDIRBIT 1
#define CLAWMOTORSTEPBIT 2
#define CLAWMOTORMS3BIT 3
#define CLAWMOTORMS2BIT 4
#define CLAWMOTORMS1BIT 5
#define CLAWMOTORSTEPANGLE 1.8
#define CLAWSTEPSPERREV (360 / CLAWMOTORSTEPANGLE)

enum MovementDirection{
  FORWARD,
  BACK,
  LEFT,
  RIGHT,
  STOPPED 
};

MovementDirection clawMovementDirection = STOPPED;

enum ClawStatus{
  DROPPING,
  GRABBING
};

ClawStatus clawStatus = GRABBING;

enum StepResolution{
  FULL,
  HALF,
  QUARTER,
  EIGTH,
  SIXTEENTH
};

enum StepperDirection{
  COUNTERCLOCKWISE,
  CLOCKWISE
};

enum StepperSpeed{
  SLOW,
  MEDIUM,
  FAST
};

enum WhichMotor{
  X,
  Y,
  CLAW
};

bool moving = false;
WhichMotor mtr;
StepperSpeed spd;
StepperDirection dir;


#endif
