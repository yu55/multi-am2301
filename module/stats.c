#include "stats.h"
#include <linux/kfifo.h>
#include <linux/kernel.h>
#include <linux/ktime.h>

#define FIFO_SIZE 1024
static DECLARE_KFIFO(fifo_1m, MEASUREMENT, FIFO_SIZE);

void evict_outdated_in_fifo_1m(time_t);

void stats_init() {
    INIT_KFIFO(fifo_1m);
}

void stats_update(MEASUREMENT measurement) {
    if (!kfifo_put(&fifo_1m, measurement)) {
       printk(KERN_INFO "Cannot put measurement: fifo full\n");
    }
    evict_outdated_in_fifo_1m(measurement.timestamp);
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
            printk(KERN_INFO "peek: %d,%ld\n", m.temp, m.timestamp);

            if (current_time - m.timestamp > 60) {
                if (kfifo_get(&fifo_1m, &m)) {
                    printk(KERN_INFO "get: %d,%ld\n", m.temp, m.timestamp);
                    continue_validation = true;
                }
            }
        }
    }

}

