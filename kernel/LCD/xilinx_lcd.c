/*
 *  Xilinx ML403 16x2 LCD Driver ( hd44780 compatible ) 
 *
 *  Jorge Sánchez de Nova  - jssdn@kth.se
 *  
 *  Licensed under GPL ver2 or over. 
 *
 */

/*
 *  Driver details: 
 *
 *   This driver works with GPIO IP Core from Xilinx. 
 *   The interface should be as following: 
 *   Pin0 - ENABLE - Mapped to register0 bit 6
 *   Pin1 - RS     - Mapped to register0 bit 5
 *   Pin2 -'RW     - Mapped to register0 bit 4
 *   Pin3 - DB7    - Mapped to register0 bit 3
 *   Pin4 - DB6    - Mapped to register0 bit 2
 *   Pin5 - DB5    - Mapped to register0 bit 1
 *   Pin6 - DB4    - Mapped to register0 bit 0
 *  
 *   It must be configured in single-channel. 
 *   The busy waits are not performed neither by polling or interrupts, but by simple delays ( The interface lack's
 *   reading capabilities ). 
 *
 *  TODO: Clean this driver completely. It is a complete mess!
 *
 */
 
 
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/io.h>
#include <asm/io.h>
#include <asm/irq.h>

// Xilinx specific
#include <linux/xilinx_devices.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
    #include <linux/device.h>
#else
    #include <linux/platform_device.h>
#endif

// OTHER VARS

#define SUCCESS 0
#define DEVICE_NAME "xilinx_lcd"
#define BUF_LEN 80 

// TODO: GET THE ADDRESSES FROM THE PLATFORM 

#define LCD_BASE  0x40060000
#define LCD_END   0x4006FFFF


// LCD commands

#define LCD_SET  (0x28)		// b000101000 function set, 4 bit, 2 line, 5x7
#define LCD_SET8 (0x38)		// b000111000 function set, 8 bit, 2 line, 5x7
#define LCD_ON   (0x0c)		// b000001100 display on, cursor off, blink off
#define LCD_OFF  (0x08)		// b000001000 display off, cursor off, blink off
#define LCD_MD   (0x06)		// b000000110 entry mode, increment, no shift
#define LCD_ADR  (0x80)		// b010000000 set address to zero
#define LCD_ADR2 (0xA8)		// b010000000 set address to zero
#define LCD_CLR  (0x01)		// b000000001 clear display

/* 
 *   Prototypes 
 */

static int __init init_function(void);
static void __exit exit_function(void);

