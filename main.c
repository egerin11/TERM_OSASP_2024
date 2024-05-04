#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUFFER_SIZE (1024 * (EVENT_SIZE + 16))

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <IP address> <destination path>\n", argv[0]);
        return 1;
    }

//    char ip_address[256];
    char destination_path[256];
//    strcpy(ip_address, argv[1]);
    strcpy(destination_path, argv[1]);

    int fd, wd;
    struct inotify_event *event;
    char buffer[BUFFER_SIZE];

    fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        return 1;
    }

    wd = inotify_add_watch(fd, "/home/egerin/OSISP/test", IN_CREATE | IN_DELETE | IN_MODIFY);
    if (wd < 0) {
        perror("inotify_add_watch");
        return 1;
    }

    while (1) {
        ssize_t len = read(fd, buffer, BUFFER_SIZE);
        if (len < 0) {
            perror("read");
            return 1;
        }

        for (char *p = buffer; p < buffer + len;) {
            event = (struct inotify_event *) p;
            p += EVENT_SIZE;
            printf("Event: %s\n", event->name);

            switch (event->mask) {
                case IN_CREATE:
                    printf("File created: %s\n", event->name);
                    char command[512];
                    snprintf(command, sizeof(command), "rsync -avzhlHl /home/egerin/OSISP/test/%s %s",
                             event->name,  destination_path);
                    system(command);
                    break;
                case IN_DELETE:
                    printf("File deleted: %s\n", event->name);
                    break;
                case IN_MODIFY:
                    printf("File modified: %s\n", event->name);
                    break;
                default:
                    printf("Unknown event: %s\n", event->name);
                    break;
            }
        }
    }

    close(fd);
    return 0;
}
