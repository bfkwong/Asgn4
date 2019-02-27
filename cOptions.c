#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include "special.c"

struct Header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char empty12[12];
};

int cTarfile(int fd, int argc, char *argv[], char vbose);
int cTarDirHelper(int fd, char *path, char vbose);
int cWriteFile(int fd, char *path, struct stat *buf, char vbose, char type);
int writeHeader(int fd, struct Header *hContent);
int getPermissionFromMode(int mode);
void writeChksum(struct Header **h);

int cTarfile(int fd, int argc, char *argv[], char vbose) {
    struct stat buf;
    char *twoEmpty512Block;
    int i;

    twoEmpty512Block = (char *)calloc(1024, sizeof(char));
    
    for(i=3; i<argc; i++) {
        if(lstat(argv[i], &buf) < 0)
            return -1;
        if(S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) {
            if (cWriteFile(fd, argv[i], &buf, vbose, 'r'))
                return -1;
        } else if (S_ISDIR(buf.st_mode)) {
            if (cWriteFile(fd, argv[i], &buf, vbose, 'd'))
                return -1;
            if (cTarDirHelper(fd, argv[i], vbose))
                return -1;
        }
    }
    
    if(write(fd, twoEmpty512Block, 1024) < 0)
        return -1;

    
    return 0;
}

int cTarDirHelper(int fd, char *path, char vbose) {
    DIR * dp;
    struct dirent *dirFile;
    struct stat buf;
    char *tempPath;

    if ((tempPath = (char *)calloc(strlen(path), sizeof(char))) < 0)
        return -1;
    if ((dp = opendir(path)) == NULL)
        return -1;
    while((dirFile = readdir(dp)) != NULL) {
        strcpy(tempPath, path);
        strcat(tempPath, "/");
        strcat(tempPath, dirFile->d_name);

        if(lstat(tempPath, &buf) < 0)
            return -1;
        if(S_ISREG(buf.st_mode)) {
            if (cWriteFile(fd, tempPath, &buf, vbose, 'r'))
                return -1;
        } else if (S_ISDIR(buf.st_mode)) {
            if (strcmp(dirFile->d_name, ".") && strcmp(dirFile->d_name, "..")) {
                if (cWriteFile(fd, tempPath, &buf, vbose, 'd'))
                    return -1;
                if (cTarDirHelper(fd, tempPath, vbose))
                    return -1;
            }
        }
    }

    closedir(dp);
    free(tempPath);
    return 0;
}

int cWriteFile(int fd, char *path, struct stat *buf, char vbose, char type) {

    struct Header *hContent;
    struct passwd *pw;
    struct group *gp;
    char bufferBlck[512] = "\0";
    int fileFd;

    if (!(hContent = (struct Header *)calloc(1, sizeof(struct Header))))
        return -1;
    
    if (S_ISREG(buf->st_mode)||S_ISLNK(buf->st_mode)||S_ISDIR(buf->st_mode)) {
        strncpy(hContent->name, path, 100);
        if (S_ISDIR(buf->st_mode))
            strcat(hContent->name, "/");
        if (strlen(path) > 100)
            strncpy(hContent->prefix, path+100, 155);
        if (strlen(path) > 255) {
            fprintf(stderr, "Path name longer than 255");
            return -1;
        }
        if (vbose)
            printf("%s%s\n", hContent->name, hContent->prefix);
    } else {
        printf("Error: Unsupported File Type");
        return 0;
    }

    snprintf(hContent->mode, 8, "%07o", getPermissionFromMode(buf->st_mode));
    insert_special_int(hContent->uid, sizeof(hContent->uid), buf->st_uid);
    insert_special_int(hContent->gid, sizeof(hContent->gid), buf->st_gid);
    
    (S_ISREG(buf->st_mode))?
        snprintf(hContent->size, 12, "%011o", (int)buf->st_size):
        snprintf(hContent->size, 12, "00000000000");
    
    snprintf(hContent->mtime, 12, "%011o", (int)buf->st_mtim.tv_sec);

    if (S_ISREG(buf->st_mode))
        hContent->typeflag[0] = '0';
    else if (S_ISDIR(buf->st_mode))
        hContent->typeflag[0] = '5';
    else if (S_ISLNK(buf->st_mode))
        hContent->typeflag[0] = '2';
    else
        hContent->typeflag[0] = '\0';

    strncpy(hContent->magic, "ustar", 6);
    strncpy(hContent->version, "00", 2);

    pw = getpwuid(buf->st_uid);
    strncpy(hContent->uname, pw->pw_name, 31);
    hContent->uname[31] = 0;

    gp = getgrgid(buf->st_gid);
    strncpy(hContent->gname, gp->gr_name, 31);
    hContent->uname[31] = 0;

    strncpy(hContent->chksum, "        ", 8);
    writeChksum(&hContent);
    writeHeader(fd, hContent);
    
    if (S_ISREG(buf->st_mode)) {
        if((fileFd = open(path, O_RDONLY, 0644)) < 0)
            return -1;
        while(read(fileFd, bufferBlck, 512) > 0) {
            if (write(fd, bufferBlck, 512) < 0)
                return -1;
        }
    }
    return 0;
    
}