static int dev_open(struct inode *, struct file *);
static int dev_rel(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static void send_data_4bit(char ctrl, char data) ;
inline static void lcd_init(void); 
inline static void lcd_on(void);
inline static void lcd_off(void);  
inline static void lcd_clear(void);
static void lcd_print(const char*);

static int procfs_finished = 0;
static ssize_t procfs_read(struct file* , char*, size_t, loff_t*);
static ssize_t procfs_write(struct file*, const char*, size_t, loff_t*);
static ssize_t procfs_cmd_write(struct file*, const char*, size_t, loff_t*);
static ssize_t procfs_l1_write(struct file*, const char*, size_t, loff_t*);
static ssize_t procfs_l2_write(struct file*, const char*, size_t, loff_t*);

static int module_permission(struct inode*, int, struct nameidata*);
int procfs_open(struct inode*, struct file*);
int procfs_close(struct inode*, struct file*);

/* 
 *   Module details 
 */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jorge Sanchez de Nova <jssdn(mail)_(at)kth.se>");
MODULE_DESCRIPTION("Xilinx ML403 LCD driver");


/*
 *  Global variables 
 */

static struct file_operations fops = {
    .read = dev_read,
    .write = dev_write,     
    .open = dev_open, 
    .release = dev_rel
};

static int device_open = 0; 

static int Major; 

static char msg[BUF_LEN]; 
static char *msg_Ptr; 

static u32 lcd_remap_address; 

 
/*
 *  Procfs
 */

static struct file_operations proc_fops = {
	.read 	 = procfs_read,
	.write 	 = procfs_write,
	.open 	 = procfs_open,
	.release = procfs_close,
};

static struct file_operations proc_cmd_fops = {
	.read 	 = procfs_read,
	.write 	 = procfs_cmd_write,
	.open 	 = procfs_open,
	.release = procfs_close,
};

static struct file_operations proc_l1_fops = {
	.read 	 = procfs_read,
	.write 	 = procfs_l1_write,
	.open 	 = procfs_open,
	.release = procfs_close,
};

static struct file_operations proc_l2_fops = {
	.read 	 = procfs_read,
	.write 	 = procfs_l2_write,
	.open 	 = procfs_open,
	.release = procfs_close,
};

static struct inode_operations proc_iops = {
	.permission = module_permission,
};

static struct inode_operations proc_cmd_iops = {
	.permission = module_permission,
};

#define PROC_ENTRY_FILENAME 	"xilinx_lcd"
#define PROC_CMD_ENTRY_FILENAME "xilinx_lcd_cmd"
#define PROC_L1_ENTRY_FILENAME 	"xilinx_lcd_line1"
#define PROC_L2_ENTRY_FILENAME 	"xilinx_lcd_line2"

#define PROCFS_MAX_SIZE 	2048

static char procfs_buffer[PROCFS_MAX_SIZE]; 
static unsigned long procfs_buffer_size = 0;
static struct proc_dir_entry *proc_file;
static struct proc_dir_entry *proc_cmd_file;
static struct proc_dir_entry *proc_l1_file;
static struct proc_dir_entry *proc_l2_file;

/*
 *   Params 
 */

//static int speed1 = 0;

/* 
 * module_param(foo, int, 0000)
 * The first param is the parameters name
 * The second param is it's data type
 * The final argument is the permissions bits, 
 * for exposing parameters in sysfs (if non-zero) at a later stage.
 */

//module_param(parm1, int , S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
//MODULE_PARM_DESC(parm1, "PWM speed of motor no.1");

/* 
 *   IOCTL commands 
 */ 


// int dev_ioctl(struct inode *inode, struct file *file,unsigned int ioctl_num,  unsigned long ioctl_param)
// {
// 	int i;
// 	char *temp;
// 	char ch;
// 
// 	/* 
// 	 * Switch according to the ioctl called 
// 	 */
// 	switch (ioctl_num) {
// 	case :
// 		/* 
// 		 * Receive a pointer to a message (in user space) and set that
// 		 * to be the device's message.  Get the parameter given to 
// 		 * ioctl by the process. 
// 		 */
// 		temp = (char *)ioctl_param;
// 
// 		/* 
// 		 * Find the length of the message 
// 		 */
// 		get_user(ch, temp);
// 		for (i = 0; ch && i < BUF_LEN; i++, temp++)
// 			get_user(ch, temp);
// 
// 		device_write(file, (char *)ioctl_param, i, 0);
// 		break;
// 
// 	case IOCTL_GET_MSG:
// 		/* 
// 		 * Give the current message to the calling process - 
// 		 * the parameter we got is a pointer, fill it. 
// 		 */
// 		i = device_read(file, (char *)ioctl_param, 99, 0);
// 
// 		/* 
// 		 * Put a zero at the end of the buffer, so it will be 
// 		 * properly terminated 
// 		 */
// 		put_user('\0', (char *)ioctl_param + i);
// 		break;
// 
// 	case IOCTL_GET_NTH_BYTE:
// 		/* 
// 		 * This ioctl is both input (ioctl_param) and 
// 		 * output (the return value of this function) 
// 		 */
// 		return Message[ioctl_param];
// 		break;
// 	}
// 
// 	return SUCCESS;
// }

/* 
 *   Init / Exit
 */ 

static int __init init_function(void)
{
        // Chardev registration
        Major = register_chrdev(0,DEVICE_NAME,&fops);

        if( Major < 0){
            printk(KERN_ALERT "Failed to register major with %d \n", Major ); 

        }
	
	printk(KERN_INFO "Xilinx ML403 LCD 16x2 Driver\n");	
	printk(KERN_INFO "---------Assigned Major:%d\n", Major);
	
	// Procfs configuration 
		
	proc_file = create_proc_entry(PROC_ENTRY_FILENAME, 0644, NULL);	
	
	if (proc_file == NULL){
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROC_ENTRY_FILENAME);
		return -ENOMEM;
	}
	
	proc_file->owner = THIS_MODULE;
	proc_file->proc_iops = &proc_iops;
	proc_file->proc_fops = &proc_fops;
	proc_file->mode = S_IFREG | S_IRUGO | S_IWUSR;
	proc_file->uid = 0;
	proc_file->gid = 0;
	proc_file->size = 80;

	printk(KERN_INFO "/proc/%s created\n", PROC_ENTRY_FILENAME);
// 	
	// Procfs cmd interface
		
	proc_cmd_file = create_proc_entry(PROC_CMD_ENTRY_FILENAME, 0644, NULL);	

	if (proc_cmd_file == NULL){
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROC_CMD_ENTRY_FILENAME);
		return -ENOMEM;
	}
	
	proc_cmd_file->owner = THIS_MODULE;
	proc_cmd_file->proc_iops = &proc_iops;
	proc_cmd_file->proc_fops = &proc_cmd_fops;
	proc_cmd_file->mode = S_IFREG | S_IRUGO | S_IWUSR;
	proc_cmd_file->uid = 0;
	proc_cmd_file->gid = 0;
	proc_cmd_file->size = 80;

	printk(KERN_INFO "/proc/%s created\n", PROC_CMD_ENTRY_FILENAME);

	// Procfs l1 interface
		
	proc_l1_file = create_proc_entry(PROC_L1_ENTRY_FILENAME, 0644, NULL);	

	if (proc_l1_file == NULL){
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROC_L1_ENTRY_FILENAME);
		return -ENOMEM;
	}
	
	proc_l1_file->owner = THIS_MODULE;
	proc_l1_file->proc_iops = &proc_iops;
	proc_l1_file->proc_fops = &proc_l1_fops;
	proc_l1_file->mode = S_IFREG | S_IRUGO | S_IWUSR;
	proc_l1_file->uid = 0;
	proc_l1_file->gid = 0;
	proc_l1_file->size = 80;

	printk(KERN_INFO "/proc/%s created\n", PROC_L1_ENTRY_FILENAME);

        // Procfs l2 interface
		
	proc_l2_file = create_proc_entry(PROC_L2_ENTRY_FILENAME, 0644, NULL);	

	if (proc_l2_file == NULL){
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROC_L2_ENTRY_FILENAME);
		return -ENOMEM;
	}
	
	proc_l2_file->owner = THIS_MODULE;
	proc_l2_file->proc_iops = &proc_iops;
	proc_l2_file->proc_fops = &proc_l2_fops;
	proc_l2_file->mode = S_IFREG | S_IRUGO | S_IWUSR;
	proc_l2_file->uid = 0;
	proc_l2_file->gid = 0;
	proc_l2_file->size = 80;

	printk(KERN_INFO "/proc/%s created\n", PROC_L2_ENTRY_FILENAME);

	
	// GPIO Mapping 
	
        if ( (lcd_remap_address = (u32) ioremap(LCD_BASE, LCD_END - LCD_BASE +1 )) == 0) {
		printk(KERN_ERR "Couldn't ioremap memory at 0x%08lX\n",
		       (unsigned long) LCD_BASE);
		return -EFAULT;		
	}
	
	printk(KERN_INFO "LCD_VADDRESS:0x%x\n",(unsigned int)lcd_remap_address);
	printk(KERN_INFO "Configuring the gpio as Outputs\n");	
	out_be32(lcd_remap_address  + 0x4,0x00);		   
        out_be32(lcd_remap_address, 0x00 );

        lcd_init();

        lcd_print("Autonomous Robotics Development");

	return 0;
}

