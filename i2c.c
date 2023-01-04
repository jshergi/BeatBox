#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <time.h>
#include "i2c.h"


int initI2cBus(char* bus, int address)
{
	int i2cFileDesc = open(bus, O_RDWR);
	if (i2cFileDesc < 0) {
		printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
		perror("Error is:");
		exit(-1);
	}

	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
	if (result < 0) {
		perror("Unable to set I2C device to slave address.");
		exit(-1);
	}
	return i2cFileDesc;
}

void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
{
	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDesc, buff, 2);
	if (res != 2) {
		perror("Unable to write i2c register");
		exit(-1);
	}
}

unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr)
{
	// To read a register, must first write the address
	int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
	if (res != sizeof(regAddr)) {
		perror("Unable to write i2c register.");
		exit(-1);
	}

	// Now read the value and return it
	char value = 0;
	res = read(i2cFileDesc, &value, sizeof(value));
	if (res != sizeof(value)) {
		perror("Unable to read i2c register");
		exit(-1);
	}
	return value;
}

static int* getDigits (int digitNumber){
	int* nums = malloc(7*sizeof(int));
	switch(digitNumber){
		case 0: 
			nums[0] = 0x02; nums[1] = 0x05; nums[2] = 0x05; nums[3] = 0x05; nums[4] = 0x05; nums[5] = 0x05; nums[6] = 0x02;
			break;
		case 1: 
			nums[0] = 0x06; nums[1] = 0x02; nums[2] = 0x02; nums[3] = 0x02; nums[4] = 0x02; nums[5] = 0x02; nums[6] = 0x07;
			break;
		case 2: 
			nums[0] = 0x07; nums[1] = 0x01; nums[2] = 0x01; nums[3] = 0x07; nums[4] = 0x04; nums[5] = 0x04; nums[6] = 0x07;
			break; 
		case 3: 
			nums[0] = 0x07; nums[1] = 0x01; nums[2] = 0x01; nums[3] = 0x07; nums[4] = 0x01; nums[5] = 0x01; nums[6] = 0x07;
			break;
		case 4: 
			nums[0] = 0x05;	nums[1] = 0x05; nums[2] = 0x05; nums[3] = 0x07; nums[4] = 0x01; nums[5] = 0x01; nums[6] = 0x01;
			break;
		case 5: 
			nums[0] = 0x07; nums[1] = 0x04; nums[2] = 0x04; nums[3] = 0x07; nums[4] = 0x01; nums[5] = 0x01; nums[6] = 0x07;
			break;
		case 6: 
			nums[0] = 0x07; nums[1] = 0x04; nums[2] = 0x04; nums[3] = 0x07; nums[4] = 0x05; nums[5] = 0x05; nums[6] = 0x07;
			break;
		case 7: 
			nums[0] = 0x07; nums[1] = 0x01; nums[2] = 0x01; nums[3] = 0x01; nums[4] = 0x01; nums[5] = 0x01; nums[6] = 0x01;
			break;
		case 8:
			nums[0] = 0x07; nums[1] = 0x05; nums[2] = 0x05; nums[3] = 0x07; nums[4] = 0x05; nums[5] = 0x05; nums[6] = 0x07;
			break;
		case 9:
			nums[0] = 0x07; nums[1] = 0x05; nums[2] = 0x05; nums[3] = 0x07; nums[4] = 0x01; nums[5] = 0x01; nums[6] = 0x01;
			break;
		case 10: 
			nums[0] = 0x05; nums[1] = 0x07; nums[2] = 0x05; nums[3] = 0x05; nums[4] = 0x05; nums[5] = 0x05; nums[6] = 0x05; 
	}
	return nums; 
}

