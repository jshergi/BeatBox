//i2c.h contains all of the functions needed for writing to the I2C Reg as well as all necessary
//functions that need are need to display an integer and double value


#ifndef i2c_h_
#define i2c_h_

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

#define REG_DIRA 0x21
#define REG_DIRB 0x81

#define I2C_DEVICE_ADDRESS 0x70

int initI2cBus(char* bus, int address);
void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);
unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr);
int displayInteger (int displayTypeInt);
int displayMode (int mode);
void displayDouble (int displayTypeInt);



#endif