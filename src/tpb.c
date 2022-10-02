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

/* includes {{{ */
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <locale.h>
#include <signal.h>
#include <sys/soundcard.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "config.h"
#include <math.h>

#if ENABLE_NLS
#include <libintl.h>
#endif /* ENABLE_NLS */

#ifdef HAVE_LIBXOSD
#include <xosd.h>
#endif /* HAVE_LIBXOSD */

#ifdef HAVE_LIBX11
#include <X11/Xlib.h>
#endif /* HAVE_LIBX11 */

#include "cfg.h"
#include "tpb.h"
/* }}} */

/* RCS version string for later identification using rcs ident {{{ */
#ifndef lint
static volatile const char *RCSid = "@(#) $Id: tpb.c,v 1.31 2005/07/18 14:15:59 mbr Exp $";
#endif /* lint */
/* }}} */

/* global variables  {{{ */
extern config cfg;
/* }}} */

/* function prototypes {{{ */
#ifdef ENABLE_NLS
void init_i18n(void);
#endif /* ENABLE_NLS */
void daemonize(void);
#ifdef HAVE_LIBXOSD
xosd *init_xosd(void);
#endif /* HAVE_LIBXOSD */
#ifdef HAVE_LIBX11
Display *init_xgrabkey(void);
#endif /* HAVE_LIBX11 */
int get_nvram_state(t_thinkpad_state *thinkpad_state);
int get_apm_state(t_thinkpad_state *thinkpad_state);
#ifdef HAVE_LIBX11
int xgrabkey(t_thinkpad_state *thinkpad_state, Display *display);
#endif /* HAVE_LIBX11 */
int fork_app(char * cmd);
void set_nvram_volume_level(t_thinkpad_state *thinkpad_state);
unsigned int change_volume(int change);
int apmiser_running(void);
#ifdef HAVE_LIBX11
int *xerrorhandler(Display *display, XErrorEvent *event);
#endif /* HAVE_LIBX11 */
void sig_chld_handler(int signo);
/* }}} */

