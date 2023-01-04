#include "audioMixer.h"
#include "intervalTimer.h"
#include "beatBox.h"
#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#include <alloca.h>

static void runCommand(char* command){

    // Execute the shell command (output into pipe)
    FILE *pipe = popen(command, "r");

    // Ignore output of the command; but consume it 
    // so we don't get an error when closing the pipe. 
    char buffer[1024];
    while (!feof(pipe) && !ferror(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL){
            break;
        }
        // printf("--> %s", buffer);
    }

    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0){
        perror("Unable to execute command:");
        printf(" command:   %s\n", command);
        printf(" exit code: %d\n", exitCode);
    }

}

void initI2C()
{
    runCommand("config-pin P9_18 i2c");
    runCommand("config-pin P9_17 i2c");

}



int main()
{
    Interval_init();
    initI2C(); // No cleanup necessary
    AudioMixer_init();
    BeatBox_init();
    BeatBox_getNextBeatInit();
    BeatBox_joystickControl();
    
    printf("Enter 'Q' to quit.\n");
    while(true){
        if (toupper(getchar()) == 'Q') {
            break;
        }
    }
    
    BeatBox_joystickCleanup();
    BeatBox_getNextBeatCleanup();
    BeatBox_cleanup();
    AudioMixer_cleanup();
    Interval_cleanup();
}

