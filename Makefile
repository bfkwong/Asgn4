all: mytar mytar.o cOptions.o

mytar: mytar.o 
	gcc -Wall -g -o mytar mytar.o cOptions.o 
mytar.o: mytar.c
	gcc -Wall -g -c mytar.c
cOptions.o: cOptions.c
	gcc -Wall -g -c cOptions.c
