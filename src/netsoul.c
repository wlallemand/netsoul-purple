/*
 * @file netsoul.c
 *
 * gaim-netsoul Protocol Plugin
 *
 * Copyright (C) 2004, 2007, Edward Hervey <bilboed@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "netsoul.h"

static GaimPlugin *_netsoul_plugin = NULL;

/*
  netsoul_list_icon
  Returns the name of the icon to use for netsoul
*/

static const char *netsoul_list_icon (GaimAccount *account, GaimBuddy *buddy)
{
  return "netsoul";
}

/*
  netsoul_status_text
  Returns the text to display next to the icon in the contact list
*/

static char *netsoul_status_text(GaimBuddy *gb)
{
  NetsoulBuddy	*nb = gb->proto_data;

  if (!nb)
    return NULL;
  gaim_debug_info("netsoul", "status_text %s\n", nb->login);
  if ((nb->state == NS_STATE_AWAY) || (nb->state == NS_STATE_IDLE))
    return g_strdup("Away");
  if (nb->state == NS_STATE_SERVER)
    return g_strdup("Server");
  if (nb->state == NS_STATE_LOCK)
    return g_strdup("Lock");
  if (nb->state == NS_STATE_ACTIF_MORE)
    return g_strdup("Actif+");
  if (nb->state == NS_STATE_SEVERAL_ACTIF)
    return g_strdup("Several Active");
  if (nb->state == NS_STATE_SEVERAL_INACTIF)
    return g_strdup("Away+");
  if (nb->state == NS_STATE_CONNECTION)
    return g_strdup("Connection");
  return NULL;
}

/*
  netsoul_conn_text
  Returns the description of the given netsoul connection
  used in netsoul_tooltip_text
*/

char *netsoul_conn_text(NetsoulConn *nc)
{
  char	*loggedin;
  char	*statetime;
  char	*state;
  char	*resp;

  loggedin = ns_readable_time(nc->logintime);
  statetime = ns_readable_time(nc->statetime);
  state = ns_state_to_text(nc->state);
  resp = g_strdup_printf("\n<b>Location:</b> %s\n<b>IP:</b> %s\n<b>Comment:</b> %s\n<b>Logged in:</b> %s<b>State:</b> %s\n  <b>since:</b> %s",
			 nc->location, nc->ip, nc->comment, loggedin, state, statetime);
  g_free(loggedin);
  g_free(statetime);
  g_free(state);
  return resp;
}

char *netsoul_conn_text_html(NetsoulConn *nc)
{
  char	*loggedin;
  char	*statetime;
  char	*state;
  char	*resp;

  loggedin = ns_readable_time(nc->logintime);
  statetime = ns_readable_time(nc->statetime);
  state = ns_state_to_text(nc->state);
  resp = g_strdup_printf("<b>Location:</b> %s<br><b>IP:</b> %s<br><b>Comment:</b> %s<br><b>Logged in:</b> %s<br><b>State:</b> %s<br>  <b>since:</b> %s",
			 nc->location, nc->ip, nc->comment, loggedin, state, statetime);
  g_free(loggedin);
  g_free(statetime);
  g_free(state);
  return resp;
}


/*
  netsoul_tooltip_text
  Returns the content of the tooltip for the given buddy
*/

