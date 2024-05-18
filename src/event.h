

#ifndef UNTITLED1_EVENT_H
#define UNTITLED1_EVENT_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include "file_data.h"
void copy_file(const file_data_t *data);
void handle_event(const struct inotify_event *event, const char *destination_path, const char *user_path);

#endif //UNTITLED1_EVENT_H
