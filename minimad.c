/*
 * minimad.c (C) 2010 Daniel Barlow
 *
 * based in part on minimad.c, the example program for 
 * libmad - MPEG audio decoder library
 * Copyright (C) 2000-2004 Underbit Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "mad.h"

#include "pcm.h"


/* A generic pointer to this structure is passed to each of the
 * callback functions. Put here any data you need to access from
 * within the callbacks.
 */

struct buffer {
  unsigned char const *start;
  unsigned long length;
};

/* Original minimad.c said:
 * This is perhaps the simplest example use of the MAD high-level API.
 * Standard input is mapped into memory via mmap(), then the high-level API
 * is invoked with three callbacks: input, output, and error. The output
 * callback converts MAD's high-resolution PCM samples to 16 bits, then
 * writes them to standard output in little-endian, stereo-interleaved
 * format.
 */

/* our changes: we use libao for output not stdout.  We poll the screen
 * in the output() function
 */


static enum mad_flow input_fn(void *data, struct mad_stream *stream);
static enum mad_flow output_fn(void *data,struct mad_header const *header,
			    struct mad_pcm *pcm);
static enum mad_flow error_fn(void *data, struct mad_stream *stream,
			   struct mad_frame *frame);


/* 
 * Don't call this recursively
 */

int mad_start_playback(char * pathname)
{
    /* Open and mmap the specified file.  Instantiate a decoder object
     * and configures it with the input, output, and error callback
     * functions.  A single call to mad_decoder_run() continues until
     * a callback function returns MAD_FLOW_STOP (to stop decoding) or
     * MAD_FLOW_BREAK (to stop decoding and signal an error).
     */

    struct stat mstat;
    char *mp3_data;
    int ret;
    int fd=open(pathname,O_RDONLY);

    fprintf(stderr,"starting playback: \"%s\" (%d)\n",pathname,fd);
    if (stat(pathname, &mstat) == -1 || mstat.st_size == 0)
	return 2;
    
    mp3_data = (char *) mmap(0, mstat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (mp3_data == MAP_FAILED) {
	close(fd);
	return 3;
    }
    pcm_open_device(16,2,44100);
    
    struct buffer buffer;
    struct mad_decoder decoder;
    
    /* initialize our private message structure */
    
    buffer.start = mp3_data;
    buffer.length = mstat.st_size;
    
    /* configure input, output, and error functions */
    mad_decoder_init(&decoder, &buffer,
		     input_fn, 0 /* header */, 0 /* filter */, output_fn,
		     error_fn, 0 /* message */);
    
    /* start decoding */
    ret = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
    
    /* cleanup */
    mad_decoder_finish(&decoder);
    munmap(mp3_data, mstat.st_size);
    close(fd);
    return 0;
}




/*
 * This is the input callback. The purpose of this callback is to (re)fill
 * the stream buffer which is to be decoded. In this example, an entire file
 * has been mapped into memory, so we just call mad_stream_buffer() with the
 * address and length of the mapping. When this callback is called a second
 * time, we are finished decoding.
 */

static
enum mad_flow input_fn(void *data,
		       struct mad_stream *stream)
{
  struct buffer *buffer = data;

  if (!buffer->length)
    return MAD_FLOW_STOP;

  mad_stream_buffer(stream, buffer->start, buffer->length);

  buffer->length = 0;

  return MAD_FLOW_CONTINUE;
}

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */

static inline
signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

/*
 * This is the output callback function. It is called after each frame of
 * MPEG audio data has been completely decoded. The purpose of this callback
 * is to output (or play) the decoded PCM audio.
 */

static char pcm_buffer[1024*16]; /* no idea how big this should be */
static char *ob_watermark;

static
enum mad_flow output_fn(void *data,
			struct mad_header const *header,
			struct mad_pcm *pcm)
{
  unsigned int nchannels, nsamples;
  mad_fixed_t const *left_ch, *right_ch;

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  nsamples  = pcm->length;
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];

  char *ob=pcm_buffer;

  while (nsamples--) {
    signed int sample;

    /* output sample(s) in 16-bit signed little-endian PCM */

    sample = scale(*left_ch++);
    
    *ob++=((sample >> 0) & 0xff);
    *ob++=((sample >> 8) & 0xff);

    if (nchannels == 2) {
      sample = scale(*right_ch++);
      *ob++=((sample >> 0) & 0xff);
      *ob++=((sample >> 8) & 0xff);
    }
  }
  if(ob>ob_watermark ) ob_watermark=ob;
  ao_play(pcm_out_device,pcm_buffer,ob-pcm_buffer);
  
  return input_poll(0) ?  MAD_FLOW_CONTINUE : MAD_FLOW_STOP;
}

/*
 * This is the error callback function. It is called whenever a decoding
 * error occurs. The error is indicated by stream->error; the list of
 * possible MAD_ERROR_* errors can be found in the mad.h (or stream.h)
 * header file.
 */

static
enum mad_flow error_fn(void *data,
		       struct mad_stream *stream,
		       struct mad_frame *frame)
{
  struct buffer *buffer = data;

  fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",
	  stream->error, mad_stream_errorstr(stream),
	  stream->this_frame - buffer->start);

  /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

  return MAD_FLOW_CONTINUE;
}

