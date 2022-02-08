
/*
 * Sample kfifo int type implementation
 *
 *
 * Released under the GPL version 2 only.
 *
 */
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>

static dev_t first; // Global variable for the first device number
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl; // Global variable for the device class

#define _IOC_WRITE     1U
#define MY_MACIG 'G'
#define DATA_READ _IOC(_IOC_WRITE,'K',0x0d,0x3FFF)

struct data_read {
        int buffer[32];
};


/*
 * This module shows how to create a int type fifo.
 */

/* fifo size in elements (ints) */
#define FIFO_SIZE   32

/* lock for procfs read access */
static DEFINE_MUTEX(read_lock);

/* lock for procfs write access */
static DEFINE_MUTEX(write_lock);

/*
 * define DYNAMIC in this example for a dynamically allocated fifo.
 *
 * Otherwise the fifo storage will be a part of the fifo structure.
 */

//static DECLARE_KFIFO_PTR(test, int);   // This is for int type
static DECLARE_KFIFO_PTR(test, char);

//DECLARE_KFIFO(test, int, FIFO_SIZE);


#if 0   // This is validation of int type data in Fifo
static const int expected_result[FIFO_SIZE] = {
     3,  4,  5,  6,  7,  8,  9,  0,
     1, 20, 21, 22, 23, 24, 25, 26,
    27, 28, 29, 30, 31, 32, 33, 34,
    35, 36, 37, 38, 39, 40, 41, 42,
};

static int __init testfunc(void)
{
    int     buf[6];
    int     i;
//      int j ;
    unsigned int    ret;

    printk(KERN_INFO "int fifo test start\n");

    /* put values into the fifo */
    for (i = 0; i != 10; i++)
        kfifo_put(&test, i);

    /* show the number of used elements */
    printk(KERN_INFO "fifo len: %u\n", kfifo_len(&test));
    if (kfifo_peek(&test, &i))
        printk(KERN_INFO "[1]: %d\n", i);

    /* get max of 2 elements from the fifo */
    ret = kfifo_out(&test, buf, 2);
    /* and put it back to the end of the fifo */
    ret = kfifo_in(&test, buf, ret);
    if (kfifo_peek(&test, &i))
        printk(KERN_INFO "[1]: %d\n", i);

    /* skip first element of the fifo */
    printk(KERN_INFO "skip 1st element\n");
    kfifo_skip(&test);
    if (kfifo_peek(&test, &i))
        printk(KERN_INFO "after skip [1]: %d\n", i);


    printk(KERN_INFO "queue len after skip: %u\n", kfifo_len(&test));

        for (i = 20; kfifo_put(&test, i); i++);


    /* show the first value without removing from the fifo */
    if (kfifo_peek(&test, &i))
        printk(KERN_INFO "after full [1]: %d\n", i);

#if 0
    /* check the correctness of all values in the fifo */
    j = 0;

        while (kfifo_get(&test, &i)) {
       // printk(KERN_INFO "%d  ", i);
        if (i != expected_result[j++]) {
            printk(KERN_WARNING "value mismatch: test failed\n");
            return -EIO;
        }
    }

        printk("\n");
    if (j != ARRAY_SIZE(expected_result)) {
        printk(KERN_WARNING "size mismatch: test failed\n");
        return -EIO;
    }
#endif
    printk(KERN_INFO "Kfifo is Filled Properly\n");

    return 0;
}
#endif


static int __init testfunc(void)
{
    int     i;
//      int j ;
    char arr[15] = {'P', 'u', 's', 'h', 'p', 'e', 'n', 'd', 'r', 'a', 'S', 'i', 'n', 'g', 'h'};
        char p;

    printk(KERN_INFO "int fifo test start\n");

    /* put values into the fifo */
    for (i = 0; i != 15; i++)
        kfifo_put(&test, arr[i]);

    /* show the number of used elements */
    printk(KERN_INFO "fifo len: %u\n", kfifo_len(&test));
    if (kfifo_peek(&test, &p))
        printk(KERN_INFO "[1]: %c\n", p);

    printk(KERN_INFO "Kfifo is Filled Properly\n");

    return 0;
}


