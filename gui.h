
struct event {
    int x;
    int y;
    int pressure;
    int keysym;
};

struct button_style {
    int background_color;
    int border_color;
    int label_color;
};

struct button {
    int tab;
    int x;
    int y;
    int w;
    int h;
    void (*render)(struct button *);
    /* update is called when the button is already on the screen 
     * but may want to change its contents.  e.g. clocks or
     * animations
     */
    void (*update)(struct button *);
    int (*handler)(struct button * ,struct event *);
    char * label;
    void * user_data;
    void * style;
};

int add_button(struct button b) ;
struct button * get_button_index(int ndx);
struct button *find_button(int tab,int x,int y) ;
void draw_button(struct button *b); /* example render function */

int set_tab(int new_tab);	/* new_tab==0 to query current tab */
void refresh_screen(void);
void update_screen(void);


#ifndef IN_GUI_C
extern
#endif
struct command {
    enum { NONE=0,SKIP,SEEK,PAUSE } tag;
    int value;
} pending_command;

int gui_init() ;
int gui_shutdown() ;
int input_poll(int wait);

