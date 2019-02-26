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
};

int cTarfile(int fd, int argc, char *argv[], char vbose);
int cTarDirHelper(int fd, char *path, char vbose);
int cWriteFile(int fd, char *path, struct stat *buf, char vbose, char type);
int getPermissionFromMode(int mode);

int cTarfile(int fd, int argc, char *argv[], char vbose) {
    struct stat buf;
    int i;

    for(i=3; i<argc; i++) {
        if(lstat(argv[i], &buf) < 0)
            return 1;
        if(S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) {
            if (cWriteFile(fd, argv[i], &buf, vbose, 'r'))
                return 1;
        } else if (S_ISDIR(buf.st_mode)) {
            if (cWriteFile(fd, argv[i], &buf, vbose, 'd'))
                return 1;
            if (cTarDirHelper(fd, argv[i], vbose))
                return 1;
        }
    }
    return 0;
}

int cTarDirHelper(int fd, char *path, char vbose) {
    DIR * dp;
    struct dirent *dirFile;
    struct stat buf;
    char *tempPath;

    if ((tempPath = (char *)calloc(strlen(path), sizeof(char))) < 0)
        return 1;
    if ((dp = opendir(path)) == NULL)
        return 1;
    while((dirFile = readdir(dp)) != NULL) {
        strcpy(tempPath, path);
        strcat(tempPath, "/");
        strcat(tempPath, dirFile->d_name);

        if(lstat(tempPath, &buf) < 0)
            return 1;
        if(S_ISREG(buf.st_mode)) {
            if (cWriteFile(fd, tempPath, &buf, vbose, 'r'))
                return 1;
        } else if (S_ISDIR(buf.st_mode)) {
            if (strcmp(dirFile->d_name, ".") != 0 && strcmp(dirFile->d_name, "..") != 0) {
                if (cWriteFile(fd, tempPath, &buf, vbose, 'd'))
                    return 1;
                if (cTarDirHelper(fd, tempPath, vbose))
                    return 1;
            }
        }
    }

    closedir(dp);
    free(tempPath);
    return 0;
}

int cWriteFile(int fd, char *path, struct stat *buf, char vbose, char type) {

    // char buffer100[100] = "\0";
    // char buffer8[8] = "\0";
    struct Header *hContent;
    struct passwd *pw;
    struct group *gp;
    int i, chksumCount;

    hContent = (struct Header *)calloc(1, sizeof(struct Header));

    if (S_ISREG(buf->st_mode) || S_ISLNK(buf->st_mode) || S_ISDIR(buf->st_mode)) {
        strncpy(hContent->name, path, 100);
        if (S_ISDIR(buf->st_mode))
            strcat(hContent->name, "/");
        if (strlen(path) > 100)
            strncpy(hContent->prefix, path+100, 155);
        if (strlen(path) > 255) {
            fprintf(stderr, "Path name longer than 255");
            return 1;
        }
        if (vbose)
            printf("%s%s\n", hContent->name, hContent->prefix);
    } else {
        printf("Error: Unsupported File Type");
        return 0;
    }

    if (write(fd, hContent->name, 100) < 0)
        return 1;

    snprintf(hContent->mode, 8, "%07o", getPermissionFromMode(buf->st_mode));
    if (write(fd, hContent->mode, 8) < 0)
        return 1;

    insert_special_int(hContent->uid, sizeof(hContent->uid), buf->st_uid);
    if (write(fd, hContent->uid, 8) < 0)
        return 1;

    insert_special_int(hContent->gid, sizeof(hContent->gid), buf->st_gid);
    if (write(fd, hContent->gid, 8) < 0)
        return 1;

    (S_ISREG(buf->st_mode))?snprintf(hContent->size, 12, "%011o", (int)buf->st_size):
        snprintf(hContent->size, 12, "00000000000");
    if (write(fd, hContent->size, 12) < 0)
        return 1;

    snprintf(hContent->mtime, 12, "%011o", (int)buf->st_mtim.tv_sec);
    if (write(fd, hContent->mtime, 12) < 0)
        return 1;

    if (write(fd, "0000000", 8) < 0)
        return 1;

    if (S_ISREG(buf->st_mode))
        hContent->typeflag[0] = '0';
    else if (S_ISDIR(buf->st_mode))
        hContent->typeflag[0] = '5';
    else if (S_ISLNK(buf->st_mode))
        hContent->typeflag[0] = '2';
    else
        hContent->typeflag[0] = '\0';
    if (write(fd, hContent->typeflag, 1) < 0)
        return 1;

    strncpy(hContent->magic, "ustar", 6);
    strncpy(hContent->version, "00", 2);

    pw = getpwuid(buf->st_uid);
    strncpy(hContent->uname, pw->pw_name, 31);
    hContent->uname[31] = 0;

    gp = getgrgid(buf->st_gid);
    strncpy(hContent->gname, gp->gr_name, 31);
    hContent->uname[31] = 0;

    snprintf(hContent->devmajor, 8, "0000000");
    snprintf(hContent->devminor, 8, "0000000");
    snprintf(hContent->chksum, 8, "       ");


    return 0;
}

int getPermissionFromMode(int mode) {
    return  (mode&S_IRUSR)|(mode&S_IWUSR)|(mode&S_IXUSR)|
            (mode&S_IRGRP)|(mode&S_IWGRP)|(mode&S_IXGRP)|
            (mode&S_IROTH)|(mode&S_IWOTH)|(mode&S_IXOTH);
}
