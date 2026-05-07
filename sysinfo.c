#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/utsname.h>

#define MAX_LINE 256

static void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len-1] == '\n') s[len-1] = '\0';
}

static int file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

static char *read_line_from_file(const char *path, const char *prefix) {
    static char buf[MAX_LINE];
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    while (fgets(buf, sizeof(buf), f)) {
        if (prefix && strncmp(buf, prefix, strlen(prefix)) == 0) {
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

static void strip_quotes(char *s) {
    size_t len = strlen(s);
    if (len == 0) return;
    if (s[0] == '"' || s[0] == '\'') {
        char q = s[0];
        memmove(s, s+1, len);
        char *end = strrchr(s, q);
        if (end) *end = '\0';
    }
}

static char *get_os_name(void) {
    static char os[64] = {0};
    char *id = read_line_from_file("/etc/os-release", "ID=");
    if (id) {
        strip_quotes(id);
        strncpy(os, id, sizeof(os)-1);
        for (char *p = os; *p; p++) *p = tolower(*p);
    } else if (file_exists("/etc/debian_version")) {
        strcpy(os, "debian");
    } else if (file_exists("/etc/arch-release")) {
        strcpy(os, "arch");
    } else if (file_exists("/etc/fedora-release")) {
        strcpy(os, "fedora");
    }
    return os;
}

static char *get_pretty_name(void) {
    static char name[256];
    char *pretty = read_line_from_file("/etc/os-release", "PRETTY_NAME=");
    if (pretty) {
        strip_quotes(pretty);
        strncpy(name, pretty, sizeof(name)-1);
        return name;
    }
    struct utsname u;
    uname(&u);
    snprintf(name, sizeof(name), "%s %s", u.sysname, u.release);
    return name;
}

static char *get_hostname(void) {
    static char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) return hostname;
    return "unknown";
}

static char *get_kernel(void) {
    static char kernel[256];
    struct utsname u;
    uname(&u);
    snprintf(kernel, sizeof(kernel), "%s %s", u.sysname, u.release);
    return kernel;
}

static char *get_cpu(void) {
    static char cpu[256];
    char *model = read_line_from_file("/proc/cpuinfo", "model name");
    if (model) {
        strncpy(cpu, model, sizeof(cpu)-1);
        char *src = cpu, *dst = cpu;
        while (*src) {
            if (*src == ' ' && *(src+1) == ' ') { src++; continue; }
            *dst++ = *src++;
        }
        *dst = '\0';
        return cpu;
    }
    return "Unknown CPU";
}

static char *get_gpu(void) {
    static char gpu[256] = {0};
    FILE *f = popen("lspci 2>/dev/null | grep -iE 'vga|3d|display' | head -1 | sed 's/.*: //'", "r");
    if (!f) return "Unknown GPU";
    if (fgets(gpu, sizeof(gpu), f)) {
        trim_newline(gpu);
        pclose(f);
        if (strlen(gpu) > 0) return gpu;
    }
    pclose(f);
    return "Unknown GPU";
}

static char *get_memory(void) {
    static char mem[64];
    long total = 0, available = 0;
    FILE *f = fopen("/proc/meminfo", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            long val;
            if (sscanf(line, "MemTotal: %ld kB", &val) == 1) total = val / 1024;
            if (sscanf(line, "MemAvailable: %ld kB", &val) == 1) available = val / 1024;
        }
        fclose(f);
    }
    snprintf(mem, sizeof(mem), "%ldMiB / %ldMiB", total - available, total);
    return mem;
}

static char *get_uptime(void) {
    static char uptime_str[64];
    FILE *f = fopen("/proc/uptime", "r");
    if (!f) return "unknown";
    double up;
    if (fscanf(f, "%lf", &up) != 1) { fclose(f); return "unknown"; }
    fclose(f);
    int days = (int)(up / 86400);
    int hours = (int)((up - days*86400) / 3600);
    int mins = (int)((up - days*86400 - hours*3600) / 60);
    if (days > 0)
        snprintf(uptime_str, sizeof(uptime_str), "%d days, %d hours, %d mins", days, hours, mins);
    else if (hours > 0)
        snprintf(uptime_str, sizeof(uptime_str), "%d hours, %d mins", hours, mins);
    else
        snprintf(uptime_str, sizeof(uptime_str), "%d mins", mins);
    return uptime_str;
}

static char *get_packages(void) {
    static char pkgs[64];
    int count = 0;
    FILE *f;
    char buf[128];

    f = popen("dpkg --list 2>/dev/null | wc -l", "r");
    if (f && fgets(buf, sizeof(buf), f)) { count = atoi(buf); pclose(f); }
    if (count > 0) { snprintf(pkgs, sizeof(pkgs), "%d (dpkg)", count > 5 ? count - 5 : count); return pkgs; }

    f = popen("rpm -qa 2>/dev/null | wc -l", "r");
    if (f && fgets(buf, sizeof(buf), f)) { count = atoi(buf); pclose(f); }
    if (count > 0) { snprintf(pkgs, sizeof(pkgs), "%d (rpm)", count); return pkgs; }

    f = popen("pacman -Q 2>/dev/null | wc -l", "r");
    if (f && fgets(buf, sizeof(buf), f)) { count = atoi(buf); pclose(f); }
    if (count > 0) { snprintf(pkgs, sizeof(pkgs), "%d (pacman)", count); return pkgs; }

    f = popen("apk info 2>/dev/null | wc -l", "r");
    if (f && fgets(buf, sizeof(buf), f)) { count = atoi(buf); pclose(f); }
    if (count > 0) { snprintf(pkgs, sizeof(pkgs), "%d (apk)", count); return pkgs; }

    f = popen("xbps-query -l 2>/dev/null | wc -l", "r");
    if (f && fgets(buf, sizeof(buf), f)) { count = atoi(buf); pclose(f); }
    if (count > 0) { snprintf(pkgs, sizeof(pkgs), "%d (xbps)", count); return pkgs; }

    return "unknown";
}

static char *get_shell(void) {
    char *shell = getenv("SHELL");
    if (!shell) return "unknown";
    char *p = strrchr(shell, '/');
    return p ? p + 1 : shell;
}

static char *get_de_wm(void) {
    static char de[64] = {0};
    char *xdg = getenv("XDG_CURRENT_DESKTOP");
    if (xdg) { strncpy(de, xdg, sizeof(de)-1); return de; }
    xdg = getenv("DESKTOP_SESSION");
    if (xdg) { strncpy(de, xdg, sizeof(de)-1); return de; }
    xdg = getenv("GDMSESSION");
    if (xdg) { strncpy(de, xdg, sizeof(de)-1); return de; }
    return "unknown";
}

static char *get_terminal(void) {
    char *term = getenv("TERM");
    return term ? term : "unknown";
}

static char *get_user(void) {
    char *user = getenv("USER");
    if (!user) user = getenv("LOGNAME");
    return user ? user : "unknown";
}

static void print_logo(const char *os) {
    if (strcmp(os, "arch") == 0) {
        printf("\033[36m");
        printf("       /\\\n");
        printf("      /  \\\n");
        printf("     /\\   \\\n");
        printf("    /      \\\n");
        printf("   /   ,,   \\\n");
        printf("  /   |  |  \\\n");
        printf(" /_-''    ''-_\\\n");
    } else if (strcmp(os, "debian") == 0) {
        printf("\033[31m");
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
        printf("\033[31m");
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
        printf("\033[34m");
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
        printf("\033[32m");
        printf("       ______\n");
        printf("      /      \\\n");
        printf("     |  () () |\n");
        printf("      \\  __  /\n");
        printf("       |    |\n");
        printf("       |    |\n");
        printf("       |____|\n");
    } else if (strcmp(os, "gentoo") == 0) {
        printf("\033[35m");
        printf("        _-----_\n");
        printf("       /       \\\n");
        printf("      |  O   O  |\n");
        printf("      |    _    |\n");
        printf("       \\  ---  /\n");
        printf("        \\_____/\n");
    } else if (strcmp(os, "alpine") == 0) {
        printf("\033[34m");
        printf("       /\\ /\\\n");
        printf("      /  \\ /  \\\n");
        printf("     /    /\\    \\\n");
        printf("    /    /  \\    \\\n");
        printf("   /    /    \\    \\\n");
        printf("  /    /      \\    \\\n");
        printf(" /____/        \\____\\\n");
    } else if (strcmp(os, "manjaro") == 0) {
        printf("\033[32m");
        printf(" ██████████████████\n");
        printf(" ██████████████████\n");
        printf(" ██████████████████\n");
        printf(" ██████████████████\n");
        printf(" ██████████████████\n");
        printf(" ██████████████████\n");
    } else if (strcmp(os, "mint") == 0) {
        printf("\033[32m");
        printf(" _______________\n");
        printf("|  ___________  |\n");
        printf("| |           | |\n");
        printf("| |  LINUX    | |\n");
        printf("| |   MINT    | |\n");
        printf("| |___________| |\n");
        printf("|_______________|\n");
    } else if (strcmp(os, "freebsd") == 0) {
        printf("\033[31m");
        printf("  ,        ,\n");
        printf("  |\\      /|\n");
        printf("  | \\    / |\n");
        printf("  |  \\  /  |\n");
        printf("  |   \\/   |\n");
        printf("  |        |\n");
        printf("  |        |\n");
        printf("  ----------\n");
    } else {
        printf("\033[33m");
        printf("  ╔══════════════════╗\n");
        printf("  ║                  ║\n");
        printf("  ║    %-12s   ║\n", os);
        printf("  ║                  ║\n");
        printf("  ╚══════════════════╝\n");
    }
    printf("\033[0m");
}

int main(void) {
    char *os = get_os_name();
    if (strlen(os) == 0) os = "linux";

    char *user = get_user();
    char *host = get_hostname();

    printf("\n");
    printf("  \033[1m%s@%s\033[0m\n", user, host);
    printf("  %s\n\n", get_pretty_name());

    print_logo(os);
    printf("\n");

    printf("  \033[1mOS:\033[0m       %s\n", get_pretty_name());
    printf("  \033[1mHost:\033[0m     %s\n", host);
    printf("  \033[1mKernel:\033[0m   %s\n", get_kernel());
    printf("  \033[1mUptime:\033[0m   %s\n", get_uptime());
    printf("  \033[1mPackages:\033[0m %s\n", get_packages());
    printf("  \033[1mShell:\033[0m    %s\n", get_shell());
    printf("  \033[1mDE/WM:\033[0m    %s\n", get_de_wm());
    printf("  \033[1mTerminal:\033[0m %s\n", get_terminal());
    printf("  \033[1mCPU:\033[0m      %s\n", get_cpu());
    printf("  \033[1mGPU:\033[0m      %s\n", get_gpu());
    printf("  \033[1mMemory:\033[0m   %s\n", get_memory());

    return 0;
}
