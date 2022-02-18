#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/mman.h>
#include <dirent.h>
#include <signal.h>

#define FBLUE       "\033[34;"
#define NONE        "\033[0m"

void softKillOverride(int sig){
    return;
}

int main(){
    signal(SIGINT, softKillOverride);// CTRL+C
    signal(SIGTSTP, softKillOverride);// CTRL+Z
    signal(SIGQUIT, softKillOverride);/* CTRL+ \ */
    signal(SIGTERM, softKillOverride);// CTRL+U?
    char *input = (char*)mmap(NULL, sizeof(char*)*100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    
    int *c_status = (int*)mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    *c_status = 0;

    char path[1000];
    getcwd(path, 1000); // store cwd in path
    printf("Current Path: %s\n", path);

    DIR *dir;
  
    int pid = fork();
    int *c_pid = (int*)mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    int *p_pid = (int*)mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    int *msec = (int*)mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    *msec = 0; 
    int trigger = 10;

    if(!pid){
        *c_pid = getpid();
        while(1){
            // stat prog scanf line
            getcwd(path, 1000);
            printf("\033[0;34m");
            printf("Stat Prog ");
            printf("%s ", path);
            printf("\033[0m");
            printf("$ ");
            scanf("%s", input);
            *c_status += 1;
            // quit
            if(strcmp(input, "q") == 0){
                kill(*p_pid, 9);
                munmap(input, sizeof(char*)*100);
                munmap(c_status, sizeof(int));
                munmap(p_pid, sizeof(int));
                munmap(c_pid, sizeof(int));
                return 0;
            }
            // go up a directory
            else if(strcmp(input, "..") == 0){
                chdir("..");
            }
            // change directory by name ./name for down /name for up
            else if(input[0] == '/' ||(input[0]=='.'&&input[1]=='/')){
                if(chdir(input) !=0){
                    perror(__FUNCTION__);
                }
            }
            // if input == list, list files in the cw directory
            else if(strcmp(input, "list") == 0){
                // use readdir to print files in directory
                struct dirent *entry;
                if((dir = opendir(".")) == NULL){
                    perror("Cannot open .");
                    exit (1);
                }
                entry = (struct dirent *)readdir(dir);
                printf("\n --------------------------- \n");
                while(entry != NULL){
                    printf("%s\n", entry->d_name);
                    entry = (struct dirent *)readdir(dir);
                }
                printf(" --------------------------- \n");
            }
            // else (input == filename), list stats about file
            else{
                //get stats of whats in stat buffer and print
                struct stat sb;
                int ret = stat(input, &sb);
                if(ret!=0){
                    perror(__FUNCTION__);
                }
                else{
                printf("File type:                ");
                switch (sb.st_mode & S_IFMT) {
                case S_IFBLK:  printf("block device\n");            break;
                case S_IFCHR:  printf("character device\n");        break;
                case S_IFDIR:  printf("directory\n");               break;
                case S_IFIFO:  printf("FIFO/pipe\n");               break;
                case S_IFLNK:  printf("symlink\n");                 break;
                case S_IFREG:  printf("regular file\n");            break;
                case S_IFSOCK: printf("socket\n");                  break;
                default:       printf("unknown?\n");                break;
                }
                printf("I-node number:            %ld\n", (long) sb.st_ino);
                printf("Mode:                     %lo (octal)\n",(unsigned long) sb.st_mode);
                printf("Link count:               %ld\n", (long) sb.st_nlink);
                printf("Ownership:                UID=%ld   GID=%ld\n",(long) sb.st_uid, (long) sb.st_gid);
                printf("Preferred I/O block size: %ld bytes\n",(long) sb.st_blksize);
                printf("File size:                %lld bytes\n",(long long) sb.st_size);
                printf("Blocks allocated:         %lld\n",(long long) sb.st_blocks);
                printf("Last status change:       %s", ctime(&sb.st_ctime));
                printf("Last file access:         %s", ctime(&sb.st_atime));
                printf("Last file modification:   %s", ctime(&sb.st_mtime));
                }
            }
        }
    }
    else{
        *p_pid = getpid();
        int iterations_t1 = 0;
        int iterations_t2 = 0;
        iterations_t1 = *c_status;
        while(1){
            sleep(10);
            iterations_t2 = *c_status;
            // check how many iterations in 10 seconds
            if(iterations_t1 != iterations_t2){
                iterations_t1 = *c_status;
            }
            // if child hasn't done anything
            else{
                kill(*c_pid, 9);
                wait(0);
                munmap(input, sizeof(char*)*100);
                munmap(c_status, sizeof(int));
                munmap(c_pid, sizeof(int));
                munmap(p_pid, sizeof(int));
                return 0;
            }

        }
    }
    return 0;
}