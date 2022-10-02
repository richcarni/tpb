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

#ifndef __CFG_H__
#define __CFG_H__

#if ENABLE_NLS
#include <libintl.h>
#define _(String) gettext (String)
#else /* ENABLE_NLS */
#define _(String) (String)
#endif /* ENABLE_NLS */

enum state {UNDEFINED = -1, STATE_OFF, STATE_ON, STATE_AUTO};
enum mode {MODE_COMMENT, MODE_INDENT, MODE_KEYWORD, MODE_SEPARATOR, MODE_ARGUMENT, MODE_END};

/* configuration defaults */
#define DEFAULT_DAEMON STATE_OFF
#define DEFAULT_APM STATE_OFF
#define DEFAULT_POWERMGT STATE_AUTO
#define DEFAULT_XEVENTS STATE_ON
#define DEFAULT_VERBOSE STATE_OFF
#define DEFAULT_POLLTIME 200000
#define DEFAULT_NVRAMDEV "/dev/nvram"
#define DEFAULT_NVRAMDEV_DEVFS "/dev/misc/nvram"
#define DEFAULT_MIXER STATE_OFF
#define DEFAULT_MIXERSTEPS 14
#define DEFAULT_MIXERDEV "/dev/mixer"
#ifdef HAVE_LIBXOSD
#define DEFAULT_OSD STATE_ON
#define DEFAULT_OSDZOOM UNDEFINED
#define DEFAULT_OSDTHINKLIGHT UNDEFINED
#define DEFAULT_OSDDISPLAY UNDEFINED
#define DEFAULT_OSDHVEXPANSION UNDEFINED
#define DEFAULT_OSDBRIGHTNESS UNDEFINED
#define DEFAULT_OSDVOLUME UNDEFINED
#define DEFAULT_OSDPOWERMGT UNDEFINED
#define DEFAULT_OSDLINES 2
#if HAVE_LIBXOSD_VERSION > 700
#define DEFAULT_OSDFONT osd_default_font
#else
#define DEFAULT_OSDFONT "10x20"
#endif /* HAVE_LIBXOSD_VERSION */
#define DEFAULT_OSDCOLOR "BLUE"
#define DEFAULT_OSDTIMEOUT 3
#define DEFAULT_OSDSHADOW 2
#define DEFAULT_OSDSHADOWCOLOR "BLACK"
#define DEFAULT_OSDOUTLINE 1
#define DEFAULT_OSDOUTLINECOLOR "BLACK"
#define DEFAULT_OSDVERTICAL 25
#define DEFAULT_OSDHORIZONTAL 25
#define DEFAULT_OSDPOS XOSD_bottom
#define DEFAULT_OSDALIGN XOSD_left
#endif /* HAVE_LIBXOSD */

/* configuration files */
#ifdef SYSCONFDIR
#define GLOBAL_CONFIG_FILE SYSCONFDIR "/tpbrc"
#else /* SYSCONFDIR */
#define GLOBAL_CONFIG_FILE "/etc/tpbrc"
#endif /* SYSCONFDIR */
#define PRIVATE_CONFIG_FILE ".tpbrc"

