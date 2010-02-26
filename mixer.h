#include <alsa/asoundlib.h>

enum mixer_controls { MASTER=0,BASS,TREBLE,NCONTROLS };
struct mixer_control {
    long min,max;
    char *name;
    snd_mixer_elem_t *elem;
};

void mixer_init(void);
