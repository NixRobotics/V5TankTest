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

#define CONTROLLER_DEADBAND 5.0 // Joystick deadband for arcade drive

bool bDisableArm = false; // Stop arm from moving if catch is deployed
int armMotorSpeed = 0;
#define ARM_MAX_RAISE_SPEED 80
#define ARM_MAX_LOWER_SPEED -100
#define ARM_MAX_RAISE_TORQUE 100.0 // percent. We need full torque while raising. TODO: Check if arm has stopped moving
#define ARM_MAX_LOWER_TORQUE 50.0 // percent. Do not need full torque while lowering. TODO: Check if arm has stopped moving
#define ARM_MIN_ANGLE 0.0 // Fully retracted or backstop
#define ARM_MAX_ANGLE -1174.0 // Fully extended or frontstop
#define ARM_MIN_DEADBAND_ANGLE -10.0 // No motor power once close to backstop
#define ARM_MAX_DEADBAND_ANGLE -1164.0 // No motor pwoer once close to frontstop

bool bDisableIntake = false; // No conditions yet
int intakeMotorSpeed = 0;
#define ARM_MAX_COLLECT_SPEED 100
#define ARM_MAX_EJECT_SPEED -100

// Used to detect is intake motor has stopped rotating, if it is we set to hold
bool intakeMotorStart = false;
int intakeMotorStartCount = 0;
#define INTAKE_STALL_SPEED 25
#define INTAKE_STALL_COUNT 80 // Crude timer to see how long intake is not spinning, approx 2sec

#define CATCH_MAX_SPEED 25
#define CATCH_MAX_ANGLE 90.0

// Main loop runrate, approx 25ms
int loopCount = 0;

// Raise Arm
// This is positive direction
void whenControllerL1Pressed() {

  if (bDisableArm) return;

  // The basic algorithm here is to check if the arm is already moving, if it is we stop it
  // Else we move it in the request direction
  // However, we need to check if we are close to an endstop (front or back) if that's the case
  // make sure we don't drive it towards the closest endstop

  if (!ArmMotor.isSpinning()) armMotorSpeed = 0; // Arm may have been stopped due to back/frontstop deadband
  if (armMotorSpeed != 0) armMotorSpeed = 0; // If we are spinning already, stop
  else armMotorSpeed = ARM_MAX_RAISE_SPEED;

  // NOTE!!! Gives position from program start, not absolute to motor
  double pos = ArmMotor.position(vex::degrees);
  // Check if arm is going to move towards an endstop, if not see if we move or hold position
  // If arm was already moving, we stop it
  // We want to be in coast if we within the deadband (10deg) to either endstop
  ArmMotor.setMaxTorque(ARM_MAX_RAISE_TORQUE, vex::percent);
  if (armMotorSpeed != 0 && pos < ARM_MIN_DEADBAND_ANGLE) {
    printf("start spin raise\n");
    ArmMotor.setBrake(hold);
    ArmMotor.setVelocity(armMotorSpeed, vex::percent);
    ArmMotor.spinToPosition(0.0, vex::degrees, false);
  } else if (armMotorSpeed == 0 && (pos < ARM_MIN_DEADBAND_ANGLE || pos > ARM_MAX_DEADBAND_ANGLE)) {
    printf("stop spin\n");
    ArmMotor.stop(hold);
  }

}

// Arm Lower
// This is negative direction
void whenControllerL2Pressed() {

  if (bDisableArm) return;

  // The basic algorithm here is to check if the arm is already moving, if it is we stop it
  // Else we move it in the request direction
  // However, we need to check if we are close to an endstop (front or back) if that's the case
  // make sure we don't drive it towards the closest endstop  

  if (!ArmMotor.isSpinning()) armMotorSpeed = 0; // Arm may have been stopped due to back/frontstop deadband
  if (armMotorSpeed != 0) armMotorSpeed = 0;  // If we are spinning already, stop
  else armMotorSpeed = ARM_MAX_LOWER_SPEED;

  // printf("Set spin to %d\n", armMotorSpeed);
  // NOTE!!! Gives position from program start, not absolute to motor
  double pos = ArmMotor.position(vex::degrees);
  // Check if arm is going to move towards an endstop, if not see if we move or hold position
  // If arm was already moving, we stop it
  // We want to be in coast if we within the deadband (10deg) to either endstop
  ArmMotor.setMaxTorque(ARM_MAX_LOWER_TORQUE, vex::percent);
  if (armMotorSpeed != 0 && pos > ARM_MAX_DEADBAND_ANGLE) {
    printf("start spin lower\n");
    ArmMotor.setBrake(hold);
    ArmMotor.setVelocity(-armMotorSpeed, vex::percent);
    ArmMotor.spinToPosition(ARM_MAX_ANGLE, vex::degrees, false);
  } else if (armMotorSpeed == 0 && (pos < ARM_MIN_DEADBAND_ANGLE || pos > ARM_MAX_DEADBAND_ANGLE)) {
    printf("stop spin\n");
    ArmMotor.stop(hold);
  }

}

