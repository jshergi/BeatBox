
// Note: Generates low latency audio on BeagleBone Black; higher latency found on host.
#include "audioMixer.h"
#include "beatBox.h"
#include "intervalTimer.h"
#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <alloca.h> // needed for mixer


//enum interval_whichInterval NUM_INTERVALS;
static snd_pcm_t *handle;
int beat = 0;

#define DEFAULT_VOLUME 80

#define DEFAULT_BPM 120
#define MAX_BPM 300
#define MIN_BPM 40

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 1
#define SAMPLE_SIZE (sizeof(short)) 			// bytes per sample
// Sample size note: This works for mono files because each sample ("frame') is 1 value.
// If using stereo files then a frame would be two samples.

static unsigned long playbackBufferSize = 0;
static short *playbackBuffer = NULL;




// Currently active (waiting to be played) sound bites
#define MAX_SOUND_BITES 30
typedef struct {
	// A pointer to a previously allocated sound bite (wavedata_t struct).
	// Note that many different sound-bite slots could share the same pointer
	// (overlapping cymbal crashes, for example)
	wavedata_t *pSound;

	// The offset into the pData of pSound. Indicates how much of the
	// sound has already been played (and hence where to start playing next).
	int location;
} playbackSound_t;
static playbackSound_t soundBites[MAX_SOUND_BITES];

// Playback threading
void* playbackThread(void* arg);
static _Bool stopping = false;
static pthread_t playbackThreadId;
static pthread_mutex_t audioMutex = PTHREAD_MUTEX_INITIALIZER;

static int volume = 0;
static int BPM = DEFAULT_BPM;

int AudioMixer_getCurrentBeat()
{
	return beat; 
}

void AudioMixer_getNextBeat()
{
	if (beat < 2){
		beat += 1;
	}
	else{
		beat = 0;
	}
	
	//printf("%d\n", beat);
}

void AudioMixer_init(void)
{
	AudioMixer_setVolume(DEFAULT_VOLUME);

	
    for (int i = 0; i < MAX_SOUND_BITES; i++){
        soundBites[i].pSound = NULL;
        soundBites[i].location = 0;
    }

	// Open the PCM output
	int err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	// Configure parameters of PCM output
	err = snd_pcm_set_params(handle,
			SND_PCM_FORMAT_S16_LE,
			SND_PCM_ACCESS_RW_INTERLEAVED,
			NUM_CHANNELS,
			SAMPLE_RATE,
			1,			// Allow software resampling
			50000);		// 0.05 seconds per buffer
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	// Allocate this software's playback buffer to be the same size as the
	// the hardware's playback buffers for efficient data transfers.
	// ..get info on the hardware buffers:
 	unsigned long unusedBufferSize = 0;
	snd_pcm_get_params(handle, &unusedBufferSize, &playbackBufferSize);
	// ..allocate playback buffer:
	playbackBuffer = malloc(playbackBufferSize * sizeof(*playbackBuffer));

	// Launch playback thread:
	pthread_create(&playbackThreadId, NULL, playbackThread, NULL);
}


// Client code must call AudioMixer_freeWaveFileData to free dynamically allocated data.
void AudioMixer_readWaveFileIntoMemory(char *fileName, wavedata_t *pSound)
{
	assert(pSound);

	// The PCM data in a wave file starts after the header:
	const int PCM_DATA_OFFSET = 44;

	// Open the wave file
	FILE *file = fopen(fileName, "r");
	if (file == NULL) {
		fprintf(stderr, "ERROR: Unable to open file %s.\n", fileName);
		exit(EXIT_FAILURE);
	}

	// Get file size
	fseek(file, 0, SEEK_END);
	int sizeInBytes = ftell(file) - PCM_DATA_OFFSET;
	pSound->numSamples = sizeInBytes / SAMPLE_SIZE;

	// Search to the start of the data in the file
	fseek(file, PCM_DATA_OFFSET, SEEK_SET);

	// Allocate space to hold all PCM data
	pSound->pData = malloc(sizeInBytes);
	if (pSound->pData == 0) {
		fprintf(stderr, "ERROR: Unable to allocate %d bytes for file %s.\n",
				sizeInBytes, fileName);
		exit(EXIT_FAILURE);
	}

	// Read PCM data from wave file into memory
	int samplesRead = fread(pSound->pData, SAMPLE_SIZE, pSound->numSamples, file);
	if (samplesRead != pSound->numSamples) {
		fprintf(stderr, "ERROR: Unable to read %d samples from file %s (read %d).\n",
				pSound->numSamples, fileName, samplesRead);
		exit(EXIT_FAILURE);
	}
}

void AudioMixer_freeWaveFileData(wavedata_t *pSound)
{
	pSound->numSamples = 0;
	free(pSound->pData);
	pSound->pData = NULL;
}

void AudioMixer_queueSound(wavedata_t *pSound)
{
	// Ensure we are only being asked to play "good" sounds:
	assert(pSound->numSamples > 0);
	assert(pSound->pData);

    int incrementing_counter = 0;

    pthread_mutex_lock(&audioMutex);  //#1
    for (int i = 0; i < MAX_SOUND_BITES; i++){
       
        if (soundBites[i].pSound == NULL)
        {
            soundBites[i].pSound = pSound;
            soundBites[i].location = 0; // sound has not been played
            pthread_mutex_unlock(&audioMutex);
            break;
        }

        incrementing_counter++;
    }

    if (incrementing_counter == MAX_SOUND_BITES){
        printf("ERROR: There are no free slots available\n");
    }





}