int writeHeader(int fd, struct Header *hContent) {
    if (write(fd, hContent->name, 100) < 0)
        return -1;
    if (write(fd, hContent->mode, 8) < 0)
        return -1;
    if (write(fd, hContent->uid, 8) < 0)
        return -1;
    if (write(fd, hContent->gid, 8) < 0)
        return -1;
    if (write(fd, hContent->size, 12) < 0)
        return -1;
    if (write(fd, hContent->mtime, 12) < 0)
        return -1;
    if (write(fd, hContent->chksum, 8) < 0)
        return -1;
    if (write(fd, hContent->typeflag, 1) < 0)
        return -1;
    if (write(fd, hContent->linkname, 100) < 0)
        return -1;
    if (write(fd, hContent->magic, 6) < 0)
        return -1;
    if (write(fd, hContent->version, 2) < 0)
        return -1;
    if (write(fd, hContent->uname, 32) < 0)
        return -1;
    if (write(fd, hContent->gname, 32) < 0)
        return -1;
    if (write(fd, hContent->devminor, 8) < 0)
        return -1;
    if (write(fd, hContent->devmajor, 8) < 0)
        return -1;
    if (write(fd, hContent->prefix, 155) < 0)
        return -1;
    if (write(fd, hContent->empty12, 12) < 0)
        return -1;
    return 0;
}

int getPermissionFromMode(int mode) {
    return  (mode&S_IRUSR)|(mode&S_IWUSR)|(mode&S_IXUSR)|
            (mode&S_IRGRP)|(mode&S_IWGRP)|(mode&S_IXGRP)|
            (mode&S_IROTH)|(mode&S_IWOTH)|(mode&S_IXOTH);
}

void writeChksum(struct Header **h) {
    uint32_t ckSumSize = 0;
    int i;
    for(i=0; i<8; i++) {
        ckSumSize += (uint8_t)(*h)->mode[i];
        ckSumSize += (uint8_t)(*h)->uid[i];
        ckSumSize += (uint8_t)(*h)->gid[i];
        ckSumSize += (uint8_t)(*h)->chksum[i];
        ckSumSize += (uint8_t)(*h)->devminor[i];
        ckSumSize += (uint8_t)(*h)->devmajor[i];
    }
    for(i=0; i<12; i++) {
        ckSumSize += (uint8_t)(*h)->size[i];
        ckSumSize += (uint8_t)(*h)->mtime[i];
    }
    for(i=0; i<100; i++){
        ckSumSize += (uint8_t)(*h)->name[i];
        ckSumSize += (uint8_t)(*h)->linkname[i];
    }
    for(i=0; i<32; i++){
        ckSumSize += (uint8_t)(*h)->uname[i];
        ckSumSize += (uint8_t)(*h)->gname[i];
    }
    for(i=0; i<155; i++)
        ckSumSize += (uint8_t)(*h)->prefix[i];
    for(i=0; i<2; i++)
        ckSumSize += (uint8_t)(*h)->version[i];
    for(i=0; i<6; i++)
        ckSumSize += (uint8_t)(*h)->magic[i];
    for(i=0; i<1; i++)
        ckSumSize += (uint8_t)(*h)->typeflag[i];
    
    snprintf((*h)->chksum, 8, "%07o",ckSumSize);
    return;
}
