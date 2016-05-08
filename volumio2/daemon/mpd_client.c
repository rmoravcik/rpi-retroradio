#include <stdio.h>

#include <mpd/connection.h>
#include <mpd/mixer.h>

#include "mpd_client.h"

struct mpd_connection *conn;

static int mpc_client_handle_error(struct mpd_connection *conn)
{
    fprintf(stderr, "mpc_client_handle_error(): %s!\n", mpd_connection_get_error_message(conn));
    mpd_connection_free(conn);

    return 1;
}

int mpd_client_connect(void)
{
    conn = mpd_connection_new(NULL, 0, 30000);

    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
        return mpc_client_handle_error(conn);

    printf("mpd_client_connect(): Connected to MPD version: %i.%i.%i\n",
            mpd_connection_get_server_version(conn)[0],
            mpd_connection_get_server_version(conn)[1],
            mpd_connection_get_server_version(conn)[2]);

    return 0;
}

int mpd_client_disconnect(void)
{
    if (!conn) {
        fprintf(stderr, "mpd_client_disconnect(): Invalid connection %p!\n", conn);
        return 1;
    }

    mpd_connection_free(conn);

    return 0;
}

int mpd_client_set_volume(int volume)
{
    int ret;

    if ((volume < 0) || (volume > 100)) {
        fprintf(stderr, "mpd_client_set_volume(): Volume out of range %d!\n", volume);
        return 1;
    }

    ret = mpd_run_set_volume(conn, volume);

    if (!ret) {
        fprintf(stderr, "mpd_client_set_volume(): Failed to set volume to %d!\n", volume);
        return 1;
    }

    return 0;
}