static void netsoul_tooltip_text(GaimBuddy *gb, GaimNotifyUserInfo *user_info, gboolean full)
{
  NetsoulBuddy	*nb = gb->proto_data;
  char	*resp, *nctxt;
  GList	*tmp;
  NetsoulConn	*nc;
  char	**tab;
  int	i;
  GaimConnection *gc = gaim_account_get_connection (gaim_buddy_get_account(gb));
  NetsoulData *ns = gc->proto_data;
  GaimBuddyIcon *icon = gaim_buddy_get_icon (gb);

  gaim_debug_info("netsoul", "netsoul_tooltip_text %s icon_type: %s\n",
		  gb->name, gaim_buddy_icon_get_type(icon));
  if (nb == NULL)
  {
    nb = g_new0(NetsoulBuddy, 1);
    gb->proto_data = nb;
    nb->login = g_strdup(gb->name);
    ns_watch_buddy(gc, gb);
    // watch_log_user
    ns_watch_log_user(gc);
    ns_list_users(gc, ns->watchlist);
  }
  if (nb->nblocations == 0)
    return;
  tab = g_new0(char *, nb->nblocations + 1);
  for (i = 0, tmp = nb->locationlist; tmp; tmp = tmp->next, i++) {
    nc = tmp->data;
    tab[i] = netsoul_conn_text(nc);
  }
  nctxt = g_strjoinv("", tab);
  g_strfreev(tab);
  resp = g_strdup_printf("\n<b>Status:</b> %s\n<b>Groupe:</b> %s\n<b>Connections:</b>%s",
			 ns_state_to_text(nb->state), nb->group, nctxt);
  g_free(nctxt);
  user_info = gaim_notify_user_info_new();
  gaim_notify_user_info_add_pair(user_info, "Informations", resp);
  g_free(resp);
}

/*
  netsoul_away_states
  Returns the list of possible away states
*/

static GList * netsoul_away_states (GaimAccount* account)
{
  GList	*types;
  GaimStatusType* status;

  types = NULL;
  status = gaim_status_type_new_full(GAIM_STATUS_AVAILABLE,
				     NULL, NULL, TRUE, TRUE, FALSE);
  types = g_list_append(types, status);
  status = gaim_status_type_new_full(GAIM_STATUS_AVAILABLE,
				     "actif", NULL, FALSE, TRUE, FALSE);
  types = g_list_append(types, status);

  status = gaim_status_type_new_full(GAIM_STATUS_AWAY,
				     "away", NULL, FALSE, TRUE, FALSE);
  types = g_list_append(types, status);

  status = gaim_status_type_new_full(GAIM_STATUS_AWAY,
				     "lock", NULL, FALSE, TRUE, FALSE);
  types = g_list_append(types, status);

  status = gaim_status_type_new_full(GAIM_STATUS_OFFLINE,
				     NULL, NULL, TRUE, TRUE, FALSE);
  types = g_list_append(types, status);


  return (types);
}

/*
  netsoul_set_away
  Sets the account in away mode
*/

static void netsoul_set_away(GaimAccount *account, GaimStatus* status)
{
  int	ns_state;
  GaimPresence* state = gaim_status_get_presence (status);

  if (gaim_presence_is_available (state))
    ns_state = NS_STATE_ACTIF;
  else if (gaim_presence_is_idle (state))
    ns_state = NS_STATE_IDLE;
  else
    ns_state = NS_STATE_AWAY;
  ns_send_state(gaim_account_get_connection (account), ns_state, time(NULL));
}

/*
  netsoul_set_idle
  Set the account to idle
*/

static void netsoul_set_idle(GaimConnection *gc, int idletime)
{
  gaim_debug_info("netsoul", "netsoul_set_idle. idletime:%d\n", idletime);
}

/*
  netsoul_close
  Closes the account
*/

static void netsoul_close (GaimConnection *gc)
{
  GaimBlistNode *gnode, *cnode, *bnode;
  NetsoulData	*ns = gc->proto_data;

  gaim_debug_info("netsoul", "netsoul_close\n");

  for(gnode = gaim_get_blist()->root; gnode; gnode = gnode->next) {
    if(!GAIM_BLIST_NODE_IS_GROUP(gnode)) continue;
    for(cnode = gnode->child; cnode; cnode = cnode->next) {
      if(!GAIM_BLIST_NODE_IS_CONTACT(cnode)) continue;
      for(bnode = cnode->child; bnode; bnode = bnode->next) {
	if(!GAIM_BLIST_NODE_IS_BUDDY(bnode)) continue;
	if(((GaimBuddy*)bnode)->account == gc->account)
	{
	  GaimBuddy *buddy = (GaimBuddy*)bnode;
	  gaim_buddy_icon_uncache(buddy);
	}
      }
    }
  }

  g_free(ns->challenge);
  g_free(ns->host);
  close(ns->fd);
  g_free(ns);
  gaim_input_remove(gc->inpa);
}

