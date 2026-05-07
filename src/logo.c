#include "logo.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

static const char *get_os_id(void) {
    static char osid[64] = {0};
    if (osid[0]) return osid;

    char *id = read_value_from_proc("/etc/os-release", "ID=");
    if (id) {
        if (id[0] == '"') { memmove(id, id+1, strlen(id)); char *e = strrchr(id, '"'); if (e) *e = '\0'; }
        strncpy(osid, id, sizeof(osid)-1);
        for (char *p = osid; *p; p++) *p = tolower(*p);
    } else if (access("/etc/debian_version", F_OK) == 0) strcpy(osid, "debian");
    else if (access("/etc/arch-release", F_OK) == 0) strcpy(osid, "arch");
    else if (access("/etc/fedora-release", F_OK) == 0) strcpy(osid, "fedora");
    else strcpy(osid, "linux");
    return osid;
}

void print_logo(int use_color) {
    const char *os = get_os_id();
    const char *color = "";

    if (use_color) {
        if (strcmp(os, "arch") == 0) color = "\033[36m";
        else if (strcmp(os, "debian") == 0) color = "\033[31m";
        else if (strcmp(os, "ubuntu") == 0) color = "\033[31m";
        else if (strcmp(os, "fedora") == 0) color = "\033[34m";
        else if (strcmp(os, "void") == 0) color = "\033[32m";
        else if (strcmp(os, "gentoo") == 0) color = "\033[35m";
        else if (strcmp(os, "alpine") == 0) color = "\033[34m";
        else if (strcmp(os, "manjaro") == 0) color = "\033[32m";
        else if (strcmp(os, "mint") == 0) color = "\033[32m";
        else if (strcmp(os, "freebsd") == 0) color = "\033[31m";
        else if (strcmp(os, "pop") == 0) color = "\033[33m";
        else color = "\033[33m";
    }

    if (use_color) printf("%s", color);

    if (strcmp(os, "arch") == 0) {
        printf("       /\\\n");
        printf("      /  \\\n");
        printf("     /\\   \\\n");
        printf("    /      \\\n");
        printf("   /   ,,   \\\n");
        printf("  /   |  |  \\\n");
        printf(" /_-''    ''-_\\\n");
    } else if (strcmp(os, "debian") == 0) {
        printf("   _,met$$$$$$gg.\n");
        printf(" ,g$$$$$$$$$$$$$$P.\n");
        printf(",g$$P\"\"       \"\"\"Y$.\"\n");
        printf(",$$P'              `$$$.\n");
        printf("',$$P       ,ggs.     `$$b\n");
        printf("`d$$'     ,$P\"'   .    $$$\n");
        printf(" $$P      d$'     ,    $$P\n");
        printf(" $$:      $$.   -    ,d$$'\n");
        printf(" $$;      Y$b._   _,d$P'\n");
        printf(" Y$$.    `.`\"Y$$$$P\"'\n");
    } else if (strcmp(os, "ubuntu") == 0) {
        printf("         .-.\n");
        printf("        /   \\\n");
        printf("       |     |\n");
        printf("       |     |\n");
        printf("        \\   /\n");
        printf("         `-'\n");
        printf("      _   _   _\n");
        printf("    _| |_| |_| |_\n");
        printf("   |               |\n");
        printf("   |    Ubuntu     |\n");
    } else if (strcmp(os, "fedora") == 0) {
        printf("       _____\n");
        printf("      /   __|.\n");
        printf("     |  /    |\n");
        printf("     | |     |\n");
        printf("     |  \\___/|\n");
        printf("     |       |\n");
        printf("     |   |   |\n");
        printf("     |   |   |\n");
        printf("     |___|___|\n");
    } else if (strcmp(os, "void") == 0) {
        printf("       ______\n");
        printf("      /      \\\n");
        printf("     |  () () |\n");
        printf("      \\  __  /\n");
        printf("       |    |\n");
        printf("       |    |\n");
        printf("       |____|\n");
    } else if (strcmp(os, "gentoo") == 0) {
        printf("        _-----_\n");
        printf("       /       \\\n");
        printf("      |  O   O  |\n");
        printf("      |    _    |\n");
        printf("       \\  ---  /\n");
        printf("        \\_____/\n");
    } else if (strcmp(os, "alpine") == 0) {
        printf("       /\\ /\\\n");
        printf("      /  \\ /  \\\n");
        printf("     /    /\\    \\\n");
        printf("    /    /  \\    \\\n");
        printf("   /    /    \\    \\\n");
        printf("  /    /      \\    \\\n");
        printf(" /____/        \\____\\\n");
    } else if (strcmp(os, "manjaro") == 0) {
        printf(" ██████████████████\n");
        printf(" ██████████████████\n");
        printf(" ██████████████████\n");
        printf(" ██████████████████\n");
        printf(" ██████████████████\n");
        printf(" ██████████████████\n");
    } else if (strcmp(os, "mint") == 0) {
        printf(" _______________\n");
        printf("|  ___________  |\n");
        printf("| |           | |\n");
        printf("| |  LINUX    | |\n");
        printf("| |   MINT    | |\n");
        printf("| |___________| |\n");
        printf("|_______________|\n");
    } else if (strcmp(os, "freebsd") == 0) {
        printf("  ,        ,\n");
        printf("  |\\      /|\n");
        printf("  | \\    / |\n");
        printf("  |  \\  /  |\n");
        printf("  |   \\/   |\n");
        printf("  |        |\n");
        printf("  |        |\n");
        printf("  ----------\n");
    } else if (strcmp(os, "pop") == 0) {
        printf("            .\n");
        printf("           / \\\n");
        printf("          /   \\\n");
        printf("         /  .  \\\n");
        printf("        /  /\\  \\\n");
        printf("       /  /  \\  \\\n");
        printf("      /  /    \\  \\\n");
        printf("     /  /      \\  \\\n");
        printf("    /  /        \\  \\\n");
        printf("   /  /          \\  \\\n");
        printf("  /__/            \\__\\\n");
    } else {
        printf("  +------------------+\n");
        printf("  |                  |\n");
        printf("  |    %-12s   |\n", os);
        printf("  |                  |\n");
        printf("  +------------------+\n");
    }
    if (use_color) printf("\033[0m");
}
