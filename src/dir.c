#include "dir.h"

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


char* explore_directory(char *path) {
    DIR *dir;
    struct dirent *entry;
    char new_path[1024];

    dir = opendir(path);
    if (dir == NULL) {
        printf("error open dir: %s\n", path);
        return NULL;
    }
    print_files(path);

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
