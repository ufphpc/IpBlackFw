#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26))
#include <linux/file.h>
#else
#include <linux/fdtable.h>
#endif

#include "fw_policy.h"
#include "device_io.h"
#include "interface_io.h"

#define DRIVER_DEVICE_NAME "fwDevice"

struct fw_device {
	struct cdev cdev;
};

static struct fw_device g_fwDev;
static dev_t g_devno;
static struct class *g_fwdev_class;

static int device_io_open(struct inode *inode, struct file *filp)
{
    pr_info("device_io_open\n");
    return 0;
}

static int device_io_release(struct inode *inode, struct file *filp)
{
    pr_info("device_io_release\n");
    return 0;
}

static long device_io_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret;

    if (cmd == FW_IOCTL_ADD_INFO) {

        struct fw_set_info info;

        if (copy_from_user(&info, (void *)arg, sizeof(info)) != 0) {
            pr_err("device_io_ioctl: FW_IOCTL_ADD_INFO invalid info\n");
			ret = -EINVAL;
			goto err_out;
		}

        pr_info("device_io_ioctl: add ip 0x%x\n", info.ip);

        fw_add_policy_node(info.ip);
        ret = 0;
    } else if (cmd == FW_IOCTL_DEL_INFO) {

        struct fw_set_info info;

        if (copy_from_user(&info, (void *)arg, sizeof(info)) != 0) {
            pr_err("device_io_ioctl: FW_IOCTL_DEL_INFO invalid info\n");
			ret = -EINVAL;
			goto err_out;
		}

        pr_info("device_io_ioctl: del ip 0x%x\n", info.ip);

        fw_remove_policy_node(info.ip);
        ret = 0;
    } else {

        pr_err("device_io_ioctl: invalid cmd...\n");
        ret = -ENOTTY;
    }

err_out:
    return ret;
}

//file_operations
struct file_operations g_device_io_fops = {
    .owner = THIS_MODULE,
    .open = device_io_open,
	.release = device_io_release,
	.unlocked_ioctl = device_io_ioctl,
};

int device_io_init(void)
{
    int ret;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 20)
	struct device *device = NULL;
#else
	struct class_device *device = NULL;
#endif

    /*
	 * Initialize the user I/O
	 */
	ret = alloc_chrdev_region(&g_devno, 0, 1, DRIVER_DEVICE_NAME);
	if (ret < 0) {
		pr_err("device_io_init: could not allocate major number for %s\n", DRIVER_DEVICE_NAME);

		ret = -ENOMEM;
		goto out;
	}

    g_fwdev_class = class_create(THIS_MODULE, DRIVER_DEVICE_NAME);
	if (IS_ERR(g_fwdev_class)) {
		pr_err("device_io_init: can't create device class\n");

		ret = PTR_ERR(g_fwdev_class);
		goto out_chrdev;
	}

    cdev_init(&g_fwDev.cdev, &g_device_io_fops);
    g_fwDev.cdev.owner = THIS_MODULE;
    //g_fwDev.cdev.ops = &g_device_io_fops;

    if (cdev_add(&g_fwDev.cdev, g_devno, 1) < 0) {
        pr_err("device_io_init: could not add chrdev for %s\n", DRIVER_DEVICE_NAME);
        ret = -EFAULT;
        goto out_class;
    }

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 20)
		device = device_create(
#else
		device = class_device_create(
#endif
						g_fwdev_class, NULL, /* no parent device */
						g_devno,
						NULL, /* no additional data */
						DRIVER_DEVICE_NAME);

    if (IS_ERR(device)) {
        pr_err("device_io_init: create the device err for %s\n", DRIVER_DEVICE_NAME);
        
        ret = PTR_ERR(device);
        goto out_add;
    }

    pr_info("device_io_init: ...\n");

    ret = 0;
    goto out;

out_add:
    cdev_del(&g_fwDev.cdev);
out_class:
	class_destroy(g_fwdev_class);
out_chrdev:
    unregister_chrdev_region(g_devno, 1);
out:
    return ret;
}

void device_io_uninit(void)
{
    device_destroy(g_fwdev_class, g_devno);
    cdev_del(&g_fwDev.cdev);
    class_destroy(g_fwdev_class);
    unregister_chrdev_region(g_devno, 1);
}



