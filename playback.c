#include "songlist.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "pcm.h"

int start_playback(struct song *song)
{
    /* Open and mmap the specified file, then call decoder-specific
     * start_playback function 
     */

    char *data;
    int ret;
    int fd=open(song->filename,O_RDONLY);

    fprintf(stderr,"starting playback: \"%s\" (%d)\n",song->filename,fd);
    
    data = (char *) mmap(0, song->filesize_bytes, PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
	close(fd);
	return 0;
    }
    pcm_open_device(16,song->channels,song->samplerate);
    ret=song->decoder->start_play(song,data,song->filesize_bytes,0);
    munmap(data, song->filesize_bytes);
    close(fd);
    return ret;
}
    
