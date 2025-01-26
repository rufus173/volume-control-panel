CFLAGS=-g
LIBS=
CC=gcc
volume-control-panel-interpreter : main.o
	$(CC) $^ -o $@ $(LIBS)
main.o : src/main.c
	$(CC) $(CFLAGS) -c src/main.c