// Intake Collect
// This is positive direction
void whenControllerR2Pressed() {

  if (bDisableIntake) return;
  
  // Same as arm ...
  // If intake is already moving, we stop
  // Else we spin in requested direction
  // For collect direction we use hold when the motor is not spinning

  if (intakeMotorSpeed != 0) intakeMotorSpeed = 0;
  else intakeMotorSpeed = ARM_MAX_COLLECT_SPEED;

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

// Intake Eject
// This is negative direction
void whenControllerR1Pressed() {

  if (bDisableIntake) return;

  // Same as arm ...
  // If intake is already moving, we stop
  // Else we spin in requested direction
  // For eject direction we use hold when the motor is not spinning

  if (intakeMotorSpeed != 0) intakeMotorSpeed = 0;
  else intakeMotorSpeed = ARM_MAX_EJECT_SPEED;

  if (intakeMotorSpeed != 0) {
    intakeMotorStart = true;
    intakeMotorStartCount = 0;
    IntakeMotor.setVelocity(intakeMotorSpeed, vex::percent);
    IntakeMotor.spin(forward);
  } else {
    intakeMotorStart = false;
    intakeMotorStartCount = 0;
    IntakeMotor.setVelocity(intakeMotorSpeed, vex::percent);
    IntakeMotor.stop(coast);
  }
}

// All Stop
// Simulates losing power at the end of the match
void whenControllerXPressed() {
  intakeMotorSpeed = 0;
  armMotorSpeed = 0;
  IntakeMotor.stop(coast);
  ArmMotor.stop(coast);
  CatchMotor.stop(coast);
}

// Deploy Catch
// NOTE!!! - Must disable arm once this is done
// Assumes there that catch is roughly horizonal at the start of the program
void whenControllerUpPressed() {
  bDisableArm = true;
  CatchMotor.setVelocity(CATCH_MAX_SPEED, vex::percent);
  CatchMotor.spinToPosition(CATCH_MAX_ANGLE, vex::degrees, false);
}


int main() {
  // Initializing Robot Configuration. DO NOT REMOVE!
  vexcodeInit();

  // Deadband stops the motors when Axis values are close to zero.
  int deadband = CONTROLLER_DEADBAND;
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

  // Install callbacks for buttons
  Controller1.ButtonL1.pressed(whenControllerL1Pressed);
  Controller1.ButtonL2.pressed(whenControllerL2Pressed);

  Controller1.ButtonR1.pressed(whenControllerR1Pressed);
  Controller1.ButtonR2.pressed(whenControllerR2Pressed);

  Controller1.ButtonUp.pressed(whenControllerUpPressed);
  Controller1.ButtonX.pressed(whenControllerXPressed);

  printf("START-- Arm motor pos = %f deg, torque = %f Nm, current = %f A\n", ArmMotor.position(vex::degrees), ArmMotor.torque(vex::Nm), ArmMotor.current(vex::amp));

  // Main loop, run at around 25ms
  while (true) {

    /**/ // Start drive train

    // DriveTrain - simple arcade drive using left joystick
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

    /**/ // End DriveTrain

    // Check to see if intake has stopped spinning for roughly 2sec
    // If it has, then set to hold so we are not fighting against a presumable game piece
    if (intakeMotorSpeed != 0 && intakeMotorStartCount > INTAKE_STALL_COUNT) {
      int currentSpeed = IntakeMotor.velocity(vex::percent);
      if (abs(currentSpeed) < INTAKE_STALL_SPEED) {
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

    // Check if arm is spinning and closing in on and endstop, if so let it coast the rest of the way
    if (armMotorSpeed < 0 && ArmMotor.position(vex::degrees) < ARM_MAX_DEADBAND_ANGLE) {
      printf("stop spin\n");
      armMotorSpeed = 0;
      ArmMotor.stop(coast);
    } else if (armMotorSpeed > 0 && ArmMotor.position(vex::degrees) > ARM_MIN_DEADBAND_ANGLE) {
      printf("stop spin\n");
      armMotorSpeed = 0;
      ArmMotor.stop(coast);
    }

/**/

    if ((loopCount % 80) == 0) {
      printf("Arm motor pos = %f deg, torque = %f Nm, current = %f A\n", ArmMotor.position(vex::degrees), ArmMotor.torque(vex::Nm), ArmMotor.current(vex::amp));
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
