
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>

#include "oi.h"
#include "serial.h"



// Global variables
enum bool {false, true};
typedef unsigned char byte;
Serial* serial;



void send_byte(byte b) {

	serialSend(serial, b);

};

byte get_byte() {

	byte c;

	// if there is no waiting byte, wait
	while ( serialNumBytesWaiting(serial) == 0 )
		usleep(15000);

	serialGetChar(serial, &c);
	return c;

};

void start(byte state) {

	// allocate memory for Serial struct
	serial = (Serial*)malloc(sizeof(Serial));

	// ensure serial is closed from any previous execution
	serialClose(serial);

	// constant B115200 comes from termios.h
	serialOpen(serial, "/dev/ttyUSB0", B115200, false);
	send_byte( CmdStart );	// Send Start
	send_byte( state );	// Send state

	// turn off any motors
	send_byte( CmdMotors );
	send_byte( 0 );
	send_byte( 0 );
	send_byte( 0 );

	// clear out any bites from previous runs
	while( serialNumBytesWaiting(serial) > 0 )
		get_byte();

};

// stops for 15ms
unsigned char get_bump() {

	send_byte( CmdSensors );
	send_byte( SenBumpDrop ); //bumps and wheel drops

	return get_byte() & BmpBoth; //discard wheel drops

};

/*
Returns byte detailing which prox sensors have detected obstacles.
For example, a wall
*/
byte wall_detected() {

	send_byte( CmdSensors );
	send_byte( 45 );
	return get_byte();

}

unsigned int get_wall() {

	send_byte( CmdSensors );
	send_byte( 51 ); // light bumper

	unsigned int value = get_byte();
	value += value << 8;
	value += get_byte();

	return value;

};

int get_angle() {

	// Potential problem [The value returned must be divided by 0.324056 to get degrees]

	send_byte( CmdSensors );
	send_byte( 20 );

	int value = get_byte();
	value += value << 8;
	value += get_byte();

	return value;

};

// stops for 15ms
unsigned char get_button() {

	send_byte( CmdSensors );
	send_byte( SenButton );

	return get_byte();

};

/*
Enables robot to drive directly non linearly
using the sign and value of both velocity values
by moving the wheels at different velocities.
*/
void drive(short leftWheelVelocity, short rightWheelVelocity) {

	byte left_low = leftWheelVelocity; // cast short to byte (discard high byte)
	byte left_high = leftWheelVelocity >> 8; // bitwise shift high to low to save high byte

	byte right_low = rightWheelVelocity;
	byte right_high = rightWheelVelocity >> 8;

	send_byte( CmdDriveWheels );
	send_byte( right_high );
	send_byte( right_low );
	send_byte( left_high );
	send_byte( left_low );

};

/*
Enables robot to drive directly angularly
using the sign and value of both the velocity values and the radius
positve wheelVelocity moves robot forward
negative wheelVelocity moves robot in reverse
positve radius turns robot left
negative radius turns robot right
*/
void angular_drive(short wheelVelocity, short radius) {

	byte wheel_low = wheelVelocity; // cast short to byte (discard high byte)
	byte wheel_high = wheelVelocity >> 8; // bitwise shift high to low to save high byte

	byte radius_low = radius;
	byte radius_high = radius >> 8;

	send_byte( CmdDrive );
	send_byte( wheel_high );
	send_byte( wheel_low );
	send_byte( radius_high );
	send_byte( radius_low );

};

/*
Robot will drive straight until bump is detected.
Bump is assumed to be the wall.
*/
void find_wall(int enabled) {

	//previous get_bump() != BmpBoth
	while (enabled && !(get_bump() != 0) && !get_button()) {
		drive(50, 50);
		usleep(100000);
	}

}

/*
Rotate until all prox. sensors
are not detecting anything.
*/
void find_open_space() {

	// If non-zero, one of six sensors detect signal
	while (wall_detected() != 0) {
		drive(-50, 50);
		usleep(100000);
	}

	// Stop rotating
	drive(0, 0);

}

/*
Rotate until only right
prox sensor is detecting something
*/
void find_obstacle() {

	while (wall_detected() != 32) {
		drive(-50, 50);
		usleep(100000);
	}

	// Stop rotating
	drive(0, 0);

}

