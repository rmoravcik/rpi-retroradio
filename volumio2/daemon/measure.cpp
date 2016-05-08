/******************************************************************************/
/*                                                                            */
/*                                                          FILE: measure.cpp */
/*                                                                            */
/*    Measures an analog value using an MCP3202 wired to a Raspberry PI       */
/*    =================================================================       */
/*                                                                            */
/*    The Microchip MCP3202 is a two channel 12bit AD converter. It was       */
/*    connected to the SPI bus of the Raspberry PI as shown at                */
/*    http://blog.heimetli.ch/raspberry-pi-mcp3202-12bit-ad-converter.html    */
/*                                                                            */
/*    This code was compiled with g++ and run under the Raspbian OS. It       */
/*    uses the spidev driver bundled with current Raspbian releases.          */
/*                                                                            */
/*    The program uses the device file spidev0.0 which belongs to root        */
/*    by default. For a first test run it like this: sudo ./measure           */
/*                                                                            */
/*    V0.01  07-SEP-2013   Te                                                 */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

static const char *device = "/dev/spidev0.0" ;
static uint32_t    speed  = 500000 ;
static uint8_t     mode   = SPI_MODE_0 ;
static uint8_t     bits   = 8 ;

/**********/
 int main()
/**********/

{
   int fd = open( device, O_RDWR ) ;
   if( fd < 0 )
   {
      fprintf( stderr, "can't open device\n" ) ;
      return 1 ;
   }

   int ret = ioctl( fd, SPI_IOC_WR_MODE, &mode ) ;
   if( ret == -1 )
   {
      close( fd ) ;
      fprintf( stderr, "can't set mode\n" ) ;
      return 2 ;
   }

   ret = ioctl( fd, SPI_IOC_WR_BITS_PER_WORD, &bits ) ;
   if( ret == -1 )
   {
      close( fd ) ;
      fprintf( stderr, "can't set bits\n" ) ;
      return 3 ;
   }

   ret = ioctl( fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed ) ;
   if( ret == -1 )
   {
      close( fd ) ;
      fprintf( stderr, "can't set speed\n" ) ;
      return 4 ;
   }

   // Read channel 1, unsigned mode
   unsigned char tx[]   = { 0x01, 0x80, 0x00 } ;
   unsigned char rx[]   = { 0x00, 0x00, 0x00 } ;

   struct spi_ioc_transfer spi[1];
   memset( &spi, 0, sizeof(spi) ) ;
 
   spi[0].tx_buf        = (unsigned long)tx ;
   spi[0].rx_buf        = (unsigned long)rx ;
   spi[0].len           = 3 ;
   spi[0].delay_usecs   = 0 ; 
   spi[0].speed_hz      = speed ;
   spi[0].bits_per_word = bits ;
   spi[0].cs_change     = 0 ;
 
   ret = ioctl( fd, SPI_IOC_MESSAGE(1), &spi ) ;
   if( ret == -1 )
   {
      close( fd ) ;
      fprintf( stderr, "can't transfer data\n" ) ;
      return 5 ;
   }
 
   close( fd ) ;

   printf( "%d\n", ((rx[1] & 0x0F) << 8) | rx[2] ) ;

   return 0 ;
}
