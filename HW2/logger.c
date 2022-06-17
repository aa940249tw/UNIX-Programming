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
FILE *f = NULL;

FILE *fopen(const char *filename, const char *mode) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    char path[MAXLEN];
    char *exsist;
    if((exsist = realpath(filename, path)) == NULL) strcpy(path, filename);
    FILE *(*tmp)(const char *, const char *) = (FILE *(*)(const char *, const char *)) dlsym(RTLD_NEXT, "fopen");
    FILE *fp = tmp(filename, mode);
    fprintf(f, "[logger] fopen(\"%s\", \"%s\") = %p\n", path, mode, fp);
    return fp;
}

FILE *fopen64(const char *filename, const char *mode) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    char path[MAXLEN];
    char *exsist;
    if((exsist = realpath(filename, path)) == NULL) strcpy(path, filename);
    FILE *(*tmp)(const char *, const char *) = (FILE *(*)(const char *, const char *)) dlsym(RTLD_NEXT, "fopen64");
    FILE *fp = tmp(filename, mode);
    fprintf(f, "[logger] fopen64(\"%s\", \"%s\") = %p\n", path, mode, fp);
    return fp;
}

int fclose(FILE *stream) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    char procpath[MAXLEN + 1];
    char filepath[MAXLEN + 1];
    int read_size = 0;
    int fd = fileno(stream);
    snprintf(procpath, MAXLEN, "/proc/self/fd/%d", fd);
    if ((read_size = readlink(procpath, filepath, (size_t) MAXLEN)) < 0) return -1;
    filepath[read_size] = '\0';
    int (*tmp)(FILE *) = (int (*)(FILE *)) dlsym(RTLD_NEXT, "fclose");
    int ret = tmp(stream);
    fprintf(f, "[logger] fclose(\"%s\") = %d\n", filepath, ret);
    return ret;
}

int chmod(const char *pathname, mode_t mode) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    int (*tmp)(const char *, mode_t) = (int (*)(const char *, mode_t)) dlsym(RTLD_NEXT, "chmod");
    int ret = tmp(pathname, mode);
    fprintf(f, "[logger] chmod(\"%s\", %o) = %d\n", pathname, mode, ret);
    return ret;
}

int chown(const char *pathname, uid_t owner, gid_t group) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    int (*tmp)(const char *, uid_t, gid_t) = (int (*)(const char *, uid_t, gid_t)) dlsym(RTLD_NEXT, "chown");
    int ret = tmp(pathname, owner, group);
    fprintf(f, "[logger] chown(\"%s\", %d, %d) = %d\n", pathname, owner, group, ret);
    return ret;
}

int close(int fd) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    char procpath[MAXLEN + 1];
    char filepath[MAXLEN + 1];
    int read_size = 0;
    snprintf(procpath, MAXLEN, "/proc/self/fd/%d", fd);
    if ((read_size = readlink(procpath, filepath, (size_t) MAXLEN)) < 0) return -1;
    filepath[read_size] = '\0';
    int (*tmp)(int) = (int (*)(int)) dlsym(RTLD_NEXT, "close");
    int ret = tmp(fd);
    fprintf(f, "[logger] close(\"%s\") = %d\n", filepath, ret);
    return ret;
}

int creat(const char *path, mode_t mode) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    int (*tmp)(const char *, mode_t) = (int (*)(const char *, mode_t)) dlsym(RTLD_NEXT, "creat");
    int ret = tmp(path, mode);
    fprintf(f, "[logger] creat(\"%s\", %o) = %d\n", path, mode, ret);
    return ret;
}

int creat64(const char *path, mode_t mode) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    int (*tmp)(const char *, mode_t) = (int (*)(const char *, mode_t)) dlsym(RTLD_NEXT, "creat64");
    int ret = tmp(path, mode);
    fprintf(f, "[logger] creat64(\"%s\", %o) = %d\n", path, mode, ret);
    return ret;
}

size_t fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    size_t (*tmp)(void *restrict, size_t, size_t, FILE *restrict) = (size_t (*)(void *restrict, size_t, size_t, FILE *restrict)) dlsym(RTLD_NEXT, "fread");
    size_t ret = tmp(ptr, size, nmemb, stream);
    char procpath[MAXLEN + 1];
    char filepath[MAXLEN + 1];
    int read_size = 0;
    int fd = fileno(stream);
    snprintf(procpath, MAXLEN, "/proc/self/fd/%d", fd);
    if ((read_size = readlink(procpath, filepath, (size_t) MAXLEN)) < 0) return -1;
    filepath[read_size] = '\0';
    char rename[33];
    int cnt = 0;
    while(cnt < 32 && cnt < ret) 
        rename[cnt++] = isprint(((char *)ptr)[cnt]) ? ((char *)ptr)[cnt] : '.';
    rename[cnt] = '\0';
    fprintf(f, "[logger] fread(\"%s\", %ld, %ld, \"%s\") = %ld\n", rename, size, nmemb, filepath, ret);
    return ret;
}