static ssize_t fifo_write(struct file *file, const char __user *buf,
                        size_t count, loff_t *ppos)
{
    int ret;
    unsigned int copied;

        printk("fifo_write\n");
    if (mutex_lock_interruptible(&write_lock))
        return -ERESTARTSYS;

    ret = kfifo_from_user(&test, buf, count, &copied);

    mutex_unlock(&write_lock);

        printk("Driver Data copied from user: %d\n", ret);
    return ret ? ret : copied;
}

static ssize_t fifo_read(struct file *file, char __user *buf,
                        size_t count, loff_t *ppos)
{
    int ret;
    unsigned int copied;
        //int *arr;
        //int p = 100;

        printk(" Driver fifo_read : %ld\n ", count);

        //arr = (int *)kmalloc((sizeof(int)* 10), GFP_KERNEL);

        //for(ret=0; ret < 10; ret++)
        //      arr[ret] = p++;

        if(kfifo_is_empty(&test)){
                printk("FIFO Empty\n");
                return -EIO;
        }

      if (mutex_lock_interruptible(&write_lock))
                return -ERESTARTSYS;

        ret = kfifo_to_user(&test, buf, count, &copied);
        //ret = copy_to_user(&arr, buf, count);

        mutex_unlock(&write_lock);

        printk("Driver: %d   %d\n", ret, copied);
        //kfree(arr);
    return ret ? ret : copied;
}

static int fifo_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "Driver: close()\n");
    return 0;
}

static int fifo_open(struct inode *i, struct file *f)
{
        printk(KERN_INFO "Driver: open()\n");
        return 0;
}

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct data_read kcs_data;
        int i = 0;
        int ab[10];
        int p = 100;

        switch(cmd){
                case DATA_READ:
                        printk("IOCTL : Data Read\n");
                        for(i = 0; i < 10; i++)
                                ab[i] = p++;

                         if(copy_from_user(&kcs_data,(void*) arg,sizeof(struct data_read)))
                                  return -EFAULT;

                        i = copy_to_user(&kcs_data, ab, 10);
                        printk("End Switch case : %d\n", i);
                        break;

                default:
                        printk("NO COMMAND FOUND \n");
                        break;
                }

        return i;
}



static const struct file_operations fifo_fops = {
    .owner      = THIS_MODULE,
    .open       = fifo_open,
    .read       = fifo_read,
    .write      = fifo_write,
  //  .llseek     = noop_llseek,
    .release    = fifo_close,
    .unlocked_ioctl = my_ioctl
};

static int __init example_init(void)
{
    int ret;
    struct device *dev_ret;

//      INIT_KFIFO(test);

#if 1
    ret = kfifo_alloc(&test, FIFO_SIZE, GFP_KERNEL);
    if (ret) {
        printk(KERN_ERR "error kfifo_alloc\n");
        return ret;
    }
#endif
    if (testfunc() < 0) {
        kfifo_free(&test);
        return -EIO;
    }

        if ((ret = alloc_chrdev_region(&first, 0, 1, "pskdev")) < 0)
        {
            return ret;
        }
        if (IS_ERR(cl = class_create(THIS_MODULE, "pskdev")))
        {
            unregister_chrdev_region(first, 1);
            return PTR_ERR(cl);
        }
        if (IS_ERR(dev_ret = device_create(cl, NULL, first, NULL, "pskdev")))
        {
            class_destroy(cl);
            unregister_chrdev_region(first, 1);
            return PTR_ERR(dev_ret);
        }

        cdev_init(&c_dev, &fifo_fops);
        if ((ret = cdev_add(&c_dev, first, 1)) < 0)
        {
            device_destroy(cl, first);
            class_destroy(cl);
            unregister_chrdev_region(first, 1);
            return ret;
        }

        printk("Driver Registerd\n");
    return 0;
}

static void __exit example_exit(void)
{
        cdev_del(&c_dev);
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        kfifo_free(&test);
        printk(KERN_INFO "Alvida: Driver unregistered");
}

module_init(example_init);
module_exit(example_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pushpendra Singh");
