#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "intervalTimer.h"

// Written by Brian Fraser



// Data collected
typedef struct {
    // Store the timestamp samples each time we mark an interval.
    long timestampCount;
    long long timestampsInNs[MAX_TIMESTAMPS];

    // Used for recording the interval between analysis periods.
    long long prevTimestampInNs;
} timestamps_t;
static timestamps_t s_intervalData[NUM_INTERVALS];

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static bool s_initialized = false;


// Prototypes
static void updateStats(
    timestamps_t *pData, 
    Interval_statistics_t *pStats
);
static long long getTimeInNanoS(void);


void Interval_init(void)
{
    memset(s_intervalData, 0, sizeof(s_intervalData[0]) * NUM_INTERVALS);
    s_initialized = true;
}
void Interval_cleanup(void)
{
    // nothing
    s_initialized = false;
}

void Interval_markInterval(enum Interval_whichInterval whichInterval)
{
    assert (whichInterval >= 0 && whichInterval < NUM_INTERVALS);
    assert (s_initialized);

    timestamps_t *pData = &s_intervalData[whichInterval];
    pthread_mutex_lock(&lock);
    {
        if (pData->timestampCount < MAX_TIMESTAMPS) {
            pData->timestampsInNs[pData->timestampCount] = getTimeInNanoS();
            pData->timestampCount++;
        } else {
            printf("WARNING: No sample space for interval collection on %d\n", whichInterval);
        }
    }
    pthread_mutex_unlock(&lock);
}

void Interval_getStatisticsAndClear(
    enum Interval_whichInterval whichInterval,
    Interval_statistics_t *pStats
)
{
    assert (whichInterval >= 0 && whichInterval < NUM_INTERVALS);
    assert (s_initialized);
    timestamps_t *pData = &s_intervalData[whichInterval];
    pthread_mutex_lock(&lock);
    {
        // Compute stats
        updateStats(pData, pStats);

        // Update the "previous" sample (if we have any)
        if (pData->timestampCount > 0) {
            pData->prevTimestampInNs = pData->timestampsInNs[pData->timestampCount - 1];
        }

        // Clear
        pData->timestampCount = 0;
    }
    pthread_mutex_unlock(&lock);
}

static void updateStats(
    timestamps_t *pData, 
    Interval_statistics_t *pStats
)
{
    long long prevInNs = pData->prevTimestampInNs;

    // Handle startup (no previous sample)
    if (prevInNs == 0) {
        prevInNs = pData->timestampsInNs[0];
    }
    
    // Find min/max/sum time delta between consecutive samples
    long long sumDeltasNs = 0;
    long long minNs = 0;
    long long maxNs = 0;
    for (int i = 0; i < pData->timestampCount; i++) {
        long long thisTime = pData->timestampsInNs[i];
        long long deltaNs = thisTime - prevInNs;
        sumDeltasNs += deltaNs;

        if (i == 0 || deltaNs < minNs) {
            minNs = deltaNs;
        }
        if (i == 0 || deltaNs > maxNs) {
            maxNs = deltaNs;
        }

        prevInNs = thisTime;
    }

    long long avgNs = 0;
    if (pData->timestampCount > 0) {
        avgNs = sumDeltasNs / pData->timestampCount;
    } 

    // Save stats
    #define MS_PER_NS (1000*1000.0)
    pStats->minIntervalInMs = minNs / MS_PER_NS;
    pStats->maxIntervalInMs = maxNs / MS_PER_NS;
    pStats->avgIntervalInMs = avgNs / MS_PER_NS;
    pStats->numSamples = pData->timestampCount;
}





// Timing function
static long long getTimeInNanoS(void) 
{
    struct timespec spec;
    clock_gettime(CLOCK_BOOTTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec + seconds * 1000*1000*1000;
	assert(nanoSeconds > 0);
    
    static long long lastTimeHack = 0;
    assert(nanoSeconds > lastTimeHack);
    lastTimeHack = nanoSeconds;

    return nanoSeconds;
}
