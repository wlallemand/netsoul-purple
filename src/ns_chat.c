/*
 * @file ns_chat.c
 *
 * gaim-netsoul Protocol Plugin
 *
 * Copyright (C) 2004, Edward Hervey <bilboed@gmail.com>
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

#include"netsoul.h"

void ns_initiate_chat(PurpleConnection *gc, char *who)
{
  NetsoulData *ns;
  PurpleAccount *acc;

  purple_debug_info("netsoul", "ns_initiate_chat\n");
  acc = purple_connection_get_account(gc);
  ns = gc->proto_data;
  ns->conv.conv = serv_got_joined_chat(gc, 1, "Netsoul Chat");

  purple_conv_chat_add_user(&(ns->conv), purple_account_get_username(acc), NULL, PURPLE_CBFLAGS_NONE, TRUE);
}

void ns_chat_send_enter(PurpleConnection *gc, const char *who)
{
  NetsoulData	*ns = gc->proto_data;
  char		*towho;
  char		*resp;

  towho = get_good_msg_user(gc, who);
  purple_debug_info("netsoul", "confirm a chat with %s\n", towho);
  resp = g_strdup_printf("user_cmd msg_user %s chat_enter\n", towho);
  netsoul_write(ns, resp);
  ns_initiate_chat(gc, towho);
  g_free(towho);
  g_free(resp);
}

void ns_chat_send_start(PurpleBlistNode *node, gpointer data)
{
  PurpleConnection *gc;
  NetsoulData	 *ns;
  PurpleBuddy	 *buddy;
  NetsoulBuddy	 *nb;
  char		 *towho;
  char		 *resp;

  buddy = (PurpleBuddy *) data;
  nb = buddy->proto_data;
  gc = purple_account_get_connection(buddy->account);
  ns = gc->proto_data;
  towho = get_good_msg_user(gc, nb->login);
  purple_debug_info("netsoul", "start a chat with %s\n", towho);
  resp = g_strdup_printf("user_cmd msg_user %s chat_start\n", towho);
  netsoul_write(ns, resp);
  g_free(towho);
  g_free(resp);
}
