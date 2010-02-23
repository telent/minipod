#include <dirent.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define IN_SONGLIST_C
#include "songlist.h"


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
	if(!strcmp(de->d_name+len,".mp3")) {
	    nsongs++;
	    buf_size+=strlen(de->d_name)+1;
	} else {
	    printf("skipping %s %s\n",de->d_name,de->d_name+len);
	}
    }
    buf=calloc(buf_size,1);
    songs=calloc(nsongs,sizeof (struct song));
    song=&songs[0];
    rewinddir(dir);
    while(nsongs-- && (de=readdir(dir))) {
	int len=strlen(de->d_name)-4;
	if(!strcmp(de->d_name+len,".mp3")) {
	    song->filename=buf;
	    strcpy(buf,de->d_name);
	    printf("--%s\n",buf);
	    buf+=strlen(buf)+1;
	    song++;
	}
    }
    song->filename=(char *)0;
    return songs;
}
