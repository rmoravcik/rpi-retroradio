#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "adc_volctl.h"
#include "gpio_pm.h"
#include "mpd_client.h"

int main(void)
{
    int ret = 0;
    int volume, volume_new = 0;

    ret = gpio_pm_init();
    if (ret < 0) {
        return -1;
    }

    ret = adc_volctl_init();
    if (ret < 0) {
        return -1;
    }

    ret = mpd_client_connect();
    if (ret < 0) {
        return -1;
    }

    while (1) {
        volume_new = adc_volctl_get_volume();

        if ((volume_new >= 0) && (volume_new != volume)) {
            printf("main(): Setting volume to %d\n", volume_new);
            ret = mpd_client_set_volume(volume_new);
            volume = volume_new;
        }

        if (gpio_pm_get_restart_request()) {
            printf("main(): Running comannd 'poweroff'!\n");
            ret = system("poweroff");
            if (ret != 0) {
                fprintf(stderr, "main(): Failed to run command 'poweroff'!\n");
            }
        }

        usleep(100000);
    }

    mpd_client_disconnect();

    adc_volctl_close();

    return 0;
}
