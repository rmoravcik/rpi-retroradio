#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "adc.h"
#include "mpd_client.h"

int main(void)
{
    int ret = 0;

    ret = mpd_client_connect();

    if (ret) {
        return 1;
    }

    ret = adc_init();

    while (1) {
        int volume = adc_get_volume();
        usleep(100000);
    }

    ret = mpd_client_set_volume(10);
    if (ret) {
    }

    adc_close();

    mpd_client_disconnect();

    return 0;
}
