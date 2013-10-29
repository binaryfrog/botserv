#ifndef BOTS_H
#define BOTS_H

#include <glib.h>

typedef struct _Bot Bot;

struct _Bot
{
	char *nickname;
	GSList *channels;
};

extern GSList *Bots;

/* Reads bot database from disk */
void bots_read_db();

/* Writes bot database to disk */
void bots_write_db();

#endif /* not BOTS_H */
