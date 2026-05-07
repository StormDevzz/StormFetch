#include "info.h"
#include "util.h"
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <time.h>
#include <pwd.h>

char *gen_os(void) {
    static char out[OUTPUT_LEN];
    char *pretty = read_value_from_proc("/etc/os-release", "PRETTY_NAME=");
    if (pretty) {
        if (pretty[0] == '"') { memmove(pretty, pretty+1, strlen(pretty)); char *e = strrchr(pretty, '"'); if (e) *e = '\0'; }
        strncpy(out, pretty, sizeof(out)-1);
        return out;
    }
    struct utsname u;
    uname(&u);
    snprintf(out, sizeof(out), "%s %s", u.sysname, u.release);
    return out;
}

char *gen_host(void) {
    static char out[256];
    if (gethostname(out, sizeof(out)) == 0) return out;
    return "unknown";
}

char *gen_kernel(void) {
    static char out[OUTPUT_LEN];
    struct utsname u;
    uname(&u);
    snprintf(out, sizeof(out), "%s %s", u.sysname, u.release);
    return out;
}

char *gen_uptime(void) {
    static char out[64];
    FILE *f = fopen("/proc/uptime", "r");
    if (!f) return "unknown";
    double up;
    if (fscanf(f, "%lf", &up) != 1) { fclose(f); return "unknown"; }
    fclose(f);
    int days = (int)(up / 86400);
    int hours = (int)((up - days*86400) / 3600);
    int mins = (int)((up - days*86400 - hours*3600) / 60);
    if (days > 0) snprintf(out, sizeof(out), "%dd %dh %dm", days, hours, mins);
    else if (hours > 0) snprintf(out, sizeof(out), "%dh %dm", hours, mins);
    else snprintf(out, sizeof(out), "%dm", mins);
    return out;
}

char *gen_packages(void) {
    static char out[64];
    int count = 0;
    char *buf;

    if ((buf = exec_cmd("dpkg --list 2>/dev/null | wc -l")) && (count = atoi(buf)) > 0)
        { snprintf(out, sizeof(out), "%d (dpkg)", count > 5 ? count - 5 : count); return out; }
    if ((buf = exec_cmd("rpm -qa 2>/dev/null | wc -l")) && (count = atoi(buf)) > 0)
        { snprintf(out, sizeof(out), "%d (rpm)", count); return out; }
    if ((buf = exec_cmd("pacman -Q 2>/dev/null | wc -l")) && (count = atoi(buf)) > 0)
        { snprintf(out, sizeof(out), "%d (pacman)", count); return out; }
    if ((buf = exec_cmd("apk info 2>/dev/null | wc -l")) && (count = atoi(buf)) > 0)
        { snprintf(out, sizeof(out), "%d (apk)", count); return out; }
    if ((buf = exec_cmd("xbps-query -l 2>/dev/null | wc -l")) && (count = atoi(buf)) > 0)
        { snprintf(out, sizeof(out), "%d (xbps)", count); return out; }
    if ((buf = exec_cmd("flatpak list --app 2>/dev/null | wc -l")) && (count = atoi(buf)) > 0)
        { snprintf(out, sizeof(out), "%d (flatpak)", count); return out; }
    if ((buf = exec_cmd("snap list 2>/dev/null | wc -l")) && (count = atoi(buf)) > 0)
        { snprintf(out, sizeof(out), "%d (snap)", count > 1 ? count - 1 : count); return out; }
    return "unknown";
}

char *gen_shell(void) {
    char *shell = getenv("SHELL");
    if (!shell) return "unknown";
    char *p = strrchr(shell, '/');
    return p ? p + 1 : shell;
}

