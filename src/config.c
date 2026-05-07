#include "config.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

int find_section(const char *name, Section *sections, int num_sections) {
    for (int i = 0; i < num_sections; i++) {
        if (strcmp(sections[i].name, name) == 0) return i;
    }
    return -1;
}

void parse_config(const char *path, Section *sections, int num_sections, int *logo_enabled, int *use_color) {
    FILE *f = fopen(path, "r");
    if (!f) return;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (line[0] == '#' || line[0] == '\0') continue;
        char sec[SECTION_NAME_LEN] = {0};
        char val[8] = {0};
        if (sscanf(line, "%31s = %7s", sec, val) < 2) continue;
        for (int i = 0; i < num_sections; i++) {
            if (strcmp(sections[i].name, sec) == 0) {
                if (strcmp(val, "no") == 0 || strcmp(val, "false") == 0 || strcmp(val, "0") == 0)
                    sections[i].enabled = 0;
                else if (strcmp(val, "yes") == 0 || strcmp(val, "true") == 0 || strcmp(val, "1") == 0)
                    sections[i].enabled = 1;
                break;
            }
        }
        if (strcmp(sec, "logo") == 0) {
            if (strcmp(val, "no") == 0 || strcmp(val, "false") == 0 || strcmp(val, "0") == 0)
                *logo_enabled = 0;
            else *logo_enabled = 1;
        }
        if (strcmp(sec, "color") == 0) {
            if (strcmp(val, "no") == 0 || strcmp(val, "false") == 0 || strcmp(val, "0") == 0)
                *use_color = 0;
            else *use_color = 1;
        }
    }
    fclose(f);
}

void gen_default_config(void) {
    printf("# StormFetch configuration\n");
    printf("# Format: section = yes/no\n\n");
    printf("logo = yes\n");
    printf("color = yes\n\n");
    printf("# Sections\n");
    printf("os = yes\n");
    printf("host = yes\n");
    printf("kernel = yes\n");
    printf("uptime = yes\n");
    printf("packages = yes\n");
    printf("shell = yes\n");
    printf("de-wm = yes\n");
    printf("terminal = yes\n");
    printf("cpu = yes\n");
    printf("gpu = yes\n");
    printf("memory = yes\n");
    printf("swap = yes\n");
    printf("disk = yes\n");
    printf("network = yes\n");
    printf("local-ip = yes\n");
    printf("public-ip = no\n");
    printf("processes = yes\n");
    printf("load-avg = yes\n");
    printf("battery = yes\n");
    printf("motherboard = yes\n");
    printf("bios = yes\n");
    printf("sound = yes\n");
    printf("resolution = yes\n");
    printf("temperature = yes\n");
    printf("users = yes\n");
    printf("locale = yes\n");
}
