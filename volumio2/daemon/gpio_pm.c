#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "gpio_pm.h"

#define BCM2708_PERI_BASE	0x20000000
#define GPIO_BASE		(BCM2708_PERI_BASE + 0x200000)

#define PAGE_SIZE		(4*1024)
#define BLOCK_SIZE		(4*1024)

#define INP_GPIO(g)		*(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define GET_GPIO(g)		(*(gpio+13)&(1<<g))

volatile unsigned *gpio;

int gpio_pm_init(void)
{
    int mem_fd;
    void *gpio_map;
    int ret = 0;

    if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
        fprintf(stderr, "gpio_pm_init(): Can't open /dev/mem!\n");
        return -1;
    }

    gpio_map = mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE,
                    MAP_SHARED, mem_fd, GPIO_BASE);

    close(mem_fd);

    if (gpio_map == MAP_FAILED) {
        fprintf(stderr, "gpio_pm_init(): map error %p!\n", gpio_map);
        return -1;
    }

    gpio = (volatile unsigned *)gpio_map;

    INP_GPIO(17);

    return 0;
}

int gpio_pm_get_restart_request(void)
{
    if (GET_GPIO(17)) {
        return 1;
    } else {
        return 0;
    }
}