int main(int argc, char **argv) /* {{{ */
{
  t_thinkpad_state thinkpad_state, last_thinkpad_state;
  unsigned int vol = 0;
  unsigned int display_state = 1;
  char *home;
  char *cfg_file;
  char *msg = "";
  char callback_cmd[CALLBACK_CMD_LENGTH];
  struct sigaction signal_action;
#ifdef HAVE_LIBXOSD
  xosd *osd_ptr = NULL;
#endif /* HAVE_LIBXOSD */
#ifdef HAVE_LIBX11
  Display *display = NULL;
#endif /* HAVE_LIBX11 */

  /* zero thinkpad_state */
  memset(&thinkpad_state, 0, sizeof(thinkpad_state));

  /* register my own handler for SIG_CHLD */
  signal_action.sa_handler = sig_chld_handler;
  signal_action.sa_flags = SA_NOCLDSTOP;
  sigemptyset(&signal_action.sa_mask);
  if (sigaction(SIGCHLD, &signal_action, NULL)  < 0)
  {
    fprintf(stderr, _("Unable to register signal handler:"));
    perror(NULL);
    _exit(-1);
  }

  /* initialize i18n */
#ifdef ENABLE_NLS
  init_i18n();
#endif /* ENABLE_NLS */

  /* initialize config */
  init_cfg(&cfg);

  /* read global config file */
  parse_cfg_file(&cfg, GLOBAL_CONFIG_FILE);

  /* read user config file */
  if((home = getenv("HOME")) != NULL) {
    cfg_file = (char *)malloc(strlen(home) + strlen("/") + strlen(PRIVATE_CONFIG_FILE) + 1);
    strcpy(cfg_file, home);
    strcat(cfg_file, "/");
    strcat(cfg_file, PRIVATE_CONFIG_FILE);
    parse_cfg_file(&cfg, cfg_file);
    free(cfg_file);
  }

  /* parse options */
  parse_option(&cfg, argc, argv);

  /* become a daemon if requested */
  if(cfg.daemon == STATE_ON) {
    daemonize();
  }

  /* initialize osd */
#ifdef HAVE_LIBXOSD
  osd_ptr = init_xosd();
#endif /* HAVE_LIBXOSD */

#ifdef HAVE_LIBX11
  /* initialize key grabbing */
  if(cfg.xevents == STATE_ON) {
    display = init_xgrabkey();
  }
#endif /* HAVE_LIBX11 */

  /* to initialize struct */
  memset(&last_thinkpad_state, 0x00, sizeof(t_thinkpad_state));
  if(get_nvram_state(&thinkpad_state) != 0) {
    _exit(1);
  }
  if(cfg.apm == STATE_ON) {
    get_apm_state(&thinkpad_state);
  }
#ifdef HAVE_LIBX11
  if(cfg.xevents == STATE_ON) {
    xgrabkey(&thinkpad_state, display);
  }
#endif /* HAVE_LIBX11 */


  /* initialize volume */
  if(cfg.mixer == STATE_ON) {
    set_nvram_volume_level(&thinkpad_state);
    vol = change_volume(0);
  }
  else {
    vol = thinkpad_state.volume_level * 100 / 14;
  }

  while(0 == 0) {
    /* sleep for polltime */
    usleep(cfg.polltime);

    /* save last state and get new one */

    memcpy(&last_thinkpad_state, &thinkpad_state, sizeof(t_thinkpad_state));
    // printf("old brightness %i\n", last_thinkpad_state.brightness_level);
    // printf("old brightness toggle %i\n", last_thinkpad_state.brightness_toggle); 

    if(get_nvram_state(&thinkpad_state) != 0) {
      _exit(1);
    }
    if(cfg.apm == STATE_ON) {
      get_apm_state(&thinkpad_state);
    }
#ifdef HAVE_LIBX11
    if(cfg.xevents == STATE_ON) {
      xgrabkey(&thinkpad_state, display);
    }
#endif /* HAVE_LIBX11 */
    
    // printf("new brightness %i\n", thinkpad_state.brightness_level);
    // printf("new brightness toggle %i\n", thinkpad_state.brightness_toggle); 

    /* check if anything changed */
    if(memcmp(&last_thinkpad_state, &thinkpad_state, sizeof(t_thinkpad_state)) == 0) {
      continue;
    }

    /* determine the state of the Thinkpad button  {{{ */
    if(thinkpad_state.thinkpad_toggle != last_thinkpad_state.thinkpad_toggle &&
       thinkpad_state.hibernate_toggle == last_thinkpad_state.hibernate_toggle) {
      if(cfg.verbose == STATE_ON) {
	puts(_("ThinkPad button pressed"));
      }
      if(cfg.tpb_cmd != NULL) {
	if(fork_app(cfg.tpb_cmd) != 0) {
	  _exit(0);
	}
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s thinkpad pressed", cfg.callback);
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
    } /* }}} */

    /* determine the state of zoom  {{{ */
    if(thinkpad_state.zoom_toggle != last_thinkpad_state.zoom_toggle) {
      if(cfg.verbose == STATE_ON) {
	printf(_("Zoom is %s\n"), thinkpad_state.zoom_toggle == 1 ? _("on") : _("off"));
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s zoom %s", cfg.callback, thinkpad_state.zoom_toggle == 1 ? "on" : "off");
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
#ifdef HAVE_LIBXOSD
      if(osd_ptr != NULL &&
	 ((cfg.osd == STATE_OFF && cfg.osdzoom == STATE_ON) || (cfg.osd == STATE_ON && cfg.osdzoom != STATE_OFF))) {
	xosd_display(osd_ptr, 0, XOSD_string, thinkpad_state.zoom_toggle == 1 ? _("Zoom on") : _("Zoom off"));
	xosd_display(osd_ptr, 1, XOSD_string, "");
      }
#endif /* HAVE_LIBXOSD */
    } /* }}} */

    /* determine the state of the home button  {{{ */
    if(thinkpad_state.home_toggle != last_thinkpad_state.home_toggle &&
       thinkpad_state.hibernate_toggle == last_thinkpad_state.hibernate_toggle) {
      if(cfg.verbose == STATE_ON) {
	puts(_("Home button pressed"));
      }
      if(cfg.home_cmd != NULL) {
	if(fork_app(cfg.home_cmd) != 0) {
	  _exit(0);
	}
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s home pressed", cfg.callback);
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
    } /* }}} */

    /* determine the state of the search button  {{{ */
    if(thinkpad_state.search_toggle != last_thinkpad_state.search_toggle &&
       thinkpad_state.hibernate_toggle == last_thinkpad_state.hibernate_toggle) {
      if(cfg.verbose == STATE_ON) {
	puts(_("Search button pressed"));
      }
      if(cfg.search_cmd != NULL) {
	if(fork_app(cfg.search_cmd) != 0) {
	  _exit(0);
	}
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s search pressed", cfg.callback);
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
    } /* }}} */

    /* determine the state of the mail button  {{{ */
    if(thinkpad_state.mail_toggle != last_thinkpad_state.mail_toggle &&
       thinkpad_state.hibernate_toggle == last_thinkpad_state.hibernate_toggle) {
      if(cfg.verbose == STATE_ON) {
	puts(_("Mail button pressed"));
      }
      if(cfg.mail_cmd != NULL) {
	if(fork_app(cfg.mail_cmd) != 0) {
	  _exit(0);
	}
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s mail pressed", cfg.callback);
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
    } /* }}} */

    /* determine the state of the wireless button  {{{ */
    if(thinkpad_state.wireless_toggle != last_thinkpad_state.wireless_toggle &&
       thinkpad_state.hibernate_toggle == last_thinkpad_state.hibernate_toggle) {
      if(cfg.verbose == STATE_ON) {
	puts(_("Wireless button pressed"));
      }
      if(cfg.wireless_cmd != NULL) {
	if(fork_app(cfg.wireless_cmd) != 0) {
	  _exit(0);
	}
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s wireless pressed", cfg.callback);
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
    } /* }}} */

    /* determine the state of the favorites button  {{{ */
    if(thinkpad_state.favorites_toggle != last_thinkpad_state.favorites_toggle) {
      if(cfg.verbose == STATE_ON) {
	puts(_("Favorites button pressed"));
      }
      if(cfg.favorites_cmd != NULL) {
	if(fork_app(cfg.favorites_cmd) != 0) {
	  _exit(0);
	}
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s favorites pressed", cfg.callback);
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
    } /* }}} */

    /* determine the state of the reload button  {{{ */
    if(thinkpad_state.reload_toggle != last_thinkpad_state.reload_toggle) {
      if(cfg.verbose == STATE_ON) {
	puts(_("Reload button pressed"));
      }
      if(cfg.reload_cmd != NULL) {
	if(fork_app(cfg.reload_cmd) != 0) {
	  _exit(0);
	}
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s reload pressed", cfg.callback);
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
    } /* }}} */

    /* determine the state of the abort button  {{{ */
    if(thinkpad_state.abort_toggle != last_thinkpad_state.abort_toggle) {
      if(cfg.verbose == STATE_ON) {
	puts(_("Abort button pressed"));
      }
      if(cfg.abort_cmd != NULL) {
	if(fork_app(cfg.abort_cmd) != 0) {
	  _exit(0);
	}
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s abort pressed", cfg.callback);
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
    } /* }}} */

    /* determine the state of the backward button  {{{ */
    if(thinkpad_state.backward_toggle != last_thinkpad_state.backward_toggle) {
      if(cfg.verbose == STATE_ON) {
	puts(_("Backward button pressed"));
      }
      if(cfg.backward_cmd != NULL) {
	if(fork_app(cfg.backward_cmd) != 0) {
	  _exit(0);
	}
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s backward pressed", cfg.callback);
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
    } /* }}} */

    /* determine the state of the forward button  {{{ */
    if(thinkpad_state.forward_toggle != last_thinkpad_state.forward_toggle) {
      if(cfg.verbose == STATE_ON) {
	puts(_("Forward button pressed"));
      }
      if(cfg.forward_cmd != NULL) {
	if(fork_app(cfg.forward_cmd) != 0) {
	  _exit(0);
	}
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s forward pressed", cfg.callback);
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
    } /* }}} */

    /* determine the state of the fn button  {{{ */
    if(thinkpad_state.fn_toggle != last_thinkpad_state.fn_toggle) {
      if(cfg.verbose == STATE_ON) {
	puts(_("Fn button pressed"));
      }
      if(cfg.fn_cmd != NULL) {
	if(fork_app(cfg.fn_cmd) != 0) {
	  _exit(0);
	}
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s fn pressed", cfg.callback);
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
    } /* }}} */

    /* determine the state of ThinkLight {{{ */
    if(thinkpad_state.thinklight_toggle != last_thinkpad_state.thinklight_toggle) {
      if(cfg.verbose == STATE_ON) {
	printf(_("ThinkLight is %s\n"), thinkpad_state.thinklight_toggle == 1 ? _("on") : _("off"));
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s thinklight %s", cfg.callback, thinkpad_state.thinklight_toggle == 1 ? "on" : "off");
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
#ifdef HAVE_LIBXOSD
      if(osd_ptr != NULL &&
	 ((cfg.osd == STATE_OFF && cfg.osdthinklight == STATE_ON) || (cfg.osd == STATE_ON && cfg.osdthinklight != STATE_OFF))) {
	xosd_display(osd_ptr, 0, XOSD_string, thinkpad_state.thinklight_toggle == 1 ? _("ThinkLight on") : _("ThinkLight off"));
	xosd_display(osd_ptr, 1, XOSD_string, "");
      }
#endif /* HAVE_LIBXOSD */
    } /* }}} */

    /* determine the state of display  {{{ */
    if((thinkpad_state.display_toggle != last_thinkpad_state.display_toggle ||
	thinkpad_state.display_state != last_thinkpad_state.display_state) &&
       thinkpad_state.hibernate_toggle == last_thinkpad_state.hibernate_toggle) {

      /* Some thinkpads have no hardware support to switch lcd/crt. They also
       * don't reflect the current state in thinkpad_state.display_state. So, if
       * thinkpad_state.display_toggle changes, but thinkpad_state.display_state does
       * not change, simulate the display state.
       */
      if(thinkpad_state.display_toggle != last_thinkpad_state.display_toggle &&
	 thinkpad_state.display_state == last_thinkpad_state.display_state) {
	display_state = display_state % 3 + 1;
      }
      else {
	display_state = thinkpad_state.display_state;
      }

      switch (display_state & 0x03) {
	case 0x1:
	  msg = _("LCD on, CRT off");
	  snprintf(callback_cmd, sizeof(callback_cmd), "%s display lcd", cfg.callback);
	  break;

	case 0x2:
	  msg = _("LCD off, CRT on");
	  snprintf(callback_cmd, sizeof(callback_cmd), "%s display crt", cfg.callback);
	  break;

	case 0x3:
	  msg = _("LCD on, CRT on");
	  snprintf(callback_cmd, sizeof(callback_cmd), "%s display both", cfg.callback);
	  break;
      }
      if(cfg.verbose == STATE_ON) {
	printf(_("Display changed: %s\n"), msg);
      }
      if(cfg.callback != NULL) {
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
#ifdef HAVE_LIBXOSD
      if(osd_ptr != NULL &&
	 ((cfg.osd == STATE_OFF && cfg.osddisplay == STATE_ON) || (cfg.osd == STATE_ON && cfg.osddisplay != STATE_OFF))) {
	xosd_display(osd_ptr, 0, XOSD_string, _("Display"));
	xosd_display(osd_ptr, 1, XOSD_string, msg);
      }
#endif /* HAVE_LIBXOSD */
    } /* }}} */

    /* determine the state of hv expansion  {{{ */
    if(thinkpad_state.expand_toggle != last_thinkpad_state.expand_toggle) {
      if(cfg.verbose == STATE_ON) {
	printf(_("HV Expansion is %s\n"), (thinkpad_state.expand_toggle & 0x01) == 1 ? _("on") : _("off"));
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s expand %s", cfg.callback, thinkpad_state.expand_toggle == 1 ? "on" : "off");
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
#ifdef HAVE_LIBXOSD
      if(osd_ptr != NULL &&
	 ((cfg.osd == STATE_OFF && cfg.osdhvexpansion == STATE_ON) || (cfg.osd == STATE_ON && cfg.osdhvexpansion != STATE_OFF))) {
	xosd_display(osd_ptr, 0, XOSD_string, thinkpad_state.expand_toggle == 1 ? _("HV Expansion on") : _("HV Expansion off"));
	xosd_display(osd_ptr, 1, XOSD_string, "");
      }
#endif /* HAVE_LIBXOSD */
    } /* }}} */

    /* determine the state of the brightness buttons {{{ */
    if(thinkpad_state.brightness_level != last_thinkpad_state.brightness_level) {
      if(cfg.verbose == STATE_ON) {
	printf(_("Brightness changed: Level %d\n"), thinkpad_state.brightness_level * 100 / 7);
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s brightness %d", cfg.callback, thinkpad_state.brightness_level * 100 / 7);
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
    }
#ifdef HAVE_LIBXOSD
    // printf("brightness toggle %i\n",thinkpad_state.brightness_toggle);
    // printf("last brightness toggle %i\n",last_thinkpad_state.brightness_toggle);
    if((thinkpad_state.brightness_toggle != last_thinkpad_state.brightness_toggle) || (thinkpad_state.brightness_level != last_thinkpad_state.brightness_level)) { // RC: level check added
      if(osd_ptr != NULL &&
	 ((cfg.osd == STATE_OFF && cfg.osdbrightness == STATE_ON) || (cfg.osd == STATE_ON && cfg.osdbrightness != STATE_OFF))) {
  int norm_brightness_level = ceil(thinkpad_state.brightness_level/2.0);
  // printf("> %i\n", norm_brightness_level * 100 / 4);
  xosd_display(osd_ptr, 0, XOSD_string, _("Brightness"));
	// xosd_display(osd_ptr, 1, XOSD_percentage, thinkpad_state.brightness_level * 100 / 7);
  xosd_display(osd_ptr, 1, XOSD_percentage, norm_brightness_level * 100 / 4);
      }
    }
#endif /* HAVE_LIBXOSD */ /* }}} */

    /* determine the state of the volume buttons {{{ */
    if(thinkpad_state.volume_level != last_thinkpad_state.volume_level) {
      if(cfg.mixer == STATE_ON) {
	/* if we use the DEFAULT_MIXERSTEPS, we don't need to modify the nvram */
	if(cfg.mixersteps == DEFAULT_MIXERSTEPS) {
	  vol = change_volume((1. * MAX_VOLUME / cfg.mixersteps * thinkpad_state.volume_level) - vol);
	}
	else
	{
	  vol = change_volume(1. * MAX_VOLUME / cfg.mixersteps * ((int)thinkpad_state.volume_level - (int)last_thinkpad_state.volume_level)); /* thinkpad_state.volume_level-last_thinkpad_state.volume_level gives the direction */
	  set_nvram_volume_level(&thinkpad_state);
	}
      }
      else {
	vol = thinkpad_state.volume_level * 100 / 14;
      }
      if(cfg.verbose == STATE_ON) {
	printf(_("Volume changed: Level %d\n"), vol);
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s volume %d", cfg.callback, vol);
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
    }
#ifdef HAVE_LIBXOSD
    /* show volume every time a volume button is pressed and not mute */
    if(thinkpad_state.volume_toggle != last_thinkpad_state.volume_toggle &&
       thinkpad_state.mute_toggle == 0) {
      if(osd_ptr != NULL &&
	 ((cfg.osd == STATE_OFF && cfg.osdvolume == STATE_ON) || (cfg.osd == STATE_ON && cfg.osdvolume != STATE_OFF))) {
	xosd_display(osd_ptr, 0, XOSD_string, _("Volume"));
	xosd_display(osd_ptr, 1, XOSD_percentage, vol);
      }
    }
#endif /* HAVE_LIBXOSD */ /* }}} */

    /* determine the state of the mute button {{{ */
    if(thinkpad_state.mute_toggle != last_thinkpad_state.mute_toggle) {
      if(cfg.verbose == STATE_ON) {
	printf("%s\n", thinkpad_state.mute_toggle == 1 ? _("Mute on") : _("Mute off"));
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s mute %s", cfg.callback, thinkpad_state.mute_toggle == 1 ? "on" : "off");
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
    }
#ifdef HAVE_LIBXOSD
    if(thinkpad_state.mute_toggle != last_thinkpad_state.mute_toggle ||
       (thinkpad_state.volume_toggle != last_thinkpad_state.volume_toggle && last_thinkpad_state.mute_toggle == 1)) {
      if(cfg.mixer == STATE_ON) {
	if(thinkpad_state.mute_toggle == 1) {
	  change_volume(-vol); /* mute */
	}
	else {
	  change_volume(vol); /* unmute */
	}
      }

      if(osd_ptr != NULL &&
	 ((cfg.osd == STATE_OFF && cfg.osdvolume == STATE_ON) || (cfg.osd == STATE_ON && cfg.osdvolume != STATE_OFF))) {
	if(thinkpad_state.mute_toggle == 1) {
	  xosd_display(osd_ptr, 0, XOSD_string, _("Mute on"));
	  xosd_display(osd_ptr, 1, XOSD_percentage, 0);
	}
	else {
	  xosd_display(osd_ptr, 0, XOSD_string, _("Mute off"));
	  xosd_display(osd_ptr, 1, XOSD_percentage, vol);
	}
      }
    }
#endif /* HAVE_LIBXOSD */ /* }}} */

    /* determine the state of power {{{ */
    if(thinkpad_state.ac_state != last_thinkpad_state.ac_state) {
      if(cfg.verbose == STATE_ON) {
	printf(_("Power line changed: %s\n"), thinkpad_state.ac_state == 1 ? _("AC connected") : _("AC disconnected"));
      }
      if(cfg.callback != NULL) {
	snprintf(callback_cmd, sizeof(callback_cmd), "%s ac_power %s", cfg.callback, thinkpad_state.ac_state == 1 ? "connected" : "disconnected");
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
#ifdef HAVE_LIBXOSD
      if(osd_ptr != NULL &&
	 ((cfg.osd == STATE_OFF && cfg.osdpowermgt == STATE_ON) || (cfg.osd == STATE_ON && cfg.osdpowermgt != STATE_OFF))) {
	xosd_display(osd_ptr, 0, XOSD_string, thinkpad_state.ac_state == 1 ? _("AC connected") : _("AC disconnected"));
	xosd_display(osd_ptr, 1, XOSD_string, "");
      }
#endif /* HAVE_LIBXOSD */
    } /* }}} */

    /* determine power management mode AC {{{ */
    if(thinkpad_state.powermgt_ac != last_thinkpad_state.powermgt_ac) {
      switch(thinkpad_state.powermgt_ac) {
	case 0x4:
	  msg = _("PM AC high");
	  snprintf(callback_cmd, sizeof(callback_cmd), "%s powermgt_ac high", cfg.callback);
	  break;

	case 0x2:
	  msg = _("PM AC auto");
	  snprintf(callback_cmd, sizeof(callback_cmd), "%s powermgt_ac auto", cfg.callback);
	  break;

	case 0x1:
	  msg = _("PM AC manual");
	  snprintf(callback_cmd, sizeof(callback_cmd), "%s powermgt_ac manual", cfg.callback);
	  break;

	default:
	  msg = _("PM AC unknown");
	  break;
      }
      if(cfg.verbose == STATE_ON) {
	printf(_("Power management mode AC changed: %s\n"), msg);
      }
      if(cfg.callback != NULL) {
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
#ifdef HAVE_LIBXOSD
      if(osd_ptr != NULL &&
	 ((cfg.osd == STATE_OFF && cfg.osdpowermgt == STATE_ON) || (cfg.osd == STATE_ON && cfg.osdpowermgt != STATE_OFF))) {
	if (cfg.powermgt == STATE_ON ||  (cfg.powermgt == STATE_AUTO && apmiser_running() == 0)) {
	  xosd_display(osd_ptr, 0, XOSD_string, msg);
	  xosd_display(osd_ptr, 1, XOSD_string, "");
	}
      }
#endif /* HAVE_LIBXOSD */
    } /* }}} */

    /* determine power management mode battery {{{ */
    if(thinkpad_state.powermgt_battery != last_thinkpad_state.powermgt_battery) {
      switch(thinkpad_state.powermgt_battery) {
	case 0x4:
	  msg = _("PM battery high");
	  snprintf(callback_cmd, sizeof(callback_cmd), "%s powermgt_battery high", cfg.callback);
	  break;

	case 0x2:
	  msg = _("PM battery auto");
	  snprintf(callback_cmd, sizeof(callback_cmd), "%s powermgt_battery auto", cfg.callback);
	  break;

	case 0x1:
	  msg = _("PM battery manual");
	  snprintf(callback_cmd, sizeof(callback_cmd), "%s powermgt_battery manual", cfg.callback);
	  break;

	default:
	  msg = _("PM battery unknown");
	  break;
      }
      if(cfg.verbose == STATE_ON) {
	printf(_("Power management mode battery changed: %s\n"), msg);
      }
      if(cfg.callback != NULL) {
	if(fork_app(callback_cmd) != 0) {
	  _exit(0);
	}
      }
#ifdef HAVE_LIBXOSD
      if(osd_ptr != NULL &&
	 ((cfg.osd == STATE_OFF && cfg.osdpowermgt == STATE_ON) || (cfg.osd == STATE_ON && cfg.osdpowermgt != STATE_OFF))) {
	if (cfg.powermgt == STATE_ON ||  (cfg.powermgt == STATE_AUTO && apmiser_running() == 0)) {
	  xosd_display(osd_ptr, 0, XOSD_string, msg);
	  xosd_display(osd_ptr, 1, XOSD_string, "");
	}
      }
#endif /* HAVE_LIBXOSD */
    } /* }}} */
  }

  _exit(0); /* never reached */
} /* }}} */

#ifdef ENABLE_NLS
/* initialize the i18n system */
void init_i18n(void) /* {{{ */
{
  setlocale (LC_ALL, "");              
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  return;
} /* }}} */
#endif /* ENABLE_NLS */

/* when called, the application becomes a daemon */
void daemonize(void) /* {{{ */
{
  int fdsc, fdsc_max;

  /* code inspired by comp.unix.programmer FAQ */
  switch (fork())
  {
    case 0:
      break;
    case -1:
      fprintf(stderr, _("Unable to fork daemon:"));
      perror(NULL);
      _exit(-1);
    default:
      _exit(0);          /* exit the original process */
  }

  /* create a new session */
  if (setsid() < 0) {
    _exit(-1);
  }

  switch (fork())
  {
    case 0:
      break;
    case -1:
      fprintf(stderr, _("Unable to fork daemon:"));
      perror(NULL);
      _exit(-1);
    default:
      _exit(0);
  }

  if(cfg.verbose == STATE_ON) {
    puts(_("Daemon started."));
  }

  /* change to root directory */
  chdir("/");

  /* close all file descriptors */
  for (fdsc = 0, fdsc_max = sysconf(_SC_OPEN_MAX); fdsc < fdsc_max; fdsc++) {
    close(fdsc);
  }

  /* open /dev/null as file descriptor 0 */
  open("/dev/null",O_RDWR);

  /* duplicate file descriptor 0 to file descriptor 1 (next free) */
  dup(0);

  /* duplicate file descriptor 0 to file descriptor 2 (next free) */
  dup(0);

  return;
} /* }}} */

#ifdef HAVE_LIBXOSD
/* initialize the on-screen-display */
xosd *init_xosd(void) /* {{{ */
{
  xosd *osd_ptr = NULL;

#if HAVE_LIBXOSD_VERSION >= 20000
  osd_ptr = xosd_create(DEFAULT_OSDLINES);
#elif HAVE_LIBXOSD_VERSION >= 10000
  osd_ptr = xosd_init(DEFAULT_OSDFONT, DEFAULT_OSDCOLOR, DEFAULT_OSDTIMEOUT,
		      DEFAULT_OSDPOS, DEFAULT_OSDVERTICAL, DEFAULT_OSDSHADOW, DEFAULT_OSDLINES);
#elif HAVE_LIBXOSD_VERSION >= 700
  osd_ptr = xosd_init(DEFAULT_OSDFONT, DEFAULT_OSDCOLOR, DEFAULT_OSDTIMEOUT,
		      DEFAULT_OSDPOS, DEFAULT_OSDVERTICAL, DEFAULT_OSDSHADOW);
#endif /* HAVE_LIBXOSD_VERSION */

  if(osd_ptr == NULL) {
    fprintf(stderr, _("Unable to initialize xosd. Running without onsceen display.\n"));
  }
  else {
    /* initialize font */
    if(xosd_set_font(osd_ptr, cfg.osdfont) != 0) {
      if(xosd_set_font(osd_ptr, DEFAULT_OSDFONT) != 0) {
	/* even the default font is not available */
	fprintf(stderr, _("Invalid xosd font \"%s\". Even the default font \"%s\" "),
		cfg.osdfont, DEFAULT_OSDFONT);
	fprintf(stderr, _("is not available. Please add an existing font to your %s file.\n"),
		GLOBAL_CONFIG_FILE);
	fprintf(stderr, _("Running without onsceen display.\n"));

#if HAVE_LIBXOSD_VERSION >= 20000
	/* Disabled because of a bug in xosd lib
	 * xosd_destroy(osd_ptr);
	 */
#else /* HAVE_LIBXOSD_VERSION */
	/* Disabled because of a bug in xosd lib
	 * xosd_uninit(osd_ptr);
	 */
#endif /* HAVE_LIBXOSD_VERSION */

	return NULL;
      }
      else {
	/* we run with the default font */
	fprintf(stderr, _("Invalid xosd font \"%s\". Running with the default \"%s\".\n"),
		cfg.osdfont, DEFAULT_OSDFONT);
      }
    }

    /* initialize color */
    if(xosd_set_colour(osd_ptr, cfg.osdcolor) != 0) {
      fprintf(stderr, _("Invalid xosd color \"%s\". Running with the default \"%s\".\n"),
	      cfg.osdcolor, DEFAULT_OSDCOLOR);
      xosd_set_colour(osd_ptr, DEFAULT_OSDCOLOR);
    }

    /* initialize timeout */
    if(xosd_set_timeout(osd_ptr, cfg.osdtimeout) != 0) {
      fprintf(stderr, _("Invalid xosd timeout \"%d\". Running with the default \"%d\".\n"),
	      cfg.osdtimeout, DEFAULT_OSDTIMEOUT);
      xosd_set_timeout(osd_ptr, DEFAULT_OSDTIMEOUT);
    }

    /* initialize position */
    if(xosd_set_pos(osd_ptr, cfg.osdpos) != 0) {
      fprintf(stderr, _("Invalid xosd position \"%d\". Running with the default \"%d\".\n"),
	      cfg.osdpos, DEFAULT_OSDPOS);
      xosd_set_pos(osd_ptr, DEFAULT_OSDPOS);
    }

#if HAVE_LIBXOSD_VERSION >= 20000
    /* initialize horizontal offset */
    if(xosd_set_horizontal_offset(osd_ptr, cfg.osdhorizontal) != 0) {
      fprintf(stderr, _("Invalid horizontal xosd offset \"%d\". Running with the default \"%d\".\n"),
	      cfg.osdhorizontal, DEFAULT_OSDHORIZONTAL);
      xosd_set_horizontal_offset(osd_ptr, DEFAULT_OSDHORIZONTAL);
    }

    /* initialize vertical offset */
    if(xosd_set_vertical_offset(osd_ptr, cfg.osdvertical) != 0) {
      fprintf(stderr, _("Invalid vertical xosd offset \"%d\". Running with the default \"%d\".\n"),
	      cfg.osdvertical, DEFAULT_OSDVERTICAL);
      xosd_set_vertical_offset(osd_ptr, DEFAULT_OSDVERTICAL);
    }
#else /* HAVE_LIBXOSD_VERSION */
    /* initialize vertical offset */
    if(xosd_set_offset(osd_ptr, cfg.osdvertical) != 0) {
      fprintf(stderr, _("Invalid vertical xosd offset \"%d\". Running with the default \"%d\".\n"),
	      cfg.osdvertical, DEFAULT_OSDVERTICAL);
      xosd_set_offset(osd_ptr, DEFAULT_OSDVERTICAL);
    }
#endif /* HAVE_LIBXOSD_VERSION */

    /* initialize shadow offset */
    if(xosd_set_shadow_offset(osd_ptr, cfg.osdshadow) != 0) {
      fprintf(stderr, _("Invalid shadow xosd offset \"%d\". Running with the default \"%d\".\n"),
	      cfg.osdshadow, DEFAULT_OSDSHADOW);
      xosd_set_shadow_offset(osd_ptr, DEFAULT_OSDSHADOW);
    }

#if HAVE_LIBXOSD_VERSION >= 20200
    /* initialize shadow color */
    if(xosd_set_shadow_colour(osd_ptr, cfg.osdshadowcolor) != 0) {
      fprintf(stderr, _("Invalid xosd shadow color \"%s\". Running with the default \"%s\".\n"),
	      cfg.osdshadowcolor, DEFAULT_OSDSHADOWCOLOR);
      xosd_set_shadow_colour(osd_ptr, DEFAULT_OSDSHADOWCOLOR);
    }

    /* initialize outline offset */
    if(xosd_set_outline_offset(osd_ptr, cfg.osdoutline) != 0) {
      fprintf(stderr, _("Invalid outline xosd width \"%d\". Running with the default \"%d\".\n"),
	      cfg.osdoutline, DEFAULT_OSDOUTLINE);
      xosd_set_outline_offset(osd_ptr, DEFAULT_OSDOUTLINE);
    }

    /* initialize outline color */
    if(xosd_set_outline_colour(osd_ptr, cfg.osdoutlinecolor) != 0) {
      fprintf(stderr, _("Invalid xosd outline color \"%s\". Running with the default \"%s\".\n"),
	      cfg.osdoutlinecolor, DEFAULT_OSDOUTLINECOLOR);
      xosd_set_outline_colour(osd_ptr, DEFAULT_OSDOUTLINECOLOR);
    }
#endif /* HAVE_LIBXOSD_VERSION */

#if HAVE_LIBXOSD_VERSION >= 10000
    /* initialize alignment */
    if(xosd_set_align(osd_ptr, cfg.osdalign) != 0) {
      fprintf(stderr, _("Invalid xosd alignment \"%d\". Running with default \"%d\".\n"),
	      cfg.osdalign, DEFAULT_OSDALIGN);
      xosd_set_align(osd_ptr, DEFAULT_OSDALIGN);
    }
#endif /* HAVE_LIBXOSD_VERSION */
  }

  return osd_ptr;
} /* }}} */
#endif /* HAVE_LIBXOSD */

#ifdef HAVE_LIBX11
/* initialize X for grabbing key events */
Display *init_xgrabkey(void) /* {{{ */
{
  char *display_name = NULL;
  Display *display;

  /* set up our own error handler */
  XSetErrorHandler ((XErrorHandler) xerrorhandler);

  /* get the name of the current X display */
  display_name = XDisplayName(NULL);
  if(display_name == NULL) {
    fprintf(stderr, _("Can't get display name, X may not be running!\n"));
    _exit(1);
  }

  /* open the X display */
  display = XOpenDisplay(display_name);
  if(!display) {
    fprintf(stderr, _("Can't open display, X may not be running!\n"));
    _exit(1);
  }

  /* allow X events */
  XAllowEvents(display, AsyncKeyboard, CurrentTime);

  /* select the input */
  XSelectInput (display, DefaultRootWindow(display), KeyPressMask);

  /* set up the keys to grab */
  if ((cfg.home_cmd      != NULL) || (cfg.callback != NULL)) XGrabKey(display, KEYCODE_HOME        , AnyModifier, DefaultRootWindow(display), False, GrabModeAsync, GrabModeAsync); 
  if ((cfg.search_cmd    != NULL) || (cfg.callback != NULL)) XGrabKey(display, KEYCODE_SEARCH      , AnyModifier, DefaultRootWindow(display), False, GrabModeAsync, GrabModeAsync); 
  if ((cfg.mail_cmd      != NULL) || (cfg.callback != NULL)) XGrabKey(display, KEYCODE_MAIL        , AnyModifier, DefaultRootWindow(display), False, GrabModeAsync, GrabModeAsync); 
  if ((cfg.favorites_cmd != NULL) || (cfg.callback != NULL)) XGrabKey(display, KEYCODE_FAVORITES   , AnyModifier, DefaultRootWindow(display), False, GrabModeAsync, GrabModeAsync); 
  if ((cfg.reload_cmd    != NULL) || (cfg.callback != NULL)) XGrabKey(display, KEYCODE_RELOAD      , AnyModifier, DefaultRootWindow(display), False, GrabModeAsync, GrabModeAsync); 
  if ((cfg.abort_cmd     != NULL) || (cfg.callback != NULL)) XGrabKey(display, KEYCODE_ABORT       , AnyModifier, DefaultRootWindow(display), False, GrabModeAsync, GrabModeAsync); 
  if ((cfg.backward_cmd  != NULL) || (cfg.callback != NULL)) XGrabKey(display, KEYCODE_BACKWARD    , AnyModifier, DefaultRootWindow(display), False, GrabModeAsync, GrabModeAsync); 
  if ((cfg.forward_cmd   != NULL) || (cfg.callback != NULL)) XGrabKey(display, KEYCODE_FORWARD     , AnyModifier, DefaultRootWindow(display), False, GrabModeAsync, GrabModeAsync); 
  if ((cfg.fn_cmd        != NULL) || (cfg.callback != NULL)) XGrabKey(display, KEYCODE_FN_KEY      , AnyModifier, DefaultRootWindow(display), False, GrabModeAsync, GrabModeAsync); 

  return(display);
} /* }}} */
#endif /* HAVE_LIBX11 */

/* get the current state from the nvram */
int get_nvram_state(t_thinkpad_state *thinkpad_state) /* {{{ */
{
  static int fdsc = -1; /* -1 -> file not opened */
  unsigned char buffer[114];
  struct {
    int pos;
    int len;
  } pos_len[] = {
    { 0x39, 0x01 },
    { 0x56, 0x04 },
    { 0x5e, 0x01 },
    { 0x60, 0x01 },
    { 0x00, 0x00 } /* end */
  };
  int pos_len_idx = 0;

  /* open nvram for reading */
  if(fdsc == -1) { /* if not already opened, open nvram */
    if((fdsc=open(cfg.nvram, O_RDONLY|O_NONBLOCK)) == -1) {
      fprintf(stderr, _("Unable to open device %s: "), cfg.nvram);
      perror(NULL);
      return -1;
    }
  }

  /* Read only the interesting bytes from nvram to reduce the CPU consupmtion of tpb */
  /* The kernel nvram driver reads byte-by-byte from nvram, so just reading interesting bytes reduces the amount of inb() calls */
  while (pos_len[pos_len_idx].pos != 0x0) {
    if(lseek(fdsc, pos_len[pos_len_idx].pos, SEEK_SET) != pos_len[pos_len_idx].pos ) {
      fprintf(stderr, _("Unable to seek in device %s: "), cfg.nvram);
      perror(NULL);
      return -1;
    }
    if(read(fdsc, buffer+pos_len[pos_len_idx].pos, pos_len[pos_len_idx].len) != pos_len[pos_len_idx].len) {

      fprintf(stderr, _("Unable to read from device %s: "), cfg.nvram);
      perror(NULL);
      return -1;
    }
    pos_len_idx++;
  }

  thinkpad_state->thinkpad_toggle   = (thinkpad_state->thinkpad_toggle   & ~0x01) | (( buffer[0x57] & 0x08) >> 3);
  thinkpad_state->zoom_toggle       = (thinkpad_state->zoom_toggle       & ~0x01) | ((~buffer[0x57] & 0x20) >> 5);
  thinkpad_state->display_toggle    = (thinkpad_state->display_toggle    & ~0x01) | (( buffer[0x57] & 0x40) >> 6);
  thinkpad_state->home_toggle       = (thinkpad_state->home_toggle       & ~0x01) | (( buffer[0x56] & 0x01)     );
  thinkpad_state->search_toggle     = (thinkpad_state->search_toggle     & ~0x01) | (( buffer[0x56] & 0x02) >> 1);
  thinkpad_state->mail_toggle       = (thinkpad_state->mail_toggle       & ~0x01) | (( buffer[0x56] & 0x04) >> 2);
  thinkpad_state->wireless_toggle   = (thinkpad_state->wireless_toggle   & ~0x01) | (( buffer[0x56] & 0x20) >> 5);
  thinkpad_state->thinklight_toggle = (thinkpad_state->thinklight_toggle & ~0x01) | (( buffer[0x58] & 0x10) >> 4);
  thinkpad_state->hibernate_toggle  = (thinkpad_state->hibernate_toggle  & ~0x01) | (( buffer[0x58] & 0x01)     );
  thinkpad_state->display_state     =                                               (( buffer[0x59] & 0x03)     );
  thinkpad_state->expand_toggle     = (thinkpad_state->expand_toggle     & ~0x01) | (( buffer[0x59] & 0x10) >> 4);
  thinkpad_state->brightness_level  =                                               (( buffer[0x5E] & 0x07)     );
  thinkpad_state->brightness_toggle = (thinkpad_state->brightness_toggle & ~0x01) | (( buffer[0x5E] & 0x20) >> 5);
  thinkpad_state->volume_level      =                                               (( buffer[0x60] & 0x0f)     );
  thinkpad_state->volume_toggle     = (thinkpad_state->volume_toggle     & ~0x01) | (( buffer[0x60] & 0x80) >> 7);
  thinkpad_state->mute_toggle       = (thinkpad_state->mute_toggle       & ~0x01) | (( buffer[0x60] & 0x40) >> 6);
  thinkpad_state->powermgt_ac       =                                               (( buffer[0x39] & 0x07)     );
  thinkpad_state->powermgt_battery  =                                               (( buffer[0x39] & 0x38) >> 3);

  return 0;
} /* }}} */

/* get the current state from the apm subsystem */
int get_apm_state(t_thinkpad_state *thinkpad_state) /* {{{ */
{
  unsigned int i;
  static int fdsc = -1; /* -1 -> file not opened */
  char buffer[38];
  char *tokens[9];

  /* Read the state of the ac line from proc filesystem.
   * Documentation of /proc/apm from linux kernel (/usr/src/linux/arch/i386/kernel/apm.c)
   * 
   * 0) Linux driver version (this will change if format changes)
   * 1) APM BIOS Version.  Usually 1.0, 1.1 or 1.2.              
   * 2) APM flags from APM Installation Check (0x00):            
   *    bit 0: APM_16_BIT_SUPPORT                                
   *    bit 1: APM_32_BIT_SUPPORT                                
   *    bit 2: APM_IDLE_SLOWS_CLOCK                              
   *    bit 3: APM_BIOS_DISABLED                                 
   *    bit 4: APM_BIOS_DISENGAGED                               
   * 3) AC line status                                           
   *    0x00: Off-line                                           
   *    0x01: On-line                                            
   *    0x02: On backup power (BIOS >= 1.1 only)                 
   *    0xff: Unknown                                            
   * 4) Battery status                                           
   *    0x00: High                                               
   *    0x01: Low                                                
   *    0x02: Critical                                           
   *    0x03: Charging                                           
   *    0x04: Selected battery not present (BIOS >= 1.2 only)    
   *    0xff: Unknown                                            
   * 5) Battery flag                                             
   *    bit 0: High                                              
   *    bit 1: Low                                               
   *    bit 2: Critical                                          
   *    bit 3: Charging                                          
   *    bit 7: No system battery                                 
   *    0xff: Unknown                                            
   * 6) Remaining battery life (percentage of charge):           
   *    0-100: valid                                             
   *    -1: Unknown                                              
   * 7) Remaining battery life (time units):                     
   *    Number of remaining minutes or seconds                   
   *    -1: Unknown                                              
   * 8) min = minutes; sec = seconds
   */

   /* open /proc/apm */
   if(fdsc == -1) { /* if not already opened, open apm */
     if((fdsc = open("/proc/apm", O_RDONLY)) == -1) {
       return -1;
     }
   }

   /* seek to the beginning of the file */
   if(lseek(fdsc, 0, SEEK_SET) != 0 ) {
     return -1;
   }

  /* read apm state */
  if(read(fdsc, buffer, sizeof(buffer)) != sizeof(buffer)) {
    return -1;
  }

  /* tokenize the apm string */
  tokens[0] = strtok(buffer, " ");
  for(i = 1 ; i < sizeof(tokens)/sizeof(char*) ; i++) {
    tokens[i] = strtok(NULL, " ");
  }

  /* determine the AC line status */
  switch(strtol(tokens[3], NULL, 16)) {
    case 0x00:
      thinkpad_state->ac_state = STATE_OFF;
      break;

    case 0x01:
      thinkpad_state->ac_state = STATE_ON;
      break;
  }

  return 0;
} /* }}} */

#ifdef HAVE_LIBX11
/* get key events from X */
int xgrabkey(t_thinkpad_state *thinkpad_state, Display *display) /* {{{ */
{
  XEvent event;

  while(XPending(display) > 0) {
    XNextEvent(display, &event);

    if (event.xkey.type == KeyPress) { /* needed, because KeyReleases are also received, even if XSelectInput only specifies KeyPressMask!! */
      switch(event.xkey.keycode) {
	case KEYCODE_HOME:
	  thinkpad_state->home_toggle ^= 0x02;
	  break;

	case KEYCODE_SEARCH:
	  thinkpad_state->search_toggle ^= 0x02;
	  break;

	case KEYCODE_MAIL:
	  thinkpad_state->mail_toggle ^= 0x02;
	  break;

	case KEYCODE_FAVORITES:
	  thinkpad_state->favorites_toggle ^= 0x02;
	  break;

	case KEYCODE_RELOAD:
	  thinkpad_state->reload_toggle ^= 0x02;
	  break;

	case KEYCODE_ABORT:
	  thinkpad_state->abort_toggle ^= 0x02;
	  break;

	case KEYCODE_BACKWARD:
	  thinkpad_state->backward_toggle ^= 0x02;
	  break;

	case KEYCODE_FORWARD:
	  thinkpad_state->forward_toggle ^= 0x02;
	  break;

	case KEYCODE_FN_KEY:
	  thinkpad_state->fn_toggle ^= 0x02;
	  break;

	default:
	  break;
      }
    }
  }

  return 0;
} /* }}} */
#endif /* HAVE_LIBX11 */

/* fork an application */
int fork_app(char * cmd) /* {{{ */
{
  enum mode_t {SKIP, IN_QUOTED, IN_UNQUOTED};

  char *cmd_cpy;
  int cmd_len;
  char *args[CALLBACK_CMD_ARGS]; 
  int ptr;
  int argc;
  enum mode_t mode;
  char quote_char;
  int fdsc, fdsc_max;

  switch(fork()) {
    case -1:
      return -1;
      break;

    case 0:
      ptr = 0;
      argc = 0;
      mode = SKIP;
      quote_char = '"';
      memset(args, 0, sizeof(args));
      cmd_cpy = strdup(cmd);
      cmd_len = strlen(cmd_cpy);

      /* close ALL file descriptors except stdin, stdout and stderr */
      for (fdsc = 3, fdsc_max = sysconf(_SC_OPEN_MAX); fdsc < fdsc_max; fdsc++) {
        close(fdsc);
      }

      /* generate an array of cmd_cpy string */
      while (ptr < cmd_len) {
	switch (mode) {
	  /* skip leading whitespaces */
	  case SKIP:
	    if (cmd_cpy[ptr] != ' ' && cmd_cpy[ptr] != '\t') {
	      if (cmd_cpy[ptr] == '"' || cmd_cpy[ptr] == '\'') {
		mode = IN_QUOTED;
		quote_char = cmd_cpy[ptr];
		ptr++;
	      }
	      else {
		mode = IN_UNQUOTED;
	      }
	      args[argc] = &cmd_cpy[ptr];
	      argc++;
	    }
	    ptr++;
	    break;

	  /* within a quoted string */
	  case IN_QUOTED:
	    if (cmd_cpy[ptr] == quote_char) {
	      cmd_cpy[ptr] = '\0';
	      mode = SKIP;
	    }
	    ptr++;
	    break;

	  /* within a unquoted string */
	  case IN_UNQUOTED:
	    if (cmd_cpy[ptr] == ' ' || cmd_cpy[ptr] == '\t') { /* this is the end of the string */
	      cmd_cpy[ptr] = '\0';
	      mode = SKIP;
	    }
	    ptr++;
	    break;
	}
      }

      setsid(); /* children should not be killed if tpb ends */
      chdir(getenv("HOME")); /* try to change to users home directory */
      execv(args[0], args);

      /* should never be reached, execv only returns on error */
      fprintf(stderr, _("Unable to fork application \"%s\".\n"), args[0]); 
      fprintf(stderr, _("HINT:\n")); 
      fprintf(stderr, _("With tpb version 0.6.2 or later you need to specify applications\n")); 
      fprintf(stderr, _("using the whole path to the executable program.\n")); 
      free(cmd_cpy);
      _exit(0);
      return 127;
      break;

    default:
      return 0;
      break;
  }
  return -1; /* never reached */
} /* }}} */

/* set the volume level in the nvram */
void set_nvram_volume_level(t_thinkpad_state *thinkpad_state) /* {{{ */
{
  int  fdsc;
  char buffer;

  /* only use writeback to nvram when cfg.mixersteps is different from DEFAULT_MIXERSTEPS */
  if(cfg.mixersteps != DEFAULT_MIXERSTEPS) {
    /* open nvram */
    if((fdsc = open(cfg.nvram, O_RDWR|O_NONBLOCK)) == -1) {
      fprintf(stderr, _("Unable to open device %s: "), cfg.nvram);
      perror(NULL);
      fprintf(stderr, _("To use mixersteps other than %d you need write access to %s.\n"), cfg.mixersteps, cfg.nvram);
      _exit(1);
    }

    /* jump to volume section */
    if(lseek(fdsc, 0x60, SEEK_SET) == -1 ) {
      fprintf(stderr, _("Unable to seek device %s: "), cfg.nvram);
      perror(NULL);
      _exit(1);
    }

    /* read nvram */
    if(read(fdsc, &buffer,sizeof(buffer)) != sizeof(buffer)) {
      fprintf(stderr, _("Unable to read from device %s: "), cfg.nvram);
      perror(NULL);
      _exit(1);
    }

    thinkpad_state->volume_level = 0x07; /* set volume_level to the value we write back to nvram */
    buffer &= 0xf0;
    buffer |= thinkpad_state->volume_level;

    /* jump to volume section */
    if(lseek(fdsc, 0x60, SEEK_SET) == -1 ) {
      fprintf(stderr, _("Unable to seek device %s: "), cfg.nvram);
      perror(NULL);
      _exit(1);
    }

    /* write std value for volume */
    if(write(fdsc, &buffer, sizeof(buffer)) != sizeof(buffer)) {
      fprintf(stderr, _("Unable to write to device %s: "), cfg.nvram);
      perror(NULL);
      _exit(1);
    }

    close(fdsc);
  }

  return;

} /* }}} */

/* change the volume level of the OSS mixer device */
unsigned int change_volume(int change) /* {{{ */
{
  int mixer;
  unsigned int volume;
  unsigned int left,right;

  /* open mixer */
  if((mixer = open(cfg.mixerdev, O_RDWR)) == -1) {
    fprintf(stderr, _("Unable to open mixer device %s: "), cfg.mixerdev);
    perror(NULL);
    _exit(1);
  }

  /* read mixer volume */
  if(ioctl(mixer, SOUND_MIXER_READ_VOLUME, &volume) == -1) {
    fprintf(stderr, _("Unable to read volume from mixer device %s: "), cfg.mixerdev);
    perror(NULL);
    _exit(1);
  }

  /* adjust volume */
  if(((int)(volume & 0xff) + change) < 0) {
    left = 0;
  }
  else {
    left = (volume & 0xff) + change;
    if(left > MAX_VOLUME) {
      left = MAX_VOLUME;
    }
  }
  if((((int)(volume >> 8) & 0xff) + change) < 0) {
    right = 0;
  }
  else {
    right = ((volume >> 8) & 0xff) + change;
    if(right > MAX_VOLUME) {
      right = MAX_VOLUME;
    }
  }
  volume = left | (right << 8);

  /* write volume back to mixer */
  if(ioctl(mixer, SOUND_MIXER_WRITE_VOLUME, &volume) == -1) {
    fprintf(stderr, _("Unable to write volume to mixer device %s: "), cfg.mixerdev);
    perror(NULL);
    _exit(1);
  }

  /* close mixer device */
  if(close(mixer) == -1 ) {
    fprintf(stderr, _("Unable to close mixer device %s: "), cfg.mixerdev);
    perror(NULL);
    _exit(1);
  }

  /* calc volume percentage and return it */
  return ((left + right) / 2) * 100 / MAX_VOLUME;

} /* }}} */

/* check if apmiser is running */
int apmiser_running(void) /* {{{ */
{
  /* code inspired by comp.unix.programmer FAQ */
  char line[133];
  char *linep;
  char *token;
  char *cmd = NULL;
  FILE *fp;

  /* open command like a file */
  fp = popen("ps -e 2>/dev/null", "r");
  if (fp == NULL) {
    return 0;
  }

  /* get header */
  if (fgets(line, sizeof(line), fp) == NULL) {
    pclose(fp);
    return 0;
  }

  /* determine column of command name */
  linep = line;
  while(cmd == NULL)
  {
    if ((token = strtok(linep, " \t\n")) == NULL) {
      pclose(fp);
      return 0;
    }
    linep = NULL;

    if (strcmp("COMMAND", token) == 0 || strcmp("CMD", token) == 0) {
      cmd = token;
    }
  }

  /* check if any of the commands is apmiser */
  while(fgets(line, sizeof line, fp) != NULL) {
    if (strstr(strtok(cmd, " \t\n"), "apmiser") != NULL) {
      pclose(fp);
      return 1;
    }
  }

  pclose(fp);

  return 0;
} /* }}} */

#ifdef HAVE_LIBX11
/* handler for X errors */
int *xerrorhandler(Display *display, XErrorEvent *event) /* {{{ */
{
  static int called = 0;

  if (called == 0) {
    called = 1;
    fputs(_("There is already a application grabbing some ThinkPad keys.\n"), stderr);
    fputs(_("Possibly you restarted tpb too fast.\n"), stderr);
  }

  return(NULL);
} /* }}} */
#endif /* HAVE_LIBX11 */

/* handler for SIG_CHLD */
void sig_chld_handler(int signo) /* {{{ */
{   
  int status;
  waitpid(-1, &status, WNOHANG);
  return;
} /* }}} */

/* vim600:set fen:set fdm=marker:set fdl=0: */
