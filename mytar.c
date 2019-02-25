#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

#define C_INDEX 0
#define T_INDEX 1
#define X_INDEX 2
#define V_INDEX 3
#define S_INDEX 4
#define F_INDEX 5

void triggerError(const char *arg, char myErrorCode);
int cTarfile(int fd, int argc, char *argv[]);
int cTarDirHelper(int fd, char *path);
int cWriteFile(int fd, char *path, struct stat *buf);


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
    if ((tarFileFD = open(argv[2], O_RDWR|O_CREAT, 0644)) < 0)
        triggerError("", 'S');

    if (options[C_INDEX] == 1)
        if (cTarfile(tarFileFD, argc, argv) == 1)
            triggerError("", 'S');


    return 0; 
}

int cTarfile(int fd, int argc, char *argv[]) {
    struct stat buf;
    int i;
    
    for(i=3; i<argc; i++) {
        if(lstat(argv[i], &buf) < 0)
            return 1;
        if(S_ISREG(buf.st_mode)) {
            if (cWriteFile(fd, argv[i], &buf))
                return 1;
        } else if (S_ISDIR(buf.st_mode)) {
            if (cWriteFile(fd, argv[i], &buf))
                return 1;
            if (cTarDirHelper(fd, argv[i]))
                return 1;
        }
    }
    
    return 0;
}

int cTarDirHelper(int fd, char *path) {
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
            if (cWriteFile(fd, tempPath, &buf))
                return 1;
        } else if (S_ISDIR(buf.st_mode)) {
            if (strcmp(dirFile->d_name, ".") != 0 && strcmp(dirFile->d_name, "..") != 0) {
                if (cWriteFile(fd, tempPath, &buf))
                    return 1;
                if (cTarDirHelper(fd, tempPath))
                    return 1;
            }
        }
    }
    
    closedir(dp);
    free(tempPath);
    return 0;
}

int cWriteFile(int fd, char *path, struct stat *buf) {
    char pathBuff[100] = "\0";
    char tempMode[8], finalMode[8];
    int i;

    strcpy(pathBuff, path);
    if (S_ISDIR(buf->st_mode))
        strcat(pathBuff, "/");
    if (write(fd, pathBuff, 100) < 0)
        return 1;
    
    for (i = 0; i<9; i++) {
        tempMode[i] = 0;
        finalMode[i] = 0;
    }
    sprintf(tempMode, "%o", buf->st_mode);
    strcpy(finalMode, "000");
    strcat(finalMode, tempMode + (strlen(tempMode) - 4));
    if (write(fd, finalMode, 8) < 0)
        return 1;
    
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
