#define IN_GUI_C
#include "gui.h"
#include "fbutils.h"
#include  <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <tslib.h>

struct button *buttons;
static int buttons_length=0;
static int buttons_next=0;
static struct tsdev *ts; 

int gui_init() {
    char *tsdevice=NULL;
    if ((tsdevice = getenv("TSLIB_TSDEVICE")) == NULL) {
	tsdevice = strdup ("/dev/input/event1");
    }
    
    ts = ts_open (tsdevice, 0);
    
    if (!ts) {
	perror (tsdevice);
	return 1;
    }
    
    if (ts_config(ts)) {
	perror("ts_config");
	return(1);
    }
    
    if (open_framebuffer()) {
	close_framebuffer();
	return(1);
    }
    return 0;
}
int gui_shutdown() 
{
    close_framebuffer();
    return 0;
}


struct button *add_button(struct button b) 
{
    const int inc=10;
    if(buttons==0) {
	buttons_length=inc;
	buttons=calloc(buttons_length,sizeof (struct button));
    }
    if(buttons_next==buttons_length) {
	buttons_length+=inc;
	buttons=realloc(buttons,buttons_length);
    }
    return (struct button*) (memcpy(&buttons[buttons_next++],&b,sizeof (struct button)));
}

/* return smallest button on tab surrounding x,y */
struct button *find_button(int tab,int x,int y) 
{
    /* if we had lots of buttons we should approach this a bit more
     * seriously: use an r-tree or similar.  But let's see if it
     * matters first
     */
    int best_el=-1;
    int best_area=INT_MAX;
    int i;
    for(i=0;i< buttons_next;i++) {
	struct button *b=&buttons[i];
	if((b->tab==tab) &&
	   (b->x <= x) && (b->y <= y) &&
	   ((b->x+b->w) >= x) && ((b->y+b->h) >= y)) {
	    int a = b->w*b->h;
	    if(a< best_area) {
		best_el=i; best_area=a;
	    }
	}
    }
    return (best_el>-1) ? &buttons[best_el] : 0;
}

static int current_tab=1;
int set_tab(int new_tab) 
{
    if(new_tab) {
	current_tab=new_tab;
	refresh_screen();
    }
    return current_tab;
}



void refresh_screen() 
{
    int i;
    fillrect(0,0,239,319,0);
    for(i=0;i<buttons_next;i++) {
	struct button *b=&buttons[i];
	if((current_tab==b->tab) && b->render) {
	    (b->render)((void *)b);
	}
    }
}
    
int input_poll(int wait)
{
    /* return non-0 from this to interrupt playback */
    struct ts_sample samp;
    int ret;
    fd_set fdset;
    struct timeval timeout;
    timeout.tv_sec=0; timeout.tv_usec=0;

    if(pending_command.tag) return 1;
    if(!wait) {
	/* during playback, we don't want to hang waiting for input */
	FD_ZERO(&fdset);
	FD_SET(ts_fd(ts), &fdset);
	if(select(ts_fd(ts)+1, &fdset,0,0, &timeout)==0)
	    return 0;
    }
    
    do{
	ret = ts_read(ts, &samp, 1);
	if (ret < 0) {
	    perror("ts_read");
	    closedown(0);
	}
	
	if (ret != 1) return 0;
#if 1
	printf("%ld.%06ld: %6d %6d %6d\n", samp.tv.tv_sec, samp.tv.tv_usec,
	       samp.x, samp.y, samp.pressure);
#endif
	if(samp.pressure==0) {
	    struct button *b=find_button(current_tab,samp.x,samp.y);
	    if(b) {
		printf("button %s\n", b->label);
		if(b->handler) {
		    struct event e={samp.x,samp.y,samp.pressure,-1};
		    (b->handler)((void *)b,&e);
		}
	    }
	    refresh_screen();
	}
    } while(wait && (pending_command.tag==NONE));
    return 1;
}

/* global scope: this is used as a callback when buttons are 
 * created */
void draw_button(struct button *b) 
{
    struct button_style *s=b->style;
    if(s->background_color>-1) {
	fillrect(b->x, b->y, b->x+b->w, b->y+b->h, s->background_color);
    }
    /* border overlaps the background if both are present */
    if(s->border_color>-1) {
	line(b->x, b->y, b->x+b->w, b->y, s->border_color);
	line(b->x+b->w, b->y, b->x+b->w, b->y+b->h, s->border_color);
	line(b->x+b->w, b->y+b->h, b->x, b->y+b->h, s->border_color);
	line(b->x, b->y+b->h, b->x, b->y, s->border_color);
    };
    if(b->label) {
	put_string_center (b->x + b->w / 2,
			   b->y + b->h / 2,
			   b->label, s->label_color);
    }
}

#if 0
int main() 
{
    add_button((struct button){tab: 1, x: 10,y: 10, w: 200, h: 200, user_data: "foo"});
    add_button((struct button){tab: 1, x: 10,y: 10, w: 20, h: 20, user_data: "foo1"});
    add_button((struct button){tab: 1, x: 100, y: 10, w: 20, h: 100, user_data: "bar"});
    struct button *b;
    b=find_button(1,20,15);
    if(b)
	printf("%s\n",(char *)b->user_data);
    else 
	printf("not found\n");

}
#endif
