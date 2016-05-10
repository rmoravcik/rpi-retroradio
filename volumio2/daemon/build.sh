#!/bin/sh

gcc -o rpi-psud main.c adc_volctl.c gpio_pm.c mpd_client.c -lmpdclient
