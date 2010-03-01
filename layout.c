#include <stdlib.h>


#include "gui.h"
#include "songlist.h"
#include "fbutils.h"
#include "mixer.h"

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
static struct button_style slider_style ={
 background_color: 0,
 border_color: 1,
 label_color: 1,
};
static struct button_style divider_style ={
 background_color: 0, border_color: 4, label_color: 1,
};
static struct button_style unstyled ={
 background_color: -1,
 border_color: -1,
 label_color: 1,
};

static int switch_tab(struct button *b,struct event *ev)
{
    set_tab((int)b->user_data);
    return 0;
}

extern int current_track;
static void render_tracks(struct button *b) 
{
    draw_button(b); 
    int i;
    for(i=0;songs[i].filename;i++) {
	int y=i*30;
	if(y>b->h) return;
	if(i==current_track) {
	    fillrect(b->x+2,b->y+y+2,b->x+b->w-2,b->y+y+30-2,5);
	}
	put_string(b->x+20,b->y+y +8 ,songs[i].filename,1);
    }
}

static void render_volume_slider(struct button *b) 
{
    struct mixer_control *c = (struct mixer_control *)b->user_data;
    long v=mixer_get_value(c);

    draw_button(b);
    fillrect(b->x+2, b->y+b->h-2, 
	     b->x+b->w-2, 
	     b->y + b->h -2 - (b->h* v)/(c->max-c->min),4);
}

static void set_volume_slider(struct button *b,struct event *ev) 
{
    struct mixer_control *c = (struct mixer_control *)b->user_data;
    long off=ev->y-b->y;
    long range=c->max-c->min;

    long oldvol=mixer_get_value(c);
    long newvol=range- range*off/b->h;
    long set;
    printf("oldvol %d, newvol %d\n",oldvol,newvol);
    /* don't allow a misplaced key to send the volume to max 
     * in one step */
    if(newvol>oldvol) {
	set=oldvol+range/10;
	if(set==oldvol) set++;
	if(set>newvol) set=newvol;
    } else 
	set=newvol;
    mixer_set_value(c,set);
    render_volume_slider(b);
    return 0;
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
    if((i!=current_track) && (i<n_songs)) {
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

    /* tab3: mixer sliders */

    add_button((struct button)
	       {tab: 3, x: 10,y: 40, w: 40, h: 240,
		       render: render_volume_slider,
		       handler: set_volume_slider,
		       user_data: mixer_get_control(0),
		       style: &slider_style,
		       });
    add_button((struct button)
	       {tab: 3, x: 10+50,y: 40, w: 40, h: 240,
		       render: render_volume_slider,
		       handler: set_volume_slider,
		       user_data: mixer_get_control(1),
		       style: &slider_style,
		       });
    add_button((struct button)
	       {tab: 3, x: 10+50+50,y: 40, w: 40, h: 240,
		       render: render_volume_slider,
		       handler: set_volume_slider,
		       user_data: mixer_get_control(2),
		       style: &slider_style,
		       });

    /* tab3: +6dB button */

    add_button((struct button)
	       {tab: 3, x: 10+50+50+50+10,y: 240, w: 40, h: 40,
		       render: draw_button,
		       style: &slider_style,
		       });

    /* tab3: boost */

    add_button((struct button)	/* divider */
	       {tab: 3, x: 10+50+50+50+5, y: 40, w: 50, h: 190,
		       render: draw_button,
		       style: &divider_style,
		       });
    add_button((struct button)	/* label */
	       {tab: 3, x: 10+50+50+50+9, y: 45, w: 35, h: 20,
		       render: draw_button,
		       style: &unstyled, label: " Boost "
		       });
    add_button((struct button)
	       {tab: 3, x: 10+50+50+50+10,y: 180, w: 40, h: 40,
		       render: draw_button,
		       style: &slider_style, label: "Off"
		       });
    add_button((struct button)
	       {tab: 3, x: 10+50+50+50+10,y: 130, w: 40, h: 40,
		       render: draw_button,
		       style: &slider_style, label: "Min"
		       });
    add_button((struct button)
	       {tab: 3, x: 10+50+50+50+10,y: 80, w: 40, h: 40,
		       render: draw_button,
		       style: &slider_style, label: "Max"
		       });


    

    /* tab3 : labels */
    add_button((struct button)
	       {tab: 3, x: 10, y: 280, w: 40, h: 20,
		       render: draw_button,
		       style: &unstyled, label: "Volume"
		       });
    add_button((struct button)
	       {tab: 3, x: 10+50, y: 280, w: 40, h: 20,
		       render: draw_button,
		       style: &unstyled, label: "Bass"
		       });
    add_button((struct button)
	       {tab: 3, x: 10+50+50, y: 280, w: 40, h: 20,
		       render: draw_button,
		       style: &unstyled, label: "Treble"
		       });
    add_button((struct button)
	       {tab: 3, x: 10+50+50+50+10, y: 280, w: 40, h: 20,
		       render: draw_button,
		       style: &unstyled, label: "+6dB"
		       });
     
}

