#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "adc_volctl.h"

static int debug = 0;

static int fd = -1;

static unsigned int speed = 500000;
static unsigned char mode = SPI_MODE_0;
static unsigned char bits = 8;

int adc_volctl_init(void)
{
    int ret;

    const char *device = "/dev/spidev0.0";

    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        fprintf(stderr, "adc_volctl_init(): Can't open device %s!\n", device);
        return -1;
    }

    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
    {
        fprintf(stderr, "adc_volctl_init(): Can't set mode!\n");
        return -1;
    }

    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
    {
        fprintf(stderr, "adc_volctl_init(): Can't set bits!\n");
        return -1;
   }

    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
    {
        fprintf(stderr, "adc_volctl_init(): can't set speed!\n");
        return -1;
    }

    return 0;
}

int adc_volctl_close(void)
{
    if (fd < 0) {
        fprintf(stderr, "adc_volctl_close(): Invalid fd %d!\n", fd);
        return -1;
    }

    close(fd);

    return 0;
}

int adc_volctl_get_volume(void)
{
    int ret;
    int val = 0;
    int volume = 0;

    unsigned char tx[] = { 0x01, 0x80, 0x00 };
    unsigned char rx[] = { 0x00, 0x00, 0x00 };

    struct spi_ioc_transfer spi[1];

    if (fd < 0) {
        fprintf(stderr, "adc_volctl_get_volume(): SPI device not opened!\n");
        return -1;
    }

    memset(&spi, 0, sizeof(spi));

    spi[0].tx_buf        = (unsigned long)tx;
    spi[0].rx_buf        = (unsigned long)rx;
    spi[0].len           = 3;
    spi[0].delay_usecs   = 0;
    spi[0].speed_hz      = speed;
    spi[0].bits_per_word = bits;
    spi[0].cs_change     = 0;

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &spi);
    if(ret == -1)
    {
        fprintf(stderr, "adc_volctl_get_volume(): Can't transfer data!\n");
        return -1;
    }

    val = ((rx[1] & 0x0F) << 8) | rx[2];
    volume = (val * 101) / 4096;

    if (debug) {
        printf("adc_volctl_get_volume(): val=%d volume=%d\n", val, volume);
    }

    return volume;
}
