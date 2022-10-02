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
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "config.h"

#if ENABLE_NLS
#include <libintl.h>
#endif /* ENABLE_NLS */

#ifdef HAVE_LIBXOSD
#include <xosd.h>
#endif /* HAVE_LIBXOSD */

#include "cfg.h"
/* }}} */

/* RCS version string for later identification using rcs ident {{{*/
#ifndef lint
static volatile const char *RCSid = "@(#) $Id: cfg.c,v 1.15 2005/07/18 14:15:59 mbr Exp $";
#endif /* lint */
/* }}} */

/* global variables  {{{ */
config cfg;
/* }}} */

/* function prototypes {{{ */
void find_nvram(config *cfg);
/* }}} */

/* parse the config file and override the given cfg with the values of the file */
int parse_cfg_file(config *cfg, char *name) { /* {{{ */
  FILE *file_desc;
  char keyword_buffer[BUFFER_SIZE];
  char argument_buffer[BUFFER_SIZE];
  int next_char;
  int keyword_buffer_ptr = 0;
  int argument_buffer_ptr = 0;
  enum mode mode = MODE_INDENT;
  config file_cfg;

  /* clear the config */
  clear_cfg(&file_cfg);

  /* open configuration file */
  if((file_desc = fopen(name, "r")) == NULL) {
    return -1;
  }

  /* loop until we reach the end of the file {{{ */
  while(mode != MODE_END) {
    next_char = fgetc(file_desc);
    switch(next_char) {
      /* case '=': */
      case ' ':
      case '\t':
	switch(mode) {
	  case MODE_KEYWORD:
	    mode = MODE_SEPARATOR;
	    break;

	  case MODE_ARGUMENT:
	    if(argument_buffer_ptr < BUFFER_SIZE - 1) {
	      argument_buffer[argument_buffer_ptr++] = (char)next_char;
	    }
	    else {
	      fputs(_("Argument in configuration too long!\n"), stderr);
	      _exit(1);
	    }
	    break;

	  default:
	    break;
	}
	break;

      case '\n':
	mode = MODE_INDENT;
	keyword_buffer[keyword_buffer_ptr] = '\0';
	argument_buffer[argument_buffer_ptr] = '\0';
	if(strlen(keyword_buffer) > 0 && strlen(argument_buffer) > 0) {
	  argument_buffer_ptr--;
	  while(argument_buffer[argument_buffer_ptr] == ' ' || argument_buffer[argument_buffer_ptr] == '\t') {
	    argument_buffer[argument_buffer_ptr] = '\0';
	    argument_buffer_ptr--;
	  }
	  set_value(keyword_buffer, argument_buffer, &file_cfg);
	}
	/* if(strlen(keyword_buffer) >0 && strlen(argument_buffer)>0) printf("Keyword: \"%s\", Argument: \"%s\"\n",keyword_buffer,argument_buffer); */
	keyword_buffer_ptr = 0;
	argument_buffer_ptr = 0;
	break;

      case '#':
	mode = MODE_COMMENT;
	break;

      case EOF:
	mode = MODE_END;
	break;

      default:
	switch(mode) {
	  case MODE_INDENT:
	    mode = MODE_KEYWORD; /* no break is intentional */
	  case MODE_KEYWORD:
	    if(keyword_buffer_ptr < BUFFER_SIZE - 1) {
	      keyword_buffer[keyword_buffer_ptr++] = tolower(next_char);
	    }
	    else {
	      fputs(_("Keyword in configuration too long!\n"), stderr);
	      _exit(1);
	    }
	    break;

	  case MODE_SEPARATOR:
	    mode = MODE_ARGUMENT; /* no break is intentional */
	  case MODE_ARGUMENT:
	    if(argument_buffer_ptr < BUFFER_SIZE - 1) {
	      argument_buffer[argument_buffer_ptr++] = (char)next_char;
	    }
	    else {
	      fputs(_("Argument in configuration too long!\n"), stderr);
	      _exit(1);
	    }
	    break;

	  default:
	    break;
	}
	break;
    }
  } /* }}} */

  /* override the config with values from configuration file */
  override_cfg(&file_cfg, cfg);

  /* free all strings allocated in file_cfg */
  free_cfg(&file_cfg);

  return 0;
} /* }}} */

