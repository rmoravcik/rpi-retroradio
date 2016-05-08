#ifndef MPD_CLIENT_H
#define MPD_CLIENT_H

int mpd_client_connect(void);
int mpd_client_disconnect(void);

int mpd_client_set_volume(int volume);

#endif
