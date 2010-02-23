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

static int palette [] =
{
	0x000000, 0xffe080, 0xffffff, 0xe0c0a0, 0x304050, 0x80b8c0
};
#define NR_COLORS (sizeof (palette) / sizeof (palette [0]))

struct ts_button {
	int x, y, w, h;
	char *text;
	int flags;
#define BUTTON_ACTIVE 0x00000001
};

/* [inactive] border fill text [active] border fill text */
static int button_palette [6] =
{
	1, 4, 2,
	1, 5, 0
};

#define NR_BUTTONS 2
static struct ts_button buttons [NR_BUTTONS];

static void sig(int sig)
{
	close_framebuffer();
	fflush(stderr);
	printf("signal %d caught\n", sig);
	fflush(stdout);
	exit(1);
}

static void button_draw (struct ts_button *button)
{
	int s = (button->flags & BUTTON_ACTIVE) ? 3 : 0;
	rect (button->x, button->y, button->x + button->w - 1,
	      button->y + button->h - 1, button_palette [s]);
	fillrect (button->x + 1, button->y + 1,
		  button->x + button->w - 2,
		  button->y + button->h - 2, button_palette [s + 1]);
	put_string_center (button->x + button->w / 2,
			   button->y + button->h / 2,
			   button->text, button_palette [s + 2]);
}

static int button_handle (struct ts_button *button, struct ts_sample *samp)
{
	int inside = (samp->x >= button->x) && (samp->y >= button->y) &&
		(samp->x < button->x + button->w) &&
		(samp->y < button->y + button->h);

	if (samp->pressure > 0) {
		if (inside) {
			if (!(button->flags & BUTTON_ACTIVE)) {
				button->flags |= BUTTON_ACTIVE;
				button_draw (button);
			}
		} else if (button->flags & BUTTON_ACTIVE) {
			button->flags &= ~BUTTON_ACTIVE;
			button_draw (button);
		}
	} else if (button->flags & BUTTON_ACTIVE) {
		button->flags &= ~BUTTON_ACTIVE;
		button_draw (button);
                return 1;
	}

        return 0;
}

static struct song *songs;


static void refresh_screen ()
{
    int i;
    
    fillrect (0, 0, xres - 1, yres - 1, 0);
    
    for(i=0;songs[i].filename;i++) {
	int y=20+i*30;
	songs[i].y=i;
	printf("%s\n",songs[i].filename);
	put_string(20,y,songs[i].filename,1);
	if(y>yres) return;
    }
}

struct tsdev *ts;
int mouse_x, mouse_y;

int input_poll(int wait)
{
    struct ts_sample samp;
    int ret;
    fd_set fdset;
    struct timeval timeout;
    timeout.tv_sec=0; timeout.tv_usec=0;

    FD_ZERO(&fdset);
    FD_SET(ts_fd(ts), &fdset);
    
    if(pending_command.tag) return 0;
    if(!wait) {
	/* during playback, we don't want to hang waiting for input */
	if(select(ts_fd(ts)+1, &fdset,0,0, &timeout)==0)
	    return 1;
    }

    ret = ts_read(ts, &samp, 1);
    if (ret < 0) {
	perror("ts_read");
	close_framebuffer();
	pcm_shutdown();
	exit(1);
    }
    
    if (ret != 1) return 1;
    
    printf("%ld.%06ld: %6d %6d %6d\n", samp.tv.tv_sec, samp.tv.tv_usec,
	   samp.x, samp.y, samp.pressure);
    if(samp.pressure==0) {
	pending_command.tag=SKIP;
	pending_command.value=(samp.y-20)/30;
    }
    return 1;
}

int main()
{
	unsigned int i;

	char *tsdevice=NULL;
	char * directory="/mnt/"; /* trailing / is important! */

	chdir(directory);
	songs=read_songs(directory);

	signal(SIGSEGV, sig);
	signal(SIGINT, sig);
	signal(SIGTERM, sig);

	if ((tsdevice = getenv("TSLIB_TSDEVICE")) == NULL) {
		tsdevice = strdup ("/dev/input/event1");
        }

	ts = ts_open (tsdevice, 0);

	if (!ts) {
		perror (tsdevice);
		exit(1);
	}

	if (ts_config(ts)) {
		perror("ts_config");
		exit(1);
	}

	if (open_framebuffer()) {
		close_framebuffer();
		exit(1);
	}

	if(pcm_init()) {
	    close_framebuffer();
	    exit(1);
	}
	
	rb_wma_start_playback("/dan/minipod/foo.wma");

	mouse_x = xres/2;
	mouse_y = yres/2;

	for (i = 0; i < NR_COLORS; i++)
		setcolor (i, palette [i]);

	/* Initialize buttons */
	memset (&buttons, 0, sizeof (buttons));
	buttons [0].w = buttons [1].w = xres / 4;
	buttons [0].h = buttons [1].h = 20;
	buttons [0].x = xres / 4 - buttons [0].w / 2;
	buttons [1].x = (3 * xres) / 4 - buttons [0].w / 2;
	buttons [0].y = buttons [1].y = 10;
	buttons [0].text = "Drag";
	buttons [1].text = "Draw";

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