/* parse options and override the given cfg with the values of the file */ 
void parse_option(config *cfg, int argc, char **argv) { /* {{{ */
  config cmd_cfg;

  /* clear the config */
  clear_cfg(&cmd_cfg);

  while (0 == 0) {
    int option_index = 0;
    int next_option;
    static struct option long_options[] = {
      {"help",     0, NULL, 'h'},
      {"daemon",   0, NULL, 'd'},
      {"apm",      1, NULL, 'A'},
      {"powermgt", 1, NULL, 'P'},
      {"config",   1, NULL, 'c'},
      {"callback", 1, NULL, 'C'},
      {"mixer",    1, NULL, 'm'},
#ifdef HAVE_LIBXOSD
      {"osd",      1, NULL, 'o'},
#endif /* HAVE_LIBXOSD */
      {"polltime", 1, NULL, 'p'},
      {"thinkpad", 1, NULL, 't'},
      {"home",     1, NULL, 'H'},
      {"search",   1, NULL, 'S'},
      {"mail",     1, NULL, 'M'},
      {"wireless", 1, NULL, 'W'},
      {"xevents",  1, NULL, 'x'},
      {"verbose",  0, NULL, 'v'},
      {NULL,       0, NULL,   0}
    };

#ifdef HAVE_LIBXOSD
    next_option = getopt_long (argc, argv, "hdA:P:c:C:m:o:p:t:H:S:M:W:x:v", long_options, &option_index);
#else /* HAVE_LIBXOSD */
    next_option = getopt_long (argc, argv, "hdA:P:c:C:m:p:t:H:S:M:W:x:v", long_options, &option_index);
#endif /* HAVE_LIBXOSD */

    if (next_option == -1) {
      break;
    }

    switch (next_option) {
      case 'h':
	print_usage(argv[0]);
	_exit(1);
	break;

      case 'd':
	cmd_cfg.daemon = STATE_ON;
	break;

      case 'A':
	set_value(CFG_APM, optarg, &cmd_cfg);
	break;

      case 'P':
	set_value(CFG_POWERMGT, optarg, &cmd_cfg);
	break;

      case 'c':
	if(parse_cfg_file(cfg, optarg) != 0) {
	  fprintf(stderr, _("Unable to open config file %s: "), optarg);
	  perror(NULL);
	}
	break;

      case 'C':
	set_value(CFG_CALLBACK, optarg, &cmd_cfg);
	break;

      case 'm':
	set_value(CFG_MIXER, optarg, &cmd_cfg);
	break;

#ifdef HAVE_LIBXOSD
      case 'o':
	set_value(CFG_OSD, optarg, &cmd_cfg);
	break;
#endif /* HAVE_LIBXOSD */

      case 'p':
	set_value(CFG_POLLTIME, optarg, &cmd_cfg);
	break;

      case 't':
	set_value(CFG_THINKPAD, optarg, &cmd_cfg);
	break;

      case 'H':
	set_value(CFG_HOME, optarg, &cmd_cfg);
	break;

      case 'S':
	set_value(CFG_SEARCH, optarg, &cmd_cfg);
	break;

      case 'M':
	set_value(CFG_MAIL, optarg, &cmd_cfg);
	break;

      case 'W':
	set_value(CFG_WIRELESS, optarg, &cmd_cfg);
	break;

      case 'x':
	set_value(CFG_XEVENTS, optarg, &cmd_cfg);
	break;

      case 'v':
	cmd_cfg.verbose = STATE_ON;
	break;

      default:
	print_usage(argv[0]);
	_exit(1);
    }
  }

  /* override the config with values set on command line */
  override_cfg(&cmd_cfg, cfg);

  /* free all strings allocated in cmd_cfg */
  free_cfg(&cmd_cfg);

  return;
} /* }}} */

/* initialize a configuration with default values */
void init_cfg(config *cfg) { /* {{{ */
  /* set all values to defaults */

  /* cfg->daemon */
  cfg->daemon = DEFAULT_DAEMON;

  /* cfg->apm */
  cfg->apm = DEFAULT_APM;

  /* cfg->powermgt */
  cfg->powermgt = DEFAULT_POWERMGT;

  /* cfg->xevents */
  cfg->xevents = DEFAULT_XEVENTS;

  /* cfg->nvram */
  if(cfg->nvram != NULL) {
    free(cfg->nvram);
  }
  cfg->nvram = NULL;
  /* find nvram device file */
  find_nvram(cfg);

  /* cfg->polltime */
  cfg->polltime = DEFAULT_POLLTIME;

  /* cfg->tpb_cmd */
  if(cfg->tpb_cmd != NULL) {
    free(cfg->tpb_cmd);
  }
  cfg->tpb_cmd = NULL;

  /* cfg->home_cmd */
  if(cfg->home_cmd != NULL) {
    free(cfg->home_cmd);
  }
  cfg->home_cmd = NULL;

  /* cfg->search_cmd */
  if(cfg->search_cmd != NULL) {
    free(cfg->search_cmd);
  }
  cfg->search_cmd = NULL;

  /* cfg->mail_cmd */
  if(cfg->mail_cmd != NULL) {
    free(cfg->mail_cmd);
  }
  cfg->mail_cmd = NULL;

  /* cfg->wireless_cmd */
  if(cfg->wireless_cmd != NULL) {
    free(cfg->wireless_cmd);
  }
  cfg->wireless_cmd = NULL;

  /* cfg->favorites_cmd */
  if(cfg->favorites_cmd != NULL) {
    free(cfg->favorites_cmd);
  }
  cfg->favorites_cmd = NULL;

  /* cfg->reload_cmd */
  if(cfg->reload_cmd != NULL) {
    free(cfg->reload_cmd);
  }
  cfg->reload_cmd = NULL;

  /* cfg->abort_cmd */
  if(cfg->abort_cmd != NULL) {
    free(cfg->abort_cmd);
  }
  cfg->abort_cmd = NULL;

  /* cfg->backward_cmd */
  if(cfg->backward_cmd != NULL) {
    free(cfg->backward_cmd);
  }
  cfg->backward_cmd = NULL;

  /* cfg->forward_cmd */
  if(cfg->forward_cmd != NULL) {
    free(cfg->forward_cmd);
  }
  cfg->forward_cmd = NULL;

  /* cfg->fn_cmd */
  if(cfg->fn_cmd != NULL) {
    free(cfg->fn_cmd);
  }
  cfg->fn_cmd = NULL;

  /* cfg->callback */
  if(cfg->callback != NULL) {
    free(cfg->callback);
  }
  cfg->callback = NULL;

  /* cfg->mixer */
  cfg->mixer = DEFAULT_MIXER;

  /* cfg->mixersteps */
  cfg->mixersteps = DEFAULT_MIXERSTEPS;

  /* cfg->mixerdev */
  if(cfg->mixerdev != NULL) {
    free(cfg->mixerdev);
  }
  if((cfg->mixerdev = strdup(DEFAULT_MIXERDEV)) == NULL) {
    fputs(_("Not enough memory"), stderr);
    _exit(1);
  }

  /* cfg->verbose */
  cfg->verbose = DEFAULT_VERBOSE;

#ifdef HAVE_LIBXOSD
  /* cfg->osd */
  cfg->osd = DEFAULT_OSD;

  /* cfg->osdzoom */
  cfg->osdzoom = DEFAULT_OSDZOOM;

  /* cfg->osdthinklight */
  cfg->osdthinklight = DEFAULT_OSDTHINKLIGHT;

  /* cfg->osddisplay */
  cfg->osddisplay = DEFAULT_OSDDISPLAY;

  /* cfg->osdhvexpansion */
  cfg->osdhvexpansion = DEFAULT_OSDHVEXPANSION;

  /* cfg->osdbrightness */
  cfg->osdbrightness = DEFAULT_OSDBRIGHTNESS;

  /* cfg->osdvolume */
  cfg->osdvolume = DEFAULT_OSDVOLUME;

  /* cfg->osdpowermgt */
  cfg->osdpowermgt = DEFAULT_OSDPOWERMGT;

  /* cfg->osdfont */
  if(cfg->osdfont != NULL) {
    free(cfg->osdfont);
  }
  if((cfg->osdfont = strdup(DEFAULT_OSDFONT)) == NULL) {
    fputs(_("Not enough memory"), stderr);
    _exit(1);
  }

  /* cfg->osdcolor */
  if(cfg->osdcolor != NULL) {
    free(cfg->osdcolor);
  }
  if((cfg->osdcolor = strdup(DEFAULT_OSDCOLOR)) == NULL) {
    fputs(_("Not enough memory"), stderr);
    _exit(1);
  }

  /* cfg->osdtimeout */
  cfg->osdtimeout = DEFAULT_OSDTIMEOUT;

  /* cfg->osdshadow */
  cfg->osdshadow = DEFAULT_OSDSHADOW;

#if HAVE_LIBXOSD_VERSION >= 20200
  /* cfg->osdshadowcolor */
  cfg->osdshadowcolor = DEFAULT_OSDSHADOWCOLOR;

  /* cfg->osdoutline */
  cfg->osdoutline = DEFAULT_OSDOUTLINE;

  /* cfg->osdoutlinecolor */
  cfg->osdoutlinecolor = DEFAULT_OSDOUTLINECOLOR;
#endif /* HAVE_LIBXOSD_VERSION */

  /* cfg->osdvertical */
  cfg->osdvertical = DEFAULT_OSDVERTICAL;

  /* cfg->osdhorizontal */
  cfg->osdhorizontal = DEFAULT_OSDHORIZONTAL;

  /* cfg->osdpos */
  cfg->osdpos = DEFAULT_OSDPOS;

#if HAVE_LIBXOSD_VERSION >= 10000
  /* cfg->osdalign */
  cfg->osdalign = DEFAULT_OSDALIGN;
#endif /* HAVE_LIBXOSD_VERSION */
#endif /* HAVE_LIBXOSD */

  return;
} /* }}} */

