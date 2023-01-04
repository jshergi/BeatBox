#ifndef _INTERVAL_TIMER_H_
#define _INTERVAL_TIMER_H_

// Module to record and report interval times
//     Written by Brian Fraser
// Usage:
//  1. Define your Interval_whichInterval enums of interest
//     (Ensure NUM_INTERVALS is last)
//  2. Call Interval_markInterval() periodically to mark each
//     occurance of the interval.
//  3. Call getStatisticsAndClear() to get the statistics for
//     an interval of interest. Calling this will clear the 
//     data collected for this interval (but not others).

// Maximum number of timestamps to record for a given interval.
#define MAX_TIMESTAMPS (1024*4)

enum Interval_whichInterval {
    INTERVAL_LOW_LEVEL_AUDIO,
    INTERVAL_BEAT_BOX,
    NUM_INTERVALS
};

typedef struct {
    int numSamples;
    double minIntervalInMs;
    double maxIntervalInMs;
    double avgIntervalInMs;
} Interval_statistics_t;

void Interval_init(void);
void Interval_cleanup(void);

void Interval_markInterval(enum Interval_whichInterval whichInterval);
void Interval_getStatisticsAndClear(
    enum Interval_whichInterval whichInterval,
    Interval_statistics_t *pStats
);

#endif