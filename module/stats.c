#include "stats.h"
#include <linux/kfifo.h>
#include <linux/kernel.h>
#include <linux/ktime.h>

#define FIFO_SIZE 64 
static DECLARE_KFIFO(fifo_1m, MEASUREMENT, FIFO_SIZE);

void evict_outdated_in_fifo_1m(time_t);
void calculate_stats_1m(void);

static int *temps_1m;
static int *rhs_1m;
static unsigned short int pins_count;

void stats_init(int temp_1m[], int rh_1m[], unsigned short int _1m_size) {
    INIT_KFIFO(fifo_1m);
    temps_1m = temp_1m;
    rhs_1m = rh_1m;
    pins_count = _1m_size;
}

void stats_update(MEASUREMENT measurement) {
    if (!kfifo_put(&fifo_1m, measurement)) {
       printk(KERN_INFO "Cannot put measurement: fifo full\n");
    }
    evict_outdated_in_fifo_1m(measurement.timestamp);
    calculate_stats_1m();
}

void evict_outdated_in_fifo_1m(time_t current_time) {
    bool continue_eviction = true;
    MEASUREMENT m;
    m.temp = -1;
    m.timestamp = -1;

    while(continue_eviction) {
        continue_eviction = false;
        if (kfifo_peek(&fifo_1m, &m)) {
            if (current_time - m.timestamp > 60) {
                if (kfifo_get(&fifo_1m, &m)) {
                    continue_eviction = true;
                }
            }
        }
    }
}

void calculate_stats_1m() {
    MEASUREMENT measurements[FIFO_SIZE];
    int fifo_1m_length;
    int i;
    int temp_acc[pins_count];
    int rh_acc[pins_count];
    int counts[pins_count];

    memset(temp_acc, 0, sizeof(int)*pins_count);
    memset(rh_acc, 0, sizeof(int)*pins_count);
    memset(counts, 0, sizeof(int)*pins_count);

    fifo_1m_length = kfifo_len(&fifo_1m);
    if (kfifo_out_peek(&fifo_1m, measurements, fifo_1m_length)) {
        for (i = 0; i < fifo_1m_length; i++) {
            temp_acc[measurements[i].pin_index] += measurements[i].temp;
            rh_acc[measurements[i].pin_index] += measurements[i].rh;
            counts[measurements[i].pin_index]++;
        }
        for (i = 0; i < pins_count; i++) {
            temps_1m[i] = (counts[i] == 0) ? 0 : temp_acc[i]/counts[i];
            rhs_1m[i] = (counts[i] == 0) ? 0 : rh_acc[i]/counts[i];
        }
    }
}

