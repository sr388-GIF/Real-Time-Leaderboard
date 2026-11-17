#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "api.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Start the HTTP server on given port.
 * Returns 0 on success, -1 on failure. */
int http_server_start(AppState *app, int port);

/* Stop the server gracefully */
void http_server_stop(void);

#ifdef __cplusplus
}
#endif

#endif