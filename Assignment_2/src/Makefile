CC=gcc
CFLAGS=
FMT=indent

mysh: shell.c interpreter.c shellmemory.c readyqueue.c
	$(CC) $(CFLAGS) -c shell.c interpreter.c shellmemory.c readyqueue.c
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o readyqueue.o

style: shell.c shell.h interpreter.c interpreter.h shellmemory.c shellmemory.h readyqueue.c readyqueue.h
	$(FMT) $?

clean: 
	$(RM) mysh; $(RM) *.o; $(RM) *~

