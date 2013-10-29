#ifndef IRC_TYPES_H
#define IRC_TYPES_H

#include <glib.h>

typedef enum _UserModes		UserModes;
typedef enum _ChannelModes	ChannelModes;
typedef enum _ChannelUserModes	ChannelUserModes;
typedef enum _ServerNoticeModes	ServerNoticeModes;
typedef struct _Server		Server;
typedef struct _User		User;
typedef struct _Channel		Channel;

enum _UserModes
{
	USER_MODE_INVISIBLE		= 1 << 0,
	USER_MODE_SNOTICE_MASK		= 1 << 1,
	USER_MODE_SNOTICE		= 1 << 2,
	USER_MODE_WALLOPS		= 1 << 3,
	USER_MODE_OPERATOR		= 1 << 4
};

enum _ChannelModes
{
	CHANNEL_MODE_INVITE_ONLY	= 1 << 0,
	CHANNEL_MODE_MODERATED		= 1 << 1,
	CHANNEL_MODE_NO_EXTERNAL	= 1 << 2,
	CHANNEL_MODE_PRIVATE		= 1 << 3,
	CHANNEL_MODE_SECRET		= 1 << 4,
	CHANNEL_MODE_TOPIC		= 1 << 5
};

enum _ChannelUserModes
{
	CHANNEL_USER_MODE_OP		= 1 << 0,
	CHANNEL_USER_MODE_VOICE		= 1 << 1,
	CHANNEL_USER_MODE_HALFOP	= 1 << 2,
	CHANNEL_USER_MODE_FOUNDER	= 1 << 3,
	CHANNEL_USER_MODE_PROTECTED	= 1 << 4
};

enum _ServerNoticeModes
{
	SERVER_NOTICE_MODE_REMOTE_CONNECT	= 1 << 0,
	SERVER_NOTICE_MODE_LOCAL_CONNECT	= 1 << 1,
	SERVER_NOTICE_MODE_REMOTE_QUIT		= 1 << 2,
	SERVER_NOTICE_MODE_LOCAL_QUIT		= 1 << 3,
	SERVER_NOTICE_MODE_REMOTE_KILL		= 1 << 4,
	SERVER_NOTICE_MODE_LOCAL_KILL		= 1 << 5,
	SERVER_NOTICE_MODE_LINKS		= 1 << 6,
	SERVER_NOTICE_MODE_OPERATORS		= 1 << 7,
	SERVER_NOTICE_MODE_DEBUG		= 1 << 8,
	SERVER_NOTICE_MODE_XLINES		= 1 << 9,
	SERVER_NOTICE_MODE_STATS		= 1 << 10,
	SERVER_NOTICE_MODE_FLOODS		= 1 << 11,
	SERVER_NOTICE_MODE_GLOBOPS		= 1 << 12,
	SERVER_NOTICE_MODE_OVERRIDE		= 1 << 13,
	SERVER_NOTICE_MODE_LOCAL_NICKS		= 1 << 14,
	SERVER_NOTICE_MODE_REMOTE_NICKS		= 1 << 15,
	SERVER_NOTICE_MODE_NEW_CHANNELS		= 1 << 16
};

struct _Server
{
	char	*name;
	int	distance;
	char	*description;
};

struct _User
{
	Server	*server;
	time_t	timestamp;
	char	*nickname;
	char	*hostname;
	char	*vhostname;
	char	*username;
	char	*ip;
	char	*realname;
	UserModes modes;
	ServerNoticeModes snotice_modes;
};

struct _Channel
{
	char	*name;
	time_t	timestamp;

	ChannelModes modes;
	GSList	*bans;
	char	*key;
	int	limit;

	char	*topic;
	time_t	topic_time;
	char	*topic_author;

	GSList	*users;
	GHashTable *user_modes;
};

inline gchar *user_modes_to_string(UserModes modes);
inline UserModes user_modes_from_string(const char *s);
inline gchar *server_notice_modes_to_string(ServerNoticeModes modes);
inline ChannelUserModes channel_user_modes_from_prefix(const char *prefix);
inline gchar *channel_user_modes_to_prefix(ChannelUserModes modes);

#endif // not IRC_TYPES_H
