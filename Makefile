all: mytar.o cOptions.o tOptions.o special.o mytar

mytar: mytar.o 
	gcc -Wall -g -o mytar mytar.o cOptions.o special.o tOptions.o
mytar.o: mytar.c
	gcc -Wall -g -c mytar.c
cOptions.o: cOptions.c
	gcc -Wall -g -c cOptions.c
special.o: special.c
	gcc -Wall -g -c special.c 
tOptions.o: tOptions.c
	gcc -Wall -g -c tOptions.c
clean: 
	rm tOptions.o special.o cOptions.o mytar.o
