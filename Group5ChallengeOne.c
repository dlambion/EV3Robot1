/*
 * Authors: Joshua Neighbarger and Danielle Lambion
 */

#pragma config(Sensor, S1,     LEFT_TOUCH,     sensorEV3_Touch)
#pragma config(Sensor, S2,     RIGHT_TOUCH,    sensorEV3_Touch)
#pragma config(Motor,  motorA,          RIGHT_MOTOR,   tmotorEV3_Large, PIDControl, driveRight, encoder)
#pragma config(Motor,  motorB,          LEFT_MOTOR,    tmotorEV3_Large, PIDControl, driveLeft, encoder)
#pragma config(Motor,  motorC,           ,             tmotorEV3_Large, openLoop)
#pragma config(Motor,  motorD,           ,             tmotorEV3_Large, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

/* The amount of time in ms the motors will move between wander direction is chosen */
#define DRIVE_TIME 200
/* The amount of time in ms the motors will move when backing up */
#define REVERSE_TIME 800
/* The amount of time in ms the motors will move for a set turn before randomness */
#define TURN_TIME 600
/* The amount of time in ms the motors will move when turning around */
#define TURNAROUND_TIME 800
/* The number of times a bumper will be pressed in succession before a tight turn is triggered */
#define PONG_TOLERANCE 4


/* 
 * The system state flags. For readability, they were each separated from a single register. The
 * motorSign determines if the enabled motors (leftMotor, rightMotor) are to be driven in reverse.
 * The leftTouch and rightTouch are if the robot currently detects that either touch sensor is pressed.
 * isWandering determines if the wheels should slow down when making turns or stop if false.
 */

bool motorSign;
bool leftMotor;
bool rightMotor;
bool leftTouch;
bool rightTouch;
bool isWandering;


/* 
 * Task sets the leftTouch and rightTouch flags on if either touch sensor is pressed within 50ms of 
 * each other.
 */
task sensors() {
	while (1) {
		wait1Msec(50);
		leftTouch = getTouchValue(LEFT_TOUCH);
		rightTouch = getTouchValue(RIGHT_TOUCH);
	}
}


/* 
 * Task sends signals to the motors based on how the main thread sets each motor flag. The motorSign 
 * determines if the enabled motors (leftMotor, rightMotor) are to be driven in reverse. The leftTouch 
 * and rightTouch are if the robot currently detects that either touch sensor is pressed. isWandering 
 * determines if the wheels should slow down when making turns or stop if false. If the PONG_TOLERANCE 
 * is met, where the number of times a bumper will be pressed in succession before a tight turn is 
 * triggered, then the speed of the required motor will be set to a lower speed, rather than zero.
 */
task motors() {
	while (1) {
		if (leftMotor && rightMotor) { 
			if (motorSign) {
				/* Reverse */
				setMotorSpeed(LEFT_MOTOR, -100);
				setMotorSpeed(RIGHT_MOTOR, -100);
			} else {
				/* Forward */
				setMotorSpeed(LEFT_MOTOR, 100);
				setMotorSpeed(RIGHT_MOTOR, 100);
			}
		} else if (!leftMotor && !rightMotor) {
			if (motorSign) {
				/* Turn Around */
				setMotorSpeed(LEFT_MOTOR, -100);
				setMotorSpeed(RIGHT_MOTOR, 100);
			} else {
				/* Stop */
				setMotorSpeed(LEFT_MOTOR, 0);
				setMotorSpeed(RIGHT_MOTOR, 0);
			}
		} else { /* Motors are to run at different speeds. */
			/* Set left motor speed */
			if (leftMotor) setMotorSpeed(LEFT_MOTOR, motorSign ? -100 : 100);
			else setMotorSpeed(LEFT_MOTOR, isWandering ? 35 : 0);
			/* Set right motor speed */
			if (rightMotor) setMotorSpeed(RIGHT_MOTOR, motorSign ? -100 : 100);
			else setMotorSpeed(RIGHT_MOTOR, isWandering ? 35 : 0);
		}
	}
}


/* 
 * Biased random turn for when both bumpers are pressed
 */
