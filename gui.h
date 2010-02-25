
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
    int (*handler)(struct button * ,struct event *);
    char * label;
    void * user_data;
    void * style;
};

struct button *add_button(struct button b) ;
struct button *find_button(int tab,int x,int y) ;
void draw_button(struct button *b); /* example render function */

int set_tab(int new_tab);	/* new_tab==0 to query current tab */
void refresh_screen(void);


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

