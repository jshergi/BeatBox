#include "audioMixer.h"
#include "beatBox.h"
#include "joystick.h"
#include "i2c.h"
#include "intervalTimer.h"
#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <alloca.h>

static pthread_t idBeat;
static pthread_t idNext;
static pthread_t idJoystick;

static wavedata_t beats[3];
static _Bool stopping = false;

static void sleepForMs(long long delayInMs)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoSeconds = delayNs % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoSeconds};
    nanosleep(&reqDelay, (struct timespec *)NULL);
}

//GPIO
static int readFromFileToScreen(char *fileName)
{
    FILE *pFile = fopen(fileName, "r");
    if (pFile == NULL)
    {
        printf("ERROR: Unable to open file (%s) for read\n", fileName);
        exit(-1);
    }

    // Read string (line)
    const int MAX_LENGTH = 1024;
    char buff[MAX_LENGTH];
    fgets(buff, MAX_LENGTH, pFile);

    // Close
    fclose(pFile);
    return atoi(buff);
    // printf("Read: '%s\n", buff);
}

static void BeatBox_rockDrumBeat()
{
    
    // Hi-hat, Base
    AudioMixer_queueSound(&beats[1]);
    AudioMixer_queueSound(&beats[0]);
    sleepForMs(timeToWait());
    Interval_markInterval(INTERVAL_BEAT_BOX);

    // Hi-Hat
    AudioMixer_queueSound(&beats[1]);
    sleepForMs(timeToWait());
    Interval_markInterval(INTERVAL_BEAT_BOX);

    // Hi-Hat, Snare
    AudioMixer_queueSound(&beats[1]);
    AudioMixer_queueSound(&beats[2]);
    sleepForMs(timeToWait());
    Interval_markInterval(INTERVAL_BEAT_BOX);

    // Hi-Hat
    AudioMixer_queueSound(&beats[1]);
    sleepForMs(timeToWait());
    Interval_markInterval(INTERVAL_BEAT_BOX);
    
}

static void BeatBox_custom()
{

    AudioMixer_queueSound(&beats[1]);

    AudioMixer_queueSound(&beats[2]);
    AudioMixer_queueSound(&beats[1]);
    sleepForMs(timeToWait());
    Interval_markInterval(INTERVAL_BEAT_BOX);

    AudioMixer_queueSound(&beats[1]);
    AudioMixer_queueSound(&beats[2]);
    AudioMixer_queueSound(&beats[1]);

    sleepForMs(timeToWait());
    Interval_markInterval(INTERVAL_BEAT_BOX);

    AudioMixer_queueSound(&beats[0]);
    sleepForMs(timeToWait());
    Interval_markInterval(INTERVAL_BEAT_BOX);
}

static void BeatBox_noSound()
{
    return;
}

static void textDisplay()
{

    int mode = AudioMixer_getCurrentBeat();
    int volume = AudioMixer_getVolume();
    int bpm = AudioMixer_getBPM();
    printf("M%d, %dbpm vol:%d   Low", mode, bpm, volume);
    lowLevelAudioStats();

    Interval_statistics_t beatbox;
    Interval_getStatisticsAndClear(INTERVAL_BEAT_BOX, &beatbox);
    printf(" Beat[%.2lf, %.2lf] avg %.2lf/%d\n", beatbox.minIntervalInMs, beatbox.maxIntervalInMs, beatbox.avgIntervalInMs, beatbox.numSamples);
    // printf("\n");
}

void *joystickControl(void *_)
{
    while (!stopping)
    {
        double xValue = Joystick_X();
        double yValue = Joystick_Y();

        // UP
        if (xValue > 0.5 && yValue < 0.5 && yValue > -0.5){
            AudioMixer_setVolume(AudioMixer_getVolume() + 5);
            displayInteger(AudioMixer_getVolume());
            textDisplay();
            sleepForMs(1000);
        }

        // DOWN
        else if (xValue < -0.5 && yValue < 0.5 && yValue > -0.5){
            AudioMixer_setVolume(AudioMixer_getVolume() - 5);
            displayInteger(AudioMixer_getVolume());
            textDisplay();
            sleepForMs(1000);
        }

        // RIGHT
        else if (yValue > 0.5 && xValue < 0.5 && xValue > -0.5){
            AudioMixer_setBPM(AudioMixer_getBPM() + 5);
            displayDouble(AudioMixer_getBPM());
            textDisplay();
            sleepForMs(1000);
        }

        // LEFT
        else if (yValue < -0.5 && xValue < 0.5 && xValue > -0.5){
            AudioMixer_setBPM(AudioMixer_getBPM() - 5);
            displayDouble(AudioMixer_getBPM());
            textDisplay();
            sleepForMs(1000);
        }

        else{
            displayMode(AudioMixer_getCurrentBeat());
            textDisplay();
            sleepForMs(1000);
        }
    }
    return NULL;
}

