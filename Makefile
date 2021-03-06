CFLAGS+=-Wall #-Werror
LDLIBS = -lmad -lao -lts -lasound -L. -lwma 
SRC=minipod.c fbutils.c font_8x8.c font_8x16.c minimad.c songlist.c \
	wma.c pcm.c gui.c layout.c playback.c mixer.c
OBJ=$(SRC:.c=.o)
WMA_SRC=$(addprefix libwma/,bitstream.c  wmadeci.c  wmafixed.c mdct.c mdct_lookup.c fft-ffmpeg.c asf.c)

minipod: libwma.a $(OBJ)

libwma.a: libwma.a($(WMA_SRC:.c=.o))

include depend 

depend: $(SRC) 
	gcc -MM $^ >depend

tags: $(SRC) $(WMA_SRC)
	etags $^

clean: 
	-rm *.o minipod libwma/*.o libwma.a depend
