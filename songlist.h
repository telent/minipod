struct song {
    char * filename;
    char * artist;
    char * album;
    char * title;
    int tracknumber;
    int y;
};

struct song *read_songs(char *dirname) ;

#ifndef IN_SONGLIST_C
extern
#endif
struct command {
    enum { NONE=0,SKIP,SEEK,PAUSE } tag;
    int value;
} pending_command;
