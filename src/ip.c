#include "ip.h"

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