/*

 */
static void netsoul_got_photo (GaimUtilFetchUrlData *url, void *user_data,
			       const char *photo, size_t len, const char *error_msg)
{
  int id;
  GaimBuddy *gb = user_data;
  GaimAccount *account = gaim_buddy_get_account (gb);

  // Check if connection is still existing
  GaimConnection *gc = gaim_account_get_connection (account);
  if (gc == NULL)
    return;
  gaim_debug_info ("netsoul", "netsoul_got_photo (size: %d) for %s\n",
		   len,
		   gb->name);
  if (len)
  {
    id = gaim_imgstore_add (photo, len, NULL);
    GaimStoredImage *imgstore = gaim_imgstore_get (id);
    gpointer img = gaim_imgstore_get_data (imgstore);
    gaim_buddy_icons_set_for_user (account, gb->name, img, len);
  }
}

/*
  netsoul_add_buddy
  Add the given buddy to the contact list
*/

static void netsoul_add_buddy (GaimConnection *gc, GaimBuddy *buddy, GaimGroup *group)
{
  NetsoulData *ns= gc->proto_data;
  NetsoulBuddy	*nb;
  gchar		*photo = NULL;

  gaim_debug_info("netsoul", "netsoul_add_buddy %s\n", buddy->name);
  nb = g_new0(NetsoulBuddy, 1);
  buddy->proto_data = nb;
  nb->login = g_strdup(buddy->name);
  // Get photo
  photo = g_strdup_printf("%s%s", NETSOUL_PHOTO_URL, buddy->name);
  gaim_util_fetch_url(photo, TRUE, NULL, FALSE, netsoul_got_photo, buddy);
  // if contact is not already is watch list, add it
  ns_watch_buddy(gc, buddy);
  // watch_log_user
  ns_watch_log_user(gc);
  ns_list_users(gc, ns->watchlist);
}

/*
  netsoul_add_buddies
  Add the given buddies to the contact list
*/

static void netsoul_add_buddies(GaimConnection *gc, GList *buddies, GList *groups)
{
  GList	*tmp;
  NetsoulData *ns = gc->proto_data;
  NetsoulBuddy	*nb;
  GaimBuddy	*gb;

  gaim_debug_info("netsoul", "netsoul_add_buddies\n");
  // for each contact
  for (tmp = buddies; tmp; tmp = tmp->next) {
  //   if contact is not already in watch list add it
    gb = (GaimBuddy *) tmp->data;
    nb = g_new0(NetsoulBuddy, 1);
    nb->login = g_strdup(gb->name);
    gb->proto_data = nb;
    ns_watch_buddy(gc, gb);
  }
  // watch_log_user
  ns_watch_log_user(gc);
  ns_list_users(gc, ns->watchlist);
}


/*
  ns_get_buddies
  Add buddies to watchlist
*/
void netsoul_get_buddies (GaimConnection* gc)
{
  GaimBlistNode *gnode, *cnode, *bnode;

  gaim_debug_info("netsoul", "ns_get_buddies\n");

  for(gnode = gaim_get_blist()->root; gnode; gnode = gnode->next) {
    if(!GAIM_BLIST_NODE_IS_GROUP(gnode)) continue;
    for(cnode = gnode->child; cnode; cnode = cnode->next) {
      if(!GAIM_BLIST_NODE_IS_CONTACT(cnode)) continue;
      for(bnode = cnode->child; bnode; bnode = bnode->next) {
	if(!GAIM_BLIST_NODE_IS_BUDDY(bnode)) continue;
	if(((GaimBuddy*)bnode)->account == gc->account)
	{
	  GaimBuddy *buddy = (GaimBuddy*)bnode;
	  gchar *photo = NULL;
	  gaim_debug_info("netsoul", "netsoul_add_buddy %s\n", buddy->name);

	  NetsoulBuddy *nb = g_new0(NetsoulBuddy, 1);
	  buddy->proto_data = nb;
	  nb->login = g_strdup(buddy->name);
	  // Get photo
	  photo = g_strdup_printf("%s%s", NETSOUL_PHOTO_URL, buddy->name);
	  gaim_util_fetch_url(photo, TRUE, NULL, FALSE, netsoul_got_photo, buddy);
	  // if contact is not already is watch list, add it
	  ns_watch_buddy(gc, buddy);
	}
      }
    }
  }
  // watch_log_user
  NetsoulData *ns = gc->proto_data;
  ns_watch_log_user(gc);
  ns_list_users(gc, ns->watchlist);
}


