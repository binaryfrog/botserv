#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include <glib.h>

#include "irc.h"
#include "irc_types.h"
#include "conf.h"
#include "bots.h"
#include "server.h"
#include "user.h"
#include "channel.h"

Sock *IrcSock;

static int Registered = 0;

static void irc_handle(IrcMsg *msg);
static void irc_burst_state();
static void irc_change_channel_modes(Channel *channel, const char *modes,
		const char **args, int nargs);
static void irc_change_user_modes(User *user, const char *modes);
static void on_server(IrcMsg *msg);
static void on_ping(IrcMsg *msg);
static void on_burst(IrcMsg *msg);
static void on_endburst(IrcMsg *msg);
static void on_nick(IrcMsg *msg);
static void on_fjoin(IrcMsg *msg);
static void on_join(IrcMsg *msg);
static void on_part(IrcMsg *msg);
static void on_kick(IrcMsg *msg);
static void on_fmode(IrcMsg *msg);
static void on_mode(IrcMsg *msg);
static void on_time(IrcMsg *msg);

//
// Initiates connection to remote server
//
void irc_connect()
{
	server_init();
	user_init();
	channel_init();

	IrcSock = sock_new();

	if(sock_open(IrcSock, ConfRemoteHostName, ConfRemotePort) != 0)
		exit(1);

	Registered = 0;
	sock_writef(IrcSock, "SERVER %s %s 0 :%s\r\n", ConfServerName,
			ConfRemoteSendPass, ConfServerInfo);

	Server *server = server_new(ConfServerName, 0, ConfServerInfo);

	for(GSList *i = Bots; i; i = i->next)
	{
		Bot *bot = i->data;

		User *user = user_new(bot->nickname);
		user->server = server;
		user->timestamp = time(NULL);
		user->hostname = g_strdup(ConfBotsHostName);
		user->vhostname = g_strdup(ConfBotsVHostName);
		user->username = g_strdup(ConfBotsUserName);
		user->modes = user_modes_from_string(ConfBotsUModes);
		user->ip = g_strdup(ConfBotsIP);
		user->realname = g_strdup(ConfBotsRealName);

		for(GSList *j = bot->channels; j; j = j->next)
		{
			Channel *channel = channel_find(j->data);

			if(!channel)
				channel = channel_new(j->data);

			channel->users = g_slist_append(channel->users, user);
			if(ConfBotsVoice)
				channel_set_user_modes(channel, user,
						CHANNEL_USER_MODE_VOICE);
		}
	}
}

//
// Reads from remote server and processes waiting messages
//
void irc_poll()
{
	if(sock_poll(IrcSock))
		sock_read(IrcSock);

	char *line;

	while((line = sock_getline(IrcSock)))
	{
		IrcMsg *msg = irc_msg_new(line);
		irc_handle(msg);
		irc_msg_delete(msg);
		free(line);
	}
}

//
// Handles a single message from remote server
//
static void irc_handle(IrcMsg *msg)
{
	if(strcasecmp(msg->args[0], "SERVER") == 0)
		on_server(msg);
	else if(strcasecmp(msg->args[0], "PING") == 0)
		on_ping(msg);
	else if(strcasecmp(msg->args[0], "BURST") == 0)
		on_burst(msg);
	else if(strcasecmp(msg->args[0], "ENDBURST") == 0)
		on_endburst(msg);
	else if(strcasecmp(msg->args[0], "NICK") == 0)
		on_nick(msg);
	else if(strcasecmp(msg->args[0], "FJOIN") == 0)
		on_fjoin(msg);
	else if(strcasecmp(msg->args[0], "JOIN") == 0)
		on_join(msg);
	else if(strcasecmp(msg->args[0], "PART") == 0)
		on_part(msg);
	else if(strcasecmp(msg->args[0], "KICK") == 0)
		on_kick(msg);
	else if(strcasecmp(msg->args[0], "FMODE") == 0)
		on_fmode(msg);
	else if(strcasecmp(msg->args[0], "MODE") == 0)
		on_mode(msg);
	else if(strcasecmp(msg->args[0], "TIME") == 0)
		on_time(msg);
}

