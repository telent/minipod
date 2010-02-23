CC=arm-linux-gnueabi-gcc

SRC=bitstream.c  wmadeci.c  wmafixed.c mdct.c mdct_lookup.c fft-ffmpeg.c	
OBJ=$(SRC:.c=.o)
libwma.a: $(OBJ)
	$(AR) r  $@ $(OBJ)

clean: 
	-rm $(OBJ) *.a
