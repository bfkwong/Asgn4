#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include "special.c"

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
    
    char buffer100[100] = "\0";
    char buffer8[8] = "\0";
    char buffer12[12] = "\0";
    
    strcpy(buffer100, path);
    if (S_ISDIR(buf->st_mode))
        strcat(buffer100, "/");
    if (S_ISREG(buf->st_mode) || S_ISLNK(buf->st_mode) || S_ISDIR(buf->st_mode))
        if (write(fd, buffer100, 100) < 0)
            return 1;
    if (vbose)
        printf("%s\n",buffer100);
    
    snprintf(buffer8, 8, "%07o", getPermissionFromMode(buf->st_mode));
    if (write(fd, buffer8, 8) < 0)
        return 1;
    
    insert_special_int(buffer8, sizeof(buffer8), buf->st_uid);
    if (write(fd, buffer8, 8) < 0)
        return 1;
    
    insert_special_int(buffer8, sizeof(buffer8), buf->st_gid);
    if (write(fd, buffer8, 8) < 0)
        return 1;
    
    (S_ISREG(buf->st_mode))?snprintf(buffer12, 12, "%011o", (int)buf->st_size):
        snprintf(buffer12, 12, "00000000000");
    if (write(fd, buffer12, 12) < 0)
        return 1;
    
    snprintf(buffer12, 12, "%011o", (int)buf->st_mtim.tv_sec);
    if (write(fd, buffer12, 12) < 0)
        return 1;
    
    
    
    
    return 0;
}

int getPermissionFromMode(int mode) {
    return  (mode&S_IRUSR)|(mode&S_IWUSR)|(mode&S_IXUSR)|
            (mode&S_IRGRP)|(mode&S_IWGRP)|(mode&S_IXGRP)|
            (mode&S_IROTH)|(mode&S_IWOTH)|(mode&S_IXOTH);
}

