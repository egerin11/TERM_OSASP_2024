#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUFFER_SIZE (1024 * (EVENT_SIZE + 16)) * 100

typedef struct {
    char root[512];
    char filename[256];
    char destination[256];
} file_data_t;

void copy_file(const file_data_t *data) {
    char command[512];
    snprintf(command,
             sizeof(command),
             "rsync -avzhlHlr -e \"ssh\" %s/%s %s",
             data->root,
             data->filename,
             data->destination);

    system(command);
}

char *memory(const int capacity) {
    char *array = (char *) malloc(capacity * sizeof(char));
    return array;
}

int is_valid_ip(const char *ip_adr) {
    int dots = 0;
    int len = (int) strlen(ip_adr);
    if (len < 7 || len > 15)
        return 0;
    for (int i = 0; i < len; i++) {
        if (!isdigit(ip_adr[i]) && ip_adr[i] != '.')
            return 0;
        if (ip_adr[i] == '.')
            dots++;
    }
    if (dots != 3)
        return 0;
    char *token;
    char *ptr;
    char temp_ip[strlen(ip_adr) + 1];
    strcpy(temp_ip, ip_adr);
    token = strtok(temp_ip, ".");
    while (token != NULL) {
        long num = strtol(token, &ptr, 10);
        if (num < 0 || num > 255)
            return 0;
        if (*ptr)
            return 0;
        token = strtok(NULL, ".");
    }

    return 1;
}

bool check_ip(char **string) {
    if (strlen(*string) == 0) {
        fprintf(stderr, "size < 0");
        return false;
    }

    char *token = strtok(*string, "@");
    if (token == NULL) {
        fprintf(stderr, "Invalid format: missing '@'");
        return false;
    }

    token = strtok(NULL, ":");
    if (token == NULL) {
        fprintf(stderr, "Invalid format: missing ':'");
        return false;
    }

    if (!is_valid_ip(token)) {
        return false;
    }

    return true;
}

int print_files(char *path) {
    int count_file = 0;
    DIR *dir;
    struct dirent *entry;
    char new_path[1024];

    dir = opendir(path);
    if (dir == NULL) {
        printf("error read file %s\n", path);
        return 0;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_REG) {
            printf("file: %s\n", new_path);
            count_file++;
        }
    }

    closedir(dir);

    return count_file;
}

//void explore_directory(char *path) {
//    DIR *dir;
//    struct dirent *entry;
//    char new_path[1024];
//
//    dir = opendir(path);
//    if (dir == NULL) {
//        printf("error open dir: %s\n", path);
//        return;
//    }
//
//    while ((entry = readdir(dir)) != NULL) {
//        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
//            continue;
//
//        snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);
//
//        if (entry->d_type == DT_DIR) {
//            if (strcmp(entry->d_name, "tmp") == 0) {
//                printf("dir: %s\n", new_path);
////                explore_directory(new_path);
////                *previous_path = strdup(path);
////                if (*previous_path == NULL) {
////                    perror("strdup");
////                    closedir(dir);
////                    return;
//            }
//        }
//    else if (entry->d_type != DT_REG) {
//        printf("unknown: %s\n", new_path);
//    }
////            } else {
////                printf("dir: %s\n", new_path);
////                int count_files_tmp = print_files(new_path);
////                printf("files in dir %s: %d\n", new_path, count_files_tmp);
////                explore_directory(new_path, previous_path);
////            }
//
//}
//
//closedir(dir);
//}
//void explore_directory(char *path, char **previous_path) {
//    DIR *dir;
//    struct dirent *entry;
//    char new_path[1024];
//
//    dir = opendir(path);
//    if (dir == NULL) {
//        printf("error open dir: %s\n", path);
//        return;
//    }
//
//    while ((entry = readdir(dir)) != NULL) {
//        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
//            continue;
//
//        snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);
//
//        if (entry->d_type == DT_DIR) {
//            if (strcmp(entry->d_name, "tmp") == 0) {
//                printf("dir: %s\n", new_path);
//                *previous_path = strdup(path);
//                if (*previous_path == NULL) {
//                    perror("strdup");
//                    closedir(dir);
//                    return;
//                }
//            } else {
//                printf("dir: %s\n", new_path);
//                int count_files_tmp = print_files(new_path);
//                printf("files in dir %s: %d\n", new_path, count_files_tmp);
//                explore_directory(new_path, previous_path);
//            }
//        } else if (entry->d_type != DT_REG) {
//            printf("unknown: %s\n", new_path);
//        }
//    }
//
//    closedir(dir);
//}
char* explore_directory(char *path) {
    DIR *dir;
    struct dirent *entry;
    char new_path[1024];

    dir = opendir(path);
    if (dir == NULL) {
        printf("error open dir: %s\n", path);
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            char *temp_path = explore_directory(new_path);
            if (temp_path != NULL) {
                closedir(dir);
                return temp_path;
            }
        }
    }

    closedir(dir);
    return path;
}
void handle_event(const struct inotify_event *event, const char *destination_path, const char *user_path) {
    printf("Event: %s\n", event->name);
    file_data_t *data = malloc(sizeof(file_data_t));
    if (data == NULL) {
        perror("malloc");
        return;
    }
    strcpy(data->root, user_path);
    strcpy(data->destination, destination_path);
    strcpy(data->filename, event->name);

    switch (event->mask) {
        case IN_CREATE:
        case IN_MOVED_TO:
            printf("File created: %s\n", event->name);
            copy_file(data);
            break;
        case IN_DELETE:
            printf("File deleted: %s\n", event->name);
            break;
        case IN_MODIFY:
            printf("File modified: %s\n", event->name);
            break;
        default:
            printf("Unknown event: %s\n", event->name);
            copy_file(data);
            break;
    }
    free(data);
}

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
