#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/ktime.h>
#include <linux/proc_fs.h>

#include <linux/seq_file.h>
#include <linux/time.h>

enum eState {
	READ_START,
	READ_START_HIGH,
	READ_BIT_START,
	READ_BIT_HIGH,
	READ_BIT_LOW,
	READ_STOP,
};

struct st_inf {
	int t;
	int rh;
};

#define SHORT_DELAY 1
#define DEFAULT_DELAY 2

static int _pins[] = {23, 24, 25, 8};
static int _irqs[ARRAY_SIZE(_pins)];
static int _reads_ok[ARRAY_SIZE(_pins)];
static int _reads_total[ARRAY_SIZE(_pins)];
static struct st_inf _sp[ARRAY_SIZE(_pins)];
static struct st_inf _sns[ARRAY_SIZE(_pins)];
static time_t _timestamps[ARRAY_SIZE(_pins)];

static int _pin = -1;
static int _read_delay = DEFAULT_DELAY; /* in seconds */
static volatile int _read_req = READ_STOP;
static struct task_struct *ts = NULL;
static wait_queue_head_t _queue;
static ktime_t _old;
static volatile int _ulen;
static unsigned char _data[5];

static int proc_fs_show(struct seq_file *m, void *v) {
	long local_time;
	struct tm s_tm;
	int i;

	for (i=0; i<ARRAY_SIZE(_pins); i++)
	{
		if (_sns[i].t < 0) {
            seq_printf(m, "%d_temperature :\t\t-%d.%d C\n", _pins[i], (int) abs(_sns[i].t / 10), (int) abs(_sns[i].t % 10) );
        } else {
            seq_printf(m, "%d_temperature :\t\t%d.%d C\n", _pins[i], _sns[i].t / 10, _sns[i].t % 10 );
        }
		seq_printf(m, "%d_RH          :\t\t%d.%d %%\n", _pins[i], _sns[i].rh / 10, _sns[i].rh%10);

		local_time = (u32)(_timestamps[i] - (sys_tz.tz_minuteswest * 60));
		time_to_tm(local_time, 0, &s_tm);
		seq_printf(m, "%d_date        :\t\t%04ld-%02d-%02d %02d:%02d:%02d\n", _pins[i], s_tm.tm_year + 1900, s_tm.tm_mon + 1, s_tm.tm_mday, (s_tm.tm_hour + 2), s_tm.tm_min, s_tm.tm_sec);
		seq_printf(m, "%d_timestamp   :\t\t%ld\n", _pins[i], _timestamps[i]);
		seq_printf(m, "%d_QUAL        :\t\t%d/%d %d%c\n", _pins[i], _reads_ok[i], _reads_total[i], _reads_ok[i] * 100 / _reads_total[i], '\%');

		seq_printf(m, "\n");
	}

  return 0;
}

static int proc_fs_open(struct inode *inode, struct  file *file) {
  return single_open(file, proc_fs_show, NULL);
}

static const struct file_operations proc_fs_fops = {
  .owner = THIS_MODULE,
  .open = proc_fs_open,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
};

#define CHECK_RET(r) do { \
		if (r != 0) {			\
			return r;		\
		}				\
	} while (0)

/*
 * GPIO ISR
 * State machine for reading the sensor request.
 * Hopefuly the hardware performs some filtering.
 */
static irqreturn_t read_isr(int irq, void *data)
{
	ktime_t now = ktime_get_real();
	static int bit_count, char_count;

	switch (_read_req) {
	case READ_START:
		if (gpio_get_value(_pin) == 0) {
			_read_req = READ_START_HIGH;
		}
		break;
	case READ_START_HIGH:
		if (gpio_get_value(_pin) == 1) {
			_read_req = READ_BIT_START;
		}
		break;
	case READ_BIT_START:
		if (gpio_get_value(_pin) == 0) {
			_read_req = READ_BIT_HIGH;
			bit_count = 7;
			char_count = 0;
			memset(_data, 0, sizeof(_data));
		}
		break;
	case READ_BIT_HIGH:
		if (gpio_get_value(_pin) == 1) {
			_read_req = READ_BIT_LOW;
		}
		break;
	case READ_BIT_LOW:
		if (gpio_get_value(_pin) == 0) {
			_ulen = ktime_us_delta(now, _old);
			if (_ulen > 40) {
				_data[char_count] |= (1 << bit_count);
			}
			if (--bit_count < 0) {
				char_count++;
				bit_count = 7;
			}
			if (char_count == 5) {
				_read_req = READ_STOP;
				wake_up_interruptible(&_queue);
			} else {
				_read_req = READ_BIT_HIGH;
			}
		}
		break;
	case READ_STOP:
	default:
		break;
	}
	_old = now;
	return IRQ_HANDLED;
}

static int start_read(void)
{
	int ret;

	/*
	 * Set pin high and wait for two milliseconds.
	 */
 	ret = gpio_direction_output(_pin, 1);
	CHECK_RET(ret);

	udelay(2000);

	/*
	 * Set pin low and wait for at least 750 us.
	 * Set it high again, then wait for the sensor to put out a low pulse.
	 */
	gpio_set_value(_pin, 0);
	udelay(800);
	gpio_set_value(_pin, 1);

	_read_req = READ_START;

 	ret = gpio_direction_input(_pin);
	CHECK_RET(ret);

	return 0;
}

