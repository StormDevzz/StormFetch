#ifndef INFO_H
#define INFO_H

#define OUTPUT_LEN 512

char *gen_os(void);
char *gen_host(void);
char *gen_kernel(void);
char *gen_uptime(void);
char *gen_packages(void);
char *gen_shell(void);
char *gen_de_wm(void);
char *gen_terminal(void);
char *gen_cpu(void);
char *gen_gpu(void);
char *gen_memory(void);
char *gen_swap(void);
char *gen_disk(void);
char *gen_network(void);
char *gen_local_ip(void);
char *gen_public_ip(void);
char *gen_processes(void);
char *gen_load_avg(void);
char *gen_battery(void);
char *gen_motherboard(void);
char *gen_bios(void);
char *gen_sound(void);
char *gen_resolution(void);
char *gen_temperature(void);
char *gen_users(void);
char *gen_locale(void);

#endif
