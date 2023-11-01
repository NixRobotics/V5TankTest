#include "vex.h"

using namespace vex;
using signature = vision::signature;
using code = vision::code;

// A global instance of brain used for printing to the V5 brain screen
brain Brain;

// VEXcode device constructors
// NOTE: PUTT-PUTT PORT 19 is DEAD - do not use
controller Controller1 = controller(primary);
motor LeftMotor = motor(PORT16, ratio18_1, false);
motor RightMotor = motor(PORT17, ratio18_1, true);
motor IntakeMotor = motor(PORT14, ratio18_1, false);
motor ArmMotor = motor(PORT18, ratio36_1, false);
motor CatchMotor = motor(PORT15, ratio18_1, true);
limit ArmLimit = limit(Brain.ThreeWirePort.A);
limit CatchLimit = limit(Brain.ThreeWirePort.B);

// https://www.vexforum.com/t/vexcode-motor-groups-and-drivetrain-example/69161

// VEXcode generated functions

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 *
 * This should be called at the start of your int main function.
 */
void vexcodeInit(void) {
  // nothing to initialize
  wait(100, msec);
}