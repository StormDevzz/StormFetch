#include "info.h"
#include "logo.h"
#include "config.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int use_color = -1;
static int logo_enabled = 1;
static int bare = 0;
static int json = 0;
static int separator = 0;
static int do_sort = 0;

static Section sections[] = {
    {"os",           1, gen_os},
    {"host",         1, gen_host},
    {"kernel",       1, gen_kernel},
    {"uptime",       1, gen_uptime},
    {"packages",     1, gen_packages},
    {"shell",        1, gen_shell},
    {"de-wm",        1, gen_de_wm},
    {"terminal",     1, gen_terminal},
    {"cpu",          1, gen_cpu},
    {"gpu",          1, gen_gpu},
    {"memory",       1, gen_memory},
    {"swap",         1, gen_swap},
    {"disk",         1, gen_disk},
    {"network",      1, gen_network},
    {"local-ip",     1, gen_local_ip},
    {"public-ip",    0, gen_public_ip},
    {"processes",    1, gen_processes},
    {"load-avg",     1, gen_load_avg},
    {"battery",      1, gen_battery},
    {"motherboard",  1, gen_motherboard},
    {"bios",         1, gen_bios},
    {"sound",        1, gen_sound},
    {"resolution",   1, gen_resolution},
    {"temperature",  1, gen_temperature},
    {"users",        1, gen_users},
    {"locale",       1, gen_locale},
};

static int num_sections = sizeof(sections) / sizeof(sections[0]);

static int cmp_sections(const void *a, const void *b) {
    return strcmp(((const Section*)a)->name, ((const Section*)b)->name);
}

static void print_help(const char *prog) {
    printf("Usage: %s [OPTIONS]\n\n", prog);
    printf("StormFetch - system information tool\n\n");
    printf("Options:\n");
    printf("  -h, --help              Show this help\n");
    printf("  --list-sections         List all available sections\n");
    printf("  --no-<section>          Disable a section\n");
    printf("  --only-<section>        Show ONLY specified section (can be repeated)\n");
    printf("  --no-logo               Disable ASCII logo\n");
    printf("  --color <mode>          Color mode: always, never, auto (default)\n");
    printf("  --bare                  Minimal output (no header)\n");
    printf("  --json                  Output in JSON format\n");
    printf("  --separator             Add separator line\n");
    printf("  --sort                  Sort sections alphabetically\n");
    printf("  --gen-config            Generate default config to stdout\n");
    printf("  --config <file>         Use custom config file\n");
    printf("\nAvailable sections:");
    for (int i = 0; i < num_sections; i++) {
        printf("\n  %s", sections[i].name);
        if (!sections[i].enabled) printf(" (disabled by default)");
    }
    printf("\n  logo\n");
    printf("\nConfig file: ~/.config/stormfetch/config\n");
}

static void list_sections(void) {
    for (int i = 0; i < num_sections; i++) {
        printf("%s\n", sections[i].name);
    }
    printf("logo\n");
}

static void parse_color_arg(const char *arg) {
    if (strcmp(arg, "always") == 0) use_color = 1;
    else if (strcmp(arg, "never") == 0) use_color = 0;
    else if (strcmp(arg, "auto") == 0) use_color = -1;
}

static void parse_args(int argc, char **argv) {
    int only_mode = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            exit(0);
        }
        if (strcmp(argv[i], "--list-sections") == 0) {
            list_sections();
            exit(0);
        }
        if (strcmp(argv[i], "--no-logo") == 0) {
            logo_enabled = 0;
            continue;
        }
        if (strcmp(argv[i], "--bare") == 0) {
            bare = 1;
            continue;
        }
        if (strcmp(argv[i], "--json") == 0) {
            json = 1;
            continue;
        }
        if (strcmp(argv[i], "--separator") == 0) {
            separator = 1;
            continue;
        }
        if (strcmp(argv[i], "--sort") == 0) {
            do_sort = 1;
            continue;
        }
        if (strcmp(argv[i], "--gen-config") == 0) {
            gen_default_config();
            exit(0);
        }
        if (strcmp(argv[i], "--color") == 0) {
            if (i + 1 < argc) parse_color_arg(argv[++i]);
            continue;
        }
        if (strcmp(argv[i], "--config") == 0) {
            if (i + 1 < argc) {
                parse_config(argv[++i], sections, num_sections, &logo_enabled, &use_color);
            }
            continue;
        }
        if (strncmp(argv[i], "--no-", 5) == 0) {
            char *sec = argv[i] + 5;
            int idx = find_section(sec, sections, num_sections);
            if (idx >= 0) sections[idx].enabled = 0;
            continue;
        }
        if (strncmp(argv[i], "--only-", 7) == 0) {
            if (!only_mode) {
                for (int j = 0; j < num_sections; j++) sections[j].enabled = 0;
                only_mode = 1;
            }
            char *sec = argv[i] + 7;
            int idx = find_section(sec, sections, num_sections);
            if (idx >= 0) sections[idx].enabled = 1;
            continue;
        }
        fprintf(stderr, "Unknown option: %s\n", argv[i]);
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        exit(1);
    }
}

static int should_use_color(void) {
    if (use_color == 1) return 1;
    if (use_color == 0) return 0;
    return isatty(STDOUT_FILENO);
}

static int is_header_section(const char *name) {
    return strcmp(name, "os") == 0 || strcmp(name, "host") == 0;
}

static void print_json(void) {
    char *user = getenv("USER");
    if (!user) user = getenv("LOGNAME");
    if (!user) user = "unknown";
    char *host = gen_host();

    printf("{\n");
    printf("  \"user\": \"%s\",\n", user);
    printf("  \"host\": \"%s\",\n", host);
    printf("  \"os\": \"%s\"", gen_os());

    for (int i = 0; i < num_sections; i++) {
        if (!sections[i].enabled) continue;
        if (is_header_section(sections[i].name)) continue;
        char *val = sections[i].gen();
        if (val && strlen(val) > 0) {
            printf(",\n  \"%s\": \"%s\"", sections[i].name, val);
        }
    }
    printf("\n}\n");
}

static void print_normal(void) {
    int color = should_use_color();
    char *user = getenv("USER");
    if (!user) user = getenv("LOGNAME");
    if (!user) user = "unknown";
    char *host = gen_host();

    if (!bare) {
        printf("\n");
        if (color)
            printf("  \033[1m%s@%s\033[0m\n", user, host);
        else
            printf("  %s@%s\n", user, host);
        printf("  %s\n\n", gen_os());
    }

    if (logo_enabled) print_logo(color);

    if (separator && !bare) {
        if (color)
            printf("  \033[2m%s\033[0m\n", "------------------------------");
        else
            printf("  %s\n", "------------------------------");
    }

    if (do_sort) qsort(sections, num_sections, sizeof(Section), cmp_sections);

    for (int i = 0; i < num_sections; i++) {
        if (!sections[i].enabled) continue;
        char *val = sections[i].gen();
        if (val && strlen(val) > 0) {
            if (color)
                printf("  \033[1m%-12s\033[0m %s\n", sections[i].name, val);
            else
                printf("  %-12s %s\n", sections[i].name, val);
        }
    }
    printf("\n");
}

int main(int argc, char **argv) {
    char config_path[512] = {0};
    char *home = getenv("HOME");
    if (home) snprintf(config_path, sizeof(config_path), "%s/.config/stormfetch/config", home);

    if (config_path[0]) parse_config(config_path, sections, num_sections, &logo_enabled, &use_color);

    parse_args(argc, argv);

    if (json) print_json();
    else print_normal();

    return 0;
}