/* clear a configuration */
void clear_cfg(config *cfg) { /* {{{ */
  /* cfg->daemon */
  cfg->daemon = UNDEFINED;

  /* cfg->apm */
  cfg->apm = UNDEFINED;

  /* cfg->powermgt */
  cfg->powermgt = UNDEFINED;

  /* cfg->xevents */
  cfg->xevents = UNDEFINED;

  /* cfg->nvram */
  cfg->nvram = NULL;

  /* cfg->polltime */
  cfg->polltime = UNDEFINED;

  /* cfg->tpb_cmd */
  cfg->tpb_cmd = NULL;

  /* cfg->home_cmd */
  cfg->home_cmd = NULL;

  /* cfg->search_cmd */
  cfg->search_cmd = NULL;

  /* cfg->mail_cmd */
  cfg->mail_cmd = NULL;

  /* cfg->wireless_cmd */
  cfg->wireless_cmd = NULL;

  /* cfg->favorites_cmd */
  cfg->favorites_cmd = NULL;

  /* cfg->reload_cmd */
  cfg->reload_cmd = NULL;

  /* cfg->abort_cmd */
  cfg->abort_cmd = NULL;

  /* cfg->backward_cmd */
  cfg->backward_cmd = NULL;

  /* cfg->forward_cmd */
  cfg->forward_cmd = NULL;

  /* cfg->fn_cmd */
  cfg->fn_cmd = NULL;

  /* cfg->callback */
  cfg->callback = NULL;

  /* cfg->mixer */
  cfg->mixer = UNDEFINED;

  /* cfg->mixersteps */
  cfg->mixersteps = UNDEFINED;

  /* cfg->mixerdev */
  cfg->mixerdev = NULL;

  /* cfg->verbose */
  cfg->verbose = UNDEFINED;

#ifdef HAVE_LIBXOSD
  /* cfg->osd */
  cfg->osd = UNDEFINED;

  /* cfg->osdzoom */
  cfg->osdzoom = UNDEFINED;

  /* cfg->osdthinklight */
  cfg->osdthinklight = UNDEFINED;

  /* cfg->osddisplay */
  cfg->osddisplay = UNDEFINED;

  /* cfg->osdhvexpansion */
  cfg->osdhvexpansion = UNDEFINED;

  /* cfg->osdbrightness */
  cfg->osdbrightness = UNDEFINED;

  /* cfg->osdvolume */
  cfg->osdvolume = UNDEFINED;

  /* cfg->osdpowermgt */
  cfg->osdpowermgt = UNDEFINED;

  /* cfg->osdfont */
  cfg->osdfont = NULL;

  /* cfg->osdcolor */
  cfg->osdcolor = NULL;

  /* cfg->osdtimeout */
  cfg->osdtimeout = UNDEFINED;

  /* cfg->osdshadow */
  cfg->osdshadow = UNDEFINED;

#if HAVE_LIBXOSD_VERSION >= 20200
  /* cfg->osdshadowcolor */
  cfg->osdshadowcolor = NULL;

  /* cfg->osdoutline */
  cfg->osdoutline = UNDEFINED;

  /* cfg->osdoutlinecolor */
  cfg->osdoutlinecolor = NULL;
#endif /* HAVE_LIBXOSD_VERSION */

  /* cfg->osdvertical */
  cfg->osdvertical = UNDEFINED;

  /* cfg->osdhorizontal */
  cfg->osdhorizontal = UNDEFINED;

  /* cfg->osdpos */
  cfg->osdpos = UNDEFINED;

#if HAVE_LIBXOSD_VERSION >= 10000
  /* cfg->osdalign */
  cfg->osdalign = UNDEFINED;
#endif /* HAVE_LIBXOSD_VERSION */
#endif /* HAVE_LIBXOSD */

  return;
} /* }}} */

