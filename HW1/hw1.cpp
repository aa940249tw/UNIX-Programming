#include <bits/stdc++.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <unistd.h>
#include <regex>

using namespace std;

unordered_map<int, string> username_table;

void eraseSubStr(std::string & mainStr, const std::string & toErase)
{
    // Search for the substring in string
    size_t pos = mainStr.find(toErase);
    if (pos != std::string::npos)
    {
        // If found then erase it from string
        mainStr.erase(pos, toErase.length());
    }
}

struct proc_info {
    string  path;
    string  cmd;
    pid_t   pid;
    string  user;
};

void print_maps(struct proc_info *p, long int init, regex FILES) {
    long int ino, lino = init;
    char filename[1024];
    string p_fd = "mem";

    FILE *maps = fopen((p->path + "maps").c_str(), "rb");
    if(!maps) return;
    while(fscanf(maps, "%*s %*s %*x %*s %ld %s\n", &ino, filename) == 2) {
        if(ino == 0 || ino == lino) continue;
        lino = ino;
        string p_type = "REG";
        smatch m;
        string tmp = string(filename);

        size_t pos = tmp.find("(deleted)");
        if (pos != std::string::npos) {
        // If found then erase it from string
            tmp.erase(pos, string("(deleted)").length());
            p_fd = "DEL";
        }

        if(!regex_search(tmp, m, FILES)) continue;

        printf("%-50s %-13d %-20s %-12s %-14s %-14ld %s\n", p->cmd.c_str(), p->pid, p->user.c_str(), p_fd.c_str(), p_type.c_str(), ino, filename);  
    }
    fclose(maps);
}

void print_fds(struct proc_info *p, string TYPE, regex FILES) {
    string fd_path = p->path + "fd";
    DIR *dir = opendir(fd_path.c_str());

    smatch m;

    if (dir == NULL) {
        fd_path += " (Permission denied)";
        if((TYPE.empty() || TYPE == "NOFD") && regex_search(fd_path, m, FILES))
            printf("%-50s %-13d %-20s %-12s %-14s %-14s %s\n", p->cmd.c_str(), p->pid, p->user.c_str(), "NOFD", " ", " ", fd_path.c_str());
        return;
    }

    struct dirent *dp;
    while((dp = readdir(dir)) != NULL) {
        if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) continue;

        string fds_path = fd_path + "/" + string(dp->d_name);

        struct stat s, s_link;
        string p_type;
        string p_fd = dp->d_name;
        if(!stat(fds_path.c_str(), &s)) {
            switch (s.st_mode & S_IFMT) {
                case S_IFCHR:  p_type = "CHR";      break;
                case S_IFDIR:  p_type = "DIR";      break;
                case S_IFIFO:  p_type = "FIFO";     break;
                case S_IFREG:  p_type = "REG";      break;
                case S_IFSOCK: p_type = "SOCK";     break;
                default:       p_type = "unknown";  break;
            }
        }
        else p_type = "unknown"; 
        if(!TYPE.empty() && TYPE != p_type) continue;

        ssize_t link_size = 0;
        char link_buf[1024];
        string p_readlink;
        if((link_size = readlink(fds_path.c_str(), link_buf, 1024)) < 0) {
            if(errno == EACCES) p_readlink = fds_path + " (Permission denied)";
            else return;
            if((TYPE.empty() || TYPE == "NOFD") && regex_search(p_readlink, m, FILES))
                printf("%-50s %-13d %-20s %-12s %-14s %-14s %s\n", p->cmd.c_str(), p->pid, p->user.c_str(), "NOFD", " ", " ", p_readlink.c_str());
        }
        else {
            link_buf[link_size] = '\0';
            p_readlink = string(link_buf);
        }
        eraseSubStr(p_readlink, "(deleted)");

        if(!regex_search(p_readlink, m, FILES)) continue;

        if(!lstat(fds_path.c_str(), &s_link)) {
            if((s_link.st_mode & S_IRUSR) && (s_link.st_mode & S_IWUSR)) p_fd += 'u';
            else if(s_link.st_mode & S_IRUSR) p_fd += 'r';
            else if(s_link.st_mode & S_IWUSR) p_fd += 'w';
        }

        printf("%-50s %-13d %-20s %-12s %-14s %-14ld %s\n", p->cmd.c_str(), p->pid, p->user.c_str(), p_fd.c_str(), p_type.c_str(), s.st_ino, p_readlink.c_str());
    }
}

