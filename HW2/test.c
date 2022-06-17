#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

int main()
{
    char buf[100] = {0};
    creat("test.txt", 0600);
    chmod("test.txt", 0666);
    chown("test.txt", 65534, 65534);
    rename("test.txt", "test2.txt");
    int fd = open("test2.txt", O_APPEND | O_RDWR, 0666);
    write(fd, "ccc", 3);
    close(fd);
    fd = open("test2.txt", O_RDONLY, 0);
    read(fd, buf, 100);
    close(fd);
    FILE *f = tmpfile();
    fwrite("12345\0  678901234567890\n", 1, 10, f);
    fclose(f);
    f = fopen("test2.txt", "r");
    char buf2[100] = {0};
    fread(buf2, 1, 100, f);
    fclose(f);
    remove("test2.txt");
    //printf("sample done.\n");
    return 0;
}