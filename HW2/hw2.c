#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h> 

#define MAXLEN 1024

int main(int argc, char **argv) {
    char so_path[MAXLEN];
    char output_file[MAXLEN];
    char **cmd = NULL;

    snprintf(so_path, MAXLEN, "./logger.so");
    snprintf(output_file, MAXLEN, "stderr");

    int i = 1;
    if(argc == 1) {
        printf("no command given.\n");
        return -1;
    }
    while(i < argc) {
        if(!strcmp(argv[i], "-p")) 
            strncpy(so_path, argv[i+1], MAXLEN - 1);
        else if(!strcmp(argv[i], "-o")) 
            strncpy(output_file, argv[i+1], MAXLEN - 1);
        else if(!strcmp(argv[i], "--")) {
            i += 1;
            break;
        }
        else if(argv[i][0] != '-') 
            break;
        else {
            printf("./logger: invalid option -- \'%c\'\n", argv[i][1]);
            printf("usage: ./logger [-o file] [-p sopath] [--] cmd [cmd args ...]\n");
            printf("       -p: set the path to logger.so, default = ./logger.so\n");
            printf("       -o: print output to file, print to \"stderr\" if no file specified\n");
            printf("       --: separate the arguments for logger and for the command\n");
            return -1;
        }
        i += 2;
    }

    cmd = argv + i;
    int fd; 
    if(!strcmp(output_file, "stderr")) fd = dup(STDERR_FILENO);
    else fd  = fileno(fopen(output_file, "w")); 

    char env_log[MAXLEN + 1];
    snprintf(env_log, MAXLEN, "LD_PRELOAD=");
    strncat(env_log, so_path, MAXLEN);
    snprintf(output_file, MAXLEN, "OUTPUT=%d", fd);
    char *envp[] = {env_log, output_file, NULL};

    execvpe(cmd[0], cmd, envp);

    return 0;
}