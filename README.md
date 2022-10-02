tpb - program to use the IBM ThinkPad(tm) special keys

All the information used in this program was gathered by observing the device
/dev/nvram while pressing the buttons. This was done with the command

'watch --differences=cumulative -n 0 xxd /dev/nvram'

The meaning of the bits in nvram is documented in doc/nvram.txt

This program was originally developed on a Thinkpad T21 (2647-4AG).
It is known to work on 

	- A 30
	- A 31
	- R 30 (no hardware mixer)
	- R 31 (no hardware mixer)
	- R 32 (no hardware mixer)
	- R 40
	- S 30
	- T 20
	- T 23
	- T 30 (no hardware switching lcd/crt/both)
	- T 40
	- X 20
	- X 24
	- X 30
	- X 31
	- X 40

Use it at your own risk!!!

Installation:

- enable nvram in your kernel (section character devices), either as module
  or compiled into the kernel

- if you use nvram as module, load it

- make sure that the device /dev/nvram exists, else create it with
  'mknod /dev/nvram c 10 144'

- if you want to use on-screen display install xosd
  (http://www.ignavus.net/software.html)

- if you use the cvs version run "make -f Makefile.dist all"

- run the usual "./configure && make && make install"

- modify the template configuration file /usr/local/etc/tpbrc to change defaults

- run it with something like
  'tpb --osd=on --verbose --thinkpad="/usr/bin/X11/xterm -T ntpctl -e ntpctl"'

  NOTE: With tpb 0.6.2 or later you need to specify the full path for
        applications!

- have a look at the documentation in the man page, the template configuration
  file tpbrc and the output of 'tpb -h'
