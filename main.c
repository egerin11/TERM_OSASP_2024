#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUFFER_SIZE (1024 * (EVENT_SIZE + 16)) * 100
#define MAX_THREADS 10

typedef struct {
    char filename[256];
    char destination[256];
} thread_data_t;

int active_threads = 0;
pthread_mutex_t thread_lock = PTHREAD_MUTEX_INITIALIZER;

void *copy_file(void *arg) {
    thread_data_t *data = (thread_data_t *) arg;
    char command[512];
    snprintf(command, sizeof(command), "rsync -avzhlHl --ignore-missing-args /home/egerin/OSISP/test/%s %s", data->filename, data->destination);
    system(command);
    free(data);
    pthread_mutex_lock(&thread_lock);
    active_threads--;
    pthread_mutex_unlock(&thread_lock);
    return NULL;
}

void handle_event(const struct inotify_event *event, const char *destination_path) {
    printf("Event: %s\n", event->name);

    switch (event->mask) {
        case IN_CREATE:
        case IN_MOVED_TO:
            printf("File created: %s\n", event->name);
            pthread_mutex_lock(&thread_lock);
            if (active_threads < MAX_THREADS) {
                active_threads++;
                pthread_mutex_unlock(&thread_lock);
                thread_data_t *data = malloc(sizeof(thread_data_t));
                strcpy(data->filename, event->name);
                strcpy(data->destination, destination_path);
                pthread_t thread;
                if (pthread_create(&thread, NULL, copy_file, data) != 0) {
                    perror("pthread_create");
                    free(data);
                    pthread_mutex_lock(&thread_lock);
                    active_threads--;
                    pthread_mutex_unlock(&thread_lock);
                }
            } else {
                pthread_mutex_unlock(&thread_lock);
                printf("Maximum number of threads reached, skipping file: %s\n", event->name);
            }
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <destination path>\n", argv[0]);
        return 1;
    }

    char destination_path[256];
    strcpy(destination_path, argv[1]);

    int fd, wd;
    char buffer[BUFFER_SIZE];

    fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        return 1;
    }

    wd = inotify_add_watch(fd, "/home/egerin/OSISP/test", IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_TO);    if (wd < 0) {
        perror("inotify_add_watch");
        close(fd);
        return 1;
    }

    while (1) {
        ssize_t len = read(fd, buffer, BUFFER_SIZE);
        if (len < 0) {
            perror("read");
            close(fd);
            return 1;
        }

        for (char *p = buffer; p < buffer + len;) {
            struct inotify_event *event = (struct inotify_event *) p;
            handle_event(event, destination_path);
            p += EVENT_SIZE + event->len;
        }
    }

    close(fd);
    return 0;
}