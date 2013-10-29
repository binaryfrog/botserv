#include <stdlib.h>

#include "conf.h"

typedef enum _ConfType ConfType;
typedef struct _ConfItem ConfItem;

enum _ConfType
{
	CONF_TYPE_STRING,
	CONF_TYPE_INTEGER,
	CONF_TYPE_BOOLEAN
};

struct _ConfItem
{
	char *group;
	char *key;
	ConfType type;
	void *value;
	int set:1;
};

char *ConfServerName;
char *ConfServerInfo;
char *ConfRemoteHostName;
int ConfRemotePort;
char *ConfRemoteSendPass;
char *ConfRemoteRecvPass;
char *ConfBotsDatabase;
char *ConfBotsUserName;
char *ConfBotsHostName;
char *ConfBotsVHostName;
char *ConfBotsIP;
char *ConfBotsUModes;
char *ConfBotsRealName;
gboolean ConfBotsVoice;

static ConfItem ConfItems[] =
{
	{ "server", "name", CONF_TYPE_STRING, &ConfServerName, 0 },
	{ "server", "info", CONF_TYPE_STRING, &ConfServerInfo, 0 },
	{ "remote", "hostname", CONF_TYPE_STRING, &ConfRemoteHostName, 0 },
	{ "remote", "port", CONF_TYPE_INTEGER, &ConfRemotePort, 0 },
	{ "remote", "sendpass", CONF_TYPE_STRING, &ConfRemoteSendPass, 0 },
	{ "remote", "recvpass", CONF_TYPE_STRING, &ConfRemoteRecvPass, 0 },
	{ "bots", "database", CONF_TYPE_STRING, &ConfBotsDatabase, 0 },
	{ "bots", "username", CONF_TYPE_STRING, &ConfBotsUserName, 0 },
	{ "bots", "hostname", CONF_TYPE_STRING, &ConfBotsHostName, 0 },
	{ "bots", "vhostname", CONF_TYPE_STRING, &ConfBotsVHostName, 0 },
	{ "bots", "ipaddress", CONF_TYPE_STRING, &ConfBotsIP, 0 },
	{ "bots", "umodes", CONF_TYPE_STRING, &ConfBotsUModes, 0 },
	{ "bots", "realname", CONF_TYPE_STRING, &ConfBotsRealName, 0 },
	{ "bots", "voice", CONF_TYPE_BOOLEAN, &ConfBotsVoice, 0 },
	{ NULL, NULL, 0, NULL, 0 }
};

static const char *FileName;
static int FileErrors;

static void conf_set_string(GKeyFile *key_file, ConfItem *conf_item);
static void conf_set_integer(GKeyFile *key_file, ConfItem *conf_item);
static void conf_set_boolean(GKeyFile *key_file, ConfItem *conf_item);

int conf_read(const char *filename)
{
	g_debug("Using configuration file: %s\n", filename);

	GKeyFile *key_file = g_key_file_new();
	GError *error = NULL;

	if(!g_key_file_load_from_file(key_file, filename, 0, &error))
	{
		g_critical("%s: configuration file error: %s",
				filename, error->message);
		exit(1);
	}

	FileName = filename;
	FileErrors = 0;

	int i;
	for(i = 0; ConfItems[i].group; ++i)
	{
		if(!g_key_file_has_key(key_file, ConfItems[i].group,
					ConfItems[i].key, &error))
		{
			g_critical("%s: required key %s.%s not defined "
					"in configuration file", filename,
					ConfItems[i].group, ConfItems[i].key);
			++FileErrors;
			continue;
		}

		switch(ConfItems[i].type)
		{
			case CONF_TYPE_STRING:
				conf_set_string(key_file, &ConfItems[i]);
				break;
			case CONF_TYPE_INTEGER:
				conf_set_integer(key_file, &ConfItems[i]);
				break;
			case CONF_TYPE_BOOLEAN:
				conf_set_boolean(key_file, &ConfItems[i]);
				break;
			default:
				g_assert_not_reached();
		}
	}

	if(FileErrors)
	{
		g_critical("%s: %d configuration errors",
				filename, FileErrors);
		exit(1);
	}

	g_key_file_free(key_file);

	g_debug("Successfully parsed %d configuration items\n", i);

	return 0;
}

static void conf_set_string(GKeyFile *key_file, ConfItem *conf_item)
{
	GError *error = NULL;
	char **conf_value = conf_item->value;
	*conf_value = g_key_file_get_string(key_file, conf_item->group,
			conf_item->key, &error);

	if(*conf_value == NULL && error != NULL)
	{
		g_critical("%s: invalid string value for configuration "
				"item %s.%s", FileName, conf_item->group,
				conf_item->key);
		++FileErrors;
	}
}

static void conf_set_integer(GKeyFile *key_file, ConfItem *conf_item)
{
	GError *error = NULL;
	int *conf_value = conf_item->value;
	*conf_value = g_key_file_get_integer(key_file, conf_item->group,
			conf_item->key, &error);

	if(*conf_value == 0 && error != NULL)
	{
		g_critical("%s: invalid integer value for configuration "
				"item %s.%s", FileName, conf_item->group,
				conf_item->key);
		++FileErrors;
	}
}

static void conf_set_boolean(GKeyFile *key_file, ConfItem *conf_item)
{
	GError *error = NULL;
	gboolean *conf_value = conf_item->value;
	*conf_value = g_key_file_get_boolean(key_file, conf_item->group,
			conf_item->key, &error);

	if(*conf_value == FALSE && error != NULL)
	{
		g_critical("%s: invalid boolean value for configuration "
				"item %s.%s", FileName, conf_item->group,
				conf_item->key);
		++FileErrors;
	}
}

