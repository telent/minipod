/*
 * minipod.c 
 *
 *  Was tslib/src/ts_test.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the GPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: ts_test.c,v 1.6 2004/10/19 22:01:27 dlowder Exp $
 *
 * Basic test program for touchscreen library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <tslib.h>
#include <unistd.h>		/* chdir */


#include "fbutils.h"
#include "songlist.h"
#include "libwma/codecs.h"
#include "pcm.h"
#include "gui.h"

void closedown(int sig)
{
	close_framebuffer();
	pcm_shutdown();
	fflush(stderr);
	if(sig>0) printf("signal %d caught\n", sig);
	fflush(stdout);
	exit(1);
}

static int palette [] =
{
	0x000000, 0xffe080, 0xffffff, 0xe0c0a0, 0x304050, 0x80b8c0
};
#define NR_COLORS (sizeof (palette) / sizeof (palette [0]))

int mouse_x, mouse_y;


int main(int argc,char *argv[])
{
	unsigned int i;

	char * directory="/mnt/"; /* trailing / is important! */
	if(argc==1) directory=argv[1];
	chdir(directory);
	songs=read_songs(directory);

	signal(SIGSEGV, closedown);
	signal(SIGINT, closedown);
	signal(SIGTERM, closedown);

	if(gui_init()) {
	    exit(1);
	}
	if(pcm_init()) {
	    gui_shutdown();
	    exit(1);
	}
	
	/*	rb_wma_start_playback("/dan/minipod/foo.wma"); */

	/*	mouse_x = xres/2;
		mouse_y = yres/2; */

	for (i = 0; i < NR_COLORS; i++)
		setcolor (i, palette [i]);

	/* Initialize buttons */
	layouts_init();
	refresh_screen ();

	pending_command.tag=SKIP; pending_command.value=0;
	int current_track=0;
	while(1) {
	    int v=pending_command.value;
	    int tag=pending_command.tag;
	    char *p;
	    fprintf(stderr,"command loop: %d, %d\n",pending_command.tag,v);
	    pending_command.tag=NONE;
	    switch(tag) {
	    case NONE:
		/* end of track, no other operation requested */
		p=songs[++current_track].filename;
		if(p) mad_start_playback(p);
		else input_poll(1);
		break;
	    case SKIP:
		current_track=v;
		mad_start_playback(songs[current_track].filename);
		break;
	    default:
		fprintf(stderr," command %d unimplemented\n",tag);
		break;
	    }
	}

	close_framebuffer();
	pcm_shutdown();
}

