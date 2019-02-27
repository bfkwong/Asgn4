#include "mytar.h"

int xTarfile(int argc, char *argv[]) {

    int fd, i, j, permOct, numBlocks, newfildfd;
    char buffer[512], empty100[100], subdir[256], pathName[256], temp[256];
    struct Header *hContent;
    memset(buffer, 0, 512);
    memset(empty100, 0, 100);
    i = 0;

    if ((fd=open(argv[2], O_RDONLY)) < 0)
        return -1;
    
    
    for ((argc==3)?(i=2):(i=3); i<argc; i++) {
        if (i==2)
            subdir[0] = '\0';
        else
            strcpy(subdir, argv[i]);

        while(read(fd, buffer, 512) != 0) {

            hContent = processHeader(buffer);
            if (!strcmp(empty100, hContent->name))
                break;

            if (!strncmp(subdir, hContent->name, strlen(subdir))) {
                if (hContent->typeflag[0] == '5') {
                    strcpy(pathName, hContent->name);
                    strcat(pathName, hContent->prefix);
                    
                    permOct = strtol(hContent->mode, NULL, 8);
                    strcpy(temp, "./");
                    strcat(temp, pathName);
                    strcpy(pathName, temp);
                    
                    writeEmptyDir(pathName, permOct);
                }
            }
        }
        
        if (lseek(fd, 0, SEEK_SET) < 0)
            return -1;
        
        while(read(fd, buffer, 512) != 0) {
            hContent = processHeader(buffer);
            if (!strcmp(empty100, hContent->name))
                break;
            
            if (!strncmp(subdir, hContent->name, strlen(subdir))) {
                if (hContent->typeflag[0] == '0') {
                    newfildfd = writeEmptyFile(hContent);
                    
                    numBlocks = strtol(hContent->size, NULL, 8);
                    if (numBlocks/512.0 == 0.0)
                        numBlocks = 0;
                    else if (numBlocks/512.0 > 0)
                        numBlocks = numBlocks/512 + 1;
                    
                    /* Found the number of blocks that exist, just loop through that many blocks */
                    for (j = 0; j<numBlocks; j++) {
                        if(read(fd, buffer, 512) < 0)
                            return -1;
                        writeInfoToFile(buffer, newfildfd);
                    }
                    
                } else if (hContent->typeflag[0] == '2') {
                    /*writeSymLink(hContent);*/
                }
            }
        }
        
    }

    close(fd);
    return 0;

}

int writeEmptyDir(char *pathName, int permissions) {
    DIR * dir;

    dir = opendir(pathName);
    if (dir) {
    } else if (ENOENT == errno) {
        mkdir(pathName, permissions);
    } else {
        closedir(dir);
        return -1;
    }
    
    closedir(dir);
    return 0;
    
}

int writeEmptyFile(struct Header *h) {

    int permissions, fd;
    char path[256];
    
    memset(path, 0, 256);
    permissions = strtol(h->mode, NULL, 8);
    
    strcpy(path, h->name);
    strcat(path, h->prefix);
    if ((fd = open(path, O_RDWR|O_CREAT|O_TRUNC, permissions)) < 0)
        return -1;
    
    return fd;

}

int writeInfoToFile(char *buffer, int fd) {
    
    if(write(fd, buffer, 512) < 0)
        return -1;
    return 0;
}

//int writeSymLink(struct Header *h) {
//
//    return 0;
//
//}
