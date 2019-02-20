//https://github.com/Reinbert/pca9685/blob/master/examples/servo.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "thruster-control.h"
#include "pca9685.h"

#define PIN_BASE 300
#define MAX_PWM 4096
#define HERTZ 50
#define MIN_PWM 0
//#define SERVOMIN 

//zero/nutral 1.5ms
//left 1ms
//right 2ms

/*
need to calibrate servo find the Servomin/max at 0 and 180
question about turning? -? calcticks how long to get right turn value
goal value is what? amouunt of speed due to 
how far forward/backward/sideways -> pwm value
*/

//Calculate the number of ticks the signal should be high for the required about of time
int calcTicks(float impulseMs, int hertz)
{
	float cycleMs = 1000.0f / hertz;
	return (int)(MAX_PWM * impulseMs / cycleMs + 0.5f);
}

/**
 * input is [-1..1]
 * output is [min..max]
 */
float map(float input, float min, float max)
{
	return (input * max) + (1 - input) * min;
}


/**
 * This entire program is devoted to a single thruster, as specified by the value passed to argv[argc].
 * This allows thruster control to be asynchronous and makes spawning new thruter programs pretty easy.
 */
int main(int argc, char* argv[]) {
    if(argc != 2) {
        perror("Wrong amount of arguments!\n");
        exit(1);
    }

    populate_whichami(argv[argc]);
    int fd = pca9586Setup(PIN_BASE, Whichami.data_send /*address for the thruster */, HERTZ);
    if(Whichami.data_source == -1) {
        perror("Didn't recognize that thruster\n");
        exit(2);
    }

    //initalizes the I2C system - use I2C detect to find the give device identifier
    //need to upate the fd's to reflect the identifies
    if (wiringPiI2CSetup(fd) == -1){
        sprintf(string, "Failed to set up. (thruster %i)\n", Whichami.data_send);
        perror(string);
    }

    /* This should be done in the master to reset before we use it
    reset all output
    pca9685PWMReset(Whichami.fd);
    */

    //set servo to neutral position (90 degrees, at 1.5 milliseconds)
    float millis = 1.5;
    int tick = calcTicks(millis, HERTZ);
    pwmWrite(Whichami.fd, tick);
    delay(100);

    while(true) {

        int thruster_goal_value = comms_get_int(Whichami.data_source);
        int error = do_thruster_movement(thruster_goal_value);

        //error is returned as 0 if there is an issue
        if(error == 0) {
            char string[75];
            sprintf(string, "Catastrophic failure of some kind, probably. (thruster %i)\n", Whichami.data_send);
            perror(string);
        }
    }
}
 
/* should be in the master file

 void pca9685PWMReset(int fd){
     wiringPiI2CWriteReg16(fd, LEDALLL_ON_L, 0x0);
     wiringPiI2CWriteReg16(fd, LEDALLL_ON_L + 2, 0x1000);
 }
*/

/**
 * Does the thruster movement. The value provided gets sent to Whichami.data_send.
 * @param goalval The goal value.
 * @return true on success, false on fail.
 */
//PWM max = 4095
int do_thruster_movement(double goalval)
    // goal value= percent pressed forward on joystick... speed
    //.h file that takes the goalval and translates into a pwm for speed

    //error = 0 when it fails and 1 when it works
    int error = 0;
    if ( -1 < goalval && goalval < 1)
        double pwm;
        if(goalval > 0){ //goes "forward"
            pwm = (goalval*4095);
	        millis = map(goalval, MIN_PWM, MAX_PWM);
	        tick = calcTicks(millis, HERTZ)
            wiringPiI2CWrite(Whichami.fd, tick);
            //may need to check that it actually wrote the correct value for troubleshooting
            error = 1;
        }
        else{  //goes "backward"
            pwm = (goalval*4095*(-1));
		    //how do we make the motor spin backwards???
		    //need to reverse the power(look at old code)
	        millis = map((goalval8*(-1)), MIN_PWM, MAX_PWM);
	        tick = calcTicks(millis, HERTZ)
            wiringPiI2CWrite(Whichami.fd, tick);
            //may need to check that it actua
	        //this is not going to make it spin backwards
            mwiringPiI2CWrite(Whichami.fd, pwm);
            //may need to add in reading
            error =  1;
        }
    else{
        error = 0;
    }

    return error;
}



/**
 * Populates the 'Whichami' struct.
 */
void populate_whichami(char* input) {
    strcpy(Whichami.name, input);

    if(!strcmp(input, "T_H_FRONTLEFT")) {
        Whichami.data_source = PORT_T_H_FRONTLEFT;
        Whichami.data_send   = T_H_FRONTLEFT;
        //fd = 5; //need to change to actual values when electrical gives them to us
        return;
    }

    if(!strcmp(input, "T_H_FRONTRIGHT")) {
        Whichami.data_source = PORT_T_H_FRONTRIGHT;
        Whichami.data_send   = T_H_FRONTRIGHT;
        //fd = 6; //need to change to actual values when electrical gives them to us
        return;
    }

    if(!strcmp(input, "T_H_BACKLEFT")) {
        Whichami.data_source = PORT_T_H_BACKLEFT;
        Whichami.data_send   = T_H_BACKLEFT;
        //fd = 7; //need to change to actual values when electrical gives them to us
        return;
    }

    if(!strcmp(input, "T_H_BACKRIGHT")) {
        Whichami.data_source = PORT_T_H_BACKRIGHT;
        Whichami.data_send   = T_H_BACKRIGHT;
        //fd = 8; //need to change to actual values when electrical gives them to us
        return;
    }

    if(!strcmp(input, "T_V_LEFT")) {
        Whichami.data_source = PORT_T_V_LEFT;
        Whichami.data_send   = T_V_LEFT;
        //fd = 4; //need to change to actual values when electrical gives them to us
        return;
    }

    if(!strcmp(input, "T_V_RIGHT")) {
        Whichami.data_source = PORT_T_V_RIGHT;
        Whichami.data_send   = T_V_RIGHT;
        //fd = 2; //need to change to actual values when electrical gives them to us
        return;
    }

    if(!strcmp(input, "T_V_FRONT")) {
        Whichami.data_source = PORT_T_V_FRONT;
        Whichami.data_send   = T_V_FRONT;
        //fd = 1; //need to change to actual values when electrical gives them to us
        return;
    }

    if(!strcmp(input, "T_V_BACK")) {
        Whichami.data_source = PORT_T_V_BACK;
        Whichami.data_send   = T_V_BACK;
        //fd = 3; //need to change to actual values when electrical gives them to us
        return;
    }

    // If we got here, there was no match... populate -1 so main can handle it.
    Whichami.data_source = -1;
    Whichami.data_send   = -1;
}
