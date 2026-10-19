#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define PROC_PATH "/proc/myled"

/* Write buf "text" in /proc/myled. Return 0 if OK, -errno if failed */
static int proc_send(const char *text)
{
    int saved_errno;

    int fd = open(PROC_PATH, O_WRONLY);
    if (fd < 0)
        return -errno;

    ssize_t n = write(fd, text, strlen(text));
    saved_errno = errno;            /* save data after call */
    close(fd);

    if (n < 0)
        return -saved_errno;
    return 0;
}

/* Read /proc/myled to buf */
static int proc_show(char *buf, size_t size)
{
    int fd = open(PROC_PATH, O_RDONLY);
    if (fd < 0){
        return -errno;
    }
        
    ssize_t n = read(fd, buf, size - 1);
    close(fd);
    if (n < 0)
    {
        return -errno;
    }
        
    buf[n] = '\0';
    return 0;
}

static void try_write(const char *text)
{
    char buf[64];

    int ret = proc_send(text);
    if (ret < 0) {
        printf("Write \"%s\" failed: errno = %d (%s)\n",text, -ret, strerror(-ret));
        return;
    }

    if (proc_show(buf, sizeof(buf)) == 0)
        printf("Write \"%s\" -> Read again: %s", text, buf);
}

int main(void)
{
    try_write("1");     /* turn on LED */
    try_write("0");     /* turn off LED */
    try_write("1");     /* turn on LED */
    try_write("5");     /* invalid number -> EINVAL */
    try_write("abc");   /* not number -> EINVAL */

    return EXIT_SUCCESS;
}