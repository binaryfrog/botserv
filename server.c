#include "server.h"

static GHashTable *Servers;

void server_init()
{
	Servers = g_hash_table_new(g_str_hash, g_str_equal);
}

Server *server_new(const char *name, int distance, const char *description)
{
	Server *server = g_new0(Server, 1);
	server->name = g_strdup(name);
	server->distance = distance;
	server->description = g_strdup(description);
	g_hash_table_insert(Servers, server->name, server);
	return server;
}

inline Server *server_find(const char *name)
{
	return g_hash_table_lookup(Servers, name);
}

