#include <ao/ao.h>

ao_device *pcm_out_device;
static int ao_default_driver;
int pcm_init(void)
{
    ao_initialize();
    ao_default_driver = ao_default_driver_id();
    return 0;
}

ao_device *pcm_open_device(int bits,int channels,int rate)	
{
    ao_sample_format format;

    if(pcm_out_device) ao_close(pcm_out_device);

    format.bits = bits;
    format.channels = channels;
    format.rate = rate;
    format.byte_format = AO_FMT_LITTLE;
    
    /* -- Open driver -- */
    pcm_out_device = ao_open_live(ao_default_driver, &format, NULL);
    if (pcm_out_device == NULL) {
	fprintf(stderr, "Error opening device.\n");
	return 0;
    }   
    return pcm_out_device;
}

static struct {
    unsigned char pcm_buffer[1024*16]; /* no idea how big this should be */
    unsigned char end[1];
} b;

int pcm_play_buf(char *buf,unsigned int length, unsigned int bytes_per_sample, 
		 unsigned int bit_depth)
{
    unsigned char *p=b.pcm_buffer;
    while(length-->0 && p<b.end) {
	unsigned int val=0;
	switch(bytes_per_sample) {
	case 4:	
	    val = (*(buf+2)<<16) + (*(buf+3)<<24);
	case 2:
	    val+=*buf  + (*(buf+1)<<8);
	    break;
	default:
	    return 0;
	}
	/* if bit depth is 32, shift right 16 */
	/* if bit depth is 30, shift right 14 */
	/* if bit depth is 29, >> 13 */
	
	buf+=bytes_per_sample;
	val = val >> (bit_depth -15); 
	*p++=(val & 0xff);
	*p++=((val & 0xff00)>>8);
    }
    int bytes=p-b.pcm_buffer;
    ao_play(pcm_out_device,b.pcm_buffer,bytes);
    return bytes/2;
}

void pcm_shutdown()
{
    if(pcm_out_device) ao_close(pcm_out_device);
    ao_shutdown();
}

