/*
 * @file ns_listen.c
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
#include <string.h>
#include "netsoul.h"

void	ns_use_rep(GaimConnection *gc, char **msg)
{
  NetsoulData	*ns = gc->proto_data;

  if (ns->state == NS_STATE_SENT_EXTUSERLOG) {
    if (atoi(*msg) == 2) {
      ns->state = NS_STATE_CONNECTION;
      gaim_debug_info("netsoul", "end ns_use_rep\n");
      gaim_connection_set_state(gc, GAIM_CONNECTED);
      gaim_debug_info("netsoul", "end ns_use_rep\n");
      //serv_finish_login(gc);
      ns_send_state(gc, NS_STATE_ACTIF, time(NULL));
      ns_list_users(gc, ns->watchlist);
      return;
    }
    gaim_connection_error(gc, _("Bad Authentification"));
  }
}

/*
  get_good_stored_buddy
  fullname : compte name (<login>@<location>)
  returns : The good GaimBuddy if it exists, or NULL if it doesn't
*/

GaimBuddy	*get_good_stored_buddy(GaimConnection *gc, char *fullname)
{
  char	**login;
  GaimBuddy	*gb;

  login = g_strsplit(fullname, "@", 2);
  if (!(gb = gaim_find_buddy(gaim_connection_get_account(gc), fullname)))
    gb = gaim_find_buddy(gaim_connection_get_account(gc), *login);
  g_strfreev(login);
  return gb;
}

/*
  find_conn_id
  nb : a NetsoulBuddy
  id : a connection id
  returns : the buddy's connection corresponding to the given id, or NULL if there isn't any
*/

NetsoulConn	*find_conn_id(NetsoulBuddy *nb, int id)
{
  GList	*tmp;
  NetsoulConn	*nc;

  for (tmp = nb->locationlist; tmp; tmp = tmp->next) {
    nc = tmp->data;
    if (nc->id == id)
      return nc;
  }
  return NULL;
}

void	ns_buddy_got_msg(GaimConnection *gc, char *who, char *msg)
{
  GaimBuddy	*gb;
  char	*msgdec;
  char	*towho;

  if (!*msg)
    return;
  if (!(gb = get_good_stored_buddy(gc, who)))
    towho = who;
  else
    towho = gb->name;

  msgdec = url_decode(msg);
  if (msgdec)
    serv_got_im(gc, towho, msgdec, 0, time(NULL));
  else
    {
      gaim_debug_warning("netsoul", "msgdecoded == NULL\n");
      serv_got_im(gc, towho, msg, 0, time(NULL));
    }
  g_free(msgdec);
}

void	ns_buddy_typing_notification(GaimConnection *gc, char *who, int typing_state)
{
  GaimBuddy	*gb;
  char	*towho;

  gaim_debug_info("netsoul", "l'utilisateur %s est en train de taper un msg \n", who);
  if (!(gb = get_good_stored_buddy(gc, who)))
    towho = who;
  else
    towho = gb->name;

  if (typing_state)
    serv_got_typing(gc, towho, time(NULL), GAIM_TYPING);
  else
    serv_got_typing_stopped (gc, towho);
}


void	ns_buddy_got_user_state(GaimConnection *gc, char **who, char *state)
{
  char	**tab, *speclogin;
  GaimBuddy	*gb;
  NetsoulBuddy	*nb;
  NetsoulConn	*nc;

  tab = g_strsplit(state, ":", 0);
  speclogin = convertname(who);

  // get the gaimbuddy
  if (!(gb = get_good_stored_buddy(gc, speclogin))) {
      g_strfreev(tab);
      g_free(speclogin);
      return;
    }

  nb = gb->proto_data;
  // find the corresponding id
  if (!(nc = find_conn_id(nb, atoi(*who)))) {
    // not possible
    g_strfreev(tab);
    g_free(speclogin);
    return;
  }

  // if id exists in list, update it
  nc->state = ns_text_to_state(*tab);
  if (tab[1])
    nc->statetime = atol(tab[1]);
  else
    nc->statetime = time(NULL);

  g_strfreev(tab);
  // update buddy state
  ns_compute_update_state(gc, gb);
}

void	ns_user_update(GaimConnection *gc, char **msg)
{
  GaimBuddy	*gb;
  NetsoulBuddy	*nb;
  NetsoulConn	*nc;
  char		**tab;
  char		**msg2;
  char		*speclogin;

  msg2 = g_strsplit(msg[1], " ", 0);
  speclogin = g_strdup_printf("%s@%s", *msg2, url_decode(msg2[7]));
  gaim_debug_info("netsoul", "ns_user_update : %s[%s]\n", speclogin, *msg);
  // get the gaimbuddy
  if (!(gb = get_good_stored_buddy(gc, speclogin))) {
      g_free(speclogin);
      g_strfreev(msg2);
      return;
    }
  nb = gb->proto_data;
  tab = g_strsplit(msg2[9], ":", 0);

  if (!nb->group)
    nb->group = g_strdup(msg2[8]);
  // find the corresponding id
  if (!(nc = find_conn_id(nb, atoi(*msg)))) {
    // if not, create new NetsoulConn and add it to list
    nc = g_new0(NetsoulConn, 1);
    nc->id = atoi(*msg);
    nc->logintime = atol(msg2[2]);
    gaim_debug_info("netsoul", "state time : %s\n", tab[1]);
    if (tab[1])
      nc->statetime = atol(tab[1]);
    else
      nc->statetime = time(NULL);
    nc->ip = g_strdup(msg2[1]);
    nc->location = url_decode(msg2[7]);
    nc->comment = url_decode(msg2[10]);
    nc->state = ns_text_to_state(*tab);
    nb->locationlist = g_list_append(nb->locationlist, nc);
    nb->nblocations++;
  } else {
    // if id exists in list, update it
    nc->state = ns_text_to_state(*tab);
    nc->statetime = atol(tab[1]);
  }
  g_free(speclogin);
  g_strfreev(tab);
  g_strfreev(msg2);
  // update buddy state
  ns_compute_update_state(gc, gb);
}

