#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Doc /proc/uptime: "<Run time> <Free time>" (s) */
static int read_uptime(double *up, double *idle)
{
    FILE *fp = fopen("/proc/uptime", "r");
    if (!fp) {
        perror("fopen /proc/uptime");
        return -1;
    }

    int n = fscanf(fp, "%lf %lf", up, idle);
    fclose(fp);
    return (n == 2) ? 0 : -1;
}

/* Doc dong "MemTotal:  xxxx kB" trong /proc/meminfo */
static long read_mem_total_kb(void)
{
    char line[128];
    long kb = -1;

    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        perror("fopen /proc/meminfo");
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "MemTotal: %ld kB", &kb) == 1)
            break;
    }
    fclose(fp);
    return kb;
}

int main(void)
{
    double up, idle;

    if (read_uptime(&up, &idle) == 0)
        printf("Uptime     : %.2f giay (%.1f phut)\n", up, up / 60.0);

    long kb = read_mem_total_kb();
    if (kb >= 0)
        printf("MemTotal   : %ld kB (%.1f MB)\n", kb, kb / 1024.0);

    return EXIT_SUCCESS;
}