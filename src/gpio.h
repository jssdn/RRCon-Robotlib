#ifndef __GPIO_H__
#define __GPIO_H__

#include <native/mutex.h>
#include <native/task.h>
#include <native/intr.h>

#define GPIO_TRISTATE_OFFSET 1 
#define GPIO_FLAGS_INPUT 0x01	
#define GPIO_FLAGS_OUTPUT 0x02 

struct struct_gpio
{
	volatile int* vadd;
	unsigned long base_add; 
	unsigned long end_add;
	char flags; // IO/Allow Interrupt
	unsigned tristate; 
	RT_MUTEX mutex;
	RT_TASK interrupt_task;
	RT_INTR intr_desc;
	void (*isr)(void*); // ISR
};

typedef struct struct_gpio GPIO; 

int gpio_write(GPIO* gpio, unsigned mask, unsigned offset, unsigned value);
int gpio_read(GPIO* gpio, unsigned mask, unsigned offset, unsigned* ret);

//so far only for 1channel gpios
int gpio_init(GPIO* gpio, 
	      long unsigned int base_add, 
	      long unsigned int end_add, 
	      unsigned tristate, 
	      char flags, 
	      unsigned irqno,
	      void (*isr)(void*),
	      int irq_prio);

int gpio_clean(GPIO* gpio);

#endif