void	ns_buddy_got_user_login(GaimConnection *gc, char **who)
{
  ns_list_users_id(gc, atoi(*who));
}

void	ns_buddy_got_user_logout(GaimConnection *gc, char **who)
{
  GaimBuddy	*gb;
  NetsoulBuddy	*nb;
  NetsoulConn	*nc;
  char		*speclogin;

  // find the corresponding id
  speclogin = convertname(who);
  gaim_debug_info("netsoul", "ns_buddy_got_user_logout %s\n", speclogin);
  if (!(gb = get_good_stored_buddy(gc, speclogin))) {
      g_free(speclogin);
      return;
    }
  nb = gb->proto_data;
  if (!(nc = find_conn_id(nb, atoi(*who)))) {
    // not possible
    g_free(speclogin);
    return;
  } else {
    // if id exists in list, remove it
    nb->locationlist = g_list_remove(nb->locationlist, nc);
    nb->nblocations--;
  }
  // update buddy state
  ns_compute_update_state(gc, gb);
  g_free(speclogin);
}

void	ns_buddy_user_cmd(GaimConnection *gc, char **who, char *cmd)
{
  char	*nameid;
  char	**tab;

  nameid = convertname(who);
  gaim_debug_info("netsoul", "ns_buddy_user_cmd %s\n", nameid);
  tab = g_strsplit(cmd, " ", 0);
  if (!strcmp(*tab, "msg"))
    ns_buddy_got_msg(gc, nameid, tab[1]);
  else if (!strcmp(*tab, "state"))
    ns_buddy_got_user_state(gc, who, tab[1]);
  else if (!strcmp(*tab, "login"))
    ns_buddy_got_user_login(gc, who);
  else if (!strcmp(*tab, "logout"))
    ns_buddy_got_user_logout(gc, who);
  else if (!strcmp(*tab, "dotnetSoul_UserTyping"))
    ns_buddy_typing_notification(gc, nameid, 1);
  else if (!strcmp(*tab, "dotnetSoul_UserCancelledTyping"))
    ns_buddy_typing_notification(gc, nameid, 0);
  else if (!strcmp(*tab, "chat_start"))
    ns_chat_send_enter(gc, nameid);
  g_strfreev(tab);
  g_free(nameid);
}

void	ns_got_mail(GaimConnection *gc, char *msg)
{
  char	**tab;
  char	*from;
  char	*subject;

  if (!gaim_account_get_check_mail(gaim_connection_get_account(gc)))
    return;
  gaim_debug_info("netsoul", "ns_got_mail msg:%s\n", msg);
  tab = g_strsplit(g_strstrip(msg), " ", 0);
  gaim_debug_info("netsoul", "got_mail 0:%s, 1:%s\n", tab[0], tab[1]);
  from = url_decode(tab[2]);
  if (*tab[3])
    subject = url_decode(tab[3]);
  else
    subject = NULL;
  gaim_notify_email (gc, subject, from, "me", "", NULL, NULL);
  g_strfreev(tab);
}

void	ns_user_cmd(GaimConnection *gc, char **msg)
{
  char	**who;
  char	**tab2;

  tab2 = g_strsplit(*msg, "|", 2);
  who = g_strsplit(*tab2, ":", 0);
  if (!who[1]) {
    g_strfreev(who);
    g_strfreev(tab2);
    return;
  }
  if (!strcmp(who[1], "mail"))
    ns_got_mail(gc, tab2[1]);
  else if (!strcmp(who[1], "user"))
    ns_buddy_user_cmd(gc, who, g_strstrip(tab2[1]));
  g_strfreev(who);
  g_strfreev(tab2);
}


void	ns_listen(gpointer data, gint source, GaimInputCondition cond)
{
  GaimConnection	*gc = data;
  NetsoulData		*ns = gc->proto_data;
  char	buf[NS_BUF_LEN];
  char	**tab;
  int	len;

  for (len = 0; len < NS_BUF_LEN; len++) {
    if ((read(source, buf + len, 1)) <= 0)
    {
      gaim_connection_error(gc, _("Error reading from server"));
      return;
    }
    if (buf[len] == '\n')
      break;
  }
  buf[len] = '\0';
  gaim_debug_info("netsoul", "Netsoul received (%d) : %s\n", len, buf);
  tab = g_strsplit(buf, " ", 2);
  if (!(strncmp(*tab, "rep", 5)))
    ns_use_rep(gc, tab + 1);
  else if (!(strncmp(*tab, "user_cmd", 8)))
    ns_user_cmd(gc, tab + 1);
  else if (!strcmp(*tab, "ping"))
    netsoul_write(ns, "ping\n");
  else if (atoi(*tab) > 0)
    ns_user_update(gc, tab);
  g_strfreev(tab);
}
