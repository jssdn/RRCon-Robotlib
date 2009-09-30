#ifndef __SPIDEV_H__
#define __SPIDEV_H__

#include <native/mutex.h>

#define SPI_DEFAULT_SPEED 500000
//TODO: IS THIS DELAY CORRECT?
#define SPI_DEFAULT_DELAY 0
#define SPI_DEFAULT_BITS 8 

#define DEVBUFFERSIZE 25

typedef struct struct_xspidev {
    char device[DEVBUFFERSIZE]; //file descriptor 
    int fd;
    uint8_t mode;
    uint8_t bits;
    uint32_t speed;
    uint16_t delay;
    RT_MUTEX mutex;
} xspidev;

//transfer
int spi_full_transfer(xspidev* xspi, uint8_t* tx, uint8_t* rx, int len);
// write only
inline int spi_half_tranfer( xspidev* xspi, uint8_t* data, int len );
//read read only
inline int spi_half_read( xspidev* xspi , uint8_t* data, int len );
// configure structure
int spi_configure(xspidev* xspi, // STRUCT TO CONFIGURE
                  const char* devname, // DEVICE NAME 
                  uint32_t speed, // SPEED IN HZ
                  uint16_t delay, // DELAY IN ¿¿??
                  uint8_t bits, // BITS AT A TIME
                  uint8_t loop, // LOOPBACK MODE
                  uint8_t cpha, // CPHA
                  uint8_t cpol, // CPOL 
                  uint8_t lsb_first, // LSB FIRST
                  uint8_t cs_high, // CS HIGH
                  uint8_t spi_3wire); // 3 WIRE SPI MODE
// apply config
int spi_set_config(xspidev* xspi);


#endif
