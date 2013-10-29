#ifndef SERVER_H
#define SERVER_H

#include <glib.h>

#include "irc_types.h"

void server_init();

Server *server_new(const char *name, int distance, const char *description);
inline Server *server_find(const char *name);

#endif // not SERVER_H