static int prevGrayButtonState = 0;
static int currentGrayButtonState = 0;

static int prevRedButtonState = 0;
static int currentRedButtonState = 0;

static int prevYellowButtonState = 0;
static int currentYellowButtonState = 0;

static int prevGreenButtonState = 0;
static int currentGreenButtonState = 0;

void *nextBeatThread(void *_)

{

    while (!stopping){
        prevGrayButtonState = currentGrayButtonState;
        prevRedButtonState = currentRedButtonState;
        prevYellowButtonState = currentYellowButtonState;
        prevGreenButtonState = currentGreenButtonState;

        currentGrayButtonState = readFromFileToScreen(GRAY_BUTTON_VALUE_FILE);
        currentRedButtonState = readFromFileToScreen(RED_BUTTON_VALUE_FILE);
        currentYellowButtonState = readFromFileToScreen(YELLOW_BUTTON_VALUE_FILE);
        currentGreenButtonState = readFromFileToScreen(GREEN_BUTTON_VALUE_FILE);

        if (currentGrayButtonState == 1 && prevGrayButtonState == 0){
            AudioMixer_getNextBeat();
            // break;
        }
        if (currentRedButtonState == 1 && prevRedButtonState == 0){
            BeatBox_noSound();
            wavedata_t red;
            AudioMixer_readWaveFileIntoMemory(BASE_DRUM, &red);
            AudioMixer_queueSound(&red);
        }
        if (currentYellowButtonState == 1 && prevYellowButtonState == 0){
            BeatBox_noSound();
            wavedata_t yellow;
            AudioMixer_readWaveFileIntoMemory(SNARE, &yellow);
            AudioMixer_queueSound(&yellow);
        }
        if (currentGreenButtonState == 1 && prevGreenButtonState == 0){
            BeatBox_noSound();
            wavedata_t green;
            AudioMixer_readWaveFileIntoMemory(HI_HAT, &green);
            AudioMixer_queueSound(&green);
        }


    }
    return NULL;
}

void *beatBoxThread(void *_)
{
    while (!stopping)
    {

        
        if (AudioMixer_getCurrentBeat() == 0)
        {
            BeatBox_rockDrumBeat();
        }
        if (AudioMixer_getCurrentBeat() == 1)
        {
            BeatBox_custom();
        }
        if (AudioMixer_getCurrentBeat() == 2)
        {
            BeatBox_noSound();
        }
    }
    return NULL;
}


void BeatBox_init()
{

    wavedata_t base;
    AudioMixer_readWaveFileIntoMemory(BASE_DRUM, &base);

    wavedata_t hihat;
    AudioMixer_readWaveFileIntoMemory(SNARE, &hihat);

    wavedata_t snare;
    AudioMixer_readWaveFileIntoMemory(HI_HAT, &snare);

    beats[0] = base;
    beats[1] = hihat;
    beats[2] = snare;

    pthread_create(&idBeat, NULL, &beatBoxThread, NULL);
}

void BeatBox_getNextBeatInit()
{
    pthread_create(&idNext, NULL, &nextBeatThread, NULL);
}

void BeatBox_joystickControl()
{
    pthread_create(&idJoystick, NULL, &joystickControl, NULL);
}

void BeatBox_cleanup()
{
    stopping = true;
    pthread_join(idBeat, NULL);
}

void BeatBox_getNextBeatCleanup()
{
    stopping = true;
    pthread_join(idNext, NULL);
}

void BeatBox_joystickCleanup()
{
    stopping = true;
    pthread_join(idJoystick, NULL);
}