char *gen_de_wm(void) {
    static char out[OUTPUT_LEN];
    char *xdg = getenv("XDG_CURRENT_DESKTOP");
    if (xdg) { strncpy(out, xdg, sizeof(out)-1); return out; }
    xdg = getenv("DESKTOP_SESSION");
    if (xdg) { strncpy(out, xdg, sizeof(out)-1); return out; }
    xdg = getenv("GDMSESSION");
    if (xdg) { strncpy(out, xdg, sizeof(out)-1); return out; }
    char *wm = exec_cmd("wmctrl -m 2>/dev/null | head -1 | cut -d: -f2");
    if (wm && strlen(wm) > 0) { snprintf(out, sizeof(out), "%s", wm); return out; }
    return "unknown";
}

char *gen_terminal(void) {
    char *term = getenv("TERM");
    return term ? term : "unknown";
}

char *gen_cpu(void) {
    static char out[OUTPUT_LEN];
    char *model = read_value_from_proc("/proc/cpuinfo", "model name");
    if (!model) return "Unknown CPU";
    char model_buf[OUTPUT_LEN];
    strncpy(model_buf, model, sizeof(model_buf)-1);
    model_buf[sizeof(model_buf)-1] = '\0';

    int cores = 0, threads = 0;
    char buf[MAX_LINE];
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (f) {
        while (fgets(buf, sizeof(buf), f)) {
            if (strncmp(buf, "cpu cores", 9) == 0) {
                char *p = buf + 9;
                while (*p == ' ' || *p == '\t' || *p == ':') p++;
                cores = atoi(p);
            }
            if (strncmp(buf, "processor", 9) == 0) threads++;
        }
        fclose(f);
    }

    char *freq_str = read_value_from_proc("/proc/cpuinfo", "cpu MHz");
    double freq = freq_str ? atof(freq_str) / 1000.0 : 0.0;

    snprintf(out, sizeof(out), "%s (%dc/%dt)", model_buf, cores, threads);
    if (freq > 0) {
        char tmp[OUTPUT_LEN];
        snprintf(tmp, sizeof(tmp), "%s @ %.2fGHz", out, freq);
        strncpy(out, tmp, sizeof(out)-1);
        out[sizeof(out)-1] = '\0';
    }
    return out;
}

char *gen_gpu(void) {
    static char out[OUTPUT_LEN] = {0};
    char *gpu = exec_cmd("lspci 2>/dev/null | grep -iE -- 'vga|3d|display' | head -5 | sed 's/.*: //'");
    if (gpu && strlen(gpu) > 0) {
        strncpy(out, gpu, sizeof(out)-1);
        char *more = exec_cmd("lspci 2>/dev/null | grep -iE 'vga|3d|display' | wc -l");
        int n = more ? atoi(more) : 1;
        if (n > 1) {
            char tmp[OUTPUT_LEN];
            snprintf(tmp, sizeof(tmp), "%s (+%d more)", out, n - 1);
            strncpy(out, tmp, sizeof(out)-1);
        }
        return out;
    }
    DIR *d = opendir("/sys/class/drm");
    if (d) {
        struct dirent *de;
        while ((de = readdir(d))) {
            if (strstr(de->d_name, "card") && !strstr(de->d_name, "-")) {
                char path[256];
                snprintf(path, sizeof(path), "/sys/class/drm/%s/device/vendor", de->d_name);
                char *vendor = read_first_line(path);
                snprintf(path, sizeof(path), "/sys/class/drm/%s/device/device", de->d_name);
                char *device = read_first_line(path);
                if (vendor && device) {
                    snprintf(out, sizeof(out), "PCI %s:%s", vendor, device);
                    closedir(d);
                    return out;
                }
            }
        }
        closedir(d);
    }
    return "Unknown GPU";
}

int g_use_color = 0;

char *gen_memory(void) {
    static char out[64];
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
    long used = total - available;
    int pct = total > 0 ? (int)(used * 100 / total) : 0;
    snprintf(out, sizeof(out), "%ldMiB / %ldMiB %s", used, total, make_bar(pct, g_use_color));
    return out;
}

