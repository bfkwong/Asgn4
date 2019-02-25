mytar: mytar.o 
	gcc -Wall -g -o mytar mytar.o 
mytar.o: mytar.c
	gcc -Wall -g -c mytar.c
