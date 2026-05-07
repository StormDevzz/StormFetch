#ifndef CONFIG_H
#define CONFIG_H

#define SECTION_NAME_LEN 32
#define MAX_SECTIONS 64

typedef struct {
    char name[SECTION_NAME_LEN];
    int enabled;
    char *(*gen)(void);
} Section;

int  find_section(const char *name, Section *sections, int num_sections);
void parse_config(const char *path, Section *sections, int num_sections, int *logo_enabled, int *use_color);
void gen_default_config(void);

#endif
