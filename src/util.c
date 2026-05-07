#include "util.h"

void trim_newline(char *s) {
    size_t l = strlen(s);
    while (l > 0 && (s[l-1] == '\n' || s[l-1] == '\r')) s[--l] = '\0';
}

char *read_first_line(const char *path) {
    static char buf[MAX_LINE];
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    if (!fgets(buf, sizeof(buf), f)) { fclose(f); return NULL; }
    fclose(f);
    trim_newline(buf);
    return buf;
}

char *read_value_from_proc(const char *path, const char *prefix) {
    static char buf[MAX_LINE];
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    while (fgets(buf, sizeof(buf), f)) {
        if (strncmp(buf, prefix, strlen(prefix)) == 0) {
            char *val = buf + strlen(prefix);
            while (*val == ' ' || *val == '\t' || *val == ':') val++;
            trim_newline(val);
            fclose(f);
            return val;
        }
    }
    fclose(f);
    return NULL;
}

char *exec_cmd(const char *cmd) {
    static char buf[MAX_LINE];
    FILE *f = popen(cmd, "r");
    if (!f) return NULL;
    if (!fgets(buf, sizeof(buf), f)) { pclose(f); return NULL; }
    pclose(f);
    trim_newline(buf);
    return buf;
}

int count_digits(int n) {
    if (n == 0) return 1;
    int c = 0;
    while (n) { n /= 10; c++; }
    return c;
}
