
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "oi.h"
#include "serial.h"

enum bool {false, true};
typedef unsigned char byte;
Serial* serial;

void send_byte(byte b)
{
	serialSend(serial, b);
};

byte get_byte()
{
	byte c;
	
	// if there is no waiting byte, wait
	while ( serialNumBytesWaiting(serial) == 0 )
		usleep(15000);

	serialGetChar(serial, &c);
	return c;
};

void start(byte state)
{
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

/*
 * stops for 15ms
 */
unsigned char get_bump()
{
	send_byte( CmdSensors );
	send_byte( SenBumpDrop ); //bumps and wheel drops

	return get_byte() & BmpBoth; //discard wheel drops
};

int get_wall()
{
	send_byte( CmdSensors );
	send_byte( 27 );

	int value = get_byte();
	value += value << 8;
	value += get_byte();

	return value;
};

/*
 * stops for 15ms
 */
unsigned char get_button()
{
	send_byte( CmdSensors );
	send_byte( SenButton );

	return get_byte();
};

void set_led(byte ledBits, byte pwrLedColor)
{
	send_byte( CmdLeds );
	send_byte( ledBits );
	send_byte( pwrLedColor );
	send_byte( 255 ); // set intensity high
};

/*
Enables robot to drive directly non linearly 
using the sign and value of both velocity values
by moving the wheels at different velocities.
*/
void drive(short leftWheelVelocity, short rightWheelVelocity)
{
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
Enables robot to drive directly forward/backwards 
based on the sign and value of the velocity value
by moving the wheels at the same velocity.
positve wheelVelocity moves robot forward
negative wheelVelocity moves robot in reverse
*/
void linear_drive(short wheelVelocity)
{
	byte low = wheelVelocity; // cast short to byte (discard high byte)
	byte high = wheelVelocity >> 8; // bitwise shift high to low to save high byte
	
	send_byte( CmdDriveWheels );
	send_byte( high );
	send_byte( low );
	send_byte( high );
	send_byte( low );
};

/*
Enables robot to drive directly angularly 
using the sign and value of both the velocity values and the radius
positve wheelVelocity moves robot forward
negative wheelVelocity moves robot in reverse
positve radius turns robot left
negative radius turns robot right
*/
void angular_drive(short wheelVelocity, short radius)
{
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

void playSong()
{
	// song opcode
	send_byte( 140 );

	// set song track to 0
	send_byte( 0 );

	//send length in notes
	send_byte( 8 );

	// compose song
	send_byte( 91 ); // note
	send_byte( 8 ); // note duration

	send_byte( 90 );
	send_byte( 8 );

	send_byte( 99 );
	send_byte( 8 );

	send_byte( 69 );
	send_byte( 8 );

	send_byte( 80 );
	send_byte( 8 );

	send_byte( 88 );
	send_byte( 8 );

	send_byte( 92 );
	send_byte( 8 );

	send_byte( 96 );
	send_byte( 25 );
	
	usleep(100000);
	
	// play song opcode
	send_byte( 141 );

	// select song track to play
	send_byte( 0 );
	
	usleep(5000000);
}



int main(int args, char** argv)
{
	byte bmp = 0;
	byte btn = 0;
	
	// all for tests
	int turns = 4;
	int drive_enabled = 1;
	int turn_enabled = 1;
	
	start(CmdFull); //full mode

	for (int i = 0; i < turns && !btn; i++) {

		// drive straight & stop when bumped
		for (int j = 0; j < 100 && drive_enabled; j++) {

			/*
			if bumped 
			stop and wait 1/10 of a second then check sensor again
			repeat until no bump detected
			*/
			bmp = get_bump();
			while (bmp != 0) {
				angular_drive(0, 0);
				usleep(100000);
				bmp = get_bump();
			}

			/*
			drive straight with a velocity of 100mm per second
			for loop repeats for 10 iterations, each iteration lasts 1 second
			100mm * 10 seconds = 10cm
			*/
			//angular_drive(100, 32768);
			//angular_drive(100, 32767);
			linear_drive(100);

			//if button is pushed end program
			btn = get_button(); // halts 15ms
			if ( btn ) {
				break;
			}

			// sleep for 1/10 seconds, needed to measure distance traveled
			// therefore, for every (10 j) @ 1/10 seconds I've traveled 1 seconds
			// need 10 seconds
			usleep(100000);
		}

		// turn PI / 2 counter-clockwise
		if (turn_enabled) {
			angular_drive(185, 1);
			usleep(1000000);
			angular_drive(0,0);
		}

	}

	// square completed! play sound
	playSong();
		

	send_byte(CmdPwrDwn);
	return 0;
}
