#ifndef CONF_H
#define CONF_H

#include <glib.h>

/*
 * The various parameters that can be set from configuration file
 */
extern char *ConfServerName;
extern char *ConfServerInfo;
extern char *ConfRemoteHostName;
extern int ConfRemotePort;
extern char *ConfRemoteSendPass;
extern char *ConfRemoteRecvPass;
extern char *ConfBotsDatabase;
extern char *ConfBotsUserName;
extern char *ConfBotsHostName;
extern char *ConfBotsVHostName;
extern char *ConfBotsIP;
extern char *ConfBotsUModes;
extern char *ConfBotsRealName;
extern gboolean ConfBotsVoice;

/* Reads configuration file from disk */
int conf_read(const char *filename);

#endif /* not CONF_H */
