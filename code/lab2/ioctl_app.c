#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>      /* open, O_RDWR */
#include <unistd.h>     /* close */
#include <sys/ioctl.h>  /* ioctl */

#include "myled_ioctl.h"

#define DEV_PATH "/dev/myled"

static int get_state(int fd)
{
    int state;

    if (ioctl(fd, MYLED_IOC_GET_STATE, &state) < 0) {
        perror("ioctl GET_STATE");
        return -1;
    }
    return state;
}

int main(void)
{
    int fd = open(DEV_PATH, O_RDWR);
    if (fd < 0) {
        perror("open " DEV_PATH);
        return EXIT_FAILURE;
    }

    printf("Start state        : %d\n", get_state(fd));

    int on = 1;
    if (ioctl(fd, MYLED_IOC_SET_STATE, &on) < 0)
        perror("ioctl SET_STATE");
    printf("After SET_STATE 1  : %d\n", get_state(fd));

    if (ioctl(fd, MYLED_IOC_TOGGLE) < 0)      /* khong tham so */
        perror("ioctl TOGGLE");
    printf("After TOGGLE       : %d\n", get_state(fd));

    /* Tham so sai: driver tra -EINVAL */
    int bad = 5;
    if (ioctl(fd, MYLED_IOC_SET_STATE, &bad) < 0)
        printf("SET_STATE 5 failed : errno = %d (%s)\n",
               errno, strerror(errno));

    /* Ma lenh khong ton tai: driver tra -ENOTTY */
    if (ioctl(fd, _IO(MYLED_IOC_MAGIC, 99)) < 0)
        printf("Unknown cmd failed : errno = %d (%s)\n",
               errno, strerror(errno));

    close(fd);
    return EXIT_SUCCESS;
}