long int print_type(const char *type, struct proc_info *p, string TYPE, regex FILES) {
    string tmp_path = p->path + string(type);
    smatch m;
    struct stat s;
    string p_type;
    if(!stat(tmp_path.c_str(), &s)) {
        switch (s.st_mode & S_IFMT) {
            case S_IFCHR:  p_type = "CHR";      break;
            case S_IFDIR:  p_type = "DIR";      break;
            case S_IFIFO:  p_type = "FIFO";     break;
            case S_IFREG:  p_type = "REG";      break;
            case S_IFSOCK: p_type = "SOCK";     break;
            default:       p_type = "unknown";  break;
        }
    }
    else p_type = "unknown"; 

    if(!TYPE.empty() && TYPE != p_type) return s.st_ino;

    ssize_t link_size = 0;
    char link_buf[1024];
    string p_readlink;
    string p_fd = string(type) == "cwd" ? "cwd" : (string(type) == "root" ? "rtd" : "txt"); 
    if((link_size = readlink(tmp_path.c_str(), link_buf, 1024)) < 0) {
        if(errno == EACCES) p_readlink = tmp_path + " (Permission denied)";
        else p_readlink = tmp_path;
        if((TYPE.empty() || TYPE == "unknown") && regex_search(p_readlink, m, FILES))
            printf("%-50s %-13d %-20s %-12s %-14s %-14s %s\n", p->cmd.c_str(), p->pid, p->user.c_str(), p_fd.c_str(), "unknown", " ", p_readlink.c_str());
        return -1;
    }
    else {
        link_buf[link_size] = '\0';
        p_readlink = string(link_buf);
        eraseSubStr(p_readlink, "(deleted)");
    }

    if(regex_search(p_readlink, m, FILES))
        printf("%-50s %-13d %-20s %-12s %-14s %-14ld %s\n", p->cmd.c_str(), p->pid, p->user.c_str(), p_fd.c_str(), p_type.c_str(), s.st_ino, p_readlink.c_str());    
    return s.st_ino;
}

void lsof_info(pid_t pid, regex COMM, string TYPE, regex FILES) {
    struct proc_info info;
    struct stat procstat;
    struct passwd *pw;

    info.pid = pid;
    info.path = "/proc/" + to_string(pid) + "/";



    if(!stat(info.path.c_str(), &procstat)) {
        if(username_table.find(procstat.st_uid) == username_table.end()) {
            pw = getpwuid(procstat.st_uid);
            username_table[procstat.st_uid] = string(pw->pw_name);
        }
        info.user = username_table[procstat.st_uid];
    }
    else return;

    FILE *cmdline = fopen((info.path + "comm").c_str(), "rb");
    char *arg = 0;
    size_t size = 0;
    ssize_t read = 0;
    if((read = getline(&arg, &size, cmdline)) != -1) {
        *(arg + read - 1) = '\0';
        info.cmd = string(arg);
    }
    free(arg);
    fclose(cmdline);

    smatch m;
    if(regex_search(info.cmd, m, COMM)) {
        print_type("cwd", &info, TYPE, FILES);
        print_type("root", &info, TYPE, FILES);
        long int ino_tmp = print_type("exe", &info, TYPE, FILES);
        if(TYPE.empty() || TYPE == "REG") print_maps(&info, ino_tmp, FILES);
        print_fds(&info, TYPE, FILES);
    }
}

int main(int argc, char *argv[]) {
    string COMM = "", TYPE = "", FILES = "";

    int i = 1;
    while(i < argc) {
        if(!strcmp(argv[i], "-c")) COMM = string(argv[i+1]);
        else if(!strcmp(argv[i], "-t")) TYPE = string(argv[i+1]);
        else if(!strcmp(argv[i], "-f")) FILES = string(argv[i+1]);
        i += 2;
    } 

    if(TYPE != "REG" && TYPE != "CHR" && TYPE != "DIR" && TYPE != "FIFO" && TYPE != "SOCK" && TYPE != "unknown" && TYPE != "") {
        cout << "Invalid TYPE option." << endl;
        return -1;
    }

    printf("%-50s %-13s %-20s %-12s %-14s %-14s %s\n", "COMMAND", "PID", "USER", "FD", "TYPE", "NODE", "NAME");

    regex COMM_reg(COMM);
    regex FILE_reg(FILES);

    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        fprintf(stderr, "%s: opendir(): %s: %s\n", argv[0], "/proc", strerror(errno));
        return errno;
    }
    struct dirent *dp;
    while((dp = readdir(dir)) != NULL) {
        if(strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
            long int pid = 0;
            char *endptr;
            pid = strtol(dp->d_name, &endptr, 10);
            if(*endptr == '\0') lsof_info(pid, COMM_reg, TYPE, FILE_reg);
        }
    }
}