/*
Robot will rotate counterclockwise until wall sensor detects wall.
1023 means robot is on the wall
*/
unsigned int align(int enabled) {

	byte btn = 0;

	// Find wall
	unsigned int wall = get_wall();
	while (enabled && wall < 50 && !btn) {
		drive(-50, 50);
		printf("%u \n", wall);
		wall = get_wall();
		usleep(100000);

		btn = get_button();
		btn = get_button();
		if (!btn) {
			drive(0, 0);
			break;
		}
	}

	// Stop
	drive(0, 0);

	// Return wall distance
	return wall - 100;

}

/*
Used for testing how get_wall() return values change
based on the color/distance of the wall
*Note wall in lab (color = white, distance ~ 0) returns [1480 - 1500]
*/
void test_wall_sensor(int enabled) {

	while (enabled && !get_button()) {
		printf("%u \n", get_wall());
		usleep(100000);
	}

}

void wall_drive(int enabled, unsigned int referenceDistance) {

	// Brown wall has a refDistance = 500;

	// Store button
	byte btn = 0;
	byte bmp = 0;

	// Store error
	double error_p = 0;
	double error_i = 0;
	//float error_d = 0;
	//float error_prev = 0;

	// Store error weight
	double k_p = 0.1;
	double k_i = 0.01;
	//float k_d = 0.001;

	short leftWheelVelocity = 100;
	short rightWheelVelocity = 100;
	unsigned int refDistance = referenceDistance;
	unsigned int measuredDistance;

	// If enabled and bmp not detected, drive along wall
	while (enabled && !btn) {

		// If BmpBoth, realign
		bmp = get_bump();
		if (bmp == BmpBoth) {
			find_obstacle();
			refDistance = align(1);
		}

		// If BmpRight, move around obstacle
		if (bmp == BmpRight) {

			/*
			int xPosition = 0;
			int yPosition = 0;
			double theta = 0;

			get_angle(); // clean garbage values
			int angle = get_angle();
			find_obstacle();
			angle = get_angle();
			*/

		}

		// Read wall sensor
		measuredDistance = get_wall();

		// Calculate error
		error_p = refDistance - measuredDistance;

		// Calculate accumulated error
		error_i = error_i + error_p;

		// Derivative omitted
		//error_d = error_p - error_prev;

		// Store current error for time (t + 1)
		//error_prev = error_p;

		// Calculate error
		double error = k_p * error_p + k_i * error_i;

		/*
		// Check for convex vertex
		if (error_p == refDistance) {

			// Store current wheel speeds
			short tempLeft = leftWheelVelocity;
			short tempRight = rightWheelVelocity;

			// Rotate

		}
		*/

		// Set wheel speed values
		rightWheelVelocity -= error;
		leftWheelVelocity +=  error;

		// Execute speeds
		drive(leftWheelVelocity, rightWheelVelocity);

		// Display values
		printf("Wall: %u\n", measuredDistance);
		printf("Error_p: %f\n", error_p);
		printf("Weighted Error_p: %f\n", k_p * error_p);
		printf("Error_i: %f\n", error_i);
		printf("Weighted Error_i: %f\n", k_i * error_i);
		printf("Left: %d\n", leftWheelVelocity);
		printf("Right: %d\n", rightWheelVelocity);

		// Stop, if clean button is pressed
		btn = get_button();
		if (!btn) {
			drive(0, 0);
			break;
		}

		// Sleep for a tenth of second
		usleep(100000);

	}
	
	drive(0, 0);

}

void crashNBurn() {

	find_open_space();

	angular_drive(50, 100);
	unsigned int distance = align(1);
	wall_drive(1, distance);

}

int main(int args, char** argv) {

	//full mode
	start(CmdFull);

	// Goto wall
	find_wall(1);

	// Align with wall
	unsigned int distance = align(1);

	// Drive along wall
	wall_drive(1, distance);

	// Test wall sensor values
	test_wall_sensor(0);

	// Power down
	send_byte(CmdPwrDwn);

	return 0;

}
