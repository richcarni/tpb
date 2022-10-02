#!/bin/sh
# This script is a skeleton for callback script to use with tpb.
# tpb can call a program on each button press and state change. It passes an
# identifier as first argument and the new state as second argument to the
# callback. So you can do fancy things :)
# Supported identifiers and states are:
#
# IDENTIFIER        STATES/VALUE
# thinkpad          pressed
# home              pressed
# search            pressed
# mail              pressed
# favorites         pressed
# reload            pressed
# abort             pressed
# backward          pressed
# forward           pressed
# wireless          pressed
# fn                pressed
# zoom              on, off
# thinklight        on, off
# display           lcd, crt, both
# expand            on, off
# brightness        PERCENT
# volume            PERCENT
# mute              on, off
# ac_power          connected, disconnected
# powermgt_ac       high, auto, manual
# powermgt_battery  high, auto, manual
case $1 in
  (thinkpad)
		echo "CALLBACK: $0 $1 $2 (should be thinkpad pressed)"
		;;
  (home)
		echo "CALLBACK: $0 $1 $2 (should be home pressed)"
		;;
  (search)
		echo "CALLBACK: $0 $1 $2 (should be search pressed)"
		;;
  (mail)
		echo "CALLBACK: $0 $1 $2 (should be mail pressed)"
		;;
  (favorites)
		echo "CALLBACK: $0 $1 $2 (should be favorites pressed)"
		;;
  (reload)
		echo "CALLBACK: $0 $1 $2 (should be reload pressed)"
		;;
  (abort)
		echo "CALLBACK: $0 $1 $2 (should be abort pressed)"
		;;
  (backward)
		echo "CALLBACK: $0 $1 $2 (should be backward pressed)"
		;;
  (forward)
		echo "CALLBACK: $0 $1 $2 (should be forward pressed)"
		;;
  (wireless)
		echo "CALLBACK: $0 $1 $2 (should be wireless pressed)"
		;;
  (fn)
		echo "CALLBACK: $0 $1 $2 (should be fn pressed)"
		;;
  (zoom)
		case $2 in
			(on)
				echo "CALLBACK: $0 $1 $2 (should be zoom on)"
				;;
			(off)
				echo "CALLBACK: $0 $1 $2 (should be zoom off)"
				;;
		esac
		;;
  (thinklight)
		case $2 in
			(on)
				echo "CALLBACK: $0 $1 $2 (should be thinklight on)"
				;;
			(off)
				echo "CALLBACK: $0 $1 $2 (should be thinklight off)"
				;;
		esac
		;;
  (display)
		case $2 in
			(lcd)
				echo "CALLBACK: $0 $1 $2 (should be display lcd)"
				;;
			(crt)
				echo "CALLBACK: $0 $1 $2 (should be display crt)"
				;;
			(both)
				echo "CALLBACK: $0 $1 $2 (should be display both)"
				;;
		esac
		;;
  (expand)
		case $2 in
			(on)
				echo "CALLBACK: $0 $1 $2 (should be expand on)"
				;;
			(off)
				echo "CALLBACK: $0 $1 $2 (should be expand off)"
				;;
		esac
		;;
  (brightness)
		echo "CALLBACK: $0 $1 $2 (should be brightness PERCENT)"
		;;
  (volume)
		echo "CALLBACK: $0 $1 $2 (should be volume PERCENT)"
		;;
  (mute)
		case $2 in
			(on)
				echo "CALLBACK: $0 $1 $2 (should be mute on)"
				;;
			(off)
				echo "CALLBACK: $0 $1 $2 (should be mute off)"
				;;
		esac
		;;
  (ac_power)
		case $2 in
			(connected)
				echo "CALLBACK: $0 $1 $2 (should be ac_power connected)"
				;;
			(disconnected)
				echo "CALLBACK: $0 $1 $2 (should be ac_power disconnected)"
				;;
		esac
		;;
  (powermgt_ac)
		case $2 in
			(high)
				echo "CALLBACK: $0 $1 $2 (should be powermgt_ac high)"
				;;
			(auto)
				echo "CALLBACK: $0 $1 $2 (should be powermgt_ac auto)"
				;;
			(manual)
				echo "CALLBACK: $0 $1 $2 (should be powermgt_ac manual)"
				;;
		esac
		;;
  (powermgt_battery)
		case $2 in
			(high)
				echo "CALLBACK: $0 $1 $2 (should be powermgt_battery high)"
				;;
			(auto)
				echo "CALLBACK: $0 $1 $2 (should be powermgt_battery auto)"
				;;
			(manual)
				echo "CALLBACK: $0 $1 $2 (should be powermgt_battery manual)"
				;;
		esac
		;;
	(*)
		echo "CALLBACK: $0 $1 $2 (TYPE UNKNOWN)"
		;;
esac
