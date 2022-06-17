#include "libmini.h"

long errno;
extern long _syscall(long number, ...);

#define	WRAPPER_RETval(type)    errno = 0; if(ret < 0) { errno = -ret; return -1; } return ((type) ret);
#define	WRAPPER_RETptr(type)    errno = 0; if(ret < 0) { errno = -ret; return NULL; } return ((type) ret);


size_t strlen(const char *str) {
    const char *s;
    for (s = str; *s; ++s);
    return (s - str);
}

void *memset (void *dest, int val, size_t len) {
    unsigned char *ptr = dest;
    while (len-- > 0)
        *ptr++ = val;
    return dest;
}

void *memcpy (void *dest, const void *src, size_t len) {
    char *d = dest;
    const char *s = src;
    while (len--)
        *d++ = *s++;
    return dest;
}

ssize_t	write(int fd, const void *buf, size_t count) {
    long ret = _syscall(sys_write, fd, buf, count);
    WRAPPER_RETval(ssize_t);
}

unsigned int alarm (unsigned int seconds) {
    long ret = _syscall(sys_alarm, seconds);
    WRAPPER_RETval(unsigned int);
}

int	pause(void) {
    long ret = _syscall(sys_pause);
    WRAPPER_RETval(int);
}

int	nanosleep(struct timespec *rqtp, struct timespec *rmtp) {
    long ret = _syscall(sys_nanosleep, rqtp, rmtp);
    WRAPPER_RETval(int);
}

unsigned int sleep(unsigned int seconds) {
    long ret;
    struct timespec req, rem;
    req.tv_sec = seconds;
    req.tv_nsec = 0;
    ret = _syscall(sys_nanosleep, &req, &rem);
    if(ret >= 0) return ret;
    if(ret == -EINTR) {
        return rem.tv_sec;
    }
    return 0;
}

int sigemptyset(sigset_t* set) {
    if (set == NULL) {
        errno = EINVAL;
        return -1;
    }
    memset(set, 0, sizeof(sigset_t));
    return 0;
}

int sigfillset(sigset_t* set) {
    if (set == NULL) {
        errno = EINVAL;
        return -1;
    }
    memset(set, 0xff, sizeof(sigset_t));
    return 0;
}

# define __sigmask(sig) (((unsigned long int) 1) << (((sig) - 1) % (8 * sizeof (unsigned long int))))
# define __sigword(sig) (((sig) - 1) / (8 * sizeof (unsigned long int)))

static inline bool __is_internal_signal (int sig) {
    return (sig == SIGCANCEL) || (sig == SIGSETXID);
}

int sigaddset (sigset_t *set, int signo) {
    if (set == NULL || signo <= 0 || signo >= _NSIG || __is_internal_signal (signo)) {
        errno = EINVAL;
        return -1;
    }
    unsigned long int __mask = __sigmask (signo);
    unsigned long int __word = __sigword (signo);
    (set)->__val[__word] |= __mask;
    return 0;
}

int sigismember(const sigset_t* set, int signo) {
    if (set == NULL || signo <= 0 || signo >= _NSIG) {
        errno = EINVAL;
        return -1;
    }
    unsigned long int __mask = __sigmask (signo);
    unsigned long int __word = __sigword (signo);  
    return (set)->__val[__word] & __mask ? 1 : 0;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oset) {
    long ret = _syscall(sys_rt_sigprocmask, how, set, oset, _NSIG / 8);
    WRAPPER_RETval(int);
}

int sigpending(sigset_t *set) {
    long ret = _syscall(sys_rt_sigpending, set, _NSIG / 8);
    WRAPPER_RETval(int);
}

extern void _sigret_rt();
# define STUB(act, sigsetsize) (sigsetsize)
int sigaction(int sig, const struct sigaction *act, struct sigaction *oact) {
    int ret;
    struct kernel_sigaction kact, koact;
    if(act) {
        kact.k_sa_handler = act->sa_handler;
        memcpy (&kact.sa_mask, &act->sa_mask, sizeof (sigset_t));
        kact.sa_flags = act->sa_flags;
        kact.sa_restorer = act->sa_restorer;
    }
    kact.sa_restorer = _sigret_rt;
    kact.sa_flags |= SA_RESTORER;
    ret = _syscall(sys_rt_sigaction, sig, act ? &kact : NULL, oact ? &koact : NULL, STUB(act, _NSIG / 8));
    if (oact && ret >= 0) {
        oact->sa_handler = koact.k_sa_handler;
        memcpy (&oact->sa_mask, &koact.sa_mask, sizeof (sigset_t));
        oact->sa_flags = koact.sa_flags;
        oact->sa_restorer = koact.sa_restorer;
    }
    WRAPPER_RETval(int);
}

sighandler_t signal(int signum, sighandler_t handler) {
    struct sigaction act, oact;
    if (handler == SIG_ERR || signum < 1 || signum >= _NSIG || __is_internal_signal (signum)) {
        errno = EINVAL;
        return SIG_ERR;
    }
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, signum);
    act.sa_flags = 0;
    if (sigaction(signum, &act, &oact) < 0)
        return SIG_ERR;
    return oact.sa_handler;
}

#define	PERRMSG_MIN	0
#define	PERRMSG_MAX	34

static const char *errmsg[] = {
	"Success",
	"Operation not permitted",
	"No such file or directory",
	"No such process",
	"Interrupted system call",
	"I/O error",
	"No such device or address",
	"Argument list too long",
	"Exec format error",
	"Bad file number",
	"No child processes",
	"Try again",
	"Out of memory",
	"Permission denied",
	"Bad address",
	"Block device required",
	"Device or resource busy",
	"File exists",
	"Cross-device link",
	"No such device",
	"Not a directory",
	"Is a directory",
	"Invalid argument",
	"File table overflow",
	"Too many open files",
	"Not a typewriter",
	"Text file busy",
	"File too large",
	"No space left on device",
	"Illegal seek",
	"Read-only file system",
	"Too many links",
	"Broken pipe",
	"Math argument out of domain of func",
	"Math result not representable"
};

void perror(const char *prefix) {
    const char *unknown = "Unknown";
    long backup = errno;
    if(prefix) {
        write(2, prefix, strlen(prefix));
        write(2, ": ", 2);
    }
    if(errno < PERRMSG_MIN || errno > PERRMSG_MAX) write(2, unknown, strlen(unknown));
    else write(2, errmsg[backup], strlen(errmsg[backup]));
    write(2, "\n", 1);
    return;
}
