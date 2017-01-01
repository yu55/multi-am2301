#ifndef STATS_H_
#define STATS_H_

#include <linux/ktime.h>

typedef struct MEASUREMENT {
    unsigned short int pin_index;
    short int temp;
    unsigned short int rh;
    time_t timestamp;
} MEASUREMENT;

void stats_init(int temp_1m[], int rh_1m[], unsigned short int _1m_size);
void stats_update(MEASUREMENT measurement);

#endif 
