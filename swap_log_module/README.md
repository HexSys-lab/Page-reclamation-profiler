# README for Swap Log Module

## Overview
The Swap Log Module is a Linux kernel module that provides a proc filesystem interface for the `swap_log_ctl` structure.

## Features
- Interaction with the profiler via the proc filesystem.

## Building the Module
To build the kernel module, navigate to the project directory and run the following command:

```bash
make
```

This will compile the swap_log_module.c source file and create a loadable kernel module.

## Loading the Module
To load the module into the kernel, use the following command:

```bash
sudo insmod swap_log_module.ko
```

To check if the module is loaded, you can use:

```bash
lsmod | grep swap_log_module
```

## Interacting with the Proc Filesystem
Once the module is loaded, it will create entries in the proc filesystem. You can interact with these entries using standard file operations.

### Reading Values
To read the current values of the `swap_log_ctl` structure, you can use:

```bash
cat /proc/swap_log_ctl
```

This will output:
```
enable_swap_log: <value>
start_write_log: <value>
pa_va_ht_size_max: <value>
```

### Writing Values
To modify the values of the `swap_log_ctl` structure, you can use:

```bash
echo <value> > /proc/swap_log_ctl
```

Where `<value>` can be:
- `0` to disable the swap log.
- `1` to build the hash table.
- `2` to start writing log file.

For example, to enable the swap log, you can use:

```bash
echo 1 > /proc/swap_log_ctl
```

## Unloading the Module
To remove the module from the kernel, use the following command:

```bash
sudo rmmod swap_log_module
```