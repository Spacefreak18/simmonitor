#include "dirhelper.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>

#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <stddef.h>

#include "../slog/src/slog.h"

void create_dir(char* dir)
{
    struct stat st = {0};
    if (stat(dir, &st) == -1)
    {
        mkdir(dir, 0700);
    }
}

void create_xdg_dir(const char* dir)
{
    struct stat st = {0};
    if (stat(dir, &st) == -1)
    {
        mkdir(dir, 0700);
    }
}

char* create_user_dir(char* home_dir_str, const char* dirtype, const char* programname)
{
    // +3 for slashes
    size_t ss = (4 + strlen(home_dir_str) + strlen(dirtype) + strlen(programname));
    char* config_dir_str = malloc(ss);

    snprintf (config_dir_str, ss, "%s/%s/%s/", home_dir_str, dirtype, programname);

    slogt("creating dir for %s if necessary", config_dir_str);
    create_dir(config_dir_str);
    return config_dir_str;
}


char* gethome()
{
    char* homedir = getenv("HOME");
    return homedir;

    if (homedir != NULL)
    {
        printf("Home dir in enviroment");
        printf("%s\n", homedir);
    }

    uid_t uid = getuid();
    struct passwd* pw = getpwuid(uid);

    if (pw == NULL)
    {
        printf("Failed\n");
        exit(EXIT_FAILURE);
    }

    return pw->pw_dir;
}

time_t get_file_creation_time(char* path)
{
    struct stat attr;
    stat(path, &attr);
    return attr.st_mtime;
}

void delete_dir(char* path)
{

    struct dirent* de;
    DIR* dr = opendir(path);

    if (dr == NULL)
    {
        printf("Could not open current directory");
    }

    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    while ((de = readdir(dr)) != NULL)
    {
        char* fullpath = ( char* ) malloc(1 + strlen(path) + strlen("/") + strlen(de->d_name));
        strcpy(fullpath, path);
        strcat(fullpath, "/");
        strcat(fullpath, de->d_name);
        unlink(fullpath);
        free(fullpath);
    }
    closedir(dr);
    rmdir(path);

}

void delete_oldest_dir(char* path)
{
    char* oldestdir = path;

    struct dirent* de;
    DIR* dr = opendir(path);

    if (dr == NULL)
    {
        printf("Could not open current directory");
    }

    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    char filename_qfd[100] ;
    char* deletepath = NULL;
    time_t tempoldest = 0;
    while ((de = readdir(dr)) != NULL)
    {
        struct stat stbuf;

        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
        {
            continue;
        }

        char* fullpath = ( char* ) malloc(1 + strlen(path) + strlen(de->d_name));
        strcpy(fullpath, path);
        strcat(fullpath, de->d_name);

        stat(fullpath, &stbuf);
        if (S_ISDIR(stbuf.st_mode))
        {
            strcpy(fullpath, path);
            strcat(fullpath, de->d_name);
            if (tempoldest == 0)
            {
                tempoldest = get_file_creation_time(fullpath);
                free(deletepath);
                deletepath = strdup(fullpath);
            }
            else
            {
                time_t t = get_file_creation_time(fullpath);
                double diff = tempoldest - t;
                if (diff > 0)
                {
                    tempoldest = t;
                    free(deletepath);
                    deletepath = strdup(fullpath);
                }
            }

        }

        free(fullpath);
    }
    closedir(dr);
    delete_dir(deletepath);
    free(deletepath);
}

void restrict_folders_to_cache(char* path, int cachesize)
{
    int numfolders = 0;

    struct dirent* de;
    DIR* dr = opendir(path);

    if (dr == NULL)
    {
        printf("Could not open current directory");
    }

    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    while ((de = readdir(dr)) != NULL)
    {

        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
        {
            continue;
        }

        char* fullpath = ( char* ) malloc(1 + strlen(path) + strlen(de->d_name));
        strcpy(fullpath, path);
        strcat(fullpath, de->d_name);
        strcat(fullpath, "/");

        struct stat stbuf;
        stat(fullpath,&stbuf);

        if (S_ISDIR(stbuf.st_mode))
        {
            numfolders++;
        }
        free(fullpath);
    }

    while (numfolders >= cachesize)
    {
        delete_oldest_dir(path);
        numfolders--;
    }
    closedir(dr);

}

