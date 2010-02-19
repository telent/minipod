CC=arm-linux-gnueabi-gcc
CFLAGS= -march=armv4t -mtune=arm920t -I .
LDLIBS = -lmad -lao -lts

minipod: minipod.o fbutils.o font_8x8.o font_8x16.o minimad.o songlist.o

clean: 
	-rm *.o minipod