static void __exit exit_function(void)
{
        printk(KERN_INFO "Unregistering chardev...");
        unregister_chrdev(Major,DEVICE_NAME); // doesn't return an int anymore...
        printk(KERN_INFO "Clearing LCD..."); 
        lcd_clear();  

/*        printk(KERN_INFO "Unregistering proc interfaces...");
      	remove_proc_entry(PROC_ENTRY_FILENAME, &proc_root);
	printk(KERN_INFO "Removing /proc/%s...", PROC_ENTRY_FILENAME);
	remove_proc_entry(PROC_CMD_ENTRY_FILENAME, &proc_root);
	printk(KERN_INFO "Removing /proc/%s...", PROC_CMD_ENTRY_FILENAME);
	remove_proc_entry(PROC_L1_ENTRY_FILENAME, &proc_root);
	printk(KERN_INFO "Removing /proc/%s...", PROC_L1_ENTRY_FILENAME);
	remove_proc_entry(PROC_L2_ENTRY_FILENAME, &proc_root);
	printk(KERN_INFO "Removing /proc/%s...", PROC_L2_ENTRY_FILENAME);*/

	//For >2.6.26 kernels
        printk(KERN_INFO "Unregistering proc interfaces...");
       	remove_proc_entry(PROC_ENTRY_FILENAME, NULL);
	printk(KERN_INFO "Removing /proc/%s...", PROC_ENTRY_FILENAME);
	remove_proc_entry(PROC_CMD_ENTRY_FILENAME, NULL);
	printk(KERN_INFO "Removing /proc/%s...", PROC_CMD_ENTRY_FILENAME);
	remove_proc_entry(PROC_L1_ENTRY_FILENAME, NULL);
	printk(KERN_INFO "Removing /proc/%s...", PROC_L1_ENTRY_FILENAME);
	remove_proc_entry(PROC_L2_ENTRY_FILENAME, NULL);
	printk(KERN_INFO "Removing /proc/%s...", PROC_L2_ENTRY_FILENAME);
	
	printk(KERN_INFO "Exiting! \n");	
}