/* release all allocated strings in a configuration */
void free_cfg(config *cfg) { /* {{{ */
  /* cfg->nvram */
  if(cfg->nvram != NULL) {
    free(cfg->nvram);
  }

  /* cfg->tpb_cmd */
  if(cfg->tpb_cmd != NULL) {
    free(cfg->tpb_cmd);
  }

  /* cfg->home_cmd */
  if(cfg->home_cmd != NULL) {
    free(cfg->home_cmd);
  }

  /* cfg->search_cmd */
  if(cfg->search_cmd != NULL) {
    free(cfg->search_cmd);
  }

  /* cfg->mail_cmd */
  if(cfg->mail_cmd != NULL) {
    free(cfg->mail_cmd);
  }

  /* cfg->wireless_cmd */
  if(cfg->wireless_cmd != NULL) {
    free(cfg->wireless_cmd);
  }

  /* cfg->favorites_cmd */
  if(cfg->favorites_cmd != NULL) {
    free(cfg->favorites_cmd);
  }

  /* cfg->reload_cmd */
  if(cfg->reload_cmd != NULL) {
    free(cfg->reload_cmd);
  }

  /* cfg->abort_cmd */
  if(cfg->abort_cmd != NULL) {
    free(cfg->abort_cmd);
  }

  /* cfg->backward_cmd */
  if(cfg->backward_cmd != NULL) {
    free(cfg->backward_cmd);
  }

  /* cfg->forward_cmd */
  if(cfg->forward_cmd != NULL) {
    free(cfg->forward_cmd);
  }

  /* cfg->fn_cmd */
  if(cfg->fn_cmd != NULL) {
    free(cfg->fn_cmd);
  }

  /* cfg->callback */
  if(cfg->callback != NULL) {
    free(cfg->callback);
  }
  /* cfg->mixerdev */
  if(cfg->mixerdev != NULL) {
    free(cfg->mixerdev);
  }

#ifdef HAVE_LIBXOSD
  /* cfg->osdfont */
  if(cfg->osdfont != NULL) {
    free(cfg->osdfont);
  }

  /* cfg->osdcolor */
  if(cfg->osdcolor != NULL) {
    free(cfg->osdcolor);
  }

#endif /* HAVE_LIBXOSD */

  return;
} /* }}} */

