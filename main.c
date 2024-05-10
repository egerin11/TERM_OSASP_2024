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
             "rsync -avzhlHlr %s/%s %s",
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
        printf("Ошибка открытия каталога: %s\n", path);
        return 0;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_REG) {
            printf("Файл: %s\n", new_path);
            count_file++;
        }
    }

    closedir(dir);

    return count_file;
}

void explore_directory(char *path, char **previous_path) {
    DIR *dir;
    struct dirent *entry;
    char *new_path;

    dir = opendir(path);
    if (dir == NULL) {
        printf("Ошибка открытия каталога: %s\n", path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        asprintf(&new_path, "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, "tmp") == 0) {
                printf("Каталог: %s\n", new_path);
                *previous_path = strdup(path);
            } else {
                printf("Каталог: %s\n", new_path);
                int count_files_tmp = print_files(new_path);
                printf("Всего файлов в каталоге %s: %d\n", new_path, count_files_tmp);
                explore_directory(new_path, previous_path);
            }
        } else if (entry->d_type != DT_REG) {
            printf("Неизвестный тип: %s\n", new_path);
        }

        free(new_path);
    }

    closedir(dir);
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
        fprintf(stderr, "Usage: %s <destination path> <path to other >\n", argv[0]);
        return 1;
    }

    char *destination_path = NULL;
    char *root = NULL;
    char *last_path = NULL;
    asprintf(&destination_path, "%s", argv[1]);
    asprintf(&root, "%s", argv[2]);
    puts(destination_path);
    puts(root);
    explore_directory(root, &last_path);
    puts(destination_path);
    puts(root);
    puts(last_path);
    //    if (!check_ip(&root)) {
    //        return 1;
    //    }

    int fd, wd, wd2;
    char buffer[BUFFER_SIZE];

    fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        return 1;
    }

    char *new_path = strdup(last_path);
    strcat(new_path, "/tmp");
    puts(new_path);
    puts(last_path);

    wd = inotify_add_watch(fd, new_path, IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_TO);

    if (wd < 0) {
        perror("inotify_add_watch");
        close(fd);
        free(new_path);
        return 1;
    }

    wd2 = inotify_add_watch(fd, last_path, IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_TO);
    if (wd2 < 0) {
        perror("inotify_add_watch");
        close(fd);
        free(new_path);
        return 1;
    }


    int max_files_in_last_path = print_files(last_path);

    while (1) {
        ssize_t len = read(fd, buffer, BUFFER_SIZE);
        if (len < 0) {
            perror("read");
            close(fd);
            return 1;
        }

        for (char *p = buffer; p < buffer + len;) {
            struct inotify_event *event = (struct inotify_event *) p;

            if (event->wd == wd2) {
                handle_event(event, destination_path, last_path);

                if (event->mask == IN_CREATE || event->mask == IN_MOVED_TO) {
                    max_files_in_last_path++;
                    if (max_files_in_last_path >=1) {
                        inotify_rm_watch(fd, wd);
                    }
                }
            } else {
                handle_event(event, destination_path, new_path);
            }

            p += EVENT_SIZE + event->len;
        }
    }

    close(fd);
    return 0;
}