/*
  netsoul_remove_buddy
  Remove the given buddy from the contact list
*/

static void netsoul_remove_buddy (GaimConnection *gc, GaimBuddy *buddy, GaimGroup *group)
{
  NetsoulBuddy	*nb;
  NetsoulConn	*nc;
  NetsoulData	*ns = gc->proto_data;
  GList		*tmp;

  gaim_debug_info("netsoul", "netsoul_remove_buddy\n");
  nb = buddy->proto_data;
  // remove buddy from watchlist
  if ((tmp = g_list_find_custom(ns->watchlist, nb->login, (GCompareFunc) g_ascii_strcasecmp)))
    ns->watchlist = g_list_delete_link(ns->watchlist, tmp);
  g_free(nb->login);
  if (nb->group)
    g_free(nb->group);
  for (tmp = nb->locationlist; tmp; tmp = tmp->next) {
    nc = (NetsoulConn *) tmp->data;
    g_free(nc->ip);
    g_free(nc->location);
    g_free(nc->comment);
  }
  g_list_free(nb->locationlist);
  g_free(nb);
}

/*
  netsoul_send_im
  Send a message to the given person
*/

static int netsoul_send_im (GaimConnection *gc, const char *who, const char *what, GaimMessageFlags flags)
{
  gaim_debug_info("netsoul", "netsoul_send_im\n");
  ns_msg_user(gc, who, what);
  return 1;
}

static void netsoul_keepalive(GaimConnection *gc)
{
  NetsoulData	*ns = gc->proto_data;

  if (netsoul_write(ns, "ping\n") < 0) {
    gaim_debug_warning("netsoul", "Error sending ping\n");
  }
}

static int netsoul_send_typing(GaimConnection *gc, const char *name, GaimTypingState typing)
{
  gaim_debug_info("netsoul", "netsoul_send_typing\n");
  ns_send_typing(gc, name, typing);
  return 1;
}

static void netsoul_get_info(GaimConnection *gc, const char *who)
{
  GaimBuddy	*gb;
  NetsoulBuddy	*nb;
  char	*primary;
  char	*text;
  char	*title;
  char	**tab;
  int	i;
  GList	*tmp;
  NetsoulConn	*nc;

  gaim_debug_info("netsoul", "netsoul_get_info %s\n", who);
  if (!(gb = get_good_stored_buddy(gc, (char *) who))) {
    GaimNotifyUserInfo *user_info;
    user_info = gaim_notify_user_info_new();
    gaim_notify_user_info_add_pair(user_info,"Error", "No Info about this user!");
    gaim_notify_userinfo(gc, who, user_info, NULL, NULL);
    gaim_notify_user_info_destroy(user_info);
    return;
  }
  nb = gb->proto_data;

  tab = g_new0(char *, nb->nblocations + 1);
  for (i = 0, tmp = nb->locationlist; tmp; tmp = tmp->next, i++) {
    nc = tmp->data;
    tab[i] = netsoul_conn_text_html(nc);
  }
  text = g_strjoinv("<br>", tab);
  g_strfreev(tab);

  primary = g_strdup_printf("<b>Status:</b> %s<br><b>Groupe:</b> %s<hr>%s",
			    ns_state_to_text(nb->state), nb->group, text);
  title = g_strdup_printf("Info for %s", who);

  GaimNotifyUserInfo *user_info;
  user_info = gaim_notify_user_info_new();
  gaim_notify_user_info_add_pair(user_info, title, primary);
  gaim_notify_userinfo(gc, gb->name, user_info, NULL, NULL);
  gaim_notify_user_info_destroy(user_info);
  g_free(primary);
  g_free(text);
}

