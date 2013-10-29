#include "irc_types.h"

inline gchar *user_modes_to_string(UserModes modes)
{
	gchar *string, *ptr;
	string = ptr = g_malloc0(32);
	if(modes & USER_MODE_INVISIBLE) *ptr++ = 'i';
	if(modes & USER_MODE_SNOTICE) *ptr++ = 's';
	if(modes & USER_MODE_WALLOPS) *ptr++ = 'w';
	if(modes & USER_MODE_OPERATOR) *ptr++ = 'o';
	return string;
}

inline UserModes user_modes_from_string(const char *s)
{
	UserModes modes = 0;
	while(*s)
	{
		switch(*s)
		{
			case '+': break;
			case '-': break;
			case 'i': modes |= USER_MODE_INVISIBLE; break;
			case 'n': modes |= USER_MODE_SNOTICE_MASK; break;
			case 's': modes |= USER_MODE_SNOTICE; break;
			case 'w': modes |= USER_MODE_WALLOPS; break;
			case 'o': modes |= USER_MODE_OPERATOR; break;
			default: g_assert_not_reached();
		};
		++s;
	}
	return modes;
}

inline gchar *server_notice_modes_to_string(ServerNoticeModes modes)
{
	gchar *string, *ptr;
	string = ptr = g_malloc0(32);
	if(modes & SERVER_NOTICE_MODE_REMOTE_CONNECT) *ptr++ = 'C';
	if(modes & SERVER_NOTICE_MODE_LOCAL_CONNECT) *ptr++ = 'c';
	if(modes & SERVER_NOTICE_MODE_REMOTE_QUIT) *ptr++ = 'Q';
	if(modes & SERVER_NOTICE_MODE_LOCAL_QUIT) *ptr++ = 'q';
	if(modes & SERVER_NOTICE_MODE_REMOTE_KILL) *ptr++ = 'K';
	if(modes & SERVER_NOTICE_MODE_LOCAL_KILL) *ptr++ = 'k';
	if(modes & SERVER_NOTICE_MODE_LINKS) *ptr++ = 'l';
	if(modes & SERVER_NOTICE_MODE_OPERATORS) *ptr++ = 'o';
	if(modes & SERVER_NOTICE_MODE_DEBUG) *ptr++ = 'd';
	if(modes & SERVER_NOTICE_MODE_XLINES) *ptr++ = 'x';
	if(modes & SERVER_NOTICE_MODE_STATS) *ptr++ = 't';
	if(modes & SERVER_NOTICE_MODE_FLOODS) *ptr++ = 'f';
	return string;
}

inline ChannelUserModes channel_user_modes_from_prefix(const char *prefix)
{
	ChannelUserModes modes = 0;
	while(*prefix)
	{
		switch(*prefix)
		{
			case '@': modes |= CHANNEL_USER_MODE_OP; break;
			case '+': modes |= CHANNEL_USER_MODE_VOICE; break;
			case '%': modes |= CHANNEL_USER_MODE_HALFOP; break;
			case '~': modes |= CHANNEL_USER_MODE_FOUNDER; break;
			case '&': modes |= CHANNEL_USER_MODE_PROTECTED; break;
			default: g_assert_not_reached();
		};
		++prefix;
	};
	return modes;
}

inline gchar *channel_user_modes_to_prefix(ChannelUserModes modes)
{
	gchar *prefix, *ptr;
	prefix = ptr = g_malloc0(32);
	if(modes & CHANNEL_USER_MODE_OP) *ptr++ = '@';
	if(modes & CHANNEL_USER_MODE_VOICE) *ptr++ = '+';
	if(modes & CHANNEL_USER_MODE_HALFOP) *ptr++ = '%';
	if(modes & CHANNEL_USER_MODE_FOUNDER) *ptr++ = '~';
	if(modes & CHANNEL_USER_MODE_PROTECTED) *ptr++ = '&';
	return prefix;
}

