CFLAGS=-g
LIBS=
CC=gcc
install : volume-control-panel-interpreter
	./install
volume-control-panel-interpreter : main.o
	$(CC) $^ -o $@ $(LIBS)
main.o : src/main.c
	$(CC) $(CFLAGS) -c src/main.c
