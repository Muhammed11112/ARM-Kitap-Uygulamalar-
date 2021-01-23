#include	<stdint.h>

#ifndef	__SENSOR_H
#define	__SENSOR_H

#define			WHO_AM_I			(0X0F)																// Registerlar isimleriyle birlikte tanimlaniyor
#define			CTRL_REG3			(0X23)
#define			CTRL_REG4			(0X20)
#define			CTRL_REG5			(0X24)
#define			CTRL_REG6			(0X25)
#define			FIFO_CTRL			(0X2E)
#define			OUT_X					(0X29)
#define			OUT_Y					(0X2B)
#define			OUT_Z					(0X2D)
#define			FIFO_SRC 			(0X2F)



void	  spi1_init(void);
void		SPI_LIS3DSH_Tx( uint8_t	adr,	uint8_t data);						// SPI ILE MODULE DATA GONDERME FONK 
uint8_t	SPI_LIS3DSH_Rx( uint8_t adr);														// SPI ILE MODULDEN DATA OKUMA FONK
void		LIS3DSH_Init( void);
#endif

