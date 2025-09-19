# Page reclamation profiler for Linux kernel v6.9.0
A lightweight integrated profiler that can capture detailed page reclaim behavior of 2QLRU and MGLRU.

---

## Collected metrics
| **Metric**               | **Description** |
|--------------------------|-----------------|
| `flag_update_cycles`     | Cycles spent on sequential PTW and rmap look-around operations (MGLRU-specific; excluded from stage-level cycles) |
| `stage_2_cycles`         | Cycles spent on stage 2 (candidate selection) |
| `stage_3_cycles`         | Cycles spent on stage 3 (state verification) |
| `stage_4_cycles`         | Cycles spent on stage 4 (unmap and writeback) |
| `stage_5_cycles`         | Cycles spent on stage 5 (free page and clean up) |
| `unmap_cycles`           | Cycles spent on unmap operations at stage 4 |
| `nr_pg_unmap`            | # of unmap attempts |
| `pageout_cycles`         | Cycles spent on writeback operations at stage 4 |
| `nr_pg_pageout`          | # of writeback attempts |
| `nr_pg_scan`             | # of pages rotated to the tail of the victim lists during stage 2, regardless of whether they are ultimately selected or excluded |
| `nr_pte_scan`            | # of PTEs scanned during the flag update process (MGLRU-specific) |
| `nr_pg_promotion`        | # of pages promoted to younger generations during candidate selection (MGLRU-specific) |
| `nr_pg_demotion`         | # of pages demoted to the inactive list during candidate selection (2QLRU-specific) |
| `nr_pg_rotation`         | # of pages rotated in the same LRU list at stage 2 |
| `nr_reclaim_try`         | # of reclaim attempts; differs from nr_pg_scan as MGLRU may retry some reclaim candidates |
| `nr_reclaim_success`     | # of successful reclaims |
| `nr_pg_fail_promote`     | # of candidates returned to younger generations/active list after failed reclaim |
| `nr_pg_fail_rotate`      | # of candidates returned to the oldest generation/inactive list after failed reclaim |
| `Reclaim trace`          | Format: `<PFN, VA, page size (# of base pages), NUMA node ID, page type (anonymous/file)>` |

### Reclaim record capture
Detailed instructions on enabling this feature can be found in `swap_log_module/README.md`. Here, we just want to take GUPS as an example to showcase how to leverage this feature for your own analysis. A typical profiling flow for GUPS looks like this:

```bash
# Step 1: Create the cgroup and add the target application to it
mkdir /sys/fs/cgroup/swap_log
./gups_app &
echo $$ > /sys/fs/cgroup/swap_log/cgroup.procs

# Step 2: Enable PA-VA mapping updates before memory warm-up
echo 1 > /proc/swap_log_ctl

# Step 3: Start record dumping at the beginning of the execution phase
echo 2 > /proc/swap_log_ctl

# Step 4: After the application finishes, disable the profiler to cleanly exit
echo 0 > /proc/swap_log_ctl
```

### Page reclaim breakdown

For metrics other than reclaim trace, we adopt an always-on/off design. To enable such functionality, use:

```bash
make menuconfig
```

and enable the following option:

```bash
Memory Management options -> Page reclaim time breakdown
```

This feature is always set at kernel compile time and cannot be toggled at runtime. Once it's enabled, **all processes** running on the system will have this feature active. We make this design choice as its overhead is negligible.

Per-thread breakdown data is exposed via procfs at:

```bash
/proc/$pid/page_reclaim_breakdown (e.g. pthread) or
/proc/$pid/task/$tid/page_reclaim_breakdown (e.g., Open-MP)
```

Reading the file resets the counters to zero, allowing repeated reads for interval-based profiling.

---

## Building and running:
Our profiler is integrated into the Linux kernel, so you should use the conventional kernel build process. Please refer to the official documentation for detailed instructions. Please note that on our tested kernel version (v6.9.0), MGLRU is not compiled by default.

---

## Profiler implementation details:
You can find major changes in the following files.
- `mm/vmscan.c`: most profiler logic is implemented here, including stage-level cycle measurement, reclaim metrics collection, and reclaim trace logging
- `mm/rmap.c`: update the PA-VA mapping information for each reclaim candidate
- `fs/proc/base.c`: helper functions for translating cgroup to memcgroup
- `kernel/cgroup/cgroup.c`: limit the reclaim record capture scope to a specific cgroup
- `kernel/fork.c` and `kernel/exit.c`: allocate swap record buffer upon thread creation and flush remaining buffered records upon thread exit; for logging optimization and data integrity
- `include/linux/sched.h`: structure definition for per-thread breakdown data

For more details, please refer to our paper, comments in the code, and commit messages.

We also provide a sample profiler control interface that is implemented as a kernel module for researchers who want to test our tool:
- `swap_log_module/`

## Cite this tool
```bash
@unpublished{liu2025assessing,
  author = {Shaochang Liu and Jie Ren},
  title = {Assessing Page Reclamation Mechanisms for Linux},
  year = {2025},
  month = nov,
  note = {Workshops of the International Conference for High Performance Computing, Networking, Storage and Analysis (SC Workshops ’25), November 16–21, 2025, St Louis, MO, USA},
  publisher = {ACM},
  doi = {10.1145/3731599.3767537},
  url = {https://doi.org/10.1145/3731599.3767537}
}
```
