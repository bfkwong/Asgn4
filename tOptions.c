#include "mytar.h"

extern char options[6]; 

int tTarfile(int argc, char *argv[]) {

    int fd, numBlocks, i;
    time_t octalTime;
    char buffer[512], empty100[100], permStr[11],
         ownerGrp[18], mTime[17], subdir[256];
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
                memset(permStr, '-', 11);
                getPermissions(hContent->mode, hContent->typeflag, permStr);
                
                strcpy(ownerGrp, hContent->uname);
                strcat(ownerGrp, "/");
                strcat(ownerGrp, hContent->gname);
                
                octalTime = (time_t)strtol(hContent->mtime, NULL, 8);
                strftime(mTime, sizeof(mTime), "%F %H:%M", localtime(&octalTime));
                
                if (options[V_INDEX] == 1)
                    printf("%s %.17s %8o %s %s\n",
                           permStr, ownerGrp,
                           (uint32_t)strtol(hContent->size, NULL, 8),
                           mTime, hContent->name);
                else
                    printf("%s\n", hContent->name);
                
                numBlocks = strtol(hContent->size, NULL, 8);
                
                if (numBlocks/512.0 == 0.0)
                    numBlocks = 0;
                else if (numBlocks/512.0 > 0)
                    numBlocks = numBlocks/512 + 1;
                
                for(i=0; i<numBlocks; i++) {
                    if(read(fd, buffer, 512) < 0)
                        return -1;
                }
            }
        }
    }
    
    close(fd);
    return 0; 

}

int getPermissions(char *permStr, char *type, char *buf) {
    int permOctal = strtol(permStr, NULL, 8);
    
    if (type[0] == '5')
        buf[0] = 'd';
    else if (type[0] == '2')
        buf[0] = 'l';
    
    if (permOctal&S_IRUSR) buf[1] = 'r';
    if (permOctal&S_IWUSR) buf[2] = 'w';
    if (permOctal&S_IXUSR) buf[3] = 'x';
    if (permOctal&S_IRGRP) buf[4] = 'r';
    if (permOctal&S_IWGRP) buf[5] = 'w';
    if (permOctal&S_IXGRP) buf[6] = 'x';
    if (permOctal&S_IROTH) buf[7] = 'r';
    if (permOctal&S_IWOTH) buf[8] = 'w';
    if (permOctal&S_IXOTH) buf[9] = 'x';
    
    buf[10] = '\0';
    
    return 0;
}
