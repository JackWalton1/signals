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
/*
void softKillOverride(int sig){
    fprintf(stderr, "\nSignal %d Detected! Better luck next time.\n", sig);
    fprintf(stderr, "\033[0;34m");
    fprintf(stderr, "Stat Prog ");
    fprintf(stderr,"\033[0m");
    fprintf(stderr, "$ ");
}
*/

int main(){
    //char input[1000];
    //signal(SIGINT, softKillOverride);// CTRL+C
    //signal(SIGTSTP, softKillOverride);// CTRL+Z
    //signal(SIGQUIT, softKillOverride);/* CTRL+ \ */
    //signal(SIGTERM, softKillOverride);// CTRL+U?
    char *input = (char*)mmap(NULL, sizeof(char*)*100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    char path[1000];
    DIR *dir;
    getcwd(path, 1000); // store cwd in path
    printf("Current Path: %s\n", path);
    
    int p_pid = getpid();
    printf("Parent PID: %d\n", p_pid);
    int pid = fork();
    if(pid>0){
        //wait(0);
        while(strcmp(input, "q") != 0 && pid > 0){
            wait(0);
            //wait(0);
            if(strcmp(input, "q") != 0){
                printf("\nChild Terminated... Re-paralleling \n");
                sleep(10);
                pid = fork();
            }
        }
        //return 0;
    }if (pid==0){
        while(1){
        // stat prog scanf line
            getcwd(path, 1000);
            printf("\033[0;34m");
            printf("Stat Prog ");
            printf("%s ", path);
            printf("\033[0m");
            printf("$ ");
            scanf("%s", input);
            // quit
            if(strcmp(input, "q") == 0){
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
    munmap(input, sizeof(char*)*100);
    return 0;
}