/* Keywords in configuration file */
#define CFG_APM "apm"
#define CFG_APM_ON "on"
#define CFG_APM_OFF "off"
#define CFG_POWERMGT "powermgt"
#define CFG_POWERMGT_ON "on"
#define CFG_POWERMGT_OFF "off"
#define CFG_POWERMGT_AUTO "auto"
#define CFG_XEVENTS "xevents"
#define CFG_XEVENTS_ON "on"
#define CFG_XEVENTS_OFF "off"
#define CFG_NVRAM "nvram"
#define CFG_POLLTIME "polltime"
#define CFG_THINKPAD "thinkpad"
#define CFG_HOME "home"
#define CFG_SEARCH "search"
#define CFG_MAIL "mail"
#define CFG_WIRELESS "wireless"
#define CFG_FAVORITES "favorites"
#define CFG_RELOAD "reload"
#define CFG_ABORT "abort"
#define CFG_BACKWARD "backward"
#define CFG_FORWARD "forward"
#define CFG_FN "fn"
#define CFG_CALLBACK "callback"
#define CFG_MIXER "mixer"
#define CFG_MIXER_ON "on"
#define CFG_MIXER_OFF "off"
#define CFG_MIXERSTEPS "mixersteps"
#define CFG_MIXERDEV "mixerdevice"
#ifdef HAVE_LIBXOSD
#define CFG_OSDFONT "osdfont"
#define CFG_OSDCOLOR "osdcolor"
#define CFG_OSDTIMEOUT "osdtimeout"
#define CFG_OSDOFFSET "osdoffset"
#define CFG_OSDSHADOW "osdshadow"
#if HAVE_LIBXOSD_VERSION >= 20200
#define CFG_OSDSHADOWCOLOR "osdshadowcolor"
#define CFG_OSDOUTLINE "osdoutline"
#define CFG_OSDOUTLINECOLOR "osdoutlinecolor"
#endif /* HAVE_LIBXOSD_VERSION */
#define CFG_OSDVERTICAL "osdvertical"
#define CFG_OSDHORIZONTAL "osdhorizontal"
#define CFG_OSDPOS "osdpos"
#define CFG_OSDPOS_TOP "top"
#define CFG_OSDPOS_MIDDLE "middle"
#define CFG_OSDPOS_BOTTOM "bottom"
#define CFG_OSDALIGN "osdalign"
#define CFG_OSDALIGN_LEFT "left"
#define CFG_OSDALIGN_CENTER "center"
#define CFG_OSDALIGN_RIGHT "right"
#define CFG_OSD "osd"
#define CFG_OSD_ON "on"
#define CFG_OSD_OFF "off"
#define CFG_OSDZOOM "osdzoom"
#define CFG_OSDZOOM_ON "on"
#define CFG_OSDZOOM_OFF "off"
#define CFG_OSDTHINKLIGHT "osdthinklight"
#define CFG_OSDTHINKLIGHT_ON "on"
#define CFG_OSDTHINKLIGHT_OFF "off"
#define CFG_OSDDISPLAY "osddisplay"
#define CFG_OSDDISPLAY_ON "on"
#define CFG_OSDDISPLAY_OFF "off"
#define CFG_OSDHVEXPANSION "osdhvexpansion"
#define CFG_OSDHVEXPANSION_ON "on"
#define CFG_OSDHVEXPANSION_OFF "off"
#define CFG_OSDBRIGHTNESS "osdbrightness"
#define CFG_OSDBRIGHTNESS_ON "on"
#define CFG_OSDBRIGHTNESS_OFF "off"
#define CFG_OSDVOLUME "osdvolume"
#define CFG_OSDVOLUME_ON "on"
#define CFG_OSDVOLUME_OFF "off"
#define CFG_OSDPOWERMGT "osdpowermgt"
#define CFG_OSDPOWERMGT_ON "on"
#define CFG_OSDPOWERMGT_OFF "off"
#endif /* HAVE_LIBXOSD */

#define BUFFER_SIZE 256

typedef struct {
  int daemon;
  int apm;
  int powermgt;
  int xevents;
  char *nvram;
  int polltime;
  char *tpb_cmd;
  char *home_cmd;
  char *search_cmd;
  char *mail_cmd;
  char *wireless_cmd;
  char *favorites_cmd;
  char *reload_cmd;
  char *abort_cmd;
  char *backward_cmd;
  char *forward_cmd;
  char *fn_cmd;
  char *callback;
  int mixer;
  int mixersteps;
  char *mixerdev;
  int verbose;
#ifdef HAVE_LIBXOSD
  int osd;
  int osdzoom;
  int osdthinklight;
  int osddisplay;
  int osdhvexpansion;
  int osdbrightness;
  int osdvolume;
  int osdpowermgt;
  char *osdfont;
  char *osdcolor;
  int osdtimeout;
  int osdshadow;
#if HAVE_LIBXOSD_VERSION >= 20200
  char *osdshadowcolor;
  int osdoutline;
  char *osdoutlinecolor;
#endif /* HAVE_LIBXOSD_VERSION */
  int osdvertical;
  int osdhorizontal;
  xosd_pos osdpos;
#if HAVE_LIBXOSD_VERSION >= 10000
  xosd_align osdalign;
#endif /* HAVE_LIBXOSD_VERSION */
#endif /* HAVE_LIBXOSD */
} config;

int parse_cfg_file(config *cfg, char *name);
void parse_option(config *cfg, int argc, char **argv);
void init_cfg(config *cfg);
void clear_cfg(config *cfg);
void free_cfg(config *cfg);
void override_cfg(config *master, config *slave);
void set_value(char *key, char *arg, config *cfg);
void print_usage(const char *prog);

#endif /* __CFG_H__ */

/* vim600:set fen:set fdm=marker:set fdl=0: */
