#ifndef STATS_H_
#define STATS_H_

#include <linux/ktime.h>

typedef struct MEASUREMENT {
    unsigned short int pin_index;
    int temp;
    time_t timestamp;
} MEASUREMENT;

void stats_init(int temp_1m[], unsigned short int temp_1m_size);
void stats_update(MEASUREMENT measurement);
int stats_1m(void);

#endif 