int displayInteger (int displayTypeInt)
{
	if (displayTypeInt >= 100){
		displayTypeInt = 99;
	}
	if (displayTypeInt < 0){
		displayTypeInt = 0;
	}
	int typeIntLeft = displayTypeInt / 10;
	int* LDigit = getDigits(typeIntLeft); 

	int typeIntRight = displayTypeInt % 10;
	int* RDigit = getDigits(typeIntRight);
	
	int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
	
	writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
	writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);

	writeI2cReg(i2cFileDesc, 0x0E, (LDigit[0]<<4) + RDigit[0]);
	writeI2cReg(i2cFileDesc, 0x0C, (LDigit[1]<<4) + RDigit[1]);
	writeI2cReg(i2cFileDesc, 0x0A, (LDigit[2]<<4) + RDigit[2]);
	writeI2cReg(i2cFileDesc, 0x08, (LDigit[3]<<4) + RDigit[3]);
	writeI2cReg(i2cFileDesc, 0x06, (LDigit[4]<<4) + RDigit[4]);
	writeI2cReg(i2cFileDesc, 0x04, (LDigit[5]<<4 )+ RDigit[5]);
	writeI2cReg(i2cFileDesc, 0x02, (LDigit[6]<<4) + RDigit[6]);
	writeI2cReg(i2cFileDesc, 0x00, 0x00);
	
	free(LDigit);
	free(RDigit);
	
	close(i2cFileDesc);
	return 0;

}

void displayDouble (int displayTypeInt)
{
	//Rounding Logic
	double displayValue = (double)displayTypeInt / 60.0; // 60 seconds in a minute
	int typeIntLeft = (int)displayValue;

	double x = 100*displayValue;
	int y = (int)x;
	int y2 = y - 100*typeIntLeft;
	int typeIntRight = y2 / 10;
	int z = typeIntRight * 10;
	int y3 = y2 - z;
	if (y3 >= 5){
		typeIntRight += 1;
	}

	//End of Logic
	
	int* LDigit = getDigits(typeIntLeft);
	int* RDigit = getDigits(typeIntRight);
	
	int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
	
	writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
	writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);

	writeI2cReg(i2cFileDesc, 0x0E, (LDigit[0]<<4) + RDigit[0]);
	writeI2cReg(i2cFileDesc, 0x0C, (LDigit[1]<<4) + RDigit[1]);
	writeI2cReg(i2cFileDesc, 0x0A, (LDigit[2]<<4) + RDigit[2]);
	writeI2cReg(i2cFileDesc, 0x08, (LDigit[3]<<4) + RDigit[3]);
	writeI2cReg(i2cFileDesc, 0x06, (LDigit[4]<<4) + RDigit[4]);
	writeI2cReg(i2cFileDesc, 0x04, (LDigit[5]<<4 )+ RDigit[5]);
	writeI2cReg(i2cFileDesc, 0x02, (LDigit[6]<<4) + RDigit[6]);
	writeI2cReg(i2cFileDesc, 0x00, 0x08);
	
	free(LDigit);
	free(RDigit);
	
	close(i2cFileDesc);



}


int displayMode(int mode)
{
	int* LDigit = getDigits(10);
	int* RDigit; 

	if (mode == 0){
		RDigit = getDigits(0);
	}

	else if (mode == 1){
		RDigit = getDigits(1);
	}

	else if (mode == 2){
		RDigit = getDigits(2);
	}

	int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
	
	writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
	writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);

	writeI2cReg(i2cFileDesc, 0x0E, (LDigit[0]<<4) + RDigit[0]);
	writeI2cReg(i2cFileDesc, 0x0C, (LDigit[1]<<4) + RDigit[1]);
	writeI2cReg(i2cFileDesc, 0x0A, (LDigit[2]<<4) + RDigit[2]);
	writeI2cReg(i2cFileDesc, 0x08, (LDigit[3]<<4) + RDigit[3]);
	writeI2cReg(i2cFileDesc, 0x06, (LDigit[4]<<4) + RDigit[4]);
	writeI2cReg(i2cFileDesc, 0x04, (LDigit[5]<<4 )+ RDigit[5]);
	writeI2cReg(i2cFileDesc, 0x02, (LDigit[6]<<4) + RDigit[6]);
	writeI2cReg(i2cFileDesc, 0x00, 0x00);
	
	free(LDigit);
	free(RDigit);
	
	close(i2cFileDesc);
	return 0;

}