/* all values set in the master override those in the slave */
void override_cfg(config *master, config *slave) { /* {{{ */
  /* override the defaults if the are set in master struct */

  /* cfg->daemon */
  if(master->daemon != UNDEFINED) {
    slave->daemon = master->daemon;
  }

  /* cfg->apm */
  if(master->apm != UNDEFINED) {
    slave->apm = master->apm;
  }

  /* cfg->powermgt */
  if(master->powermgt != UNDEFINED) {
    slave->powermgt = master->powermgt;
  }

  /* cfg->xevents */
  if(master->xevents != UNDEFINED) {
    slave->xevents = master->xevents;
  }

  /* cfg->nvram */
  if(master->nvram != NULL) {
    if(slave->nvram != NULL) {
      free(slave->nvram);
    }
    slave->nvram = strdup(master->nvram);
  }

  /* cfg->polltime */
  if(master->polltime != UNDEFINED) {
    slave->polltime = master->polltime;
  }

  /* cfg->tpb_cmd */
  if(master->tpb_cmd != NULL) {
    if(slave->tpb_cmd != NULL) {
      free(slave->tpb_cmd);
    }
    slave->tpb_cmd = strdup(master->tpb_cmd);
  }

  /* cfg->home_cmd */
  if(master->home_cmd != NULL) {
    if(slave->home_cmd != NULL) {
      free(slave->home_cmd);
    }
    slave->home_cmd = strdup(master->home_cmd);
  }

  /* cfg->search_cmd */
  if(master->search_cmd != NULL) {
    if(slave->search_cmd != NULL) {
      free(slave->search_cmd);
    }
    slave->search_cmd = strdup(master->search_cmd);
  }

  /* cfg->mail_cmd */
  if(master->mail_cmd != NULL) {
    if(slave->mail_cmd != NULL) {
      free(slave->mail_cmd);
    }
    slave->mail_cmd = strdup(master->mail_cmd);
  }

  /* cfg->wireless_cmd */
  if(master->wireless_cmd != NULL) {
    if(slave->wireless_cmd != NULL) {
      free(slave->wireless_cmd);
    }
    slave->wireless_cmd = strdup(master->wireless_cmd);
  }

  /* cfg->favorites_cmd */
  if(master->favorites_cmd != NULL) {
    if(slave->favorites_cmd != NULL) {
      free(slave->favorites_cmd);
    }
    slave->favorites_cmd = strdup(master->favorites_cmd);
  }

  /* cfg->reload_cmd */
  if(master->reload_cmd != NULL) {
    if(slave->reload_cmd != NULL) {
      free(slave->reload_cmd);
    }
    slave->reload_cmd = strdup(master->reload_cmd);
  }

  /* cfg->abort_cmd */
  if(master->abort_cmd != NULL) {
    if(slave->abort_cmd != NULL) {
      free(slave->abort_cmd);
    }
    slave->abort_cmd = strdup(master->abort_cmd);
  }

  /* cfg->backward_cmd */
  if(master->backward_cmd != NULL) {
    if(slave->backward_cmd != NULL) {
      free(slave->backward_cmd);
    }
    slave->backward_cmd = strdup(master->backward_cmd);
  }

  /* cfg->forward_cmd */
  if(master->forward_cmd != NULL) {
    if(slave->forward_cmd != NULL) {
      free(slave->forward_cmd);
    }
    slave->forward_cmd = strdup(master->forward_cmd);
  }

  /* cfg->fn_cmd */
  if(master->fn_cmd != NULL) {
    if(slave->fn_cmd != NULL) {
      free(slave->fn_cmd);
    }
    slave->fn_cmd = strdup(master->fn_cmd);
  }

  /* cfg->callback */
  if(master->callback != NULL) {
    if(slave->callback != NULL) {
      free(slave->callback);
    }
    slave->callback = strdup(master->callback);
  }

  /* cfg->mixer */
  if(master->mixer != UNDEFINED) {
    slave->mixer = master->mixer;
  }

  /* cfg->mixersteps */
  if(master->mixersteps != UNDEFINED) {
    slave->mixersteps = master->mixersteps;
  }

  /* cfg->mixerdev */
  if(master->mixerdev != NULL) {
    if(slave->mixerdev != NULL) {
      free(slave->mixerdev);
    }
    slave->mixerdev = strdup(master->mixerdev);
  }

  /* cfg->verbose */
  if(master->verbose != UNDEFINED) {
    slave->verbose = master->verbose;
  }

#ifdef HAVE_LIBXOSD
  /* cfg->osd */
  if(master->osd != UNDEFINED) {
    slave->osd = master->osd;
  }

  /* cfg->osdzoom */
  if(master->osdzoom != UNDEFINED) {
    slave->osdzoom = master->osdzoom;
  }

  /* cfg->osdthinklight */
  if(master->osdthinklight != UNDEFINED) {
    slave->osdthinklight = master->osdthinklight;
  }

  /* cfg->osddisplay */
  if(master->osddisplay != UNDEFINED) {
    slave->osddisplay = master->osddisplay;
  }

  /* cfg->osdhvexpansion */
  if(master->osdhvexpansion != UNDEFINED) {
    slave->osdhvexpansion = master->osdhvexpansion;
  }

  /* cfg->osdbrightness */
  if(master->osdbrightness != UNDEFINED) {
    slave->osdbrightness = master->osdbrightness;
  }

  /* cfg->osdvolume */
  if(master->osdvolume != UNDEFINED) {
    slave->osdvolume = master->osdvolume;
  }

  /* cfg->osdpowermgt */
  if(master->osdpowermgt != UNDEFINED) {
    slave->osdpowermgt = master->osdpowermgt;
  }

  /* cfg->osdfont */
  if(master->osdfont != NULL) {
    if(slave->osdfont != NULL) {
      free(slave->osdfont);
    }
    slave->osdfont = strdup(master->osdfont);
  }

  /* cfg->osdcolor */
  if(master->osdcolor != NULL) {
    if(slave->osdcolor != NULL) {
      free(slave->osdcolor);
    }
    slave->osdcolor = strdup(master->osdcolor);
  }

  /* cfg->osdtimeout */
  if(master->osdtimeout != UNDEFINED) {
    slave->osdtimeout = master->osdtimeout;
  }

  /* cfg->osdshadow */
  if(master->osdshadow != UNDEFINED) {
    slave->osdshadow = master->osdshadow;
  }

#if HAVE_LIBXOSD_VERSION >= 20200
  /* cfg->osdshadowcolor */
  if(master->osdshadowcolor != NULL) {
    slave->osdshadowcolor = master->osdshadowcolor;
  }

  /* cfg->osdoutline */
  if(master->osdoutline != UNDEFINED) {
    slave->osdoutline = master->osdoutline;
  }

  /* cfg->osdoutlinecolor */
  if(master->osdoutlinecolor != NULL) {
    slave->osdoutlinecolor = master->osdoutlinecolor;
  }
#endif /* HAVE_LIBXOSD_VERSION */

  /* cfg->osdvertical */
  if(master->osdvertical != UNDEFINED) {
    slave->osdvertical = master->osdvertical;
  }

  /* cfg->osdhorizontal */
  if(master->osdhorizontal != UNDEFINED) {
    slave->osdhorizontal = master->osdhorizontal;
  }

  /* cfg->osdpos */
  if(master->osdpos != UNDEFINED) {
    slave->osdpos = master->osdpos;
  }

#if HAVE_LIBXOSD_VERSION >= 10000
  /* cfg->osdalign */
  if(master->osdalign != UNDEFINED) {
    slave->osdalign = master->osdalign;
  }
#endif /* HAVE_LIBXOSD_VERSION */
#endif /* HAVE_LIBXOSD */

  return;
} /* }}} */

