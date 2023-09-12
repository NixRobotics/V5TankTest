/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       VEX                                                       */
/*    Created:      Wed Sep 25 2019                                           */
/*    Description:  Left Arcade                                               */
/*    This example allows you to control the V5 Clawbot using the left        */
/*    joystick. Adjust the deadband value for more accurate movements.        */
/*----------------------------------------------------------------------------*/

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller
// LeftMotor            motor         1
// RightMotor           motor         10
// ClawMotor            motor         3
// ArmMotor             motor         8
// ---- END VEXCODE CONFIGURED DEVICES ----

#include "vex.h"

using namespace vex;

int armMotorSpeed = 0;
int intakeMotorSpeed = 0;
bool intakeMotorStart = false;
int intakeMotorStartCount = 0;
int loopCount = 0;

void whenControllerL1Pressed() {

  if (!ArmMotor.isSpinning()) armMotorSpeed = 0;
  armMotorSpeed += 80;
  if (armMotorSpeed > 80) armMotorSpeed = 0;
  double pos = ArmMotor.position(vex::degrees);
  if (armMotorSpeed != 0 && pos < -10.0) {
    printf("start spin up\n");
    ArmMotor.setBrake(hold);
    ArmMotor.setVelocity(armMotorSpeed, vex::percent);
    ArmMotor.spinToPosition(0.0, vex::degrees, false);
  } else if (armMotorSpeed == 0 && (pos < -10.0 || pos > -1164.0)) {
    printf("stop spin\n");
    ArmMotor.stop(hold);
  }

  // ArmMotor.spin(forward);
}

void whenControllerL2Pressed() {

  if (!ArmMotor.isSpinning()) armMotorSpeed = 0;
  armMotorSpeed -= 80;
  if (armMotorSpeed < -80) armMotorSpeed = 0;
  // printf("Set spin to %d\n", armMotorSpeed);
  double pos = ArmMotor.position(vex::degrees);
  if (armMotorSpeed != 0 && pos > -1164.0) {
    printf("start spin down\n");
    ArmMotor.setBrake(hold);
    ArmMotor.setVelocity(-armMotorSpeed, vex::percent);
    ArmMotor.spinToPosition(-1174.0, vex::degrees, false);
  } else if (armMotorSpeed == 0 && (pos < -10.0 || pos > -1164.0)) {
    printf("stop spin\n");
    ArmMotor.stop(hold);
  }
  // ArmMotor.spin(reverse);
}

void whenControllerR2Pressed() {

  intakeMotorSpeed += 100;
  if (intakeMotorSpeed > 100) intakeMotorSpeed = 0;

  if (intakeMotorSpeed != 0) {
    intakeMotorStart = true;
    intakeMotorStartCount = 0;
    IntakeMotor.setVelocity(intakeMotorSpeed, vex::percent);
    IntakeMotor.spin(forward);
  } else {
    intakeMotorStart = false;
    intakeMotorStartCount = 0;
    IntakeMotor.setVelocity(intakeMotorSpeed, vex::percent);
    IntakeMotor.stop(hold);
  }

}

void whenControllerR1Pressed() {

  intakeMotorSpeed -= 100;
  if (intakeMotorSpeed < -100) intakeMotorSpeed = 0;

  if (intakeMotorSpeed != 0) {
    intakeMotorStart = true;
    intakeMotorStartCount = 0;
    IntakeMotor.setVelocity(-intakeMotorSpeed, vex::percent);
    IntakeMotor.spin(reverse);
  } else {
    intakeMotorStart = false;
    intakeMotorStartCount = 0;
    IntakeMotor.setVelocity(-intakeMotorSpeed, vex::percent);
    IntakeMotor.stop(coast);
  }
}

void whenControllerXPressed() {
  intakeMotorSpeed = 0;
  armMotorSpeed = 0;
  IntakeMotor.stop(coast);
  ArmMotor.stop(coast);
  CatchMotor.stop(coast);
}

void whenControllerUpPressed() {
  CatchMotor.setVelocity(25, vex::percent);
  CatchMotor.spinToPosition(90.0, vex::degrees, false);
}


