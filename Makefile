FLAGS = -Wall -lm

main: ejemplo.c funciones.o
	gcc $(FLAGS) -o ejemplo ejemplo.c funciones.o

funciones.o: funciones.c
	gcc -c $(FLAGS) funciones.c

clear:
	rm -f ejemplo *.o
