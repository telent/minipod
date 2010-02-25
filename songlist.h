

struct song {
    char * filename;
    char * artist;
    char * album;
    char * title;
    int tracknumber;

    int channels; int samplerate;
    unsigned int length_ms;
    unsigned int filesize_bytes;

    struct decoder *decoder;
    /* decoders can malloc a block of memory for unparsed metadata or
     * other private stuff and stash a pointer to it here.  This may
     * be freed at any time minipod thinks the decode should have finished.
     */
    void * format_data;		
};
struct decoder {
    void * (*read_metadata)(struct song *song);
    void * (*start_play)(struct song *song, void *buf,int bytes, int offset_ms);
    void * (*stop_play)();
};

struct song *read_songs(char *dirname) ;

#ifndef IN_SONGLIST_C
extern
#endif

struct song *songs;
#ifndef IN_SONGLIST_C
extern
#endif
int n_songs;

