//joystick.h contains the functions necessary for reading the A2D files as well as 
//outputing a corresponeding voltage value in the range of -1 and 1 V


#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

//module scales values from -1 to 1 V

#define A2D_FILE_VOLTAGE2  "/sys/bus/iio/devices/iio:device0/in_voltage2_raw"
#define A2D_FILE_VOLTAGE3  "/sys/bus/iio/devices/iio:device0/in_voltage3_raw"

#define A2D_VOLTAGE_REF_V  1.0
#define A2D_MAX_READING    4095

double Joystick_X();
double Joystick_Y();
int Joystick_readX();
int Joystick_readY();


#endif


