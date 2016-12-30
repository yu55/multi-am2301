#include "stats.h"
#include <linux/kfifo.h>
#include <linux/kernel.h>
#include <linux/ktime.h>

#define FIFO_SIZE 64
static DECLARE_KFIFO(fifo_1m, MEASUREMENT, FIFO_SIZE);

void evict_outdated_in_fifo_1m(time_t);
void calculate_stats_1m(void);

static int *temps_1m;
static unsigned short int pins_count;

void stats_init(int temp_1m[], unsigned short int temp_1m_size) {
    INIT_KFIFO(fifo_1m);
    temps_1m = temp_1m;
    pins_count = temp_1m_size;
}

void stats_update(MEASUREMENT measurement) {
    if (!kfifo_put(&fifo_1m, measurement)) {
       printk(KERN_INFO "Cannot put measurement: fifo full\n");
    }
    evict_outdated_in_fifo_1m(measurement.timestamp);
    calculate_stats_1m();
}

void evict_outdated_in_fifo_1m(time_t current_time) {
    bool continue_validation = true;
    MEASUREMENT m;
    m.temp = -1;
    m.timestamp = -1;
    printk(KERN_INFO "queue len: %u\n", kfifo_len(&fifo_1m));

    while(continue_validation) {
        continue_validation = false;
        if (kfifo_peek(&fifo_1m, &m)) {
            printk(KERN_INFO "peek: %d,%d,%ld\n", m.pin_index, m.temp, m.timestamp);

            if (current_time - m.timestamp > 60) {
                if (kfifo_get(&fifo_1m, &m)) {
                    printk(KERN_INFO "get: %d,%d,%ld\n", m.pin_index, m.temp, m.timestamp);
                    continue_validation = true;
                }
            }
        }
    }
}

void calculate_stats_1m() {
    MEASUREMENT measurements[FIFO_SIZE];
    int fifo_1m_length;
    int i;
    int accumulators[pins_count];
    int counts[pins_count];

    memset(accumulators, 0, sizeof(int)*pins_count);
    memset(counts, 0, sizeof(int)*pins_count);

    fifo_1m_length = kfifo_len(&fifo_1m);
    if (kfifo_out_peek(&fifo_1m, measurements, fifo_1m_length)) {
        for (i = 0; i < fifo_1m_length; i++) {
            accumulators[measurements[i].pin_index] += measurements[i].temp;
            counts[measurements[i].pin_index]++;
        }
        for (i = 0; i < pins_count; i++) {
            temps_1m[i] = (counts[i] == 0) ? 0 : accumulators[i]/counts[i];
            printk(KERN_INFO "stats_1m=%d (%d/%d)\n", ((counts[i] == 0) ? 0 : accumulators[i]/counts[i]), accumulators[i], counts[i]);
        }
    }
}

int stats_1m() {
   return 321;
}

