#include <stdlib.h>
#include <strings.h>

#include "channel.h"
#include "conf.h"
#include "irc.h"
#include "user.h"

static GHashTable *Channels;
static void channel_change_ban(Channel *channel, const char *arg, gboolean
		add);
static void channel_change_key(Channel *channel, const char *arg, gboolean
		add);
static void channel_change_limit(Channel *channel, const char *arg,
		gboolean add);
static void channel_change_user_modes(Channel *channel, char mode,
		const char *nickname, gboolean add);

void channel_init()
{
	Channels = g_hash_table_new(g_str_hash, g_str_equal);
}

Channel *channel_new(const char *name)
{
	Channel *channel = g_new0(Channel, 1);
	channel->name = g_strdup(name);
	channel->timestamp = time(NULL);
	channel->user_modes = g_hash_table_new(NULL, NULL);
	g_hash_table_insert(Channels, channel->name, channel);
	return channel;
}

void channel_introduce(Channel *channel)
{
	GString *user_list = g_string_new(NULL);

	for(GSList *i = channel->users; i; i = i->next)
	{
		User *user = i->data;

		if(user_list->len)
			g_string_append_c(user_list, ' ');

		ChannelUserModes modes = channel_get_user_modes(channel, user);
		gchar *modes_str = channel_user_modes_to_prefix(modes);
		g_string_append(user_list, modes_str);
		g_free(modes_str);

		g_string_append_c(user_list, ',');
		g_string_append(user_list, user->nickname);
	}

	sock_writef(IrcSock, ":%s FJOIN %s %lu :%s\r\n",
			ConfServerName, channel->name,
			channel->timestamp, user_list->str);

	g_string_free(user_list, TRUE);
}

inline Channel *channel_find(const char *name)
{
	return g_hash_table_lookup(Channels, name);
}

inline GList *channel_find_all()
{
	return g_hash_table_get_values(Channels);
}

void channel_clear_modes(Channel *channel)
{
	channel->modes = 0;
	channel->limit = 0;

	if(channel->key)
	{
		g_free(channel->key);
		channel->key = NULL;
	}

	while(channel->bans)
	{
		g_free(channel->bans->data);
		channel->bans = g_slist_delete_link(channel->bans,
				channel->bans);
	}

	for(GSList *i = channel->users; i; i = i->next)
		channel_set_user_modes(channel, i->data, 0);
}

void channel_clear_topic(Channel *channel)
{
	if(channel->topic)
	{
		g_free(channel->topic);
		channel->topic = NULL;
	}

	if(channel->topic_author)
	{
		g_free(channel->topic_author);
		channel->topic_author = NULL;
	}

	channel->topic_time = 0;
}

void channel_push_user(Channel *channel, User *user)
{
	channel->users = g_slist_append(channel->users, user);
	g_hash_table_insert(channel->user_modes, user, 0);
}

void channel_pop_user(Channel *channel, User *user)
{
	channel->users = g_slist_remove(channel->users, user);

	if(channel->users == NULL)
	{
		channel_clear_modes(channel);
		channel_clear_topic(channel);
		g_hash_table_destroy(channel->user_modes);
		g_free(channel->name);
		g_hash_table_remove(Channels, channel);
		g_free(channel);
	}
}

void channel_pop_user_from_all(User *user)
{
	GList *list = channel_find_all();
	for(GList *i = list; i; i = i->next)
	{
		Channel *channel = i->data;
		channel_pop_user(channel, user);
	}
	g_list_free(list);
}

gboolean channel_change_mode(Channel *channel, char mode, const char *arg,
		gboolean add)
{
	gboolean used_arg = FALSE;
	ChannelModes modifier = 0;

	switch(mode)
	{
		case 'i':
			modifier |= CHANNEL_MODE_INVITE_ONLY;
			break;
		case 'm':
			modifier |= CHANNEL_MODE_MODERATED;
			break;
		case 'n':
			modifier |= CHANNEL_MODE_NO_EXTERNAL;
			break;
		case 's':
			modifier |= CHANNEL_MODE_SECRET;
			break;
		case 't':
			modifier |= CHANNEL_MODE_TOPIC;
			break;
		case 'o':
		case 'v':
		case 'h':
		case 'q':
		case 'a':
			g_assert(arg != NULL);
			channel_change_user_modes(channel, mode, arg, add);
			used_arg = TRUE;
			break;
		case 'b':
			g_assert(arg != NULL);
			channel_change_ban(channel, arg, add);
			used_arg = TRUE;
			break;
		case 'k':
			if(add) { g_assert(arg != NULL); }
			channel_change_key(channel, arg, add);
			used_arg = add;
			break;
		case 'l':
			if(add) { g_assert(arg != NULL); }
			channel_change_limit(channel, arg, add);
			used_arg = add;
			break;
	}

	if(add)
		channel->modes |= modifier;
	else
		channel->modes &= ~modifier;

	return used_arg;
}

inline void channel_set_user_modes(Channel *channel, User *user,
		ChannelUserModes modes)
{
	g_hash_table_insert(channel->user_modes, user, GINT_TO_POINTER(modes));
}

inline ChannelUserModes channel_get_user_modes(Channel *channel, User *user)
{
	return GPOINTER_TO_INT(g_hash_table_lookup(channel->user_modes, user));
}

static void channel_change_ban(Channel *channel, const char *arg, gboolean add)
{
	if(add)
	{
		channel->bans = g_slist_append(channel->bans, g_strdup(arg));
	} else
	{
		for(GSList *i = channel->bans; i; i = i->next)
		{
			if(strcasecmp(i->data, arg) == 0)
			{
				g_free(i->data);
				channel->bans = g_slist_delete_link(
						channel->bans, i);
			}
		}
	}
}

static void channel_change_key(Channel *channel, const char *arg, gboolean add)
{
	if(channel->key)
		g_free(channel->key);

	channel->key = (add ? g_strdup(arg) : NULL);
}

static void channel_change_limit(Channel *channel, const char *arg,
		gboolean add)
{
	channel->limit = (add ? strtoul(arg, NULL, 10) : 0);
}

static void channel_change_user_modes(Channel *channel, char mode,
		const char *nickname, gboolean add)
{
	User *user = user_find(nickname);
	g_assert(user != NULL);

	ChannelUserModes modifier = 0;

	switch(mode)
	{
		case 'o':
			modifier |= CHANNEL_USER_MODE_OP;
			break;
		case 'v':
			modifier |= CHANNEL_USER_MODE_VOICE;
			break;
		case 'h':
			modifier |= CHANNEL_USER_MODE_HALFOP;
			break;
		case 'q':
			modifier |= CHANNEL_USER_MODE_FOUNDER;
			break;
		case 'a':
			modifier |= CHANNEL_USER_MODE_PROTECTED;
			break;
	}

	ChannelUserModes user_modes = channel_get_user_modes(channel, user);

	if(add)
		user_modes |= modifier;
	else
		user_modes &= ~modifier;

	channel_set_user_modes(channel, user, user_modes);
}

