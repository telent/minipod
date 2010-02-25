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
struct song *songs;