int main() {
  // Initializing Robot Configuration. DO NOT REMOVE!
  vexcodeInit();

  // Deadband stops the motors when Axis values are close to zero.
  int deadband = 5;
  int intakeHaltCount = 0;
  double armMaxTorque = 0.0;

  printf("hello vex world\n");
  Brain.Screen.print("## PUTT-PUTT ##");
  Brain.Screen.newLine();

  Brain.Screen.print("Velocity: %.2f", IntakeMotor.velocity(percent));
  Brain.Screen.newLine();
 
  ArmMotor.setStopping(coast);
  IntakeMotor.setStopping(coast);
  CatchMotor.setStopping(hold);

  // IntakeMotor.setMaxTorque(100, vex::percent);

  Controller1.ButtonL1.pressed(whenControllerL1Pressed);
  Controller1.ButtonL2.pressed(whenControllerL2Pressed);

  Controller1.ButtonR1.pressed(whenControllerR1Pressed);
  Controller1.ButtonR2.pressed(whenControllerR2Pressed);

  Controller1.ButtonUp.pressed(whenControllerUpPressed);
  Controller1.ButtonX.pressed(whenControllerXPressed);

  printf("Arm motor pos = %f\n", ArmMotor.position(vex::degrees));

  while (true) {
    // Get the velocity percentage of the left motor. (Axis3 + Axis4)
    int leftMotorSpeed =
        Controller1.Axis3.position() + Controller1.Axis4.position();
    // Get the velocity percentage of the right motor. (Axis3 - Axis4)
    int rightMotorSpeed =
        Controller1.Axis3.position() - Controller1.Axis4.position();

    // Set the speed of the left motor. If the value is less than the deadband,
    // set it to zero.
    if (abs(leftMotorSpeed) < deadband) {
      // Set the speed to zero.
      LeftMotor.setVelocity(0, percent);
    } else {
      // Set the speed to leftMotorSpeed
      LeftMotor.setVelocity(leftMotorSpeed, percent);
    }

    // Set the speed of the right motor. If the value is less than the deadband,
    // set it to zero.
    if (abs(rightMotorSpeed) < deadband) {
      // Set the speed to zero
      RightMotor.setVelocity(0, percent);
    } else {
      // Set the speed to rightMotorSpeed
      RightMotor.setVelocity(rightMotorSpeed, percent);
    }

    // Spin both motors in the forward direction.
    LeftMotor.spin(forward);
    RightMotor.spin(forward);

/**/
    if (intakeMotorSpeed != 0 && intakeMotorStartCount > 80) {
      int currentSpeed = IntakeMotor.velocity(vex::percent);
      if (abs(currentSpeed) < 25) {
        IntakeMotor.setVelocity(0, vex::percent);
        IntakeMotor.stop(hold);
        intakeMotorSpeed = 0;

        intakeHaltCount++;
        Brain.Screen.clearLine(2);
        Brain.Screen.setCursor(2 , 0);
        Brain.Screen.print("stop intake activated %d", intakeHaltCount);
        Brain.Screen.newLine();
     }
    }
    else if (intakeMotorSpeed != 0) {
      intakeMotorStartCount++;
    }

    if (armMotorSpeed < 0 && ArmMotor.position(vex::degrees) < -1164.0) {
      printf("stop spin\n");
      armMotorSpeed = 0;
      ArmMotor.stop(coast);
    } else if (armMotorSpeed > 0 && ArmMotor.position(vex::degrees) > -10.0) {
      printf("stop spin\n");
      armMotorSpeed = 0;
      ArmMotor.stop(coast);
    }

/**/

    if ((loopCount % 80) == 0) {
      printf("Arm motor pos = %f\n", ArmMotor.position(vex::degrees));
    }

    double armTorque = ArmMotor.torque(vex::Nm);
    if (fabs(armTorque) > fabs(armMaxTorque)) {
      armMaxTorque = armTorque;
      Brain.Screen.clearLine(3);
      Brain.Screen.setCursor(3 , 0);
      Brain.Screen.print("max arm torque %lfNm", armMaxTorque);
      Brain.Screen.newLine();
    }

    wait(25, msec);
    loopCount++;
  }
}
