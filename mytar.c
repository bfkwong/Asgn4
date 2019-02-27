#include "mytar.h"

char options[6] = "\0\0\0\0\0\0";

int main(int argc, char *argv[]) {

    int i, unrecogChar, tarFileFD;
    
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

    if (options[C_INDEX] == 1) {
        if ((tarFileFD = open(argv[2], O_RDWR|O_CREAT|O_TRUNC, 0644)) < 0)
                triggerError("", 'S');
        if (cTarfile(tarFileFD, argc, argv) < 0)
            printf("mytar: error\n");
    } else if (options[T_INDEX] == 1) {
        if (tTarfile(argc, argv) < 0)
                printf("mytar: error\n");
    } else if (options[X_INDEX] == 1) {
        if (xTarfile(argc, argv) < 0)
                printf("mytar: error\n");
    }

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

struct Header *processHeader(char *buf) {
    
    struct Header *h;
    h = (struct Header *)calloc(1, sizeof(struct Header));
    
    strncpy(h->name, buf, 100);
    strncpy(h->mode, buf+100, 8);
    strncpy(h->uid, buf+108, 8);
    strncpy(h->gid, buf+116, 8);
    strncpy(h->size, buf+124, 12);
    strncpy(h->mtime, buf+136, 12);
    strncpy(h->chksum, buf+148, 8);
    strncpy(h->typeflag, buf+156, 1);
    strncpy(h->linkname, buf+157, 100);
    strncpy(h->magic, buf+257, 6);
    strncpy(h->version, buf+263, 2);
    strncpy(h->uname, buf+265, 32);
    strncpy(h->gname, buf+297, 32);
    strncpy(h->devmajor, buf+329, 8);
    strncpy(h->devminor, buf+337, 8);
    strncpy(h->prefix, buf+345, 155);
    
    return h;
}
