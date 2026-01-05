PKGS=libconfig
PKG_CFLAGS=$(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -lm
OBJS=write.o read.o
CFLAGS+=-Wall -Werror

%.o: %.c
	$(CC) $(PKG_CFLAGS) $(CFLAGS) -c $< -o $@

main: main.c $(OBJS)
	$(CC) $(PKG_CFLAGS) $(CFLAGS) $(LIBS) $(OBJS) main.c -o main

all:	main

clean:
	rm -fv main *.o