void turnRandomDirection() {
	float r = random(100) / 100.0 ;
	if (r < 0.3) r = 1;
	else if (r < 0.6) r = 2;
	else if (r < 0.8) r = 3;
	else r = 0;

	switch (r) {
		case 0:
			/* Forward */
			motorSign = false;
			leftMotor = true;
			rightMotor = true;
			break;
		case 1:
			/* Turn Left */
			motorSign = false;
			leftMotor = false;
			rightMotor = true;
			break;
		case 2:
			/* Turn Right */
			motorSign = false;
			leftMotor = true;
			rightMotor = false;
			break;
		case 3:
			/* Turn Around */
			motorSign = true;
			leftMotor = false;
			rightMotor = false;
			break;
	}
}


/* 
 * Biased random turning for wandering
 */
void wanderRandomDirection() {
	float r = random(100) / 100.0 ;
	if (r <= 0.9) r = 0;
	else if (r <= 0.96) r = 1;
	else r = 2;

	switch (r) {
		case 0:
			/* Forward */
			motorSign = false;
			leftMotor = true;
			rightMotor = true;
			break;
		case 1:
			/* Turn Left */
			motorSign = false;
			leftMotor = false;
			rightMotor = true;
			break;
		case 2:
			/* Turn Right */
			motorSign = false;
			leftMotor = true;
			rightMotor = false;
			break;
	}
}


/* 
 * Main task interprets the sensor data and sets the motor-related flags
 */
task main() {
	startTask(sensors);
	startTask(motors);
	resetBumpedValue(LEFT_TOUCH);
	resetBumpedValue(RIGHT_TOUCH);
	
	while (1) {
		if (leftTouch && rightTouch) {
			/* 
			 * If both bumpers are pressed, reset bumped value, play
			 * tone, reverse, stop, and turn random direction.
			 */
			resetBumpedValue(LEFT_TOUCH);
			resetBumpedValue(RIGHT_TOUCH);
			isWandering = false;
			playTone(440, 100);
			/* Reverse */
			motorSign = true;
			leftMotor = true;
			rightMotor = true;
			wait1Msec(REVERSE_TIME);
			/* Stop for 2 seconds */
			motorSign = false;
			leftMotor = false;
			rightMotor = false;
			sleep(2000);
			/* Turn random direction */
			turnRandomDirection();
			if (motorSign) wait1Msec(TURNAROUND_TIME);
			else wait1Msec(TURN_TIME + random(500));
			/* Drive straight */
			motorSign = false;
			leftMotor = true;
			rightMotor = true;
		} else if (getBumpedValue(LEFT_TOUCH) + getBumpedValue(RIGHT_TOUCH) >= PONG_TOLERANCE) {
			/* Sharp turn is triggered from detecting oscillation */
			resetBumpedValue(LEFT_TOUCH);
			resetBumpedValue(RIGHT_TOUCH);
			bool left = leftTouch, right = rightTouch;
			leftMotor = !left;
			rightMotor = !right;
			wait1Msec(TURNAROUND_TIME);
		} else if (leftTouch) {
			/* Left bumper is pressed. Back up and turn right. */
			isWandering = false;
			motorSign = true;
			leftMotor = true;
			rightMotor = true;
			wait1Msec(REVERSE_TIME);
			motorSign = false;
			leftMotor = true;
			rightMotor = false;
			wait1Msec(TURN_TIME + random(500));
			motorSign = false;
			leftMotor = true;
			rightMotor = true;
		} else if (rightTouch) {
			/* Right bumper is pressed. Back up and turn left. */
			isWandering = false;
			motorSign = true;
			leftMotor = true;
			rightMotor = true;
			wait1Msec(REVERSE_TIME);
			motorSign = false;
			leftMotor = false;
			rightMotor = true;
			wait1Msec(TURN_TIME + random(500));
			motorSign = false;
			leftMotor = true;
			rightMotor = true;
		} else {
			/*
			 * No bumpers are pressed. Robot will wander and poll 
			 * for random turn every DRIVE_TIME ms.
			 */
			isWandering = true;
			wanderRandomDirection();
			wait1Msec(DRIVE_TIME);
		}
	}
}
