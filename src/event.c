#include "event.h"
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
void copy_file(const file_data_t *data) {
    char command[1024];
    snprintf(command,
             sizeof(command),
             "rsync -avzhlHlr -e \"ssh -i ~/.ssh/id_rsa\" %s/%s %s",
             data->root,
             data->filename,
             data->destination);

    int ret = system(command);
    if (ret == -1) {
        perror("system");
    } else {
        printf("Command executed with return code: %d\n", ret);
    }
}