static int do_read_data(struct st_inf *s)
{
	unsigned char cks = 0;
	int max_u = 100;

 	if (!wait_event_interruptible_timeout(_queue, (_read_req == READ_STOP), max_u)) {
		_read_req = READ_STOP;
		return -1;
	}

	/*
	 * This seems to fail often.
	 * Assuming that sometimes one bit is lost and, if the values are low enough,
	 * the checksum is identical.
	 */
	cks = _data[0] + _data[1] + _data[2] + _data[3];
	if (cks != _data[4]) {
		return -1;
	}

	s->rh = (int) (int16_t)(((uint16_t) _data[0] << 8) | (uint16_t) _data [1]);

	if (_data[2] & 0x80) {
		_data[2] = _data[2] & 0x7f;
		s->t  = -1 * ( (int) (((uint16_t) _data[2] << 8) | (uint16_t) _data [3]) );
	} else {
		s->t  = (int) (((uint16_t) _data[2] << 8) | (uint16_t) _data [3]);
	}

	if (s->rh > 1000 || s->rh < 0 || s->t > 800 || s->t < -400 ) {
		return -1;
	}

	return 0;
}

static int read_thread(void *data)
{
	int local_delay = 0;
	struct st_inf s;
	static int pin_selector_counter = 0;
	struct timeval time;

        while (!kthread_should_stop()) {

		/*
		 * Do not sleep the whole chunk, otherwise if
		 *  the module is removed it will wait for that whole delay.
		 */
		if (local_delay != 0) {
			local_delay--;
			/* ToDo: Find a better interruptible delay implementation */
			wait_event_interruptible_timeout(_queue, 0, HZ);
			continue;
		}

		pin_selector_counter++;

		_pin = _pins[pin_selector_counter % ARRAY_SIZE(_pins)];

		local_delay = _read_delay;

		_reads_total[pin_selector_counter % ARRAY_SIZE(_pins)]++;

		if (start_read() != 0)
                {
			local_delay = SHORT_DELAY;
			continue;
		}

		if (do_read_data(&s) != 0)
		{
			local_delay = SHORT_DELAY; /* Ignore this reading */
		}
		else
                {
			if (_reads_ok[pin_selector_counter % ARRAY_SIZE(_pins)] == 0)
			{
				local_delay = SHORT_DELAY;
				_sns[pin_selector_counter % ARRAY_SIZE(_pins)] = s;
				_sp[pin_selector_counter % ARRAY_SIZE(_pins)] = s;
				_reads_ok[pin_selector_counter % ARRAY_SIZE(_pins)]++ ;
			}
			else
                        {
				if ((s.t - _sp[pin_selector_counter % ARRAY_SIZE(_pins)].t > 50) ||  /* 5 degrees difference */
				    (s.t - _sp[pin_selector_counter % ARRAY_SIZE(_pins)].t < -50) ||
				    (s.rh - _sp[pin_selector_counter % ARRAY_SIZE(_pins)].rh > 100) || /* or 10 RH differene */
				    (s.rh - _sp[pin_selector_counter % ARRAY_SIZE(_pins)].rh < -100))
				{
					/* Ignore this reading */
					local_delay = SHORT_DELAY;
				}
                                else
				{
					_sns[pin_selector_counter % ARRAY_SIZE(_pins)] = s;
					_sp[pin_selector_counter % ARRAY_SIZE(_pins)] = s;
					_reads_ok[pin_selector_counter % ARRAY_SIZE(_pins)]++;
					do_gettimeofday(&time);
					_timestamps[pin_selector_counter % ARRAY_SIZE(_pins)] = time.tv_sec;
				}
			}
		}
        }
        return 0;
}

static int __init multiple_am2301_init(void)
{
	int i;

	printk(KERN_INFO "Init multi-am2301\n");

	init_waitqueue_head(&_queue);

	for (i = 0; i < ARRAY_SIZE(_pins); i++) {
		int ret;

		ret = gpio_request_one(_pins[i], GPIOF_OUT_INIT_HIGH, "AM2301");

		if (ret != 0) {
			printk(KERN_ERR "multi-am2301: Unable to request GPIO%d, err: %d\n", _pins[i], ret);
			return ret;
		}

		_irqs[i] =  gpio_to_irq(_pins[i]);
		if (_irqs[i] < 0) {
			printk(KERN_ERR "multi-am2301: Unable to create IRQ for GPIO%d\n", _pins[i]);
			goto _cleanup_1;

		}

	        ret = request_irq(_irqs[i], read_isr,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				  "read_isr", NULL);
	}

	ts = kthread_create(read_thread, NULL, "multi-am2301");

	if (ts) {
		wake_up_process(ts);
	} else {
		printk(KERN_ERR "multi-am2301: Unable to create thread\n");
		goto _cleanup_2;
	}

	proc_create("multi-am2301", 0, NULL, &proc_fs_fops);

	return 0;

_cleanup_2:
	for (i=0; i < ARRAY_SIZE(_irqs); i++) {
		if (_irqs[i] >= 0) {
			free_irq(_irqs[i], NULL);
		}
	}

_cleanup_1:
	for (i=0; i < ARRAY_SIZE(_pins); i++) {
		gpio_free(_pins[i]);
	}

	return -1;
}

static void __exit multiple_am2301_exit(void)
{
	int i;
	if (ts) {
                kthread_stop(ts);
        }

	for (i=0; i < ARRAY_SIZE(_irqs); i++) {
		if (_irqs[i] >= 0) {
			free_irq(_irqs[i], NULL);
		}
	}

	for (i=0; i < ARRAY_SIZE(_pins); i++) {
	 	(void) gpio_direction_output(_pins[i], 1);
		gpio_free(_pins[i]);
	}

	remove_proc_entry("multi-am2301", NULL);
	printk(KERN_INFO "multi-am2301: exit\n");
}

module_init(multiple_am2301_init);
module_exit(multiple_am2301_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Constantin Petra");
MODULE_AUTHOR("Marcin Pilaczynski");
MODULE_DESCRIPTION("multi AM2301 driver");

