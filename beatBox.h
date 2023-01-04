// Play beat: rockBeat, custom or no beat
#ifndef BEATBOX_H
#define BEATBOX_H

#define GRAY_BUTTON_VALUE_FILE "/sys/class/gpio/gpio47/value"
#define RED_BUTTON_VALUE_FILE "/sys/class/gpio/gpio46/value"
#define YELLOW_BUTTON_VALUE_FILE "/sys/class/gpio/gpio27/value"
#define GREEN_BUTTON_VALUE_FILE "/sys/class/gpio/gpio65/value"




void BeatBox_init();
void BeatBox_cleanup();

void BeatBox_getNextBeatInit();
void BeatBox_getNextBeatCleanup();

void BeatBox_joystickControl();
void BeatBox_joystickCleanup();

#endif