bool does_directory_exist(char* path)
{
    DIR* dir = opendir(path);
    if (dir)
    {
        // Directory exists
        closedir(dir);
        return true;
    }
    else
    {
        // Directory does not exist or cannot be opened
        return false;
    }
}

bool does_file_exist(const char* file)
{
    if (file == NULL)
    {
        return false;
    }
#if defined(OS_WIN)
#if defined(WIN_API)
    // if you want the WinAPI, versus CRT
    if (strnlen(file, MAX_PATH+1) > MAX_PATH)
    {
        // ... throw error here or ...
        return false;
    }
    DWORD res = GetFileAttributesA(file);
    return (res != INVALID_FILE_ATTRIBUTES &&
            !(res& FILE_ATTRIBUTE_DIRECTORY));
#else
    // Use Win CRT
    struct stat fi;
    if (_stat(file, &fi) == 0)
    {
#if defined(S_ISSOCK)
        // sockets come back as a 'file' on some systems
        // so make sure it's not a socket or directory
        // (in other words, make sure it's an actual file)
        return !(S_ISDIR(fi.st_mode)) &&
               !(S_ISSOCK(fi.st_mode));
#else
        return !(S_ISDIR(fi.st_mode));
#endif
    }
    return false;
#endif
#else
    struct stat fi;
    if (stat(file, &fi) == 0)
    {
#if defined(S_ISSOCK)
        return !(S_ISDIR(fi.st_mode)) &&
               !(S_ISSOCK(fi.st_mode));
#else
        return !(S_ISDIR(fi.st_mode));
#endif
    }
    return false;
#endif
}

// this is lazy as it is unix only
// and stolen from https://gist.github.com/FreeSlave/e55e1c94439162d479a390cb28620203
int findExecutable(const char* baseName, char* buf, size_t size)
{
    char* envPath;
    char* part;
    size_t length;
    size_t baseNameLength;
    size_t needTrailingSlash;

    if (!baseName || !baseName[0]) {
        return 0;
    }

    envPath = getenv("PATH");
    if (!envPath) {
        return 0;
    }

    baseNameLength = strlen(baseName);
    while(*envPath) {
        part = strchr(envPath, ':');
        if (part) {
            length = part - envPath;
        } else {
            length = strlen(envPath);
        }

        if (length > 0) {
            needTrailingSlash = (envPath[length-1] == '/') ? 0 : 1;
            if (length + baseNameLength + needTrailingSlash < size) {
                strncpy(buf, envPath, length);
                strncpy(buf + length, "/", needTrailingSlash);
                strncpy(buf + length + needTrailingSlash, baseName, baseNameLength);
                buf[length + needTrailingSlash + baseNameLength] = '\0';

                if (access(buf, X_OK) == 0) {
                    return 1;
                }
            }
        }

        envPath += length;
        if (*envPath == ':') {
            envPath++;
        }
    }
    return 0;
}

void execProcess(const char* executable, char* const args[])
{
    //close all file descriptors
    //Possible optimization: use poll and close only valid descriptors.
    struct rlimit r;
    if (getrlimit(RLIMIT_NOFILE, &r) != 0) {
        int maxDescriptors = (int)r.rlim_cur;
        int i;
        for (i=0; i<maxDescriptors; ++i) {
            close(i);
        }
    }

    //set standard streams to /dev/null
    int devNull = open("/dev/null", O_RDWR);
    if (devNull > 0) {
        dup2(devNull, STDIN_FILENO);
        dup2(devNull, STDOUT_FILENO);
        dup2(devNull, STDERR_FILENO);
    }
    close(devNull);

    //detach from process group of parent
    setsid();

    execv(executable, args);
    _exit(1);
}

int openFile(char* path)
{
    char xdgOpen[256];
    if (findExecutable("xdg-open", xdgOpen, sizeof(xdgOpen))) {
        char* argv[] = {xdgOpen, path, NULL};
        pid_t pid = fork();
        if (pid == 0) {
            pid_t doubleFork = fork();
            if (doubleFork == 0) {
                execProcess(xdgOpen, argv);
            } else if (doubleFork > 0) {
                _exit(0);
            } else {
                _exit(1);
            }
        } else if (pid < 0) {
            perror("Could not fork");
            return 1;
        } else {
            int status;
            //wait for the first fork
            waitpid(pid, &status, 0);
            return status;
        }
    } else {
        fprintf(stderr, "Could not find xdg-open utility\n");
        return 1;
    }
    return 0;
}
