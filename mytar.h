
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <grp.h>

#define C_INDEX 0
#define T_INDEX 1
#define X_INDEX 2
#define V_INDEX 3
#define S_INDEX 4
#define F_INDEX 5


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

int cTarfile(int fd, int argc, char *argv[]);
int cTarDirHelper(int fd, char *path);
int cWriteFile(int fd, char *path, struct stat *buf);
int writeHeader(int fd, struct Header *hContent);
int getPermissionFromMode(int mode);
void writeChksum(struct Header **h);
void triggerError(const char *arg, char myErrorCode);
int insert_special_int(char *where, size_t size, int32_t val);

int tTarfile(int argc, char *argv[]);
struct Header *processHeader(char *buf);  