static void irc_burst_state()
{
	sock_writef(IrcSock, "BURST\r\n");

	GList *list;

	list = user_find_all();
	for(GList *i = list; i; i = i->next)
	{
		User *user = i->data;
		user_introduce(user);
	}
	g_list_free(list);

	list = channel_find_all();
	for(GList *i = list; i; i = i->next)
	{
		Channel *channel = i->data;
		channel_introduce(channel);
	}
	g_list_free(list);

	sock_writef(IrcSock, "ENDBURST\r\n");
}

static void irc_change_channel_modes(Channel *channel, const char *modes,
		const char **args, int nargs)
{
	gboolean add_mode = TRUE;
	int arg_index = 0;
	for(const char *ptr = modes; *ptr; ++ptr)
	{
		if(*ptr == '+')
		{
			add_mode = TRUE;
			continue;
		} else if(*ptr == '-')
		{
			add_mode = FALSE;
			continue;
		}

		const char *arg = (arg_index < nargs ? args[arg_index] : NULL);

		if(channel_change_mode(channel, *ptr, arg, add_mode))
			++arg_index;
	}

	g_debug("channel modes: %u", channel->modes);

	if(channel->key)
		g_debug("channel key: %s", channel->key);

	if(channel->limit)
		g_debug("channel limit: %d", channel->limit);

	for(GSList *i = channel->bans; i; i = i->next)
		g_debug("channel ban: %s", (char *) i->data);

	for(GSList *i = channel->users; i; i = i->next)
	{
		User *user = i->data;
		g_debug("channel user mode: %s: %d",
				user->nickname,
				channel_get_user_modes(channel, user));
	}
}

static void irc_change_user_modes(User *user, const char *modes)
{
	gboolean add_mode = TRUE;
	for(const char *ptr = modes; *ptr; ++ptr)
	{
		if(*ptr == '+')
		{
			add_mode = TRUE;
			continue;
		} else if(*ptr == '-')
		{
			add_mode = FALSE;
			continue;
		}

		user_change_mode(user, *ptr, add_mode);
	}

	g_debug("user mode: %u", user->modes);
}

static void on_server(IrcMsg *msg)
{
	if(!Registered)
	{
		// Not registered yet, so this must be remote's authentication
		// message

		if(strcmp(msg->args[2], ConfRemoteRecvPass) != 0)
		{
			fprintf(stderr, "Password mismatch from remote\n");
			exit(1);
		}

		irc_burst_state();

		Registered = 1;
	}

	server_new(msg->args[1], strtol(msg->args[3], NULL, 10) + 1,
			msg->args[4]);
}

static void on_ping(IrcMsg *msg)
{
	sock_writef(IrcSock, ":%s PONG :%s\r\n", ConfServerName, msg->args[1]);
}

static void on_burst(IrcMsg *msg)
{
}

static void on_endburst(IrcMsg *msg)
{
}

static void on_nick(IrcMsg *msg)
{
	if(msg->nargs == 2)
	{
		// Existing user changing nickname
		User *user = user_find(msg->source);
		g_assert(user != NULL);
		user_set_nickname(user, msg->args[1]);
	} else if(msg->nargs == 9)
	{
		// Remote server introducing a new user
		User *user = user_find(msg->args[2]);

		if(user)
		{
			// Nickname collision
			sock_writef(IrcSock,
					":%s KILL %s :Nickname collision\r\n",
					ConfServerName, msg->args[2]);
			user_delete(user);
		} else
		{
			user = user_new(msg->args[2]);
			user->server = server_find(msg->source);
			g_assert(user->server != NULL);
			user->timestamp = strtoul(msg->args[1], NULL, 10);
			user->hostname = g_strdup(msg->args[3]);
			user->vhostname = g_strdup(msg->args[4]);
			user->username = g_strdup(msg->args[5]);
			user->modes = user_modes_from_string(msg->args[6]);
			user->ip = g_strdup(msg->args[7]);
			user->realname = g_strdup(msg->args[8]);
		}
	} else
		g_assert_not_reached();
}