char *gen_swap(void) {
    static char out[64];
    long total = 0, free = 0;
    FILE *f = fopen("/proc/meminfo", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            long val;
            if (sscanf(line, "SwapTotal: %ld kB", &val) == 1) total = val / 1024;
            if (sscanf(line, "SwapFree: %ld kB", &val) == 1) free = val / 1024;
        }
        fclose(f);
    }
    if (total == 0) return "none";
    long used = total - free;
    int pct = total > 0 ? (int)(used * 100 / total) : 0;
    snprintf(out, sizeof(out), "%ldMiB / %ldMiB %s", used, total, make_bar(pct, g_use_color));
    return out;
}

char *gen_disk(void) {
    static char out[OUTPUT_LEN];
    struct statvfs vfs;
    char total = 0;
    out[0] = '\0';

    const char *mounts[] = {"/", "/home", NULL};
    for (int i = 0; mounts[i]; i++) {
        if (statvfs(mounts[i], &vfs) != 0) continue;
        unsigned long size = (unsigned long)(vfs.f_frsize * vfs.f_blocks) / (1024UL*1024UL*1024UL);
        unsigned long free = (unsigned long)(vfs.f_frsize * vfs.f_bfree) / (1024UL*1024UL*1024UL);
        unsigned long used = size - free;
        int pct = size > 0 ? (int)(used * 100 / size) : 0;
        char part[128];
        if (strcmp(mounts[i], "/") == 0)
            snprintf(part, sizeof(part), "/: %luG/%luG %s %d%%", used, size, make_bar(pct, g_use_color), pct);
        else
            snprintf(part, sizeof(part), "%s: %luG/%luG %s %d%%", mounts[i], used, size, make_bar(pct, g_use_color), pct);

        if (total == 0) strncpy(out, part, sizeof(out)-1);
        else { strncat(out, ", ", sizeof(out)-strlen(out)-1); strncat(out, part, sizeof(out)-strlen(out)-1); }
        total = 1;
    }
    if (strlen(out) == 0) return "unknown";
    return out;
}

char *gen_network(void) {
    static char out[OUTPUT_LEN];
    out[0] = '\0';
    struct dirent **entries;
    int n = scandir("/sys/class/net", &entries, NULL, NULL);
    if (n < 0) return "unknown";

    int found = 0;
    for (int i = 0; i < n; i++) {
        if (entries[i]->d_name[0] == '.' ||
            strcmp(entries[i]->d_name, "lo") == 0) {
            free(entries[i]);
            continue;
        }
        char path[256];
        snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", entries[i]->d_name);
        char *state = read_first_line(path);
        if (state && strcmp(state, "up") == 0) {
            snprintf(path, sizeof(path), "/sys/class/net/%s/address", entries[i]->d_name);
            char *mac = read_first_line(path);
            if (found > 0) strncat(out, ", ", sizeof(out)-strlen(out)-1);
            if (mac) snprintf(out + strlen(out), sizeof(out)-strlen(out), "%s (%s)", entries[i]->d_name, mac);
            else snprintf(out + strlen(out), sizeof(out)-strlen(out), "%s", entries[i]->d_name);
            found++;
        }
        free(entries[i]);
    }
    free(entries);
    if (strlen(out) == 0) return "no active interfaces";
    return out;
}

char *gen_local_ip(void) {
    static char out[OUTPUT_LEN];
    char *ip = exec_cmd("ip -4 addr show scope global 2>/dev/null | grep inet | awk '{print $2}' | head -3 | paste -sd, ");
    if (ip && strlen(ip) > 0) { strncpy(out, ip, sizeof(out)-1); return out; }
    ip = exec_cmd("ifconfig 2>/dev/null | grep 'inet ' | grep -v 127.0.0.1 | awk '{print $2}' | head -3 | paste -sd, ");
    if (ip && strlen(ip) > 0) { strncpy(out, ip, sizeof(out)-1); return out; }
    return "unknown";
}

