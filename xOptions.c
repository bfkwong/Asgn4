#include "mytar.h"

int xTarfile(int argc, char *argv[]) {
    
    int fd, i;
    char buffer[512], empty100[100], subdir[256];
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
                
            }
        }
    }
    
    close(fd);
    return 0;

}
