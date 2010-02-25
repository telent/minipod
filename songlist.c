#include <dirent.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define IN_SONGLIST_C
#include "songlist.h"

extern struct decoder mp3_decoder,wma_decoder;

static int find_decoder(char *pathname) 
{
    char * suffix=strrchr(pathname,'.');
    /* this could use some work to be a bit more general.
     * Really we'd rather do magic number detection than depend
     * on file types
     */
    if(!suffix) return 0;
    if(!strcmp(suffix,".mp3")) return &mp3_decoder;
    if(!strcmp(suffix,".wma")) return &wma_decoder;
    return 0;
}
    

struct song *read_songs(char *dirname) 
{
    /* bleught.  string manipulation in C.  I hate this part */

    struct dirent *de;
    DIR *dir=opendir(dirname);
    int buf_size=0;
    int nsongs=0;
    char *buf;
    struct song *songs,*song;

    while((de=readdir(dir))) {
	int len=strlen(de->d_name)-4;
	if(find_decoder(de->d_name)) {
	    nsongs++;
	    buf_size+=strlen(de->d_name)+1;
	} else {
	    printf("skipping %s\n",de->d_name);
	}
    }
    buf=calloc(buf_size,1);
    songs=calloc(nsongs,sizeof (struct song));
    song=&songs[0];
    rewinddir(dir);
    struct decoder *d;
    while(nsongs && (de=readdir(dir))) {
	if(d=find_decoder(de->d_name)) {
	    nsongs--;
	    strcpy(buf,de->d_name);
	    song->filename=buf;
	    printf("--%s\n",buf);
	    buf+=strlen(buf)+1;
	    d->read_metadata(song);
	    song->decoder=d;
	    song++;
	}
    }
    song->filename=(char *)0;
    n_songs=song-songs;
    return songs;
}
