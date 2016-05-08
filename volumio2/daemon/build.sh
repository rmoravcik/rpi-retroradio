#!/bin/sh

gcc -o rpi-retroradiod main.c adc.c mpd_client.c -lmpdclient
