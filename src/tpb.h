/* tpb - program to use the IBM ThinkPad(tm) special keys
 * Copyright (C) 2002-2005 Markus Braun <markus.braun@krawel.de>
 *
 * This file is part of tpb.
 *
 * tpb is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * tpb is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tpb; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __TPB_H__
#define __TPB_H__

#if ENABLE_NLS
#include <libintl.h>
#define _(String) gettext (String)
#else /* ENABLE_NLS */
#define _(String) (String)
#endif /* ENABLE_NLS */

#define MAX_VOLUME 100
#define CALLBACK_CMD_LENGTH 256
#define CALLBACK_CMD_ARGS 30

#define KEYCODE_HOME 178
#define KEYCODE_SEARCH 229
#define KEYCODE_MAIL 236
#define KEYCODE_FAVORITES 230
#define KEYCODE_RELOAD 231
#define KEYCODE_ABORT 232
#define KEYCODE_BACKWARD 234
#define KEYCODE_FORWARD 233
#define KEYCODE_FN_KEY 227

/* all nvram toggle values are stored in bit 0 */
/* all xevents toggle values are stored in bit 1 */
typedef struct {
  unsigned int thinkpad_toggle;     /* ThinkPad button */
  unsigned int zoom_toggle;         /* zoom toggle */
  unsigned int display_toggle;      /* display toggle */
  unsigned int home_toggle;         /* Home button */
  unsigned int search_toggle;       /* Search button */
  unsigned int mail_toggle;         /* Mail button */
  unsigned int wireless_toggle;     /* Wireless button */
  unsigned int favorites_toggle;    /* Favorites button */
  unsigned int reload_toggle;       /* Reload button */
  unsigned int abort_toggle;        /* Abort button */
  unsigned int backward_toggle;     /* Backward button */
  unsigned int forward_toggle;      /* Forward button */
  unsigned int fn_toggle;           /* Fn button */
  unsigned int thinklight_toggle;   /* ThinkLight */
  unsigned int hibernate_toggle;    /* hibernation/suspend toggle */
  unsigned int display_state;       /* display state */
  unsigned int expand_toggle;       /* hv expansion state */
  unsigned int brightness_level;    /* brightness level */
  unsigned int brightness_toggle;   /* brightness toggle */
  unsigned int volume_level;        /* volume level */
  unsigned int volume_toggle;       /* volume toggle */
  unsigned int mute_toggle;         /* mute toggle */
  unsigned int ac_state;            /* ac connected */
  unsigned int powermgt_ac;         /* power management mode ac */
  unsigned int powermgt_battery;    /* power management mode battery */
} t_thinkpad_state;

#endif /* __TPB_H__*/

/* vim600:set fen:set fdm=marker:set fdl=0: */
