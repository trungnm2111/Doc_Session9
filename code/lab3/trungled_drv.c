#include "myled_ioctl.h"

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/gpio/consumer.h>

static long myled_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

/* (A) Dữ liệu riêng của driver */
struct myled_data {
    struct miscdevice misc;
    struct gpio_desc *led;
    bool on;
    struct proc_dir_entry *proc;   /* entry /proc/myled */
};

/* (B) Lấy dữ liệu riêng từ struct file */
static struct myled_data *to_data(struct file *filp)
{
    struct miscdevice *m = filp->private_data;

    return container_of(m, struct myled_data, misc);
}

/* (C) Hàm write: user ghi lệnh xuống */
static ssize_t myled_write(struct file *filp, const char __user *ubuf,
                           size_t count, loff_t *ppos)
{
    struct myled_data *d = to_data(filp);
    char cmd;

    if (count == 0)
        return 0;
    if (copy_from_user(&cmd, ubuf, 1))
        return -EFAULT;

    switch (cmd) {
    case '1':
        gpiod_set_value_cansleep(d->led, 1);
        d->on = true;
        break;
    case '0':
        gpiod_set_value_cansleep(d->led, 0);
        d->on = false;
        break;
    case '2':
        pr_info("myled: trang thai = %d\n", d->on);
        break;
    default:
        return -EINVAL;
    }
    return count;
}

/* (D) Hàm read: user đọc trạng thái lên */
static ssize_t myled_read(struct file *filp, char __user *ubuf,
                          size_t count, loff_t *ppos)
{
    struct myled_data *d = to_data(filp);
    char buf[3];
    int len = scnprintf(buf, sizeof(buf), "%d\n", d->on);

    return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

/* (E) Bảng hàm callback */
static const struct file_operations myled_fops = {
    .owner          = THIS_MODULE,
    .write          = myled_write,
    .read           = myled_read,
    .unlocked_ioctl = myled_ioctl,
};

/* (P1) Doc: in noi dung ra /proc/myled */
static int myled_proc_show(struct seq_file *m, void *v)
{
    struct myled_data *d = m->private;

    seq_printf(m, "state: %d\n", d->on);
    return 0;
}

/* (P2) Mo file: gan ham show va du lieu rieng cho seq_file */
static int myled_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, myled_proc_show, pde_data(inode));
}

/* (P3) Ghi: user ghi "0" hoac "1" de dieu khien LED */
static ssize_t myled_proc_write(struct file *file, const char __user *ubuf,
                                size_t count, loff_t *ppos)
{
    struct myled_data *d = pde_data(file_inode(file));
    int val, ret;

    ret = kstrtoint_from_user(ubuf, count, 10, &val);
    if (ret)
        return ret;                 /* chuoi khong phai so */
    if (val != 0 && val != 1)
        return -EINVAL;

    gpiod_set_value_cansleep(d->led, val);
    d->on = val;
    return count;
}

/* (P4) Bang callback cho entry /proc */
static const struct proc_ops myled_proc_ops = {
    .proc_open    = myled_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = myled_proc_write,
};

/* (F) probe: xin GPIO rồi đăng ký misc device */
static int myled_probe(struct platform_device *pdev)
{
    struct myled_data *d;
    int ret;

    d = devm_kzalloc(&pdev->dev, sizeof(*d), GFP_KERNEL);
    if (!d)
        return -ENOMEM;

    d->led = devm_gpiod_get(&pdev->dev, "my_led", GPIOD_OUT_LOW);
    if (IS_ERR(d->led))
        return dev_err_probe(&pdev->dev, PTR_ERR(d->led),
                             "khong lay duoc GPIO led\n");

    d->misc.minor = MISC_DYNAMIC_MINOR;
    d->misc.name  = "myled";
    d->misc.fops  = &myled_fops;

    ret = misc_register(&d->misc);
    if (ret)
        return ret;

    d->proc = proc_create_data("myled", 0644, NULL, &myled_proc_ops, d);
    if (!d->proc) {
        misc_deregister(&d->misc);
        return -ENOMEM;
    }

    platform_set_drvdata(pdev, d);
    return 0;
}

/* (G) remove: gỡ misc device */
static void myled_remove(struct platform_device *pdev)
{
    struct myled_data *d = platform_get_drvdata(pdev);

    misc_deregister(&d->misc);
    gpiod_set_value_cansleep(d->led, 0);
    
    proc_remove(d->proc);
    misc_deregister(&d->misc);
    gpiod_set_value_cansleep(d->led, 0);
}

/* (H) Phần khung platform driver, giữ nguyên */
static const struct of_device_id myled_of_match[] = {
    { .compatible = "nguoi_ly_hop_tinh,led_control" },
    { }
};

/* (E1) Ham ioctl: user gui lenh dieu khien xuong */
static long myled_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct myled_data *d = to_data(filp);
    int val;

    switch (cmd) {
    case MYLED_IOC_GET_STATE:
        val = d->on;
        if (put_user(val, (int __user *)arg))
            return -EFAULT;
        return 0;

    case MYLED_IOC_SET_STATE:
        if (get_user(val, (int __user *)arg))
            return -EFAULT;
        if (val != 0 && val != 1)
            return -EINVAL;
        gpiod_set_value_cansleep(d->led, val);
        d->on = val;
        return 0;

    case MYLED_IOC_TOGGLE:
        d->on = !d->on;
        gpiod_set_value_cansleep(d->led, d->on);
        return 0;

    default:
        return -ENOTTY;   /* command no support */
    }
}

MODULE_DEVICE_TABLE(of, myled_of_match);

static struct platform_driver myled_driver = {
    .probe  = myled_probe,
    .remove = myled_remove,
    .driver = {
        .name = "myled",
        .of_match_table = myled_of_match,
    },
};



module_platform_driver(myled_driver);
MODULE_LICENSE("GPL");