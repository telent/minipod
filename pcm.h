
#include <ao/ao.h>

#ifndef IN_PCM_C
extern ao_device *pcm_out_device;
#endif
int pcm_init(void);

ao_device *pcm_open_device(int bits,int channels,int rate);
int pcm_play_buf(char *buf,unsigned int length, unsigned int bytes_per_sample, 
		 unsigned int bit_depth);
void pcm_shutdown(void);