void AudioMixer_cleanup(void)
{
	printf("Stopping audio...\n");

	// Stop the PCM generation thread
	stopping = true;
	pthread_join(playbackThreadId, NULL);

	// Shutdown the PCM output, allowing any pending sound to play out (drain)
	snd_pcm_drain(handle);
	snd_pcm_close(handle);

	// Free playback buffer
	// (note that any wave files read into wavedata_t records must be freed
	//  in addition to this by calling AudioMixer_freeWaveFileData() on that struct.)
	free(playbackBuffer);
	playbackBuffer = NULL;

	printf("Done stopping audio...\n");
	fflush(stdout);
}


int AudioMixer_getVolume()
{
	// Return the cached volume; good enough unless someone is changing
	// the volume through other means and the cached value is out of date.
	return volume;
}

// Function copied from:
// http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
// Written by user "trenki".
void AudioMixer_setVolume(int newVolume)
{
	// Ensure volume is reasonable; If so, cache it for later getVolume() calls.
	if (newVolume < 0 || newVolume > AUDIOMIXER_MAX_VOLUME) {
		printf("ERROR: Volume must be between 0 and 100.\n");
		return;
	}
	volume = newVolume;

    long min, max;
    snd_mixer_t *mixerHandle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    // const char *selem_name = "PCM";	// For ZEN cape
    const char *selem_name = "Speaker";	// For USB Audio

    snd_mixer_open(&mixerHandle, 0);
    snd_mixer_attach(mixerHandle, card);
    snd_mixer_selem_register(mixerHandle, NULL, NULL);
    snd_mixer_load(mixerHandle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(mixerHandle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(mixerHandle);
}

int  AudioMixer_getBPM()
{
	return BPM;

}

void AudioMixer_setBPM(int newBPM)
{
	if (newBPM < MIN_BPM){
		BPM = MIN_BPM;
	}
	else if (newBPM > MAX_BPM){
		BPM = MAX_BPM;
	}
	else{
		BPM = newBPM;
	}
}

long long timeToWait()
{
	// Multiply by 1000 for delay in ms
	return ((double)60 / AudioMixer_getBPM()/2) *1000; 
}

// Fill the buff array with new PCM values to output.
//    buff: buffer to fill with new PCM data from sound bites.
//    size: the number of *values* to store into buff
static void fillPlaybackBuffer(short *buff, int size)
{
    // #1
    memset(buff, 0, size*sizeof(*buff));

    // #2
    pthread_mutex_lock(&audioMutex);

    // #3
    for (int i = 0; i < MAX_SOUND_BITES; i++){
        if (soundBites[i].pSound != NULL){

			//Steps are recommended below 
            int offset = soundBites[i].location;
            int data = 0; //NOTE TO SELF: originally used short but sound was horrible
            

			for (int j = 0; j < playbackBufferSize; j++){
				if (offset < soundBites[i].pSound->numSamples){
					int offset2 = (int)soundBites[i].pSound->pData[offset];
					data = ((int)buff[j] + offset2);

					//Clipping
					if (data < SHRT_MIN){
						data = SHRT_MIN;
					}
					if (data > SHRT_MAX){
						data = SHRT_MAX;
					}
					buff[j] = (short)data;
					offset++;
				}
			}

            soundBites[i].location = offset;
            if (soundBites[i].location >= soundBites[i].pSound->numSamples){
                soundBites[i].pSound = NULL;
            }
			

        }

    }
    pthread_mutex_unlock(&audioMutex);
	Interval_markInterval(INTERVAL_LOW_LEVEL_AUDIO);
	
	


}




void* playbackThread(void* _arg)
{
	while (!stopping) {
		// Generate next block of audio
		fillPlaybackBuffer(playbackBuffer, playbackBufferSize);

		// Output the audio
		snd_pcm_sframes_t frames = snd_pcm_writei(handle,
				playbackBuffer, playbackBufferSize);

		// Check for (and handle) possible error conditions on output
		if (frames < 0) {
			fprintf(stderr, "AudioMixer: writei() returned %li\n", frames);
			frames = snd_pcm_recover(handle, frames, 1);
		}
		if (frames < 0) {
			fprintf(stderr, "ERROR: Failed writing audio with snd_pcm_writei(): %li\n",
					frames);
			exit(EXIT_FAILURE);
		}
		if (frames > 0 && frames < playbackBufferSize) {
			printf("Short write (expected %li, wrote %li)\n",
					playbackBufferSize, frames);
		}
	}

	return NULL;
}

void lowLevelAudioStats()
{
	Interval_statistics_t lowlevel ;
	Interval_getStatisticsAndClear(INTERVAL_LOW_LEVEL_AUDIO, &lowlevel);
	printf("[%.2lf, %.2lf] avg %.2lf/%d", lowlevel.minIntervalInMs, lowlevel.maxIntervalInMs, lowlevel.avgIntervalInMs, lowlevel.numSamples);
}
