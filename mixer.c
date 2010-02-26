
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <alsa/asoundlib.h>
#include <sys/poll.h>


static snd_mixer_t *mixer_handle = NULL;

enum mixer_controls { MASTER=0,BASS,TREBLE,NCONTROLS };
struct mixer_control {
    long min,max;
    char *name;
    snd_mixer_elem_t *elem;
} mixer_controls[]={
    {0,0,"Master"},
    {0,0,"Bass"},
    {0,0,"Treble"},
    {0,0,0}
};

static int error(char *fmt,...){
    printf("burp\n");
    exit(1);
}


int mixer_init_control(struct mixer_control * c)
{
    snd_mixer_selem_id_t *sid;

    if(c->elem == 0) {
	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_name(sid, c->name);
	c->elem = snd_mixer_find_selem(mixer_handle, sid);
    }
    if (!c->elem) return 0;

    if (snd_mixer_selem_has_playback_volume(c->elem)) {
	snd_mixer_selem_get_playback_volume_range(c->elem,&(c->min),&(c->max));
	snd_mixer_selem_set_playback_volume(c->elem, SND_MIXER_SCHN_MONO,
					    c->min);
	return 1;
    }
    return 0;
}

void mixer_init() 
{
    static char card[64] = "default";
    int err;
    if ((err = snd_mixer_open(&mixer_handle, 0)) < 0) {
	error("Mixer %s open error: %s\n", card, snd_strerror(err));
    }
    if ((err = snd_mixer_attach(mixer_handle, card)) < 0) {
	error("Mixer attach %s error: %s", card, snd_strerror(err));
	snd_mixer_close(mixer_handle);
    }
    /* no idea what this call is for: copy & paste from amixer.c */
    if ((err = snd_mixer_selem_register(mixer_handle, NULL, NULL)) < 0) {
	error("Mixer register error: %s", snd_strerror(err));
	snd_mixer_close(mixer_handle);
    }
    err = snd_mixer_load(mixer_handle);
    if (err < 0) {
	error("Mixer %s load error: %s", card, snd_strerror(err));
	snd_mixer_close(mixer_handle);
    }
  
    struct mixer_control * c=mixer_controls;
    while(c->name) {
	mixer_init_control(c);
	c++;
    }
}

int mixer_set_value(struct mixer_control * c, long new_value)
{
    if(new_value>= c->min && new_value <= c->max) {
	snd_mixer_selem_set_playback_volume(c->elem, SND_MIXER_SCHN_MONO,
					    new_value);
	return new_value;
    } 
    return c->min-1;
}

int mixer_get_value(struct mixer_control * c)
{
    long new_value;
    snd_mixer_selem_get_playback_volume(c->elem, SND_MIXER_SCHN_MONO,
					&new_value);
    return new_value;
} 

#if 0
int main() {
    mixer_init();
    int i;
    for(i=0; mixer_controls[i].name; i++) {
	mixer_set_value(&mixer_controls[i],i);
    }
    return 0;
}
#endif


