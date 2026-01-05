PKGS=libconfig
PKG_CFLAGS=$(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -lm
CFLAGS+=-Wall -Werror
OBJS=

%.o: %.c
	$(CC) $(PKG_CFLAGS) $(CFLAGS) -c $< -o $@

main: main.c $(OBJS)
	$(CC) $(PKG_CFLAGS) $(CFLAGS) $(LIBS) $(OBJS) main.c -o main

all:	main

clean:
	rm -fv main *.o