char *gen_public_ip(void) {
    static char out[128];
    char *ip = exec_cmd("curl -s --connect-timeout 3 https://ifconfig.me 2>/dev/null");
    if (!ip || strlen(ip) == 0)
        ip = exec_cmd("wget -qO- --timeout=3 https://ifconfig.me 2>/dev/null");
    if (ip && strlen(ip) > 0) { strncpy(out, ip, sizeof(out)-1); return out; }
    return "unavailable";
}

char *gen_processes(void) {
    static char out[32];
    DIR *d = opendir("/proc");
    if (!d) return "unknown";
    int count = 0;
    struct dirent *de;
    while ((de = readdir(d))) {
        if (de->d_type == DT_DIR && isdigit(de->d_name[0])) count++;
    }
    closedir(d);
    snprintf(out, sizeof(out), "%d", count);
    return out;
}

char *gen_load_avg(void) {
    static char out[64];
    char *load = read_first_line("/proc/loadavg");
    if (!load) return "unknown";
    char *space = strrchr(load, ' ');
    if (space) *space = '\0';
    strncpy(out, load, sizeof(out)-1);
    return out;
}

char *gen_battery(void) {
    static char out[OUTPUT_LEN];
    out[0] = '\0';
    for (int i = 0; i < 10; i++) {
        char path[256];
        snprintf(path, sizeof(path), "/sys/class/power_supply/BAT%d/uevent", i);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        char line[256];
        char status[64] = {0}, capacity[64] = {0}, name[64] = {0};
        while (fgets(line, sizeof(line), f)) {
            trim_newline(line);
            if (strncmp(line, "POWER_SUPPLY_STATUS=", 20) == 0) strncpy(status, line + 20, sizeof(status)-1);
            if (strncmp(line, "POWER_SUPPLY_CAPACITY=", 22) == 0) strncpy(capacity, line + 22, sizeof(capacity)-1);
            if (strncmp(line, "POWER_SUPPLY_NAME=", 18) == 0) strncpy(name, line + 18, sizeof(name)-1);
        }
        fclose(f);
        if (capacity[0]) {
            char buf[128];
            if (name[0])
                snprintf(buf, sizeof(buf), "%s: %s%% (%s)", name, capacity, status[0] ? status : "unknown");
            else
                snprintf(buf, sizeof(buf), "BAT%d: %s%% (%s)", i, capacity, status[0] ? status : "unknown");
            if (out[0]) strncat(out, ", ", sizeof(out)-strlen(out)-1);
            strncat(out, buf, sizeof(out)-strlen(out)-1);
        }
    }
    if (strlen(out) == 0) return "no battery";
    return out;
}

char *gen_motherboard(void) {
    static char out[OUTPUT_LEN];
    char vbuf[128] = {0}, nbuf[128] = {0};
    char *v = read_first_line("/sys/class/dmi/id/board_vendor");
    if (v) strncpy(vbuf, v, sizeof(vbuf)-1);
    char *n = read_first_line("/sys/class/dmi/id/board_name");
    if (n) strncpy(nbuf, n, sizeof(nbuf)-1);
    if (vbuf[0] && nbuf[0]) { snprintf(out, sizeof(out), "%s %s", vbuf, nbuf); return out; }
    if (vbuf[0]) { strncpy(out, vbuf, sizeof(out)-1); return out; }
    if (nbuf[0]) { strncpy(out, nbuf, sizeof(out)-1); return out; }
    return "unknown";
}

char *gen_bios(void) {
    static char out[OUTPUT_LEN];
    char vbuf[128] = {0}, ver[128] = {0}, dbuf[128] = {0};
    char *v = read_first_line("/sys/class/dmi/id/bios_vendor");
    if (v) strncpy(vbuf, v, sizeof(vbuf)-1);
    char *ve = read_first_line("/sys/class/dmi/id/bios_version");
    if (ve) strncpy(ver, ve, sizeof(ver)-1);
    char *d = read_first_line("/sys/class/dmi/id/bios_date");
    if (d) strncpy(dbuf, d, sizeof(dbuf)-1);
    if (vbuf[0] || ver[0] || dbuf[0])
        snprintf(out, sizeof(out), "%s %s (%s)",
            vbuf[0] ? vbuf : "?", ver[0] ? ver : "?", dbuf[0] ? dbuf : "?");
    else
        return "unknown";
    return out;
}

