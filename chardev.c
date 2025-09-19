#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mychardev"
#define BUF_LEN 128

static char device_buffer[BUF_LEN];
static int major_num;
static struct cdev my_cdev;

// File operations declarations
static int my_open(struct inode *inode, struct file *file);
static int my_release(struct inode *inode, struct file *file);
static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
};

/* Callbacks */
static int my_open(struct inode *inode, struct file *file) {
    pr_info("mychardev: Device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file) {
    pr_info("mychardev: Device released\n");
    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    size_t bytes = BUF_LEN - *ppos;
    if (count < bytes) bytes = count;
    if (bytes == 0) return 0;
    if (copy_to_user(buf, device_buffer + *ppos, bytes)) return -EFAULT;
    *ppos += bytes;
    return bytes;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    size_t bytes = BUF_LEN - *ppos;
    if (count < bytes) bytes = count;
    if (bytes == 0) return 0;
    if (copy_from_user(device_buffer + *ppos, buf, bytes)) return -EFAULT;
    *ppos += bytes;
    return bytes;
}

/* Module registration */
static int __init mychardev_init(void) {
    dev_t dev;
    int ret;
    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0) return ret;
    major_num = MAJOR(dev);
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;
    ret = cdev_add(&my_cdev, dev, 1);
    if (ret < 0) unregister_chrdev_region(dev, 1);
    pr_info("mychardev: registered with major number %d\n", major_num);
    return ret;
}

static void __exit mychardev_exit(void) {
    cdev_del(&my_cdev);
    unregister_chrdev_region(MKDEV(major_num, 0), 1);
    pr_info("mychardev: unloaded\n");
}

module_init(mychardev_init);
module_exit(mychardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Simple Character Device Lab");