static void on_fjoin(IrcMsg *msg)
{
	Channel *channel = channel_find(msg->args[1]);

	time_t their_ts = strtoul(msg->args[2], NULL, 10);
	time_t our_ts = (channel ? channel->timestamp : their_ts + 1);

	if(channel && our_ts > their_ts)
	{
		/* XXX: clear_modes below will forget this
		for(GSList *i = channel->users; i; i = i->next)
		{
			User *user = i->data;
			int user_modes = channel_get_user_modes(channel, user);
			if(user_modes & CHANNEL_USER_MODE_VOICE)
				sock_writef(IrcSock, ":%s MODE %s +v %s\r\n",
						user->nickname, channel->name,
						user->nickname);
		}*/

		channel_clear_modes(channel);
		channel->timestamp = their_ts;
	}

	if(!channel)
	{
		channel = channel_new(msg->args[1]);
		channel->timestamp = their_ts;
	}

	char *ptr = msg->args[3];
	for(;;)
	{
		// Format is:
		// [<prefix>],<nickname> [[<prefix>],<nickname>]...

		char *prefix = ptr;

		while(*ptr && *ptr != ',')
			++ptr;

		if(!*ptr)
			break;

		*ptr++ = '\0';

		char *nickname = ptr;

		while(*ptr && *ptr != ' ')
			++ptr;

		if(*ptr)
			*ptr++ = '\0';

		if(*nickname)
		{
			User *user = user_find(nickname);
			g_assert(user != NULL);

			for(GSList *i = channel->users; i; i = i->next)
				g_assert(i->data != user);

			ChannelUserModes user_modes = (our_ts > their_ts ?
					channel_user_modes_from_prefix(prefix)
					: 0);

			channel_push_user(channel, user);
			channel_set_user_modes(channel, user, user_modes);
		}
	}
}

static void on_join(IrcMsg *msg)
{
	User *user = user_find(msg->source);
	g_assert(user != NULL);

	gchar **channels = g_strsplit(msg->args[1], ",", 0);

	for(int i = 0; channels[i]; ++i)
	{
		if(*channels[i])
		{
			Channel *channel = channel_find(channels[i]);
			g_assert(channel != NULL);
			channel_push_user(channel, user);
		}
	}

	g_strfreev(channels);
}

static void on_part(IrcMsg *msg)
{
	User *user = user_find(msg->source);
	g_assert(user != NULL);

	gchar **channels = g_strsplit(msg->args[1], ",", 0);

	for(int i = 0; channels[i]; ++i)
	{
		if(*channels[i])
		{
			Channel *channel = channel_find(channels[i]);
			g_assert(channel != NULL);
			channel_pop_user(channel, user);
		}
	}

	g_strfreev(channels);
}

static void on_kick(IrcMsg *msg)
{
	Channel *channel = channel_find(msg->args[1]);
	g_assert(channel != NULL);

	User *user = user_find(msg->args[2]);
	g_assert(channel != NULL);

	channel_pop_user(channel, user);
}

static void on_fmode(IrcMsg *msg)
{
	time_t their_ts = strtoul(msg->args[2], NULL, 10);

	Channel *channel = channel_find(msg->args[1]);
	if(channel)
	{
		if(channel->timestamp < their_ts)
		{
			g_warning("Ignoring FMODE: %lu (ours) > %lu (theirs)",
					channel->timestamp, their_ts);
			return;
		}

		irc_change_channel_modes(channel, msg->args[3],
				(const char **) &msg->args[4], msg->nargs - 4);

		return;
	}

	User *user = user_find(msg->args[1]);
	if(user)
	{
		if(user->timestamp < their_ts)
		{
			g_warning("Ignoring FMODE: %lu (ours) > %lu (theirs)",
					user->timestamp, their_ts);
			return;
		}

		irc_change_user_modes(user, msg->args[3]);

		return;
	}
}

static void on_mode(IrcMsg *msg)
{
	Channel *channel = channel_find(msg->args[1]);
	if(channel)
	{
		irc_change_channel_modes(channel, msg->args[2],
				(const char **) &msg->args[3], msg->nargs - 3);
		return;
	}

	User *user = user_find(msg->args[1]);
	if(user)
	{
		irc_change_user_modes(user, msg->args[2]);
		return;
	}
}

static void on_time(IrcMsg *msg)
{
	sock_writef(IrcSock, ":%s TIME %s %s %ld\r\n", ConfServerName,
			msg->source, msg->args[2], time(NULL));
}