/* 
 *   File operations functions
 */ 

static int dev_open(struct inode *inode, struct file *file)
{
        if(device_open)
           return -EBUSY; 

        device_open++; 
        try_module_get(THIS_MODULE); 

        return SUCCESS; 
}

static int dev_rel(struct inode *inode , struct file *file)
{
        device_open--; 
        module_put(THIS_MODULE);

        return SUCCESS; 
}

static ssize_t dev_read(struct file * filep, char * buffer, size_t length, loff_t * offset)
{
//         int bytes_read = 0; 
//         char tmp;
//                         
//         
//         msg_Ptr = msg; 
//         
//         if( *msg_Ptr == 0 ) 
//             return 0; 
//             
//         while( length && *msg_Ptr ) 
//         {
//             put_user(*(msg_Ptr++),buffer++);
//                
//             length--;
//             bytes_read++;
// 
//         }
//                         
//         return bytes_read; 
//         lcd_print("Autonomous Robotics Development Platform");
        return 0; 
}

// TODO: Add the possibility of writing only 4bits!

static ssize_t dev_write(struct file * filep, const char * buffer, size_t len, loff_t * off)
{
	char tmp[2];
	char order; 
	//printk(KERN_INFO "device_write(%p,%s,%d)", filep, buffer, len);
        printk(KERN_INFO "length:%d\n", len);
        // we only accept 2 char orders
        if(len != 3){
            printk(KERN_INFO "Only two chars are accepted\n");
            return -EINVAL; 
        }

        get_user(tmp[0], buffer);
        get_user(tmp[1], buffer + 1);

        order = (char)simple_strtoul(buffer,NULL,16);
        printk(KERN_INFO "Got: %s\thex:0x%x\n",tmp,order);
        send_data_4bit(0, order); 

	/* 
	 * Again, return the number of input characters used 
	 */
	return len;
}

/* 
 *   Procfs functions
 */ 

/**
 * This funtion is called when the /proc file is read
 *
 */
 