char *gen_sound(void) {
    static char out[OUTPUT_LEN];
    char *cards = exec_cmd("cat /proc/asound/cards 2>/dev/null | grep -v -- '---' | grep -v '^ *$' | head -5 | awk -F'[][]' '{print $2}' | paste -sd, ");
    if (cards && strlen(cards) > 0) { strncpy(out, cards, sizeof(out)-1); return out; }
    return "unknown";
}

char *gen_resolution(void) {
    static char out[OUTPUT_LEN];
    char *res = exec_cmd("xrandr --current 2>/dev/null | grep ' connected' | grep -oP '\\d+x\\d+' | head -3 | paste -sd, ");
    if (res && strlen(res) > 0) { snprintf(out, sizeof(out), "%s", res); return out; }
    DIR *d = opendir("/sys/class/drm");
    if (d) {
        struct dirent *de;
        int first = 1;
        out[0] = '\0';
        while ((de = readdir(d))) {
            if (strstr(de->d_name, "card") && !strstr(de->d_name, "-")) {
                char path[256];
                snprintf(path, sizeof(path), "/sys/class/drm/%s/", de->d_name);
                DIR *sub = opendir(path);
                if (!sub) continue;
                struct dirent *sde;
                while ((sde = readdir(sub))) {
                    if (strstr(sde->d_name, "card")) {
                        char mode_path[384];
                        snprintf(mode_path, sizeof(mode_path), "%s%s/modes", path, sde->d_name);
                        char *mode = read_first_line(mode_path);
                        if (mode) {
                            if (!first) strncat(out, ", ", sizeof(out)-strlen(out)-1);
                            strncat(out, mode, sizeof(out)-strlen(out)-1);
                            first = 0;
                        }
                    }
                }
                closedir(sub);
            }
        }
        closedir(d);
        if (strlen(out) > 0) return out;
    }
    return "unknown";
}

char *gen_temperature(void) {
    static char out[OUTPUT_LEN];
    out[0] = '\0';
    struct dirent **entries;
    int n = scandir("/sys/class/thermal", &entries, NULL, NULL);
    if (n < 0) return "unknown";

    int found = 0;
    for (int i = 0; i < n; i++) {
        if (strstr(entries[i]->d_name, "thermal_zone")) {
            char path[256];
            snprintf(path, sizeof(path), "/sys/class/thermal/%s/temp", entries[i]->d_name);
            char *temp_str = read_first_line(path);
            if (temp_str) {
                long millideg = atol(temp_str);
                if (millideg > 0 && millideg < 120000) {
                    char part[32];
                    snprintf(part, sizeof(part), "%.1f°C", millideg / 1000.0);
                    if (found > 0) strncat(out, ", ", sizeof(out)-strlen(out)-1);
                    strncat(out, part, sizeof(out)-strlen(out)-1);
                    found++;
                }
            }
        }
        free(entries[i]);
    }
    free(entries);
    if (strlen(out) == 0) return "unknown";
    return out;
}

char *gen_users(void) {
    static char out[256];
    char *users = exec_cmd("who 2>/dev/null | awk '{print $1}' | sort -u | paste -sd, ");
    if (users && strlen(users) > 0) { strncpy(out, users, sizeof(out)-1); return out; }
    return "unknown";
}

char *gen_locale(void) {
    static char out[128];
    char *lang = getenv("LANG");
    if (lang) { strncpy(out, lang, sizeof(out)-1); return out; }
    return "unknown";
}
