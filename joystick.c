#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "joystick.h"


FILE* openVoltageFile(char* voltageFile, char* type)
{
	FILE *f = fopen(voltageFile, type);
	if (!f) {
		printf("ERROR: Unable to open voltage input file. Cape loaded?\n");
		printf("       Check /boot/uEnv.txt for correct options.\n");
		exit(-1);
	}

	return f;

}

int Joystick_readX()
{
	FILE *f = openVoltageFile(A2D_FILE_VOLTAGE2, "r");
	int a2dReadingX = 0;
	int itemsReadX = fscanf(f, "%d", &a2dReadingX);
	if (itemsReadX <= 0) {
		printf("ERROR: Unable to read values from voltage input file.\n");
		exit(-1);
	}

	// Close file
	fclose(f);

	return a2dReadingX;
}

int Joystick_readY()
{
	FILE *f = openVoltageFile(A2D_FILE_VOLTAGE3, "r");
	int a2dReadingY = 0;
	int itemsReadY = fscanf(f, "%d", &a2dReadingY);
	if (itemsReadY <= 0) {
		printf("ERROR: Unable to read values from voltage input file.\n");
		exit(-1);
	}

	// Close file
	fclose(f);

	return a2dReadingY;
}

double Joystick_X()
{
    double voltageX; 
    int sign; 
    int readingX = Joystick_readX();
    if (readingX > (A2D_MAX_READING/2)){
        sign= -1;
        voltageX = (((double)readingX - (double)(A2D_MAX_READING/2))/ A2D_MAX_READING) * A2D_VOLTAGE_REF_V;
        voltageX = voltageX* sign *2; 
        
    }
    else {
        sign = 1;
        voltageX = (((double)readingX - (double)(A2D_MAX_READING/2))/ A2D_MAX_READING) * A2D_VOLTAGE_REF_V;
        voltageX = voltageX* sign *-2;

    }

    return voltageX;
}

double Joystick_Y()
{
    int sign; 
    double voltageY; 
    int readingY = Joystick_readY();
    if (readingY > (A2D_MAX_READING/2)){
        sign= -1;
        voltageY = (((double)readingY - (double)(A2D_MAX_READING/2))/ A2D_MAX_READING) * A2D_VOLTAGE_REF_V;
        voltageY = voltageY* sign *2; 
        
    }
    else {
        sign = 1;
        voltageY = (((double)readingY - (double)(A2D_MAX_READING/2))/ A2D_MAX_READING) * A2D_VOLTAGE_REF_V;
        voltageY = voltageY* sign *-2; 
    }
    return voltageY;
    

    
}