static ssize_t procfs_read(struct file *filp,	/* see include/linux/fs.h   */
			     char *buffer,	/* buffer to fill with data */
			     size_t length,	/* length of the buffer     */
			     loff_t * offset)
{	

	/* 
	 * We return 0 to indicate end of file, that we have
	 * no more information. Otherwise, processes will
	 * continue to read from us in an endless loop. 
	 */
	if ( procfs_finished ) {
		printk(KERN_INFO "procfs_read: END\n");
		procfs_finished = 0;
		return 0;
	}
	
	procfs_finished = 1;
		
	/* 
	 * We use put_to_user to copy the string from the kernel's
	 * memory segment to the memory segment of the process
	 * that called us. get_from_user, BTW, is
	 * used for the reverse. 
	 */
	if ( copy_to_user(buffer, procfs_buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}

	printk(KERN_INFO "procfs_read: read %lu bytes\n", procfs_buffer_size);

	return procfs_buffer_size;	/* Return the number of bytes "read" */
}

/*
 * This function is called when /proc is written
 */
static ssize_t procfs_write(struct file *file, const char *buffer, size_t len, loff_t * off)
{
	if ( len > PROCFS_MAX_SIZE )	{
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}else{
		procfs_buffer_size = len;
	}
	
	if ( copy_from_user(procfs_buffer, buffer, procfs_buffer_size - 1) ) {
		return -EFAULT;
	}

	printk(KERN_INFO "procfs_write: write %lu bytes\n", procfs_buffer_size);
	
	lcd_clear();
	
	lcd_print(procfs_buffer); 
	
	return procfs_buffer_size;
}

static ssize_t procfs_cmd_write(struct file *file, const char *buffer, size_t len, loff_t * off)
{
	char tmp[2];
	char order; 
	//printk(KERN_INFO "device_write(%p,%s,%d)", filep, buffer, len);
        printk(KERN_INFO "length:%d\n", len);
        // we only accept 2 char orders
        if(len != 3){
            printk(KERN_INFO "Only two chars are accepted\n");
            return -EINVAL; 
        }

        get_user(tmp[0], buffer);
        get_user(tmp[1], buffer + 1);

        order = (char)simple_strtoul(buffer,NULL,16);
        printk(KERN_INFO "Got: %s\thex:0x%x\n",tmp,order);
        send_data_4bit(0, order); 

	/* 
	 * Again, return the number of input characters used 
	 */
	return len;
}

static ssize_t procfs_l1_write(struct file *file, const char *buffer, size_t len, loff_t * off)
{
        int i,nlen; 
	char* cit = procfs_buffer; 
	
	nlen = (len<=16?len:16) - 1;
	
	if( copy_from_user(procfs_buffer, buffer, nlen) ) {
		return -EFAULT;
	}

        send_data_4bit(0, LCD_ADR); 

        for( i = 0 ; i < 16 && *cit != 0 ; i++ ,cit++) {
            send_data_4bit( 1 , *cit);
        }	
	
	return len;
}

static ssize_t procfs_l2_write(struct file *file, const char *buffer, size_t len, loff_t * off)
{
        int i,nlen; 
	char* cit = procfs_buffer; 
	
	nlen = (len<=16?len:16) - 1;
	
	if( copy_from_user(procfs_buffer, buffer, nlen) ) {
		return -EFAULT;
	}

        send_data_4bit(0, LCD_ADR2); 

        for( i = 0 ; i < 16 && *cit != 0 ; i++ ,cit++) {
            send_data_4bit( 1 , *cit);
        }

	return len;
}

/* 
 * This function decides whether to allow an operation
 * (return zero) or not allow it (return a non-zero
 * which indicates why it is not allowed).
 *
 * The operation can be one of the following values:
 * 0 - Execute (run the "file" - meaningless in our case)
 * 2 - Write (input to the kernel module)
 * 4 - Read (output from the kernel module)
 *
 * This is the real function that checks file
 * permissions. The permissions returned by ls -l are
 * for referece only, and can be overridden here.
 */

static int module_permission(struct inode *inode, int op, struct nameidata *foo)
{
	/* 
	 * We allow everybody to read from our module, but
	 * only root (uid 0) may write to it 
	 */
	if (op == 4 || (op == 2 && current->cred->euid == 0))
		return 0;

	/* 
	 * If it's anything else, access is denied 
	 */
	return -EACCES;
}

/* 
 * The file is opened - we don't really care about
 * that, but it does mean we need to increment the
 * module's reference count. 
 */
 
int procfs_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return 0;
}

