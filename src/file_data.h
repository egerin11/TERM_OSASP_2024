
#ifndef UNTITLED1_FILE_DATA_H
#define UNTITLED1_FILE_DATA_H
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUFFER_SIZE (1024 * (EVENT_SIZE + 16)) * 100

typedef struct {
    char root[512];
    char filename[256];
    char destination[256];
} file_data_t;

#endif //UNTITLED1_FILE_DATA_H