/* set the field for key in cfg to arg */
void set_value(char *key, char *arg, config *cfg) { /* {{{ */
  char *endptr;

  if (strcmp(CFG_APM, key) == 0) { /* {{{ */
    int i = 0;
    while(arg[i] != '\0') {
      arg[i] = tolower(arg[i]);
      i++;
    }
    if(strcmp(CFG_APM_ON, arg) == 0) {
      cfg->apm = STATE_ON;
    }
    else {
      if(strcmp(CFG_APM_OFF, arg) == 0) {
	cfg->apm = STATE_OFF;
      }
      else {
	fprintf(stderr, _("Illegal apm state: %s\n"), arg);
	_exit(1);
      }
    }
  } /* }}} */

  else if (strcmp(CFG_POWERMGT, key) == 0) { /* {{{ */
    int i=0;
    while(arg[i] != '\0') {
      arg[i] = tolower(arg[i]);
      i++;
    }
    if(strcmp(CFG_POWERMGT_ON, arg) == 0) {
      cfg->powermgt = STATE_ON;
    }
    else {
      if(strcmp(CFG_POWERMGT_OFF, arg) == 0) {
	cfg->powermgt = STATE_OFF;
      }
      else {
	if(strcmp(CFG_POWERMGT_AUTO, arg) == 0) {
	  cfg->powermgt = STATE_AUTO;
	}
	else {
	  fprintf(stderr, _("Illegal powermgt state: %s\n"), arg);
	  _exit(1);
	}
      }
    }
  } /* }}} */

  if (strcmp(CFG_XEVENTS, key) == 0) { /* {{{ */
    int i = 0;
    while(arg[i] != '\0') {
      arg[i] = tolower(arg[i]);
      i++;
    }
    if(strcmp(CFG_XEVENTS_ON, arg) == 0) {
      cfg->xevents = STATE_ON;
    }
    else {
      if(strcmp(CFG_XEVENTS_OFF, arg) == 0) {
	cfg->xevents = STATE_OFF;
      }
      else {
	fprintf(stderr, _("Illegal xevents state: %s\n"), arg);
	_exit(1);
      }
    }
  } /* }}} */

  else if (strcmp(CFG_NVRAM, key) == 0) { /* {{{ */
    if(cfg->nvram != NULL) {
      free(cfg->nvram);
    }
    if((cfg->nvram=strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_POLLTIME, key) == 0) { /* {{{ */
    cfg->polltime = (int)strtol(arg, &endptr, 10);
    if(strlen(endptr) > 0 || cfg->polltime < 0) {
      fprintf(stderr, _("Illegal polltime: %s\n"), arg);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_THINKPAD, key) == 0) { /* {{{ */
    if(cfg->tpb_cmd != NULL) {
      free(cfg->tpb_cmd);
    }
    if((cfg->tpb_cmd=strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_HOME, key) == 0) { /* {{{ */
    if(cfg->home_cmd != NULL) {
      free(cfg->home_cmd);
    }
    if((cfg->home_cmd = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_SEARCH, key) == 0) { /* {{{ */
    if(cfg->search_cmd != NULL) {
      free(cfg->search_cmd);
    }
    if((cfg->search_cmd = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_MAIL, key) == 0) { /* {{{ */
    if(cfg->mail_cmd != NULL) {
      free(cfg->mail_cmd);
    }
    if((cfg->mail_cmd = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_WIRELESS, key) == 0) { /* {{{ */
    if(cfg->wireless_cmd != NULL) {
      free(cfg->wireless_cmd);
    }
    if((cfg->wireless_cmd = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_FAVORITES, key) == 0) { /* {{{ */
    if(cfg->favorites_cmd != NULL) {
      free(cfg->favorites_cmd);
    }
    if((cfg->favorites_cmd = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_RELOAD, key) == 0) { /* {{{ */
    if(cfg->reload_cmd != NULL) {
      free(cfg->reload_cmd);
    }
    if((cfg->reload_cmd = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_ABORT, key) == 0) { /* {{{ */
    if(cfg->abort_cmd != NULL) {
      free(cfg->abort_cmd);
    }
    if((cfg->abort_cmd = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_BACKWARD, key) == 0) { /* {{{ */
    if(cfg->backward_cmd != NULL) {
      free(cfg->backward_cmd);
    }
    if((cfg->backward_cmd = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_FORWARD, key) == 0) { /* {{{ */
    if(cfg->forward_cmd != NULL) {
      free(cfg->forward_cmd);
    }
    if((cfg->forward_cmd = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_FN, key) == 0) { /* {{{ */
    if(cfg->fn_cmd != NULL) {
      free(cfg->fn_cmd);
    }
    if((cfg->fn_cmd = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_CALLBACK, key) == 0) { /* {{{ */
    if(cfg->callback != NULL) {
      free(cfg->callback);
    }
    if((cfg->callback = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_MIXER, key) == 0) { /* {{{ */
    int i = 0;
    while(arg[i] != '\0') {
      arg[i] = tolower(arg[i]);
      i++;
    }
    if(strcmp(CFG_MIXER_ON, arg) == 0) {
      cfg->mixer = STATE_ON;
    }
    else {
      if(strcmp(CFG_MIXER_OFF, arg) == 0) {
	cfg->mixer = STATE_OFF;
      }
      else {
	fprintf(stderr, _("Illegal mixer state: %s\n"), arg);
	_exit(1);
      }
    }
  } /* }}} */

  else if (strcmp(CFG_MIXERSTEPS, key) == 0) { /* {{{ */
    cfg->mixersteps = (int)strtol(arg, &endptr, 10);
    if(strlen(endptr) > 0 || cfg->mixersteps < 0) {
      fprintf(stderr, _("Illegal mixersteps: %s\n"), arg);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_MIXERDEV, key) == 0) { /* {{{ */
    if(cfg->mixerdev != NULL) {
      free(cfg->mixerdev);
    }
    if((cfg->mixerdev = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

#ifdef HAVE_LIBXOSD
  else if (strcmp(CFG_OSDFONT, key) == 0) { /* {{{ */
    if(cfg->osdfont != NULL) {
      free(cfg->osdfont);
    }
    if((cfg->osdfont = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_OSDCOLOR, key) == 0) { /* {{{ */
    if(cfg->osdcolor != NULL) {
      free(cfg->osdcolor);
    }
    if((cfg->osdcolor = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_OSDTIMEOUT, key) == 0) { /* {{{ */
    cfg->osdtimeout = (int)strtol(arg, &endptr, 10);
    if(strlen(endptr) > 0 || cfg->osdtimeout < 0) {
      fprintf(stderr, _("Illegal xosd timeout: %s\n"), arg);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_OSDOFFSET, key) == 0) { /* {{{ */
    cfg->osdvertical = (int)strtol(arg, &endptr, 10);
    if(strlen(endptr) > 0) {
      fprintf(stderr, _("Illegal xosd offset: %s\n"), arg);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_OSDSHADOW, key) == 0) { /* {{{ */
    cfg->osdshadow = (int)strtol(arg, &endptr, 10);
    if(strlen(endptr) > 0) {
      fprintf(stderr, _("Illegal xosd shadow offset: %s\n"), arg);
      _exit(1);
    }
  } /* }}} */

#if HAVE_LIBXOSD_VERSION >= 20200
  else if (strcmp(CFG_OSDSHADOWCOLOR, key) == 0) { /* {{{ */
    if(cfg->osdshadowcolor != NULL) {
      free(cfg->osdshadowcolor);
    }
    if((cfg->osdshadowcolor = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_OSDOUTLINE, key) == 0) { /* {{{ */
    cfg->osdoutline = (int)strtol(arg, &endptr, 10);
    if(strlen(endptr) > 0) {
      fprintf(stderr, _("Illegal xosd outline width: %s\n"), arg);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_OSDOUTLINECOLOR, key) == 0) { /* {{{ */
    if(cfg->osdoutlinecolor != NULL) {
      free(cfg->osdoutlinecolor);
    }
    if((cfg->osdoutlinecolor = strdup(arg)) == NULL) {
      fputs(_("Not enough memory"), stderr);
      _exit(1);
    }
  } /* }}} */
#endif /* HAVE_LIBXOSD_VERSION */

  else if (strcmp(CFG_OSDVERTICAL, key) == 0) { /* {{{ */
    cfg->osdvertical = (int)strtol(arg, &endptr, 10);
    if(strlen(endptr) > 0) {
      fprintf(stderr, _("Illegal xosd vertical offset: %s\n"), arg);
      _exit(1);
    }
  } /* }}} */

  else if (strcmp(CFG_OSDHORIZONTAL, key) == 0) { /* {{{ */
#if HAVE_LIBXOSD_VERSION >= 20000
    cfg->osdhorizontal = (int)strtol(arg, &endptr, 10);
    if(strlen(endptr) > 0) {
      fprintf(stderr, _("Illegal xosd horizontal offset: %s\n"), arg);
      _exit(1);
    }
#else /* HAVE_LIBXOSD_VERSION */
    fputs(_("Sorry, OSDHORIZONTAL is only supported by xosd 2.0.0 and above.\n"), stderr);
#endif /* HAVE_LIBXOSD_VERSION */
  } /* }}} */

  else if (strcmp(CFG_OSDPOS, key) == 0) { /* {{{ */
    int i = 0;
    while(arg[i] != '\0') {
      arg[i] = tolower(arg[i]);
      i++;
    }
    if(strcmp(CFG_OSDPOS_TOP, arg) == 0) {
      cfg->osdpos = XOSD_top;
    }
    else {
      if(strcmp(CFG_OSDPOS_MIDDLE, arg) == 0) {
#if HAVE_LIBXOSD_VERSION >= 20000
	cfg->osdpos = XOSD_middle;
#else /* HAVE_LIBXOSD_VERSION */
	fputs(_("Sorry, OSDPOS MIDDLE is only supported by xosd 2.0.0 and above.\n"), stderr);
#endif /* HAVE_LIBXOSD_VERSION */
      }
      else {
	if(strcmp(CFG_OSDPOS_BOTTOM, arg) == 0) {
	  cfg->osdpos = XOSD_bottom;
	}
	else {
	  fprintf(stderr, _("Illegal xosd position: %s\n"), arg);
	  _exit(1);
	}
      }
    }
  } /* }}} */

#if HAVE_LIBXOSD_VERSION > 700
  else if (strcmp(CFG_OSDALIGN, key) == 0) { /* {{{ */
    int i = 0;
    while(arg[i] != '\0') {
      arg[i] = tolower(arg[i]);
      i++;
    }
    if(strcmp(CFG_OSDALIGN_LEFT, arg) == 0) {
      cfg->osdalign = XOSD_left;
    }
    else {
      if(strcmp(CFG_OSDALIGN_CENTER, arg) == 0) {
	cfg->osdalign = XOSD_center;
      }
      else {
	if(strcmp(CFG_OSDALIGN_RIGHT, arg) == 0) {
	  cfg->osdalign = XOSD_right;
	}
	else {
	  fprintf(stderr, _("Illegal xosd alignment: %s\n"), arg);
	  _exit(1);
	}
      }
    }
  } /* }}} */
#endif /* HAVE_LIBXOSD_VERSION */

  else if (strcmp(CFG_OSD, key) == 0) { /* {{{ */
    int i = 0;
    while(arg[i] != '\0') {
      arg[i] = tolower(arg[i]);
      i++;
    }
    if(strcmp(CFG_OSD_ON, arg) == 0) {
      cfg->osd = STATE_ON;
    }
    else {
      if(strcmp(CFG_OSD_OFF, arg) == 0) {
	cfg->osd = STATE_OFF;
      }
      else {
	fprintf(stderr, _("Illegal xosd state: %s\n"), arg);
	_exit(1);
      }
    }
  } /* }}} */

  else if (strcmp(CFG_OSDZOOM, key) == 0) { /* {{{ */
    int i = 0;
    while(arg[i] != '\0') {
      arg[i] = tolower(arg[i]);
      i++;
    }
    if(strcmp(CFG_OSDZOOM_ON, arg) == 0) {
      cfg->osdzoom = STATE_ON;
    }
    else {
      if(strcmp(CFG_OSDZOOM_OFF, arg) == 0) {
	cfg->osdzoom = STATE_OFF;
      }
      else {
	fprintf(stderr, _("Illegal xosd zoom state: %s\n"), arg);
	_exit(1);
      }
    }
  } /* }}} */

  else if (strcmp(CFG_OSDTHINKLIGHT, key) == 0) { /* {{{ */
    int i = 0;
    while(arg[i] != '\0') {
      arg[i] = tolower(arg[i]);
      i++;
    }
    if(strcmp(CFG_OSDTHINKLIGHT_ON, arg) == 0) {
      cfg->osdthinklight = STATE_ON;
    }
    else {
      if(strcmp(CFG_OSDTHINKLIGHT_OFF, arg) == 0) {
	cfg->osdthinklight = STATE_OFF;
      }
      else {
	fprintf(stderr, _("Illegal xosd thinklight state: %s\n"), arg);
	_exit(1);
      }
    }
  } /* }}} */

  else if (strcmp(CFG_OSDDISPLAY, key) == 0) { /* {{{ */
    int i = 0;
    while(arg[i] != '\0') {
      arg[i] = tolower(arg[i]);
      i++;
    }
    if(strcmp(CFG_OSDDISPLAY_ON, arg) == 0) {
      cfg->osddisplay = STATE_ON;
    }
    else {
      if(strcmp(CFG_OSDDISPLAY_OFF, arg) == 0) {
	cfg->osddisplay = STATE_OFF;
      }
      else {
	fprintf(stderr, _("Illegal xosd display state: %s\n"), arg);
	_exit(1);
      }
    }
  } /* }}} */

  else if (strcmp(CFG_OSDHVEXPANSION, key) == 0) { /* {{{ */
    int i = 0;
    while(arg[i] != '\0') {
      arg[i] = tolower(arg[i]);
      i++;
    }
    if(strcmp(CFG_OSDHVEXPANSION_ON, arg) == 0) {
      cfg->osdhvexpansion = STATE_ON;
    }
    else {
      if(strcmp(CFG_OSDHVEXPANSION_OFF, arg) == 0) {
	cfg->osdhvexpansion = STATE_OFF;
      }
      else {
	fprintf(stderr, _("Illegal xosd HV expansion state: %s\n"), arg);
	_exit(1);
      }
    }
  } /* }}} */

  else if (strcmp(CFG_OSDBRIGHTNESS, key) == 0) { /* {{{ */
    int i = 0;
    while(arg[i] != '\0') {
      arg[i] = tolower(arg[i]);
      i++;
    }
    if(strcmp(CFG_OSDBRIGHTNESS_ON, arg) == 0) {
      cfg->osdbrightness = STATE_ON;
    }
    else {
      if(strcmp(CFG_OSDBRIGHTNESS_OFF, arg) == 0) {
	cfg->osdbrightness = STATE_OFF;
      }
      else {
	fprintf(stderr, _("Illegal xosd brightness state: %s\n"), arg);
	_exit(1);
      }
    }
  } /* }}} */

  else if (strcmp(CFG_OSDVOLUME, key) == 0) { /* {{{ */
    int i = 0;
    while(arg[i] != '\0') {
      arg[i] = tolower(arg[i]);
      i++;
    }
    if(strcmp(CFG_OSDVOLUME_ON, arg) == 0) {
      cfg->osdvolume = STATE_ON;
    }
    else {
      if(strcmp(CFG_OSDVOLUME_OFF, arg) == 0) {
	cfg->osdvolume = STATE_OFF;
      }
      else {
	fprintf(stderr, _("Illegal xosd volume state: %s\n"), arg);
	_exit(1);
      }
    }
  } /* }}} */

  else if (strcmp(CFG_OSDPOWERMGT, key) == 0) { /* {{{ */
    int i = 0;
    while(arg[i] != '\0') {
      arg[i] = tolower(arg[i]);
      i++;
    }
    if(strcmp(CFG_OSDPOWERMGT_ON, arg) == 0) {
      cfg->osdpowermgt = STATE_ON;
    }
    else {
      if(strcmp(CFG_OSDPOWERMGT_OFF, arg) == 0) {
	cfg->osdpowermgt = STATE_OFF;
      }
      else {
	fprintf(stderr, _("Illegal xosd powermanagement state: %s\n"), arg);
	_exit(1);
      }
    }
  } /* }}} */
#endif /* HAVE_LIBXOSD */

  return;
} /* }}} */

/* find the nvram device */
void find_nvram(config *cfg) /* {{{ */
{
  int fdsc;

  /* Try default device */
  if((cfg->nvram=strdup(DEFAULT_NVRAMDEV)) == NULL) {
    fputs(_("Not enough memory"),stderr);
    _exit(1);
  }
  if((fdsc=open(cfg->nvram, O_RDONLY|O_NONBLOCK) == -1) && errno == ENOENT) {
    /* Try devfs device */
    if((cfg->nvram=strdup(DEFAULT_NVRAMDEV_DEVFS)) == NULL) {
      fputs(_("Not enough memory"),stderr);
      _exit(1);
    }
    fdsc = open(cfg->nvram, O_RDONLY|O_NONBLOCK);
  }

  if(fdsc != -1) {
    close(fdsc);
  }

  return;
} /* }}} */

/* print the usage to sdtout */
void print_usage(const char *prog) /* {{{ */
{
  printf("%s " VERSION " (C) 2002-2005 Markus Braun \n\n", prog);
  printf(_("Usage: %s options\n\n"), prog);
  printf(_("options:\n"));
  printf(_("   -h, --help           display this help\n"));
  printf(_("   -d, --daemon         start in daemon mode\n"));
  printf(_("   -c, --config=FILE    read FILE as additional configuration file\n"));
  printf(_("   -m, --mixer=STATE    use OSS software mixer [%s]\n"), DEFAULT_MIXER == STATE_ON ? CFG_MIXER_ON : CFG_MIXER_OFF);
#ifdef HAVE_LIBXOSD
  printf(_("   -o, --osd=STATE      show informations as on-screen display [%s]\n"), DEFAULT_OSD == STATE_ON ? CFG_OSD_ON : CFG_OSD_OFF);
#endif /* HAVE_LIBXOSD */
  printf(_("   -p, --poll=DELAY     set delay between polls in microseconds [%d]\n"), DEFAULT_POLLTIME);
  printf(_("   -t, --thinkpad=CMD   string with command and options that should be executed\n"));
  printf(_("                        when ThinkPad button is pressed [none]\n"));
  printf(_("   -H, --home=CMD       command and options for Home button [none]\n"));
  printf(_("   -S, --search=CMD     command and options for Search button [none]\n"));
  printf(_("   -M, --mail=CMD       command and options for Mail button [none]\n"));
  printf(_("   -W, --wireless=CMD   command and options for Wireless button [none]\n"));
  printf(_("   -C, --callback=CMD   string with command that should be executed for each\n"));
  printf(_("                        pressed button. It is called with pressed button as\n"));
  printf(_("                        first argument and new state as second [none]\n"));
  printf(_("   -A, --apm=STATE      poll /proc/apm for apm information [%s]\n"), DEFAULT_APM == STATE_ON ? CFG_APM_ON : CFG_APM_OFF);
  printf(_("   -P, --powermgt=STATE display of power management messages [%s]\n"), DEFAULT_POWERMGT == STATE_ON ? CFG_POWERMGT_ON : DEFAULT_POWERMGT == STATE_OFF ? CFG_POWERMGT_OFF : CFG_POWERMGT_AUTO);
  printf(_("   -x, --xevents=STATE  grab X11 events of special keys [%s]\n"), DEFAULT_XEVENTS == STATE_ON ? CFG_XEVENTS_ON : CFG_XEVENTS_OFF);
  printf(_("   -v, --verbose        print information about pressed keys\n"));
  return;
} /* }}} */

/* vim600:set fen:set fdm=marker:set fdl=0: */
