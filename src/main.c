#include "event.h"
#include "dir.h"
#include "ip.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <destination path> <path to other>\n", argv[0]);
        return 1;
    }

    char *destination_path = malloc(strlen(argv[1]) + 1);
    if (destination_path == NULL) {
        perror("malloc");
        return 1;
    }
    strcpy(destination_path, argv[1]);
//    if(!check_ip(&destination_path)){
//        perror("error");
//    }
    char *root = malloc(strlen(argv[2]) + 1);
    if (root == NULL) {
        perror("malloc");
        free(destination_path);
        return 1;
    }
    strcpy(root, argv[2]);

    char *last_path = explore_directory(root);
    if (last_path == NULL) {
        fprintf(stderr, "Error exploring directories\n");
        free(root);
        free(destination_path);
        return 1;
    }
    puts(last_path);

    int fd, wd;
    char buffer[BUFFER_SIZE];

    fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        free(root);
        free(destination_path);
        return 1;
    }

    wd = inotify_add_watch(fd, last_path, IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_TO);
    if (wd < 0) {
        perror("inotify_add_watch");
        close(fd);
        free(root);
        free(destination_path);
        return 1;
    }

    while (1) {
        ssize_t len = read(fd, buffer, BUFFER_SIZE);
        if (len < 0) {
            perror("read");
            close(fd);
            free(root);
            free(destination_path);
            return 1;
        }

        for (char *p = buffer; p < buffer + len;) {
            struct inotify_event *event = (struct inotify_event *) p;
            handle_event(event, destination_path, last_path);
            p += EVENT_SIZE + event->len;
        }
    }

    close(fd);
    free(root);
    free(destination_path);
    return 0;
}


//#include<stdio.h>
//#include <time.h>
//#include <unistd.h>
//
////
////int main() {
////
////    char name[50];
////    for(int i=0;i<10;i++){
////        sprintf(name,"file%d.jpg",i);
////        FILE *fp= fopen(name,"w");
////        if(fp==NULL){
////            perror("error");
////        }
////        fclose(fp);
////        sleep(5);
////    }
////    return 0;
////}