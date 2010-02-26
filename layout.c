#include <stdlib.h>


#include "gui.h"
#include "songlist.h"
#include "fbutils.h"

#define IN_LAYOUT_C
#include "layout.h"

static struct button_style fg_tab_style ={
 background_color: 4,
 border_color: 0,
 label_color: 1,
};
static struct button_style bg_tab_style ={
 background_color: 3,
 border_color: 0,
 label_color: 1,
};
static struct button_style track_list_style ={
 background_color: 0,
 border_color: 0,
 label_color: 0,
};

static int switch_tab(struct button *b,struct event *ev)
{
    set_tab((int)b->user_data);
    return 0;
}

static void render_tracks(struct button *b) 
{
    draw_button(b); 
    int i;
    for(i=0;songs[i].filename;i++) {
	int y=i*30;
	if(y>b->h) return;
	put_string(b->x+20,b->y+y,songs[i].filename,1);
    }
}

static void update_waveform(struct button *b) 
{
    int x;
    signed short* data=get_pcm_buffer();
    if(!data) return;
    if(*data>0) {
	while(*data>0) data++;
    } else {
	while(*data<0) data++;
    }
    int midline= b->y+(b->h)/2;

    int x2=b->x+b->w;
    for(x=b->x; x<x2; x++,data+=16) {
	signed short val=(*data>>9);
	line(x, b->y, x, b->y+b->h, 0);
	line(x, midline, x, midline + val, 5);
	pixel(x,midline+val/4,1);
    }
}

static int switch_track(struct button *b,struct event *ev)
{
    int y_off=ev->y - b->y ;
    int i=y_off/30;
    if(i<n_songs) {
	pending_command.tag=SKIP; pending_command.value=i;
	return 1;
    } else 
	return 0;
}

void layouts_init() 
{
    /* tab 1  */
    add_button((struct button)
	       {tab: 1, x: 5,y: 5, w: 70, h: 30,
		       render: draw_button, style: &fg_tab_style,
		       label: "Tracks"
		       });
    add_button((struct button)
	       {tab: 1, x: 85,y: 5, w: 70, h: 30,
		       render: draw_button,
		       handler: switch_tab, user_data: (void *)2,
		       style: &bg_tab_style, label: "Playing"
		       });
    add_button((struct button)
	       {tab: 1, x: 165,y: 5, w: 70, h: 30,
		       render: draw_button,
		       handler: switch_tab, user_data: (void *)3,
		       style: &bg_tab_style, label: "Volume"
		       });

    add_button((struct button)
	       {tab: 1, x: 5,y: 45, w: 225, h: 250,
		       render: render_tracks,
		       handler: switch_track,
		       style: &track_list_style,
		       });

    /* tab 2 */

    add_button((struct button)
	       {tab: 2, x: 5,y: 5, w: 70, h: 30,
		       render: draw_button,
		       handler: switch_tab, user_data: (void *)1,
		       style: &bg_tab_style, label: "Tracks"
		       });
    add_button((struct button)
	       {tab: 2, x: 85,y: 5, w: 70, h: 30,
		       render: draw_button,
		       style: &fg_tab_style, label: "Playing"
		       });
    add_button((struct button)
	       {tab: 2, x: 165,y: 5, w: 70, h: 30,
		       render: draw_button,
		       handler: switch_tab, user_data: (void *)3,
		       style: &bg_tab_style, label: "Volume"
		       });

    add_button((struct button)
	       {tab: 2, x: 5,y: 45, w: 225, h: 200,
		       update: update_waveform, user_data: 0,
		       render: draw_button,
		       style: &track_list_style,
		       });

    /* tab 3 */
    add_button((struct button)
	       {tab: 3, x: 5,y: 5, w: 70, h: 30,
		       render: draw_button,
		       handler: switch_tab, user_data: (void *)1,
		       style: &bg_tab_style, label: "Tracks"
		       });
    add_button((struct button)
	       {tab: 3, x: 85,y: 5, w: 70, h: 30,
		       render: draw_button,
		       handler: switch_tab, user_data: (void *)2,
		       style: &bg_tab_style, label: "Playing"
		       });
    add_button((struct button)
	       {tab: 3, x: 165,y: 5, w: 70, h: 30,
		       render: draw_button,
		       style: &fg_tab_style, label: "Volume"
		       });
 }

