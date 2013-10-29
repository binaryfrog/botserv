#include "user.h"
#include "irc.h"
#include "conf.h"
#include "channel.h"

static GHashTable *Users;

void user_init()
{
	Users = g_hash_table_new(g_str_hash, g_str_equal);
}

User *user_new(const char *nickname)
{
	User *user = g_new0(User, 1);
	user->nickname = g_strdup(nickname);
	user->timestamp = time(NULL);
	g_hash_table_insert(Users, user->nickname, user);
	return user;
}

void user_delete(User *user)
{
	channel_pop_user_from_all(user);

	g_free(user->nickname);
	g_free(user->hostname);
	g_free(user->vhostname);
	g_free(user->username);
	g_free(user->ip);
	g_free(user->realname);

	g_hash_table_remove(Users, user);
	g_free(user);
}

inline User *user_find(const char *nickname)
{
	return g_hash_table_lookup(Users, nickname);
}

inline GList *user_find_all()
{
	return g_hash_table_get_values(Users);
}

void user_introduce(User *user)
{
	gchar *modes = user_modes_to_string(user->modes);
	sock_writef(IrcSock, ":%s NICK %lu %s %s %s %s +%s %s :%s\r\n",
			user->server->name, user->timestamp, user->nickname,
			user->hostname, user->vhostname, user->username,
			modes, user->ip, user->realname);
	g_free(modes);

	if(user->snotice_modes)
	{
		modes = server_notice_modes_to_string(user->snotice_modes);
		sock_writef(IrcSock, ":%s FMODE %s %lu +n %s\r\n",
				user->nickname, user->nickname,
				user->timestamp, modes);
		g_free(modes);
	}
}

void user_set_nickname(User *user, const char *nickname)
{
	g_hash_table_remove(Users, user);

	if(user->nickname)
		g_free(user->nickname);

	user->nickname = g_strdup(nickname);
	g_hash_table_insert(Users, user->nickname, user);
}

void user_change_mode(User *user, char mode, gboolean add)
{
	UserModes modifier = 0;

	switch(mode)
	{
		case 'o':
			modifier |= USER_MODE_OPERATOR;
			break;
		case 'i':
			modifier |= USER_MODE_INVISIBLE;
			break;
		case 'n':
			modifier |= USER_MODE_SNOTICE_MASK;
			break;
		case 's':
			modifier |= USER_MODE_SNOTICE;
			break;
		case 'w':
			modifier |= USER_MODE_WALLOPS;
			break;
	}

	if(add)
		user->modes |= modifier;
	else
		user->modes &= ~modifier;
}

