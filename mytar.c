#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include "cOptions.c"

#define C_INDEX 0
#define T_INDEX 1
#define X_INDEX 2
#define V_INDEX 3
#define S_INDEX 4
#define F_INDEX 5

void triggerError(const char *arg, char myErrorCode);

int main(int argc, char *argv[]) {

    int i, unrecogChar, tarFileFD;
    char options[6] = "\0\0\0\0\0\0";
    
    if (argc == 1)
        triggerError(argv[0], 'N');
    
    unrecogChar = 0;
    for(i = 0; i < strlen(argv[1]); i++) {
        switch (argv[1][i])
        {
            case 'c': options[C_INDEX] = 1; break;
            case 't': options[T_INDEX] = 1; break;
            case 'x': options[X_INDEX] = 1; break;
            case 'v': options[V_INDEX] = 1; break;
            case 'S': options[S_INDEX] = 1; break;
            case 'f': options[F_INDEX] = 1; break;
            default:
                printf("%s: unrecognized option '%c'.\n", argv[0], argv[1][i]);
                unrecogChar = 1; break;
        }
    }

    if (options[C_INDEX] + options[T_INDEX] + options[X_INDEX] > 1)
        triggerError(argv[0], 'T');
    if (argc < 3 || unrecogChar == 1)
        triggerError(argv[0], 'U');
    
    if ((tarFileFD = open(argv[2], O_RDWR|O_CREAT|O_TRUNC, 0644)) < 0)
        triggerError("", 'S');

    if (options[C_INDEX] == 1)
        if (cTarfile(tarFileFD, argc, argv, options[V_INDEX]) == 1)
            triggerError("", 'S');


    return 0; 
}

void triggerError(const char *arg, char myErrorCode) {
    if (myErrorCode == 'N')
        printf("%s: you must specify at least one of the 'ctx' options.\n"
               "usage: %s [ctxSp[f tarfile]] [file1[file2[...]]]\n", arg, arg);
    else if (myErrorCode == 'T')
        printf("%s: you may only choose one of the 'ctx' options.\n"
               "usage: %s [ctxSp[f tarfile]] [file1[file2[...]]]\n", arg, arg);
    else if (myErrorCode == 'U')
        printf("usage: %s [ctxSp[f tarfile]] [file1 [ file2 [...] ] ]\n", arg);
    else if (myErrorCode == 'S')
        perror("mytar");
    exit(1);
}
