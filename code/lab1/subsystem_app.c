#define _GNU_SOURCE
#include <unistd.h>       /* syscall() */
#include <sys/syscall.h>  /* SYS_openat, SYS_write, ... */
#include <fcntl.h>        /* AT_FDCWD, O_RDWR */
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define DEV_PATH "/dev/myled"

/* Open device file, return file descriptor or -1 */
static long raw_open(const char *path, int flags)
{
    return syscall(SYS_openat, AT_FDCWD, path, flags);
}

/* Send 1 character to kernel driver.
 * Return 0 on success, or -errno on failure. */
static int led_cmd(char cmd)
{
    int saved_errno;

    long fd = raw_open(DEV_PATH, O_WRONLY);
    if (fd < 0) {
        saved_errno = errno;            /* save immediately */
        fprintf(stderr, "openat: %s\n", strerror(saved_errno));
        return -saved_errno;
    }

    long n = syscall(SYS_write, fd, &cmd, 1);
    saved_errno = errno;                /* save immediately */

    syscall(SYS_close, fd);

    if (n < 0) {
        fprintf(stderr, "write('%c'): %s\n", cmd, strerror(saved_errno));
        return -saved_errno;
    }
    return 0;
}

/* Read state led */
static int led_get(void)
{
    char buf[8] = {0};
    int saved_errno;

    long fd = raw_open(DEV_PATH, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "openat: %s\n", strerror(errno));
        return -1;
    }

    long n = syscall(SYS_read, fd, buf, sizeof(buf) - 1);
    saved_errno = errno;
    syscall(SYS_close, fd);

    if (n < 0) {
        fprintf(stderr, "read: %s\n", strerror(saved_errno));
        return -1;
    }
    return buf[0] - '0';
}

int main(void)
{
    printf("Start state: %d\n", led_get());

    led_cmd('1');
    printf("After write '1'   : %d\n", led_get());

    led_cmd('2');

    led_cmd('0');
    printf("After write '0'   : %d\n", led_get());

    /* Try invalid command to get -EINVAL from driver */
    int ret = led_cmd('x');
    if (ret < 0)
        printf("Command 'x' rejected, errno = %d (%s)\n",
               -ret, strerror(-ret));

    return 0;
}