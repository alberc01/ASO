#!/bin/bash
all: dem cli

dem: 
	gcc -o dem demonio.c
	@echo "se ha creado el ejecutable dem para el demonio"
cli:  
	gcc -o cli cliente.c
	@echo " se ha creado el ejecutable cli para el cliente"

clean:
	-rm dem
	-rm cli
