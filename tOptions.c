#include "mytar.h"

extern char options[6]; 

int tTarfile(int argc, char *argv[]) {

    int fd;
    char buffer[512];  
    memset(buffer, 0, 512); 

    if ((fd=open(argv[2], O_RDONLY)) < 0)
        return -1; 
    while(read(fd, buffer, 512) != 0) {
        processHeader(buffer); 
        break; 
    }

    return 0; 

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
