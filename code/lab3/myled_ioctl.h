#ifndef MYLED_IOCTL_H
#define MYLED_IOCTL_H

#include <linux/ioctl.h>

/* Magic number: chon tam, nen doi chieu voi */
#define MYLED_IOC_MAGIC  0xE7

#define MYLED_IOC_GET_STATE  _IOR(MYLED_IOC_MAGIC, 1, int)  /* driver -> user */
#define MYLED_IOC_SET_STATE  _IOW(MYLED_IOC_MAGIC, 2, int)  /* user -> driver */
#define MYLED_IOC_TOGGLE     _IO(MYLED_IOC_MAGIC, 3)        /* no paramater */

#endif