/* 
 * The file is closed - again, interesting only because
 * of the reference count. 
 */
 
int procfs_close(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;		/* success */
}


/* 
 *   Other functions 
 */ 

/* 
 *   send_data_4bit : Send data in two times in 4 bit mode
 */ 


static void send_data_4bit(char rs, char data) 
{
  char data4b; 

  rs = rs? 0x20 : 0x00 ; 

  // First nibble - MSbs
  data4b = (data >> 4); 

  // Enable up/down
  out_be32((volatile unsigned*)lcd_remap_address, rs | data4b );
  udelay(41);
  out_be32((volatile unsigned*)lcd_remap_address, 0x40 | rs | data4b );
  udelay(41);  
  out_be32((volatile unsigned*)lcd_remap_address, rs | data4b );
  udelay(41);

//   printk(KERN_INFO "Sent data:0x%x\n", data4b );

  //second nibble - LSbs
  data4b = data & 0x0f; 
  // Enable up/down
  out_be32((volatile unsigned*)lcd_remap_address, rs | data4b );
  udelay(41);
  out_be32((volatile unsigned*)lcd_remap_address, 0x40 | rs | data4b );
  udelay(41);
  out_be32((volatile unsigned*)lcd_remap_address, rs | data4b );
  udelay(41);

//   printk(KERN_INFO "Sent data:0x%x\n",data4b);
}

static void send_nibble(char rs, char data) 
{
  char data4b; 

  rs = rs? 0x20 : 0x00 ; 

  // First nibble - MSbs
  data4b = data & 0x0f; 

  // Enable up/down
  out_be32((volatile unsigned*)lcd_remap_address, rs | data4b );
  udelay(41);
  out_be32((volatile unsigned*)lcd_remap_address, 0x40 | rs | data4b );
  udelay(41);  
  out_be32((volatile unsigned*)lcd_remap_address, rs | data4b );
  udelay(41);

//  printk(KERN_INFO "Sent nibble:0x%x\n", data4b );
 
}

inline static void lcd_init(void)
{
    int i;
    printk(KERN_INFO "LCD_INITIALIZATION\n");

    // delay 15ms after processor reset
    udelay (15000);
    // output Display Set 3 times with 50ms delay
    for (i=0; i < 3; i++)
            { send_nibble(0, LCD_SET8 >> 4); udelay (50000); }

    // now force display to 4 bit mode
    send_nibble(0, LCD_SET >> 4); udelay(50000);
    // finally send out full display set command
    send_data_4bit(0, LCD_SET); udelay(50);

    lcd_off();
    lcd_clear();
    lcd_on();

    // set display mode
    send_data_4bit(0, LCD_MD);
    // set display address
    send_data_4bit(0, LCD_ADR); 
	
    printk(KERN_INFO "------------------\n");

}

inline static void lcd_on(void)
{
    send_data_4bit(0, LCD_ON); 
}

inline static void lcd_off(void)
{
    send_data_4bit(0, LCD_OFF); 
}

inline static void lcd_clear(void)
{
  send_data_4bit(0, LCD_CLR); 
  udelay(1640);
}

static void lcd_print(const char *msg)
{
    int i= 0 ; 
    //char* msg = "Autonomous Robotics Development Platform"; 

    send_data_4bit(0, LCD_ADR); 
    for( i = 0 ; i < 16 && *msg != 0 ; i++ ) {
        send_data_4bit( 1 , *msg);
        msg++;
    }
    // we jump to the position 40
    send_data_4bit(0, LCD_ADR2); 
    for( i = 0 ; i < 16 && *msg != 0 ; i++ ) {
        send_data_4bit( 1 , *msg);
        msg++;
    }
}


module_init(init_function);
module_exit(exit_function);