/*
  netsoul_list_emblem
  Add little emblems on buddy icon
*/

static void netsoul_list_emblems(GaimBuddy *buddy,
				 const char **se, const char **sw,
				 const char **nw, const char **ne)
{

  NetsoulBuddy	*nb = buddy->proto_data;
  char *emblems[4] = {NULL, NULL, NULL, NULL};
  int i = 0;

  if (!nb)
    return;
  gaim_debug_info("netsoul", "list_emblems %s\n", nb->login);
  if ((nb->state == NS_STATE_AWAY) || (nb->state == NS_STATE_IDLE))
    emblems[i++] = "away";
  if (nb->state == NS_STATE_SEVERAL_INACTIF)
    emblems[i++] == "extendedaway";
  if ((nb->state == NS_STATE_SERVER) || (nb->state == NS_STATE_LOCK))
    emblems[i++] = "secure";
  if ((nb->state == NS_STATE_SEVERAL_ACTIF) || (nb->state == NS_STATE_ACTIF_MORE))
    emblems[i++] = "activebuddy";

  *se = emblems[0];
  *sw = emblems[1];
  *nw = emblems[2];
  *ne = emblems[3];

}

/*
** Si on a un clic droit sur un buddy dans la buddy liste,
** on va rajouter quelques champ dans le menu
*/
static GList *netsoul_blist_node_menu(GaimBlistNode *node)
{
  if (GAIM_BLIST_NODE_IS_BUDDY(node))
    {
      return ns_buddy_menu((GaimBuddy *) node);
    }
  else
    return NULL;
}

static void netsoul_join_chat(GaimConnection *gc, GHashTable *components)
{
   gaim_debug_info("netsoul", "join_chat\n");
}

static void netsoul_reject_chat(GaimConnection *gc, GHashTable *components)
{
  gaim_debug_info("netsoul", "reject_chat\n");
}

static void netsoul_chat_invite(GaimConnection *gc, int id, const char *who, const char *message)
{
  gaim_debug_info("netsoul", "chat_invite\n");
}

static int netsoul_chat_send(GaimConnection *gc, int id, const char *message)
{
  gaim_debug_info("netsoul", "chat_send\n");
  return 0;
}

