#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/atomic.h>

extern struct swap_log_control
{
    bool enable_swap_log;
    bool start_write_log;
    long pa_va_ht_size_max;
} swap_log_ctl;
extern int init_swap_log_file(void);
extern void close_swap_log_file(void);
extern void clear_pa_va_table(void);

#define PROC_NAME "swap_log_ctl"
#define BUFFER_SIZE 256

static char buffer[BUFFER_SIZE];
static struct proc_dir_entry *proc_entry;

static ssize_t swap_log_ctl_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{
    ssize_t len = snprintf(buffer, BUFFER_SIZE, 
        "enable_swap_log: %d\nstart_write_log: %d\npa_va_ht_size_max: %ld\n",
                           swap_log_ctl.enable_swap_log,
                           swap_log_ctl.start_write_log,
                           swap_log_ctl.pa_va_ht_size_max);
    return simple_read_from_buffer(user_buffer, count, offset, buffer, len);
}

static ssize_t swap_log_ctl_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset)
{
    char input[BUFFER_SIZE];
    int enable;

    if (count > BUFFER_SIZE - 1)
        return -EINVAL;

    if (copy_from_user(input, user_buffer, count))
        return -EFAULT;

    input[count] = '\0';

    if (sscanf(input, "%d", &enable) == 1)
    {
        if (enable == 0)
        {
            swap_log_ctl.enable_swap_log = 0;
            swap_log_ctl.start_write_log = 0;
            close_swap_log_file();
            clear_pa_va_table();
            swap_log_ctl.pa_va_ht_size_max = 0;
            printk(KERN_INFO "swap log disabled\n");
            return count;
        }
        if (enable == 1)    // We need to build PA-VA mapping even if we haven't reached the code region we want to log
        {
            init_swap_log_file();
            swap_log_ctl.enable_swap_log = 1;
            swap_log_ctl.start_write_log = 0;
            swap_log_ctl.pa_va_ht_size_max = 0;
            printk(KERN_INFO "swap log enabled (build PA-VA mapping)\n");
            return count;
        }
        if (enable == 2)
        {
            swap_log_ctl.start_write_log = 1;
            printk(KERN_INFO "start writing log file (must enable swap log first)\n");
            return count;
        }
    }

    return -EINVAL;
}

static const struct proc_ops proc_fops = {
    .proc_read = swap_log_ctl_read,
    .proc_write = swap_log_ctl_write,
};

static int __init swap_log_module_init(void)
{
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_fops);
    if (!proc_entry)
    {
        return -ENOMEM;
    }
    printk(KERN_INFO "swap_log_module loaded\n");
    return 0;
}

static void __exit swap_log_module_exit(void)
{
    proc_remove(proc_entry);
    printk(KERN_INFO "swap_log_module unloaded\n");
}

module_init(swap_log_module_init);
module_exit(swap_log_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anon");
MODULE_DESCRIPTION("A kernel module to interface with swap_log_ctl via proc filesystem.");