size_t fwrite(const void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    size_t (*tmp)(const void *restrict, size_t, size_t, FILE *restrict) = (size_t (*)(const void *restrict, size_t, size_t, FILE *restrict)) dlsym(RTLD_NEXT, "fwrite");
    size_t ret = tmp(ptr, size, nitems, stream);
    char procpath[MAXLEN + 1];
    char filepath[MAXLEN + 1];
    int read_size = 0;
    int fd = fileno(stream);
    snprintf(procpath, MAXLEN, "/proc/self/fd/%d", fd);
    if ((read_size = readlink(procpath, filepath, (size_t) MAXLEN)) < 0) return -1;
    filepath[read_size] = '\0';
    char rename[33];
    int cnt = 0;
    while(cnt < 32 && cnt < ret) 
        rename[cnt++] = isprint(((char *)ptr)[cnt]) ? ((char *)ptr)[cnt] : '.';
    rename[cnt] = '\0';
    fprintf(f, "[logger] fwrite(\"%s\", %ld, %ld, \"%s\") = %ld\n", rename, size, nitems, filepath, ret);
    return ret;
}

int open(const char *pathname, int flags, ...) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    mode_t mode = 0;
    if (__OPEN_NEEDS_MODE (flags)) {
        va_list arg;
        va_start (arg, flags);
        mode = va_arg (arg, int);
        va_end (arg);
    }
    int (*tmp)(const char *, int, mode_t) = (int (*)(const char *, int, mode_t)) dlsym(RTLD_NEXT, "open");
    int ret = tmp(pathname, flags, mode);
    fprintf(f, "[logger] open(\"%s\", %o, %o) = %d\n", pathname, flags, mode, ret);
    return ret;
}

int open64(const char *pathname, int flags, ...) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    mode_t mode = 0;
    if (__OPEN_NEEDS_MODE (flags)) {
        va_list arg;
        va_start (arg, flags);
        mode = va_arg (arg, int);
        va_end (arg);
    }
    int (*tmp)(const char *, int, mode_t) = (int (*)(const char *, int, mode_t)) dlsym(RTLD_NEXT, "open64");
    int ret = tmp(pathname, flags, mode);
    fprintf(f, "[logger] open64(\"%s\", %o, %o) = %d\n", pathname, flags, mode, ret);
    return ret;
}

ssize_t read(int fd, void *buf, size_t count) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    ssize_t (*tmp)(int, void *, size_t) = (ssize_t (*)(int, void *, size_t)) dlsym(RTLD_NEXT, "read");
    ssize_t ret = tmp(fd, buf, count);
    char procpath[MAXLEN + 1];
    char filepath[MAXLEN + 1];
    int read_size = 0;
    snprintf(procpath, MAXLEN, "/proc/self/fd/%d", fd);
    if ((read_size = readlink(procpath, filepath, (size_t) MAXLEN)) < 0) return -1;
    filepath[read_size] = '\0';
    char rename[33];
    int cnt = 0;
    while(cnt < 32 && cnt < ret) 
        rename[cnt++] = isprint(((char *)buf)[cnt]) ? ((char *)buf)[cnt] : '.';
    rename[cnt] = '\0';
    fprintf(f, "[logger] read(\"%s\", \"%s\", %ld) = %ld\n", filepath, rename, count, ret);
    return ret;
}

int remove(const char *pathname) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    int (*tmp)(const char *) = (int (*)(const char *)) dlsym(RTLD_NEXT, "remove");
    int ret = tmp(pathname);
    fprintf(f, "[logger] remove(\"%s\") = %d\n", pathname, ret);
    return ret;
}

int rename(const char *oldpath, const char *newpath) {
    int (*tmp)(const char *, const char *) = (int (*)(const char *, const char *)) dlsym(RTLD_NEXT, "rename");
    int ret = tmp(oldpath, newpath);
    fprintf(f, "[logger] rename(\"%s\", \"%s\") = %d\n", oldpath, newpath, ret);
    return ret;
}

FILE *tmpfile() {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    FILE *(*tmp)(void) = (FILE *(*)(void)) dlsym(RTLD_NEXT, "tmpfile");
    FILE *fd = tmp();
    fprintf(f, "[logger] tmpfile() = %p\n", fd);
    return fd;
}

FILE *tmpfile64() {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    FILE *(*tmp)(void) = (FILE *(*)(void)) dlsym(RTLD_NEXT, "tmpfile64");
    FILE *fd = tmp();
    fprintf(f, "[logger] tmpfile64() = %p\n", fd);
    return fd;
}

ssize_t write(int fd, const void *buf, size_t count) {
    if(!f) {
        char *fds = getenv("OUTPUT");
        f = fdopen(atoi(fds), "w");
    }
    ssize_t (*tmp)(int, const void *, size_t) = (ssize_t (*)(int, const void *, size_t)) dlsym(RTLD_NEXT, "write");
    ssize_t ret = tmp(fd, buf, count);
    char procpath[MAXLEN + 1];
    char filepath[MAXLEN + 1];
    int read_size = 0;
    snprintf(procpath, MAXLEN, "/proc/self/fd/%d", fd);
    if ((read_size = readlink(procpath, filepath, (size_t) MAXLEN)) < 0) return -1;
    filepath[read_size] = '\0';
    char rename[33];
    int cnt = 0;
    while(cnt < 32 && cnt < ret) 
        rename[cnt++] = isprint(((char *)buf)[cnt]) ? ((char *)buf)[cnt] : '.';
    rename[cnt] = '\0';
    fprintf(f, "[logger] write(\"%s\", \"%s\", %ld) = %ld\n", filepath, rename, count, ret);
    return ret;
}
