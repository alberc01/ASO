#!/bin/bash
OBJS= libreria.o main.o
app: main.o
	gcc -c -fPIC libreria.c
	gcc -shared -fPIC -o libreria.so libreria.o
	mkdir lib
	mv libreria.so lib
	gcc -o $@ $(OBJS) -L/lib/libreria.so
main.o: main.c
	gcc -c main.c
libreria.o: libreria.c
	gcc -c libreria.c
static: $(OBJS)
	ar cr libreria.a main.o libreria.o
	mkdir lib
	mv libreria.a lib
	gcc -o app -static $(OBJS)  -L/lib/libreria.a
clean:
	-rm app
	-rm *.o
	-rm -R lib
	-rm copia.txt
