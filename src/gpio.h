#ifndef __GPIO_H__
#define __GPIO_H__

#include <native/mutex.h>
#include <native/task.h>
#include <native/intr.h>

/** Device memory mappings **/
#define GPIO_MAX_CHANNELS 2 /* Device max number of channels */
#define GPIO_CHANNEL1 0x00 /* Offset of channel IO from base */
#define GPIO_CHANNEL2 0x02 /* Offset of channel IO from base */
#define GPIO_TRISTATE_OFFSET 1 /* Offset from any channel */
/* Global Interrupt Enable Register
   RW 
   Offset from base 

    Desc: The Global Interrupt Enable Register provides the master enable/disable for the interrupt output to the processor.
    This is a single bit read/write register
*/
#define GPIO_GIE_OFFSET (0x11c>>2) 
#define GPIO_GIE_MASK 0x80000000
/* Interrupt Enable Register 
   RW
   Offset from base 
   Desc: Individual interrupt enable for each channel
*/
#define GPIO_IER_OFFSET (0x128>>2)
#define GPIO_IER_MASK 0xc0000000
#define GPIO_IER_CHANNEL1_MASK 0x1
#define GPIO_IER_CHANNEL2_MASK 0x2
/* Interrupt Status Register 
   R/Toggle-On-Write
   Offset from base 
   Desc: Individual interrupt status for each channel
*/
#define GPIO_ISR_OFFSET (0x120>>2)
#define GPIO_ISR_MASK GPIO_IER_MASK
#define GPIO_ISR_CHANNEL1_MASK GPIO_IER_CHANNEL1_MASK
#define GPIO_ISR_CHANNEL2_MASK GPIO_IER_CHANNEL2_MASK

/** Driver parameters **/
#define GPIO_FLAGS_INPUT 0x01	
#define GPIO_FLAGS_OUTPUT 0x02 
#define GPIO_IRQ_CHANNEL1 0x04
#define GPIO_IRQ_CHANNEL2 0x08

struct struct_gpio
{	
	unsigned long base_add; // Physical memory address where to map the device  (beginning)
	unsigned long end_add; // Physical memory address where to map the device (end)
	volatile int* vadd; // virtual address where device is mapped
	char flags; // IO/Allow Interrupt
	unsigned num_of_channels; // Number of activated channels ( either 1 or 2 )
	unsigned tristate[GPIO_MAX_CHANNELS]; // Previously assigned value (for muxed IO)
	unsigned value[GPIO_MAX_CHANNELS]; // Previously assigned value (for muxed IO)
	RT_MUTEX mutex; // Mutex for accessing IO
	RT_TASK interrupt_task; // Task from which the ISR is spawned
	RT_INTR intr_desc; // Interrupt pointer
	void (*isr)(void*); // ISR for both channels
};

typedef struct struct_gpio GPIO; 

int gpio_write(GPIO* gpio, unsigned mask, unsigned shift, unsigned offset, unsigned value);

int gpio_read(GPIO* gpio, unsigned mask, unsigned shift, unsigned offset, unsigned* ret);

/* Quick functions for IRQ handling inside the device */
int gpio_irq_enable_global(GPIO* gpio);

int gpio_irq_disable_global(GPIO* gpio);

/* By default assumes n = 1 */
int gpio_irq_enable_channel(GPIO* gpio,int n);

/* By default assumes n = 1 */
int gpio_irq_disble_channel(GPIO* gpio,int n);

int gpio_irq_isr_checkandtoggle_channel(GPIO* gpio,int n, unsigned* ret );

//so far only for 1channel gpios
int gpio_init(GPIO* gpio, 
	      long unsigned int base_add, 
	      long unsigned int end_add, 
	      int num_of_channels, 
	      char flags, 
	      unsigned irqno,
	      void (*fisr)(void*),
	      int irq_prio);

int gpio_clean(GPIO* gpio);

#endif
