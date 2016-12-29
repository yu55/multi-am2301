#ifndef STATS_H_
#define STATS_H_

#include <linux/ktime.h>

typedef struct MEASUREMENT {
    int temp;
    time_t timestamp;
} MEASUREMENT;

void stats_init(void);
void stats_update(MEASUREMENT measurement);
int stats_1m(void);

#endif 