static GaimPluginProtocolInfo prpl_info =
{
    OPT_PROTO_MAIL_CHECK,    /* options          */
    NULL,                           /* user_splits      */
    NULL,                           /* protocol_options */
    {"jpeg", 0, 0, 96, 96, GAIM_ICON_SCALE_DISPLAY},                 /* icon_spec        */
    netsoul_list_icon,              /* list_icon        */
    netsoul_list_emblems,           /* list_emblems     */
    netsoul_status_text,            /* status_text      */
    netsoul_tooltip_text,           /* tooltip_text     */
    netsoul_away_states,            /* away_states      */
    netsoul_blist_node_menu,        /* blist_node_menu  */
    NULL,                           /* chat_info        */
    NULL,                           /* chat_info_defaults */
    netsoul_login,                  /* login            */
    netsoul_close,                  /* close            */
    netsoul_send_im,                /* send_im          */
    NULL,                           /* set_info         */
    netsoul_send_typing,            /* send_typing      */
    netsoul_get_info,               /* get_info         */
    netsoul_set_away,               /* set_away         */
    netsoul_set_idle,               /* set_idle         */
    NULL,                           /* change_password  */
    netsoul_add_buddy,              /* add_buddy        */
    netsoul_add_buddies,            /* add_buddies      */
    netsoul_remove_buddy,           /* remove_buddy     */
    NULL,                           /* remove_buddies   */
    NULL,                           /* add_permit       */
    NULL,                           /* add_deny         */
    NULL,                           /* rem_permit       */
    NULL,                           /* rem_deny         */
    NULL,                           /* set_permit_deny  */
    NULL/*netsoul_join_chat*/,              /* join_chat        */
    NULL/*netsoul_reject_chat*/,            /* reject_chat      */
    NULL,				    /* get_chat_name	*/
    NULL/*netsoul_chat_invite*/,            /* chat_invite      */
    NULL,                           /* chat_leave       */
    NULL,                           /* chat_whisper     */
    NULL /*netsoul_chat_send*/,              /* chat_send        */
    netsoul_keepalive,              /* keepalive        */
    NULL,                           /* register_user    */
    NULL,                           /* get_cb_info      */
    NULL,                           /* get_cb_away      */
    NULL,                           /* alias_buddy      */
    NULL,                           /* group_buddy      */
    NULL,                           /* rename_group     */
    NULL,                           /* buddy_free       */
    NULL,                           /* convo_closed     */
    NULL,                           /* normalize        */
    NULL,                           /* set_buddy_icon   */
    NULL,                           /* remove_group     */
    NULL,                           /* get_cb_real_name */
    NULL,                           /* set_chat_topic   */
    NULL,                           /* find_blist_chat  */
    NULL,                           /* roomlist_get_list*/
    NULL,                           /* roomlist_cancel  */
    NULL,                           /* roomlist_expand_catagory */
    NULL,                           /* can_receive_file */
    NULL,                           /* send_file        */
    NULL,		       	    /* new_xfer */
    NULL,			    /* offline_message */
    NULL,			    /* whiteboard_prpl_ops */
    NULL			    /* send_raw */
};


static GaimPluginInfo info =
{
    GAIM_PLUGIN_MAGIC,
    GAIM_MAJOR_VERSION,
    GAIM_MINOR_VERSION,
    GAIM_PLUGIN_PROTOCOL,           /* type           */
    NULL,                           /* ui_requirement */
    0,                              /* flags          */
    NULL,                           /* dependencies   */
    GAIM_PRIORITY_DEFAULT,          /* priority       */
    "prpl-bilboed-netsoul",                    /* id             */
    "netsoul",                         /* name           */
    NETSOUL_VERSION,                   /* version        */
    N_("Netsoul Plugin"),              /* summary        */
    N_("Allows Gaim to send messages over the Netsoul Protocol."),    /* description    */
    N_("Edward Hervey <bilboed@gmail.com>"),   /* author     */
    NETSOUL_WEBSITE,                   /* homepage       */
    NULL,                           /* load           */
    NULL,                           /* unload         */
    NULL,                           /* destroy        */
    NULL,                           /* ui_info        */
    &prpl_info,                     /* extra_info     */
    NULL,                           /* prefs_info     */
    NULL                            /* actions        */
};


static void init_plugin(GaimPlugin *plugin)
{
    GaimAccountOption *option;
    option = gaim_account_option_string_new(_("Server"), "server", NETSOUL_DEFAULT_SERVER);
    prpl_info.protocol_options = g_list_append(prpl_info.protocol_options, option);

    option = gaim_account_option_int_new(_("Port"), "port", NETSOUL_DEFAULT_PORT);
    prpl_info.protocol_options = g_list_append(prpl_info.protocol_options, option);

    option = gaim_account_option_string_new(_("Location"), "location", NETSOUL_DEFAULT_LOCATION);
    prpl_info.protocol_options = g_list_append(prpl_info.protocol_options, option);

    option = gaim_account_option_string_new(_("Comment"), "comment", NETSOUL_DEFAULT_COMMENT);
    prpl_info.protocol_options = g_list_append(prpl_info.protocol_options, option);

    _netsoul_plugin = plugin;
};


GAIM_INIT_PLUGIN(netsoul, init_plugin, info);
