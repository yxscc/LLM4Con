; ModuleID = '/mlx_devbox/users/mayunlong.39/playground/LLM4Con/kernel_experiment/SYZBOT-3b6b32dc50537a49/src/fs/file_table.c'
source_filename = "/mlx_devbox/users/mayunlong.39/playground/LLM4Con/kernel_experiment/SYZBOT-3b6b32dc50537a49/src/fs/file_table.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

module asm ".section \22.export_symbol\22,\22a\22 ; __export_symbol_backing_file_user_path: ; .asciz \22GPL\22 ; .asciz \22\22 ; .balign 8 ; .quad backing_file_user_path ; .previous"
module asm ".section \22.export_symbol\22,\22a\22 ; __export_symbol_get_max_files: ; .asciz \22GPL\22 ; .asciz \22\22 ; .balign 8 ; .quad get_max_files ; .previous"
module asm ".section \22.export_symbol\22,\22a\22 ; __export_symbol_alloc_file_pseudo: ; .asciz \22\22 ; .asciz \22\22 ; .balign 8 ; .quad alloc_file_pseudo ; .previous"
module asm ".section \22.export_symbol\22,\22a\22 ; __export_symbol_alloc_file_pseudo_noaccount: ; .asciz \22GPL\22 ; .asciz \22\22 ; .balign 8 ; .quad alloc_file_pseudo_noaccount ; .previous"
module asm ".section \22.export_symbol\22,\22a\22 ; __export_symbol_flush_delayed_fput: ; .asciz \22GPL\22 ; .asciz \22\22 ; .balign 8 ; .quad flush_delayed_fput ; .previous"
module asm ".section \22.export_symbol\22,\22a\22 ; __export_symbol_fput: ; .asciz \22\22 ; .asciz \22\22 ; .balign 8 ; .quad fput ; .previous"
module asm ".section \22.export_symbol\22,\22a\22 ; __export_symbol___fput_sync: ; .asciz \22\22 ; .asciz \22\22 ; .balign 8 ; .quad __fput_sync ; .previous"

%struct.files_stat_struct = type { i64, i64, i64 }
%struct.percpu_counter = type { %struct.raw_spinlock, i64, %struct.list_head, ptr }
%struct.raw_spinlock = type { %struct.qspinlock }
%struct.qspinlock = type { %union.anon.4 }
%union.anon.4 = type { %struct.atomic_t }
%struct.atomic_t = type { i32 }
%struct.list_head = type { ptr, ptr }
%struct.llist_head = type { ptr }
%struct.delayed_work = type { %struct.work_struct, %struct.timer_list, ptr, i32 }
%struct.work_struct = type { %struct.atomic64_t, %struct.list_head, ptr }
%struct.atomic64_t = type { i64 }
%struct.timer_list = type { %struct.hlist_node, i64, ptr, i32 }
%struct.hlist_node = type { ptr, ptr }
%struct.kmem_cache_args = type { i32, i32, i32, i32, i8, ptr }
%struct.lock_class_key = type {}
%struct.ctl_table = type { ptr, ptr, i32, i16, ptr, ptr, ptr, ptr }
%struct.pcpu_hot = type { %union.anon.108 }
%union.anon.108 = type { %struct.anon.109, [16 x i8] }
%struct.anon.109 = type { ptr, i32, i32, i64, i64, ptr, i16, i8 }
%struct.backing_file = type { %struct.file, %struct.path }
%struct.file = type { %struct.atomic64_t, %struct.spinlock, i32, ptr, ptr, ptr, ptr, i32, i32, ptr, %struct.path, %union.anon.106, i64, ptr, ptr, i32, i32, ptr, %union.anon.107 }
%struct.spinlock = type { %union.anon.3 }
%union.anon.3 = type { %struct.raw_spinlock }
%union.anon.106 = type { %struct.mutex }
%struct.mutex = type { %struct.atomic64_t, %struct.raw_spinlock, %struct.optimistic_spin_queue, %struct.list_head }
%struct.optimistic_spin_queue = type { %struct.atomic_t }
%union.anon.107 = type { %struct.file_ra_state }
%struct.file_ra_state = type { i64, i32, i32, i32, i32, i64 }
%struct.path = type { ptr, ptr }
%struct.qstr = type { %union.anon, ptr }
%union.anon = type { i64 }
%struct.anon = type { i32, i32 }
%struct.vfsmount = type { ptr, ptr, i32, ptr }
%struct.task_struct = type { %struct.thread_info, i32, i32, ptr, %struct.refcount_struct, i32, i32, i32, %struct.__call_single_node, i32, i64, ptr, i32, i32, i32, i32, i32, i32, i32, %struct.sched_entity, %struct.sched_rt_entity, %struct.sched_dl_entity, ptr, ptr, ptr, %struct.sched_statistics, i32, i32, i64, i32, ptr, ptr, %struct.cpumask, ptr, i16, i16, i32, %union.rcu_special, %struct.list_head, ptr, i64, i8, i8, i32, %struct.list_head, i32, %struct.list_head, %struct.sched_info, %struct.list_head, %struct.plist_node, %struct.rb_node, ptr, ptr, ptr, i32, i32, i32, i32, i64, i32, i8, [3 x i8], i16, i64, %struct.restart_block, i32, i32, i64, ptr, ptr, %struct.list_head, %struct.list_head, ptr, %struct.list_head, %struct.list_head, ptr, [4 x %struct.hlist_node], %struct.list_head, ptr, ptr, ptr, ptr, i64, i64, i64, %struct.prev_cputime, i64, i64, i64, i64, i64, i64, %struct.posix_cputimers, %struct.posix_cputimers_work, ptr, ptr, ptr, ptr, [16 x i8], ptr, %struct.sysv_sem, %struct.sysv_shm, ptr, ptr, ptr, ptr, ptr, ptr, %struct.sigset_t, %struct.sigset_t, %struct.sigset_t, %struct.sigpending, i64, i64, i32, ptr, ptr, %struct.kuid_t, i32, %struct.seccomp, %struct.syscall_user_dispatch, i64, i64, %struct.spinlock, %struct.raw_spinlock, %struct.wake_q_node, %struct.rb_root_cached, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i64, ptr, %struct.task_io_accounting, i64, i64, i64, %struct.nodemask_t, %struct.seqcount_spinlock, i32, ptr, %struct.list_head, ptr, ptr, %struct.list_head, ptr, %struct.mutex, i32, [4 x i8], ptr, %struct.mutex, %struct.list_head, ptr, i16, i8, i16, ptr, i32, i32, i64, i32, i32, i32, i32, %struct.callback_head, %struct.tlbflush_unmap_batch, ptr, %struct.page_frag, ptr, i32, i32, i64, i64, i64, i64, ptr, ptr, %struct.kmap_ctrl, %struct.callback_head, %struct.refcount_struct, i32, ptr, %struct.timer_list, ptr, %struct.refcount_struct, ptr, ptr, ptr, i64, i64, i64, %struct.callback_head, i32, %struct.llist_head, %struct.llist_head, %struct.callback_head, [48 x i8], %struct.thread_struct }
%struct.thread_info = type { i64, i64, i32, i32 }
%struct.__call_single_node = type { %struct.llist_node, %union.anon.58, i16, i16 }
%struct.llist_node = type { ptr }
%union.anon.58 = type { i32 }
%struct.sched_entity = type { %struct.load_weight, %struct.rb_node, i64, i64, i64, %struct.list_head, i8, i8, i8, i8, i64, i64, i64, i64, i64, i64, i64, i32, ptr, ptr, ptr, i64, [8 x i8], %struct.sched_avg }
%struct.load_weight = type { i64, i32 }
%struct.sched_avg = type { i64, i64, i64, i32, i32, i64, i64, i64, i32 }
%struct.sched_rt_entity = type { %struct.list_head, i64, i64, i32, i16, i16, ptr }
%struct.sched_dl_entity = type { %struct.rb_node, i64, i64, i64, i64, i64, i64, i64, i32, i8, %struct.hrtimer, %struct.hrtimer, ptr, ptr, ptr, ptr }
%struct.hrtimer = type { %struct.timerqueue_node, i64, ptr, ptr, i8, i8, i8, i8 }
%struct.timerqueue_node = type { %struct.rb_node, i64 }
%struct.sched_statistics = type { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, [32 x i8] }
%struct.cpumask = type { [1 x i64] }
%union.rcu_special = type { i32 }
%struct.sched_info = type { i64, i64, i64, i64 }
%struct.plist_node = type { i32, %struct.list_head, %struct.list_head }
%struct.rb_node = type { i64, ptr, ptr }
%struct.restart_block = type { i64, ptr, %union.anon.60 }
%union.anon.60 = type { %struct.anon.61 }
%struct.anon.61 = type { ptr, i32, i32, i32, i64, ptr }
%struct.prev_cputime = type { i64, i64, %struct.raw_spinlock }
%struct.posix_cputimers = type { [3 x %struct.posix_cputimer_base], i32, i32 }
%struct.posix_cputimer_base = type { i64, %struct.timerqueue_head }
%struct.timerqueue_head = type { %struct.rb_root_cached }
%struct.posix_cputimers_work = type { %struct.callback_head, %struct.mutex, i32 }
%struct.sysv_sem = type { ptr }
%struct.sysv_shm = type { %struct.list_head }
%struct.sigset_t = type { [1 x i64] }
%struct.sigpending = type { %struct.list_head, %struct.sigset_t }
%struct.kuid_t = type { i32 }
%struct.seccomp = type { i32, %struct.atomic_t, ptr }
%struct.syscall_user_dispatch = type { ptr, i64, i64, i8 }
%struct.wake_q_node = type { ptr }
%struct.rb_root_cached = type { %struct.rb_root, ptr }
%struct.rb_root = type { ptr }
%struct.task_io_accounting = type { i64, i64, i64, i64, i64, i64, i64 }
%struct.nodemask_t = type { [1 x i64] }
%struct.seqcount_spinlock = type { %struct.seqcount }
%struct.seqcount = type { i32 }
%struct.tlbflush_unmap_batch = type { %struct.arch_tlbflush_unmap_batch, i8, i8 }
%struct.arch_tlbflush_unmap_batch = type { %struct.cpumask }
%struct.page_frag = type { ptr, i32, i32 }
%struct.kmap_ctrl = type {}
%struct.refcount_struct = type { %struct.atomic_t }
%struct.callback_head = type { ptr, ptr }
%struct.thread_struct = type { [3 x %struct.desc_struct], i64, i16, i16, i16, i16, i64, i64, [4 x ptr], i64, i64, i64, i64, i64, ptr, i64, i8, i32, [40 x i8], %struct.fpu }
%struct.desc_struct = type { i16, i16, i32 }
%struct.fpu = type { i32, i64, ptr, ptr, %struct.fpu_state_perm, %struct.fpu_state_perm, %struct.fpstate }
%struct.fpu_state_perm = type { i64, i32, i32 }
%struct.fpstate = type { i32, i32, i64, i64, i64, i8, [31 x i8], %union.fpregs_state }
%union.fpregs_state = type { %struct.xregs_state, [3520 x i8] }
%struct.xregs_state = type { %struct.fxregs_state, %struct.xstate_header, [0 x i8] }
%struct.fxregs_state = type { i16, i16, i16, i16, %union.anon.86, i32, i32, [32 x i32], [64 x i32], [12 x i32], %union.anon.89 }
%union.anon.86 = type { %struct.anon.87 }
%struct.anon.87 = type { i64, i64 }
%union.anon.89 = type { [12 x i32] }
%struct.xstate_header = type { i64, i64, [6 x i64] }
%struct.dentry = type { i32, %struct.seqcount_spinlock, %struct.hlist_bl_node, ptr, %struct.qstr, ptr, [40 x i8], ptr, ptr, i64, ptr, %struct.lockref, %union.anon.104, %struct.hlist_node, %struct.hlist_head, %union.anon.105 }
%struct.hlist_bl_node = type { ptr, ptr }
%struct.lockref = type { %union.anon.102 }
%union.anon.102 = type { i64 }
%union.anon.104 = type { %struct.list_head }
%struct.hlist_head = type { ptr }
%union.anon.105 = type { %struct.hlist_node }
%struct.inode = type { i16, i16, %struct.kuid_t, %struct.kgid_t, i32, ptr, ptr, ptr, ptr, ptr, ptr, i64, %union.anon.93, i32, i64, i64, i64, i64, i32, i32, i32, i32, %struct.spinlock, i16, i8, i8, i64, i32, %struct.rw_semaphore, i64, i64, %struct.hlist_node, %struct.list_head, %struct.list_head, %struct.list_head, %struct.list_head, %union.anon.94, %struct.atomic64_t, %struct.atomic64_t, %struct.atomic_t, %struct.atomic_t, %struct.atomic_t, %struct.atomic_t, %union.anon.95, ptr, %struct.address_space, %struct.list_head, %union.anon.101, i32, ptr, ptr }
%struct.kgid_t = type { i32 }
%union.anon.93 = type { i32 }
%struct.rw_semaphore = type { %struct.atomic64_t, %struct.atomic64_t, %struct.optimistic_spin_queue, %struct.raw_spinlock, %struct.list_head }
%union.anon.94 = type { %struct.callback_head }
%union.anon.95 = type { ptr }
%struct.address_space = type { ptr, %struct.xarray, %struct.rw_semaphore, i32, %struct.atomic_t, %struct.rb_root_cached, i64, i64, ptr, i64, i32, %struct.spinlock, %struct.list_head, %struct.rw_semaphore, ptr }
%struct.xarray = type { %struct.spinlock, i32, ptr }
%union.anon.101 = type { ptr }
%struct.file_operations = type { ptr, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%struct.cred = type { %struct.atomic64_t, %struct.kuid_t, %struct.kgid_t, %struct.kuid_t, %struct.kgid_t, %struct.kuid_t, %struct.kgid_t, %struct.kuid_t, %struct.kgid_t, i32, %struct.kernel_cap_t, %struct.kernel_cap_t, %struct.kernel_cap_t, %struct.kernel_cap_t, %struct.kernel_cap_t, i8, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, %union.anon.65 }
%struct.kernel_cap_t = type { i64 }
%union.anon.65 = type { %struct.callback_head }
%struct.super_block = type { %struct.list_head, i32, i8, i64, i64, ptr, ptr, ptr, ptr, ptr, i64, i64, i64, ptr, %struct.rw_semaphore, i32, %struct.atomic_t, ptr, ptr, %struct.hlist_bl_head, %struct.list_head, ptr, ptr, ptr, ptr, %struct.hlist_node, i32, %struct.quota_info, %struct.sb_writers, ptr, i32, i64, i64, i32, ptr, [32 x i8], %struct.uuid_t, i8, [37 x i8], i32, %struct.mutex, ptr, ptr, ptr, %struct.atomic64_t, i32, i32, ptr, %struct.hlist_head, ptr, %struct.list_lru, %struct.list_lru, %struct.callback_head, %struct.work_struct, %struct.mutex, i32, [4 x i8], %struct.spinlock, %struct.list_head, %struct.spinlock, %struct.list_head, [16 x i8] }
%struct.hlist_bl_head = type { ptr }
%struct.quota_info = type { i32, %struct.rw_semaphore, [3 x ptr], [3 x %struct.mem_dqinfo], [3 x ptr] }
%struct.mem_dqinfo = type { ptr, i32, %struct.list_head, i64, i32, i32, i64, i64, ptr }
%struct.sb_writers = type { i16, i32, i32, [3 x %struct.percpu_rw_semaphore] }
%struct.percpu_rw_semaphore = type { %struct.rcu_sync, ptr, %struct.rcuwait, %struct.wait_queue_head, %struct.atomic_t }
%struct.rcu_sync = type { i32, i32, %struct.wait_queue_head, %struct.callback_head }
%struct.rcuwait = type { ptr }
%struct.wait_queue_head = type { %struct.spinlock, %struct.list_head }
%struct.uuid_t = type { [16 x i8] }
%struct.list_lru = type { ptr }
%struct.fsnotify_sb_info = type { ptr, [3 x %struct.atomic64_t] }

@__UNIQUE_ID___addressable_backing_file_user_path506 = internal global ptr @backing_file_user_path, section ".discard.addressable", align 8, !dbg !0
@files_stat = internal global %struct.files_stat_struct { i64 0, i64 0, i64 8192 }, align 8, !dbg !5684
@__UNIQUE_ID___addressable_get_max_files507 = internal global ptr @get_max_files, section ".discard.addressable", align 8, !dbg !5650
@__UNIQUE_ID___addressable_init_module508 = internal global ptr @init_module, section ".init.data", align 8, !dbg !5652
@alloc_empty_file.old_max = internal global i64 0, align 8, !dbg !5654
@nr_files = internal global %struct.percpu_counter zeroinitializer, section ".data..cacheline_aligned", align 64, !dbg !5682
@filp_cachep = internal global ptr null, section ".data..ro_after_init", align 8, !dbg !5680
@.str = private unnamed_addr constant [35 x i8] c"\016VFS: file-max limit %lu reached\0A\00", align 1, !dbg !5659
@__UNIQUE_ID___addressable_alloc_file_pseudo509 = internal global ptr @alloc_file_pseudo, section ".discard.addressable", align 8, !dbg !5664
@__UNIQUE_ID___addressable_alloc_file_pseudo_noaccount510 = internal global ptr @alloc_file_pseudo_noaccount, section ".discard.addressable", align 8, !dbg !5666
@__UNIQUE_ID___addressable_flush_delayed_fput511 = internal global ptr @flush_delayed_fput, section ".discard.addressable", align 8, !dbg !5668
@delayed_fput_list = internal global %struct.llist_head zeroinitializer, align 8, !dbg !5725
@delayed_fput_work = internal global %struct.delayed_work { %struct.work_struct { %struct.atomic64_t { i64 4503599625273344 }, %struct.list_head { ptr getelementptr (i8, ptr @delayed_fput_work, i64 8), ptr getelementptr (i8, ptr @delayed_fput_work, i64 8) }, ptr @delayed_fput }, %struct.timer_list { %struct.hlist_node { ptr inttoptr (i64 -2401263026318605568 to ptr), ptr null }, i64 0, ptr @delayed_work_timer_fn, i32 2097152 }, ptr null, i32 0 }, align 8, !dbg !5727
@__UNIQUE_ID___addressable_fput512 = internal global ptr @fput, section ".discard.addressable", align 8, !dbg !5670
@__UNIQUE_ID___addressable___fput_sync513 = internal global ptr @__fput_sync, section ".discard.addressable", align 8, !dbg !5672
@__const.files_init.args = private unnamed_addr constant %struct.kmem_cache_args { i32 0, i32 0, i32 0, i32 152, i8 1, ptr null }, align 8
@.str.1 = private unnamed_addr constant [5 x i8] c"filp\00", align 1, !dbg !5674
@files_init.__key = internal global %struct.lock_class_key zeroinitializer, align 1, !dbg !5677
@.str.2 = private unnamed_addr constant [3 x i8] c"fs\00", align 1, !dbg !5691
@fs_stat_sysctls = internal global [3 x %struct.ctl_table] [%struct.ctl_table { ptr @.str.5, ptr @files_stat, i32 24, i16 292, ptr @proc_nr_files, ptr null, ptr null, ptr null }, %struct.ctl_table { ptr @.str.6, ptr getelementptr (i8, ptr @files_stat, i64 16), i32 8, i16 420, ptr @proc_doulongvec_minmax, ptr null, ptr @sysctl_long_vals, ptr getelementptr (i8, ptr @sysctl_long_vals, i64 16) }, %struct.ctl_table { ptr @.str.7, ptr @sysctl_nr_open, i32 4, i16 420, ptr @proc_dointvec_minmax, ptr null, ptr @sysctl_nr_open_min, ptr @sysctl_nr_open_max }], align 16, !dbg !5710
@.str.3 = private unnamed_addr constant [16 x i8] c"fs_stat_sysctls\00", align 1, !dbg !5694
@.str.4 = private unnamed_addr constant [15 x i8] c"fs/binfmt_misc\00", align 1, !dbg !5696
@.str.5 = private unnamed_addr constant [8 x i8] c"file-nr\00", align 1, !dbg !5701
@.str.6 = private unnamed_addr constant [9 x i8] c"file-max\00", align 1, !dbg !5703
@sysctl_long_vals = external constant [0 x i64], align 8
@.str.7 = private unnamed_addr constant [8 x i8] c"nr_open\00", align 1, !dbg !5708
@sysctl_nr_open = external global i32, align 4
@sysctl_nr_open_min = external global i32, align 4
@sysctl_nr_open_max = external global i32, align 4
@init_file.__key = internal global %struct.lock_class_key zeroinitializer, align 1, !dbg !5713
@.str.8 = private unnamed_addr constant [15 x i8] c"&f->f_pos_lock\00", align 1, !dbg !5718
@percpu_counter_batch = external global i32, align 4
@kmalloc_caches = external global [3 x [14 x ptr]], align 16
@.str.9 = private unnamed_addr constant [106 x i8] c"/mlx_devbox/users/mayunlong.39/playground/tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/slab.h\00", align 1, !dbg !5720
@const_pcpu_hot = external addrspace(256) constant %struct.pcpu_hot, section ".data..percpu", align 64
@pcpu_hot = external global %struct.pcpu_hot, section ".data..percpu", align 64
@system_wq = external global ptr, align 8
@.str.10 = private unnamed_addr constant [104 x i8] c"/mlx_devbox/users/mayunlong.39/playground/tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/fs.h\00", align 1, !dbg !5729
@_totalram_pages = external global %struct.atomic64_t, align 8
@vm_zone_stat = external global [10 x %struct.atomic64_t], align 16
@llvm.compiler.used = appending global [8 x ptr] [ptr @__UNIQUE_ID___addressable_backing_file_user_path506, ptr @__UNIQUE_ID___addressable_get_max_files507, ptr @__UNIQUE_ID___addressable_init_module508, ptr @__UNIQUE_ID___addressable_alloc_file_pseudo509, ptr @__UNIQUE_ID___addressable_alloc_file_pseudo_noaccount510, ptr @__UNIQUE_ID___addressable_flush_delayed_fput511, ptr @__UNIQUE_ID___addressable_fput512, ptr @__UNIQUE_ID___addressable___fput_sync513], section "llvm.metadata"

@init_module = dso_local alias i32 (), ptr @init_fs_stat_sysctls

; Function Attrs: noinline nounwind optnone uwtable
define dso_local ptr @backing_file_user_path(ptr noundef %0) #0 !dbg !5742 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !5745, metadata !DIExpression()), !dbg !5746
  %3 = load ptr, ptr %2, align 8, !dbg !5747
  %4 = call ptr @backing_file(ptr noundef %3), !dbg !5748
  %5 = getelementptr inbounds %struct.backing_file, ptr %4, i32 0, i32 1, !dbg !5749
  ret ptr %5, !dbg !5750
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @backing_file(ptr noundef %0) #0 !dbg !5751 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !5754, metadata !DIExpression()), !dbg !5755
  call void @llvm.dbg.declare(metadata ptr %3, metadata !5756, metadata !DIExpression()), !dbg !5758
  %5 = load ptr, ptr %2, align 8, !dbg !5758
  store ptr %5, ptr %3, align 8, !dbg !5758
  %6 = load ptr, ptr %3, align 8, !dbg !5758
  %7 = getelementptr i8, ptr %6, i64 0, !dbg !5758
  store ptr %7, ptr %4, align 8, !dbg !5758
  %8 = load ptr, ptr %4, align 8, !dbg !5758
  ret ptr %8, !dbg !5759
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i64 @get_max_files() #0 !dbg !5760 {
  %1 = load i64, ptr getelementptr inbounds (%struct.files_stat_struct, ptr @files_stat, i32 0, i32 2), align 8, !dbg !5763
  ret i64 %1, !dbg !5764
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @init_fs_stat_sysctls() #0 section ".init.text" !dbg !5765 {
  %1 = alloca ptr, align 8
  call void @__register_sysctl_init(ptr noundef @.str.2, ptr noundef @fs_stat_sysctls, ptr noundef @.str.3, i64 noundef 3), !dbg !5766
  call void @llvm.dbg.declare(metadata ptr %1, metadata !5767, metadata !DIExpression()), !dbg !5770
  %2 = call ptr @register_sysctl_mount_point(ptr noundef @.str.4), !dbg !5771
  store ptr %2, ptr %1, align 8, !dbg !5772
  %3 = load ptr, ptr %1, align 8, !dbg !5773
  call void @kmemleak_not_leak(ptr noundef %3), !dbg !5774
  ret i32 0, !dbg !5775
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local ptr @alloc_empty_file(i32 noundef %0, ptr noundef %1) #0 !dbg !5656 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i32, align 4
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  %12 = alloca i32, align 4
  store i32 %0, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !5776, metadata !DIExpression()), !dbg !5777
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !5778, metadata !DIExpression()), !dbg !5779
  call void @llvm.dbg.declare(metadata ptr %6, metadata !5780, metadata !DIExpression()), !dbg !5781
  call void @llvm.dbg.declare(metadata ptr %7, metadata !5782, metadata !DIExpression()), !dbg !5783
  %13 = call i64 @get_nr_files(), !dbg !5784
  %14 = load i64, ptr getelementptr inbounds (%struct.files_stat_struct, ptr @files_stat, i32 0, i32 2), align 8, !dbg !5786
  %15 = icmp uge i64 %13, %14, !dbg !5787
  br i1 %15, label %16, label %24, !dbg !5788

16:                                               ; preds = %2
  %17 = call zeroext i1 @capable(i32 noundef 21), !dbg !5789
  br i1 %17, label %24, label %18, !dbg !5790

18:                                               ; preds = %16
  %19 = call i64 @percpu_counter_sum_positive(ptr noundef @nr_files), !dbg !5791
  %20 = load i64, ptr getelementptr inbounds (%struct.files_stat_struct, ptr @files_stat, i32 0, i32 2), align 8, !dbg !5794
  %21 = icmp uge i64 %19, %20, !dbg !5795
  br i1 %21, label %22, label %23, !dbg !5796

22:                                               ; preds = %18
  br label %62, !dbg !5797

23:                                               ; preds = %18
  br label %24, !dbg !5798

24:                                               ; preds = %23, %16, %2
  call void @llvm.dbg.declare(metadata ptr %9, metadata !5799, metadata !DIExpression()), !dbg !5821
  store ptr null, ptr %9, align 8, !dbg !5821
  call void @llvm.dbg.declare(metadata ptr %10, metadata !5822, metadata !DIExpression()), !dbg !5821
  %25 = load ptr, ptr @filp_cachep, align 8, !dbg !5821
  %26 = call noalias align 8 ptr @kmem_cache_alloc_noprof(ptr noundef %25, i32 noundef 3520), !dbg !5821
  store ptr %26, ptr %10, align 8, !dbg !5821
  br label %27, !dbg !5823

27:                                               ; preds = %24
  br label %28, !dbg !5825

28:                                               ; preds = %27
  %29 = load ptr, ptr %10, align 8, !dbg !5823
  store ptr %29, ptr %11, align 8, !dbg !5825
  %30 = load ptr, ptr %11, align 8, !dbg !5823
  store ptr %30, ptr %8, align 8, !dbg !5827
  %31 = load ptr, ptr %8, align 8, !dbg !5828
  store ptr %31, ptr %6, align 8, !dbg !5829
  %32 = load ptr, ptr %6, align 8, !dbg !5830
  %33 = icmp ne ptr %32, null, !dbg !5830
  %34 = xor i1 %33, true, !dbg !5830
  %35 = xor i1 %34, true, !dbg !5830
  %36 = xor i1 %35, true, !dbg !5830
  %37 = zext i1 %36 to i32, !dbg !5830
  %38 = sext i32 %37 to i64, !dbg !5830
  %39 = icmp ne i64 %38, 0, !dbg !5830
  br i1 %39, label %40, label %42, !dbg !5832

40:                                               ; preds = %28
  %41 = call ptr @ERR_PTR(i64 noundef -12), !dbg !5833
  store ptr %41, ptr %3, align 8, !dbg !5834
  br label %75, !dbg !5834

42:                                               ; preds = %28
  %43 = load ptr, ptr %6, align 8, !dbg !5835
  %44 = load i32, ptr %4, align 4, !dbg !5836
  %45 = load ptr, ptr %5, align 8, !dbg !5837
  %46 = call i32 @init_file(ptr noundef %43, i32 noundef %44, ptr noundef %45), !dbg !5838
  store i32 %46, ptr %7, align 4, !dbg !5839
  %47 = load i32, ptr %7, align 4, !dbg !5840
  %48 = icmp ne i32 %47, 0, !dbg !5840
  %49 = xor i1 %48, true, !dbg !5840
  %50 = xor i1 %49, true, !dbg !5840
  %51 = zext i1 %50 to i32, !dbg !5840
  %52 = sext i32 %51 to i64, !dbg !5840
  %53 = icmp ne i64 %52, 0, !dbg !5840
  br i1 %53, label %54, label %60, !dbg !5842

54:                                               ; preds = %42
  %55 = load ptr, ptr @filp_cachep, align 8, !dbg !5843
  %56 = load ptr, ptr %6, align 8, !dbg !5845
  call void @kmem_cache_free(ptr noundef %55, ptr noundef %56), !dbg !5846
  %57 = load i32, ptr %7, align 4, !dbg !5847
  %58 = sext i32 %57 to i64, !dbg !5847
  %59 = call ptr @ERR_PTR(i64 noundef %58), !dbg !5848
  store ptr %59, ptr %3, align 8, !dbg !5849
  br label %75, !dbg !5849

60:                                               ; preds = %42
  call void @percpu_counter_inc(ptr noundef @nr_files), !dbg !5850
  %61 = load ptr, ptr %6, align 8, !dbg !5851
  store ptr %61, ptr %3, align 8, !dbg !5852
  br label %75, !dbg !5852

62:                                               ; preds = %22
  call void @llvm.dbg.label(metadata !5853), !dbg !5854
  %63 = call i64 @get_nr_files(), !dbg !5855
  %64 = load i64, ptr @alloc_empty_file.old_max, align 8, !dbg !5857
  %65 = icmp sgt i64 %63, %64, !dbg !5858
  br i1 %65, label %66, label %73, !dbg !5859

66:                                               ; preds = %62
  br label %67, !dbg !5860

67:                                               ; preds = %66
  br label %68, !dbg !5863

68:                                               ; preds = %67
  %69 = call i64 @get_max_files(), !dbg !5860
  %70 = call i32 (ptr, ...) @_printk(ptr noundef @.str, i64 noundef %69), !dbg !5860
  store i32 %70, ptr %12, align 4, !dbg !5863
  %71 = load i32, ptr %12, align 4, !dbg !5860
  %72 = call i64 @get_nr_files(), !dbg !5865
  store i64 %72, ptr @alloc_empty_file.old_max, align 8, !dbg !5866
  br label %73, !dbg !5867

73:                                               ; preds = %68, %62
  %74 = call ptr @ERR_PTR(i64 noundef -23), !dbg !5868
  store ptr %74, ptr %3, align 8, !dbg !5869
  br label %75, !dbg !5869

75:                                               ; preds = %73, %60, %54, %40
  %76 = load ptr, ptr %3, align 8, !dbg !5870
  ret ptr %76, !dbg !5870
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i64 @get_nr_files() #0 !dbg !5871 {
  %1 = call i64 @percpu_counter_read_positive(ptr noundef @nr_files), !dbg !5874
  ret i64 %1, !dbg !5875
}

declare zeroext i1 @capable(i32 noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal i64 @percpu_counter_sum_positive(ptr noundef %0) #0 !dbg !5876 {
  %2 = alloca ptr, align 8
  %3 = alloca i64, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !5880, metadata !DIExpression()), !dbg !5881
  call void @llvm.dbg.declare(metadata ptr %3, metadata !5882, metadata !DIExpression()), !dbg !5883
  %4 = load ptr, ptr %2, align 8, !dbg !5884
  %5 = call i64 @__percpu_counter_sum(ptr noundef %4), !dbg !5885
  store i64 %5, ptr %3, align 8, !dbg !5883
  %6 = load i64, ptr %3, align 8, !dbg !5886
  %7 = icmp slt i64 %6, 0, !dbg !5887
  br i1 %7, label %8, label %9, !dbg !5886

8:                                                ; preds = %1
  br label %11, !dbg !5886

9:                                                ; preds = %1
  %10 = load i64, ptr %3, align 8, !dbg !5888
  br label %11, !dbg !5886

11:                                               ; preds = %9, %8
  %12 = phi i64 [ 0, %8 ], [ %10, %9 ], !dbg !5886
  ret i64 %12, !dbg !5889
}

declare noalias ptr @kmem_cache_alloc_noprof(ptr noundef, i32 noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @ERR_PTR(i64 noundef %0) #0 !dbg !5890 {
  %2 = alloca i64, align 8
  store i64 %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !5894, metadata !DIExpression()), !dbg !5895
  %3 = load i64, ptr %2, align 8, !dbg !5896
  %4 = inttoptr i64 %3 to ptr, !dbg !5897
  ret ptr %4, !dbg !5898
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @init_file(ptr noundef %0, i32 noundef %1, ptr noundef %2) #0 !dbg !5715 {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i64, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i64, align 8
  %10 = alloca ptr, align 8
  %11 = alloca i64, align 8
  %12 = alloca ptr, align 8
  %13 = alloca i64, align 8
  %14 = alloca ptr, align 8
  %15 = alloca i32, align 4
  %16 = alloca ptr, align 8
  %17 = alloca i32, align 4
  %18 = alloca ptr, align 8
  %19 = alloca i32, align 4
  %20 = alloca %struct.spinlock, align 4
  store ptr %0, ptr %16, align 8
  call void @llvm.dbg.declare(metadata ptr %16, metadata !5899, metadata !DIExpression()), !dbg !5900
  store i32 %1, ptr %17, align 4
  call void @llvm.dbg.declare(metadata ptr %17, metadata !5901, metadata !DIExpression()), !dbg !5902
  store ptr %2, ptr %18, align 8
  call void @llvm.dbg.declare(metadata ptr %18, metadata !5903, metadata !DIExpression()), !dbg !5904
  call void @llvm.dbg.declare(metadata ptr %19, metadata !5905, metadata !DIExpression()), !dbg !5906
  %21 = load ptr, ptr %18, align 8, !dbg !5907
  %22 = call ptr @get_cred(ptr noundef %21), !dbg !5908
  %23 = load ptr, ptr %16, align 8, !dbg !5909
  %24 = getelementptr inbounds %struct.file, ptr %23, i32 0, i32 9, !dbg !5910
  store ptr %22, ptr %24, align 8, !dbg !5911
  %25 = load ptr, ptr %16, align 8, !dbg !5912
  %26 = call i32 @security_file_alloc(ptr noundef %25), !dbg !5913
  store i32 %26, ptr %19, align 4, !dbg !5914
  %27 = load i32, ptr %19, align 4, !dbg !5915
  %28 = icmp ne i32 %27, 0, !dbg !5915
  %29 = xor i1 %28, true, !dbg !5915
  %30 = xor i1 %29, true, !dbg !5915
  %31 = zext i1 %30 to i32, !dbg !5915
  %32 = sext i32 %31 to i64, !dbg !5915
  %33 = icmp ne i64 %32, 0, !dbg !5915
  br i1 %33, label %34, label %39, !dbg !5917

34:                                               ; preds = %3
  %35 = load ptr, ptr %16, align 8, !dbg !5918
  %36 = getelementptr inbounds %struct.file, ptr %35, i32 0, i32 9, !dbg !5920
  %37 = load ptr, ptr %36, align 8, !dbg !5920
  call void @put_cred(ptr noundef %37), !dbg !5921
  %38 = load i32, ptr %19, align 4, !dbg !5922
  store i32 %38, ptr %15, align 4, !dbg !5923
  br label %83, !dbg !5923

39:                                               ; preds = %3
  br label %40, !dbg !5924

40:                                               ; preds = %39
  %41 = load ptr, ptr %16, align 8, !dbg !5925
  %42 = getelementptr inbounds %struct.file, ptr %41, i32 0, i32 1, !dbg !5925
  store ptr %42, ptr %14, align 8
  call void @llvm.dbg.declare(metadata ptr %14, metadata !5927, metadata !DIExpression()), !dbg !5933
  %43 = load ptr, ptr %14, align 8, !dbg !5935
  %44 = load ptr, ptr %16, align 8, !dbg !5925
  %45 = getelementptr inbounds %struct.file, ptr %44, i32 0, i32 1, !dbg !5925
  %46 = getelementptr inbounds %struct.spinlock, ptr %20, i32 0, i32 0, !dbg !5925
  %47 = getelementptr inbounds %struct.raw_spinlock, ptr %46, i32 0, i32 0, !dbg !5925
  %48 = getelementptr inbounds %struct.qspinlock, ptr %47, i32 0, i32 0, !dbg !5925
  %49 = getelementptr inbounds %struct.atomic_t, ptr %48, i32 0, i32 0, !dbg !5925
  store i32 0, ptr %49, align 4, !dbg !5925
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %45, ptr align 4 %20, i64 4, i1 false), !dbg !5925
  br label %50, !dbg !5925

50:                                               ; preds = %40
  br label %51, !dbg !5936

51:                                               ; preds = %50
  %52 = load ptr, ptr %16, align 8, !dbg !5937
  %53 = getelementptr inbounds %struct.file, ptr %52, i32 0, i32 11, !dbg !5937
  call void @__mutex_init(ptr noundef %53, ptr noundef @.str.8, ptr noundef @init_file.__key), !dbg !5937
  br label %54, !dbg !5937

54:                                               ; preds = %51
  %55 = load i32, ptr %17, align 4, !dbg !5939
  %56 = load ptr, ptr %16, align 8, !dbg !5940
  %57 = getelementptr inbounds %struct.file, ptr %56, i32 0, i32 7, !dbg !5941
  store i32 %55, ptr %57, align 8, !dbg !5942
  %58 = load i32, ptr %17, align 4, !dbg !5943
  %59 = add nsw i32 %58, 1, !dbg !5943
  %60 = and i32 %59, 3, !dbg !5943
  %61 = load i32, ptr %17, align 4, !dbg !5943
  %62 = and i32 %61, 67108864, !dbg !5943
  %63 = or i32 %60, %62, !dbg !5943
  %64 = load ptr, ptr %16, align 8, !dbg !5944
  %65 = getelementptr inbounds %struct.file, ptr %64, i32 0, i32 2, !dbg !5945
  store i32 %63, ptr %65, align 4, !dbg !5946
  %66 = load ptr, ptr %16, align 8, !dbg !5947
  %67 = getelementptr inbounds %struct.file, ptr %66, i32 0, i32 0, !dbg !5948
  store ptr %67, ptr %12, align 8
  call void @llvm.dbg.declare(metadata ptr %12, metadata !5949, metadata !DIExpression()), !dbg !5954
  store i64 1, ptr %13, align 8
  call void @llvm.dbg.declare(metadata ptr %13, metadata !5956, metadata !DIExpression()), !dbg !5957
  %68 = load ptr, ptr %12, align 8, !dbg !5958
  store ptr %68, ptr %10, align 8
  call void @llvm.dbg.declare(metadata ptr %10, metadata !5959, metadata !DIExpression()), !dbg !5967
  store i64 8, ptr %11, align 8
  call void @llvm.dbg.declare(metadata ptr %11, metadata !5969, metadata !DIExpression()), !dbg !5970
  %69 = load ptr, ptr %10, align 8, !dbg !5971
  %70 = load i64, ptr %11, align 8, !dbg !5972
  %71 = trunc i64 %70 to i32, !dbg !5972
  %72 = call zeroext i1 @kasan_check_write(ptr noundef %69, i32 noundef %71), !dbg !5973
  %73 = load ptr, ptr %10, align 8, !dbg !5974
  %74 = load i64, ptr %11, align 8, !dbg !5974
  call void @kcsan_check_access(ptr noundef %73, i64 noundef %74, i32 noundef 5), !dbg !5974
  %75 = load ptr, ptr %12, align 8, !dbg !5975
  %76 = load i64, ptr %13, align 8, !dbg !5976
  store ptr %75, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !5977, metadata !DIExpression()), !dbg !5979
  store i64 %76, ptr %9, align 8
  call void @llvm.dbg.declare(metadata ptr %9, metadata !5981, metadata !DIExpression()), !dbg !5982
  %77 = load ptr, ptr %8, align 8, !dbg !5983
  %78 = load i64, ptr %9, align 8, !dbg !5984
  store ptr %77, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !5985, metadata !DIExpression()), !dbg !5991
  store i64 %78, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !5993, metadata !DIExpression()), !dbg !5994
  %79 = load ptr, ptr %6, align 8, !dbg !5995
  %80 = load i64, ptr %7, align 8, !dbg !5996
  store ptr %79, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !5997, metadata !DIExpression()), !dbg !6000
  store i64 %80, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6002, metadata !DIExpression()), !dbg !6003
  %81 = load i64, ptr %5, align 8, !dbg !6004
  %82 = load ptr, ptr %4, align 8, !dbg !6004
  store volatile i64 %81, ptr %82, align 8, !dbg !6004
  store i32 0, ptr %15, align 4, !dbg !6006
  br label %83, !dbg !6006

83:                                               ; preds = %54, %34
  %84 = load i32, ptr %15, align 4, !dbg !6007
  ret i32 %84, !dbg !6007
}

declare void @kmem_cache_free(ptr noundef, ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal void @percpu_counter_inc(ptr noundef %0) #0 !dbg !6008 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !6011, metadata !DIExpression()), !dbg !6012
  %3 = load ptr, ptr %2, align 8, !dbg !6013
  call void @percpu_counter_add(ptr noundef %3, i64 noundef 1), !dbg !6014
  ret void, !dbg !6015
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.label(metadata) #1

declare i32 @_printk(ptr noundef, ...) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local ptr @alloc_empty_file_noaccount(i32 noundef %0, ptr noundef %1) #0 !dbg !6016 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i32, align 4
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  store i32 %0, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !6017, metadata !DIExpression()), !dbg !6018
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6019, metadata !DIExpression()), !dbg !6020
  call void @llvm.dbg.declare(metadata ptr %6, metadata !6021, metadata !DIExpression()), !dbg !6022
  call void @llvm.dbg.declare(metadata ptr %7, metadata !6023, metadata !DIExpression()), !dbg !6024
  call void @llvm.dbg.declare(metadata ptr %9, metadata !6025, metadata !DIExpression()), !dbg !6028
  store ptr null, ptr %9, align 8, !dbg !6028
  call void @llvm.dbg.declare(metadata ptr %10, metadata !6029, metadata !DIExpression()), !dbg !6028
  %12 = load ptr, ptr @filp_cachep, align 8, !dbg !6028
  %13 = call noalias align 8 ptr @kmem_cache_alloc_noprof(ptr noundef %12, i32 noundef 3520), !dbg !6028
  store ptr %13, ptr %10, align 8, !dbg !6028
  br label %14, !dbg !6030

14:                                               ; preds = %2
  br label %15, !dbg !6032

15:                                               ; preds = %14
  %16 = load ptr, ptr %10, align 8, !dbg !6030
  store ptr %16, ptr %11, align 8, !dbg !6032
  %17 = load ptr, ptr %11, align 8, !dbg !6030
  store ptr %17, ptr %8, align 8, !dbg !6034
  %18 = load ptr, ptr %8, align 8, !dbg !6035
  store ptr %18, ptr %6, align 8, !dbg !6036
  %19 = load ptr, ptr %6, align 8, !dbg !6037
  %20 = icmp ne ptr %19, null, !dbg !6037
  %21 = xor i1 %20, true, !dbg !6037
  %22 = xor i1 %21, true, !dbg !6037
  %23 = xor i1 %22, true, !dbg !6037
  %24 = zext i1 %23 to i32, !dbg !6037
  %25 = sext i32 %24 to i64, !dbg !6037
  %26 = icmp ne i64 %25, 0, !dbg !6037
  br i1 %26, label %27, label %29, !dbg !6039

27:                                               ; preds = %15
  %28 = call ptr @ERR_PTR(i64 noundef -12), !dbg !6040
  store ptr %28, ptr %3, align 8, !dbg !6041
  br label %53, !dbg !6041

29:                                               ; preds = %15
  %30 = load ptr, ptr %6, align 8, !dbg !6042
  %31 = load i32, ptr %4, align 4, !dbg !6043
  %32 = load ptr, ptr %5, align 8, !dbg !6044
  %33 = call i32 @init_file(ptr noundef %30, i32 noundef %31, ptr noundef %32), !dbg !6045
  store i32 %33, ptr %7, align 4, !dbg !6046
  %34 = load i32, ptr %7, align 4, !dbg !6047
  %35 = icmp ne i32 %34, 0, !dbg !6047
  %36 = xor i1 %35, true, !dbg !6047
  %37 = xor i1 %36, true, !dbg !6047
  %38 = zext i1 %37 to i32, !dbg !6047
  %39 = sext i32 %38 to i64, !dbg !6047
  %40 = icmp ne i64 %39, 0, !dbg !6047
  br i1 %40, label %41, label %47, !dbg !6049

41:                                               ; preds = %29
  %42 = load ptr, ptr @filp_cachep, align 8, !dbg !6050
  %43 = load ptr, ptr %6, align 8, !dbg !6052
  call void @kmem_cache_free(ptr noundef %42, ptr noundef %43), !dbg !6053
  %44 = load i32, ptr %7, align 4, !dbg !6054
  %45 = sext i32 %44 to i64, !dbg !6054
  %46 = call ptr @ERR_PTR(i64 noundef %45), !dbg !6055
  store ptr %46, ptr %3, align 8, !dbg !6056
  br label %53, !dbg !6056

47:                                               ; preds = %29
  %48 = load ptr, ptr %6, align 8, !dbg !6057
  %49 = getelementptr inbounds %struct.file, ptr %48, i32 0, i32 2, !dbg !6058
  %50 = load i32, ptr %49, align 4, !dbg !6059
  %51 = or i32 %50, 536870912, !dbg !6059
  store i32 %51, ptr %49, align 4, !dbg !6059
  %52 = load ptr, ptr %6, align 8, !dbg !6060
  store ptr %52, ptr %3, align 8, !dbg !6061
  br label %53, !dbg !6061

53:                                               ; preds = %47, %41, %27
  %54 = load ptr, ptr %3, align 8, !dbg !6062
  ret ptr %54, !dbg !6062
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local ptr @alloc_empty_backing_file(i32 noundef %0, ptr noundef %1) #0 !dbg !6063 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i32, align 4
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  store i32 %0, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !6064, metadata !DIExpression()), !dbg !6065
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6066, metadata !DIExpression()), !dbg !6067
  call void @llvm.dbg.declare(metadata ptr %6, metadata !6068, metadata !DIExpression()), !dbg !6069
  call void @llvm.dbg.declare(metadata ptr %7, metadata !6070, metadata !DIExpression()), !dbg !6071
  call void @llvm.dbg.declare(metadata ptr %9, metadata !6072, metadata !DIExpression()), !dbg !6075
  store ptr null, ptr %9, align 8, !dbg !6075
  call void @llvm.dbg.declare(metadata ptr %10, metadata !6076, metadata !DIExpression()), !dbg !6075
  %12 = call noalias ptr @kzalloc_noprof(i64 noundef 200, i32 noundef 3264) #10, !dbg !6075
  store ptr %12, ptr %10, align 8, !dbg !6075
  br label %13, !dbg !6075

13:                                               ; preds = %2
  br label %14, !dbg !6077

14:                                               ; preds = %13
  %15 = load ptr, ptr %10, align 8, !dbg !6075
  store ptr %15, ptr %11, align 8, !dbg !6077
  %16 = load ptr, ptr %11, align 8, !dbg !6075
  store ptr %16, ptr %8, align 8, !dbg !6079
  %17 = load ptr, ptr %8, align 8, !dbg !6080
  store ptr %17, ptr %6, align 8, !dbg !6081
  %18 = load ptr, ptr %6, align 8, !dbg !6082
  %19 = icmp ne ptr %18, null, !dbg !6082
  %20 = xor i1 %19, true, !dbg !6082
  %21 = xor i1 %20, true, !dbg !6082
  %22 = xor i1 %21, true, !dbg !6082
  %23 = zext i1 %22 to i32, !dbg !6082
  %24 = sext i32 %23 to i64, !dbg !6082
  %25 = icmp ne i64 %24, 0, !dbg !6082
  br i1 %25, label %26, label %28, !dbg !6084

26:                                               ; preds = %14
  %27 = call ptr @ERR_PTR(i64 noundef -12), !dbg !6085
  store ptr %27, ptr %3, align 8, !dbg !6086
  br label %54, !dbg !6086

28:                                               ; preds = %14
  %29 = load ptr, ptr %6, align 8, !dbg !6087
  %30 = getelementptr inbounds %struct.backing_file, ptr %29, i32 0, i32 0, !dbg !6088
  %31 = load i32, ptr %4, align 4, !dbg !6089
  %32 = load ptr, ptr %5, align 8, !dbg !6090
  %33 = call i32 @init_file(ptr noundef %30, i32 noundef %31, ptr noundef %32), !dbg !6091
  store i32 %33, ptr %7, align 4, !dbg !6092
  %34 = load i32, ptr %7, align 4, !dbg !6093
  %35 = icmp ne i32 %34, 0, !dbg !6093
  %36 = xor i1 %35, true, !dbg !6093
  %37 = xor i1 %36, true, !dbg !6093
  %38 = zext i1 %37 to i32, !dbg !6093
  %39 = sext i32 %38 to i64, !dbg !6093
  %40 = icmp ne i64 %39, 0, !dbg !6093
  br i1 %40, label %41, label %46, !dbg !6095

41:                                               ; preds = %28
  %42 = load ptr, ptr %6, align 8, !dbg !6096
  call void @kfree(ptr noundef %42), !dbg !6098
  %43 = load i32, ptr %7, align 4, !dbg !6099
  %44 = sext i32 %43 to i64, !dbg !6099
  %45 = call ptr @ERR_PTR(i64 noundef %44), !dbg !6100
  store ptr %45, ptr %3, align 8, !dbg !6101
  br label %54, !dbg !6101

46:                                               ; preds = %28
  %47 = load ptr, ptr %6, align 8, !dbg !6102
  %48 = getelementptr inbounds %struct.backing_file, ptr %47, i32 0, i32 0, !dbg !6103
  %49 = getelementptr inbounds %struct.file, ptr %48, i32 0, i32 2, !dbg !6104
  %50 = load i32, ptr %49, align 4, !dbg !6105
  %51 = or i32 %50, 570425344, !dbg !6105
  store i32 %51, ptr %49, align 4, !dbg !6105
  %52 = load ptr, ptr %6, align 8, !dbg !6106
  %53 = getelementptr inbounds %struct.backing_file, ptr %52, i32 0, i32 0, !dbg !6107
  store ptr %53, ptr %3, align 8, !dbg !6108
  br label %54, !dbg !6108

54:                                               ; preds = %46, %41, %26
  %55 = load ptr, ptr %3, align 8, !dbg !6109
  ret ptr %55, !dbg !6109
}

; Function Attrs: noinline nounwind optnone allocsize(0) uwtable
define internal noalias ptr @kzalloc_noprof(i64 noundef %0, i32 noundef %1) #3 !dbg !6110 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i64, align 8
  %6 = alloca i32, align 4
  %7 = alloca i64, align 8
  %8 = alloca i8, align 1
  %9 = alloca ptr, align 8
  %10 = alloca i64, align 8
  %11 = alloca i32, align 4
  %12 = alloca i32, align 4
  %13 = alloca i64, align 8
  %14 = alloca i32, align 4
  store i64 %0, ptr %13, align 8
  call void @llvm.dbg.declare(metadata ptr %13, metadata !6113, metadata !DIExpression()), !dbg !6114
  store i32 %1, ptr %14, align 4
  call void @llvm.dbg.declare(metadata ptr %14, metadata !6115, metadata !DIExpression()), !dbg !6116
  %15 = load i64, ptr %13, align 8, !dbg !6117
  %16 = load i32, ptr %14, align 4, !dbg !6118
  %17 = or i32 %16, 256, !dbg !6119
  store i64 %15, ptr %10, align 8
  call void @llvm.dbg.declare(metadata ptr %10, metadata !6120, metadata !DIExpression()), !dbg !6122
  store i32 %17, ptr %11, align 4
  call void @llvm.dbg.declare(metadata ptr %11, metadata !6124, metadata !DIExpression()), !dbg !6125
  %18 = load i64, ptr %10, align 8, !dbg !6126
  %19 = call i1 @llvm.is.constant.i64(i64 %18), !dbg !6128
  br i1 %19, label %20, label %163, !dbg !6129

20:                                               ; preds = %2
  %21 = load i64, ptr %10, align 8, !dbg !6130
  %22 = icmp ne i64 %21, 0, !dbg !6130
  br i1 %22, label %23, label %163, !dbg !6131

23:                                               ; preds = %20
  call void @llvm.dbg.declare(metadata ptr %12, metadata !6132, metadata !DIExpression()), !dbg !6134
  %24 = load i64, ptr %10, align 8, !dbg !6135
  %25 = icmp ugt i64 %24, 8192, !dbg !6137
  br i1 %25, label %26, label %30, !dbg !6138

26:                                               ; preds = %23
  %27 = load i64, ptr %10, align 8, !dbg !6139
  %28 = load i32, ptr %11, align 4, !dbg !6140
  %29 = call noalias align 4096 ptr @__kmalloc_large_noprof(i64 noundef %27, i32 noundef %28) #11, !dbg !6141
  store ptr %29, ptr %9, align 8, !dbg !6142
  br label %167, !dbg !6142

30:                                               ; preds = %23
  %31 = load i64, ptr %10, align 8, !dbg !6143
  store i64 %31, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !6144, metadata !DIExpression()), !dbg !6148
  store i8 1, ptr %8, align 1
  call void @llvm.dbg.declare(metadata ptr %8, metadata !6150, metadata !DIExpression()), !dbg !6151
  %32 = load i64, ptr %7, align 8, !dbg !6152
  %33 = icmp ne i64 %32, 0, !dbg !6152
  br i1 %33, label %35, label %34, !dbg !6154

34:                                               ; preds = %30
  store i32 0, ptr %6, align 4, !dbg !6155
  br label %134, !dbg !6155

35:                                               ; preds = %30
  %36 = load i64, ptr %7, align 8, !dbg !6156
  %37 = icmp ule i64 %36, 8, !dbg !6158
  br i1 %37, label %38, label %39, !dbg !6159

38:                                               ; preds = %35
  store i32 3, ptr %6, align 4, !dbg !6160
  br label %134, !dbg !6160

39:                                               ; preds = %35
  %40 = load i64, ptr %7, align 8, !dbg !6161
  %41 = icmp ugt i64 %40, 64, !dbg !6163
  br i1 %41, label %42, label %46, !dbg !6164

42:                                               ; preds = %39
  %43 = load i64, ptr %7, align 8, !dbg !6165
  %44 = icmp ule i64 %43, 96, !dbg !6166
  br i1 %44, label %45, label %46, !dbg !6167

45:                                               ; preds = %42
  store i32 1, ptr %6, align 4, !dbg !6168
  br label %134, !dbg !6168

46:                                               ; preds = %42, %39
  %47 = load i64, ptr %7, align 8, !dbg !6169
  %48 = icmp ugt i64 %47, 128, !dbg !6171
  br i1 %48, label %49, label %53, !dbg !6172

49:                                               ; preds = %46
  %50 = load i64, ptr %7, align 8, !dbg !6173
  %51 = icmp ule i64 %50, 192, !dbg !6174
  br i1 %51, label %52, label %53, !dbg !6175

52:                                               ; preds = %49
  store i32 2, ptr %6, align 4, !dbg !6176
  br label %134, !dbg !6176

53:                                               ; preds = %49, %46
  %54 = load i64, ptr %7, align 8, !dbg !6177
  %55 = icmp ule i64 %54, 8, !dbg !6179
  br i1 %55, label %56, label %57, !dbg !6180

56:                                               ; preds = %53
  store i32 3, ptr %6, align 4, !dbg !6181
  br label %134, !dbg !6181

57:                                               ; preds = %53
  %58 = load i64, ptr %7, align 8, !dbg !6182
  %59 = icmp ule i64 %58, 16, !dbg !6184
  br i1 %59, label %60, label %61, !dbg !6185

60:                                               ; preds = %57
  store i32 4, ptr %6, align 4, !dbg !6186
  br label %134, !dbg !6186

61:                                               ; preds = %57
  %62 = load i64, ptr %7, align 8, !dbg !6187
  %63 = icmp ule i64 %62, 32, !dbg !6189
  br i1 %63, label %64, label %65, !dbg !6190

64:                                               ; preds = %61
  store i32 5, ptr %6, align 4, !dbg !6191
  br label %134, !dbg !6191

65:                                               ; preds = %61
  %66 = load i64, ptr %7, align 8, !dbg !6192
  %67 = icmp ule i64 %66, 64, !dbg !6194
  br i1 %67, label %68, label %69, !dbg !6195

68:                                               ; preds = %65
  store i32 6, ptr %6, align 4, !dbg !6196
  br label %134, !dbg !6196

69:                                               ; preds = %65
  %70 = load i64, ptr %7, align 8, !dbg !6197
  %71 = icmp ule i64 %70, 128, !dbg !6199
  br i1 %71, label %72, label %73, !dbg !6200

72:                                               ; preds = %69
  store i32 7, ptr %6, align 4, !dbg !6201
  br label %134, !dbg !6201

73:                                               ; preds = %69
  %74 = load i64, ptr %7, align 8, !dbg !6202
  %75 = icmp ule i64 %74, 256, !dbg !6204
  br i1 %75, label %76, label %77, !dbg !6205

76:                                               ; preds = %73
  store i32 8, ptr %6, align 4, !dbg !6206
  br label %134, !dbg !6206

77:                                               ; preds = %73
  %78 = load i64, ptr %7, align 8, !dbg !6207
  %79 = icmp ule i64 %78, 512, !dbg !6209
  br i1 %79, label %80, label %81, !dbg !6210

80:                                               ; preds = %77
  store i32 9, ptr %6, align 4, !dbg !6211
  br label %134, !dbg !6211

81:                                               ; preds = %77
  %82 = load i64, ptr %7, align 8, !dbg !6212
  %83 = icmp ule i64 %82, 1024, !dbg !6214
  br i1 %83, label %84, label %85, !dbg !6215

84:                                               ; preds = %81
  store i32 10, ptr %6, align 4, !dbg !6216
  br label %134, !dbg !6216

85:                                               ; preds = %81
  %86 = load i64, ptr %7, align 8, !dbg !6217
  %87 = icmp ule i64 %86, 2048, !dbg !6219
  br i1 %87, label %88, label %89, !dbg !6220

88:                                               ; preds = %85
  store i32 11, ptr %6, align 4, !dbg !6221
  br label %134, !dbg !6221

89:                                               ; preds = %85
  %90 = load i64, ptr %7, align 8, !dbg !6222
  %91 = icmp ule i64 %90, 4096, !dbg !6224
  br i1 %91, label %92, label %93, !dbg !6225

92:                                               ; preds = %89
  store i32 12, ptr %6, align 4, !dbg !6226
  br label %134, !dbg !6226

93:                                               ; preds = %89
  %94 = load i64, ptr %7, align 8, !dbg !6227
  %95 = icmp ule i64 %94, 8192, !dbg !6229
  br i1 %95, label %96, label %97, !dbg !6230

96:                                               ; preds = %93
  store i32 13, ptr %6, align 4, !dbg !6231
  br label %134, !dbg !6231

97:                                               ; preds = %93
  %98 = load i64, ptr %7, align 8, !dbg !6232
  %99 = icmp ule i64 %98, 16384, !dbg !6234
  br i1 %99, label %100, label %101, !dbg !6235

100:                                              ; preds = %97
  store i32 14, ptr %6, align 4, !dbg !6236
  br label %134, !dbg !6236

101:                                              ; preds = %97
  %102 = load i64, ptr %7, align 8, !dbg !6237
  %103 = icmp ule i64 %102, 32768, !dbg !6239
  br i1 %103, label %104, label %105, !dbg !6240

104:                                              ; preds = %101
  store i32 15, ptr %6, align 4, !dbg !6241
  br label %134, !dbg !6241

105:                                              ; preds = %101
  %106 = load i64, ptr %7, align 8, !dbg !6242
  %107 = icmp ule i64 %106, 65536, !dbg !6244
  br i1 %107, label %108, label %109, !dbg !6245

108:                                              ; preds = %105
  store i32 16, ptr %6, align 4, !dbg !6246
  br label %134, !dbg !6246

109:                                              ; preds = %105
  %110 = load i64, ptr %7, align 8, !dbg !6247
  %111 = icmp ule i64 %110, 131072, !dbg !6249
  br i1 %111, label %112, label %113, !dbg !6250

112:                                              ; preds = %109
  store i32 17, ptr %6, align 4, !dbg !6251
  br label %134, !dbg !6251

113:                                              ; preds = %109
  %114 = load i64, ptr %7, align 8, !dbg !6252
  %115 = icmp ule i64 %114, 262144, !dbg !6254
  br i1 %115, label %116, label %117, !dbg !6255

116:                                              ; preds = %113
  store i32 18, ptr %6, align 4, !dbg !6256
  br label %134, !dbg !6256

117:                                              ; preds = %113
  %118 = load i64, ptr %7, align 8, !dbg !6257
  %119 = icmp ule i64 %118, 524288, !dbg !6259
  br i1 %119, label %120, label %121, !dbg !6260

120:                                              ; preds = %117
  store i32 19, ptr %6, align 4, !dbg !6261
  br label %134, !dbg !6261

121:                                              ; preds = %117
  %122 = load i64, ptr %7, align 8, !dbg !6262
  %123 = icmp ule i64 %122, 1048576, !dbg !6264
  br i1 %123, label %124, label %125, !dbg !6265

124:                                              ; preds = %121
  store i32 20, ptr %6, align 4, !dbg !6266
  br label %134, !dbg !6266

125:                                              ; preds = %121
  %126 = load i64, ptr %7, align 8, !dbg !6267
  %127 = icmp ule i64 %126, 2097152, !dbg !6269
  br i1 %127, label %128, label %129, !dbg !6270

128:                                              ; preds = %125
  store i32 21, ptr %6, align 4, !dbg !6271
  br label %134, !dbg !6271

129:                                              ; preds = %125
  %130 = load i8, ptr %8, align 1, !dbg !6272
  %131 = trunc i8 %130 to i1, !dbg !6272
  br i1 %131, label %132, label %133, !dbg !6274

132:                                              ; preds = %129
  store i32 -1, ptr %6, align 4, !dbg !6275
  br label %134, !dbg !6275

133:                                              ; preds = %129
  call void asm sideeffect "153: nop\0A\09.pushsection .discard.instr_begin\0A\09.long 153b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 153) #12, !dbg !6276, !srcloc !6279
  call void asm sideeffect "1:\09.byte 0x0f, 0x0b\0A.pushsection __bug_table,\22aw\22\0A2:\09.long 1b - .\09# bug_entry::bug_addr\0A\09.long ${0:c} - .\09# bug_entry::file\0A\09.word ${1:c}\09# bug_entry::line\0A\09.word ${2:c}\09# bug_entry::flags\0A\09.org 2b+${3:c}\0A.popsection\0A", "i,i,i,i,~{dirflag},~{fpsr},~{flags}"(ptr @.str.9, i32 690, i32 0, i64 12) #12, !dbg !6280, !srcloc !6282
  unreachable, !dbg !6283

134:                                              ; preds = %34, %38, %45, %52, %56, %60, %64, %68, %72, %76, %80, %84, %88, %92, %96, %100, %104, %108, %112, %116, %120, %124, %128, %132
  %135 = load i32, ptr %6, align 4, !dbg !6284
  store i32 %135, ptr %12, align 4, !dbg !6285
  %136 = load i32, ptr %11, align 4, !dbg !6286
  %137 = call ptr @llvm.returnaddress(i32 0), !dbg !6287
  %138 = ptrtoint ptr %137 to i64, !dbg !6287
  store i32 %136, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !6288, metadata !DIExpression()), !dbg !6292
  store i64 %138, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6294, metadata !DIExpression()), !dbg !6295
  %139 = load i32, ptr %4, align 4, !dbg !6296
  %140 = and i32 %139, 17, !dbg !6296
  %141 = icmp eq i32 %140, 0, !dbg !6296
  %142 = xor i1 %141, true, !dbg !6296
  %143 = zext i1 %141 to i32, !dbg !6296
  %144 = sext i32 %143 to i64, !dbg !6296
  br i1 %141, label %145, label %146, !dbg !6298

145:                                              ; preds = %134
  store i32 0, ptr %3, align 4, !dbg !6299
  br label %152, !dbg !6299

146:                                              ; preds = %134
  %147 = load i32, ptr %4, align 4, !dbg !6300
  %148 = and i32 %147, 1, !dbg !6302
  %149 = icmp ne i32 %148, 0, !dbg !6302
  br i1 %149, label %150, label %151, !dbg !6303

150:                                              ; preds = %146
  store i32 2, ptr %3, align 4, !dbg !6304
  br label %152, !dbg !6304

151:                                              ; preds = %146
  store i32 1, ptr %3, align 4, !dbg !6305
  br label %152, !dbg !6305

152:                                              ; preds = %145, %150, %151
  %153 = load i32, ptr %3, align 4, !dbg !6307
  %154 = zext i32 %153 to i64, !dbg !6308
  %155 = getelementptr inbounds [3 x [14 x ptr]], ptr @kmalloc_caches, i64 0, i64 %154, !dbg !6308
  %156 = load i32, ptr %12, align 4, !dbg !6309
  %157 = zext i32 %156 to i64, !dbg !6308
  %158 = getelementptr inbounds [14 x ptr], ptr %155, i64 0, i64 %157, !dbg !6308
  %159 = load ptr, ptr %158, align 8, !dbg !6308
  %160 = load i32, ptr %11, align 4, !dbg !6310
  %161 = load i64, ptr %10, align 8, !dbg !6311
  %162 = call noalias align 8 ptr @__kmalloc_cache_noprof(ptr noundef %159, i32 noundef %160, i64 noundef %161) #13, !dbg !6312
  store ptr %162, ptr %9, align 8, !dbg !6313
  br label %167, !dbg !6313

163:                                              ; preds = %20, %2
  %164 = load i64, ptr %10, align 8, !dbg !6314
  %165 = load i32, ptr %11, align 4, !dbg !6315
  %166 = call noalias align 8 ptr @__kmalloc_noprof(i64 noundef %164, i32 noundef %165) #11, !dbg !6316
  store ptr %166, ptr %9, align 8, !dbg !6317
  br label %167, !dbg !6317

167:                                              ; preds = %26, %152, %163
  %168 = load ptr, ptr %9, align 8, !dbg !6318
  ret ptr %168, !dbg !6319
}

declare void @kfree(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local ptr @alloc_file_pseudo(ptr noundef %0, ptr noundef %1, ptr noundef %2, i32 noundef %3, ptr noundef %4) #0 !dbg !6320 {
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca i32, align 4
  %11 = alloca ptr, align 8
  %12 = alloca i32, align 4
  %13 = alloca %struct.path, align 8
  %14 = alloca ptr, align 8
  store ptr %0, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !6323, metadata !DIExpression()), !dbg !6324
  store ptr %1, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !6325, metadata !DIExpression()), !dbg !6326
  store ptr %2, ptr %9, align 8
  call void @llvm.dbg.declare(metadata ptr %9, metadata !6327, metadata !DIExpression()), !dbg !6328
  store i32 %3, ptr %10, align 4
  call void @llvm.dbg.declare(metadata ptr %10, metadata !6329, metadata !DIExpression()), !dbg !6330
  store ptr %4, ptr %11, align 8
  call void @llvm.dbg.declare(metadata ptr %11, metadata !6331, metadata !DIExpression()), !dbg !6332
  call void @llvm.dbg.declare(metadata ptr %12, metadata !6333, metadata !DIExpression()), !dbg !6334
  call void @llvm.dbg.declare(metadata ptr %13, metadata !6335, metadata !DIExpression()), !dbg !6336
  call void @llvm.dbg.declare(metadata ptr %14, metadata !6337, metadata !DIExpression()), !dbg !6338
  %15 = load ptr, ptr %9, align 8, !dbg !6339
  %16 = load ptr, ptr %7, align 8, !dbg !6340
  %17 = load ptr, ptr %8, align 8, !dbg !6341
  %18 = call i32 @alloc_path_pseudo(ptr noundef %15, ptr noundef %16, ptr noundef %17, ptr noundef %13), !dbg !6342
  store i32 %18, ptr %12, align 4, !dbg !6343
  %19 = load i32, ptr %12, align 4, !dbg !6344
  %20 = icmp ne i32 %19, 0, !dbg !6344
  br i1 %20, label %21, label %25, !dbg !6346

21:                                               ; preds = %5
  %22 = load i32, ptr %12, align 4, !dbg !6347
  %23 = sext i32 %22 to i64, !dbg !6347
  %24 = call ptr @ERR_PTR(i64 noundef %23), !dbg !6348
  store ptr %24, ptr %6, align 8, !dbg !6349
  br label %35, !dbg !6349

25:                                               ; preds = %5
  %26 = load i32, ptr %10, align 4, !dbg !6350
  %27 = load ptr, ptr %11, align 8, !dbg !6351
  %28 = call ptr @alloc_file(ptr noundef %13, i32 noundef %26, ptr noundef %27), !dbg !6352
  store ptr %28, ptr %14, align 8, !dbg !6353
  %29 = load ptr, ptr %14, align 8, !dbg !6354
  %30 = call zeroext i1 @IS_ERR(ptr noundef %29), !dbg !6356
  br i1 %30, label %31, label %33, !dbg !6357

31:                                               ; preds = %25
  %32 = load ptr, ptr %7, align 8, !dbg !6358
  call void @ihold(ptr noundef %32), !dbg !6360
  call void @path_put(ptr noundef %13), !dbg !6361
  br label %33, !dbg !6362

33:                                               ; preds = %31, %25
  %34 = load ptr, ptr %14, align 8, !dbg !6363
  store ptr %34, ptr %6, align 8, !dbg !6364
  br label %35, !dbg !6364

35:                                               ; preds = %33, %21
  %36 = load ptr, ptr %6, align 8, !dbg !6365
  ret ptr %36, !dbg !6365
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @alloc_path_pseudo(ptr noundef %0, ptr noundef %1, ptr noundef %2, ptr noundef %3) #0 !dbg !6366 {
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca %struct.qstr, align 8
  store ptr %0, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !6369, metadata !DIExpression()), !dbg !6370
  store ptr %1, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !6371, metadata !DIExpression()), !dbg !6372
  store ptr %2, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !6373, metadata !DIExpression()), !dbg !6374
  store ptr %3, ptr %9, align 8
  call void @llvm.dbg.declare(metadata ptr %9, metadata !6375, metadata !DIExpression()), !dbg !6376
  call void @llvm.dbg.declare(metadata ptr %10, metadata !6377, metadata !DIExpression()), !dbg !6378
  %11 = getelementptr inbounds %struct.qstr, ptr %10, i32 0, i32 0, !dbg !6379
  %12 = getelementptr inbounds %struct.anon, ptr %11, i32 0, i32 0, !dbg !6379
  store i32 0, ptr %12, align 8, !dbg !6379
  %13 = getelementptr inbounds %struct.anon, ptr %11, i32 0, i32 1, !dbg !6379
  %14 = load ptr, ptr %6, align 8, !dbg !6379
  %15 = call i64 @strlen(ptr noundef %14), !dbg !6379
  %16 = trunc i64 %15 to i32, !dbg !6379
  store i32 %16, ptr %13, align 4, !dbg !6379
  %17 = getelementptr inbounds %struct.qstr, ptr %10, i32 0, i32 1, !dbg !6379
  %18 = load ptr, ptr %6, align 8, !dbg !6379
  store ptr %18, ptr %17, align 8, !dbg !6379
  %19 = load ptr, ptr %8, align 8, !dbg !6380
  %20 = getelementptr inbounds %struct.vfsmount, ptr %19, i32 0, i32 1, !dbg !6381
  %21 = load ptr, ptr %20, align 8, !dbg !6381
  %22 = call ptr @d_alloc_pseudo(ptr noundef %21, ptr noundef %10), !dbg !6382
  %23 = load ptr, ptr %9, align 8, !dbg !6383
  %24 = getelementptr inbounds %struct.path, ptr %23, i32 0, i32 1, !dbg !6384
  store ptr %22, ptr %24, align 8, !dbg !6385
  %25 = load ptr, ptr %9, align 8, !dbg !6386
  %26 = getelementptr inbounds %struct.path, ptr %25, i32 0, i32 1, !dbg !6388
  %27 = load ptr, ptr %26, align 8, !dbg !6388
  %28 = icmp ne ptr %27, null, !dbg !6386
  br i1 %28, label %30, label %29, !dbg !6389

29:                                               ; preds = %4
  store i32 -12, ptr %5, align 4, !dbg !6390
  br label %39, !dbg !6390

30:                                               ; preds = %4
  %31 = load ptr, ptr %8, align 8, !dbg !6391
  %32 = call ptr @mntget(ptr noundef %31), !dbg !6392
  %33 = load ptr, ptr %9, align 8, !dbg !6393
  %34 = getelementptr inbounds %struct.path, ptr %33, i32 0, i32 0, !dbg !6394
  store ptr %32, ptr %34, align 8, !dbg !6395
  %35 = load ptr, ptr %9, align 8, !dbg !6396
  %36 = getelementptr inbounds %struct.path, ptr %35, i32 0, i32 1, !dbg !6397
  %37 = load ptr, ptr %36, align 8, !dbg !6397
  %38 = load ptr, ptr %7, align 8, !dbg !6398
  call void @d_instantiate(ptr noundef %37, ptr noundef %38), !dbg !6399
  store i32 0, ptr %5, align 4, !dbg !6400
  br label %39, !dbg !6400

39:                                               ; preds = %30, %29
  %40 = load i32, ptr %5, align 4, !dbg !6401
  ret i32 %40, !dbg !6401
}

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @alloc_file(ptr noundef %0, i32 noundef %1, ptr noundef %2) #0 !dbg !6402 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  store ptr %0, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6405, metadata !DIExpression()), !dbg !6406
  store i32 %1, ptr %6, align 4
  call void @llvm.dbg.declare(metadata ptr %6, metadata !6407, metadata !DIExpression()), !dbg !6408
  store ptr %2, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !6409, metadata !DIExpression()), !dbg !6410
  call void @llvm.dbg.declare(metadata ptr %8, metadata !6411, metadata !DIExpression()), !dbg !6412
  %10 = load i32, ptr %6, align 4, !dbg !6413
  br label %11, !dbg !6414

11:                                               ; preds = %3
  br label %12, !dbg !6416

12:                                               ; preds = %11
  %13 = load ptr, ptr addrspace(256) @const_pcpu_hot, align 8, !dbg !6418
  store ptr %13, ptr %4, align 8, !dbg !6426
  %14 = load ptr, ptr %4, align 8, !dbg !6418
  %15 = getelementptr inbounds %struct.task_struct, ptr %14, i32 0, i32 96, !dbg !6414
  %16 = load ptr, ptr %15, align 32, !dbg !6414
  store ptr %16, ptr %9, align 8, !dbg !6416
  %17 = load ptr, ptr %9, align 8, !dbg !6414
  %18 = call ptr @alloc_empty_file(i32 noundef %10, ptr noundef %17), !dbg !6427
  store ptr %18, ptr %8, align 8, !dbg !6428
  %19 = load ptr, ptr %8, align 8, !dbg !6429
  %20 = call zeroext i1 @IS_ERR(ptr noundef %19), !dbg !6431
  br i1 %20, label %25, label %21, !dbg !6432

21:                                               ; preds = %12
  %22 = load ptr, ptr %8, align 8, !dbg !6433
  %23 = load ptr, ptr %5, align 8, !dbg !6434
  %24 = load ptr, ptr %7, align 8, !dbg !6435
  call void @file_init_path(ptr noundef %22, ptr noundef %23, ptr noundef %24), !dbg !6436
  br label %25, !dbg !6436

25:                                               ; preds = %21, %12
  %26 = load ptr, ptr %8, align 8, !dbg !6437
  ret ptr %26, !dbg !6438
}

; Function Attrs: noinline nounwind optnone uwtable
define internal zeroext i1 @IS_ERR(ptr noundef %0) #0 !dbg !6439 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !6442, metadata !DIExpression()), !dbg !6443
  %3 = load ptr, ptr %2, align 8, !dbg !6444
  %4 = ptrtoint ptr %3 to i64, !dbg !6444
  %5 = inttoptr i64 %4 to ptr, !dbg !6444
  %6 = ptrtoint ptr %5 to i64, !dbg !6444
  %7 = icmp uge i64 %6, -4095, !dbg !6444
  %8 = xor i1 %7, true, !dbg !6444
  %9 = xor i1 %8, true, !dbg !6444
  %10 = zext i1 %9 to i32, !dbg !6444
  %11 = sext i32 %10 to i64, !dbg !6444
  %12 = icmp ne i64 %11, 0, !dbg !6444
  ret i1 %12, !dbg !6445
}

declare void @ihold(ptr noundef) #2

declare void @path_put(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local ptr @alloc_file_pseudo_noaccount(ptr noundef %0, ptr noundef %1, ptr noundef %2, i32 noundef %3, ptr noundef %4) #0 !dbg !6446 {
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca ptr, align 8
  %11 = alloca i32, align 4
  %12 = alloca ptr, align 8
  %13 = alloca i32, align 4
  %14 = alloca %struct.path, align 8
  %15 = alloca ptr, align 8
  %16 = alloca ptr, align 8
  store ptr %0, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !6447, metadata !DIExpression()), !dbg !6448
  store ptr %1, ptr %9, align 8
  call void @llvm.dbg.declare(metadata ptr %9, metadata !6449, metadata !DIExpression()), !dbg !6450
  store ptr %2, ptr %10, align 8
  call void @llvm.dbg.declare(metadata ptr %10, metadata !6451, metadata !DIExpression()), !dbg !6452
  store i32 %3, ptr %11, align 4
  call void @llvm.dbg.declare(metadata ptr %11, metadata !6453, metadata !DIExpression()), !dbg !6454
  store ptr %4, ptr %12, align 8
  call void @llvm.dbg.declare(metadata ptr %12, metadata !6455, metadata !DIExpression()), !dbg !6456
  call void @llvm.dbg.declare(metadata ptr %13, metadata !6457, metadata !DIExpression()), !dbg !6458
  call void @llvm.dbg.declare(metadata ptr %14, metadata !6459, metadata !DIExpression()), !dbg !6460
  call void @llvm.dbg.declare(metadata ptr %15, metadata !6461, metadata !DIExpression()), !dbg !6462
  %17 = load ptr, ptr %10, align 8, !dbg !6463
  %18 = load ptr, ptr %8, align 8, !dbg !6464
  %19 = load ptr, ptr %9, align 8, !dbg !6465
  %20 = call i32 @alloc_path_pseudo(ptr noundef %17, ptr noundef %18, ptr noundef %19, ptr noundef %14), !dbg !6466
  store i32 %20, ptr %13, align 4, !dbg !6467
  %21 = load i32, ptr %13, align 4, !dbg !6468
  %22 = icmp ne i32 %21, 0, !dbg !6468
  br i1 %22, label %23, label %27, !dbg !6470

23:                                               ; preds = %5
  %24 = load i32, ptr %13, align 4, !dbg !6471
  %25 = sext i32 %24 to i64, !dbg !6471
  %26 = call ptr @ERR_PTR(i64 noundef %25), !dbg !6472
  store ptr %26, ptr %7, align 8, !dbg !6473
  br label %46, !dbg !6473

27:                                               ; preds = %5
  %28 = load i32, ptr %11, align 4, !dbg !6474
  br label %29, !dbg !6475

29:                                               ; preds = %27
  br label %30, !dbg !6477

30:                                               ; preds = %29
  %31 = load ptr, ptr addrspace(256) @const_pcpu_hot, align 8, !dbg !6479
  store ptr %31, ptr %6, align 8, !dbg !6481
  %32 = load ptr, ptr %6, align 8, !dbg !6479
  %33 = getelementptr inbounds %struct.task_struct, ptr %32, i32 0, i32 96, !dbg !6475
  %34 = load ptr, ptr %33, align 32, !dbg !6475
  store ptr %34, ptr %16, align 8, !dbg !6477
  %35 = load ptr, ptr %16, align 8, !dbg !6475
  %36 = call ptr @alloc_empty_file_noaccount(i32 noundef %28, ptr noundef %35), !dbg !6482
  store ptr %36, ptr %15, align 8, !dbg !6483
  %37 = load ptr, ptr %15, align 8, !dbg !6484
  %38 = call zeroext i1 @IS_ERR(ptr noundef %37), !dbg !6486
  br i1 %38, label %39, label %42, !dbg !6487

39:                                               ; preds = %30
  %40 = load ptr, ptr %8, align 8, !dbg !6488
  call void @ihold(ptr noundef %40), !dbg !6490
  call void @path_put(ptr noundef %14), !dbg !6491
  %41 = load ptr, ptr %15, align 8, !dbg !6492
  store ptr %41, ptr %7, align 8, !dbg !6493
  br label %46, !dbg !6493

42:                                               ; preds = %30
  %43 = load ptr, ptr %15, align 8, !dbg !6494
  %44 = load ptr, ptr %12, align 8, !dbg !6495
  call void @file_init_path(ptr noundef %43, ptr noundef %14, ptr noundef %44), !dbg !6496
  %45 = load ptr, ptr %15, align 8, !dbg !6497
  store ptr %45, ptr %7, align 8, !dbg !6498
  br label %46, !dbg !6498

46:                                               ; preds = %42, %39, %23
  %47 = load ptr, ptr %7, align 8, !dbg !6499
  ret ptr %47, !dbg !6499
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @file_init_path(ptr noundef %0, ptr noundef %1, ptr noundef %2) #0 !dbg !6500 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !6503, metadata !DIExpression()), !dbg !6504
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6505, metadata !DIExpression()), !dbg !6506
  store ptr %2, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !6507, metadata !DIExpression()), !dbg !6508
  %7 = load ptr, ptr %4, align 8, !dbg !6509
  %8 = getelementptr inbounds %struct.file, ptr %7, i32 0, i32 10, !dbg !6510
  %9 = load ptr, ptr %5, align 8, !dbg !6511
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %8, ptr align 8 %9, i64 16, i1 false), !dbg !6512
  %10 = load ptr, ptr %5, align 8, !dbg !6513
  %11 = getelementptr inbounds %struct.path, ptr %10, i32 0, i32 1, !dbg !6514
  %12 = load ptr, ptr %11, align 8, !dbg !6514
  %13 = getelementptr inbounds %struct.dentry, ptr %12, i32 0, i32 5, !dbg !6515
  %14 = load ptr, ptr %13, align 8, !dbg !6515
  %15 = load ptr, ptr %4, align 8, !dbg !6516
  %16 = getelementptr inbounds %struct.file, ptr %15, i32 0, i32 6, !dbg !6517
  store ptr %14, ptr %16, align 8, !dbg !6518
  %17 = load ptr, ptr %5, align 8, !dbg !6519
  %18 = getelementptr inbounds %struct.path, ptr %17, i32 0, i32 1, !dbg !6520
  %19 = load ptr, ptr %18, align 8, !dbg !6520
  %20 = getelementptr inbounds %struct.dentry, ptr %19, i32 0, i32 5, !dbg !6521
  %21 = load ptr, ptr %20, align 8, !dbg !6521
  %22 = getelementptr inbounds %struct.inode, ptr %21, i32 0, i32 9, !dbg !6522
  %23 = load ptr, ptr %22, align 8, !dbg !6522
  %24 = load ptr, ptr %4, align 8, !dbg !6523
  %25 = getelementptr inbounds %struct.file, ptr %24, i32 0, i32 4, !dbg !6524
  store ptr %23, ptr %25, align 8, !dbg !6525
  %26 = load ptr, ptr %4, align 8, !dbg !6526
  %27 = getelementptr inbounds %struct.file, ptr %26, i32 0, i32 4, !dbg !6527
  %28 = load ptr, ptr %27, align 8, !dbg !6527
  %29 = call i32 @filemap_sample_wb_err(ptr noundef %28), !dbg !6528
  %30 = load ptr, ptr %4, align 8, !dbg !6529
  %31 = getelementptr inbounds %struct.file, ptr %30, i32 0, i32 15, !dbg !6530
  store i32 %29, ptr %31, align 8, !dbg !6531
  %32 = load ptr, ptr %4, align 8, !dbg !6532
  %33 = call i32 @file_sample_sb_err(ptr noundef %32), !dbg !6533
  %34 = load ptr, ptr %4, align 8, !dbg !6534
  %35 = getelementptr inbounds %struct.file, ptr %34, i32 0, i32 16, !dbg !6535
  store i32 %33, ptr %35, align 4, !dbg !6536
  %36 = load ptr, ptr %6, align 8, !dbg !6537
  %37 = getelementptr inbounds %struct.file_operations, ptr %36, i32 0, i32 2, !dbg !6539
  %38 = load ptr, ptr %37, align 8, !dbg !6539
  %39 = icmp ne ptr %38, null, !dbg !6537
  br i1 %39, label %40, label %45, !dbg !6540

40:                                               ; preds = %3
  %41 = load ptr, ptr %4, align 8, !dbg !6541
  %42 = getelementptr inbounds %struct.file, ptr %41, i32 0, i32 2, !dbg !6542
  %43 = load i32, ptr %42, align 4, !dbg !6543
  %44 = or i32 %43, 4, !dbg !6543
  store i32 %44, ptr %42, align 4, !dbg !6543
  br label %45, !dbg !6541

45:                                               ; preds = %40, %3
  %46 = load ptr, ptr %4, align 8, !dbg !6544
  %47 = getelementptr inbounds %struct.file, ptr %46, i32 0, i32 2, !dbg !6546
  %48 = load i32, ptr %47, align 4, !dbg !6546
  %49 = and i32 %48, 1, !dbg !6547
  %50 = icmp ne i32 %49, 0, !dbg !6547
  br i1 %50, label %51, label %73, !dbg !6548

51:                                               ; preds = %45
  %52 = load ptr, ptr %6, align 8, !dbg !6549
  %53 = getelementptr inbounds %struct.file_operations, ptr %52, i32 0, i32 3, !dbg !6549
  %54 = load ptr, ptr %53, align 8, !dbg !6549
  %55 = icmp ne ptr %54, null, !dbg !6549
  br i1 %55, label %61, label %56, !dbg !6549

56:                                               ; preds = %51
  %57 = load ptr, ptr %6, align 8, !dbg !6549
  %58 = getelementptr inbounds %struct.file_operations, ptr %57, i32 0, i32 5, !dbg !6549
  %59 = load ptr, ptr %58, align 8, !dbg !6549
  %60 = icmp ne ptr %59, null, !dbg !6549
  br label %61, !dbg !6549

61:                                               ; preds = %56, %51
  %62 = phi i1 [ true, %51 ], [ %60, %56 ]
  %63 = xor i1 %62, true, !dbg !6549
  %64 = xor i1 %63, true, !dbg !6549
  %65 = zext i1 %64 to i32, !dbg !6549
  %66 = sext i32 %65 to i64, !dbg !6549
  %67 = icmp ne i64 %66, 0, !dbg !6549
  br i1 %67, label %68, label %73, !dbg !6550

68:                                               ; preds = %61
  %69 = load ptr, ptr %4, align 8, !dbg !6551
  %70 = getelementptr inbounds %struct.file, ptr %69, i32 0, i32 2, !dbg !6552
  %71 = load i32, ptr %70, align 4, !dbg !6553
  %72 = or i32 %71, 131072, !dbg !6553
  store i32 %72, ptr %70, align 4, !dbg !6553
  br label %73, !dbg !6551

73:                                               ; preds = %68, %61, %45
  %74 = load ptr, ptr %4, align 8, !dbg !6554
  %75 = getelementptr inbounds %struct.file, ptr %74, i32 0, i32 2, !dbg !6556
  %76 = load i32, ptr %75, align 4, !dbg !6556
  %77 = and i32 %76, 2, !dbg !6557
  %78 = icmp ne i32 %77, 0, !dbg !6557
  br i1 %78, label %79, label %101, !dbg !6558

79:                                               ; preds = %73
  %80 = load ptr, ptr %6, align 8, !dbg !6559
  %81 = getelementptr inbounds %struct.file_operations, ptr %80, i32 0, i32 4, !dbg !6559
  %82 = load ptr, ptr %81, align 8, !dbg !6559
  %83 = icmp ne ptr %82, null, !dbg !6559
  br i1 %83, label %89, label %84, !dbg !6559

84:                                               ; preds = %79
  %85 = load ptr, ptr %6, align 8, !dbg !6559
  %86 = getelementptr inbounds %struct.file_operations, ptr %85, i32 0, i32 6, !dbg !6559
  %87 = load ptr, ptr %86, align 8, !dbg !6559
  %88 = icmp ne ptr %87, null, !dbg !6559
  br label %89, !dbg !6559

89:                                               ; preds = %84, %79
  %90 = phi i1 [ true, %79 ], [ %88, %84 ]
  %91 = xor i1 %90, true, !dbg !6559
  %92 = xor i1 %91, true, !dbg !6559
  %93 = zext i1 %92 to i32, !dbg !6559
  %94 = sext i32 %93 to i64, !dbg !6559
  %95 = icmp ne i64 %94, 0, !dbg !6559
  br i1 %95, label %96, label %101, !dbg !6560

96:                                               ; preds = %89
  %97 = load ptr, ptr %4, align 8, !dbg !6561
  %98 = getelementptr inbounds %struct.file, ptr %97, i32 0, i32 2, !dbg !6562
  %99 = load i32, ptr %98, align 4, !dbg !6563
  %100 = or i32 %99, 262144, !dbg !6563
  store i32 %100, ptr %98, align 4, !dbg !6563
  br label %101, !dbg !6561

101:                                              ; preds = %96, %89, %73
  %102 = load ptr, ptr %4, align 8, !dbg !6564
  %103 = call i32 @iocb_flags(ptr noundef %102), !dbg !6565
  %104 = load ptr, ptr %4, align 8, !dbg !6566
  %105 = getelementptr inbounds %struct.file, ptr %104, i32 0, i32 8, !dbg !6567
  store i32 %103, ptr %105, align 4, !dbg !6568
  %106 = load ptr, ptr %4, align 8, !dbg !6569
  %107 = getelementptr inbounds %struct.file, ptr %106, i32 0, i32 2, !dbg !6570
  %108 = load i32, ptr %107, align 4, !dbg !6571
  %109 = or i32 %108, 524288, !dbg !6571
  store i32 %109, ptr %107, align 4, !dbg !6571
  %110 = load ptr, ptr %6, align 8, !dbg !6572
  %111 = load ptr, ptr %4, align 8, !dbg !6573
  %112 = getelementptr inbounds %struct.file, ptr %111, i32 0, i32 3, !dbg !6574
  store ptr %110, ptr %112, align 8, !dbg !6575
  %113 = load ptr, ptr %4, align 8, !dbg !6576
  %114 = getelementptr inbounds %struct.file, ptr %113, i32 0, i32 2, !dbg !6578
  %115 = load i32, ptr %114, align 4, !dbg !6578
  %116 = and i32 %115, 3, !dbg !6579
  %117 = icmp eq i32 %116, 1, !dbg !6580
  br i1 %117, label %118, label %124, !dbg !6581

118:                                              ; preds = %101
  %119 = load ptr, ptr %5, align 8, !dbg !6582
  %120 = getelementptr inbounds %struct.path, ptr %119, i32 0, i32 1, !dbg !6583
  %121 = load ptr, ptr %120, align 8, !dbg !6583
  %122 = getelementptr inbounds %struct.dentry, ptr %121, i32 0, i32 5, !dbg !6584
  %123 = load ptr, ptr %122, align 8, !dbg !6584
  call void @i_readcount_inc(ptr noundef %123), !dbg !6585
  br label %124, !dbg !6585

124:                                              ; preds = %118, %101
  ret void, !dbg !6586
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local ptr @alloc_file_clone(ptr noundef %0, i32 noundef %1, ptr noundef %2) #0 !dbg !6587 {
  %4 = alloca ptr, align 8
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !6590, metadata !DIExpression()), !dbg !6591
  store i32 %1, ptr %5, align 4
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6592, metadata !DIExpression()), !dbg !6593
  store ptr %2, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !6594, metadata !DIExpression()), !dbg !6595
  call void @llvm.dbg.declare(metadata ptr %7, metadata !6596, metadata !DIExpression()), !dbg !6597
  %8 = load ptr, ptr %4, align 8, !dbg !6598
  %9 = getelementptr inbounds %struct.file, ptr %8, i32 0, i32 10, !dbg !6599
  %10 = load i32, ptr %5, align 4, !dbg !6600
  %11 = load ptr, ptr %6, align 8, !dbg !6601
  %12 = call ptr @alloc_file(ptr noundef %9, i32 noundef %10, ptr noundef %11), !dbg !6602
  store ptr %12, ptr %7, align 8, !dbg !6603
  %13 = load ptr, ptr %7, align 8, !dbg !6604
  %14 = call zeroext i1 @IS_ERR(ptr noundef %13), !dbg !6606
  br i1 %14, label %23, label %15, !dbg !6607

15:                                               ; preds = %3
  %16 = load ptr, ptr %7, align 8, !dbg !6608
  %17 = getelementptr inbounds %struct.file, ptr %16, i32 0, i32 10, !dbg !6610
  call void @path_get(ptr noundef %17), !dbg !6611
  %18 = load ptr, ptr %4, align 8, !dbg !6612
  %19 = getelementptr inbounds %struct.file, ptr %18, i32 0, i32 4, !dbg !6613
  %20 = load ptr, ptr %19, align 8, !dbg !6613
  %21 = load ptr, ptr %7, align 8, !dbg !6614
  %22 = getelementptr inbounds %struct.file, ptr %21, i32 0, i32 4, !dbg !6615
  store ptr %20, ptr %22, align 8, !dbg !6616
  br label %23, !dbg !6617

23:                                               ; preds = %15, %3
  %24 = load ptr, ptr %7, align 8, !dbg !6618
  ret ptr %24, !dbg !6619
}

declare void @path_get(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @flush_delayed_fput() #0 !dbg !6620 {
  call void @delayed_fput(ptr noundef null), !dbg !6621
  ret void, !dbg !6622
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @delayed_fput(ptr noundef %0) #0 !dbg !6623 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !6624, metadata !DIExpression()), !dbg !6625
  call void @llvm.dbg.declare(metadata ptr %3, metadata !6626, metadata !DIExpression()), !dbg !6627
  %10 = call ptr @llist_del_all(ptr noundef @delayed_fput_list), !dbg !6628
  store ptr %10, ptr %3, align 8, !dbg !6627
  call void @llvm.dbg.declare(metadata ptr %4, metadata !6629, metadata !DIExpression()), !dbg !6630
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6631, metadata !DIExpression()), !dbg !6632
  call void @llvm.dbg.declare(metadata ptr %6, metadata !6633, metadata !DIExpression()), !dbg !6636
  %11 = load ptr, ptr %3, align 8, !dbg !6636
  store ptr %11, ptr %6, align 8, !dbg !6636
  %12 = load ptr, ptr %6, align 8, !dbg !6636
  %13 = getelementptr i8, ptr %12, i64 -152, !dbg !6636
  store ptr %13, ptr %7, align 8, !dbg !6636
  %14 = load ptr, ptr %7, align 8, !dbg !6636
  store ptr %14, ptr %4, align 8, !dbg !6637
  br label %15, !dbg !6637

15:                                               ; preds = %32, %1
  %16 = load ptr, ptr %4, align 8, !dbg !6638
  %17 = ptrtoint ptr %16 to i64, !dbg !6638
  %18 = add i64 %17, 152, !dbg !6638
  %19 = icmp ne i64 %18, 0, !dbg !6638
  br i1 %19, label %20, label %28, !dbg !6638

20:                                               ; preds = %15
  call void @llvm.dbg.declare(metadata ptr %8, metadata !6640, metadata !DIExpression()), !dbg !6642
  %21 = load ptr, ptr %4, align 8, !dbg !6642
  %22 = getelementptr inbounds %struct.file, ptr %21, i32 0, i32 18, !dbg !6642
  %23 = getelementptr inbounds %struct.llist_node, ptr %22, i32 0, i32 0, !dbg !6642
  %24 = load ptr, ptr %23, align 8, !dbg !6642
  store ptr %24, ptr %8, align 8, !dbg !6642
  %25 = load ptr, ptr %8, align 8, !dbg !6642
  %26 = getelementptr i8, ptr %25, i64 -152, !dbg !6642
  store ptr %26, ptr %9, align 8, !dbg !6642
  %27 = load ptr, ptr %9, align 8, !dbg !6642
  store ptr %27, ptr %5, align 8, !dbg !6638
  br label %28

28:                                               ; preds = %20, %15
  %29 = phi i1 [ false, %15 ], [ true, %20 ], !dbg !6643
  br i1 %29, label %30, label %34, !dbg !6637

30:                                               ; preds = %28
  %31 = load ptr, ptr %4, align 8, !dbg !6644
  call void @__fput(ptr noundef %31), !dbg !6645
  br label %32, !dbg !6645

32:                                               ; preds = %30
  %33 = load ptr, ptr %5, align 8, !dbg !6638
  store ptr %33, ptr %4, align 8, !dbg !6638
  br label %15, !dbg !6638, !llvm.loop !6646

34:                                               ; preds = %28
  ret void, !dbg !6649
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @fput(ptr noundef %0) #0 !dbg !6650 {
  %2 = alloca ptr, align 8
  %3 = alloca i8, align 1
  %4 = alloca i8, align 1
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i64, align 8
  %9 = alloca i32, align 4
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  %12 = alloca ptr, align 8
  %13 = alloca ptr, align 8
  store ptr %0, ptr %12, align 8
  call void @llvm.dbg.declare(metadata ptr %12, metadata !6651, metadata !DIExpression()), !dbg !6652
  %14 = load ptr, ptr %12, align 8, !dbg !6653
  %15 = getelementptr inbounds %struct.file, ptr %14, i32 0, i32 0, !dbg !6655
  store ptr %15, ptr %10, align 8
  call void @llvm.dbg.declare(metadata ptr %10, metadata !6656, metadata !DIExpression()), !dbg !6660
  %16 = load ptr, ptr %10, align 8, !dbg !6662
  store ptr %16, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !6663, metadata !DIExpression()), !dbg !6665
  store i64 8, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !6667, metadata !DIExpression()), !dbg !6668
  %17 = load ptr, ptr %7, align 8, !dbg !6669
  %18 = load i64, ptr %8, align 8, !dbg !6670
  %19 = trunc i64 %18 to i32, !dbg !6670
  %20 = call zeroext i1 @kasan_check_write(ptr noundef %17, i32 noundef %19), !dbg !6671
  %21 = load ptr, ptr %7, align 8, !dbg !6672
  %22 = load i64, ptr %8, align 8, !dbg !6672
  call void @kcsan_check_access(ptr noundef %21, i64 noundef %22, i32 noundef 7), !dbg !6672
  %23 = load ptr, ptr %10, align 8, !dbg !6673
  store ptr %23, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !6674, metadata !DIExpression()), !dbg !6676
  %24 = load ptr, ptr %6, align 8, !dbg !6678
  store ptr %24, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6679, metadata !DIExpression()), !dbg !6683
  %25 = load ptr, ptr %5, align 8, !dbg !6685
  store ptr %25, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !6686, metadata !DIExpression()), !dbg !6688
  call void @llvm.dbg.declare(metadata ptr %3, metadata !6690, metadata !DIExpression()), !dbg !6692
  %26 = load ptr, ptr %2, align 8, !dbg !6692
  %27 = call i8 asm sideeffect ".pushsection .smp_locks,\22a\22\0A.balign 4\0A.long 671f - .\0A.popsection\0A671:\0A\09lock; decq $0\0A\09/* output condition code e*/\0A", "=*m,={@cce},*m,~{memory},~{dirflag},~{fpsr},~{flags}"(ptr elementtype(i64) %26, ptr elementtype(i64) %26) #12, !dbg !6692, !srcloc !6693
  %28 = icmp ult i8 %27, 2, !dbg !6692
  call void @llvm.assume(i1 %28), !dbg !6692
  store i8 %27, ptr %3, align 1, !dbg !6692
  %29 = load i8, ptr %3, align 1, !dbg !6692
  %30 = trunc i8 %29 to i1, !dbg !6692
  %31 = zext i1 %30 to i8, !dbg !6692
  store i8 %31, ptr %4, align 1, !dbg !6692
  %32 = load i8, ptr %4, align 1, !dbg !6692
  %33 = trunc i8 %32 to i1, !dbg !6692
  br i1 %33, label %34, label %88, !dbg !6694

34:                                               ; preds = %1
  call void @llvm.dbg.declare(metadata ptr %13, metadata !6695, metadata !DIExpression()), !dbg !6697
  %35 = load ptr, ptr addrspace(256) @const_pcpu_hot, align 8, !dbg !6698
  store ptr %35, ptr %11, align 8, !dbg !6700
  %36 = load ptr, ptr %11, align 8, !dbg !6698
  store ptr %36, ptr %13, align 8, !dbg !6697
  %37 = load ptr, ptr %12, align 8, !dbg !6701
  %38 = getelementptr inbounds %struct.file, ptr %37, i32 0, i32 2, !dbg !6701
  %39 = load i32, ptr %38, align 4, !dbg !6701
  %40 = and i32 %39, 34078720, !dbg !6701
  %41 = icmp ne i32 %40, 0, !dbg !6701
  %42 = xor i1 %41, true, !dbg !6701
  %43 = xor i1 %42, true, !dbg !6701
  %44 = xor i1 %43, true, !dbg !6701
  %45 = zext i1 %44 to i32, !dbg !6701
  %46 = sext i32 %45 to i64, !dbg !6701
  %47 = icmp ne i64 %46, 0, !dbg !6701
  br i1 %47, label %48, label %50, !dbg !6703

48:                                               ; preds = %34
  %49 = load ptr, ptr %12, align 8, !dbg !6704
  call void @file_free(ptr noundef %49), !dbg !6706
  br label %88, !dbg !6707

50:                                               ; preds = %34
  %51 = load i32, ptr addrspace(256) inttoptr (i64 ptrtoint (ptr getelementptr inbounds (%struct.anon.109, ptr @pcpu_hot, i32 0, i32 1) to i64) to ptr addrspace(256)), align 4, !dbg !6708
  store i32 %51, ptr %9, align 4, !dbg !6714
  %52 = load i32, ptr %9, align 4, !dbg !6708
  %53 = and i32 %52, 2147483647, !dbg !6715
  %54 = sext i32 %53 to i64, !dbg !6716
  %55 = and i64 %54, 16776960, !dbg !6716
  %56 = icmp ne i64 %55, 0, !dbg !6716
  br i1 %56, label %64, label %57, !dbg !6716

57:                                               ; preds = %50
  %58 = load ptr, ptr %13, align 8, !dbg !6716
  %59 = getelementptr inbounds %struct.task_struct, ptr %58, i32 0, i32 5, !dbg !6716
  %60 = load i32, ptr %59, align 4, !dbg !6716
  %61 = and i32 %60, 2097152, !dbg !6716
  %62 = icmp ne i32 %61, 0, !dbg !6716
  %63 = xor i1 %62, true, !dbg !6716
  br label %64

64:                                               ; preds = %57, %50
  %65 = phi i1 [ false, %50 ], [ %63, %57 ], !dbg !6717
  %66 = xor i1 %65, true, !dbg !6716
  %67 = xor i1 %66, true, !dbg !6716
  %68 = zext i1 %67 to i32, !dbg !6716
  %69 = sext i32 %68 to i64, !dbg !6716
  %70 = icmp ne i64 %69, 0, !dbg !6716
  br i1 %70, label %71, label %81, !dbg !6718

71:                                               ; preds = %64
  %72 = load ptr, ptr %12, align 8, !dbg !6719
  %73 = getelementptr inbounds %struct.file, ptr %72, i32 0, i32 18, !dbg !6721
  call void @init_task_work(ptr noundef %73, ptr noundef @____fput), !dbg !6722
  %74 = load ptr, ptr %13, align 8, !dbg !6723
  %75 = load ptr, ptr %12, align 8, !dbg !6725
  %76 = getelementptr inbounds %struct.file, ptr %75, i32 0, i32 18, !dbg !6726
  %77 = call i32 @task_work_add(ptr noundef %74, ptr noundef %76, i32 noundef 1), !dbg !6727
  %78 = icmp ne i32 %77, 0, !dbg !6727
  br i1 %78, label %80, label %79, !dbg !6728

79:                                               ; preds = %71
  br label %88, !dbg !6729

80:                                               ; preds = %71
  br label %81, !dbg !6730

81:                                               ; preds = %80, %64
  %82 = load ptr, ptr %12, align 8, !dbg !6731
  %83 = getelementptr inbounds %struct.file, ptr %82, i32 0, i32 18, !dbg !6733
  %84 = call zeroext i1 @llist_add(ptr noundef %83, ptr noundef @delayed_fput_list), !dbg !6734
  br i1 %84, label %85, label %87, !dbg !6735

85:                                               ; preds = %81
  %86 = call zeroext i1 @schedule_delayed_work(ptr noundef @delayed_fput_work, i64 noundef 1), !dbg !6736
  br label %87, !dbg !6736

87:                                               ; preds = %85, %81
  br label %88, !dbg !6737

88:                                               ; preds = %48, %79, %87, %1
  ret void, !dbg !6738
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @file_free(ptr noundef %0) #0 !dbg !6739 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !6740, metadata !DIExpression()), !dbg !6741
  %3 = load ptr, ptr %2, align 8, !dbg !6742
  call void @security_file_free(ptr noundef %3), !dbg !6743
  %4 = load ptr, ptr %2, align 8, !dbg !6744
  %5 = getelementptr inbounds %struct.file, ptr %4, i32 0, i32 2, !dbg !6744
  %6 = load i32, ptr %5, align 4, !dbg !6744
  %7 = and i32 %6, 536870912, !dbg !6744
  %8 = icmp ne i32 %7, 0, !dbg !6744
  %9 = xor i1 %8, true, !dbg !6744
  %10 = xor i1 %9, true, !dbg !6744
  %11 = xor i1 %10, true, !dbg !6744
  %12 = zext i1 %11 to i32, !dbg !6744
  %13 = sext i32 %12 to i64, !dbg !6744
  %14 = icmp ne i64 %13, 0, !dbg !6744
  br i1 %14, label %15, label %16, !dbg !6746

15:                                               ; preds = %1
  call void @percpu_counter_dec(ptr noundef @nr_files), !dbg !6747
  br label %16, !dbg !6747

16:                                               ; preds = %15, %1
  %17 = load ptr, ptr %2, align 8, !dbg !6748
  %18 = getelementptr inbounds %struct.file, ptr %17, i32 0, i32 9, !dbg !6749
  %19 = load ptr, ptr %18, align 8, !dbg !6749
  call void @put_cred(ptr noundef %19), !dbg !6750
  %20 = load ptr, ptr %2, align 8, !dbg !6751
  %21 = getelementptr inbounds %struct.file, ptr %20, i32 0, i32 2, !dbg !6751
  %22 = load i32, ptr %21, align 4, !dbg !6751
  %23 = and i32 %22, 33554432, !dbg !6751
  %24 = icmp ne i32 %23, 0, !dbg !6751
  %25 = xor i1 %24, true, !dbg !6751
  %26 = xor i1 %25, true, !dbg !6751
  %27 = zext i1 %26 to i32, !dbg !6751
  %28 = sext i32 %27 to i64, !dbg !6751
  %29 = icmp ne i64 %28, 0, !dbg !6751
  br i1 %29, label %30, label %35, !dbg !6753

30:                                               ; preds = %16
  %31 = load ptr, ptr %2, align 8, !dbg !6754
  %32 = call ptr @backing_file_user_path(ptr noundef %31), !dbg !6756
  call void @path_put(ptr noundef %32), !dbg !6757
  %33 = load ptr, ptr %2, align 8, !dbg !6758
  %34 = call ptr @backing_file(ptr noundef %33), !dbg !6759
  call void @kfree(ptr noundef %34), !dbg !6760
  br label %38, !dbg !6761

35:                                               ; preds = %16
  %36 = load ptr, ptr @filp_cachep, align 8, !dbg !6762
  %37 = load ptr, ptr %2, align 8, !dbg !6764
  call void @kmem_cache_free(ptr noundef %36, ptr noundef %37), !dbg !6765
  br label %38

38:                                               ; preds = %35, %30
  ret void, !dbg !6766
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @init_task_work(ptr noundef %0, ptr noundef %1) #0 !dbg !6767 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !6771, metadata !DIExpression()), !dbg !6772
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !6773, metadata !DIExpression()), !dbg !6774
  %5 = load ptr, ptr %4, align 8, !dbg !6775
  %6 = load ptr, ptr %3, align 8, !dbg !6776
  %7 = getelementptr inbounds %struct.callback_head, ptr %6, i32 0, i32 1, !dbg !6777
  store ptr %5, ptr %7, align 8, !dbg !6778
  ret void, !dbg !6779
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @____fput(ptr noundef %0) #0 !dbg !6780 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !6781, metadata !DIExpression()), !dbg !6782
  call void @llvm.dbg.declare(metadata ptr %3, metadata !6783, metadata !DIExpression()), !dbg !6785
  %5 = load ptr, ptr %2, align 8, !dbg !6785
  store ptr %5, ptr %3, align 8, !dbg !6785
  %6 = load ptr, ptr %3, align 8, !dbg !6785
  %7 = getelementptr i8, ptr %6, i64 -152, !dbg !6785
  store ptr %7, ptr %4, align 8, !dbg !6785
  %8 = load ptr, ptr %4, align 8, !dbg !6785
  call void @__fput(ptr noundef %8), !dbg !6786
  ret void, !dbg !6787
}

declare i32 @task_work_add(ptr noundef, ptr noundef, i32 noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal zeroext i1 @llist_add(ptr noundef %0, ptr noundef %1) #0 !dbg !6788 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !6792, metadata !DIExpression()), !dbg !6793
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !6794, metadata !DIExpression()), !dbg !6795
  %5 = load ptr, ptr %3, align 8, !dbg !6796
  %6 = load ptr, ptr %3, align 8, !dbg !6797
  %7 = load ptr, ptr %4, align 8, !dbg !6798
  %8 = call zeroext i1 @llist_add_batch(ptr noundef %5, ptr noundef %6, ptr noundef %7), !dbg !6799
  ret i1 %8, !dbg !6800
}

; Function Attrs: noinline nounwind optnone uwtable
define internal zeroext i1 @schedule_delayed_work(ptr noundef %0, i64 noundef %1) #0 !dbg !6801 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !6805, metadata !DIExpression()), !dbg !6806
  store i64 %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !6807, metadata !DIExpression()), !dbg !6808
  %5 = load ptr, ptr @system_wq, align 8, !dbg !6809
  %6 = load ptr, ptr %3, align 8, !dbg !6810
  %7 = load i64, ptr %4, align 8, !dbg !6811
  %8 = call zeroext i1 @queue_delayed_work(ptr noundef %5, ptr noundef %6, i64 noundef %7), !dbg !6812
  ret i1 %8, !dbg !6813
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @__fput_sync(ptr noundef %0) #0 !dbg !6814 {
  %2 = alloca ptr, align 8
  %3 = alloca i8, align 1
  %4 = alloca i8, align 1
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i64, align 8
  %9 = alloca ptr, align 8
  %10 = alloca ptr, align 8
  store ptr %0, ptr %10, align 8
  call void @llvm.dbg.declare(metadata ptr %10, metadata !6815, metadata !DIExpression()), !dbg !6816
  %11 = load ptr, ptr %10, align 8, !dbg !6817
  %12 = getelementptr inbounds %struct.file, ptr %11, i32 0, i32 0, !dbg !6819
  store ptr %12, ptr %9, align 8
  call void @llvm.dbg.declare(metadata ptr %9, metadata !6656, metadata !DIExpression()), !dbg !6820
  %13 = load ptr, ptr %9, align 8, !dbg !6822
  store ptr %13, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !6663, metadata !DIExpression()), !dbg !6823
  store i64 8, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !6667, metadata !DIExpression()), !dbg !6825
  %14 = load ptr, ptr %7, align 8, !dbg !6826
  %15 = load i64, ptr %8, align 8, !dbg !6827
  %16 = trunc i64 %15 to i32, !dbg !6827
  %17 = call zeroext i1 @kasan_check_write(ptr noundef %14, i32 noundef %16), !dbg !6828
  %18 = load ptr, ptr %7, align 8, !dbg !6829
  %19 = load i64, ptr %8, align 8, !dbg !6829
  call void @kcsan_check_access(ptr noundef %18, i64 noundef %19, i32 noundef 7), !dbg !6829
  %20 = load ptr, ptr %9, align 8, !dbg !6830
  store ptr %20, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !6674, metadata !DIExpression()), !dbg !6831
  %21 = load ptr, ptr %6, align 8, !dbg !6833
  store ptr %21, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6679, metadata !DIExpression()), !dbg !6834
  %22 = load ptr, ptr %5, align 8, !dbg !6836
  store ptr %22, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !6686, metadata !DIExpression()), !dbg !6837
  call void @llvm.dbg.declare(metadata ptr %3, metadata !6690, metadata !DIExpression()), !dbg !6839
  %23 = load ptr, ptr %2, align 8, !dbg !6839
  %24 = call i8 asm sideeffect ".pushsection .smp_locks,\22a\22\0A.balign 4\0A.long 671f - .\0A.popsection\0A671:\0A\09lock; decq $0\0A\09/* output condition code e*/\0A", "=*m,={@cce},*m,~{memory},~{dirflag},~{fpsr},~{flags}"(ptr elementtype(i64) %23, ptr elementtype(i64) %23) #12, !dbg !6839, !srcloc !6693
  %25 = icmp ult i8 %24, 2, !dbg !6839
  call void @llvm.assume(i1 %25), !dbg !6839
  store i8 %24, ptr %3, align 1, !dbg !6839
  %26 = load i8, ptr %3, align 1, !dbg !6839
  %27 = trunc i8 %26 to i1, !dbg !6839
  %28 = zext i1 %27 to i8, !dbg !6839
  store i8 %28, ptr %4, align 1, !dbg !6839
  %29 = load i8, ptr %4, align 1, !dbg !6839
  %30 = trunc i8 %29 to i1, !dbg !6839
  br i1 %30, label %31, label %33, !dbg !6840

31:                                               ; preds = %1
  %32 = load ptr, ptr %10, align 8, !dbg !6841
  call void @__fput(ptr noundef %32), !dbg !6842
  br label %33, !dbg !6842

33:                                               ; preds = %31, %1
  ret void, !dbg !6843
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @__fput(ptr noundef %0) #0 !dbg !6844 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  %7 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !6845, metadata !DIExpression()), !dbg !6846
  call void @llvm.dbg.declare(metadata ptr %3, metadata !6847, metadata !DIExpression()), !dbg !6848
  %8 = load ptr, ptr %2, align 8, !dbg !6849
  %9 = getelementptr inbounds %struct.file, ptr %8, i32 0, i32 10, !dbg !6850
  %10 = getelementptr inbounds %struct.path, ptr %9, i32 0, i32 1, !dbg !6851
  %11 = load ptr, ptr %10, align 8, !dbg !6851
  store ptr %11, ptr %3, align 8, !dbg !6848
  call void @llvm.dbg.declare(metadata ptr %4, metadata !6852, metadata !DIExpression()), !dbg !6853
  %12 = load ptr, ptr %2, align 8, !dbg !6854
  %13 = getelementptr inbounds %struct.file, ptr %12, i32 0, i32 10, !dbg !6855
  %14 = getelementptr inbounds %struct.path, ptr %13, i32 0, i32 0, !dbg !6856
  %15 = load ptr, ptr %14, align 8, !dbg !6856
  store ptr %15, ptr %4, align 8, !dbg !6853
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6857, metadata !DIExpression()), !dbg !6858
  %16 = load ptr, ptr %2, align 8, !dbg !6859
  %17 = getelementptr inbounds %struct.file, ptr %16, i32 0, i32 6, !dbg !6860
  %18 = load ptr, ptr %17, align 8, !dbg !6860
  store ptr %18, ptr %5, align 8, !dbg !6858
  call void @llvm.dbg.declare(metadata ptr %6, metadata !6861, metadata !DIExpression()), !dbg !6862
  %19 = load ptr, ptr %2, align 8, !dbg !6863
  %20 = getelementptr inbounds %struct.file, ptr %19, i32 0, i32 2, !dbg !6864
  %21 = load i32, ptr %20, align 4, !dbg !6864
  store i32 %21, ptr %6, align 4, !dbg !6862
  %22 = load ptr, ptr %2, align 8, !dbg !6865
  %23 = getelementptr inbounds %struct.file, ptr %22, i32 0, i32 2, !dbg !6865
  %24 = load i32, ptr %23, align 4, !dbg !6865
  %25 = and i32 %24, 524288, !dbg !6865
  %26 = icmp ne i32 %25, 0, !dbg !6865
  %27 = xor i1 %26, true, !dbg !6865
  %28 = xor i1 %27, true, !dbg !6865
  %29 = xor i1 %28, true, !dbg !6865
  %30 = zext i1 %29 to i32, !dbg !6865
  %31 = sext i32 %30 to i64, !dbg !6865
  %32 = icmp ne i64 %31, 0, !dbg !6865
  br i1 %32, label %33, label %34, !dbg !6867

33:                                               ; preds = %1
  br label %138, !dbg !6868

34:                                               ; preds = %1
  br label %35, !dbg !6869

35:                                               ; preds = %34
  %36 = call i32 @__SCT__might_resched() #12, !dbg !6870
  br label %37, !dbg !6875

37:                                               ; preds = %35
  %38 = load ptr, ptr %2, align 8, !dbg !6876
  call void @fsnotify_close(ptr noundef %38), !dbg !6877
  %39 = load ptr, ptr %2, align 8, !dbg !6878
  call void @eventpoll_release(ptr noundef %39), !dbg !6879
  %40 = load ptr, ptr %2, align 8, !dbg !6880
  call void @locks_remove_file(ptr noundef %40), !dbg !6881
  %41 = load ptr, ptr %2, align 8, !dbg !6882
  call void @security_file_release(ptr noundef %41), !dbg !6883
  %42 = load ptr, ptr %2, align 8, !dbg !6884
  %43 = getelementptr inbounds %struct.file, ptr %42, i32 0, i32 7, !dbg !6884
  %44 = load i32, ptr %43, align 8, !dbg !6884
  %45 = and i32 %44, 8192, !dbg !6884
  %46 = icmp ne i32 %45, 0, !dbg !6884
  %47 = xor i1 %46, true, !dbg !6884
  %48 = xor i1 %47, true, !dbg !6884
  %49 = zext i1 %48 to i32, !dbg !6884
  %50 = sext i32 %49 to i64, !dbg !6884
  %51 = icmp ne i64 %50, 0, !dbg !6884
  br i1 %51, label %52, label %68, !dbg !6886

52:                                               ; preds = %37
  %53 = load ptr, ptr %2, align 8, !dbg !6887
  %54 = getelementptr inbounds %struct.file, ptr %53, i32 0, i32 3, !dbg !6890
  %55 = load ptr, ptr %54, align 8, !dbg !6890
  %56 = getelementptr inbounds %struct.file_operations, ptr %55, i32 0, i32 17, !dbg !6891
  %57 = load ptr, ptr %56, align 8, !dbg !6891
  %58 = icmp ne ptr %57, null, !dbg !6887
  br i1 %58, label %59, label %67, !dbg !6892

59:                                               ; preds = %52
  %60 = load ptr, ptr %2, align 8, !dbg !6893
  %61 = getelementptr inbounds %struct.file, ptr %60, i32 0, i32 3, !dbg !6894
  %62 = load ptr, ptr %61, align 8, !dbg !6894
  %63 = getelementptr inbounds %struct.file_operations, ptr %62, i32 0, i32 17, !dbg !6895
  %64 = load ptr, ptr %63, align 8, !dbg !6895
  %65 = load ptr, ptr %2, align 8, !dbg !6896
  %66 = call i32 %64(i32 noundef -1, ptr noundef %65, i32 noundef 0), !dbg !6893
  br label %67, !dbg !6893

67:                                               ; preds = %59, %52
  br label %68, !dbg !6897

68:                                               ; preds = %67, %37
  %69 = load ptr, ptr %2, align 8, !dbg !6898
  %70 = getelementptr inbounds %struct.file, ptr %69, i32 0, i32 3, !dbg !6900
  %71 = load ptr, ptr %70, align 8, !dbg !6900
  %72 = getelementptr inbounds %struct.file_operations, ptr %71, i32 0, i32 15, !dbg !6901
  %73 = load ptr, ptr %72, align 8, !dbg !6901
  %74 = icmp ne ptr %73, null, !dbg !6898
  br i1 %74, label %75, label %84, !dbg !6902

75:                                               ; preds = %68
  %76 = load ptr, ptr %2, align 8, !dbg !6903
  %77 = getelementptr inbounds %struct.file, ptr %76, i32 0, i32 3, !dbg !6904
  %78 = load ptr, ptr %77, align 8, !dbg !6904
  %79 = getelementptr inbounds %struct.file_operations, ptr %78, i32 0, i32 15, !dbg !6905
  %80 = load ptr, ptr %79, align 8, !dbg !6905
  %81 = load ptr, ptr %5, align 8, !dbg !6906
  %82 = load ptr, ptr %2, align 8, !dbg !6907
  %83 = call i32 %80(ptr noundef %81, ptr noundef %82), !dbg !6903
  br label %84, !dbg !6903

84:                                               ; preds = %75, %68
  %85 = load ptr, ptr %5, align 8, !dbg !6908
  %86 = getelementptr inbounds %struct.inode, ptr %85, i32 0, i32 0, !dbg !6908
  %87 = load i16, ptr %86, align 8, !dbg !6908
  %88 = zext i16 %87 to i32, !dbg !6908
  %89 = and i32 %88, 61440, !dbg !6908
  %90 = icmp eq i32 %89, 8192, !dbg !6908
  br i1 %90, label %91, label %101, !dbg !6908

91:                                               ; preds = %84
  %92 = load ptr, ptr %5, align 8, !dbg !6908
  %93 = getelementptr inbounds %struct.inode, ptr %92, i32 0, i32 47, !dbg !6908
  %94 = load ptr, ptr %93, align 8, !dbg !6908
  %95 = icmp ne ptr %94, null, !dbg !6908
  br i1 %95, label %96, label %101, !dbg !6908

96:                                               ; preds = %91
  %97 = load i32, ptr %6, align 4, !dbg !6908
  %98 = and i32 %97, 16384, !dbg !6908
  %99 = icmp ne i32 %98, 0, !dbg !6908
  %100 = xor i1 %99, true, !dbg !6908
  br label %101

101:                                              ; preds = %96, %91, %84
  %102 = phi i1 [ false, %91 ], [ false, %84 ], [ %100, %96 ], !dbg !6910
  %103 = xor i1 %102, true, !dbg !6908
  %104 = xor i1 %103, true, !dbg !6908
  %105 = zext i1 %104 to i32, !dbg !6908
  %106 = sext i32 %105 to i64, !dbg !6908
  %107 = icmp ne i64 %106, 0, !dbg !6908
  br i1 %107, label %108, label %112, !dbg !6911

108:                                              ; preds = %101
  %109 = load ptr, ptr %5, align 8, !dbg !6912
  %110 = getelementptr inbounds %struct.inode, ptr %109, i32 0, i32 47, !dbg !6914
  %111 = load ptr, ptr %110, align 8, !dbg !6914
  call void @cdev_put(ptr noundef %111), !dbg !6915
  br label %112, !dbg !6916

112:                                              ; preds = %108, %101
  call void @llvm.dbg.declare(metadata ptr %7, metadata !6917, metadata !DIExpression()), !dbg !6919
  %113 = load ptr, ptr %2, align 8, !dbg !6919
  %114 = getelementptr inbounds %struct.file, ptr %113, i32 0, i32 3, !dbg !6919
  %115 = load ptr, ptr %114, align 8, !dbg !6919
  store ptr %115, ptr %7, align 8, !dbg !6919
  %116 = load ptr, ptr %7, align 8, !dbg !6920
  %117 = icmp ne ptr %116, null, !dbg !6920
  br i1 %117, label %118, label %122, !dbg !6919

118:                                              ; preds = %112
  %119 = load ptr, ptr %7, align 8, !dbg !6920
  %120 = getelementptr inbounds %struct.file_operations, ptr %119, i32 0, i32 0, !dbg !6920
  %121 = load ptr, ptr %120, align 8, !dbg !6920
  call void @module_put(ptr noundef %121), !dbg !6920
  br label %122, !dbg !6920

122:                                              ; preds = %118, %112
  %123 = load ptr, ptr %2, align 8, !dbg !6922
  call void @file_f_owner_release(ptr noundef %123), !dbg !6923
  %124 = load ptr, ptr %2, align 8, !dbg !6924
  call void @put_file_access(ptr noundef %124), !dbg !6925
  %125 = load ptr, ptr %3, align 8, !dbg !6926
  call void @dput(ptr noundef %125), !dbg !6927
  %126 = load i32, ptr %6, align 4, !dbg !6928
  %127 = and i32 %126, 268435456, !dbg !6928
  %128 = icmp ne i32 %127, 0, !dbg !6928
  %129 = xor i1 %128, true, !dbg !6928
  %130 = xor i1 %129, true, !dbg !6928
  %131 = zext i1 %130 to i32, !dbg !6928
  %132 = sext i32 %131 to i64, !dbg !6928
  %133 = icmp ne i64 %132, 0, !dbg !6928
  br i1 %133, label %134, label %136, !dbg !6930

134:                                              ; preds = %122
  %135 = load ptr, ptr %4, align 8, !dbg !6931
  call void @dissolve_on_fput(ptr noundef %135), !dbg !6932
  br label %136, !dbg !6932

136:                                              ; preds = %134, %122
  %137 = load ptr, ptr %4, align 8, !dbg !6933
  call void @mntput(ptr noundef %137), !dbg !6934
  br label %138, !dbg !6934

138:                                              ; preds = %136, %33
  call void @llvm.dbg.label(metadata !6935), !dbg !6936
  %139 = load ptr, ptr %2, align 8, !dbg !6937
  call void @file_free(ptr noundef %139), !dbg !6938
  ret void, !dbg !6939
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @files_init() #0 section ".init.text" !dbg !5679 {
  %1 = alloca %struct.kmem_cache_args, align 8
  %2 = alloca i32, align 4
  call void @llvm.dbg.declare(metadata ptr %1, metadata !6940, metadata !DIExpression()), !dbg !6949
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %1, ptr align 8 @__const.files_init.args, i64 32, i1 false), !dbg !6949
  %3 = call ptr @__kmem_cache_create_args(ptr noundef @.str.1, i32 noundef 184, ptr noundef %1, i32 noundef 784), !dbg !6950
  store ptr %3, ptr @filp_cachep, align 8, !dbg !6951
  %4 = call i32 @__percpu_counter_init_many(ptr noundef @nr_files, i64 noundef 0, i32 noundef 3264, i32 noundef 1, ptr noundef @files_init.__key), !dbg !6952
  store i32 %4, ptr %2, align 4, !dbg !6952
  %5 = load i32, ptr %2, align 4, !dbg !6952
  ret void, !dbg !6954
}

; Function Attrs: argmemonly nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #4

declare ptr @__kmem_cache_create_args(ptr noundef, i32 noundef, ptr noundef, i32 noundef) #2

declare i32 @__percpu_counter_init_many(ptr noundef, i64 noundef, i32 noundef, i32 noundef, ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @files_maxfiles_init() #0 section ".init.text" !dbg !6955 {
  %1 = alloca i64, align 8
  %2 = alloca i64, align 8
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  %9 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %1, metadata !6956, metadata !DIExpression()), !dbg !6957
  call void @llvm.dbg.declare(metadata ptr %2, metadata !6958, metadata !DIExpression()), !dbg !6959
  %10 = call i64 @totalram_pages(), !dbg !6960
  store i64 %10, ptr %2, align 8, !dbg !6959
  call void @llvm.dbg.declare(metadata ptr %3, metadata !6961, metadata !DIExpression()), !dbg !6962
  %11 = load i64, ptr %2, align 8, !dbg !6963
  %12 = call i64 @global_zone_page_state(i32 noundef 0), !dbg !6964
  %13 = sub i64 %11, %12, !dbg !6965
  %14 = mul i64 %13, 3, !dbg !6966
  %15 = udiv i64 %14, 2, !dbg !6967
  store i64 %15, ptr %3, align 8, !dbg !6962
  call void @llvm.dbg.declare(metadata ptr %4, metadata !6968, metadata !DIExpression()), !dbg !6970
  %16 = load i64, ptr %3, align 8, !dbg !6970
  store i64 %16, ptr %4, align 8, !dbg !6970
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6971, metadata !DIExpression()), !dbg !6970
  %17 = load i64, ptr %2, align 8, !dbg !6970
  %18 = sub i64 %17, 1, !dbg !6970
  store i64 %18, ptr %5, align 8, !dbg !6970
  br label %19, !dbg !6970

19:                                               ; preds = %0
  br label %20, !dbg !6972

20:                                               ; preds = %19
  %21 = load i64, ptr %4, align 8, !dbg !6970
  %22 = load i64, ptr %5, align 8, !dbg !6970
  %23 = icmp ult i64 %21, %22, !dbg !6970
  br i1 %23, label %24, label %26, !dbg !6970

24:                                               ; preds = %20
  %25 = load i64, ptr %4, align 8, !dbg !6970
  br label %28, !dbg !6970

26:                                               ; preds = %20
  %27 = load i64, ptr %5, align 8, !dbg !6970
  br label %28, !dbg !6970

28:                                               ; preds = %26, %24
  %29 = phi i64 [ %25, %24 ], [ %27, %26 ], !dbg !6970
  store i64 %29, ptr %6, align 8, !dbg !6972
  %30 = load i64, ptr %6, align 8, !dbg !6970
  store i64 %30, ptr %3, align 8, !dbg !6974
  %31 = load i64, ptr %2, align 8, !dbg !6975
  %32 = load i64, ptr %3, align 8, !dbg !6976
  %33 = sub i64 %31, %32, !dbg !6977
  %34 = mul i64 %33, 4, !dbg !6978
  %35 = udiv i64 %34, 10, !dbg !6979
  store i64 %35, ptr %1, align 8, !dbg !6980
  call void @llvm.dbg.declare(metadata ptr %7, metadata !6981, metadata !DIExpression()), !dbg !6983
  %36 = load i64, ptr %1, align 8, !dbg !6983
  store i64 %36, ptr %7, align 8, !dbg !6983
  call void @llvm.dbg.declare(metadata ptr %8, metadata !6984, metadata !DIExpression()), !dbg !6983
  store i64 8192, ptr %8, align 8, !dbg !6983
  %37 = load i64, ptr %7, align 8, !dbg !6983
  %38 = load i64, ptr %8, align 8, !dbg !6983
  %39 = icmp ugt i64 %37, %38, !dbg !6983
  br i1 %39, label %40, label %42, !dbg !6983

40:                                               ; preds = %28
  %41 = load i64, ptr %7, align 8, !dbg !6983
  br label %44, !dbg !6983

42:                                               ; preds = %28
  %43 = load i64, ptr %8, align 8, !dbg !6983
  br label %44, !dbg !6983

44:                                               ; preds = %42, %40
  %45 = phi i64 [ %41, %40 ], [ %43, %42 ], !dbg !6983
  store i64 %45, ptr %9, align 8, !dbg !6983
  %46 = load i64, ptr %9, align 8, !dbg !6983
  store i64 %46, ptr getelementptr inbounds (%struct.files_stat_struct, ptr @files_stat, i32 0, i32 2), align 8, !dbg !6985
  ret void, !dbg !6986
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i64 @totalram_pages() #0 !dbg !6987 {
  %1 = alloca ptr, align 8
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  store ptr @_totalram_pages, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !6988, metadata !DIExpression()), !dbg !6994
  %7 = load ptr, ptr %6, align 8, !dbg !6996
  store ptr %7, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !6997, metadata !DIExpression()), !dbg !6999
  store i64 8, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !7001, metadata !DIExpression()), !dbg !7002
  %8 = load ptr, ptr %4, align 8, !dbg !7003
  %9 = load i64, ptr %5, align 8, !dbg !7004
  %10 = trunc i64 %9 to i32, !dbg !7004
  %11 = call zeroext i1 @kasan_check_read(ptr noundef %8, i32 noundef %10), !dbg !7005
  %12 = load ptr, ptr %4, align 8, !dbg !7006
  %13 = load i64, ptr %5, align 8, !dbg !7006
  call void @kcsan_check_access(ptr noundef %12, i64 noundef %13, i32 noundef 4), !dbg !7006
  %14 = load ptr, ptr %6, align 8, !dbg !7007
  store ptr %14, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !7008, metadata !DIExpression()), !dbg !7010
  %15 = load ptr, ptr %3, align 8, !dbg !7012
  store ptr %15, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7013, metadata !DIExpression()), !dbg !7019
  %16 = load ptr, ptr %2, align 8, !dbg !7021
  store ptr %16, ptr %1, align 8
  call void @llvm.dbg.declare(metadata ptr %1, metadata !7022, metadata !DIExpression()), !dbg !7024
  %17 = load ptr, ptr %1, align 8, !dbg !7026
  %18 = load volatile i64, ptr %17, align 8, !dbg !7026
  ret i64 %18, !dbg !7027
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i64 @global_zone_page_state(i32 noundef %0) #0 !dbg !7028 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i32, align 4
  %9 = alloca i64, align 8
  store i32 %0, ptr %8, align 4
  call void @llvm.dbg.declare(metadata ptr %8, metadata !7032, metadata !DIExpression()), !dbg !7033
  call void @llvm.dbg.declare(metadata ptr %9, metadata !7034, metadata !DIExpression()), !dbg !7035
  %10 = load i32, ptr %8, align 4, !dbg !7036
  %11 = zext i32 %10 to i64, !dbg !7037
  %12 = getelementptr inbounds [10 x %struct.atomic64_t], ptr @vm_zone_stat, i64 0, i64 %11, !dbg !7037
  store ptr %12, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !6988, metadata !DIExpression()), !dbg !7038
  %13 = load ptr, ptr %7, align 8, !dbg !7040
  store ptr %13, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6997, metadata !DIExpression()), !dbg !7041
  store i64 8, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !7001, metadata !DIExpression()), !dbg !7043
  %14 = load ptr, ptr %5, align 8, !dbg !7044
  %15 = load i64, ptr %6, align 8, !dbg !7045
  %16 = trunc i64 %15 to i32, !dbg !7045
  %17 = call zeroext i1 @kasan_check_read(ptr noundef %14, i32 noundef %16), !dbg !7046
  %18 = load ptr, ptr %5, align 8, !dbg !7047
  %19 = load i64, ptr %6, align 8, !dbg !7047
  call void @kcsan_check_access(ptr noundef %18, i64 noundef %19, i32 noundef 4), !dbg !7047
  %20 = load ptr, ptr %7, align 8, !dbg !7048
  store ptr %20, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !7008, metadata !DIExpression()), !dbg !7049
  %21 = load ptr, ptr %4, align 8, !dbg !7051
  store ptr %21, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !7013, metadata !DIExpression()), !dbg !7052
  %22 = load ptr, ptr %3, align 8, !dbg !7054
  store ptr %22, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7022, metadata !DIExpression()), !dbg !7055
  %23 = load ptr, ptr %2, align 8, !dbg !7057
  %24 = load volatile i64, ptr %23, align 8, !dbg !7057
  store i64 %24, ptr %9, align 8, !dbg !7035
  %25 = load i64, ptr %9, align 8, !dbg !7058
  %26 = icmp slt i64 %25, 0, !dbg !7060
  br i1 %26, label %27, label %28, !dbg !7061

27:                                               ; preds = %1
  store i64 0, ptr %9, align 8, !dbg !7062
  br label %28, !dbg !7063

28:                                               ; preds = %27, %1
  %29 = load i64, ptr %9, align 8, !dbg !7064
  ret i64 %29, !dbg !7065
}

declare void @__register_sysctl_init(ptr noundef, ptr noundef, ptr noundef, i64 noundef) #2

declare ptr @register_sysctl_mount_point(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal void @kmemleak_not_leak(ptr noundef %0) #0 !dbg !7066 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7070, metadata !DIExpression()), !dbg !7071
  ret void, !dbg !7072
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @proc_nr_files(ptr noundef %0, i32 noundef %1, ptr noundef %2, ptr noundef %3, ptr noundef %4) #0 !dbg !7073 {
  %6 = alloca ptr, align 8
  %7 = alloca i32, align 4
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca ptr, align 8
  store ptr %0, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !7074, metadata !DIExpression()), !dbg !7075
  store i32 %1, ptr %7, align 4
  call void @llvm.dbg.declare(metadata ptr %7, metadata !7076, metadata !DIExpression()), !dbg !7077
  store ptr %2, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !7078, metadata !DIExpression()), !dbg !7079
  store ptr %3, ptr %9, align 8
  call void @llvm.dbg.declare(metadata ptr %9, metadata !7080, metadata !DIExpression()), !dbg !7081
  store ptr %4, ptr %10, align 8
  call void @llvm.dbg.declare(metadata ptr %10, metadata !7082, metadata !DIExpression()), !dbg !7083
  %11 = call i64 @get_nr_files(), !dbg !7084
  store i64 %11, ptr @files_stat, align 8, !dbg !7085
  %12 = load ptr, ptr %6, align 8, !dbg !7086
  %13 = load i32, ptr %7, align 4, !dbg !7087
  %14 = load ptr, ptr %8, align 8, !dbg !7088
  %15 = load ptr, ptr %9, align 8, !dbg !7089
  %16 = load ptr, ptr %10, align 8, !dbg !7090
  %17 = call i32 @proc_doulongvec_minmax(ptr noundef %12, i32 noundef %13, ptr noundef %14, ptr noundef %15, ptr noundef %16), !dbg !7091
  ret i32 %17, !dbg !7092
}

declare i32 @proc_doulongvec_minmax(ptr noundef, i32 noundef, ptr noundef, ptr noundef, ptr noundef) #2

declare i32 @proc_dointvec_minmax(ptr noundef, i32 noundef, ptr noundef, ptr noundef, ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal i64 @percpu_counter_read_positive(ptr noundef %0) #0 !dbg !7093 {
  %2 = alloca i64, align 8
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  %5 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !7094, metadata !DIExpression()), !dbg !7095
  call void @llvm.dbg.declare(metadata ptr %4, metadata !7096, metadata !DIExpression()), !dbg !7097
  br label %6, !dbg !7098

6:                                                ; preds = %1
  br label %7, !dbg !7100

7:                                                ; preds = %6
  %8 = load ptr, ptr %3, align 8, !dbg !7098
  %9 = getelementptr inbounds %struct.percpu_counter, ptr %8, i32 0, i32 1, !dbg !7098
  %10 = load volatile i64, ptr %9, align 8, !dbg !7098
  store i64 %10, ptr %5, align 8, !dbg !7100
  %11 = load i64, ptr %5, align 8, !dbg !7098
  store i64 %11, ptr %4, align 8, !dbg !7097
  %12 = load i64, ptr %4, align 8, !dbg !7102
  %13 = icmp sge i64 %12, 0, !dbg !7104
  br i1 %13, label %14, label %16, !dbg !7105

14:                                               ; preds = %7
  %15 = load i64, ptr %4, align 8, !dbg !7106
  store i64 %15, ptr %2, align 8, !dbg !7107
  br label %17, !dbg !7107

16:                                               ; preds = %7
  store i64 0, ptr %2, align 8, !dbg !7108
  br label %17, !dbg !7108

17:                                               ; preds = %16, %14
  %18 = load i64, ptr %2, align 8, !dbg !7109
  ret i64 %18, !dbg !7109
}

declare i64 @__percpu_counter_sum(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @get_cred(ptr noundef %0) #0 !dbg !7110 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7113, metadata !DIExpression()), !dbg !7114
  %3 = load ptr, ptr %2, align 8, !dbg !7115
  %4 = call ptr @get_cred_many(ptr noundef %3, i32 noundef 1), !dbg !7116
  ret ptr %4, !dbg !7117
}

declare i32 @security_file_alloc(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal void @put_cred(ptr noundef %0) #0 !dbg !7118 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7121, metadata !DIExpression()), !dbg !7122
  %3 = load ptr, ptr %2, align 8, !dbg !7123
  call void @put_cred_many(ptr noundef %3, i32 noundef 1), !dbg !7124
  ret void, !dbg !7125
}

declare void @__mutex_init(ptr noundef, ptr noundef, ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @get_cred_many(ptr noundef %0, i32 noundef %1) #0 !dbg !7126 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !7129, metadata !DIExpression()), !dbg !7130
  store i32 %1, ptr %5, align 4
  call void @llvm.dbg.declare(metadata ptr %5, metadata !7131, metadata !DIExpression()), !dbg !7132
  call void @llvm.dbg.declare(metadata ptr %6, metadata !7133, metadata !DIExpression()), !dbg !7134
  %7 = load ptr, ptr %4, align 8, !dbg !7135
  store ptr %7, ptr %6, align 8, !dbg !7134
  %8 = load ptr, ptr %4, align 8, !dbg !7136
  %9 = icmp ne ptr %8, null, !dbg !7136
  br i1 %9, label %12, label %10, !dbg !7138

10:                                               ; preds = %2
  %11 = load ptr, ptr %4, align 8, !dbg !7139
  store ptr %11, ptr %3, align 8, !dbg !7140
  br label %18, !dbg !7140

12:                                               ; preds = %2
  %13 = load ptr, ptr %6, align 8, !dbg !7141
  %14 = getelementptr inbounds %struct.cred, ptr %13, i32 0, i32 25, !dbg !7142
  store i32 0, ptr %14, align 8, !dbg !7143
  %15 = load ptr, ptr %6, align 8, !dbg !7144
  %16 = load i32, ptr %5, align 4, !dbg !7145
  %17 = call ptr @get_new_cred_many(ptr noundef %15, i32 noundef %16), !dbg !7146
  store ptr %17, ptr %3, align 8, !dbg !7147
  br label %18, !dbg !7147

18:                                               ; preds = %12, %10
  %19 = load ptr, ptr %3, align 8, !dbg !7148
  ret ptr %19, !dbg !7148
}

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @get_new_cred_many(ptr noundef %0, i32 noundef %1) #0 !dbg !7149 {
  %3 = alloca i64, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i64, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca i64, align 8
  %11 = alloca i64, align 8
  %12 = alloca ptr, align 8
  %13 = alloca ptr, align 8
  %14 = alloca i32, align 4
  store ptr %0, ptr %13, align 8
  call void @llvm.dbg.declare(metadata ptr %13, metadata !7152, metadata !DIExpression()), !dbg !7153
  store i32 %1, ptr %14, align 4
  call void @llvm.dbg.declare(metadata ptr %14, metadata !7154, metadata !DIExpression()), !dbg !7155
  %15 = load i32, ptr %14, align 4, !dbg !7156
  %16 = sext i32 %15 to i64, !dbg !7156
  %17 = load ptr, ptr %13, align 8, !dbg !7157
  %18 = getelementptr inbounds %struct.cred, ptr %17, i32 0, i32 0, !dbg !7158
  store i64 %16, ptr %11, align 8
  call void @llvm.dbg.declare(metadata ptr %11, metadata !7159, metadata !DIExpression()), !dbg !7163
  store ptr %18, ptr %12, align 8
  call void @llvm.dbg.declare(metadata ptr %12, metadata !7165, metadata !DIExpression()), !dbg !7166
  %19 = load ptr, ptr %12, align 8, !dbg !7167
  store ptr %19, ptr %9, align 8
  call void @llvm.dbg.declare(metadata ptr %9, metadata !6663, metadata !DIExpression()), !dbg !7168
  store i64 8, ptr %10, align 8
  call void @llvm.dbg.declare(metadata ptr %10, metadata !6667, metadata !DIExpression()), !dbg !7170
  %20 = load ptr, ptr %9, align 8, !dbg !7171
  %21 = load i64, ptr %10, align 8, !dbg !7172
  %22 = trunc i64 %21 to i32, !dbg !7172
  %23 = call zeroext i1 @kasan_check_write(ptr noundef %20, i32 noundef %22), !dbg !7173
  %24 = load ptr, ptr %9, align 8, !dbg !7174
  %25 = load i64, ptr %10, align 8, !dbg !7174
  call void @kcsan_check_access(ptr noundef %24, i64 noundef %25, i32 noundef 7), !dbg !7174
  %26 = load i64, ptr %11, align 8, !dbg !7175
  %27 = load ptr, ptr %12, align 8, !dbg !7176
  store i64 %26, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !7177, metadata !DIExpression()), !dbg !7179
  store ptr %27, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !7181, metadata !DIExpression()), !dbg !7182
  %28 = load i64, ptr %7, align 8, !dbg !7183
  %29 = load ptr, ptr %8, align 8, !dbg !7184
  store i64 %28, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !7185, metadata !DIExpression()), !dbg !7189
  store ptr %29, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !7191, metadata !DIExpression()), !dbg !7192
  %30 = load i64, ptr %5, align 8, !dbg !7193
  %31 = load ptr, ptr %6, align 8, !dbg !7194
  store i64 %30, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !7195, metadata !DIExpression()), !dbg !7197
  store ptr %31, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !7199, metadata !DIExpression()), !dbg !7200
  %32 = load ptr, ptr %4, align 8, !dbg !7201
  %33 = load i64, ptr %3, align 8, !dbg !7202
  %34 = load ptr, ptr %4, align 8, !dbg !7203
  call void asm sideeffect ".pushsection .smp_locks,\22a\22\0A.balign 4\0A.long 671f - .\0A.popsection\0A671:\0A\09lock; addq $1,$0", "=*m,er,*m,~{memory},~{dirflag},~{fpsr},~{flags}"(ptr elementtype(i64) %32, i64 %33, ptr elementtype(i64) %34) #12, !dbg !7204, !srcloc !7205
  %35 = load ptr, ptr %13, align 8, !dbg !7206
  ret ptr %35, !dbg !7207
}

; Function Attrs: noinline nounwind optnone uwtable
define internal zeroext i1 @kasan_check_write(ptr noundef %0, i32 noundef %1) #0 !dbg !7208 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !7212, metadata !DIExpression()), !dbg !7213
  store i32 %1, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !7214, metadata !DIExpression()), !dbg !7215
  ret i1 true, !dbg !7216
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @kcsan_check_access(ptr noundef %0, i64 noundef %1, i32 noundef %2) #0 !dbg !7217 {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !7221, metadata !DIExpression()), !dbg !7222
  store i64 %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !7223, metadata !DIExpression()), !dbg !7224
  store i32 %2, ptr %6, align 4
  call void @llvm.dbg.declare(metadata ptr %6, metadata !7225, metadata !DIExpression()), !dbg !7226
  ret void, !dbg !7227
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @put_cred_many(ptr noundef %0, i32 noundef %1) #0 !dbg !7228 {
  %3 = alloca i64, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i8, align 1
  %6 = alloca i8, align 1
  %7 = alloca i64, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i64, align 8
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  %12 = alloca i64, align 8
  %13 = alloca i64, align 8
  %14 = alloca ptr, align 8
  %15 = alloca ptr, align 8
  %16 = alloca i32, align 4
  %17 = alloca ptr, align 8
  store ptr %0, ptr %15, align 8
  call void @llvm.dbg.declare(metadata ptr %15, metadata !7231, metadata !DIExpression()), !dbg !7232
  store i32 %1, ptr %16, align 4
  call void @llvm.dbg.declare(metadata ptr %16, metadata !7233, metadata !DIExpression()), !dbg !7234
  call void @llvm.dbg.declare(metadata ptr %17, metadata !7235, metadata !DIExpression()), !dbg !7236
  %18 = load ptr, ptr %15, align 8, !dbg !7237
  store ptr %18, ptr %17, align 8, !dbg !7236
  %19 = load ptr, ptr %17, align 8, !dbg !7238
  %20 = icmp ne ptr %19, null, !dbg !7238
  br i1 %20, label %21, label %51, !dbg !7240

21:                                               ; preds = %2
  %22 = load i32, ptr %16, align 4, !dbg !7241
  %23 = sext i32 %22 to i64, !dbg !7241
  %24 = load ptr, ptr %17, align 8, !dbg !7244
  %25 = getelementptr inbounds %struct.cred, ptr %24, i32 0, i32 0, !dbg !7245
  store i64 %23, ptr %13, align 8
  call void @llvm.dbg.declare(metadata ptr %13, metadata !7246, metadata !DIExpression()), !dbg !7250
  store ptr %25, ptr %14, align 8
  call void @llvm.dbg.declare(metadata ptr %14, metadata !7252, metadata !DIExpression()), !dbg !7253
  %26 = load ptr, ptr %14, align 8, !dbg !7254
  store ptr %26, ptr %11, align 8
  call void @llvm.dbg.declare(metadata ptr %11, metadata !6663, metadata !DIExpression()), !dbg !7255
  store i64 8, ptr %12, align 8
  call void @llvm.dbg.declare(metadata ptr %12, metadata !6667, metadata !DIExpression()), !dbg !7257
  %27 = load ptr, ptr %11, align 8, !dbg !7258
  %28 = load i64, ptr %12, align 8, !dbg !7259
  %29 = trunc i64 %28 to i32, !dbg !7259
  %30 = call zeroext i1 @kasan_check_write(ptr noundef %27, i32 noundef %29), !dbg !7260
  %31 = load ptr, ptr %11, align 8, !dbg !7261
  %32 = load i64, ptr %12, align 8, !dbg !7261
  call void @kcsan_check_access(ptr noundef %31, i64 noundef %32, i32 noundef 7), !dbg !7261
  %33 = load i64, ptr %13, align 8, !dbg !7262
  %34 = load ptr, ptr %14, align 8, !dbg !7263
  store i64 %33, ptr %9, align 8
  call void @llvm.dbg.declare(metadata ptr %9, metadata !7264, metadata !DIExpression()), !dbg !7266
  store ptr %34, ptr %10, align 8
  call void @llvm.dbg.declare(metadata ptr %10, metadata !7268, metadata !DIExpression()), !dbg !7269
  %35 = load i64, ptr %9, align 8, !dbg !7270
  %36 = load ptr, ptr %10, align 8, !dbg !7271
  store i64 %35, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !7272, metadata !DIExpression()), !dbg !7276
  store ptr %36, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !7278, metadata !DIExpression()), !dbg !7279
  %37 = load i64, ptr %7, align 8, !dbg !7280
  %38 = load ptr, ptr %8, align 8, !dbg !7281
  store i64 %37, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !7282, metadata !DIExpression()), !dbg !7284
  store ptr %38, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !7286, metadata !DIExpression()), !dbg !7287
  call void @llvm.dbg.declare(metadata ptr %5, metadata !7288, metadata !DIExpression()), !dbg !7290
  %39 = load ptr, ptr %4, align 8, !dbg !7290
  %40 = load i64, ptr %3, align 8, !dbg !7290
  %41 = call i8 asm sideeffect ".pushsection .smp_locks,\22a\22\0A.balign 4\0A.long 671f - .\0A.popsection\0A671:\0A\09lock; subq $2, $0\0A\09/* output condition code e*/\0A", "=*m,={@cce},er,*m,~{memory},~{dirflag},~{fpsr},~{flags}"(ptr elementtype(i64) %39, i64 %40, ptr elementtype(i64) %39) #12, !dbg !7290, !srcloc !7291
  %42 = icmp ult i8 %41, 2, !dbg !7290
  call void @llvm.assume(i1 %42), !dbg !7290
  store i8 %41, ptr %5, align 1, !dbg !7290
  %43 = load i8, ptr %5, align 1, !dbg !7290
  %44 = trunc i8 %43 to i1, !dbg !7290
  %45 = zext i1 %44 to i8, !dbg !7290
  store i8 %45, ptr %6, align 1, !dbg !7290
  %46 = load i8, ptr %6, align 1, !dbg !7290
  %47 = trunc i8 %46 to i1, !dbg !7290
  br i1 %47, label %48, label %50, !dbg !7292

48:                                               ; preds = %21
  %49 = load ptr, ptr %17, align 8, !dbg !7293
  call void @__put_cred(ptr noundef %49), !dbg !7294
  br label %50, !dbg !7294

50:                                               ; preds = %48, %21
  br label %51, !dbg !7295

51:                                               ; preds = %50, %2
  ret void, !dbg !7296
}

declare void @__put_cred(ptr noundef) #2

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #5

; Function Attrs: noinline nounwind optnone uwtable
define internal void @percpu_counter_add(ptr noundef %0, i64 noundef %1) #0 !dbg !7297 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !7300, metadata !DIExpression()), !dbg !7301
  store i64 %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !7302, metadata !DIExpression()), !dbg !7303
  %5 = load ptr, ptr %3, align 8, !dbg !7304
  %6 = load i64, ptr %4, align 8, !dbg !7305
  %7 = load i32, ptr @percpu_counter_batch, align 4, !dbg !7306
  call void @percpu_counter_add_batch(ptr noundef %5, i64 noundef %6, i32 noundef %7), !dbg !7307
  ret void, !dbg !7308
}

declare void @percpu_counter_add_batch(ptr noundef, i64 noundef, i32 noundef) #2

; Function Attrs: convergent nocallback nofree nosync nounwind readnone willreturn
declare i1 @llvm.is.constant.i64(i64) #6

; Function Attrs: allocsize(0)
declare noalias ptr @__kmalloc_large_noprof(i64 noundef, i32 noundef) #7

; Function Attrs: allocsize(2)
declare noalias ptr @__kmalloc_cache_noprof(ptr noundef, i32 noundef, i64 noundef) #8

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare ptr @llvm.returnaddress(i32 immarg) #9

; Function Attrs: allocsize(0)
declare noalias ptr @__kmalloc_noprof(i64 noundef, i32 noundef) #7

declare i64 @strlen(ptr noundef) #2

declare ptr @d_alloc_pseudo(ptr noundef, ptr noundef) #2

declare ptr @mntget(ptr noundef) #2

declare void @d_instantiate(ptr noundef, ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @filemap_sample_wb_err(ptr noundef %0) #0 !dbg !7309 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7312, metadata !DIExpression()), !dbg !7313
  %3 = load ptr, ptr %2, align 8, !dbg !7314
  %4 = getelementptr inbounds %struct.address_space, ptr %3, i32 0, i32 10, !dbg !7315
  %5 = call i32 @errseq_sample(ptr noundef %4), !dbg !7316
  ret i32 %5, !dbg !7317
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @file_sample_sb_err(ptr noundef %0) #0 !dbg !7318 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7321, metadata !DIExpression()), !dbg !7322
  %3 = load ptr, ptr %2, align 8, !dbg !7323
  %4 = getelementptr inbounds %struct.file, ptr %3, i32 0, i32 10, !dbg !7324
  %5 = getelementptr inbounds %struct.path, ptr %4, i32 0, i32 1, !dbg !7325
  %6 = load ptr, ptr %5, align 8, !dbg !7325
  %7 = getelementptr inbounds %struct.dentry, ptr %6, i32 0, i32 8, !dbg !7326
  %8 = load ptr, ptr %7, align 8, !dbg !7326
  %9 = getelementptr inbounds %struct.super_block, ptr %8, i32 0, i32 46, !dbg !7327
  %10 = call i32 @errseq_sample(ptr noundef %9), !dbg !7328
  ret i32 %10, !dbg !7329
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @iocb_flags(ptr noundef %0) #0 !dbg !7330 {
  %2 = alloca ptr, align 8
  %3 = alloca i32, align 4
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7333, metadata !DIExpression()), !dbg !7334
  call void @llvm.dbg.declare(metadata ptr %3, metadata !7335, metadata !DIExpression()), !dbg !7336
  store i32 0, ptr %3, align 4, !dbg !7336
  %4 = load ptr, ptr %2, align 8, !dbg !7337
  %5 = getelementptr inbounds %struct.file, ptr %4, i32 0, i32 7, !dbg !7339
  %6 = load i32, ptr %5, align 8, !dbg !7339
  %7 = and i32 %6, 1024, !dbg !7340
  %8 = icmp ne i32 %7, 0, !dbg !7340
  br i1 %8, label %9, label %12, !dbg !7341

9:                                                ; preds = %1
  %10 = load i32, ptr %3, align 4, !dbg !7342
  %11 = or i32 %10, 16, !dbg !7342
  store i32 %11, ptr %3, align 4, !dbg !7342
  br label %12, !dbg !7343

12:                                               ; preds = %9, %1
  %13 = load ptr, ptr %2, align 8, !dbg !7344
  %14 = getelementptr inbounds %struct.file, ptr %13, i32 0, i32 7, !dbg !7346
  %15 = load i32, ptr %14, align 8, !dbg !7346
  %16 = and i32 %15, 16384, !dbg !7347
  %17 = icmp ne i32 %16, 0, !dbg !7347
  br i1 %17, label %18, label %21, !dbg !7348

18:                                               ; preds = %12
  %19 = load i32, ptr %3, align 4, !dbg !7349
  %20 = or i32 %19, 131072, !dbg !7349
  store i32 %20, ptr %3, align 4, !dbg !7349
  br label %21, !dbg !7350

21:                                               ; preds = %18, %12
  %22 = load ptr, ptr %2, align 8, !dbg !7351
  %23 = getelementptr inbounds %struct.file, ptr %22, i32 0, i32 7, !dbg !7353
  %24 = load i32, ptr %23, align 8, !dbg !7353
  %25 = and i32 %24, 4096, !dbg !7354
  %26 = icmp ne i32 %25, 0, !dbg !7354
  br i1 %26, label %27, label %30, !dbg !7355

27:                                               ; preds = %21
  %28 = load i32, ptr %3, align 4, !dbg !7356
  %29 = or i32 %28, 2, !dbg !7356
  store i32 %29, ptr %3, align 4, !dbg !7356
  br label %30, !dbg !7357

30:                                               ; preds = %27, %21
  %31 = load ptr, ptr %2, align 8, !dbg !7358
  %32 = getelementptr inbounds %struct.file, ptr %31, i32 0, i32 7, !dbg !7360
  %33 = load i32, ptr %32, align 8, !dbg !7360
  %34 = and i32 %33, 1048576, !dbg !7361
  %35 = icmp ne i32 %34, 0, !dbg !7361
  br i1 %35, label %36, label %39, !dbg !7362

36:                                               ; preds = %30
  %37 = load i32, ptr %3, align 4, !dbg !7363
  %38 = or i32 %37, 4, !dbg !7363
  store i32 %38, ptr %3, align 4, !dbg !7363
  br label %39, !dbg !7364

39:                                               ; preds = %36, %30
  %40 = load i32, ptr %3, align 4, !dbg !7365
  ret i32 %40, !dbg !7366
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @i_readcount_inc(ptr noundef %0) #0 !dbg !7367 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  store ptr %0, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !7368, metadata !DIExpression()), !dbg !7369
  %8 = load ptr, ptr %7, align 8, !dbg !7370
  %9 = getelementptr inbounds %struct.inode, ptr %8, i32 0, i32 42, !dbg !7371
  store ptr %9, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !7372, metadata !DIExpression()), !dbg !7377
  %10 = load ptr, ptr %6, align 8, !dbg !7379
  store ptr %10, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !6663, metadata !DIExpression()), !dbg !7380
  store i64 4, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6667, metadata !DIExpression()), !dbg !7382
  %11 = load ptr, ptr %4, align 8, !dbg !7383
  %12 = load i64, ptr %5, align 8, !dbg !7384
  %13 = trunc i64 %12 to i32, !dbg !7384
  %14 = call zeroext i1 @kasan_check_write(ptr noundef %11, i32 noundef %13), !dbg !7385
  %15 = load ptr, ptr %4, align 8, !dbg !7386
  %16 = load i64, ptr %5, align 8, !dbg !7386
  call void @kcsan_check_access(ptr noundef %15, i64 noundef %16, i32 noundef 7), !dbg !7386
  %17 = load ptr, ptr %6, align 8, !dbg !7387
  store ptr %17, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !7388, metadata !DIExpression()), !dbg !7390
  %18 = load ptr, ptr %3, align 8, !dbg !7392
  store ptr %18, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7393, metadata !DIExpression()), !dbg !7396
  %19 = load ptr, ptr %2, align 8, !dbg !7398
  call void asm sideeffect ".pushsection .smp_locks,\22a\22\0A.balign 4\0A.long 671f - .\0A.popsection\0A671:\0A\09lock; incl $0", "=*m,*m,~{memory},~{dirflag},~{fpsr},~{flags}"(ptr elementtype(i32) %19, ptr elementtype(i32) %19) #12, !dbg !7399, !srcloc !7400
  ret void, !dbg !7401
}

declare i32 @errseq_sample(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @llist_del_all(ptr noundef %0) #0 !dbg !7402 {
  %2 = alloca ptr, align 8
  %3 = alloca i64, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !7405, metadata !DIExpression()), !dbg !7406
  call void @llvm.dbg.declare(metadata ptr %5, metadata !7407, metadata !DIExpression()), !dbg !7410
  %9 = load ptr, ptr %4, align 8, !dbg !7410
  %10 = getelementptr inbounds %struct.llist_head, ptr %9, i32 0, i32 0, !dbg !7410
  store ptr %10, ptr %5, align 8, !dbg !7410
  br label %11, !dbg !7410

11:                                               ; preds = %1
  br label %12, !dbg !7411

12:                                               ; preds = %11
  %13 = load ptr, ptr %5, align 8, !dbg !7410
  store ptr %13, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !6663, metadata !DIExpression()), !dbg !7413
  store i64 8, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !6667, metadata !DIExpression()), !dbg !7415
  %14 = load ptr, ptr %2, align 8, !dbg !7416
  %15 = load i64, ptr %3, align 8, !dbg !7417
  %16 = trunc i64 %15 to i32, !dbg !7417
  %17 = call zeroext i1 @kasan_check_write(ptr noundef %14, i32 noundef %16), !dbg !7418
  %18 = load ptr, ptr %2, align 8, !dbg !7419
  %19 = load i64, ptr %3, align 8, !dbg !7419
  call void @kcsan_check_access(ptr noundef %18, i64 noundef %19, i32 noundef 7), !dbg !7419
  call void @llvm.dbg.declare(metadata ptr %7, metadata !7420, metadata !DIExpression()), !dbg !7422
  store ptr null, ptr %7, align 8, !dbg !7422
  %20 = load ptr, ptr %7, align 8, !dbg !7422
  %21 = load ptr, ptr %5, align 8, !dbg !7422
  %22 = call ptr asm sideeffect "xchgq ${0:q}, $1\0A", "=r,=*m,0,*m,~{memory},~{cc},~{dirflag},~{fpsr},~{flags}"(ptr elementtype(ptr) %21, ptr %20, ptr elementtype(ptr) %21) #12, !dbg !7422, !srcloc !7423
  store ptr %22, ptr %7, align 8, !dbg !7422
  %23 = load ptr, ptr %7, align 8, !dbg !7422
  store ptr %23, ptr %8, align 8, !dbg !7422
  %24 = load ptr, ptr %8, align 8, !dbg !7422
  store ptr %24, ptr %6, align 8, !dbg !7410
  %25 = load ptr, ptr %6, align 8, !dbg !7410
  ret ptr %25, !dbg !7424
}

declare void @security_file_free(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal void @percpu_counter_dec(ptr noundef %0) #0 !dbg !7425 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7426, metadata !DIExpression()), !dbg !7427
  %3 = load ptr, ptr %2, align 8, !dbg !7428
  call void @percpu_counter_add(ptr noundef %3, i64 noundef -1), !dbg !7429
  ret void, !dbg !7430
}

declare zeroext i1 @llist_add_batch(ptr noundef, ptr noundef, ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal zeroext i1 @queue_delayed_work(ptr noundef %0, ptr noundef %1, i64 noundef %2) #0 !dbg !7431 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !7434, metadata !DIExpression()), !dbg !7435
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !7436, metadata !DIExpression()), !dbg !7437
  store i64 %2, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !7438, metadata !DIExpression()), !dbg !7439
  %7 = load ptr, ptr %4, align 8, !dbg !7440
  %8 = load ptr, ptr %5, align 8, !dbg !7441
  %9 = load i64, ptr %6, align 8, !dbg !7442
  %10 = call zeroext i1 @queue_delayed_work_on(i32 noundef 64, ptr noundef %7, ptr noundef %8, i64 noundef %9), !dbg !7443
  ret i1 %10, !dbg !7444
}

declare zeroext i1 @queue_delayed_work_on(i32 noundef, ptr noundef, ptr noundef, i64 noundef) #2

declare void @delayed_work_timer_fn(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal void @fsnotify_close(ptr noundef %0) #0 !dbg !7445 {
  %2 = alloca ptr, align 8
  %3 = alloca i32, align 4
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7447, metadata !DIExpression()), !dbg !7448
  call void @llvm.dbg.declare(metadata ptr %3, metadata !7449, metadata !DIExpression()), !dbg !7450
  %4 = load ptr, ptr %2, align 8, !dbg !7451
  %5 = getelementptr inbounds %struct.file, ptr %4, i32 0, i32 2, !dbg !7452
  %6 = load i32, ptr %5, align 4, !dbg !7452
  %7 = and i32 %6, 2, !dbg !7453
  %8 = icmp ne i32 %7, 0, !dbg !7454
  %9 = zext i1 %8 to i64, !dbg !7454
  %10 = select i1 %8, i32 8, i32 16, !dbg !7454
  store i32 %10, ptr %3, align 4, !dbg !7450
  %11 = load ptr, ptr %2, align 8, !dbg !7455
  %12 = load i32, ptr %3, align 4, !dbg !7456
  %13 = call i32 @fsnotify_file(ptr noundef %11, i32 noundef %12), !dbg !7457
  ret void, !dbg !7458
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @eventpoll_release(ptr noundef %0) #0 !dbg !7459 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7461, metadata !DIExpression()), !dbg !7462
  %3 = load ptr, ptr %2, align 8, !dbg !7463
  %4 = getelementptr inbounds %struct.file, ptr %3, i32 0, i32 17, !dbg !7463
  %5 = load ptr, ptr %4, align 8, !dbg !7463
  %6 = icmp ne ptr %5, null, !dbg !7463
  %7 = xor i1 %6, true, !dbg !7463
  %8 = xor i1 %7, true, !dbg !7463
  %9 = xor i1 %8, true, !dbg !7463
  %10 = zext i1 %9 to i32, !dbg !7463
  %11 = sext i32 %10 to i64, !dbg !7463
  %12 = icmp ne i64 %11, 0, !dbg !7463
  br i1 %12, label %13, label %14, !dbg !7465

13:                                               ; preds = %1
  br label %16, !dbg !7466

14:                                               ; preds = %1
  %15 = load ptr, ptr %2, align 8, !dbg !7467
  call void @eventpoll_release_file(ptr noundef %15), !dbg !7468
  br label %16, !dbg !7469

16:                                               ; preds = %14, %13
  ret void, !dbg !7469
}

declare void @locks_remove_file(ptr noundef) #2

declare void @security_file_release(ptr noundef) #2

declare void @cdev_put(ptr noundef) #2

declare void @module_put(ptr noundef) #2

declare void @file_f_owner_release(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal void @put_file_access(ptr noundef %0) #0 !dbg !7470 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7472, metadata !DIExpression()), !dbg !7473
  %3 = load ptr, ptr %2, align 8, !dbg !7474
  %4 = getelementptr inbounds %struct.file, ptr %3, i32 0, i32 2, !dbg !7476
  %5 = load i32, ptr %4, align 4, !dbg !7476
  %6 = and i32 %5, 3, !dbg !7477
  %7 = icmp eq i32 %6, 1, !dbg !7478
  br i1 %7, label %8, label %12, !dbg !7479

8:                                                ; preds = %1
  %9 = load ptr, ptr %2, align 8, !dbg !7480
  %10 = getelementptr inbounds %struct.file, ptr %9, i32 0, i32 6, !dbg !7482
  %11 = load ptr, ptr %10, align 8, !dbg !7482
  call void @i_readcount_dec(ptr noundef %11), !dbg !7483
  br label %21, !dbg !7484

12:                                               ; preds = %1
  %13 = load ptr, ptr %2, align 8, !dbg !7485
  %14 = getelementptr inbounds %struct.file, ptr %13, i32 0, i32 2, !dbg !7487
  %15 = load i32, ptr %14, align 4, !dbg !7487
  %16 = and i32 %15, 65536, !dbg !7488
  %17 = icmp ne i32 %16, 0, !dbg !7488
  br i1 %17, label %18, label %20, !dbg !7489

18:                                               ; preds = %12
  %19 = load ptr, ptr %2, align 8, !dbg !7490
  call void @file_put_write_access(ptr noundef %19), !dbg !7492
  br label %20, !dbg !7493

20:                                               ; preds = %18, %12
  br label %21

21:                                               ; preds = %20, %8
  ret void, !dbg !7494
}

declare void @dput(ptr noundef) #2

declare void @dissolve_on_fput(ptr noundef) #2

declare void @mntput(ptr noundef) #2

declare i32 @__SCT__might_resched() #2

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @fsnotify_file(ptr noundef %0, i32 noundef %1) #0 !dbg !7495 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !7498, metadata !DIExpression()), !dbg !7499
  store i32 %1, ptr %5, align 4
  call void @llvm.dbg.declare(metadata ptr %5, metadata !7500, metadata !DIExpression()), !dbg !7501
  call void @llvm.dbg.declare(metadata ptr %6, metadata !7502, metadata !DIExpression()), !dbg !7503
  %7 = load ptr, ptr %4, align 8, !dbg !7504
  %8 = getelementptr inbounds %struct.file, ptr %7, i32 0, i32 2, !dbg !7506
  %9 = load i32, ptr %8, align 4, !dbg !7506
  %10 = and i32 %9, 67125248, !dbg !7507
  %11 = icmp ne i32 %10, 0, !dbg !7507
  br i1 %11, label %12, label %13, !dbg !7508

12:                                               ; preds = %2
  store i32 0, ptr %3, align 4, !dbg !7509
  br label %34, !dbg !7509

13:                                               ; preds = %2
  %14 = load ptr, ptr %4, align 8, !dbg !7510
  %15 = getelementptr inbounds %struct.file, ptr %14, i32 0, i32 10, !dbg !7511
  store ptr %15, ptr %6, align 8, !dbg !7512
  %16 = load i32, ptr %5, align 4, !dbg !7513
  %17 = and i32 %16, 458752, !dbg !7515
  %18 = icmp ne i32 %17, 0, !dbg !7515
  br i1 %18, label %19, label %27, !dbg !7516

19:                                               ; preds = %13
  %20 = load ptr, ptr %6, align 8, !dbg !7517
  %21 = getelementptr inbounds %struct.path, ptr %20, i32 0, i32 1, !dbg !7518
  %22 = load ptr, ptr %21, align 8, !dbg !7518
  %23 = getelementptr inbounds %struct.dentry, ptr %22, i32 0, i32 8, !dbg !7519
  %24 = load ptr, ptr %23, align 8, !dbg !7519
  %25 = call zeroext i1 @fsnotify_sb_has_priority_watchers(ptr noundef %24, i32 noundef 1), !dbg !7520
  br i1 %25, label %27, label %26, !dbg !7521

26:                                               ; preds = %19
  store i32 0, ptr %3, align 4, !dbg !7522
  br label %34, !dbg !7522

27:                                               ; preds = %19, %13
  %28 = load ptr, ptr %6, align 8, !dbg !7523
  %29 = getelementptr inbounds %struct.path, ptr %28, i32 0, i32 1, !dbg !7524
  %30 = load ptr, ptr %29, align 8, !dbg !7524
  %31 = load i32, ptr %5, align 4, !dbg !7525
  %32 = load ptr, ptr %6, align 8, !dbg !7526
  %33 = call i32 @fsnotify_parent(ptr noundef %30, i32 noundef %31, ptr noundef %32, i32 noundef 1), !dbg !7527
  store i32 %33, ptr %3, align 4, !dbg !7528
  br label %34, !dbg !7528

34:                                               ; preds = %27, %26, %12
  %35 = load i32, ptr %3, align 4, !dbg !7529
  ret i32 %35, !dbg !7529
}

; Function Attrs: noinline nounwind optnone uwtable
define internal zeroext i1 @fsnotify_sb_has_priority_watchers(ptr noundef %0, i32 noundef %1) #0 !dbg !7530 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i64, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i1, align 1
  %10 = alloca ptr, align 8
  %11 = alloca i32, align 4
  %12 = alloca ptr, align 8
  store ptr %0, ptr %10, align 8
  call void @llvm.dbg.declare(metadata ptr %10, metadata !7533, metadata !DIExpression()), !dbg !7534
  store i32 %1, ptr %11, align 4
  call void @llvm.dbg.declare(metadata ptr %11, metadata !7535, metadata !DIExpression()), !dbg !7536
  call void @llvm.dbg.declare(metadata ptr %12, metadata !7537, metadata !DIExpression()), !dbg !7538
  %13 = load ptr, ptr %10, align 8, !dbg !7539
  %14 = call ptr @fsnotify_sb_info(ptr noundef %13), !dbg !7540
  store ptr %14, ptr %12, align 8, !dbg !7538
  %15 = load ptr, ptr %12, align 8, !dbg !7541
  %16 = icmp ne ptr %15, null, !dbg !7541
  br i1 %16, label %18, label %17, !dbg !7543

17:                                               ; preds = %2
  store i1 false, ptr %9, align 1, !dbg !7544
  br label %37, !dbg !7544

18:                                               ; preds = %2
  %19 = load ptr, ptr %12, align 8, !dbg !7545
  %20 = getelementptr inbounds %struct.fsnotify_sb_info, ptr %19, i32 0, i32 1, !dbg !7546
  %21 = load i32, ptr %11, align 4, !dbg !7547
  %22 = sext i32 %21 to i64, !dbg !7545
  %23 = getelementptr inbounds [3 x %struct.atomic64_t], ptr %20, i64 0, i64 %22, !dbg !7545
  store ptr %23, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !6988, metadata !DIExpression()), !dbg !7548
  %24 = load ptr, ptr %8, align 8, !dbg !7550
  store ptr %24, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !6997, metadata !DIExpression()), !dbg !7551
  store i64 8, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !7001, metadata !DIExpression()), !dbg !7553
  %25 = load ptr, ptr %6, align 8, !dbg !7554
  %26 = load i64, ptr %7, align 8, !dbg !7555
  %27 = trunc i64 %26 to i32, !dbg !7555
  %28 = call zeroext i1 @kasan_check_read(ptr noundef %25, i32 noundef %27), !dbg !7556
  %29 = load ptr, ptr %6, align 8, !dbg !7557
  %30 = load i64, ptr %7, align 8, !dbg !7557
  call void @kcsan_check_access(ptr noundef %29, i64 noundef %30, i32 noundef 4), !dbg !7557
  %31 = load ptr, ptr %8, align 8, !dbg !7558
  store ptr %31, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !7008, metadata !DIExpression()), !dbg !7559
  %32 = load ptr, ptr %5, align 8, !dbg !7561
  store ptr %32, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !7013, metadata !DIExpression()), !dbg !7562
  %33 = load ptr, ptr %4, align 8, !dbg !7564
  store ptr %33, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !7022, metadata !DIExpression()), !dbg !7565
  %34 = load ptr, ptr %3, align 8, !dbg !7567
  %35 = load volatile i64, ptr %34, align 8, !dbg !7567
  %36 = icmp ne i64 %35, 0, !dbg !7568
  store i1 %36, ptr %9, align 1, !dbg !7569
  br label %37, !dbg !7569

37:                                               ; preds = %18, %17
  %38 = load i1, ptr %9, align 1, !dbg !7570
  ret i1 %38, !dbg !7570
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @fsnotify_parent(ptr noundef %0, i32 noundef %1, ptr noundef %2, i32 noundef %3) #0 !dbg !7571 {
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 8
  %7 = alloca i32, align 4
  %8 = alloca ptr, align 8
  %9 = alloca i32, align 4
  %10 = alloca ptr, align 8
  store ptr %0, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !7574, metadata !DIExpression()), !dbg !7575
  store i32 %1, ptr %7, align 4
  call void @llvm.dbg.declare(metadata ptr %7, metadata !7576, metadata !DIExpression()), !dbg !7577
  store ptr %2, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !7578, metadata !DIExpression()), !dbg !7579
  store i32 %3, ptr %9, align 4
  call void @llvm.dbg.declare(metadata ptr %9, metadata !7580, metadata !DIExpression()), !dbg !7581
  call void @llvm.dbg.declare(metadata ptr %10, metadata !7582, metadata !DIExpression()), !dbg !7583
  %11 = load ptr, ptr %6, align 8, !dbg !7584
  %12 = call ptr @d_inode(ptr noundef %11), !dbg !7585
  store ptr %12, ptr %10, align 8, !dbg !7583
  %13 = load ptr, ptr %10, align 8, !dbg !7586
  %14 = getelementptr inbounds %struct.inode, ptr %13, i32 0, i32 8, !dbg !7588
  %15 = load ptr, ptr %14, align 8, !dbg !7588
  %16 = call zeroext i1 @fsnotify_sb_has_watchers(ptr noundef %15), !dbg !7589
  br i1 %16, label %18, label %17, !dbg !7590

17:                                               ; preds = %4
  store i32 0, ptr %5, align 4, !dbg !7591
  br label %55, !dbg !7591

18:                                               ; preds = %4
  %19 = load ptr, ptr %10, align 8, !dbg !7592
  %20 = getelementptr inbounds %struct.inode, ptr %19, i32 0, i32 0, !dbg !7592
  %21 = load i16, ptr %20, align 8, !dbg !7592
  %22 = zext i16 %21 to i32, !dbg !7592
  %23 = and i32 %22, 61440, !dbg !7592
  %24 = icmp eq i32 %23, 16384, !dbg !7592
  br i1 %24, label %25, label %36, !dbg !7594

25:                                               ; preds = %18
  %26 = load i32, ptr %7, align 4, !dbg !7595
  %27 = or i32 %26, 1073741824, !dbg !7595
  store i32 %27, ptr %7, align 4, !dbg !7595
  %28 = load ptr, ptr %6, align 8, !dbg !7597
  %29 = getelementptr inbounds %struct.dentry, ptr %28, i32 0, i32 0, !dbg !7599
  %30 = load i32, ptr %29, align 8, !dbg !7599
  %31 = zext i32 %30 to i64, !dbg !7597
  %32 = and i64 %31, 16384, !dbg !7600
  %33 = icmp ne i64 %32, 0, !dbg !7600
  br i1 %33, label %35, label %34, !dbg !7601

34:                                               ; preds = %25
  br label %49, !dbg !7602

35:                                               ; preds = %25
  br label %36, !dbg !7603

36:                                               ; preds = %35, %18
  %37 = load ptr, ptr %6, align 8, !dbg !7604
  %38 = load ptr, ptr %6, align 8, !dbg !7604
  %39 = getelementptr inbounds %struct.dentry, ptr %38, i32 0, i32 3, !dbg !7604
  %40 = load ptr, ptr %39, align 8, !dbg !7604
  %41 = icmp eq ptr %37, %40, !dbg !7604
  br i1 %41, label %42, label %43, !dbg !7606

42:                                               ; preds = %36
  br label %49, !dbg !7607

43:                                               ; preds = %36
  %44 = load ptr, ptr %6, align 8, !dbg !7608
  %45 = load i32, ptr %7, align 4, !dbg !7609
  %46 = load ptr, ptr %8, align 8, !dbg !7610
  %47 = load i32, ptr %9, align 4, !dbg !7611
  %48 = call i32 @__fsnotify_parent(ptr noundef %44, i32 noundef %45, ptr noundef %46, i32 noundef %47), !dbg !7612
  store i32 %48, ptr %5, align 4, !dbg !7613
  br label %55, !dbg !7613

49:                                               ; preds = %42, %34
  call void @llvm.dbg.label(metadata !7614), !dbg !7615
  %50 = load i32, ptr %7, align 4, !dbg !7616
  %51 = load ptr, ptr %8, align 8, !dbg !7617
  %52 = load i32, ptr %9, align 4, !dbg !7618
  %53 = load ptr, ptr %10, align 8, !dbg !7619
  %54 = call i32 @fsnotify(i32 noundef %50, ptr noundef %51, i32 noundef %52, ptr noundef null, ptr noundef null, ptr noundef %53, i32 noundef 0), !dbg !7620
  store i32 %54, ptr %5, align 4, !dbg !7621
  br label %55, !dbg !7621

55:                                               ; preds = %49, %43, %17
  %56 = load i32, ptr %5, align 4, !dbg !7622
  ret i32 %56, !dbg !7622
}

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @fsnotify_sb_info(ptr noundef %0) #0 !dbg !7623 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7626, metadata !DIExpression()), !dbg !7627
  br label %4, !dbg !7628

4:                                                ; preds = %1
  br label %5, !dbg !7630

5:                                                ; preds = %4
  %6 = load ptr, ptr %2, align 8, !dbg !7628
  %7 = getelementptr inbounds %struct.super_block, ptr %6, i32 0, i32 34, !dbg !7628
  %8 = load volatile ptr, ptr %7, align 16, !dbg !7628
  store ptr %8, ptr %3, align 8, !dbg !7630
  %9 = load ptr, ptr %3, align 8, !dbg !7628
  ret ptr %9, !dbg !7632
}

; Function Attrs: noinline nounwind optnone uwtable
define internal zeroext i1 @kasan_check_read(ptr noundef %0, i32 noundef %1) #0 !dbg !7633 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !7634, metadata !DIExpression()), !dbg !7635
  store i32 %1, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !7636, metadata !DIExpression()), !dbg !7637
  ret i1 true, !dbg !7638
}

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @d_inode(ptr noundef %0) #0 !dbg !7639 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7642, metadata !DIExpression()), !dbg !7643
  %3 = load ptr, ptr %2, align 8, !dbg !7644
  %4 = getelementptr inbounds %struct.dentry, ptr %3, i32 0, i32 5, !dbg !7645
  %5 = load ptr, ptr %4, align 8, !dbg !7645
  ret ptr %5, !dbg !7646
}

; Function Attrs: noinline nounwind optnone uwtable
define internal zeroext i1 @fsnotify_sb_has_watchers(ptr noundef %0) #0 !dbg !7647 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7650, metadata !DIExpression()), !dbg !7651
  %3 = load ptr, ptr %2, align 8, !dbg !7652
  %4 = call zeroext i1 @fsnotify_sb_has_priority_watchers(ptr noundef %3, i32 noundef 0), !dbg !7653
  ret i1 %4, !dbg !7654
}

declare i32 @__fsnotify_parent(ptr noundef, i32 noundef, ptr noundef, i32 noundef) #2

declare i32 @fsnotify(i32 noundef, ptr noundef, i32 noundef, ptr noundef, ptr noundef, ptr noundef, i32 noundef) #2

declare void @eventpoll_release_file(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal void @i_readcount_dec(ptr noundef %0) #0 !dbg !7655 {
  %2 = alloca i32, align 4
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca i64, align 8
  %11 = alloca ptr, align 8
  %12 = alloca ptr, align 8
  store ptr %0, ptr %12, align 8
  call void @llvm.dbg.declare(metadata ptr %12, metadata !7656, metadata !DIExpression()), !dbg !7657
  br label %13, !dbg !7658

13:                                               ; preds = %1
  %14 = load ptr, ptr %12, align 8, !dbg !7659
  %15 = getelementptr inbounds %struct.inode, ptr %14, i32 0, i32 42, !dbg !7659
  store ptr %15, ptr %11, align 8
  call void @llvm.dbg.declare(metadata ptr %11, metadata !7662, metadata !DIExpression()), !dbg !7666
  %16 = load ptr, ptr %11, align 8, !dbg !7668
  store ptr %16, ptr %9, align 8
  call void @llvm.dbg.declare(metadata ptr %9, metadata !6663, metadata !DIExpression()), !dbg !7669
  store i64 4, ptr %10, align 8
  call void @llvm.dbg.declare(metadata ptr %10, metadata !6667, metadata !DIExpression()), !dbg !7671
  %17 = load ptr, ptr %9, align 8, !dbg !7672
  %18 = load i64, ptr %10, align 8, !dbg !7673
  %19 = trunc i64 %18 to i32, !dbg !7673
  %20 = call zeroext i1 @kasan_check_write(ptr noundef %17, i32 noundef %19), !dbg !7674
  %21 = load ptr, ptr %9, align 8, !dbg !7675
  %22 = load i64, ptr %10, align 8, !dbg !7675
  call void @kcsan_check_access(ptr noundef %21, i64 noundef %22, i32 noundef 7), !dbg !7675
  %23 = load ptr, ptr %11, align 8, !dbg !7676
  store ptr %23, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !7677, metadata !DIExpression()), !dbg !7679
  %24 = load ptr, ptr %8, align 8, !dbg !7681
  store i32 1, ptr %6, align 4
  call void @llvm.dbg.declare(metadata ptr %6, metadata !7682, metadata !DIExpression()), !dbg !7686
  store ptr %24, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !7688, metadata !DIExpression()), !dbg !7689
  %25 = load i32, ptr %6, align 4, !dbg !7690
  %26 = sub nsw i32 0, %25, !dbg !7690
  %27 = load ptr, ptr %7, align 8, !dbg !7690
  store i32 %26, ptr %2, align 4
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7691, metadata !DIExpression()), !dbg !7693
  store ptr %27, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !7695, metadata !DIExpression()), !dbg !7696
  %28 = load i32, ptr %2, align 4, !dbg !7697
  call void @llvm.dbg.declare(metadata ptr %4, metadata !7698, metadata !DIExpression()), !dbg !7700
  %29 = load i32, ptr %2, align 4, !dbg !7700
  store i32 %29, ptr %4, align 4, !dbg !7700
  %30 = load i32, ptr %4, align 4, !dbg !7700
  %31 = load ptr, ptr %3, align 8, !dbg !7700
  %32 = call i32 asm sideeffect ".pushsection .smp_locks,\22a\22\0A.balign 4\0A.long 671f - .\0A.popsection\0A671:\0A\09lock; xaddl $0, $1\0A", "=r,=*m,0,*m,~{memory},~{cc},~{dirflag},~{fpsr},~{flags}"(ptr elementtype(i32) %31, i32 %30, ptr elementtype(i32) %31) #12, !dbg !7700, !srcloc !7701
  store i32 %32, ptr %4, align 4, !dbg !7700
  %33 = load i32, ptr %4, align 4, !dbg !7700
  store i32 %33, ptr %5, align 4, !dbg !7700
  %34 = load i32, ptr %5, align 4, !dbg !7700
  %35 = add nsw i32 %28, %34, !dbg !7702
  %36 = icmp slt i32 %35, 0, !dbg !7659
  %37 = xor i1 %36, true, !dbg !7659
  %38 = xor i1 %37, true, !dbg !7659
  %39 = zext i1 %38 to i32, !dbg !7659
  %40 = sext i32 %39 to i64, !dbg !7659
  %41 = icmp ne i64 %40, 0, !dbg !7659
  br i1 %41, label %42, label %47, !dbg !7703

42:                                               ; preds = %13
  br label %43, !dbg !7659

43:                                               ; preds = %42
  call void asm sideeffect "294: nop\0A\09.pushsection .discard.instr_begin\0A\09.long 294b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 294) #12, !dbg !7704, !srcloc !7707
  br label %44, !dbg !7708

44:                                               ; preds = %43
  call void asm sideeffect "1:\09.byte 0x0f, 0x0b\0A.pushsection __bug_table,\22aw\22\0A2:\09.long 1b - .\09# bug_entry::bug_addr\0A\09.long ${0:c} - .\09# bug_entry::file\0A\09.word ${1:c}\09# bug_entry::line\0A\09.word ${2:c}\09# bug_entry::flags\0A\09.org 2b+${3:c}\0A.popsection\0A", "i,i,i,i,~{dirflag},~{fpsr},~{flags}"(ptr @.str.10, i32 3039, i32 0, i64 12) #12, !dbg !7709, !srcloc !7711
  br label %45, !dbg !7709

45:                                               ; preds = %44
  unreachable, !dbg !7708

46:                                               ; No predecessors!
  br label %47, !dbg !7708

47:                                               ; preds = %46, %13
  br label %48, !dbg !7703

48:                                               ; preds = %47
  ret void, !dbg !7712
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @file_put_write_access(ptr noundef %0) #0 !dbg !7713 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7714, metadata !DIExpression()), !dbg !7715
  %3 = load ptr, ptr %2, align 8, !dbg !7716
  %4 = getelementptr inbounds %struct.file, ptr %3, i32 0, i32 6, !dbg !7717
  %5 = load ptr, ptr %4, align 8, !dbg !7717
  call void @put_write_access(ptr noundef %5), !dbg !7718
  %6 = load ptr, ptr %2, align 8, !dbg !7719
  %7 = getelementptr inbounds %struct.file, ptr %6, i32 0, i32 10, !dbg !7720
  %8 = getelementptr inbounds %struct.path, ptr %7, i32 0, i32 0, !dbg !7721
  %9 = load ptr, ptr %8, align 8, !dbg !7721
  call void @mnt_put_write_access(ptr noundef %9), !dbg !7722
  %10 = load ptr, ptr %2, align 8, !dbg !7723
  %11 = getelementptr inbounds %struct.file, ptr %10, i32 0, i32 2, !dbg !7723
  %12 = load i32, ptr %11, align 4, !dbg !7723
  %13 = and i32 %12, 33554432, !dbg !7723
  %14 = icmp ne i32 %13, 0, !dbg !7723
  %15 = xor i1 %14, true, !dbg !7723
  %16 = xor i1 %15, true, !dbg !7723
  %17 = zext i1 %16 to i32, !dbg !7723
  %18 = sext i32 %17 to i64, !dbg !7723
  %19 = icmp ne i64 %18, 0, !dbg !7723
  br i1 %19, label %20, label %25, !dbg !7725

20:                                               ; preds = %1
  %21 = load ptr, ptr %2, align 8, !dbg !7726
  %22 = call ptr @backing_file_user_path(ptr noundef %21), !dbg !7727
  %23 = getelementptr inbounds %struct.path, ptr %22, i32 0, i32 0, !dbg !7728
  %24 = load ptr, ptr %23, align 8, !dbg !7728
  call void @mnt_put_write_access(ptr noundef %24), !dbg !7729
  br label %25, !dbg !7729

25:                                               ; preds = %20, %1
  ret void, !dbg !7730
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @put_write_access(ptr noundef %0) #0 !dbg !7731 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  store ptr %0, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !7732, metadata !DIExpression()), !dbg !7733
  %8 = load ptr, ptr %7, align 8, !dbg !7734
  %9 = getelementptr inbounds %struct.inode, ptr %8, i32 0, i32 41, !dbg !7735
  store ptr %9, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !7736, metadata !DIExpression()), !dbg !7738
  %10 = load ptr, ptr %6, align 8, !dbg !7740
  store ptr %10, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !6663, metadata !DIExpression()), !dbg !7741
  store i64 4, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !6667, metadata !DIExpression()), !dbg !7743
  %11 = load ptr, ptr %4, align 8, !dbg !7744
  %12 = load i64, ptr %5, align 8, !dbg !7745
  %13 = trunc i64 %12 to i32, !dbg !7745
  %14 = call zeroext i1 @kasan_check_write(ptr noundef %11, i32 noundef %13), !dbg !7746
  %15 = load ptr, ptr %4, align 8, !dbg !7747
  %16 = load i64, ptr %5, align 8, !dbg !7747
  call void @kcsan_check_access(ptr noundef %15, i64 noundef %16, i32 noundef 7), !dbg !7747
  %17 = load ptr, ptr %6, align 8, !dbg !7748
  store ptr %17, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !7749, metadata !DIExpression()), !dbg !7751
  %18 = load ptr, ptr %3, align 8, !dbg !7753
  store ptr %18, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !7754, metadata !DIExpression()), !dbg !7756
  %19 = load ptr, ptr %2, align 8, !dbg !7758
  call void asm sideeffect ".pushsection .smp_locks,\22a\22\0A.balign 4\0A.long 671f - .\0A.popsection\0A671:\0A\09lock; decl $0", "=*m,*m,~{memory},~{dirflag},~{fpsr},~{flags}"(ptr elementtype(i32) %19, ptr elementtype(i32) %19) #12, !dbg !7759, !srcloc !7760
  ret void, !dbg !7761
}

declare void @mnt_put_write_access(ptr noundef) #2

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { noinline nounwind optnone allocsize(0) uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { argmemonly nocallback nofree nounwind willreturn }
attributes #5 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
attributes #6 = { convergent nocallback nofree nosync nounwind readnone willreturn }
attributes #7 = { allocsize(0) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #8 = { allocsize(2) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #9 = { nocallback nofree nosync nounwind readnone willreturn }
attributes #10 = { allocsize(0) }
attributes #11 = { nounwind allocsize(0) }
attributes #12 = { nounwind }
attributes #13 = { nounwind allocsize(2) }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!5734, !5735, !5736, !5737, !5738, !5739, !5740}
!llvm.ident = !{!5741}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_backing_file_user_path506", scope: !2, file: !5631, line: 61, type: !40, isLocal: true, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "Debian clang version 15.0.6", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !487, globals: !5649, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "/mlx_devbox/users/mayunlong.39/playground/LLM4Con/kernel_experiment/SYZBOT-3b6b32dc50537a49/src/fs/file_table.c", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "9f0464e3d7acb9de8dd666f6494b200a")
!4 = !{!5, !13, !21, !229, !245, !250, !256, !262, !269, !274, !282, !289, !295, !304, !309, !314, !320, !326, !333, !341, !347, !353, !365, !370, !379, !407, !415, !436, !451, !456, !465, !473, !480}
!5 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "module_state", file: !6, line: 318, baseType: !7, size: 32, elements: !8)
!6 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/module.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "7d495b4f3724f57a488930758cacb132")
!7 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!8 = !{!9, !10, !11, !12}
!9 = !DIEnumerator(name: "MODULE_STATE_LIVE", value: 0)
!10 = !DIEnumerator(name: "MODULE_STATE_COMING", value: 1)
!11 = !DIEnumerator(name: "MODULE_STATE_GOING", value: 2)
!12 = !DIEnumerator(name: "MODULE_STATE_UNFORMED", value: 3)
!13 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "memory_type", file: !14, line: 69, baseType: !7, size: 32, elements: !15)
!14 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/memremap.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "507f8dd107faa8a15e3ee042561004ea")
!15 = !{!16, !17, !18, !19, !20}
!16 = !DIEnumerator(name: "MEMORY_DEVICE_PRIVATE", value: 1)
!17 = !DIEnumerator(name: "MEMORY_DEVICE_COHERENT", value: 2)
!18 = !DIEnumerator(name: "MEMORY_DEVICE_FS_DAX", value: 3)
!19 = !DIEnumerator(name: "MEMORY_DEVICE_GENERIC", value: 4)
!20 = !DIEnumerator(name: "MEMORY_DEVICE_PCI_P2PDMA", value: 5)
!21 = !DICompositeType(tag: DW_TAG_enumeration_type, scope: !23, file: !22, line: 187, baseType: !7, size: 32, elements: !226)
!22 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/sysctl.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "75b6b00bf6d742058040e2ec494714a8")
!23 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "ctl_table_header", file: !22, line: 162, size: 704, elements: !24)
!24 = !{!25, !137, !150, !151, !206, !207, !209, !215, !225}
!25 = !DIDerivedType(tag: DW_TAG_member, scope: !23, file: !22, line: 163, baseType: !26, size: 192)
!26 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !23, file: !22, line: 163, size: 192, elements: !27)
!27 = !{!28, !128}
!28 = !DIDerivedType(tag: DW_TAG_member, scope: !26, file: !22, line: 164, baseType: !29, size: 192)
!29 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !26, file: !22, line: 164, size: 192, elements: !30)
!30 = !{!31, !124, !125, !126, !127}
!31 = !DIDerivedType(tag: DW_TAG_member, name: "ctl_table", scope: !29, file: !22, line: 165, baseType: !32, size: 64)
!32 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !33, size: 64)
!33 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "ctl_table", file: !22, line: 135, size: 448, elements: !34)
!34 = !{!35, !39, !41, !43, !47, !64, !122, !123}
!35 = !DIDerivedType(tag: DW_TAG_member, name: "procname", scope: !33, file: !22, line: 136, baseType: !36, size: 64)
!36 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !37, size: 64)
!37 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !38)
!38 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!39 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !33, file: !22, line: 137, baseType: !40, size: 64, offset: 64)
!40 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!41 = !DIDerivedType(tag: DW_TAG_member, name: "maxlen", scope: !33, file: !22, line: 138, baseType: !42, size: 32, offset: 128)
!42 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!43 = !DIDerivedType(tag: DW_TAG_member, name: "mode", scope: !33, file: !22, line: 139, baseType: !44, size: 16, offset: 160)
!44 = !DIDerivedType(tag: DW_TAG_typedef, name: "umode_t", file: !45, line: 24, baseType: !46)
!45 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "78876adc9cb1ab1cc8763f12db45ddd5")
!46 = !DIBasicType(name: "unsigned short", size: 16, encoding: DW_ATE_unsigned)
!47 = !DIDerivedType(tag: DW_TAG_member, name: "proc_handler", scope: !33, file: !22, line: 140, baseType: !48, size: 64, offset: 192)
!48 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !49, size: 64)
!49 = !DIDerivedType(tag: DW_TAG_typedef, name: "proc_handler", file: !22, line: 64, baseType: !50)
!50 = !DISubroutineType(types: !51)
!51 = !{!42, !52, !42, !40, !54, !60}
!52 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !53, size: 64)
!53 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !33)
!54 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !55, size: 64)
!55 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !45, line: 61, baseType: !56)
!56 = !DIDerivedType(tag: DW_TAG_typedef, name: "__kernel_size_t", file: !57, line: 72, baseType: !58)
!57 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/uapi/asm-generic/posix_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "02144a993b28778c1e6c05bf0b9f51db")
!58 = !DIDerivedType(tag: DW_TAG_typedef, name: "__kernel_ulong_t", file: !57, line: 16, baseType: !59)
!59 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!60 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !61, size: 64)
!61 = !DIDerivedType(tag: DW_TAG_typedef, name: "loff_t", file: !45, line: 52, baseType: !62)
!62 = !DIDerivedType(tag: DW_TAG_typedef, name: "__kernel_loff_t", file: !57, line: 88, baseType: !63)
!63 = !DIBasicType(name: "long long", size: 64, encoding: DW_ATE_signed)
!64 = !DIDerivedType(tag: DW_TAG_member, name: "poll", scope: !33, file: !22, line: 141, baseType: !65, size: 64, offset: 256)
!65 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !66, size: 64)
!66 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "ctl_table_poll", file: !22, line: 117, size: 256, elements: !67)
!67 = !{!68, !73}
!68 = !DIDerivedType(tag: DW_TAG_member, name: "event", scope: !66, file: !22, line: 118, baseType: !69, size: 32)
!69 = !DIDerivedType(tag: DW_TAG_typedef, name: "atomic_t", file: !45, line: 177, baseType: !70)
!70 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !45, line: 175, size: 32, elements: !71)
!71 = !{!72}
!72 = !DIDerivedType(tag: DW_TAG_member, name: "counter", scope: !70, file: !45, line: 176, baseType: !42, size: 32)
!73 = !DIDerivedType(tag: DW_TAG_member, name: "wait", scope: !66, file: !22, line: 119, baseType: !74, size: 192, offset: 64)
!74 = !DIDerivedType(tag: DW_TAG_typedef, name: "wait_queue_head_t", file: !75, line: 39, baseType: !76)
!75 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/wait.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "46219597915782cd2e2112a27ba7fb90")
!76 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "wait_queue_head", file: !75, line: 35, size: 192, elements: !77)
!77 = !{!78, !116}
!78 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !76, file: !75, line: 36, baseType: !79, size: 32)
!79 = !DIDerivedType(tag: DW_TAG_typedef, name: "spinlock_t", file: !80, line: 29, baseType: !81)
!80 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/spinlock_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "fc7950471ffdc176b6c133b1f7370f88")
!81 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "spinlock", file: !80, line: 17, size: 32, elements: !82)
!82 = !{!83}
!83 = !DIDerivedType(tag: DW_TAG_member, scope: !81, file: !80, line: 18, baseType: !84, size: 32)
!84 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !81, file: !80, line: 18, size: 32, elements: !85)
!85 = !{!86}
!86 = !DIDerivedType(tag: DW_TAG_member, name: "rlock", scope: !84, file: !80, line: 19, baseType: !87, size: 32)
!87 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "raw_spinlock", file: !88, line: 14, size: 32, elements: !89)
!88 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/spinlock_types_raw.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "9bbdf30cd339c0a21e6a004bafba78e0")
!89 = !{!90}
!90 = !DIDerivedType(tag: DW_TAG_member, name: "raw_lock", scope: !87, file: !88, line: 15, baseType: !91, size: 32)
!91 = !DIDerivedType(tag: DW_TAG_typedef, name: "arch_spinlock_t", file: !92, line: 44, baseType: !93)
!92 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/asm-generic/qspinlock_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "2a1236eda9a125c2ce03b9a345491b46")
!93 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "qspinlock", file: !92, line: 14, size: 32, elements: !94)
!94 = !{!95}
!95 = !DIDerivedType(tag: DW_TAG_member, scope: !93, file: !92, line: 15, baseType: !96, size: 32)
!96 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !93, file: !92, line: 15, size: 32, elements: !97)
!97 = !{!98, !99, !109}
!98 = !DIDerivedType(tag: DW_TAG_member, name: "val", scope: !96, file: !92, line: 16, baseType: !69, size: 32)
!99 = !DIDerivedType(tag: DW_TAG_member, scope: !96, file: !92, line: 24, baseType: !100, size: 16)
!100 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !96, file: !92, line: 24, size: 16, elements: !101)
!101 = !{!102, !108}
!102 = !DIDerivedType(tag: DW_TAG_member, name: "locked", scope: !100, file: !92, line: 25, baseType: !103, size: 8)
!103 = !DIDerivedType(tag: DW_TAG_typedef, name: "u8", file: !104, line: 17, baseType: !105)
!104 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/asm-generic/int-ll64.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "12ca7bdb629352cc4c9a492f86b435a7")
!105 = !DIDerivedType(tag: DW_TAG_typedef, name: "__u8", file: !106, line: 21, baseType: !107)
!106 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/uapi/asm-generic/int-ll64.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "f4d0ec5bcdd84e825a78a7add39d54dd")
!107 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!108 = !DIDerivedType(tag: DW_TAG_member, name: "pending", scope: !100, file: !92, line: 26, baseType: !103, size: 8, offset: 8)
!109 = !DIDerivedType(tag: DW_TAG_member, scope: !96, file: !92, line: 28, baseType: !110, size: 32)
!110 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !96, file: !92, line: 28, size: 32, elements: !111)
!111 = !{!112, !115}
!112 = !DIDerivedType(tag: DW_TAG_member, name: "locked_pending", scope: !110, file: !92, line: 29, baseType: !113, size: 16)
!113 = !DIDerivedType(tag: DW_TAG_typedef, name: "u16", file: !104, line: 19, baseType: !114)
!114 = !DIDerivedType(tag: DW_TAG_typedef, name: "__u16", file: !106, line: 24, baseType: !46)
!115 = !DIDerivedType(tag: DW_TAG_member, name: "tail", scope: !110, file: !92, line: 30, baseType: !113, size: 16, offset: 16)
!116 = !DIDerivedType(tag: DW_TAG_member, name: "head", scope: !76, file: !75, line: 37, baseType: !117, size: 128, offset: 64)
!117 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "list_head", file: !45, line: 193, size: 128, elements: !118)
!118 = !{!119, !121}
!119 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !117, file: !45, line: 194, baseType: !120, size: 64)
!120 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !117, size: 64)
!121 = !DIDerivedType(tag: DW_TAG_member, name: "prev", scope: !117, file: !45, line: 194, baseType: !120, size: 64, offset: 64)
!122 = !DIDerivedType(tag: DW_TAG_member, name: "extra1", scope: !33, file: !22, line: 142, baseType: !40, size: 64, offset: 320)
!123 = !DIDerivedType(tag: DW_TAG_member, name: "extra2", scope: !33, file: !22, line: 143, baseType: !40, size: 64, offset: 384)
!124 = !DIDerivedType(tag: DW_TAG_member, name: "ctl_table_size", scope: !29, file: !22, line: 166, baseType: !42, size: 32, offset: 64)
!125 = !DIDerivedType(tag: DW_TAG_member, name: "used", scope: !29, file: !22, line: 167, baseType: !42, size: 32, offset: 96)
!126 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !29, file: !22, line: 168, baseType: !42, size: 32, offset: 128)
!127 = !DIDerivedType(tag: DW_TAG_member, name: "nreg", scope: !29, file: !22, line: 169, baseType: !42, size: 32, offset: 160)
!128 = !DIDerivedType(tag: DW_TAG_member, name: "rcu", scope: !26, file: !22, line: 171, baseType: !129, size: 128, align: 64)
!129 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "callback_head", file: !45, line: 235, size: 128, align: 64, elements: !130)
!130 = !{!131, !133}
!131 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !129, file: !45, line: 236, baseType: !132, size: 64)
!132 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !129, size: 64)
!133 = !DIDerivedType(tag: DW_TAG_member, name: "func", scope: !129, file: !45, line: 237, baseType: !134, size: 64, offset: 64)
!134 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !135, size: 64)
!135 = !DISubroutineType(types: !136)
!136 = !{null, !132}
!137 = !DIDerivedType(tag: DW_TAG_member, name: "unregistering", scope: !23, file: !22, line: 173, baseType: !138, size: 64, offset: 192)
!138 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !139, size: 64)
!139 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "completion", file: !140, line: 26, size: 256, elements: !141)
!140 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/completion.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "b05d3ab7eb02dc86ab22cabfb4a314bb")
!141 = !{!142, !143}
!142 = !DIDerivedType(tag: DW_TAG_member, name: "done", scope: !139, file: !140, line: 27, baseType: !7, size: 32)
!143 = !DIDerivedType(tag: DW_TAG_member, name: "wait", scope: !139, file: !140, line: 28, baseType: !144, size: 192, offset: 64)
!144 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "swait_queue_head", file: !145, line: 43, size: 192, elements: !146)
!145 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/swait.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "14c02fe688b799b9b59537a234766fbe")
!146 = !{!147, !149}
!147 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !144, file: !145, line: 44, baseType: !148, size: 32)
!148 = !DIDerivedType(tag: DW_TAG_typedef, name: "raw_spinlock_t", file: !88, line: 23, baseType: !87)
!149 = !DIDerivedType(tag: DW_TAG_member, name: "task_list", scope: !144, file: !145, line: 45, baseType: !117, size: 128, offset: 64)
!150 = !DIDerivedType(tag: DW_TAG_member, name: "ctl_table_arg", scope: !23, file: !22, line: 174, baseType: !52, size: 64, offset: 256)
!151 = !DIDerivedType(tag: DW_TAG_member, name: "root", scope: !23, file: !22, line: 175, baseType: !152, size: 64, offset: 320)
!152 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !153, size: 64)
!153 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "ctl_table_root", file: !22, line: 204, size: 1024, elements: !154)
!154 = !{!155, !178, !182, !202}
!155 = !DIDerivedType(tag: DW_TAG_member, name: "default_set", scope: !153, file: !22, line: 205, baseType: !156, size: 832)
!156 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "ctl_table_set", file: !22, line: 199, size: 832, elements: !157)
!157 = !{!158, !163}
!158 = !DIDerivedType(tag: DW_TAG_member, name: "is_seen", scope: !156, file: !22, line: 200, baseType: !159, size: 64)
!159 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !160, size: 64)
!160 = !DISubroutineType(types: !161)
!161 = !{!42, !162}
!162 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !156, size: 64)
!163 = !DIDerivedType(tag: DW_TAG_member, name: "dir", scope: !156, file: !22, line: 201, baseType: !164, size: 768, offset: 64)
!164 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "ctl_dir", file: !22, line: 193, size: 768, elements: !165)
!165 = !{!166, !167}
!166 = !DIDerivedType(tag: DW_TAG_member, name: "header", scope: !164, file: !22, line: 195, baseType: !23, size: 704)
!167 = !DIDerivedType(tag: DW_TAG_member, name: "root", scope: !164, file: !22, line: 196, baseType: !168, size: 64, offset: 704)
!168 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "rb_root", file: !169, line: 12, size: 64, elements: !170)
!169 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/rbtree_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "8417af8a8bff2c6b1a545902a3b285de")
!170 = !{!171}
!171 = !DIDerivedType(tag: DW_TAG_member, name: "rb_node", scope: !168, file: !169, line: 13, baseType: !172, size: 64)
!172 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !173, size: 64)
!173 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "rb_node", file: !169, line: 5, size: 192, align: 64, elements: !174)
!174 = !{!175, !176, !177}
!175 = !DIDerivedType(tag: DW_TAG_member, name: "__rb_parent_color", scope: !173, file: !169, line: 6, baseType: !59, size: 64)
!176 = !DIDerivedType(tag: DW_TAG_member, name: "rb_right", scope: !173, file: !169, line: 7, baseType: !172, size: 64, offset: 64)
!177 = !DIDerivedType(tag: DW_TAG_member, name: "rb_left", scope: !173, file: !169, line: 8, baseType: !172, size: 64, offset: 128)
!178 = !DIDerivedType(tag: DW_TAG_member, name: "lookup", scope: !153, file: !22, line: 206, baseType: !179, size: 64, offset: 832)
!179 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !180, size: 64)
!180 = !DISubroutineType(types: !181)
!181 = !{!162, !152}
!182 = !DIDerivedType(tag: DW_TAG_member, name: "set_ownership", scope: !153, file: !22, line: 207, baseType: !183, size: 64, offset: 896)
!183 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !184, size: 64)
!184 = !DISubroutineType(types: !185)
!185 = !{null, !186, !187, !195}
!186 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64)
!187 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !188, size: 64)
!188 = !DIDerivedType(tag: DW_TAG_typedef, name: "kuid_t", file: !189, line: 9, baseType: !190)
!189 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/uidgid_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "7759f40d631f3b3df184a9e1d5220c1a")
!190 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !189, line: 7, size: 32, elements: !191)
!191 = !{!192}
!192 = !DIDerivedType(tag: DW_TAG_member, name: "val", scope: !190, file: !189, line: 8, baseType: !193, size: 32)
!193 = !DIDerivedType(tag: DW_TAG_typedef, name: "uid_t", file: !45, line: 37, baseType: !194)
!194 = !DIDerivedType(tag: DW_TAG_typedef, name: "__kernel_uid32_t", file: !57, line: 49, baseType: !7)
!195 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !196, size: 64)
!196 = !DIDerivedType(tag: DW_TAG_typedef, name: "kgid_t", file: !189, line: 13, baseType: !197)
!197 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !189, line: 11, size: 32, elements: !198)
!198 = !{!199}
!199 = !DIDerivedType(tag: DW_TAG_member, name: "val", scope: !197, file: !189, line: 12, baseType: !200, size: 32)
!200 = !DIDerivedType(tag: DW_TAG_typedef, name: "gid_t", file: !45, line: 38, baseType: !201)
!201 = !DIDerivedType(tag: DW_TAG_typedef, name: "__kernel_gid32_t", file: !57, line: 50, baseType: !7)
!202 = !DIDerivedType(tag: DW_TAG_member, name: "permissions", scope: !153, file: !22, line: 209, baseType: !203, size: 64, offset: 960)
!203 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !204, size: 64)
!204 = !DISubroutineType(types: !205)
!205 = !{!42, !186, !52}
!206 = !DIDerivedType(tag: DW_TAG_member, name: "set", scope: !23, file: !22, line: 176, baseType: !162, size: 64, offset: 384)
!207 = !DIDerivedType(tag: DW_TAG_member, name: "parent", scope: !23, file: !22, line: 177, baseType: !208, size: 64, offset: 448)
!208 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !164, size: 64)
!209 = !DIDerivedType(tag: DW_TAG_member, name: "node", scope: !23, file: !22, line: 178, baseType: !210, size: 64, offset: 512)
!210 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !211, size: 64)
!211 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "ctl_node", file: !22, line: 146, size: 256, elements: !212)
!212 = !{!213, !214}
!213 = !DIDerivedType(tag: DW_TAG_member, name: "node", scope: !211, file: !22, line: 147, baseType: !173, size: 192, align: 64)
!214 = !DIDerivedType(tag: DW_TAG_member, name: "header", scope: !211, file: !22, line: 148, baseType: !186, size: 64, offset: 192)
!215 = !DIDerivedType(tag: DW_TAG_member, name: "inodes", scope: !23, file: !22, line: 179, baseType: !216, size: 64, offset: 576)
!216 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "hlist_head", file: !45, line: 197, size: 64, elements: !217)
!217 = !{!218}
!218 = !DIDerivedType(tag: DW_TAG_member, name: "first", scope: !216, file: !45, line: 198, baseType: !219, size: 64)
!219 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !220, size: 64)
!220 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "hlist_node", file: !45, line: 201, size: 128, elements: !221)
!221 = !{!222, !223}
!222 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !220, file: !45, line: 202, baseType: !219, size: 64)
!223 = !DIDerivedType(tag: DW_TAG_member, name: "pprev", scope: !220, file: !45, line: 202, baseType: !224, size: 64, offset: 64)
!224 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !219, size: 64)
!225 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !23, file: !22, line: 190, baseType: !21, size: 32, offset: 640)
!226 = !{!227, !228}
!227 = !DIEnumerator(name: "SYSCTL_TABLE_TYPE_DEFAULT", value: 0)
!228 = !DIEnumerator(name: "SYSCTL_TABLE_TYPE_PERMANENTLY_EMPTY", value: 1)
!229 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "fault_flag", file: !230, line: 1383, baseType: !7, size: 32, elements: !231)
!230 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/mm_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "7b9d0a80cdfd8a5e256d971b59fd9913")
!231 = !{!232, !233, !234, !235, !236, !237, !238, !239, !240, !241, !242, !243, !244}
!232 = !DIEnumerator(name: "FAULT_FLAG_WRITE", value: 1)
!233 = !DIEnumerator(name: "FAULT_FLAG_MKWRITE", value: 2)
!234 = !DIEnumerator(name: "FAULT_FLAG_ALLOW_RETRY", value: 4)
!235 = !DIEnumerator(name: "FAULT_FLAG_RETRY_NOWAIT", value: 8)
!236 = !DIEnumerator(name: "FAULT_FLAG_KILLABLE", value: 16)
!237 = !DIEnumerator(name: "FAULT_FLAG_TRIED", value: 32)
!238 = !DIEnumerator(name: "FAULT_FLAG_USER", value: 64)
!239 = !DIEnumerator(name: "FAULT_FLAG_REMOTE", value: 128)
!240 = !DIEnumerator(name: "FAULT_FLAG_INSTRUCTION", value: 256)
!241 = !DIEnumerator(name: "FAULT_FLAG_INTERRUPTIBLE", value: 512)
!242 = !DIEnumerator(name: "FAULT_FLAG_UNSHARE", value: 1024)
!243 = !DIEnumerator(name: "FAULT_FLAG_ORIG_PTE_VALID", value: 2048)
!244 = !DIEnumerator(name: "FAULT_FLAG_VMA_LOCK", value: 4096)
!245 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "writeback_sync_modes", file: !246, line: 33, baseType: !7, size: 32, elements: !247)
!246 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/writeback.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "97ad5230299b5dd6f74701dbc284c459")
!247 = !{!248, !249}
!248 = !DIEnumerator(name: "WB_SYNC_NONE", value: 0)
!249 = !DIEnumerator(name: "WB_SYNC_ALL", value: 1)
!250 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "migrate_mode", file: !251, line: 11, baseType: !7, size: 32, elements: !252)
!251 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/migrate_mode.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "29834bc2b80722df4a96bb473657fda3")
!252 = !{!253, !254, !255}
!253 = !DIEnumerator(name: "MIGRATE_ASYNC", value: 0)
!254 = !DIEnumerator(name: "MIGRATE_SYNC_LIGHT", value: 1)
!255 = !DIEnumerator(name: "MIGRATE_SYNC", value: 2)
!256 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "probe_type", file: !257, line: 45, baseType: !7, size: 32, elements: !258)
!257 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/device/driver.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "b4f11b4701398c6d50b65d5c0e1c7144")
!258 = !{!259, !260, !261}
!259 = !DIEnumerator(name: "PROBE_DEFAULT_STRATEGY", value: 0)
!260 = !DIEnumerator(name: "PROBE_PREFER_ASYNCHRONOUS", value: 1)
!261 = !DIEnumerator(name: "PROBE_FORCE_SYNCHRONOUS", value: 2)
!262 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "dl_dev_state", file: !263, line: 501, baseType: !7, size: 32, elements: !264)
!263 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/device.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "526a67926c02a2a3da15d8072eb47e6d")
!264 = !{!265, !266, !267, !268}
!265 = !DIEnumerator(name: "DL_DEV_NO_DRIVER", value: 0)
!266 = !DIEnumerator(name: "DL_DEV_PROBING", value: 1)
!267 = !DIEnumerator(name: "DL_DEV_DRIVER_BOUND", value: 2)
!268 = !DIEnumerator(name: "DL_DEV_UNBINDING", value: 3)
!269 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "hrtimer_restart", file: !270, line: 13, baseType: !7, size: 32, elements: !271)
!270 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/hrtimer_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "93b54ff0303805916f6b4fb65b3822c5")
!271 = !{!272, !273}
!272 = !DIEnumerator(name: "HRTIMER_NORESTART", value: 0)
!273 = !DIEnumerator(name: "HRTIMER_RESTART", value: 1)
!274 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "rpm_request", file: !275, line: 620, baseType: !7, size: 32, elements: !276)
!275 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/pm.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "8f2087326f593b2018529261bcd3707c")
!276 = !{!277, !278, !279, !280, !281}
!277 = !DIEnumerator(name: "RPM_REQ_NONE", value: 0)
!278 = !DIEnumerator(name: "RPM_REQ_IDLE", value: 1)
!279 = !DIEnumerator(name: "RPM_REQ_SUSPEND", value: 2)
!280 = !DIEnumerator(name: "RPM_REQ_AUTOSUSPEND", value: 3)
!281 = !DIEnumerator(name: "RPM_REQ_RESUME", value: 4)
!282 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "rpm_status", file: !275, line: 597, baseType: !42, size: 32, elements: !283)
!283 = !{!284, !285, !286, !287, !288}
!284 = !DIEnumerator(name: "RPM_INVALID", value: -1)
!285 = !DIEnumerator(name: "RPM_ACTIVE", value: 0)
!286 = !DIEnumerator(name: "RPM_RESUMING", value: 1)
!287 = !DIEnumerator(name: "RPM_SUSPENDED", value: 2)
!288 = !DIEnumerator(name: "RPM_SUSPENDING", value: 3)
!289 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "kobj_ns_type", file: !290, line: 26, baseType: !7, size: 32, elements: !291)
!290 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/kobject_ns.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "117a42fea53455e8a509424988d0a1f9")
!291 = !{!292, !293, !294}
!292 = !DIEnumerator(name: "KOBJ_NS_TYPE_NONE", value: 0)
!293 = !DIEnumerator(name: "KOBJ_NS_TYPE_NET", value: 1)
!294 = !DIEnumerator(name: "KOBJ_NS_TYPES", value: 2)
!295 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "device_physical_location_panel", file: !263, line: 561, baseType: !7, size: 32, elements: !296)
!296 = !{!297, !298, !299, !300, !301, !302, !303}
!297 = !DIEnumerator(name: "DEVICE_PANEL_TOP", value: 0)
!298 = !DIEnumerator(name: "DEVICE_PANEL_BOTTOM", value: 1)
!299 = !DIEnumerator(name: "DEVICE_PANEL_LEFT", value: 2)
!300 = !DIEnumerator(name: "DEVICE_PANEL_RIGHT", value: 3)
!301 = !DIEnumerator(name: "DEVICE_PANEL_FRONT", value: 4)
!302 = !DIEnumerator(name: "DEVICE_PANEL_BACK", value: 5)
!303 = !DIEnumerator(name: "DEVICE_PANEL_UNKNOWN", value: 6)
!304 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "device_physical_location_vertical_position", file: !263, line: 578, baseType: !7, size: 32, elements: !305)
!305 = !{!306, !307, !308}
!306 = !DIEnumerator(name: "DEVICE_VERT_POS_UPPER", value: 0)
!307 = !DIEnumerator(name: "DEVICE_VERT_POS_CENTER", value: 1)
!308 = !DIEnumerator(name: "DEVICE_VERT_POS_LOWER", value: 2)
!309 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "device_physical_location_horizontal_position", file: !263, line: 591, baseType: !7, size: 32, elements: !310)
!310 = !{!311, !312, !313}
!311 = !DIEnumerator(name: "DEVICE_HORI_POS_LEFT", value: 0)
!312 = !DIEnumerator(name: "DEVICE_HORI_POS_CENTER", value: 1)
!313 = !DIEnumerator(name: "DEVICE_HORI_POS_RIGHT", value: 2)
!314 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "device_removable", file: !263, line: 517, baseType: !7, size: 32, elements: !315)
!315 = !{!316, !317, !318, !319}
!316 = !DIEnumerator(name: "DEVICE_REMOVABLE_NOT_SUPPORTED", value: 0)
!317 = !DIEnumerator(name: "DEVICE_REMOVABLE_UNKNOWN", value: 1)
!318 = !DIEnumerator(name: "DEVICE_FIXED", value: 2)
!319 = !DIEnumerator(name: "DEVICE_REMOVABLE", value: 3)
!320 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "timespec_type", file: !321, line: 16, baseType: !7, size: 32, elements: !322)
!321 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/restart_block.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "7da53523d9557b9884373e8e62b182e8")
!322 = !{!323, !324, !325}
!323 = !DIEnumerator(name: "TT_NONE", value: 0)
!324 = !DIEnumerator(name: "TT_NATIVE", value: 1)
!325 = !DIEnumerator(name: "TT_COMPAT", value: 2)
!326 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "uprobe_task_state", file: !327, line: 52, baseType: !7, size: 32, elements: !328)
!327 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/uprobes.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "26b4a9327034010695484df6a93b0756")
!328 = !{!329, !330, !331, !332}
!329 = !DIEnumerator(name: "UTASK_RUNNING", value: 0)
!330 = !DIEnumerator(name: "UTASK_SSTEP", value: 1)
!331 = !DIEnumerator(name: "UTASK_SSTEP_ACK", value: 2)
!332 = !DIEnumerator(name: "UTASK_SSTEP_TRAPPED", value: 3)
!333 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "pid_type", file: !334, line: 5, baseType: !7, size: 32, elements: !335)
!334 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/pid_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "ea87a36feda64bf34ee115d02767702b")
!335 = !{!336, !337, !338, !339, !340}
!336 = !DIEnumerator(name: "PIDTYPE_PID", value: 0)
!337 = !DIEnumerator(name: "PIDTYPE_TGID", value: 1)
!338 = !DIEnumerator(name: "PIDTYPE_PGID", value: 2)
!339 = !DIEnumerator(name: "PIDTYPE_SID", value: 3)
!340 = !DIEnumerator(name: "PIDTYPE_MAX", value: 4)
!341 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "freeze_holder", file: !342, line: 2212, baseType: !7, size: 32, elements: !343)
!342 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/fs.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "477f21e18d7314c9371fc832052b11ee")
!343 = !{!344, !345, !346}
!344 = !DIEnumerator(name: "FREEZE_HOLDER_KERNEL", value: 1)
!345 = !DIEnumerator(name: "FREEZE_HOLDER_USERSPACE", value: 2)
!346 = !DIEnumerator(name: "FREEZE_MAY_NEST", value: 4)
!347 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "quota_type", file: !348, line: 54, baseType: !7, size: 32, elements: !349)
!348 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/quota.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "d2fa056191239716e95cbdb57989d0a1")
!349 = !{!350, !351, !352}
!350 = !DIEnumerator(name: "USRQUOTA", value: 0)
!351 = !DIEnumerator(name: "GRPQUOTA", value: 1)
!352 = !DIEnumerator(name: "PRJQUOTA", value: 2)
!353 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "wb_reason", file: !354, line: 44, baseType: !7, size: 32, elements: !355)
!354 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/backing-dev-defs.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "48876dd2050c245a6230a2e03c4bc3c9")
!355 = !{!356, !357, !358, !359, !360, !361, !362, !363, !364}
!356 = !DIEnumerator(name: "WB_REASON_BACKGROUND", value: 0)
!357 = !DIEnumerator(name: "WB_REASON_VMSCAN", value: 1)
!358 = !DIEnumerator(name: "WB_REASON_SYNC", value: 2)
!359 = !DIEnumerator(name: "WB_REASON_PERIODIC", value: 3)
!360 = !DIEnumerator(name: "WB_REASON_LAPTOP_TIMER", value: 4)
!361 = !DIEnumerator(name: "WB_REASON_FS_FREE_SPACE", value: 5)
!362 = !DIEnumerator(name: "WB_REASON_FORKER_THREAD", value: 6)
!363 = !DIEnumerator(name: "WB_REASON_FOREIGN_FLUSH", value: 7)
!364 = !DIEnumerator(name: "WB_REASON_MAX", value: 8)
!365 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "d_real_type", file: !366, line: 133, baseType: !7, size: 32, elements: !367)
!366 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/dcache.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "f440fdc55dffe4f1fd8a354a6bf31663")
!367 = !{!368, !369}
!368 = !DIEnumerator(name: "D_REAL_DATA", value: 0)
!369 = !DIEnumerator(name: "D_REAL_METADATA", value: 1)
!370 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "rw_hint", file: !371, line: 10, baseType: !107, size: 8, elements: !372)
!371 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/rw_hint.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "c8e683ef38a0d84184f04158b1e012f8")
!372 = !{!373, !374, !375, !376, !377, !378}
!373 = !DIEnumerator(name: "WRITE_LIFE_NOT_SET", value: 0)
!374 = !DIEnumerator(name: "WRITE_LIFE_NONE", value: 1)
!375 = !DIEnumerator(name: "WRITE_LIFE_SHORT", value: 2)
!376 = !DIEnumerator(name: "WRITE_LIFE_MEDIUM", value: 3)
!377 = !DIEnumerator(name: "WRITE_LIFE_LONG", value: 4)
!378 = !DIEnumerator(name: "WRITE_LIFE_EXTREME", value: 5)
!379 = !DICompositeType(tag: DW_TAG_enumeration_type, file: !380, line: 26, baseType: !7, size: 32, elements: !381)
!380 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/gfp_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "87ecfd87fd9d9f98a2bb39d59ed67080")
!381 = !{!382, !383, !384, !385, !386, !387, !388, !389, !390, !391, !392, !393, !394, !395, !396, !397, !398, !399, !400, !401, !402, !403, !404, !405, !406}
!382 = !DIEnumerator(name: "___GFP_DMA_BIT", value: 0)
!383 = !DIEnumerator(name: "___GFP_HIGHMEM_BIT", value: 1)
!384 = !DIEnumerator(name: "___GFP_DMA32_BIT", value: 2)
!385 = !DIEnumerator(name: "___GFP_MOVABLE_BIT", value: 3)
!386 = !DIEnumerator(name: "___GFP_RECLAIMABLE_BIT", value: 4)
!387 = !DIEnumerator(name: "___GFP_HIGH_BIT", value: 5)
!388 = !DIEnumerator(name: "___GFP_IO_BIT", value: 6)
!389 = !DIEnumerator(name: "___GFP_FS_BIT", value: 7)
!390 = !DIEnumerator(name: "___GFP_ZERO_BIT", value: 8)
!391 = !DIEnumerator(name: "___GFP_UNUSED_BIT", value: 9)
!392 = !DIEnumerator(name: "___GFP_DIRECT_RECLAIM_BIT", value: 10)
!393 = !DIEnumerator(name: "___GFP_KSWAPD_RECLAIM_BIT", value: 11)
!394 = !DIEnumerator(name: "___GFP_WRITE_BIT", value: 12)
!395 = !DIEnumerator(name: "___GFP_NOWARN_BIT", value: 13)
!396 = !DIEnumerator(name: "___GFP_RETRY_MAYFAIL_BIT", value: 14)
!397 = !DIEnumerator(name: "___GFP_NOFAIL_BIT", value: 15)
!398 = !DIEnumerator(name: "___GFP_NORETRY_BIT", value: 16)
!399 = !DIEnumerator(name: "___GFP_MEMALLOC_BIT", value: 17)
!400 = !DIEnumerator(name: "___GFP_COMP_BIT", value: 18)
!401 = !DIEnumerator(name: "___GFP_NOMEMALLOC_BIT", value: 19)
!402 = !DIEnumerator(name: "___GFP_HARDWALL_BIT", value: 20)
!403 = !DIEnumerator(name: "___GFP_THISNODE_BIT", value: 21)
!404 = !DIEnumerator(name: "___GFP_ACCOUNT_BIT", value: 22)
!405 = !DIEnumerator(name: "___GFP_ZEROTAGS_BIT", value: 23)
!406 = !DIEnumerator(name: "___GFP_LAST_BIT", value: 24)
!407 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "task_work_notify_mode", file: !408, line: 16, baseType: !7, size: 32, elements: !409)
!408 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/task_work.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "14ec5de0e46a618de4c66497a3be70e8")
!409 = !{!410, !411, !412, !413, !414}
!410 = !DIEnumerator(name: "TWA_NONE", value: 0)
!411 = !DIEnumerator(name: "TWA_RESUME", value: 1)
!412 = !DIEnumerator(name: "TWA_SIGNAL", value: 2)
!413 = !DIEnumerator(name: "TWA_SIGNAL_NO_IPI", value: 3)
!414 = !DIEnumerator(name: "TWA_NMI_CURRENT", value: 4)
!415 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "_slab_flag_bits", file: !416, line: 24, baseType: !7, size: 32, elements: !417)
!416 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/slab.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "38a3c98be731fb3e3a95e3559f05c16e")
!417 = !{!418, !419, !420, !421, !422, !423, !424, !425, !426, !427, !428, !429, !430, !431, !432, !433, !434, !435}
!418 = !DIEnumerator(name: "_SLAB_CONSISTENCY_CHECKS", value: 0)
!419 = !DIEnumerator(name: "_SLAB_RED_ZONE", value: 1)
!420 = !DIEnumerator(name: "_SLAB_POISON", value: 2)
!421 = !DIEnumerator(name: "_SLAB_KMALLOC", value: 3)
!422 = !DIEnumerator(name: "_SLAB_HWCACHE_ALIGN", value: 4)
!423 = !DIEnumerator(name: "_SLAB_CACHE_DMA", value: 5)
!424 = !DIEnumerator(name: "_SLAB_CACHE_DMA32", value: 6)
!425 = !DIEnumerator(name: "_SLAB_STORE_USER", value: 7)
!426 = !DIEnumerator(name: "_SLAB_PANIC", value: 8)
!427 = !DIEnumerator(name: "_SLAB_TYPESAFE_BY_RCU", value: 9)
!428 = !DIEnumerator(name: "_SLAB_TRACE", value: 10)
!429 = !DIEnumerator(name: "_SLAB_NOLEAKTRACE", value: 11)
!430 = !DIEnumerator(name: "_SLAB_NO_MERGE", value: 12)
!431 = !DIEnumerator(name: "_SLAB_NO_USER_FLAGS", value: 13)
!432 = !DIEnumerator(name: "_SLAB_RECLAIM_ACCOUNT", value: 14)
!433 = !DIEnumerator(name: "_SLAB_OBJECT_POISON", value: 15)
!434 = !DIEnumerator(name: "_SLAB_CMPXCHG_DOUBLE", value: 16)
!435 = !DIEnumerator(name: "_SLAB_FLAGS_LAST_BIT", value: 17)
!436 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "zone_stat_item", file: !437, line: 138, baseType: !7, size: 32, elements: !438)
!437 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/mmzone.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "61f60ac2b3e6d6d21341727b918187f9")
!438 = !{!439, !440, !441, !442, !443, !444, !445, !446, !447, !448, !449, !450}
!439 = !DIEnumerator(name: "NR_FREE_PAGES", value: 0)
!440 = !DIEnumerator(name: "NR_ZONE_LRU_BASE", value: 1)
!441 = !DIEnumerator(name: "NR_ZONE_INACTIVE_ANON", value: 1)
!442 = !DIEnumerator(name: "NR_ZONE_ACTIVE_ANON", value: 2)
!443 = !DIEnumerator(name: "NR_ZONE_INACTIVE_FILE", value: 3)
!444 = !DIEnumerator(name: "NR_ZONE_ACTIVE_FILE", value: 4)
!445 = !DIEnumerator(name: "NR_ZONE_UNEVICTABLE", value: 5)
!446 = !DIEnumerator(name: "NR_ZONE_WRITE_PENDING", value: 6)
!447 = !DIEnumerator(name: "NR_MLOCK", value: 7)
!448 = !DIEnumerator(name: "NR_BOUNCE", value: 8)
!449 = !DIEnumerator(name: "NR_FREE_CMA_PAGES", value: 9)
!450 = !DIEnumerator(name: "NR_VM_ZONE_STAT_ITEMS", value: 10)
!451 = !DICompositeType(tag: DW_TAG_enumeration_type, file: !452, line: 10, baseType: !7, size: 32, elements: !453)
!452 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/stddef.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "f808bea20fbf9b23fd364e1890694b49")
!453 = !{!454, !455}
!454 = !DIEnumerator(name: "false", value: 0)
!455 = !DIEnumerator(name: "true", value: 1)
!456 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "kmalloc_cache_type", file: !416, line: 573, baseType: !7, size: 32, elements: !457)
!457 = !{!458, !459, !460, !461, !462, !463, !464}
!458 = !DIEnumerator(name: "KMALLOC_NORMAL", value: 0)
!459 = !DIEnumerator(name: "KMALLOC_CGROUP", value: 0)
!460 = !DIEnumerator(name: "KMALLOC_RANDOM_START", value: 0)
!461 = !DIEnumerator(name: "KMALLOC_RANDOM_END", value: 0)
!462 = !DIEnumerator(name: "KMALLOC_RECLAIM", value: 1)
!463 = !DIEnumerator(name: "KMALLOC_DMA", value: 2)
!464 = !DIEnumerator(name: "NR_KMALLOC_TYPES", value: 3)
!465 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "wq_misc_consts", file: !466, line: 87, baseType: !7, size: 32, elements: !467)
!466 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/workqueue.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "3f1a44256163fa7099158d834ca4e4e7")
!467 = !{!468, !469, !470, !471, !472}
!468 = !DIEnumerator(name: "WORK_NR_COLORS", value: 16)
!469 = !DIEnumerator(name: "WORK_CPU_UNBOUND", value: 64)
!470 = !DIEnumerator(name: "WORK_BUSY_PENDING", value: 1)
!471 = !DIEnumerator(name: "WORK_BUSY_RUNNING", value: 2)
!472 = !DIEnumerator(name: "WORKER_DESC_LEN", value: 32)
!473 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "fsnotify_group_prio", file: !474, line: 183, baseType: !7, size: 32, elements: !475)
!474 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/fsnotify_backend.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "1d051fe472373476cf3b37b9491ac81b")
!475 = !{!476, !477, !478, !479}
!476 = !DIEnumerator(name: "FSNOTIFY_PRIO_NORMAL", value: 0)
!477 = !DIEnumerator(name: "FSNOTIFY_PRIO_CONTENT", value: 1)
!478 = !DIEnumerator(name: "FSNOTIFY_PRIO_PRE_CONTENT", value: 2)
!479 = !DIEnumerator(name: "__FSNOTIFY_PRIO_NUM", value: 3)
!480 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "fsnotify_data_type", file: !474, line: 290, baseType: !7, size: 32, elements: !481)
!481 = !{!482, !483, !484, !485, !486}
!482 = !DIEnumerator(name: "FSNOTIFY_EVENT_NONE", value: 0)
!483 = !DIEnumerator(name: "FSNOTIFY_EVENT_PATH", value: 1)
!484 = !DIEnumerator(name: "FSNOTIFY_EVENT_INODE", value: 2)
!485 = !DIEnumerator(name: "FSNOTIFY_EVENT_DENTRY", value: 3)
!486 = !DIEnumerator(name: "FSNOTIFY_EVENT_ERROR", value: 4)
!487 = !{!488, !489, !490, !5628, !40, !5629, !42, !5635, !5638, !5639, !59, !5641, !5643, !5644, !896, !2694, !5646}
!488 = !DIDerivedType(tag: DW_TAG_typedef, name: "gfp_t", file: !45, line: 157, baseType: !7)
!489 = !DIDerivedType(tag: DW_TAG_typedef, name: "fmode_t", file: !45, line: 159, baseType: !7)
!490 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !491, size: 64)
!491 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !492)
!492 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cred", file: !493, line: 111, size: 1472, elements: !494)
!493 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/cred.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "b5b24b213aad5dd2e60dd75f3ebe789e")
!494 = !{!495, !504, !505, !506, !507, !508, !509, !510, !511, !512, !513, !522, !523, !524, !525, !526, !527, !664, !665, !666, !667, !668, !699, !5614, !5615, !5623}
!495 = !DIDerivedType(tag: DW_TAG_member, name: "usage", scope: !492, file: !493, line: 112, baseType: !496, size: 64)
!496 = !DIDerivedType(tag: DW_TAG_typedef, name: "atomic_long_t", file: !497, line: 13, baseType: !498)
!497 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/atomic/atomic-long.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "c644fb2fca4959b7144a3c6defc79364")
!498 = !DIDerivedType(tag: DW_TAG_typedef, name: "atomic64_t", file: !45, line: 184, baseType: !499)
!499 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !45, line: 182, size: 64, elements: !500)
!500 = !{!501}
!501 = !DIDerivedType(tag: DW_TAG_member, name: "counter", scope: !499, file: !45, line: 183, baseType: !502, size: 64)
!502 = !DIDerivedType(tag: DW_TAG_typedef, name: "s64", file: !104, line: 22, baseType: !503)
!503 = !DIDerivedType(tag: DW_TAG_typedef, name: "__s64", file: !106, line: 30, baseType: !63)
!504 = !DIDerivedType(tag: DW_TAG_member, name: "uid", scope: !492, file: !493, line: 113, baseType: !188, size: 32, offset: 64)
!505 = !DIDerivedType(tag: DW_TAG_member, name: "gid", scope: !492, file: !493, line: 114, baseType: !196, size: 32, offset: 96)
!506 = !DIDerivedType(tag: DW_TAG_member, name: "suid", scope: !492, file: !493, line: 115, baseType: !188, size: 32, offset: 128)
!507 = !DIDerivedType(tag: DW_TAG_member, name: "sgid", scope: !492, file: !493, line: 116, baseType: !196, size: 32, offset: 160)
!508 = !DIDerivedType(tag: DW_TAG_member, name: "euid", scope: !492, file: !493, line: 117, baseType: !188, size: 32, offset: 192)
!509 = !DIDerivedType(tag: DW_TAG_member, name: "egid", scope: !492, file: !493, line: 118, baseType: !196, size: 32, offset: 224)
!510 = !DIDerivedType(tag: DW_TAG_member, name: "fsuid", scope: !492, file: !493, line: 119, baseType: !188, size: 32, offset: 256)
!511 = !DIDerivedType(tag: DW_TAG_member, name: "fsgid", scope: !492, file: !493, line: 120, baseType: !196, size: 32, offset: 288)
!512 = !DIDerivedType(tag: DW_TAG_member, name: "securebits", scope: !492, file: !493, line: 121, baseType: !7, size: 32, offset: 320)
!513 = !DIDerivedType(tag: DW_TAG_member, name: "cap_inheritable", scope: !492, file: !493, line: 122, baseType: !514, size: 64, offset: 384)
!514 = !DIDerivedType(tag: DW_TAG_typedef, name: "kernel_cap_t", file: !515, line: 24, baseType: !516)
!515 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/capability.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "844d16bbc1c5d5ddd3d4dd45f55bc1eb")
!516 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !515, line: 24, size: 64, elements: !517)
!517 = !{!518}
!518 = !DIDerivedType(tag: DW_TAG_member, name: "val", scope: !516, file: !515, line: 24, baseType: !519, size: 64)
!519 = !DIDerivedType(tag: DW_TAG_typedef, name: "u64", file: !104, line: 23, baseType: !520)
!520 = !DIDerivedType(tag: DW_TAG_typedef, name: "__u64", file: !106, line: 31, baseType: !521)
!521 = !DIBasicType(name: "unsigned long long", size: 64, encoding: DW_ATE_unsigned)
!522 = !DIDerivedType(tag: DW_TAG_member, name: "cap_permitted", scope: !492, file: !493, line: 123, baseType: !514, size: 64, offset: 448)
!523 = !DIDerivedType(tag: DW_TAG_member, name: "cap_effective", scope: !492, file: !493, line: 124, baseType: !514, size: 64, offset: 512)
!524 = !DIDerivedType(tag: DW_TAG_member, name: "cap_bset", scope: !492, file: !493, line: 125, baseType: !514, size: 64, offset: 576)
!525 = !DIDerivedType(tag: DW_TAG_member, name: "cap_ambient", scope: !492, file: !493, line: 126, baseType: !514, size: 64, offset: 640)
!526 = !DIDerivedType(tag: DW_TAG_member, name: "jit_keyring", scope: !492, file: !493, line: 128, baseType: !107, size: 8, offset: 704)
!527 = !DIDerivedType(tag: DW_TAG_member, name: "session_keyring", scope: !492, file: !493, line: 130, baseType: !528, size: 64, offset: 768)
!528 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !529, size: 64)
!529 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "key", file: !530, line: 195, size: 1728, elements: !531)
!530 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/key.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "d20384ea9f837c881ec6a8e61cc61496")
!531 = !{!532, !538, !543, !548, !561, !564, !565, !572, !573, !574, !575, !580, !581, !582, !584, !585, !626, !649}
!532 = !DIDerivedType(tag: DW_TAG_member, name: "usage", scope: !529, file: !530, line: 196, baseType: !533, size: 32)
!533 = !DIDerivedType(tag: DW_TAG_typedef, name: "refcount_t", file: !534, line: 17, baseType: !535)
!534 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/refcount_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "36ffe3ced1358e99348a85b7f35e3fa8")
!535 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "refcount_struct", file: !534, line: 15, size: 32, elements: !536)
!536 = !{!537}
!537 = !DIDerivedType(tag: DW_TAG_member, name: "refs", scope: !535, file: !534, line: 16, baseType: !69, size: 32)
!538 = !DIDerivedType(tag: DW_TAG_member, name: "serial", scope: !529, file: !530, line: 197, baseType: !539, size: 32, offset: 32)
!539 = !DIDerivedType(tag: DW_TAG_typedef, name: "key_serial_t", file: !530, line: 28, baseType: !540)
!540 = !DIDerivedType(tag: DW_TAG_typedef, name: "int32_t", file: !45, line: 104, baseType: !541)
!541 = !DIDerivedType(tag: DW_TAG_typedef, name: "s32", file: !104, line: 20, baseType: !542)
!542 = !DIDerivedType(tag: DW_TAG_typedef, name: "__s32", file: !106, line: 26, baseType: !42)
!543 = !DIDerivedType(tag: DW_TAG_member, scope: !529, file: !530, line: 198, baseType: !544, size: 192, offset: 64)
!544 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !529, file: !530, line: 198, size: 192, elements: !545)
!545 = !{!546, !547}
!546 = !DIDerivedType(tag: DW_TAG_member, name: "graveyard_link", scope: !544, file: !530, line: 199, baseType: !117, size: 128)
!547 = !DIDerivedType(tag: DW_TAG_member, name: "serial_node", scope: !544, file: !530, line: 200, baseType: !173, size: 192, align: 64)
!548 = !DIDerivedType(tag: DW_TAG_member, name: "sem", scope: !529, file: !530, line: 205, baseType: !549, size: 320, offset: 256)
!549 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "rw_semaphore", file: !550, line: 48, size: 320, elements: !551)
!550 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/rwsem.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "6ff9031508c897ed713a6a65db7fb4b2")
!551 = !{!552, !553, !554, !559, !560}
!552 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !549, file: !550, line: 49, baseType: !496, size: 64)
!553 = !DIDerivedType(tag: DW_TAG_member, name: "owner", scope: !549, file: !550, line: 55, baseType: !496, size: 64, offset: 64)
!554 = !DIDerivedType(tag: DW_TAG_member, name: "osq", scope: !549, file: !550, line: 57, baseType: !555, size: 32, offset: 128)
!555 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "optimistic_spin_queue", file: !556, line: 10, size: 32, elements: !557)
!556 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/osq_lock.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "1b38ac17e459d3352fc4003ba7b067e6")
!557 = !{!558}
!558 = !DIDerivedType(tag: DW_TAG_member, name: "tail", scope: !555, file: !556, line: 15, baseType: !69, size: 32)
!559 = !DIDerivedType(tag: DW_TAG_member, name: "wait_lock", scope: !549, file: !550, line: 59, baseType: !148, size: 32, offset: 160)
!560 = !DIDerivedType(tag: DW_TAG_member, name: "wait_list", scope: !549, file: !550, line: 60, baseType: !117, size: 128, offset: 192)
!561 = !DIDerivedType(tag: DW_TAG_member, name: "user", scope: !529, file: !530, line: 206, baseType: !562, size: 64, offset: 576)
!562 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !563, size: 64)
!563 = !DICompositeType(tag: DW_TAG_structure_type, name: "key_user", file: !530, line: 206, flags: DIFlagFwdDecl)
!564 = !DIDerivedType(tag: DW_TAG_member, name: "security", scope: !529, file: !530, line: 207, baseType: !40, size: 64, offset: 640)
!565 = !DIDerivedType(tag: DW_TAG_member, scope: !529, file: !530, line: 208, baseType: !566, size: 64, offset: 704)
!566 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !529, file: !530, line: 208, size: 64, elements: !567)
!567 = !{!568, !571}
!568 = !DIDerivedType(tag: DW_TAG_member, name: "expiry", scope: !566, file: !530, line: 209, baseType: !569, size: 64)
!569 = !DIDerivedType(tag: DW_TAG_typedef, name: "time64_t", file: !570, line: 8, baseType: !503)
!570 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/time64.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "d597827474cd37ab3c64b0e07d237cd6")
!571 = !DIDerivedType(tag: DW_TAG_member, name: "revoked_at", scope: !566, file: !530, line: 210, baseType: !569, size: 64)
!572 = !DIDerivedType(tag: DW_TAG_member, name: "last_used_at", scope: !529, file: !530, line: 212, baseType: !569, size: 64, offset: 768)
!573 = !DIDerivedType(tag: DW_TAG_member, name: "uid", scope: !529, file: !530, line: 213, baseType: !188, size: 32, offset: 832)
!574 = !DIDerivedType(tag: DW_TAG_member, name: "gid", scope: !529, file: !530, line: 214, baseType: !196, size: 32, offset: 864)
!575 = !DIDerivedType(tag: DW_TAG_member, name: "perm", scope: !529, file: !530, line: 215, baseType: !576, size: 32, offset: 896)
!576 = !DIDerivedType(tag: DW_TAG_typedef, name: "key_perm_t", file: !530, line: 31, baseType: !577)
!577 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint32_t", file: !45, line: 110, baseType: !578)
!578 = !DIDerivedType(tag: DW_TAG_typedef, name: "u32", file: !104, line: 21, baseType: !579)
!579 = !DIDerivedType(tag: DW_TAG_typedef, name: "__u32", file: !106, line: 27, baseType: !7)
!580 = !DIDerivedType(tag: DW_TAG_member, name: "quotalen", scope: !529, file: !530, line: 216, baseType: !46, size: 16, offset: 928)
!581 = !DIDerivedType(tag: DW_TAG_member, name: "datalen", scope: !529, file: !530, line: 217, baseType: !46, size: 16, offset: 944)
!582 = !DIDerivedType(tag: DW_TAG_member, name: "state", scope: !529, file: !530, line: 221, baseType: !583, size: 16, offset: 960)
!583 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!584 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !529, file: !530, line: 228, baseType: !59, size: 64, offset: 1024)
!585 = !DIDerivedType(tag: DW_TAG_member, scope: !529, file: !530, line: 245, baseType: !586, size: 320, offset: 1088)
!586 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !529, file: !530, line: 245, size: 320, elements: !587)
!587 = !{!588, !617}
!588 = !DIDerivedType(tag: DW_TAG_member, name: "index_key", scope: !586, file: !530, line: 246, baseType: !589, size: 320)
!589 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "keyring_index_key", file: !530, line: 114, size: 320, elements: !590)
!590 = !{!591, !592, !604, !607, !616}
!591 = !DIDerivedType(tag: DW_TAG_member, name: "hash", scope: !589, file: !530, line: 116, baseType: !59, size: 64)
!592 = !DIDerivedType(tag: DW_TAG_member, scope: !589, file: !530, line: 117, baseType: !593, size: 64, offset: 64)
!593 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !589, file: !530, line: 117, size: 64, elements: !594)
!594 = !{!595, !603}
!595 = !DIDerivedType(tag: DW_TAG_member, scope: !593, file: !530, line: 118, baseType: !596, size: 64)
!596 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !593, file: !530, line: 118, size: 64, elements: !597)
!597 = !{!598, !599}
!598 = !DIDerivedType(tag: DW_TAG_member, name: "desc_len", scope: !596, file: !530, line: 120, baseType: !113, size: 16)
!599 = !DIDerivedType(tag: DW_TAG_member, name: "desc", scope: !596, file: !530, line: 121, baseType: !600, size: 48, offset: 16)
!600 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 48, elements: !601)
!601 = !{!602}
!602 = !DISubrange(count: 6)
!603 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !593, file: !530, line: 127, baseType: !59, size: 64)
!604 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !589, file: !530, line: 129, baseType: !605, size: 64, offset: 128)
!605 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !606, size: 64)
!606 = !DICompositeType(tag: DW_TAG_structure_type, name: "key_type", file: !530, line: 102, flags: DIFlagFwdDecl)
!607 = !DIDerivedType(tag: DW_TAG_member, name: "domain_tag", scope: !589, file: !530, line: 130, baseType: !608, size: 64, offset: 192)
!608 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !609, size: 64)
!609 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "key_tag", file: !530, line: 108, size: 192, elements: !610)
!610 = !{!611, !612, !613}
!611 = !DIDerivedType(tag: DW_TAG_member, name: "rcu", scope: !609, file: !530, line: 109, baseType: !129, size: 128, align: 64)
!612 = !DIDerivedType(tag: DW_TAG_member, name: "usage", scope: !609, file: !530, line: 110, baseType: !533, size: 32, offset: 128)
!613 = !DIDerivedType(tag: DW_TAG_member, name: "removed", scope: !609, file: !530, line: 111, baseType: !614, size: 8, offset: 160)
!614 = !DIDerivedType(tag: DW_TAG_typedef, name: "bool", file: !45, line: 35, baseType: !615)
!615 = !DIBasicType(name: "_Bool", size: 8, encoding: DW_ATE_boolean)
!616 = !DIDerivedType(tag: DW_TAG_member, name: "description", scope: !589, file: !530, line: 131, baseType: !36, size: 64, offset: 256)
!617 = !DIDerivedType(tag: DW_TAG_member, scope: !586, file: !530, line: 247, baseType: !618, size: 320)
!618 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !586, file: !530, line: 247, size: 320, elements: !619)
!619 = !{!620, !621, !622, !623, !624}
!620 = !DIDerivedType(tag: DW_TAG_member, name: "hash", scope: !618, file: !530, line: 248, baseType: !59, size: 64)
!621 = !DIDerivedType(tag: DW_TAG_member, name: "len_desc", scope: !618, file: !530, line: 249, baseType: !59, size: 64, offset: 64)
!622 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !618, file: !530, line: 250, baseType: !605, size: 64, offset: 128)
!623 = !DIDerivedType(tag: DW_TAG_member, name: "domain_tag", scope: !618, file: !530, line: 251, baseType: !608, size: 64, offset: 192)
!624 = !DIDerivedType(tag: DW_TAG_member, name: "description", scope: !618, file: !530, line: 252, baseType: !625, size: 64, offset: 256)
!625 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !38, size: 64)
!626 = !DIDerivedType(tag: DW_TAG_member, scope: !529, file: !530, line: 260, baseType: !627, size: 256, offset: 1408)
!627 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !529, file: !530, line: 260, size: 256, elements: !628)
!628 = !{!629, !637}
!629 = !DIDerivedType(tag: DW_TAG_member, name: "payload", scope: !627, file: !530, line: 261, baseType: !630, size: 256)
!630 = distinct !DICompositeType(tag: DW_TAG_union_type, name: "key_payload", file: !530, line: 134, size: 256, elements: !631)
!631 = !{!632, !633}
!632 = !DIDerivedType(tag: DW_TAG_member, name: "rcu_data0", scope: !630, file: !530, line: 135, baseType: !40, size: 64)
!633 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !630, file: !530, line: 136, baseType: !634, size: 256)
!634 = !DICompositeType(tag: DW_TAG_array_type, baseType: !40, size: 256, elements: !635)
!635 = !{!636}
!636 = !DISubrange(count: 4)
!637 = !DIDerivedType(tag: DW_TAG_member, scope: !627, file: !530, line: 262, baseType: !638, size: 256)
!638 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !627, file: !530, line: 262, size: 256, elements: !639)
!639 = !{!640, !641}
!640 = !DIDerivedType(tag: DW_TAG_member, name: "name_link", scope: !638, file: !530, line: 264, baseType: !117, size: 128)
!641 = !DIDerivedType(tag: DW_TAG_member, name: "keys", scope: !638, file: !530, line: 265, baseType: !642, size: 128, offset: 128)
!642 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "assoc_array", file: !643, line: 22, size: 128, elements: !644)
!643 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/assoc_array.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "12a70c11037debe270cf6b506589459e")
!644 = !{!645, !648}
!645 = !DIDerivedType(tag: DW_TAG_member, name: "root", scope: !642, file: !643, line: 23, baseType: !646, size: 64)
!646 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !647, size: 64)
!647 = !DICompositeType(tag: DW_TAG_structure_type, name: "assoc_array_ptr", file: !643, line: 23, flags: DIFlagFwdDecl)
!648 = !DIDerivedType(tag: DW_TAG_member, name: "nr_leaves_on_tree", scope: !642, file: !643, line: 24, baseType: !59, size: 64, offset: 64)
!649 = !DIDerivedType(tag: DW_TAG_member, name: "restrict_link", scope: !529, file: !530, line: 280, baseType: !650, size: 64, offset: 1664)
!650 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !651, size: 64)
!651 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "key_restriction", file: !530, line: 176, size: 192, elements: !652)
!652 = !{!653, !662, !663}
!653 = !DIDerivedType(tag: DW_TAG_member, name: "check", scope: !651, file: !530, line: 177, baseType: !654, size: 64)
!654 = !DIDerivedType(tag: DW_TAG_typedef, name: "key_restrict_link_func_t", file: !530, line: 171, baseType: !655)
!655 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !656, size: 64)
!656 = !DISubroutineType(types: !657)
!657 = !{!42, !528, !658, !660, !528}
!658 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !659, size: 64)
!659 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !606)
!660 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !661, size: 64)
!661 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !630)
!662 = !DIDerivedType(tag: DW_TAG_member, name: "key", scope: !651, file: !530, line: 178, baseType: !528, size: 64, offset: 64)
!663 = !DIDerivedType(tag: DW_TAG_member, name: "keytype", scope: !651, file: !530, line: 179, baseType: !605, size: 64, offset: 128)
!664 = !DIDerivedType(tag: DW_TAG_member, name: "process_keyring", scope: !492, file: !493, line: 131, baseType: !528, size: 64, offset: 832)
!665 = !DIDerivedType(tag: DW_TAG_member, name: "thread_keyring", scope: !492, file: !493, line: 132, baseType: !528, size: 64, offset: 896)
!666 = !DIDerivedType(tag: DW_TAG_member, name: "request_key_auth", scope: !492, file: !493, line: 133, baseType: !528, size: 64, offset: 960)
!667 = !DIDerivedType(tag: DW_TAG_member, name: "security", scope: !492, file: !493, line: 136, baseType: !40, size: 64, offset: 1024)
!668 = !DIDerivedType(tag: DW_TAG_member, name: "user", scope: !492, file: !493, line: 138, baseType: !669, size: 64, offset: 1088)
!669 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !670, size: 64)
!670 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "user_struct", file: !671, line: 14, size: 1024, elements: !672)
!671 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/sched/user.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "5b9b728b5daa0648e64b85855f2d2d7d")
!672 = !{!673, !674, !683, !684, !685, !686, !687, !688}
!673 = !DIDerivedType(tag: DW_TAG_member, name: "__count", scope: !670, file: !671, line: 15, baseType: !533, size: 32)
!674 = !DIDerivedType(tag: DW_TAG_member, name: "epoll_watches", scope: !670, file: !671, line: 17, baseType: !675, size: 320, offset: 64)
!675 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "percpu_counter", file: !676, line: 22, size: 320, elements: !677)
!676 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/percpu_counter.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "4e64e7da846ce975d9d49f9bd43a5c92")
!677 = !{!678, !679, !680, !681}
!678 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !675, file: !676, line: 23, baseType: !148, size: 32)
!679 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !675, file: !676, line: 24, baseType: !502, size: 64, offset: 64)
!680 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !675, file: !676, line: 26, baseType: !117, size: 128, offset: 128)
!681 = !DIDerivedType(tag: DW_TAG_member, name: "counters", scope: !675, file: !676, line: 28, baseType: !682, size: 64, offset: 256)
!682 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !541, size: 64)
!683 = !DIDerivedType(tag: DW_TAG_member, name: "unix_inflight", scope: !670, file: !671, line: 19, baseType: !59, size: 64, offset: 384)
!684 = !DIDerivedType(tag: DW_TAG_member, name: "pipe_bufs", scope: !670, file: !671, line: 20, baseType: !496, size: 64, offset: 448)
!685 = !DIDerivedType(tag: DW_TAG_member, name: "uidhash_node", scope: !670, file: !671, line: 23, baseType: !220, size: 128, offset: 512)
!686 = !DIDerivedType(tag: DW_TAG_member, name: "uid", scope: !670, file: !671, line: 24, baseType: !188, size: 32, offset: 640)
!687 = !DIDerivedType(tag: DW_TAG_member, name: "locked_vm", scope: !670, file: !671, line: 29, baseType: !496, size: 64, offset: 704)
!688 = !DIDerivedType(tag: DW_TAG_member, name: "ratelimit", scope: !670, file: !671, line: 36, baseType: !689, size: 256, offset: 768)
!689 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "ratelimit_state", file: !690, line: 15, size: 256, elements: !691)
!690 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/ratelimit_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "e8200e5c8cb257432c44a54073ab5e0f")
!691 = !{!692, !693, !694, !695, !696, !697, !698}
!692 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !689, file: !690, line: 16, baseType: !148, size: 32)
!693 = !DIDerivedType(tag: DW_TAG_member, name: "interval", scope: !689, file: !690, line: 18, baseType: !42, size: 32, offset: 32)
!694 = !DIDerivedType(tag: DW_TAG_member, name: "burst", scope: !689, file: !690, line: 19, baseType: !42, size: 32, offset: 64)
!695 = !DIDerivedType(tag: DW_TAG_member, name: "printed", scope: !689, file: !690, line: 20, baseType: !42, size: 32, offset: 96)
!696 = !DIDerivedType(tag: DW_TAG_member, name: "missed", scope: !689, file: !690, line: 21, baseType: !42, size: 32, offset: 128)
!697 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !689, file: !690, line: 22, baseType: !7, size: 32, offset: 160)
!698 = !DIDerivedType(tag: DW_TAG_member, name: "begin", scope: !689, file: !690, line: 23, baseType: !59, size: 64, offset: 192)
!699 = !DIDerivedType(tag: DW_TAG_member, name: "user_ns", scope: !492, file: !493, line: 139, baseType: !700, size: 64, offset: 1152)
!700 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !701, size: 64)
!701 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "user_namespace", file: !702, line: 74, size: 4736, elements: !703)
!702 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/user_namespace.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "d3ce10902e4d1bda8fcc2fec1bd78128")
!703 = !{!704, !729, !730, !731, !732, !733, !734, !735, !5599, !5600, !5601, !5602, !5603, !5604, !5605, !5606, !5607, !5608, !5610, !5611}
!704 = !DIDerivedType(tag: DW_TAG_member, name: "uid_map", scope: !701, file: !702, line: 75, baseType: !705, size: 512)
!705 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "uid_gid_map", file: !702, line: 23, size: 512, elements: !706)
!706 = !{!707}
!707 = !DIDerivedType(tag: DW_TAG_member, scope: !705, file: !702, line: 24, baseType: !708, size: 512)
!708 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !705, file: !702, line: 24, size: 512, elements: !709)
!709 = !{!710, !723}
!710 = !DIDerivedType(tag: DW_TAG_member, scope: !708, file: !702, line: 25, baseType: !711, size: 512)
!711 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !708, file: !702, line: 25, size: 512, elements: !712)
!712 = !{!713, !722}
!713 = !DIDerivedType(tag: DW_TAG_member, name: "extent", scope: !711, file: !702, line: 26, baseType: !714, size: 480)
!714 = !DICompositeType(tag: DW_TAG_array_type, baseType: !715, size: 480, elements: !720)
!715 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "uid_gid_extent", file: !702, line: 17, size: 96, elements: !716)
!716 = !{!717, !718, !719}
!717 = !DIDerivedType(tag: DW_TAG_member, name: "first", scope: !715, file: !702, line: 18, baseType: !578, size: 32)
!718 = !DIDerivedType(tag: DW_TAG_member, name: "lower_first", scope: !715, file: !702, line: 19, baseType: !578, size: 32, offset: 32)
!719 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !715, file: !702, line: 20, baseType: !578, size: 32, offset: 64)
!720 = !{!721}
!721 = !DISubrange(count: 5)
!722 = !DIDerivedType(tag: DW_TAG_member, name: "nr_extents", scope: !711, file: !702, line: 27, baseType: !578, size: 32, offset: 480)
!723 = !DIDerivedType(tag: DW_TAG_member, scope: !708, file: !702, line: 29, baseType: !724, size: 128)
!724 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !708, file: !702, line: 29, size: 128, elements: !725)
!725 = !{!726, !728}
!726 = !DIDerivedType(tag: DW_TAG_member, name: "forward", scope: !724, file: !702, line: 30, baseType: !727, size: 64)
!727 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !715, size: 64)
!728 = !DIDerivedType(tag: DW_TAG_member, name: "reverse", scope: !724, file: !702, line: 31, baseType: !727, size: 64, offset: 64)
!729 = !DIDerivedType(tag: DW_TAG_member, name: "gid_map", scope: !701, file: !702, line: 76, baseType: !705, size: 512, offset: 512)
!730 = !DIDerivedType(tag: DW_TAG_member, name: "projid_map", scope: !701, file: !702, line: 77, baseType: !705, size: 512, offset: 1024)
!731 = !DIDerivedType(tag: DW_TAG_member, name: "parent", scope: !701, file: !702, line: 78, baseType: !700, size: 64, offset: 1536)
!732 = !DIDerivedType(tag: DW_TAG_member, name: "level", scope: !701, file: !702, line: 79, baseType: !42, size: 32, offset: 1600)
!733 = !DIDerivedType(tag: DW_TAG_member, name: "owner", scope: !701, file: !702, line: 80, baseType: !188, size: 32, offset: 1632)
!734 = !DIDerivedType(tag: DW_TAG_member, name: "group", scope: !701, file: !702, line: 81, baseType: !196, size: 32, offset: 1664)
!735 = !DIDerivedType(tag: DW_TAG_member, name: "ns", scope: !701, file: !702, line: 82, baseType: !736, size: 192, offset: 1728)
!736 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "ns_common", file: !737, line: 9, size: 192, elements: !738)
!737 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/ns_common.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "358ded68a7c4608cbd74489170f50c48")
!738 = !{!739, !5593, !5597, !5598}
!739 = !DIDerivedType(tag: DW_TAG_member, name: "stashed", scope: !736, file: !737, line: 10, baseType: !740, size: 64)
!740 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !741, size: 64)
!741 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "dentry", file: !366, line: 82, size: 1536, elements: !742)
!742 = !{!743, !744, !754, !762, !763, !778, !5558, !5562, !5563, !5564, !5565, !5566, !5579, !5585, !5586, !5587}
!743 = !DIDerivedType(tag: DW_TAG_member, name: "d_flags", scope: !741, file: !366, line: 84, baseType: !7, size: 32)
!744 = !DIDerivedType(tag: DW_TAG_member, name: "d_seq", scope: !741, file: !366, line: 85, baseType: !745, size: 32, offset: 32)
!745 = !DIDerivedType(tag: DW_TAG_typedef, name: "seqcount_spinlock_t", file: !746, line: 69, baseType: !747)
!746 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/seqlock_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "dadd9d4971959829a57ff5722128b2a8")
!747 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "seqcount_spinlock", file: !746, line: 69, size: 32, elements: !748)
!748 = !{!749}
!749 = !DIDerivedType(tag: DW_TAG_member, name: "seqcount", scope: !747, file: !746, line: 69, baseType: !750, size: 32)
!750 = !DIDerivedType(tag: DW_TAG_typedef, name: "seqcount_t", file: !746, line: 38, baseType: !751)
!751 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "seqcount", file: !746, line: 33, size: 32, elements: !752)
!752 = !{!753}
!753 = !DIDerivedType(tag: DW_TAG_member, name: "sequence", scope: !751, file: !746, line: 34, baseType: !7, size: 32)
!754 = !DIDerivedType(tag: DW_TAG_member, name: "d_hash", scope: !741, file: !366, line: 86, baseType: !755, size: 128, offset: 64)
!755 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "hlist_bl_node", file: !756, line: 38, size: 128, elements: !757)
!756 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/list_bl.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "35bd436f1f6159eef157d09cf839df5b")
!757 = !{!758, !760}
!758 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !755, file: !756, line: 39, baseType: !759, size: 64)
!759 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !755, size: 64)
!760 = !DIDerivedType(tag: DW_TAG_member, name: "pprev", scope: !755, file: !756, line: 39, baseType: !761, size: 64, offset: 64)
!761 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !759, size: 64)
!762 = !DIDerivedType(tag: DW_TAG_member, name: "d_parent", scope: !741, file: !366, line: 87, baseType: !740, size: 64, offset: 192)
!763 = !DIDerivedType(tag: DW_TAG_member, name: "d_name", scope: !741, file: !366, line: 88, baseType: !764, size: 128, offset: 256)
!764 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "qstr", file: !366, line: 49, size: 128, elements: !765)
!765 = !{!766, !775}
!766 = !DIDerivedType(tag: DW_TAG_member, scope: !764, file: !366, line: 50, baseType: !767, size: 64)
!767 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !764, file: !366, line: 50, size: 64, elements: !768)
!768 = !{!769, !774}
!769 = !DIDerivedType(tag: DW_TAG_member, scope: !767, file: !366, line: 51, baseType: !770, size: 64)
!770 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !767, file: !366, line: 51, size: 64, elements: !771)
!771 = !{!772, !773}
!772 = !DIDerivedType(tag: DW_TAG_member, name: "hash", scope: !770, file: !366, line: 52, baseType: !578, size: 32)
!773 = !DIDerivedType(tag: DW_TAG_member, name: "len", scope: !770, file: !366, line: 52, baseType: !578, size: 32, offset: 32)
!774 = !DIDerivedType(tag: DW_TAG_member, name: "hash_len", scope: !767, file: !366, line: 54, baseType: !519, size: 64)
!775 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !764, file: !366, line: 56, baseType: !776, size: 64, offset: 64)
!776 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !777, size: 64)
!777 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !107)
!778 = !DIDerivedType(tag: DW_TAG_member, name: "d_inode", scope: !741, file: !366, line: 89, baseType: !779, size: 64, offset: 384)
!779 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !780, size: 64)
!780 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "inode", file: !342, line: 632, size: 4736, elements: !781)
!781 = !{!782, !783, !784, !785, !786, !787, !790, !791, !5479, !5480, !5481, !5482, !5483, !5489, !5490, !5491, !5492, !5493, !5494, !5495, !5496, !5497, !5498, !5499, !5500, !5501, !5502, !5503, !5504, !5505, !5506, !5507, !5508, !5509, !5510, !5511, !5512, !5517, !5518, !5519, !5520, !5521, !5522, !5523, !5528, !5536, !5537, !5538, !5555, !5556, !5557}
!782 = !DIDerivedType(tag: DW_TAG_member, name: "i_mode", scope: !780, file: !342, line: 633, baseType: !44, size: 16)
!783 = !DIDerivedType(tag: DW_TAG_member, name: "i_opflags", scope: !780, file: !342, line: 634, baseType: !46, size: 16, offset: 16)
!784 = !DIDerivedType(tag: DW_TAG_member, name: "i_uid", scope: !780, file: !342, line: 635, baseType: !188, size: 32, offset: 32)
!785 = !DIDerivedType(tag: DW_TAG_member, name: "i_gid", scope: !780, file: !342, line: 636, baseType: !196, size: 32, offset: 64)
!786 = !DIDerivedType(tag: DW_TAG_member, name: "i_flags", scope: !780, file: !342, line: 637, baseType: !7, size: 32, offset: 96)
!787 = !DIDerivedType(tag: DW_TAG_member, name: "i_acl", scope: !780, file: !342, line: 640, baseType: !788, size: 64, offset: 128)
!788 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !789, size: 64)
!789 = !DICompositeType(tag: DW_TAG_structure_type, name: "posix_acl", file: !342, line: 600, flags: DIFlagFwdDecl)
!790 = !DIDerivedType(tag: DW_TAG_member, name: "i_default_acl", scope: !780, file: !342, line: 641, baseType: !788, size: 64, offset: 192)
!791 = !DIDerivedType(tag: DW_TAG_member, name: "i_op", scope: !780, file: !342, line: 644, baseType: !792, size: 64, offset: 256)
!792 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !793, size: 64)
!793 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !794)
!794 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "inode_operations", file: !342, line: 2129, size: 2048, align: 512, elements: !795)
!795 = !{!796, !800, !813, !819, !823, !827, !831, !835, !839, !843, !847, !848, !854, !858, !5397, !5430, !5434, !5440, !5444, !5448, !5452, !5456, !5460, !5466, !5470}
!796 = !DIDerivedType(tag: DW_TAG_member, name: "lookup", scope: !794, file: !342, line: 2130, baseType: !797, size: 64)
!797 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !798, size: 64)
!798 = !DISubroutineType(types: !799)
!799 = !{!740, !779, !740, !7}
!800 = !DIDerivedType(tag: DW_TAG_member, name: "get_link", scope: !794, file: !342, line: 2131, baseType: !801, size: 64, offset: 64)
!801 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !802, size: 64)
!802 = !DISubroutineType(types: !803)
!803 = !{!36, !740, !779, !804}
!804 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !805, size: 64)
!805 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "delayed_call", file: !806, line: 10, size: 128, elements: !807)
!806 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/delayed_call.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "e61eb32de84dec9b8b545d0ea330611b")
!807 = !{!808, !812}
!808 = !DIDerivedType(tag: DW_TAG_member, name: "fn", scope: !805, file: !806, line: 11, baseType: !809, size: 64)
!809 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !810, size: 64)
!810 = !DISubroutineType(types: !811)
!811 = !{null, !40}
!812 = !DIDerivedType(tag: DW_TAG_member, name: "arg", scope: !805, file: !806, line: 12, baseType: !40, size: 64, offset: 64)
!813 = !DIDerivedType(tag: DW_TAG_member, name: "permission", scope: !794, file: !342, line: 2132, baseType: !814, size: 64, offset: 128)
!814 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !815, size: 64)
!815 = !DISubroutineType(types: !816)
!816 = !{!42, !817, !779, !42}
!817 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !818, size: 64)
!818 = !DICompositeType(tag: DW_TAG_structure_type, name: "mnt_idmap", file: !515, line: 42, flags: DIFlagFwdDecl)
!819 = !DIDerivedType(tag: DW_TAG_member, name: "get_inode_acl", scope: !794, file: !342, line: 2133, baseType: !820, size: 64, offset: 192)
!820 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !821, size: 64)
!821 = !DISubroutineType(types: !822)
!822 = !{!788, !779, !42, !614}
!823 = !DIDerivedType(tag: DW_TAG_member, name: "readlink", scope: !794, file: !342, line: 2135, baseType: !824, size: 64, offset: 256)
!824 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !825, size: 64)
!825 = !DISubroutineType(types: !826)
!826 = !{!42, !740, !625, !42}
!827 = !DIDerivedType(tag: DW_TAG_member, name: "create", scope: !794, file: !342, line: 2137, baseType: !828, size: 64, offset: 320)
!828 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !829, size: 64)
!829 = !DISubroutineType(types: !830)
!830 = !{!42, !817, !779, !740, !44, !614}
!831 = !DIDerivedType(tag: DW_TAG_member, name: "link", scope: !794, file: !342, line: 2139, baseType: !832, size: 64, offset: 384)
!832 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !833, size: 64)
!833 = !DISubroutineType(types: !834)
!834 = !{!42, !740, !779, !740}
!835 = !DIDerivedType(tag: DW_TAG_member, name: "unlink", scope: !794, file: !342, line: 2140, baseType: !836, size: 64, offset: 448)
!836 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !837, size: 64)
!837 = !DISubroutineType(types: !838)
!838 = !{!42, !779, !740}
!839 = !DIDerivedType(tag: DW_TAG_member, name: "symlink", scope: !794, file: !342, line: 2141, baseType: !840, size: 64, offset: 512)
!840 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !841, size: 64)
!841 = !DISubroutineType(types: !842)
!842 = !{!42, !817, !779, !740, !36}
!843 = !DIDerivedType(tag: DW_TAG_member, name: "mkdir", scope: !794, file: !342, line: 2143, baseType: !844, size: 64, offset: 576)
!844 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !845, size: 64)
!845 = !DISubroutineType(types: !846)
!846 = !{!42, !817, !779, !740, !44}
!847 = !DIDerivedType(tag: DW_TAG_member, name: "rmdir", scope: !794, file: !342, line: 2145, baseType: !836, size: 64, offset: 640)
!848 = !DIDerivedType(tag: DW_TAG_member, name: "mknod", scope: !794, file: !342, line: 2146, baseType: !849, size: 64, offset: 704)
!849 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !850, size: 64)
!850 = !DISubroutineType(types: !851)
!851 = !{!42, !817, !779, !740, !44, !852}
!852 = !DIDerivedType(tag: DW_TAG_typedef, name: "dev_t", file: !45, line: 21, baseType: !853)
!853 = !DIDerivedType(tag: DW_TAG_typedef, name: "__kernel_dev_t", file: !45, line: 18, baseType: !578)
!854 = !DIDerivedType(tag: DW_TAG_member, name: "rename", scope: !794, file: !342, line: 2148, baseType: !855, size: 64, offset: 768)
!855 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !856, size: 64)
!856 = !DISubroutineType(types: !857)
!857 = !{!42, !817, !779, !740, !779, !740, !7}
!858 = !DIDerivedType(tag: DW_TAG_member, name: "setattr", scope: !794, file: !342, line: 2150, baseType: !859, size: 64, offset: 832)
!859 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !860, size: 64)
!860 = !DISubroutineType(types: !861)
!861 = !{!42, !817, !740, !862}
!862 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !863, size: 64)
!863 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "iattr", file: !342, line: 230, size: 640, elements: !864)
!864 = !{!865, !866, !867, !877, !886, !887, !893, !894, !895}
!865 = !DIDerivedType(tag: DW_TAG_member, name: "ia_valid", scope: !863, file: !342, line: 231, baseType: !7, size: 32)
!866 = !DIDerivedType(tag: DW_TAG_member, name: "ia_mode", scope: !863, file: !342, line: 232, baseType: !44, size: 16, offset: 32)
!867 = !DIDerivedType(tag: DW_TAG_member, scope: !863, file: !342, line: 245, baseType: !868, size: 32, offset: 64)
!868 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !863, file: !342, line: 245, size: 32, elements: !869)
!869 = !{!870, !871}
!870 = !DIDerivedType(tag: DW_TAG_member, name: "ia_uid", scope: !868, file: !342, line: 246, baseType: !188, size: 32)
!871 = !DIDerivedType(tag: DW_TAG_member, name: "ia_vfsuid", scope: !868, file: !342, line: 247, baseType: !872, size: 32)
!872 = !DIDerivedType(tag: DW_TAG_typedef, name: "vfsuid_t", file: !873, line: 17, baseType: !874)
!873 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/mnt_idmapping.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "0c91b7822fa23231c4e926f88dea1d18")
!874 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !873, line: 15, size: 32, elements: !875)
!875 = !{!876}
!876 = !DIDerivedType(tag: DW_TAG_member, name: "val", scope: !874, file: !873, line: 16, baseType: !193, size: 32)
!877 = !DIDerivedType(tag: DW_TAG_member, scope: !863, file: !342, line: 249, baseType: !878, size: 32, offset: 96)
!878 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !863, file: !342, line: 249, size: 32, elements: !879)
!879 = !{!880, !881}
!880 = !DIDerivedType(tag: DW_TAG_member, name: "ia_gid", scope: !878, file: !342, line: 250, baseType: !196, size: 32)
!881 = !DIDerivedType(tag: DW_TAG_member, name: "ia_vfsgid", scope: !878, file: !342, line: 251, baseType: !882, size: 32)
!882 = !DIDerivedType(tag: DW_TAG_typedef, name: "vfsgid_t", file: !873, line: 21, baseType: !883)
!883 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !873, line: 19, size: 32, elements: !884)
!884 = !{!885}
!885 = !DIDerivedType(tag: DW_TAG_member, name: "val", scope: !883, file: !873, line: 20, baseType: !200, size: 32)
!886 = !DIDerivedType(tag: DW_TAG_member, name: "ia_size", scope: !863, file: !342, line: 253, baseType: !61, size: 64, offset: 128)
!887 = !DIDerivedType(tag: DW_TAG_member, name: "ia_atime", scope: !863, file: !342, line: 254, baseType: !888, size: 128, offset: 192)
!888 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "timespec64", file: !570, line: 13, size: 128, elements: !889)
!889 = !{!890, !891}
!890 = !DIDerivedType(tag: DW_TAG_member, name: "tv_sec", scope: !888, file: !570, line: 14, baseType: !569, size: 64)
!891 = !DIDerivedType(tag: DW_TAG_member, name: "tv_nsec", scope: !888, file: !570, line: 15, baseType: !892, size: 64, offset: 64)
!892 = !DIBasicType(name: "long", size: 64, encoding: DW_ATE_signed)
!893 = !DIDerivedType(tag: DW_TAG_member, name: "ia_mtime", scope: !863, file: !342, line: 255, baseType: !888, size: 128, offset: 320)
!894 = !DIDerivedType(tag: DW_TAG_member, name: "ia_ctime", scope: !863, file: !342, line: 256, baseType: !888, size: 128, offset: 448)
!895 = !DIDerivedType(tag: DW_TAG_member, name: "ia_file", scope: !863, file: !342, line: 263, baseType: !896, size: 64, offset: 576)
!896 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !897, size: 64)
!897 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "file", file: !342, line: 1032, size: 1472, align: 64, elements: !898)
!898 = !{!899, !900, !901, !902, !3155, !3156, !3157, !3158, !3159, !3160, !3161, !5364, !5369, !5370, !5371, !5382, !5383, !5384, !5386}
!899 = !DIDerivedType(tag: DW_TAG_member, name: "f_count", scope: !897, file: !342, line: 1033, baseType: !496, size: 64)
!900 = !DIDerivedType(tag: DW_TAG_member, name: "f_lock", scope: !897, file: !342, line: 1034, baseType: !79, size: 32, offset: 64)
!901 = !DIDerivedType(tag: DW_TAG_member, name: "f_mode", scope: !897, file: !342, line: 1035, baseType: !489, size: 32, offset: 96)
!902 = !DIDerivedType(tag: DW_TAG_member, name: "f_op", scope: !897, file: !342, line: 1036, baseType: !903, size: 64, offset: 128)
!903 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !904, size: 64)
!904 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !905)
!905 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "file_operations", file: !342, line: 2062, size: 2112, elements: !906)
!906 = !{!907, !2890, !2892, !2896, !2900, !2904, !2905, !2906, !2912, !2925, !2929, !2933, !2934, !2938, !2942, !2947, !2948, !2952, !2956, !3049, !3053, !3057, !3058, !3064, !3068, !3069, !3128, !3132, !3136, !3140, !3144, !3145, !3151}
!907 = !DIDerivedType(tag: DW_TAG_member, name: "owner", scope: !905, file: !342, line: 2063, baseType: !908, size: 64)
!908 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !909, size: 64)
!909 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "module", file: !6, line: 408, size: 9216, align: 512, elements: !910)
!910 = !{!911, !912, !913, !917, !2560, !2586, !2587, !2588, !2589, !2593, !2596, !2597, !2598, !2649, !2650, !2651, !2652, !2653, !2654, !2655, !2656, !2664, !2668, !2688, !2706, !2707, !2708, !2709, !2718, !2741, !2742, !2745, !2748, !2749, !2750, !2751, !2752, !2753, !2754, !2759, !2760, !2849, !2857, !2858, !2859, !2861, !2865, !2866, !2870, !2871, !2872, !2873, !2874, !2875, !2876, !2883, !2884, !2885, !2889}
!911 = !DIDerivedType(tag: DW_TAG_member, name: "state", scope: !909, file: !6, line: 409, baseType: !5, size: 32)
!912 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !909, file: !6, line: 412, baseType: !117, size: 128, offset: 64)
!913 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !909, file: !6, line: 415, baseType: !914, size: 448, offset: 192)
!914 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 448, elements: !915)
!915 = !{!916}
!916 = !DISubrange(count: 56)
!917 = !DIDerivedType(tag: DW_TAG_member, name: "mkobj", scope: !909, file: !6, line: 423, baseType: !918, size: 768, offset: 640)
!918 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "module_kobject", file: !6, line: 45, size: 768, elements: !919)
!919 = !{!920, !2554, !2555, !2556, !2559}
!920 = !DIDerivedType(tag: DW_TAG_member, name: "kobj", scope: !918, file: !6, line: 46, baseType: !921, size: 512)
!921 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kobject", file: !922, line: 64, size: 512, elements: !923)
!922 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/kobject.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "96536893f97d0488068d77e7ffdc8b2c")
!923 = !{!924, !925, !926, !928, !974, !2406, !2544, !2549, !2550, !2551, !2552, !2553}
!924 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !921, file: !922, line: 65, baseType: !36, size: 64)
!925 = !DIDerivedType(tag: DW_TAG_member, name: "entry", scope: !921, file: !922, line: 66, baseType: !117, size: 128, offset: 64)
!926 = !DIDerivedType(tag: DW_TAG_member, name: "parent", scope: !921, file: !922, line: 67, baseType: !927, size: 64, offset: 192)
!927 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !921, size: 64)
!928 = !DIDerivedType(tag: DW_TAG_member, name: "kset", scope: !921, file: !922, line: 68, baseType: !929, size: 64, offset: 256)
!929 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !930, size: 64)
!930 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kset", file: !922, line: 168, size: 768, elements: !931)
!931 = !{!932, !933, !934, !935}
!932 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !930, file: !922, line: 169, baseType: !117, size: 128)
!933 = !DIDerivedType(tag: DW_TAG_member, name: "list_lock", scope: !930, file: !922, line: 170, baseType: !79, size: 32, offset: 128)
!934 = !DIDerivedType(tag: DW_TAG_member, name: "kobj", scope: !930, file: !922, line: 171, baseType: !921, size: 512, offset: 192)
!935 = !DIDerivedType(tag: DW_TAG_member, name: "uevent_ops", scope: !930, file: !922, line: 172, baseType: !936, size: 64, offset: 704)
!936 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !937, size: 64)
!937 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !938)
!938 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kset_uevent_ops", file: !922, line: 133, size: 192, elements: !939)
!939 = !{!940, !947, !952}
!940 = !DIDerivedType(tag: DW_TAG_member, name: "filter", scope: !938, file: !922, line: 134, baseType: !941, size: 64)
!941 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !942)
!942 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !943, size: 64)
!943 = !DISubroutineType(types: !944)
!944 = !{!42, !945}
!945 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !946, size: 64)
!946 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !921)
!947 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !938, file: !922, line: 135, baseType: !948, size: 64, offset: 64)
!948 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !949)
!949 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !950, size: 64)
!950 = !DISubroutineType(types: !951)
!951 = !{!36, !945}
!952 = !DIDerivedType(tag: DW_TAG_member, name: "uevent", scope: !938, file: !922, line: 136, baseType: !953, size: 64, offset: 128)
!953 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !954)
!954 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !955, size: 64)
!955 = !DISubroutineType(types: !956)
!956 = !{!42, !945, !957}
!957 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !958, size: 64)
!958 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kobj_uevent_env", file: !922, line: 125, size: 20736, elements: !959)
!959 = !{!960, !964, !968, !969, !973}
!960 = !DIDerivedType(tag: DW_TAG_member, name: "argv", scope: !958, file: !922, line: 126, baseType: !961, size: 192)
!961 = !DICompositeType(tag: DW_TAG_array_type, baseType: !625, size: 192, elements: !962)
!962 = !{!963}
!963 = !DISubrange(count: 3)
!964 = !DIDerivedType(tag: DW_TAG_member, name: "envp", scope: !958, file: !922, line: 127, baseType: !965, size: 4096, offset: 192)
!965 = !DICompositeType(tag: DW_TAG_array_type, baseType: !625, size: 4096, elements: !966)
!966 = !{!967}
!967 = !DISubrange(count: 64)
!968 = !DIDerivedType(tag: DW_TAG_member, name: "envp_idx", scope: !958, file: !922, line: 128, baseType: !42, size: 32, offset: 4288)
!969 = !DIDerivedType(tag: DW_TAG_member, name: "buf", scope: !958, file: !922, line: 129, baseType: !970, size: 16384, offset: 4320)
!970 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 16384, elements: !971)
!971 = !{!972}
!972 = !DISubrange(count: 2048)
!973 = !DIDerivedType(tag: DW_TAG_member, name: "buflen", scope: !958, file: !922, line: 130, baseType: !42, size: 32, offset: 20704)
!974 = !DIDerivedType(tag: DW_TAG_member, name: "ktype", scope: !921, file: !922, line: 69, baseType: !975, size: 64, offset: 320)
!975 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !976, size: 64)
!976 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !977)
!977 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kobj_type", file: !922, line: 116, size: 384, elements: !978)
!978 = !{!979, !983, !1005, !2394, !2398, !2402}
!979 = !DIDerivedType(tag: DW_TAG_member, name: "release", scope: !977, file: !922, line: 117, baseType: !980, size: 64)
!980 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !981, size: 64)
!981 = !DISubroutineType(types: !982)
!982 = !{null, !927}
!983 = !DIDerivedType(tag: DW_TAG_member, name: "sysfs_ops", scope: !977, file: !922, line: 118, baseType: !984, size: 64, offset: 64)
!984 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !985, size: 64)
!985 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !986)
!986 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "sysfs_ops", file: !987, line: 385, size: 128, elements: !988)
!987 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/sysfs.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "e38677ca7e6fe054e769dbb3e7ef0eb8")
!988 = !{!989, !1001}
!989 = !DIDerivedType(tag: DW_TAG_member, name: "show", scope: !986, file: !987, line: 386, baseType: !990, size: 64)
!990 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !991, size: 64)
!991 = !DISubroutineType(types: !992)
!992 = !{!993, !927, !996, !625}
!993 = !DIDerivedType(tag: DW_TAG_typedef, name: "ssize_t", file: !45, line: 66, baseType: !994)
!994 = !DIDerivedType(tag: DW_TAG_typedef, name: "__kernel_ssize_t", file: !57, line: 73, baseType: !995)
!995 = !DIDerivedType(tag: DW_TAG_typedef, name: "__kernel_long_t", file: !57, line: 15, baseType: !892)
!996 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !997, size: 64)
!997 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "attribute", file: !987, line: 30, size: 128, elements: !998)
!998 = !{!999, !1000}
!999 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !997, file: !987, line: 31, baseType: !36, size: 64)
!1000 = !DIDerivedType(tag: DW_TAG_member, name: "mode", scope: !997, file: !987, line: 32, baseType: !44, size: 16, offset: 64)
!1001 = !DIDerivedType(tag: DW_TAG_member, name: "store", scope: !986, file: !987, line: 387, baseType: !1002, size: 64, offset: 64)
!1002 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1003, size: 64)
!1003 = !DISubroutineType(types: !1004)
!1004 = !{!993, !927, !996, !36, !55}
!1005 = !DIDerivedType(tag: DW_TAG_member, name: "default_groups", scope: !977, file: !922, line: 119, baseType: !1006, size: 64, offset: 128)
!1006 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1007, size: 64)
!1007 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1008, size: 64)
!1008 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1009)
!1009 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "attribute_group", file: !987, line: 94, size: 320, elements: !1010)
!1010 = !{!1011, !1012, !1016, !2390, !2392}
!1011 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !1009, file: !987, line: 95, baseType: !36, size: 64)
!1012 = !DIDerivedType(tag: DW_TAG_member, name: "is_visible", scope: !1009, file: !987, line: 96, baseType: !1013, size: 64, offset: 64)
!1013 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1014, size: 64)
!1014 = !DISubroutineType(types: !1015)
!1015 = !{!44, !927, !996, !42}
!1016 = !DIDerivedType(tag: DW_TAG_member, name: "is_bin_visible", scope: !1009, file: !987, line: 98, baseType: !1017, size: 64, offset: 128)
!1017 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1018, size: 64)
!1018 = !DISubroutineType(types: !1019)
!1019 = !{!44, !927, !1020, !42}
!1020 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1021, size: 64)
!1021 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bin_attribute", file: !987, line: 293, size: 576, elements: !1022)
!1022 = !{!1023, !1024, !1025, !1026, !2377, !2381, !2382, !2386}
!1023 = !DIDerivedType(tag: DW_TAG_member, name: "attr", scope: !1021, file: !987, line: 294, baseType: !997, size: 128)
!1024 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !1021, file: !987, line: 295, baseType: !55, size: 64, offset: 128)
!1025 = !DIDerivedType(tag: DW_TAG_member, name: "private", scope: !1021, file: !987, line: 296, baseType: !40, size: 64, offset: 192)
!1026 = !DIDerivedType(tag: DW_TAG_member, name: "f_mapping", scope: !1021, file: !987, line: 297, baseType: !1027, size: 64, offset: 256)
!1027 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1028, size: 64)
!1028 = !DISubroutineType(types: !1029)
!1029 = !{!1030}
!1030 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1031, size: 64)
!1031 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "address_space", file: !342, line: 465, size: 1536, align: 64, elements: !1032)
!1032 = !{!1033, !1034, !1041, !1042, !1043, !1044, !1049, !1050, !1051, !2369, !2370, !2373, !2374, !2375, !2376}
!1033 = !DIDerivedType(tag: DW_TAG_member, name: "host", scope: !1031, file: !342, line: 466, baseType: !779, size: 64)
!1034 = !DIDerivedType(tag: DW_TAG_member, name: "i_pages", scope: !1031, file: !342, line: 467, baseType: !1035, size: 128, offset: 64)
!1035 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "xarray", file: !1036, line: 300, size: 128, elements: !1037)
!1036 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/xarray.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "0e6c58dbb845549bda9b7badbf73416c")
!1037 = !{!1038, !1039, !1040}
!1038 = !DIDerivedType(tag: DW_TAG_member, name: "xa_lock", scope: !1035, file: !1036, line: 301, baseType: !79, size: 32)
!1039 = !DIDerivedType(tag: DW_TAG_member, name: "xa_flags", scope: !1035, file: !1036, line: 303, baseType: !488, size: 32, offset: 32)
!1040 = !DIDerivedType(tag: DW_TAG_member, name: "xa_head", scope: !1035, file: !1036, line: 304, baseType: !40, size: 64, offset: 64)
!1041 = !DIDerivedType(tag: DW_TAG_member, name: "invalidate_lock", scope: !1031, file: !342, line: 468, baseType: !549, size: 320, offset: 192)
!1042 = !DIDerivedType(tag: DW_TAG_member, name: "gfp_mask", scope: !1031, file: !342, line: 469, baseType: !488, size: 32, offset: 512)
!1043 = !DIDerivedType(tag: DW_TAG_member, name: "i_mmap_writable", scope: !1031, file: !342, line: 470, baseType: !69, size: 32, offset: 544)
!1044 = !DIDerivedType(tag: DW_TAG_member, name: "i_mmap", scope: !1031, file: !342, line: 475, baseType: !1045, size: 128, offset: 576)
!1045 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "rb_root_cached", file: !169, line: 26, size: 128, elements: !1046)
!1046 = !{!1047, !1048}
!1047 = !DIDerivedType(tag: DW_TAG_member, name: "rb_root", scope: !1045, file: !169, line: 27, baseType: !168, size: 64)
!1048 = !DIDerivedType(tag: DW_TAG_member, name: "rb_leftmost", scope: !1045, file: !169, line: 28, baseType: !172, size: 64, offset: 64)
!1049 = !DIDerivedType(tag: DW_TAG_member, name: "nrpages", scope: !1031, file: !342, line: 476, baseType: !59, size: 64, offset: 704)
!1050 = !DIDerivedType(tag: DW_TAG_member, name: "writeback_index", scope: !1031, file: !342, line: 477, baseType: !59, size: 64, offset: 768)
!1051 = !DIDerivedType(tag: DW_TAG_member, name: "a_ops", scope: !1031, file: !342, line: 478, baseType: !1052, size: 64, offset: 832)
!1052 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1053, size: 64)
!1053 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1054)
!1054 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "address_space_operations", file: !342, line: 397, size: 1280, elements: !1055)
!1055 = !{!1056, !1619, !1623, !1627, !1631, !1656, !1662, !1666, !1671, !1675, !1679, !1683, !1781, !1785, !1789, !1793, !1798, !1802, !2361, !2365}
!1056 = !DIDerivedType(tag: DW_TAG_member, name: "writepage", scope: !1054, file: !342, line: 398, baseType: !1057, size: 64)
!1057 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1058, size: 64)
!1058 = !DISubroutineType(types: !1059)
!1059 = !{!42, !1060, !1519}
!1060 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1061, size: 64)
!1061 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "page", file: !230, line: 72, size: 512, align: 128, elements: !1062)
!1062 = !{!1063, !1064, !1513, !1518}
!1063 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !1061, file: !230, line: 73, baseType: !59, size: 64)
!1064 = !DIDerivedType(tag: DW_TAG_member, scope: !1061, file: !230, line: 81, baseType: !1065, size: 320, offset: 64)
!1065 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1061, file: !230, line: 81, size: 320, elements: !1066)
!1066 = !{!1067, !1088, !1098, !1102, !1512}
!1067 = !DIDerivedType(tag: DW_TAG_member, scope: !1065, file: !230, line: 82, baseType: !1068, size: 320)
!1068 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1065, file: !230, line: 82, size: 320, elements: !1069)
!1069 = !{!1070, !1081, !1082, !1087}
!1070 = !DIDerivedType(tag: DW_TAG_member, scope: !1068, file: !230, line: 88, baseType: !1071, size: 128)
!1071 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1068, file: !230, line: 88, size: 128, elements: !1072)
!1072 = !{!1073, !1074, !1079, !1080}
!1073 = !DIDerivedType(tag: DW_TAG_member, name: "lru", scope: !1071, file: !230, line: 89, baseType: !117, size: 128)
!1074 = !DIDerivedType(tag: DW_TAG_member, scope: !1071, file: !230, line: 92, baseType: !1075, size: 128)
!1075 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1071, file: !230, line: 92, size: 128, elements: !1076)
!1076 = !{!1077, !1078}
!1077 = !DIDerivedType(tag: DW_TAG_member, name: "__filler", scope: !1075, file: !230, line: 94, baseType: !40, size: 64)
!1078 = !DIDerivedType(tag: DW_TAG_member, name: "mlock_count", scope: !1075, file: !230, line: 96, baseType: !7, size: 32, offset: 64)
!1079 = !DIDerivedType(tag: DW_TAG_member, name: "buddy_list", scope: !1071, file: !230, line: 100, baseType: !117, size: 128)
!1080 = !DIDerivedType(tag: DW_TAG_member, name: "pcp_list", scope: !1071, file: !230, line: 101, baseType: !117, size: 128)
!1081 = !DIDerivedType(tag: DW_TAG_member, name: "mapping", scope: !1068, file: !230, line: 104, baseType: !1030, size: 64, offset: 128)
!1082 = !DIDerivedType(tag: DW_TAG_member, scope: !1068, file: !230, line: 105, baseType: !1083, size: 64, offset: 192)
!1083 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1068, file: !230, line: 105, size: 64, elements: !1084)
!1084 = !{!1085, !1086}
!1085 = !DIDerivedType(tag: DW_TAG_member, name: "index", scope: !1083, file: !230, line: 106, baseType: !59, size: 64)
!1086 = !DIDerivedType(tag: DW_TAG_member, name: "share", scope: !1083, file: !230, line: 107, baseType: !59, size: 64)
!1087 = !DIDerivedType(tag: DW_TAG_member, name: "private", scope: !1068, file: !230, line: 115, baseType: !59, size: 64, offset: 256)
!1088 = !DIDerivedType(tag: DW_TAG_member, scope: !1065, file: !230, line: 117, baseType: !1089, size: 320)
!1089 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1065, file: !230, line: 117, size: 320, elements: !1090)
!1090 = !{!1091, !1092, !1095, !1096, !1097}
!1091 = !DIDerivedType(tag: DW_TAG_member, name: "pp_magic", scope: !1089, file: !230, line: 122, baseType: !59, size: 64)
!1092 = !DIDerivedType(tag: DW_TAG_member, name: "pp", scope: !1089, file: !230, line: 123, baseType: !1093, size: 64, offset: 64)
!1093 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1094, size: 64)
!1094 = !DICompositeType(tag: DW_TAG_structure_type, name: "page_pool", file: !230, line: 123, flags: DIFlagFwdDecl)
!1095 = !DIDerivedType(tag: DW_TAG_member, name: "_pp_mapping_pad", scope: !1089, file: !230, line: 124, baseType: !59, size: 64, offset: 128)
!1096 = !DIDerivedType(tag: DW_TAG_member, name: "dma_addr", scope: !1089, file: !230, line: 125, baseType: !59, size: 64, offset: 192)
!1097 = !DIDerivedType(tag: DW_TAG_member, name: "pp_ref_count", scope: !1089, file: !230, line: 126, baseType: !496, size: 64, offset: 256)
!1098 = !DIDerivedType(tag: DW_TAG_member, scope: !1065, file: !230, line: 128, baseType: !1099, size: 64)
!1099 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1065, file: !230, line: 128, size: 64, elements: !1100)
!1100 = !{!1101}
!1101 = !DIDerivedType(tag: DW_TAG_member, name: "compound_head", scope: !1099, file: !230, line: 129, baseType: !59, size: 64)
!1102 = !DIDerivedType(tag: DW_TAG_member, scope: !1065, file: !230, line: 131, baseType: !1103, size: 128)
!1103 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1065, file: !230, line: 131, size: 128, elements: !1104)
!1104 = !{!1105, !1511}
!1105 = !DIDerivedType(tag: DW_TAG_member, name: "pgmap", scope: !1103, file: !230, line: 133, baseType: !1106, size: 64)
!1106 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1107, size: 64)
!1107 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "dev_pagemap", file: !14, line: 127, size: 1280, elements: !1108)
!1108 = !{!1109, !1120, !1141, !1142, !1143, !1144, !1145, !1493, !1494, !1495}
!1109 = !DIDerivedType(tag: DW_TAG_member, name: "altmap", scope: !1107, file: !14, line: 128, baseType: !1110, size: 448)
!1110 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "vmem_altmap", file: !14, line: 21, size: 448, elements: !1111)
!1111 = !{!1112, !1113, !1115, !1116, !1117, !1118, !1119}
!1112 = !DIDerivedType(tag: DW_TAG_member, name: "base_pfn", scope: !1110, file: !14, line: 22, baseType: !59, size: 64)
!1113 = !DIDerivedType(tag: DW_TAG_member, name: "end_pfn", scope: !1110, file: !14, line: 23, baseType: !1114, size: 64, offset: 64)
!1114 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !59)
!1115 = !DIDerivedType(tag: DW_TAG_member, name: "reserve", scope: !1110, file: !14, line: 24, baseType: !1114, size: 64, offset: 128)
!1116 = !DIDerivedType(tag: DW_TAG_member, name: "free", scope: !1110, file: !14, line: 25, baseType: !59, size: 64, offset: 192)
!1117 = !DIDerivedType(tag: DW_TAG_member, name: "align", scope: !1110, file: !14, line: 26, baseType: !59, size: 64, offset: 256)
!1118 = !DIDerivedType(tag: DW_TAG_member, name: "alloc", scope: !1110, file: !14, line: 27, baseType: !59, size: 64, offset: 320)
!1119 = !DIDerivedType(tag: DW_TAG_member, name: "inaccessible", scope: !1110, file: !14, line: 28, baseType: !614, size: 8, offset: 384)
!1120 = !DIDerivedType(tag: DW_TAG_member, name: "ref", scope: !1107, file: !14, line: 129, baseType: !1121, size: 128, offset: 448)
!1121 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "percpu_ref", file: !1122, line: 105, size: 128, elements: !1123)
!1122 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/percpu-refcount.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "c4be69e3dcb700658754c20429d5356a")
!1123 = !{!1124, !1125}
!1124 = !DIDerivedType(tag: DW_TAG_member, name: "percpu_count_ptr", scope: !1121, file: !1122, line: 110, baseType: !59, size: 64)
!1125 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !1121, file: !1122, line: 118, baseType: !1126, size: 64, offset: 64)
!1126 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1127, size: 64)
!1127 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "percpu_ref_data", file: !1122, line: 95, size: 448, elements: !1128)
!1128 = !{!1129, !1130, !1136, !1137, !1138, !1139, !1140}
!1129 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !1127, file: !1122, line: 96, baseType: !496, size: 64)
!1130 = !DIDerivedType(tag: DW_TAG_member, name: "release", scope: !1127, file: !1122, line: 97, baseType: !1131, size: 64, offset: 64)
!1131 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1132, size: 64)
!1132 = !DIDerivedType(tag: DW_TAG_typedef, name: "percpu_ref_func_t", file: !1122, line: 60, baseType: !1133)
!1133 = !DISubroutineType(types: !1134)
!1134 = !{null, !1135}
!1135 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1121, size: 64)
!1136 = !DIDerivedType(tag: DW_TAG_member, name: "confirm_switch", scope: !1127, file: !1122, line: 98, baseType: !1131, size: 64, offset: 128)
!1137 = !DIDerivedType(tag: DW_TAG_member, name: "force_atomic", scope: !1127, file: !1122, line: 99, baseType: !614, size: 1, offset: 192, flags: DIFlagBitField, extraData: i64 192)
!1138 = !DIDerivedType(tag: DW_TAG_member, name: "allow_reinit", scope: !1127, file: !1122, line: 100, baseType: !614, size: 1, offset: 193, flags: DIFlagBitField, extraData: i64 192)
!1139 = !DIDerivedType(tag: DW_TAG_member, name: "rcu", scope: !1127, file: !1122, line: 101, baseType: !129, size: 128, align: 64, offset: 256)
!1140 = !DIDerivedType(tag: DW_TAG_member, name: "ref", scope: !1127, file: !1122, line: 102, baseType: !1135, size: 64, offset: 384)
!1141 = !DIDerivedType(tag: DW_TAG_member, name: "done", scope: !1107, file: !14, line: 130, baseType: !139, size: 256, offset: 576)
!1142 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !1107, file: !14, line: 131, baseType: !13, size: 32, offset: 832)
!1143 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !1107, file: !14, line: 132, baseType: !7, size: 32, offset: 864)
!1144 = !DIDerivedType(tag: DW_TAG_member, name: "vmemmap_shift", scope: !1107, file: !14, line: 133, baseType: !59, size: 64, offset: 896)
!1145 = !DIDerivedType(tag: DW_TAG_member, name: "ops", scope: !1107, file: !14, line: 134, baseType: !1146, size: 64, offset: 960)
!1146 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1147, size: 64)
!1147 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1148)
!1148 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "dev_pagemap_ops", file: !14, line: 78, size: 192, elements: !1149)
!1149 = !{!1150, !1154, !1489}
!1150 = !DIDerivedType(tag: DW_TAG_member, name: "page_free", scope: !1148, file: !14, line: 84, baseType: !1151, size: 64)
!1151 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1152, size: 64)
!1152 = !DISubroutineType(types: !1153)
!1153 = !{null, !1060}
!1154 = !DIDerivedType(tag: DW_TAG_member, name: "migrate_to_ram", scope: !1148, file: !14, line: 90, baseType: !1155, size: 64, offset: 64)
!1155 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1156, size: 64)
!1156 = !DISubroutineType(types: !1157)
!1157 = !{!1158, !1159}
!1158 = !DIDerivedType(tag: DW_TAG_typedef, name: "vm_fault_t", file: !230, line: 1239, baseType: !7)
!1159 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1160, size: 64)
!1160 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "vm_fault", file: !1161, line: 547, size: 896, elements: !1162)
!1161 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/mm.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "1adefb81a835b8052fb1d70b70acf652")
!1162 = !{!1163, !1456, !1457, !1464, !1471, !1481, !1482, !1483, !1485, !1487}
!1163 = !DIDerivedType(tag: DW_TAG_member, scope: !1160, file: !1161, line: 548, baseType: !1164, size: 320)
!1164 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1160, file: !1161, line: 548, size: 320, elements: !1165)
!1165 = !{!1166, !1452, !1453, !1454, !1455}
!1166 = !DIDerivedType(tag: DW_TAG_member, name: "vma", scope: !1164, file: !1161, line: 549, baseType: !1167, size: 64)
!1167 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1168, size: 64)
!1168 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "vm_area_struct", file: !230, line: 667, size: 1280, elements: !1169)
!1169 = !{!1170, !1179, !1355, !1361, !1368, !1369, !1370, !1375, !1380, !1381, !1384, !1445, !1446, !1447, !1448, !1449, !1450}
!1170 = !DIDerivedType(tag: DW_TAG_member, scope: !1168, file: !230, line: 670, baseType: !1171, size: 128)
!1171 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1168, file: !230, line: 670, size: 128, elements: !1172)
!1172 = !{!1173, !1178}
!1173 = !DIDerivedType(tag: DW_TAG_member, scope: !1171, file: !230, line: 671, baseType: !1174, size: 128)
!1174 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1171, file: !230, line: 671, size: 128, elements: !1175)
!1175 = !{!1176, !1177}
!1176 = !DIDerivedType(tag: DW_TAG_member, name: "vm_start", scope: !1174, file: !230, line: 673, baseType: !59, size: 64)
!1177 = !DIDerivedType(tag: DW_TAG_member, name: "vm_end", scope: !1174, file: !230, line: 674, baseType: !59, size: 64, offset: 64)
!1178 = !DIDerivedType(tag: DW_TAG_member, name: "vm_rcu", scope: !1171, file: !230, line: 677, baseType: !129, size: 128, align: 64)
!1179 = !DIDerivedType(tag: DW_TAG_member, name: "vm_mm", scope: !1168, file: !230, line: 685, baseType: !1180, size: 64, offset: 128)
!1180 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1181, size: 64)
!1181 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mm_struct", file: !230, line: 790, size: 10240, elements: !1182)
!1182 = !{!1183, !1351}
!1183 = !DIDerivedType(tag: DW_TAG_member, scope: !1181, file: !230, line: 791, baseType: !1184, size: 10240)
!1184 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1181, file: !230, line: 791, size: 10240, elements: !1185)
!1185 = !{!1186, !1190, !1204, !1205, !1206, !1207, !1208, !1209, !1218, !1219, !1220, !1226, !1227, !1228, !1229, !1230, !1231, !1232, !1233, !1234, !1235, !1236, !1237, !1238, !1239, !1240, !1241, !1242, !1243, !1244, !1245, !1246, !1247, !1248, !1249, !1250, !1251, !1252, !1253, !1254, !1255, !1259, !1261, !1264, !1317, !1318, !1319, !1322, !1323, !1324, !1327, !1328, !1329, !1335, !1336, !1348}
!1186 = !DIDerivedType(tag: DW_TAG_member, scope: !1184, file: !230, line: 796, baseType: !1187, size: 512, align: 512)
!1187 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1184, file: !230, line: 796, size: 512, align: 512, elements: !1188)
!1188 = !{!1189}
!1189 = !DIDerivedType(tag: DW_TAG_member, name: "mm_count", scope: !1187, file: !230, line: 804, baseType: !69, size: 32)
!1190 = !DIDerivedType(tag: DW_TAG_member, name: "mm_mt", scope: !1184, file: !230, line: 807, baseType: !1191, size: 128, offset: 512)
!1191 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "maple_tree", file: !1192, line: 231, size: 128, elements: !1193)
!1192 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/maple_tree.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "ca4fdd29c772470205cf0e3b49ecedd8")
!1193 = !{!1194, !1202, !1203}
!1194 = !DIDerivedType(tag: DW_TAG_member, scope: !1191, file: !1192, line: 232, baseType: !1195, size: 32)
!1195 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1191, file: !1192, line: 232, size: 32, elements: !1196)
!1196 = !{!1197, !1198}
!1197 = !DIDerivedType(tag: DW_TAG_member, name: "ma_lock", scope: !1195, file: !1192, line: 233, baseType: !79, size: 32)
!1198 = !DIDerivedType(tag: DW_TAG_member, name: "ma_external_lock", scope: !1195, file: !1192, line: 234, baseType: !1199)
!1199 = !DIDerivedType(tag: DW_TAG_typedef, name: "lockdep_map_p", file: !1192, line: 210, baseType: !1200)
!1200 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !1192, line: 210, elements: !1201)
!1201 = !{}
!1202 = !DIDerivedType(tag: DW_TAG_member, name: "ma_flags", scope: !1191, file: !1192, line: 236, baseType: !7, size: 32, offset: 32)
!1203 = !DIDerivedType(tag: DW_TAG_member, name: "ma_root", scope: !1191, file: !1192, line: 237, baseType: !40, size: 64, offset: 64)
!1204 = !DIDerivedType(tag: DW_TAG_member, name: "mmap_base", scope: !1184, file: !230, line: 809, baseType: !59, size: 64, offset: 640)
!1205 = !DIDerivedType(tag: DW_TAG_member, name: "mmap_legacy_base", scope: !1184, file: !230, line: 810, baseType: !59, size: 64, offset: 704)
!1206 = !DIDerivedType(tag: DW_TAG_member, name: "mmap_compat_base", scope: !1184, file: !230, line: 813, baseType: !59, size: 64, offset: 768)
!1207 = !DIDerivedType(tag: DW_TAG_member, name: "mmap_compat_legacy_base", scope: !1184, file: !230, line: 814, baseType: !59, size: 64, offset: 832)
!1208 = !DIDerivedType(tag: DW_TAG_member, name: "task_size", scope: !1184, file: !230, line: 816, baseType: !59, size: 64, offset: 896)
!1209 = !DIDerivedType(tag: DW_TAG_member, name: "pgd", scope: !1184, file: !230, line: 817, baseType: !1210, size: 64, offset: 960)
!1210 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1211, size: 64)
!1211 = !DIDerivedType(tag: DW_TAG_typedef, name: "pgd_t", file: !1212, line: 295, baseType: !1213)
!1212 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/pgtable_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "fa70448799c4b7e6d8b1cbb1b03f881f")
!1213 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !1212, line: 295, size: 64, elements: !1214)
!1214 = !{!1215}
!1215 = !DIDerivedType(tag: DW_TAG_member, name: "pgd", scope: !1213, file: !1212, line: 295, baseType: !1216, size: 64)
!1216 = !DIDerivedType(tag: DW_TAG_typedef, name: "pgdval_t", file: !1217, line: 18, baseType: !59)
!1217 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/pgtable_64_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "a02c122f69dfa51c1847c70227060031")
!1218 = !DIDerivedType(tag: DW_TAG_member, name: "membarrier_state", scope: !1184, file: !230, line: 826, baseType: !69, size: 32, offset: 1024)
!1219 = !DIDerivedType(tag: DW_TAG_member, name: "mm_users", scope: !1184, file: !230, line: 838, baseType: !69, size: 32, offset: 1056)
!1220 = !DIDerivedType(tag: DW_TAG_member, name: "pcpu_cid", scope: !1184, file: !230, line: 848, baseType: !1221, size: 64, offset: 1088)
!1221 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1222, size: 64)
!1222 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mm_cid", file: !230, line: 782, size: 128, elements: !1223)
!1223 = !{!1224, !1225}
!1224 = !DIDerivedType(tag: DW_TAG_member, name: "time", scope: !1222, file: !230, line: 783, baseType: !519, size: 64)
!1225 = !DIDerivedType(tag: DW_TAG_member, name: "cid", scope: !1222, file: !230, line: 784, baseType: !42, size: 32, offset: 64)
!1226 = !DIDerivedType(tag: DW_TAG_member, name: "mm_cid_next_scan", scope: !1184, file: !230, line: 854, baseType: !59, size: 64, offset: 1152)
!1227 = !DIDerivedType(tag: DW_TAG_member, name: "pgtables_bytes", scope: !1184, file: !230, line: 857, baseType: !496, size: 64, offset: 1216)
!1228 = !DIDerivedType(tag: DW_TAG_member, name: "map_count", scope: !1184, file: !230, line: 859, baseType: !42, size: 32, offset: 1280)
!1229 = !DIDerivedType(tag: DW_TAG_member, name: "page_table_lock", scope: !1184, file: !230, line: 861, baseType: !79, size: 32, offset: 1312)
!1230 = !DIDerivedType(tag: DW_TAG_member, name: "mmap_lock", scope: !1184, file: !230, line: 876, baseType: !549, size: 320, offset: 1344)
!1231 = !DIDerivedType(tag: DW_TAG_member, name: "mmlist", scope: !1184, file: !230, line: 878, baseType: !117, size: 128, offset: 1664)
!1232 = !DIDerivedType(tag: DW_TAG_member, name: "mm_lock_seq", scope: !1184, file: !230, line: 898, baseType: !42, size: 32, offset: 1792)
!1233 = !DIDerivedType(tag: DW_TAG_member, name: "hiwater_rss", scope: !1184, file: !230, line: 902, baseType: !59, size: 64, offset: 1856)
!1234 = !DIDerivedType(tag: DW_TAG_member, name: "hiwater_vm", scope: !1184, file: !230, line: 903, baseType: !59, size: 64, offset: 1920)
!1235 = !DIDerivedType(tag: DW_TAG_member, name: "total_vm", scope: !1184, file: !230, line: 905, baseType: !59, size: 64, offset: 1984)
!1236 = !DIDerivedType(tag: DW_TAG_member, name: "locked_vm", scope: !1184, file: !230, line: 906, baseType: !59, size: 64, offset: 2048)
!1237 = !DIDerivedType(tag: DW_TAG_member, name: "pinned_vm", scope: !1184, file: !230, line: 907, baseType: !498, size: 64, offset: 2112)
!1238 = !DIDerivedType(tag: DW_TAG_member, name: "data_vm", scope: !1184, file: !230, line: 908, baseType: !59, size: 64, offset: 2176)
!1239 = !DIDerivedType(tag: DW_TAG_member, name: "exec_vm", scope: !1184, file: !230, line: 909, baseType: !59, size: 64, offset: 2240)
!1240 = !DIDerivedType(tag: DW_TAG_member, name: "stack_vm", scope: !1184, file: !230, line: 910, baseType: !59, size: 64, offset: 2304)
!1241 = !DIDerivedType(tag: DW_TAG_member, name: "def_flags", scope: !1184, file: !230, line: 911, baseType: !59, size: 64, offset: 2368)
!1242 = !DIDerivedType(tag: DW_TAG_member, name: "write_protect_seq", scope: !1184, file: !230, line: 918, baseType: !750, size: 32, offset: 2432)
!1243 = !DIDerivedType(tag: DW_TAG_member, name: "arg_lock", scope: !1184, file: !230, line: 920, baseType: !79, size: 32, offset: 2464)
!1244 = !DIDerivedType(tag: DW_TAG_member, name: "start_code", scope: !1184, file: !230, line: 922, baseType: !59, size: 64, offset: 2496)
!1245 = !DIDerivedType(tag: DW_TAG_member, name: "end_code", scope: !1184, file: !230, line: 922, baseType: !59, size: 64, offset: 2560)
!1246 = !DIDerivedType(tag: DW_TAG_member, name: "start_data", scope: !1184, file: !230, line: 922, baseType: !59, size: 64, offset: 2624)
!1247 = !DIDerivedType(tag: DW_TAG_member, name: "end_data", scope: !1184, file: !230, line: 922, baseType: !59, size: 64, offset: 2688)
!1248 = !DIDerivedType(tag: DW_TAG_member, name: "start_brk", scope: !1184, file: !230, line: 923, baseType: !59, size: 64, offset: 2752)
!1249 = !DIDerivedType(tag: DW_TAG_member, name: "brk", scope: !1184, file: !230, line: 923, baseType: !59, size: 64, offset: 2816)
!1250 = !DIDerivedType(tag: DW_TAG_member, name: "start_stack", scope: !1184, file: !230, line: 923, baseType: !59, size: 64, offset: 2880)
!1251 = !DIDerivedType(tag: DW_TAG_member, name: "arg_start", scope: !1184, file: !230, line: 924, baseType: !59, size: 64, offset: 2944)
!1252 = !DIDerivedType(tag: DW_TAG_member, name: "arg_end", scope: !1184, file: !230, line: 924, baseType: !59, size: 64, offset: 3008)
!1253 = !DIDerivedType(tag: DW_TAG_member, name: "env_start", scope: !1184, file: !230, line: 924, baseType: !59, size: 64, offset: 3072)
!1254 = !DIDerivedType(tag: DW_TAG_member, name: "env_end", scope: !1184, file: !230, line: 924, baseType: !59, size: 64, offset: 3136)
!1255 = !DIDerivedType(tag: DW_TAG_member, name: "saved_auxv", scope: !1184, file: !230, line: 926, baseType: !1256, size: 3328, offset: 3200)
!1256 = !DICompositeType(tag: DW_TAG_array_type, baseType: !59, size: 3328, elements: !1257)
!1257 = !{!1258}
!1258 = !DISubrange(count: 52)
!1259 = !DIDerivedType(tag: DW_TAG_member, name: "rss_stat", scope: !1184, file: !230, line: 928, baseType: !1260, size: 1280, offset: 6528)
!1260 = !DICompositeType(tag: DW_TAG_array_type, baseType: !675, size: 1280, elements: !635)
!1261 = !DIDerivedType(tag: DW_TAG_member, name: "binfmt", scope: !1184, file: !230, line: 930, baseType: !1262, size: 64, offset: 7808)
!1262 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1263, size: 64)
!1263 = !DICompositeType(tag: DW_TAG_structure_type, name: "linux_binfmt", file: !230, line: 930, flags: DIFlagFwdDecl)
!1264 = !DIDerivedType(tag: DW_TAG_member, name: "context", scope: !1184, file: !230, line: 933, baseType: !1265, size: 1024, offset: 7872)
!1265 = !DIDerivedType(tag: DW_TAG_typedef, name: "mm_context_t", file: !1266, line: 70, baseType: !1267)
!1266 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/mmu.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "8432775e76586b6026e9942903b541a7")
!1267 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !1266, line: 23, size: 1024, elements: !1268)
!1268 = !{!1269, !1270, !1271, !1272, !1275, !1276, !1284, !1285, !1312, !1313, !1314}
!1269 = !DIDerivedType(tag: DW_TAG_member, name: "ctx_id", scope: !1267, file: !1266, line: 28, baseType: !519, size: 64)
!1270 = !DIDerivedType(tag: DW_TAG_member, name: "tlb_gen", scope: !1267, file: !1266, line: 38, baseType: !498, size: 64, offset: 64)
!1271 = !DIDerivedType(tag: DW_TAG_member, name: "ldt_usr_sem", scope: !1267, file: !1266, line: 41, baseType: !549, size: 320, offset: 128)
!1272 = !DIDerivedType(tag: DW_TAG_member, name: "ldt", scope: !1267, file: !1266, line: 42, baseType: !1273, size: 64, offset: 448)
!1273 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1274, size: 64)
!1274 = !DICompositeType(tag: DW_TAG_structure_type, name: "ldt_struct", file: !1266, line: 42, flags: DIFlagFwdDecl)
!1275 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !1267, file: !1266, line: 46, baseType: !59, size: 64, offset: 512)
!1276 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !1267, file: !1266, line: 57, baseType: !1277, size: 256, offset: 576)
!1277 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mutex", file: !1278, line: 41, size: 256, elements: !1279)
!1278 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/mutex_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "8aeefdc3dd3f7226710df2fc51a2f723")
!1279 = !{!1280, !1281, !1282, !1283}
!1280 = !DIDerivedType(tag: DW_TAG_member, name: "owner", scope: !1277, file: !1278, line: 42, baseType: !496, size: 64)
!1281 = !DIDerivedType(tag: DW_TAG_member, name: "wait_lock", scope: !1277, file: !1278, line: 43, baseType: !148, size: 32, offset: 64)
!1282 = !DIDerivedType(tag: DW_TAG_member, name: "osq", scope: !1277, file: !1278, line: 45, baseType: !555, size: 32, offset: 96)
!1283 = !DIDerivedType(tag: DW_TAG_member, name: "wait_list", scope: !1277, file: !1278, line: 47, baseType: !117, size: 128, offset: 128)
!1284 = !DIDerivedType(tag: DW_TAG_member, name: "vdso", scope: !1267, file: !1266, line: 58, baseType: !40, size: 64, offset: 832)
!1285 = !DIDerivedType(tag: DW_TAG_member, name: "vdso_image", scope: !1267, file: !1266, line: 59, baseType: !1286, size: 64, offset: 896)
!1286 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1287, size: 64)
!1287 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1288)
!1288 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "vdso_image", file: !1289, line: 13, size: 1216, elements: !1290)
!1289 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/vdso.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "60d57dd5659a9806794848b1eaed4bc5")
!1290 = !{!1291, !1292, !1293, !1294, !1295, !1296, !1297, !1300, !1301, !1302, !1303, !1304, !1305, !1306, !1307, !1308, !1309, !1310, !1311}
!1291 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !1288, file: !1289, line: 14, baseType: !40, size: 64)
!1292 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !1288, file: !1289, line: 15, baseType: !59, size: 64, offset: 64)
!1293 = !DIDerivedType(tag: DW_TAG_member, name: "alt", scope: !1288, file: !1289, line: 17, baseType: !59, size: 64, offset: 128)
!1294 = !DIDerivedType(tag: DW_TAG_member, name: "alt_len", scope: !1288, file: !1289, line: 17, baseType: !59, size: 64, offset: 192)
!1295 = !DIDerivedType(tag: DW_TAG_member, name: "extable_base", scope: !1288, file: !1289, line: 18, baseType: !59, size: 64, offset: 256)
!1296 = !DIDerivedType(tag: DW_TAG_member, name: "extable_len", scope: !1288, file: !1289, line: 18, baseType: !59, size: 64, offset: 320)
!1297 = !DIDerivedType(tag: DW_TAG_member, name: "extable", scope: !1288, file: !1289, line: 19, baseType: !1298, size: 64, offset: 384)
!1298 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1299, size: 64)
!1299 = !DIDerivedType(tag: DW_TAG_const_type, baseType: null)
!1300 = !DIDerivedType(tag: DW_TAG_member, name: "sym_vvar_start", scope: !1288, file: !1289, line: 21, baseType: !892, size: 64, offset: 448)
!1301 = !DIDerivedType(tag: DW_TAG_member, name: "sym_vvar_page", scope: !1288, file: !1289, line: 23, baseType: !892, size: 64, offset: 512)
!1302 = !DIDerivedType(tag: DW_TAG_member, name: "sym_pvclock_page", scope: !1288, file: !1289, line: 24, baseType: !892, size: 64, offset: 576)
!1303 = !DIDerivedType(tag: DW_TAG_member, name: "sym_hvclock_page", scope: !1288, file: !1289, line: 25, baseType: !892, size: 64, offset: 640)
!1304 = !DIDerivedType(tag: DW_TAG_member, name: "sym_timens_page", scope: !1288, file: !1289, line: 26, baseType: !892, size: 64, offset: 704)
!1305 = !DIDerivedType(tag: DW_TAG_member, name: "sym_VDSO32_NOTE_MASK", scope: !1288, file: !1289, line: 27, baseType: !892, size: 64, offset: 768)
!1306 = !DIDerivedType(tag: DW_TAG_member, name: "sym___kernel_sigreturn", scope: !1288, file: !1289, line: 28, baseType: !892, size: 64, offset: 832)
!1307 = !DIDerivedType(tag: DW_TAG_member, name: "sym___kernel_rt_sigreturn", scope: !1288, file: !1289, line: 29, baseType: !892, size: 64, offset: 896)
!1308 = !DIDerivedType(tag: DW_TAG_member, name: "sym___kernel_vsyscall", scope: !1288, file: !1289, line: 30, baseType: !892, size: 64, offset: 960)
!1309 = !DIDerivedType(tag: DW_TAG_member, name: "sym_int80_landing_pad", scope: !1288, file: !1289, line: 31, baseType: !892, size: 64, offset: 1024)
!1310 = !DIDerivedType(tag: DW_TAG_member, name: "sym_vdso32_sigreturn_landing_pad", scope: !1288, file: !1289, line: 32, baseType: !892, size: 64, offset: 1088)
!1311 = !DIDerivedType(tag: DW_TAG_member, name: "sym_vdso32_rt_sigreturn_landing_pad", scope: !1288, file: !1289, line: 33, baseType: !892, size: 64, offset: 1152)
!1312 = !DIDerivedType(tag: DW_TAG_member, name: "perf_rdpmc_allowed", scope: !1267, file: !1266, line: 61, baseType: !69, size: 32, offset: 960)
!1313 = !DIDerivedType(tag: DW_TAG_member, name: "pkey_allocation_map", scope: !1267, file: !1266, line: 67, baseType: !113, size: 16, offset: 992)
!1314 = !DIDerivedType(tag: DW_TAG_member, name: "execute_only_pkey", scope: !1267, file: !1266, line: 68, baseType: !1315, size: 16, offset: 1008)
!1315 = !DIDerivedType(tag: DW_TAG_typedef, name: "s16", file: !104, line: 18, baseType: !1316)
!1316 = !DIDerivedType(tag: DW_TAG_typedef, name: "__s16", file: !106, line: 23, baseType: !583)
!1317 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !1184, file: !230, line: 935, baseType: !59, size: 64, offset: 8896)
!1318 = !DIDerivedType(tag: DW_TAG_member, name: "ioctx_lock", scope: !1184, file: !230, line: 938, baseType: !79, size: 32, offset: 8960)
!1319 = !DIDerivedType(tag: DW_TAG_member, name: "ioctx_table", scope: !1184, file: !230, line: 939, baseType: !1320, size: 64, offset: 9024)
!1320 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1321, size: 64)
!1321 = !DICompositeType(tag: DW_TAG_structure_type, name: "kioctx_table", file: !230, line: 788, flags: DIFlagFwdDecl)
!1322 = !DIDerivedType(tag: DW_TAG_member, name: "user_ns", scope: !1184, file: !230, line: 954, baseType: !700, size: 64, offset: 9088)
!1323 = !DIDerivedType(tag: DW_TAG_member, name: "exe_file", scope: !1184, file: !230, line: 957, baseType: !896, size: 64, offset: 9152)
!1324 = !DIDerivedType(tag: DW_TAG_member, name: "notifier_subscriptions", scope: !1184, file: !230, line: 959, baseType: !1325, size: 64, offset: 9216)
!1325 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1326, size: 64)
!1326 = !DICompositeType(tag: DW_TAG_structure_type, name: "mmu_notifier_subscriptions", file: !230, line: 959, flags: DIFlagFwdDecl)
!1327 = !DIDerivedType(tag: DW_TAG_member, name: "tlb_flush_pending", scope: !1184, file: !230, line: 983, baseType: !69, size: 32, offset: 9280)
!1328 = !DIDerivedType(tag: DW_TAG_member, name: "tlb_flush_batched", scope: !1184, file: !230, line: 986, baseType: !69, size: 32, offset: 9312)
!1329 = !DIDerivedType(tag: DW_TAG_member, name: "uprobes_state", scope: !1184, file: !230, line: 988, baseType: !1330, size: 64, offset: 9344)
!1330 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "uprobes_state", file: !327, line: 104, size: 64, elements: !1331)
!1331 = !{!1332}
!1332 = !DIDerivedType(tag: DW_TAG_member, name: "xol_area", scope: !1330, file: !327, line: 105, baseType: !1333, size: 64)
!1333 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1334, size: 64)
!1334 = !DICompositeType(tag: DW_TAG_structure_type, name: "xol_area", file: !327, line: 102, flags: DIFlagFwdDecl)
!1335 = !DIDerivedType(tag: DW_TAG_member, name: "hugetlb_usage", scope: !1184, file: !230, line: 993, baseType: !496, size: 64, offset: 9408)
!1336 = !DIDerivedType(tag: DW_TAG_member, name: "async_put_work", scope: !1184, file: !230, line: 995, baseType: !1337, size: 256, offset: 9472)
!1337 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "work_struct", file: !1338, line: 16, size: 256, elements: !1339)
!1338 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/workqueue_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "16c68574dd11dc9ce84d23c41ac231f3")
!1339 = !{!1340, !1341, !1342}
!1340 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !1337, file: !1338, line: 17, baseType: !496, size: 64)
!1341 = !DIDerivedType(tag: DW_TAG_member, name: "entry", scope: !1337, file: !1338, line: 18, baseType: !117, size: 128, offset: 64)
!1342 = !DIDerivedType(tag: DW_TAG_member, name: "func", scope: !1337, file: !1338, line: 19, baseType: !1343, size: 64, offset: 192)
!1343 = !DIDerivedType(tag: DW_TAG_typedef, name: "work_func_t", file: !1338, line: 13, baseType: !1344)
!1344 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1345, size: 64)
!1345 = !DISubroutineType(types: !1346)
!1346 = !{null, !1347}
!1347 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1337, size: 64)
!1348 = !DIDerivedType(tag: DW_TAG_member, name: "iommu_mm", scope: !1184, file: !230, line: 998, baseType: !1349, size: 64, offset: 9728)
!1349 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1350, size: 64)
!1350 = !DICompositeType(tag: DW_TAG_structure_type, name: "iommu_mm_data", file: !230, line: 789, flags: DIFlagFwdDecl)
!1351 = !DIDerivedType(tag: DW_TAG_member, name: "cpu_bitmap", scope: !1181, file: !230, line: 1039, baseType: !1352, offset: 10240)
!1352 = !DICompositeType(tag: DW_TAG_array_type, baseType: !59, elements: !1353)
!1353 = !{!1354}
!1354 = !DISubrange(count: -1)
!1355 = !DIDerivedType(tag: DW_TAG_member, name: "vm_page_prot", scope: !1168, file: !230, line: 686, baseType: !1356, size: 64, offset: 192)
!1356 = !DIDerivedType(tag: DW_TAG_typedef, name: "pgprot_t", file: !1212, line: 293, baseType: !1357)
!1357 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "pgprot", file: !1212, line: 293, size: 64, elements: !1358)
!1358 = !{!1359}
!1359 = !DIDerivedType(tag: DW_TAG_member, name: "pgprot", scope: !1357, file: !1212, line: 293, baseType: !1360, size: 64)
!1360 = !DIDerivedType(tag: DW_TAG_typedef, name: "pgprotval_t", file: !1217, line: 19, baseType: !59)
!1361 = !DIDerivedType(tag: DW_TAG_member, scope: !1168, file: !230, line: 692, baseType: !1362, size: 64, offset: 256)
!1362 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1168, file: !230, line: 692, size: 64, elements: !1363)
!1363 = !{!1364, !1367}
!1364 = !DIDerivedType(tag: DW_TAG_member, name: "vm_flags", scope: !1362, file: !230, line: 693, baseType: !1365, size: 64)
!1365 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1366)
!1366 = !DIDerivedType(tag: DW_TAG_typedef, name: "vm_flags_t", file: !230, line: 560, baseType: !59)
!1367 = !DIDerivedType(tag: DW_TAG_member, name: "__vm_flags", scope: !1362, file: !230, line: 694, baseType: !1366, size: 64)
!1368 = !DIDerivedType(tag: DW_TAG_member, name: "detached", scope: !1168, file: !230, line: 702, baseType: !614, size: 8, offset: 320)
!1369 = !DIDerivedType(tag: DW_TAG_member, name: "vm_lock_seq", scope: !1168, file: !230, line: 718, baseType: !42, size: 32, offset: 352)
!1370 = !DIDerivedType(tag: DW_TAG_member, name: "vm_lock", scope: !1168, file: !230, line: 720, baseType: !1371, size: 64, offset: 384)
!1371 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1372, size: 64)
!1372 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "vma_lock", file: !230, line: 618, size: 320, elements: !1373)
!1373 = !{!1374}
!1374 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !1372, file: !230, line: 619, baseType: !549, size: 320)
!1375 = !DIDerivedType(tag: DW_TAG_member, name: "shared", scope: !1168, file: !230, line: 731, baseType: !1376, size: 256, offset: 448)
!1376 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1168, file: !230, line: 728, size: 256, elements: !1377)
!1377 = !{!1378, !1379}
!1378 = !DIDerivedType(tag: DW_TAG_member, name: "rb", scope: !1376, file: !230, line: 729, baseType: !173, size: 192, align: 64)
!1379 = !DIDerivedType(tag: DW_TAG_member, name: "rb_subtree_last", scope: !1376, file: !230, line: 730, baseType: !59, size: 64, offset: 192)
!1380 = !DIDerivedType(tag: DW_TAG_member, name: "anon_vma_chain", scope: !1168, file: !230, line: 739, baseType: !117, size: 128, offset: 704)
!1381 = !DIDerivedType(tag: DW_TAG_member, name: "anon_vma", scope: !1168, file: !230, line: 741, baseType: !1382, size: 64, offset: 832)
!1382 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1383, size: 64)
!1383 = !DICompositeType(tag: DW_TAG_structure_type, name: "anon_vma", file: !230, line: 741, flags: DIFlagFwdDecl)
!1384 = !DIDerivedType(tag: DW_TAG_member, name: "vm_ops", scope: !1168, file: !230, line: 744, baseType: !1385, size: 64, offset: 896)
!1385 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1386, size: 64)
!1386 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1387)
!1387 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "vm_operations_struct", file: !1161, line: 598, size: 1024, elements: !1388)
!1388 = !{!1389, !1393, !1394, !1398, !1402, !1406, !1407, !1411, !1415, !1419, !1420, !1421, !1425, !1429, !1436, !1441}
!1389 = !DIDerivedType(tag: DW_TAG_member, name: "open", scope: !1387, file: !1161, line: 599, baseType: !1390, size: 64)
!1390 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1391, size: 64)
!1391 = !DISubroutineType(types: !1392)
!1392 = !{null, !1167}
!1393 = !DIDerivedType(tag: DW_TAG_member, name: "close", scope: !1387, file: !1161, line: 604, baseType: !1390, size: 64, offset: 64)
!1394 = !DIDerivedType(tag: DW_TAG_member, name: "may_split", scope: !1387, file: !1161, line: 606, baseType: !1395, size: 64, offset: 128)
!1395 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1396, size: 64)
!1396 = !DISubroutineType(types: !1397)
!1397 = !{!42, !1167, !59}
!1398 = !DIDerivedType(tag: DW_TAG_member, name: "mremap", scope: !1387, file: !1161, line: 607, baseType: !1399, size: 64, offset: 192)
!1399 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1400, size: 64)
!1400 = !DISubroutineType(types: !1401)
!1401 = !{!42, !1167}
!1402 = !DIDerivedType(tag: DW_TAG_member, name: "mprotect", scope: !1387, file: !1161, line: 613, baseType: !1403, size: 64, offset: 256)
!1403 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1404, size: 64)
!1404 = !DISubroutineType(types: !1405)
!1405 = !{!42, !1167, !59, !59, !59}
!1406 = !DIDerivedType(tag: DW_TAG_member, name: "fault", scope: !1387, file: !1161, line: 615, baseType: !1155, size: 64, offset: 320)
!1407 = !DIDerivedType(tag: DW_TAG_member, name: "huge_fault", scope: !1387, file: !1161, line: 616, baseType: !1408, size: 64, offset: 384)
!1408 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1409, size: 64)
!1409 = !DISubroutineType(types: !1410)
!1410 = !{!1158, !1159, !7}
!1411 = !DIDerivedType(tag: DW_TAG_member, name: "map_pages", scope: !1387, file: !1161, line: 617, baseType: !1412, size: 64, offset: 448)
!1412 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1413, size: 64)
!1413 = !DISubroutineType(types: !1414)
!1414 = !{!1158, !1159, !59, !59}
!1415 = !DIDerivedType(tag: DW_TAG_member, name: "pagesize", scope: !1387, file: !1161, line: 619, baseType: !1416, size: 64, offset: 512)
!1416 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1417, size: 64)
!1417 = !DISubroutineType(types: !1418)
!1418 = !{!59, !1167}
!1419 = !DIDerivedType(tag: DW_TAG_member, name: "page_mkwrite", scope: !1387, file: !1161, line: 623, baseType: !1155, size: 64, offset: 576)
!1420 = !DIDerivedType(tag: DW_TAG_member, name: "pfn_mkwrite", scope: !1387, file: !1161, line: 626, baseType: !1155, size: 64, offset: 640)
!1421 = !DIDerivedType(tag: DW_TAG_member, name: "access", scope: !1387, file: !1161, line: 632, baseType: !1422, size: 64, offset: 704)
!1422 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1423, size: 64)
!1423 = !DISubroutineType(types: !1424)
!1424 = !{!42, !1167, !59, !40, !42, !42}
!1425 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !1387, file: !1161, line: 638, baseType: !1426, size: 64, offset: 768)
!1426 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1427, size: 64)
!1427 = !DISubroutineType(types: !1428)
!1428 = !{!36, !1167}
!1429 = !DIDerivedType(tag: DW_TAG_member, name: "set_policy", scope: !1387, file: !1161, line: 648, baseType: !1430, size: 64, offset: 832)
!1430 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1431, size: 64)
!1431 = !DISubroutineType(types: !1432)
!1432 = !{!42, !1167, !1433}
!1433 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1434, size: 64)
!1434 = !DICompositeType(tag: DW_TAG_structure_type, name: "mempolicy", file: !1435, line: 64, flags: DIFlagFwdDecl)
!1435 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/sched.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "76cde85c5a4d823c8471d2e918602e9d")
!1436 = !DIDerivedType(tag: DW_TAG_member, name: "get_policy", scope: !1387, file: !1161, line: 660, baseType: !1437, size: 64, offset: 896)
!1437 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1438, size: 64)
!1438 = !DISubroutineType(types: !1439)
!1439 = !{!1433, !1167, !59, !1440}
!1440 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !59, size: 64)
!1441 = !DIDerivedType(tag: DW_TAG_member, name: "find_special_page", scope: !1387, file: !1161, line: 668, baseType: !1442, size: 64, offset: 960)
!1442 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1443, size: 64)
!1443 = !DISubroutineType(types: !1444)
!1444 = !{!1060, !1167, !59}
!1445 = !DIDerivedType(tag: DW_TAG_member, name: "vm_pgoff", scope: !1168, file: !230, line: 747, baseType: !59, size: 64, offset: 960)
!1446 = !DIDerivedType(tag: DW_TAG_member, name: "vm_file", scope: !1168, file: !230, line: 749, baseType: !896, size: 64, offset: 1024)
!1447 = !DIDerivedType(tag: DW_TAG_member, name: "vm_private_data", scope: !1168, file: !230, line: 750, baseType: !40, size: 64, offset: 1088)
!1448 = !DIDerivedType(tag: DW_TAG_member, name: "swap_readahead_info", scope: !1168, file: !230, line: 761, baseType: !496, size: 64, offset: 1152)
!1449 = !DIDerivedType(tag: DW_TAG_member, name: "vm_policy", scope: !1168, file: !230, line: 767, baseType: !1433, size: 64, offset: 1216)
!1450 = !DIDerivedType(tag: DW_TAG_member, name: "vm_userfaultfd_ctx", scope: !1168, file: !230, line: 772, baseType: !1451, offset: 1280)
!1451 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "vm_userfaultfd_ctx", file: !230, line: 588, elements: !1201)
!1452 = !DIDerivedType(tag: DW_TAG_member, name: "gfp_mask", scope: !1164, file: !1161, line: 550, baseType: !488, size: 32, offset: 64)
!1453 = !DIDerivedType(tag: DW_TAG_member, name: "pgoff", scope: !1164, file: !1161, line: 551, baseType: !59, size: 64, offset: 128)
!1454 = !DIDerivedType(tag: DW_TAG_member, name: "address", scope: !1164, file: !1161, line: 552, baseType: !59, size: 64, offset: 192)
!1455 = !DIDerivedType(tag: DW_TAG_member, name: "real_address", scope: !1164, file: !1161, line: 553, baseType: !59, size: 64, offset: 256)
!1456 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !1160, file: !1161, line: 555, baseType: !229, size: 32, offset: 320)
!1457 = !DIDerivedType(tag: DW_TAG_member, name: "pmd", scope: !1160, file: !1161, line: 557, baseType: !1458, size: 64, offset: 384)
!1458 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1459, size: 64)
!1459 = !DIDerivedType(tag: DW_TAG_typedef, name: "pmd_t", file: !1217, line: 22, baseType: !1460)
!1460 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !1217, line: 22, size: 64, elements: !1461)
!1461 = !{!1462}
!1462 = !DIDerivedType(tag: DW_TAG_member, name: "pmd", scope: !1460, file: !1217, line: 22, baseType: !1463, size: 64)
!1463 = !DIDerivedType(tag: DW_TAG_typedef, name: "pmdval_t", file: !1217, line: 15, baseType: !59)
!1464 = !DIDerivedType(tag: DW_TAG_member, name: "pud", scope: !1160, file: !1161, line: 559, baseType: !1465, size: 64, offset: 448)
!1465 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1466, size: 64)
!1466 = !DIDerivedType(tag: DW_TAG_typedef, name: "pud_t", file: !1212, line: 368, baseType: !1467)
!1467 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !1212, line: 368, size: 64, elements: !1468)
!1468 = !{!1469}
!1469 = !DIDerivedType(tag: DW_TAG_member, name: "pud", scope: !1467, file: !1212, line: 368, baseType: !1470, size: 64)
!1470 = !DIDerivedType(tag: DW_TAG_typedef, name: "pudval_t", file: !1217, line: 16, baseType: !59)
!1471 = !DIDerivedType(tag: DW_TAG_member, scope: !1160, file: !1161, line: 562, baseType: !1472, size: 64, offset: 512)
!1472 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1160, file: !1161, line: 562, size: 64, elements: !1473)
!1473 = !{!1474, !1480}
!1474 = !DIDerivedType(tag: DW_TAG_member, name: "orig_pte", scope: !1472, file: !1161, line: 563, baseType: !1475, size: 64)
!1475 = !DIDerivedType(tag: DW_TAG_typedef, name: "pte_t", file: !1217, line: 21, baseType: !1476)
!1476 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !1217, line: 21, size: 64, elements: !1477)
!1477 = !{!1478}
!1478 = !DIDerivedType(tag: DW_TAG_member, name: "pte", scope: !1476, file: !1217, line: 21, baseType: !1479, size: 64)
!1479 = !DIDerivedType(tag: DW_TAG_typedef, name: "pteval_t", file: !1217, line: 14, baseType: !59)
!1480 = !DIDerivedType(tag: DW_TAG_member, name: "orig_pmd", scope: !1472, file: !1161, line: 564, baseType: !1459, size: 64)
!1481 = !DIDerivedType(tag: DW_TAG_member, name: "cow_page", scope: !1160, file: !1161, line: 569, baseType: !1060, size: 64, offset: 576)
!1482 = !DIDerivedType(tag: DW_TAG_member, name: "page", scope: !1160, file: !1161, line: 570, baseType: !1060, size: 64, offset: 640)
!1483 = !DIDerivedType(tag: DW_TAG_member, name: "pte", scope: !1160, file: !1161, line: 576, baseType: !1484, size: 64, offset: 704)
!1484 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1475, size: 64)
!1485 = !DIDerivedType(tag: DW_TAG_member, name: "ptl", scope: !1160, file: !1161, line: 580, baseType: !1486, size: 64, offset: 768)
!1486 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !79, size: 64)
!1487 = !DIDerivedType(tag: DW_TAG_member, name: "prealloc_pte", scope: !1160, file: !1161, line: 584, baseType: !1488, size: 64, offset: 832)
!1488 = !DIDerivedType(tag: DW_TAG_typedef, name: "pgtable_t", file: !1212, line: 516, baseType: !1060)
!1489 = !DIDerivedType(tag: DW_TAG_member, name: "memory_failure", scope: !1148, file: !14, line: 101, baseType: !1490, size: 64, offset: 128)
!1490 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1491, size: 64)
!1491 = !DISubroutineType(types: !1492)
!1492 = !{!42, !1106, !59, !59, !42}
!1493 = !DIDerivedType(tag: DW_TAG_member, name: "owner", scope: !1107, file: !14, line: 135, baseType: !40, size: 64, offset: 1024)
!1494 = !DIDerivedType(tag: DW_TAG_member, name: "nr_range", scope: !1107, file: !14, line: 136, baseType: !42, size: 32, offset: 1088)
!1495 = !DIDerivedType(tag: DW_TAG_member, scope: !1107, file: !14, line: 137, baseType: !1496, size: 128, offset: 1152)
!1496 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1107, file: !14, line: 137, size: 128, elements: !1497)
!1497 = !{!1498, !1504}
!1498 = !DIDerivedType(tag: DW_TAG_member, name: "range", scope: !1496, file: !14, line: 138, baseType: !1499, size: 128)
!1499 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "range", file: !1500, line: 6, size: 128, elements: !1501)
!1500 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/range.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "0d4ed130b9634d4ea80b5f843ace4d76")
!1501 = !{!1502, !1503}
!1502 = !DIDerivedType(tag: DW_TAG_member, name: "start", scope: !1499, file: !1500, line: 7, baseType: !519, size: 64)
!1503 = !DIDerivedType(tag: DW_TAG_member, name: "end", scope: !1499, file: !1500, line: 8, baseType: !519, size: 64, offset: 64)
!1504 = !DIDerivedType(tag: DW_TAG_member, scope: !1496, file: !14, line: 139, baseType: !1505)
!1505 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1496, file: !14, line: 139, elements: !1506)
!1506 = !{!1507, !1509}
!1507 = !DIDerivedType(tag: DW_TAG_member, name: "__empty_ranges", scope: !1505, file: !14, line: 139, baseType: !1508)
!1508 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1505, file: !14, line: 139, elements: !1201)
!1509 = !DIDerivedType(tag: DW_TAG_member, name: "ranges", scope: !1505, file: !14, line: 139, baseType: !1510)
!1510 = !DICompositeType(tag: DW_TAG_array_type, baseType: !1499, elements: !1353)
!1511 = !DIDerivedType(tag: DW_TAG_member, name: "zone_device_data", scope: !1103, file: !230, line: 134, baseType: !40, size: 64, offset: 64)
!1512 = !DIDerivedType(tag: DW_TAG_member, name: "callback_head", scope: !1065, file: !230, line: 148, baseType: !129, size: 128, align: 64)
!1513 = !DIDerivedType(tag: DW_TAG_member, scope: !1061, file: !230, line: 151, baseType: !1514, size: 32, offset: 384)
!1514 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1061, file: !230, line: 151, size: 32, elements: !1515)
!1515 = !{!1516, !1517}
!1516 = !DIDerivedType(tag: DW_TAG_member, name: "page_type", scope: !1514, file: !230, line: 166, baseType: !7, size: 32)
!1517 = !DIDerivedType(tag: DW_TAG_member, name: "_mapcount", scope: !1514, file: !230, line: 177, baseType: !69, size: 32)
!1518 = !DIDerivedType(tag: DW_TAG_member, name: "_refcount", scope: !1061, file: !230, line: 181, baseType: !69, size: 32, offset: 416)
!1519 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1520, size: 64)
!1520 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "writeback_control", file: !246, line: 43, size: 2624, elements: !1521)
!1521 = !{!1522, !1523, !1524, !1525, !1526, !1527, !1528, !1529, !1530, !1531, !1532, !1533, !1534, !1535, !1539, !1540, !1617, !1618}
!1522 = !DIDerivedType(tag: DW_TAG_member, name: "nr_to_write", scope: !1520, file: !246, line: 45, baseType: !892, size: 64)
!1523 = !DIDerivedType(tag: DW_TAG_member, name: "pages_skipped", scope: !1520, file: !246, line: 47, baseType: !892, size: 64, offset: 64)
!1524 = !DIDerivedType(tag: DW_TAG_member, name: "range_start", scope: !1520, file: !246, line: 54, baseType: !61, size: 64, offset: 128)
!1525 = !DIDerivedType(tag: DW_TAG_member, name: "range_end", scope: !1520, file: !246, line: 55, baseType: !61, size: 64, offset: 192)
!1526 = !DIDerivedType(tag: DW_TAG_member, name: "sync_mode", scope: !1520, file: !246, line: 57, baseType: !245, size: 32, offset: 256)
!1527 = !DIDerivedType(tag: DW_TAG_member, name: "for_kupdate", scope: !1520, file: !246, line: 59, baseType: !7, size: 1, offset: 288, flags: DIFlagBitField, extraData: i64 288)
!1528 = !DIDerivedType(tag: DW_TAG_member, name: "for_background", scope: !1520, file: !246, line: 60, baseType: !7, size: 1, offset: 289, flags: DIFlagBitField, extraData: i64 288)
!1529 = !DIDerivedType(tag: DW_TAG_member, name: "tagged_writepages", scope: !1520, file: !246, line: 61, baseType: !7, size: 1, offset: 290, flags: DIFlagBitField, extraData: i64 288)
!1530 = !DIDerivedType(tag: DW_TAG_member, name: "for_reclaim", scope: !1520, file: !246, line: 62, baseType: !7, size: 1, offset: 291, flags: DIFlagBitField, extraData: i64 288)
!1531 = !DIDerivedType(tag: DW_TAG_member, name: "range_cyclic", scope: !1520, file: !246, line: 63, baseType: !7, size: 1, offset: 292, flags: DIFlagBitField, extraData: i64 288)
!1532 = !DIDerivedType(tag: DW_TAG_member, name: "for_sync", scope: !1520, file: !246, line: 64, baseType: !7, size: 1, offset: 293, flags: DIFlagBitField, extraData: i64 288)
!1533 = !DIDerivedType(tag: DW_TAG_member, name: "unpinned_netfs_wb", scope: !1520, file: !246, line: 65, baseType: !7, size: 1, offset: 294, flags: DIFlagBitField, extraData: i64 288)
!1534 = !DIDerivedType(tag: DW_TAG_member, name: "no_cgroup_owner", scope: !1520, file: !246, line: 73, baseType: !7, size: 1, offset: 295, flags: DIFlagBitField, extraData: i64 288)
!1535 = !DIDerivedType(tag: DW_TAG_member, name: "swap_plug", scope: !1520, file: !246, line: 80, baseType: !1536, size: 64, offset: 320)
!1536 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1537, size: 64)
!1537 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1538, size: 64)
!1538 = !DICompositeType(tag: DW_TAG_structure_type, name: "swap_iocb", file: !246, line: 80, flags: DIFlagFwdDecl)
!1539 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !1520, file: !246, line: 83, baseType: !120, size: 64, offset: 384)
!1540 = !DIDerivedType(tag: DW_TAG_member, name: "fbatch", scope: !1520, file: !246, line: 86, baseType: !1541, size: 2048, offset: 448)
!1541 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "folio_batch", file: !1542, line: 28, size: 2048, elements: !1543)
!1542 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/pagevec.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "5ccc0a849d6419285aba52d828d5582b")
!1543 = !{!1544, !1545, !1546, !1547}
!1544 = !DIDerivedType(tag: DW_TAG_member, name: "nr", scope: !1541, file: !1542, line: 29, baseType: !107, size: 8)
!1545 = !DIDerivedType(tag: DW_TAG_member, name: "i", scope: !1541, file: !1542, line: 30, baseType: !107, size: 8, offset: 8)
!1546 = !DIDerivedType(tag: DW_TAG_member, name: "percpu_pvec_drained", scope: !1541, file: !1542, line: 31, baseType: !614, size: 8, offset: 16)
!1547 = !DIDerivedType(tag: DW_TAG_member, name: "folios", scope: !1541, file: !1542, line: 32, baseType: !1548, size: 1984, offset: 64)
!1548 = !DICompositeType(tag: DW_TAG_array_type, baseType: !1549, size: 1984, elements: !1615)
!1549 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1550, size: 64)
!1550 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "folio", file: !230, line: 324, size: 1536, elements: !1551)
!1551 = !{!1552, !1582, !1596}
!1552 = !DIDerivedType(tag: DW_TAG_member, scope: !1550, file: !230, line: 326, baseType: !1553, size: 512)
!1553 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1550, file: !230, line: 326, size: 512, elements: !1554)
!1554 = !{!1555, !1581}
!1555 = !DIDerivedType(tag: DW_TAG_member, scope: !1553, file: !230, line: 327, baseType: !1556, size: 448)
!1556 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1553, file: !230, line: 327, size: 448, elements: !1557)
!1557 = !{!1558, !1559, !1568, !1569, !1570, !1579, !1580}
!1558 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !1556, file: !230, line: 329, baseType: !59, size: 64)
!1559 = !DIDerivedType(tag: DW_TAG_member, scope: !1556, file: !230, line: 330, baseType: !1560, size: 128, offset: 64)
!1560 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1556, file: !230, line: 330, size: 128, elements: !1561)
!1561 = !{!1562, !1563}
!1562 = !DIDerivedType(tag: DW_TAG_member, name: "lru", scope: !1560, file: !230, line: 331, baseType: !117, size: 128)
!1563 = !DIDerivedType(tag: DW_TAG_member, scope: !1560, file: !230, line: 333, baseType: !1564, size: 128)
!1564 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1560, file: !230, line: 333, size: 128, elements: !1565)
!1565 = !{!1566, !1567}
!1566 = !DIDerivedType(tag: DW_TAG_member, name: "__filler", scope: !1564, file: !230, line: 334, baseType: !40, size: 64)
!1567 = !DIDerivedType(tag: DW_TAG_member, name: "mlock_count", scope: !1564, file: !230, line: 336, baseType: !7, size: 32, offset: 64)
!1568 = !DIDerivedType(tag: DW_TAG_member, name: "mapping", scope: !1556, file: !230, line: 341, baseType: !1030, size: 64, offset: 192)
!1569 = !DIDerivedType(tag: DW_TAG_member, name: "index", scope: !1556, file: !230, line: 342, baseType: !59, size: 64, offset: 256)
!1570 = !DIDerivedType(tag: DW_TAG_member, scope: !1556, file: !230, line: 343, baseType: !1571, size: 64, offset: 320)
!1571 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1556, file: !230, line: 343, size: 64, elements: !1572)
!1572 = !{!1573, !1574}
!1573 = !DIDerivedType(tag: DW_TAG_member, name: "private", scope: !1571, file: !230, line: 344, baseType: !40, size: 64)
!1574 = !DIDerivedType(tag: DW_TAG_member, name: "swap", scope: !1571, file: !230, line: 345, baseType: !1575, size: 64)
!1575 = !DIDerivedType(tag: DW_TAG_typedef, name: "swp_entry_t", file: !230, line: 284, baseType: !1576)
!1576 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !230, line: 282, size: 64, elements: !1577)
!1577 = !{!1578}
!1578 = !DIDerivedType(tag: DW_TAG_member, name: "val", scope: !1576, file: !230, line: 283, baseType: !59, size: 64)
!1579 = !DIDerivedType(tag: DW_TAG_member, name: "_mapcount", scope: !1556, file: !230, line: 347, baseType: !69, size: 32, offset: 384)
!1580 = !DIDerivedType(tag: DW_TAG_member, name: "_refcount", scope: !1556, file: !230, line: 348, baseType: !69, size: 32, offset: 416)
!1581 = !DIDerivedType(tag: DW_TAG_member, name: "page", scope: !1553, file: !230, line: 362, baseType: !1061, size: 512, align: 128)
!1582 = !DIDerivedType(tag: DW_TAG_member, scope: !1550, file: !230, line: 364, baseType: !1583, size: 512, offset: 512)
!1583 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1550, file: !230, line: 364, size: 512, elements: !1584)
!1584 = !{!1585, !1595}
!1585 = !DIDerivedType(tag: DW_TAG_member, scope: !1583, file: !230, line: 365, baseType: !1586, size: 320)
!1586 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1583, file: !230, line: 365, size: 320, elements: !1587)
!1587 = !{!1588, !1589, !1590, !1591, !1592, !1593, !1594}
!1588 = !DIDerivedType(tag: DW_TAG_member, name: "_flags_1", scope: !1586, file: !230, line: 366, baseType: !59, size: 64)
!1589 = !DIDerivedType(tag: DW_TAG_member, name: "_head_1", scope: !1586, file: !230, line: 367, baseType: !59, size: 64, offset: 64)
!1590 = !DIDerivedType(tag: DW_TAG_member, name: "_large_mapcount", scope: !1586, file: !230, line: 369, baseType: !69, size: 32, offset: 128)
!1591 = !DIDerivedType(tag: DW_TAG_member, name: "_entire_mapcount", scope: !1586, file: !230, line: 370, baseType: !69, size: 32, offset: 160)
!1592 = !DIDerivedType(tag: DW_TAG_member, name: "_nr_pages_mapped", scope: !1586, file: !230, line: 371, baseType: !69, size: 32, offset: 192)
!1593 = !DIDerivedType(tag: DW_TAG_member, name: "_pincount", scope: !1586, file: !230, line: 372, baseType: !69, size: 32, offset: 224)
!1594 = !DIDerivedType(tag: DW_TAG_member, name: "_folio_nr_pages", scope: !1586, file: !230, line: 374, baseType: !7, size: 32, offset: 256)
!1595 = !DIDerivedType(tag: DW_TAG_member, name: "__page_1", scope: !1583, file: !230, line: 378, baseType: !1061, size: 512, align: 128)
!1596 = !DIDerivedType(tag: DW_TAG_member, scope: !1550, file: !230, line: 380, baseType: !1597, size: 512, offset: 1024)
!1597 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1550, file: !230, line: 380, size: 512, elements: !1598)
!1598 = !{!1599, !1608, !1614}
!1599 = !DIDerivedType(tag: DW_TAG_member, scope: !1597, file: !230, line: 381, baseType: !1600, size: 384)
!1600 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1597, file: !230, line: 381, size: 384, elements: !1601)
!1601 = !{!1602, !1603, !1604, !1605, !1606, !1607}
!1602 = !DIDerivedType(tag: DW_TAG_member, name: "_flags_2", scope: !1600, file: !230, line: 382, baseType: !59, size: 64)
!1603 = !DIDerivedType(tag: DW_TAG_member, name: "_head_2", scope: !1600, file: !230, line: 383, baseType: !59, size: 64, offset: 64)
!1604 = !DIDerivedType(tag: DW_TAG_member, name: "_hugetlb_subpool", scope: !1600, file: !230, line: 385, baseType: !40, size: 64, offset: 128)
!1605 = !DIDerivedType(tag: DW_TAG_member, name: "_hugetlb_cgroup", scope: !1600, file: !230, line: 386, baseType: !40, size: 64, offset: 192)
!1606 = !DIDerivedType(tag: DW_TAG_member, name: "_hugetlb_cgroup_rsvd", scope: !1600, file: !230, line: 387, baseType: !40, size: 64, offset: 256)
!1607 = !DIDerivedType(tag: DW_TAG_member, name: "_hugetlb_hwpoison", scope: !1600, file: !230, line: 388, baseType: !40, size: 64, offset: 320)
!1608 = !DIDerivedType(tag: DW_TAG_member, scope: !1597, file: !230, line: 391, baseType: !1609, size: 256)
!1609 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1597, file: !230, line: 391, size: 256, elements: !1610)
!1610 = !{!1611, !1612, !1613}
!1611 = !DIDerivedType(tag: DW_TAG_member, name: "_flags_2a", scope: !1609, file: !230, line: 392, baseType: !59, size: 64)
!1612 = !DIDerivedType(tag: DW_TAG_member, name: "_head_2a", scope: !1609, file: !230, line: 393, baseType: !59, size: 64, offset: 64)
!1613 = !DIDerivedType(tag: DW_TAG_member, name: "_deferred_list", scope: !1609, file: !230, line: 395, baseType: !117, size: 128, offset: 128)
!1614 = !DIDerivedType(tag: DW_TAG_member, name: "__page_2", scope: !1597, file: !230, line: 398, baseType: !1061, size: 512, align: 128)
!1615 = !{!1616}
!1616 = !DISubrange(count: 31)
!1617 = !DIDerivedType(tag: DW_TAG_member, name: "index", scope: !1520, file: !246, line: 87, baseType: !59, size: 64, offset: 2496)
!1618 = !DIDerivedType(tag: DW_TAG_member, name: "saved_err", scope: !1520, file: !246, line: 88, baseType: !42, size: 32, offset: 2560)
!1619 = !DIDerivedType(tag: DW_TAG_member, name: "read_folio", scope: !1054, file: !342, line: 399, baseType: !1620, size: 64, offset: 64)
!1620 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1621, size: 64)
!1621 = !DISubroutineType(types: !1622)
!1622 = !{!42, !896, !1549}
!1623 = !DIDerivedType(tag: DW_TAG_member, name: "writepages", scope: !1054, file: !342, line: 402, baseType: !1624, size: 64, offset: 128)
!1624 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1625, size: 64)
!1625 = !DISubroutineType(types: !1626)
!1626 = !{!42, !1030, !1519}
!1627 = !DIDerivedType(tag: DW_TAG_member, name: "dirty_folio", scope: !1054, file: !342, line: 405, baseType: !1628, size: 64, offset: 192)
!1628 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1629, size: 64)
!1629 = !DISubroutineType(types: !1630)
!1630 = !{!614, !1030, !1549}
!1631 = !DIDerivedType(tag: DW_TAG_member, name: "readahead", scope: !1054, file: !342, line: 407, baseType: !1632, size: 64, offset: 256)
!1632 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1633, size: 64)
!1633 = !DISubroutineType(types: !1634)
!1634 = !{null, !1635}
!1635 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1636, size: 64)
!1636 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "readahead_control", file: !1637, line: 1345, size: 448, elements: !1638)
!1637 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/pagemap.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "4321dae20f6aefabe31e71674e9465f9")
!1638 = !{!1639, !1640, !1641, !1651, !1652, !1653, !1654, !1655}
!1639 = !DIDerivedType(tag: DW_TAG_member, name: "file", scope: !1636, file: !1637, line: 1346, baseType: !896, size: 64)
!1640 = !DIDerivedType(tag: DW_TAG_member, name: "mapping", scope: !1636, file: !1637, line: 1347, baseType: !1030, size: 64, offset: 64)
!1641 = !DIDerivedType(tag: DW_TAG_member, name: "ra", scope: !1636, file: !1637, line: 1348, baseType: !1642, size: 64, offset: 128)
!1642 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1643, size: 64)
!1643 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "file_ra_state", file: !342, line: 988, size: 256, elements: !1644)
!1644 = !{!1645, !1646, !1647, !1648, !1649, !1650}
!1645 = !DIDerivedType(tag: DW_TAG_member, name: "start", scope: !1643, file: !342, line: 989, baseType: !59, size: 64)
!1646 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !1643, file: !342, line: 990, baseType: !7, size: 32, offset: 64)
!1647 = !DIDerivedType(tag: DW_TAG_member, name: "async_size", scope: !1643, file: !342, line: 991, baseType: !7, size: 32, offset: 96)
!1648 = !DIDerivedType(tag: DW_TAG_member, name: "ra_pages", scope: !1643, file: !342, line: 992, baseType: !7, size: 32, offset: 128)
!1649 = !DIDerivedType(tag: DW_TAG_member, name: "mmap_miss", scope: !1643, file: !342, line: 993, baseType: !7, size: 32, offset: 160)
!1650 = !DIDerivedType(tag: DW_TAG_member, name: "prev_pos", scope: !1643, file: !342, line: 994, baseType: !61, size: 64, offset: 192)
!1651 = !DIDerivedType(tag: DW_TAG_member, name: "_index", scope: !1636, file: !1637, line: 1350, baseType: !59, size: 64, offset: 192)
!1652 = !DIDerivedType(tag: DW_TAG_member, name: "_nr_pages", scope: !1636, file: !1637, line: 1351, baseType: !7, size: 32, offset: 256)
!1653 = !DIDerivedType(tag: DW_TAG_member, name: "_batch_count", scope: !1636, file: !1637, line: 1352, baseType: !7, size: 32, offset: 288)
!1654 = !DIDerivedType(tag: DW_TAG_member, name: "_workingset", scope: !1636, file: !1637, line: 1353, baseType: !614, size: 8, offset: 320)
!1655 = !DIDerivedType(tag: DW_TAG_member, name: "_pflags", scope: !1636, file: !1637, line: 1354, baseType: !59, size: 64, offset: 384)
!1656 = !DIDerivedType(tag: DW_TAG_member, name: "write_begin", scope: !1054, file: !342, line: 409, baseType: !1657, size: 64, offset: 320)
!1657 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1658, size: 64)
!1658 = !DISubroutineType(types: !1659)
!1659 = !{!42, !896, !1030, !61, !7, !1660, !1661}
!1660 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1549, size: 64)
!1661 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !40, size: 64)
!1662 = !DIDerivedType(tag: DW_TAG_member, name: "write_end", scope: !1054, file: !342, line: 412, baseType: !1663, size: 64, offset: 384)
!1663 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1664, size: 64)
!1664 = !DISubroutineType(types: !1665)
!1665 = !{!42, !896, !1030, !61, !7, !7, !1549, !40}
!1666 = !DIDerivedType(tag: DW_TAG_member, name: "bmap", scope: !1054, file: !342, line: 417, baseType: !1667, size: 64, offset: 448)
!1667 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1668, size: 64)
!1668 = !DISubroutineType(types: !1669)
!1669 = !{!1670, !1030, !1670}
!1670 = !DIDerivedType(tag: DW_TAG_typedef, name: "sector_t", file: !45, line: 134, baseType: !519)
!1671 = !DIDerivedType(tag: DW_TAG_member, name: "invalidate_folio", scope: !1054, file: !342, line: 418, baseType: !1672, size: 64, offset: 512)
!1672 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1673, size: 64)
!1673 = !DISubroutineType(types: !1674)
!1674 = !{null, !1549, !55, !55}
!1675 = !DIDerivedType(tag: DW_TAG_member, name: "release_folio", scope: !1054, file: !342, line: 419, baseType: !1676, size: 64, offset: 576)
!1676 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1677, size: 64)
!1677 = !DISubroutineType(types: !1678)
!1678 = !{!614, !1549, !488}
!1679 = !DIDerivedType(tag: DW_TAG_member, name: "free_folio", scope: !1054, file: !342, line: 420, baseType: !1680, size: 64, offset: 640)
!1680 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1681, size: 64)
!1681 = !DISubroutineType(types: !1682)
!1682 = !{null, !1549}
!1683 = !DIDerivedType(tag: DW_TAG_member, name: "direct_IO", scope: !1054, file: !342, line: 421, baseType: !1684, size: 64, offset: 704)
!1684 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1685, size: 64)
!1685 = !DISubroutineType(types: !1686)
!1686 = !{!993, !1687, !1725}
!1687 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1688, size: 64)
!1688 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kiocb", file: !342, line: 366, size: 384, elements: !1689)
!1689 = !{!1690, !1691, !1692, !1696, !1697, !1698, !1699}
!1690 = !DIDerivedType(tag: DW_TAG_member, name: "ki_filp", scope: !1688, file: !342, line: 367, baseType: !896, size: 64)
!1691 = !DIDerivedType(tag: DW_TAG_member, name: "ki_pos", scope: !1688, file: !342, line: 368, baseType: !61, size: 64, offset: 64)
!1692 = !DIDerivedType(tag: DW_TAG_member, name: "ki_complete", scope: !1688, file: !342, line: 369, baseType: !1693, size: 64, offset: 128)
!1693 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1694, size: 64)
!1694 = !DISubroutineType(types: !1695)
!1695 = !{null, !1687, !892}
!1696 = !DIDerivedType(tag: DW_TAG_member, name: "private", scope: !1688, file: !342, line: 370, baseType: !40, size: 64, offset: 192)
!1697 = !DIDerivedType(tag: DW_TAG_member, name: "ki_flags", scope: !1688, file: !342, line: 371, baseType: !42, size: 32, offset: 256)
!1698 = !DIDerivedType(tag: DW_TAG_member, name: "ki_ioprio", scope: !1688, file: !342, line: 372, baseType: !113, size: 16, offset: 288)
!1699 = !DIDerivedType(tag: DW_TAG_member, scope: !1688, file: !342, line: 373, baseType: !1700, size: 64, offset: 320)
!1700 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1688, file: !342, line: 373, size: 64, elements: !1701)
!1701 = !{!1702, !1721}
!1702 = !DIDerivedType(tag: DW_TAG_member, name: "ki_waitq", scope: !1700, file: !342, line: 379, baseType: !1703, size: 64)
!1703 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1704, size: 64)
!1704 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "wait_page_queue", file: !1637, line: 1072, size: 448, elements: !1705)
!1705 = !{!1706, !1707, !1708}
!1706 = !DIDerivedType(tag: DW_TAG_member, name: "folio", scope: !1704, file: !1637, line: 1073, baseType: !1549, size: 64)
!1707 = !DIDerivedType(tag: DW_TAG_member, name: "bit_nr", scope: !1704, file: !1637, line: 1074, baseType: !42, size: 32, offset: 64)
!1708 = !DIDerivedType(tag: DW_TAG_member, name: "wait", scope: !1704, file: !1637, line: 1075, baseType: !1709, size: 320, offset: 128)
!1709 = !DIDerivedType(tag: DW_TAG_typedef, name: "wait_queue_entry_t", file: !75, line: 13, baseType: !1710)
!1710 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "wait_queue_entry", file: !75, line: 28, size: 320, elements: !1711)
!1711 = !{!1712, !1713, !1714, !1720}
!1712 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !1710, file: !75, line: 29, baseType: !7, size: 32)
!1713 = !DIDerivedType(tag: DW_TAG_member, name: "private", scope: !1710, file: !75, line: 30, baseType: !40, size: 64, offset: 64)
!1714 = !DIDerivedType(tag: DW_TAG_member, name: "func", scope: !1710, file: !75, line: 31, baseType: !1715, size: 64, offset: 128)
!1715 = !DIDerivedType(tag: DW_TAG_typedef, name: "wait_queue_func_t", file: !75, line: 15, baseType: !1716)
!1716 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1717, size: 64)
!1717 = !DISubroutineType(types: !1718)
!1718 = !{!42, !1719, !7, !42, !40}
!1719 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1710, size: 64)
!1720 = !DIDerivedType(tag: DW_TAG_member, name: "entry", scope: !1710, file: !75, line: 32, baseType: !117, size: 128, offset: 192)
!1721 = !DIDerivedType(tag: DW_TAG_member, name: "dio_complete", scope: !1700, file: !342, line: 388, baseType: !1722, size: 64)
!1722 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1723, size: 64)
!1723 = !DISubroutineType(types: !1724)
!1724 = !{!993, !40}
!1725 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1726, size: 64)
!1726 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "iov_iter", file: !1727, line: 43, size: 320, elements: !1728)
!1727 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/uio.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "18dd972f1bbc5b8d2a3fb451e2c81169")
!1728 = !{!1729, !1730, !1731, !1732, !1733, !1775}
!1729 = !DIDerivedType(tag: DW_TAG_member, name: "iter_type", scope: !1726, file: !1727, line: 44, baseType: !103, size: 8)
!1730 = !DIDerivedType(tag: DW_TAG_member, name: "nofault", scope: !1726, file: !1727, line: 45, baseType: !614, size: 8, offset: 8)
!1731 = !DIDerivedType(tag: DW_TAG_member, name: "data_source", scope: !1726, file: !1727, line: 46, baseType: !614, size: 8, offset: 16)
!1732 = !DIDerivedType(tag: DW_TAG_member, name: "iov_offset", scope: !1726, file: !1727, line: 47, baseType: !55, size: 64, offset: 64)
!1733 = !DIDerivedType(tag: DW_TAG_member, scope: !1726, file: !1727, line: 58, baseType: !1734, size: 128, offset: 128)
!1734 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1726, file: !1727, line: 58, size: 128, elements: !1735)
!1735 = !{!1736, !1742}
!1736 = !DIDerivedType(tag: DW_TAG_member, name: "__ubuf_iovec", scope: !1734, file: !1727, line: 64, baseType: !1737, size: 128)
!1737 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "iovec", file: !1738, line: 17, size: 128, elements: !1739)
!1738 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/uapi/linux/uio.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "6e318eef63b7a9f613578ccb53686eb8")
!1739 = !{!1740, !1741}
!1740 = !DIDerivedType(tag: DW_TAG_member, name: "iov_base", scope: !1737, file: !1738, line: 19, baseType: !40, size: 64)
!1741 = !DIDerivedType(tag: DW_TAG_member, name: "iov_len", scope: !1737, file: !1738, line: 20, baseType: !56, size: 64, offset: 64)
!1742 = !DIDerivedType(tag: DW_TAG_member, scope: !1734, file: !1727, line: 65, baseType: !1743, size: 128)
!1743 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !1734, file: !1727, line: 65, size: 128, elements: !1744)
!1744 = !{!1745, !1774}
!1745 = !DIDerivedType(tag: DW_TAG_member, scope: !1743, file: !1727, line: 66, baseType: !1746, size: 64)
!1746 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1743, file: !1727, line: 66, size: 64, elements: !1747)
!1747 = !{!1748, !1751, !1758, !1767, !1771, !1773}
!1748 = !DIDerivedType(tag: DW_TAG_member, name: "__iov", scope: !1746, file: !1727, line: 68, baseType: !1749, size: 64)
!1749 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1750, size: 64)
!1750 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1737)
!1751 = !DIDerivedType(tag: DW_TAG_member, name: "kvec", scope: !1746, file: !1727, line: 69, baseType: !1752, size: 64)
!1752 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1753, size: 64)
!1753 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1754)
!1754 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kvec", file: !1727, line: 18, size: 128, elements: !1755)
!1755 = !{!1756, !1757}
!1756 = !DIDerivedType(tag: DW_TAG_member, name: "iov_base", scope: !1754, file: !1727, line: 19, baseType: !40, size: 64)
!1757 = !DIDerivedType(tag: DW_TAG_member, name: "iov_len", scope: !1754, file: !1727, line: 20, baseType: !55, size: 64, offset: 64)
!1758 = !DIDerivedType(tag: DW_TAG_member, name: "bvec", scope: !1746, file: !1727, line: 70, baseType: !1759, size: 64)
!1759 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1760, size: 64)
!1760 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1761)
!1761 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bio_vec", file: !1762, line: 31, size: 128, elements: !1763)
!1762 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/bvec.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "49d621f4426418e17069746c82d57682")
!1763 = !{!1764, !1765, !1766}
!1764 = !DIDerivedType(tag: DW_TAG_member, name: "bv_page", scope: !1761, file: !1762, line: 32, baseType: !1060, size: 64)
!1765 = !DIDerivedType(tag: DW_TAG_member, name: "bv_len", scope: !1761, file: !1762, line: 33, baseType: !7, size: 32, offset: 64)
!1766 = !DIDerivedType(tag: DW_TAG_member, name: "bv_offset", scope: !1761, file: !1762, line: 34, baseType: !7, size: 32, offset: 96)
!1767 = !DIDerivedType(tag: DW_TAG_member, name: "folioq", scope: !1746, file: !1727, line: 71, baseType: !1768, size: 64)
!1768 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1769, size: 64)
!1769 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1770)
!1770 = !DICompositeType(tag: DW_TAG_structure_type, name: "folio_queue", file: !1727, line: 14, flags: DIFlagFwdDecl)
!1771 = !DIDerivedType(tag: DW_TAG_member, name: "xarray", scope: !1746, file: !1727, line: 72, baseType: !1772, size: 64)
!1772 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1035, size: 64)
!1773 = !DIDerivedType(tag: DW_TAG_member, name: "ubuf", scope: !1746, file: !1727, line: 73, baseType: !40, size: 64)
!1774 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !1743, file: !1727, line: 75, baseType: !55, size: 64, offset: 64)
!1775 = !DIDerivedType(tag: DW_TAG_member, scope: !1726, file: !1727, line: 78, baseType: !1776, size: 64, offset: 256)
!1776 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !1726, file: !1727, line: 78, size: 64, elements: !1777)
!1777 = !{!1778, !1779, !1780}
!1778 = !DIDerivedType(tag: DW_TAG_member, name: "nr_segs", scope: !1776, file: !1727, line: 79, baseType: !59, size: 64)
!1779 = !DIDerivedType(tag: DW_TAG_member, name: "folioq_slot", scope: !1776, file: !1727, line: 80, baseType: !103, size: 8)
!1780 = !DIDerivedType(tag: DW_TAG_member, name: "xarray_start", scope: !1776, file: !1727, line: 81, baseType: !61, size: 64)
!1781 = !DIDerivedType(tag: DW_TAG_member, name: "migrate_folio", scope: !1054, file: !342, line: 426, baseType: !1782, size: 64, offset: 768)
!1782 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1783, size: 64)
!1783 = !DISubroutineType(types: !1784)
!1784 = !{!42, !1030, !1549, !1549, !250}
!1785 = !DIDerivedType(tag: DW_TAG_member, name: "launder_folio", scope: !1054, file: !342, line: 428, baseType: !1786, size: 64, offset: 832)
!1786 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1787, size: 64)
!1787 = !DISubroutineType(types: !1788)
!1788 = !{!42, !1549}
!1789 = !DIDerivedType(tag: DW_TAG_member, name: "is_partially_uptodate", scope: !1054, file: !342, line: 429, baseType: !1790, size: 64, offset: 896)
!1790 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1791, size: 64)
!1791 = !DISubroutineType(types: !1792)
!1792 = !{!614, !1549, !55, !55}
!1793 = !DIDerivedType(tag: DW_TAG_member, name: "is_dirty_writeback", scope: !1054, file: !342, line: 431, baseType: !1794, size: 64, offset: 960)
!1794 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1795, size: 64)
!1795 = !DISubroutineType(types: !1796)
!1796 = !{null, !1549, !1797, !1797}
!1797 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !614, size: 64)
!1798 = !DIDerivedType(tag: DW_TAG_member, name: "error_remove_folio", scope: !1054, file: !342, line: 432, baseType: !1799, size: 64, offset: 1024)
!1799 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1800, size: 64)
!1800 = !DISubroutineType(types: !1801)
!1801 = !{!42, !1030, !1549}
!1802 = !DIDerivedType(tag: DW_TAG_member, name: "swap_activate", scope: !1054, file: !342, line: 435, baseType: !1803, size: 64, offset: 1088)
!1803 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1804, size: 64)
!1804 = !DISubroutineType(types: !1805)
!1805 = !{!42, !1806, !896, !2360}
!1806 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1807, size: 64)
!1807 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "swap_info_struct", file: !1808, line: 291, size: 2624, elements: !1809)
!1808 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/swap.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "eb962595075a48434cbe9d197cb3cede")
!1809 = !{!1810, !1811, !1812, !1813, !1820, !1822, !1823, !1825, !1826, !1835, !1836, !1837, !1841, !1842, !1844, !1845, !1846, !1847, !1848, !1849, !1850, !1852, !1857, !1858, !2352, !2353, !2354, !2355, !2356, !2357, !2358}
!1810 = !DIDerivedType(tag: DW_TAG_member, name: "users", scope: !1807, file: !1808, line: 292, baseType: !1121, size: 128)
!1811 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !1807, file: !1808, line: 293, baseType: !59, size: 64, offset: 128)
!1812 = !DIDerivedType(tag: DW_TAG_member, name: "prio", scope: !1807, file: !1808, line: 294, baseType: !583, size: 16, offset: 192)
!1813 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !1807, file: !1808, line: 295, baseType: !1814, size: 320, offset: 256)
!1814 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "plist_node", file: !1815, line: 11, size: 320, elements: !1816)
!1815 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/plist_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "ff822ef947cf3b0b6a5fcee48d063ce2")
!1816 = !{!1817, !1818, !1819}
!1817 = !DIDerivedType(tag: DW_TAG_member, name: "prio", scope: !1814, file: !1815, line: 12, baseType: !42, size: 32)
!1818 = !DIDerivedType(tag: DW_TAG_member, name: "prio_list", scope: !1814, file: !1815, line: 13, baseType: !117, size: 128, offset: 64)
!1819 = !DIDerivedType(tag: DW_TAG_member, name: "node_list", scope: !1814, file: !1815, line: 14, baseType: !117, size: 128, offset: 192)
!1820 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !1807, file: !1808, line: 296, baseType: !1821, size: 8, offset: 576)
!1821 = !DIBasicType(name: "signed char", size: 8, encoding: DW_ATE_signed_char)
!1822 = !DIDerivedType(tag: DW_TAG_member, name: "max", scope: !1807, file: !1808, line: 297, baseType: !7, size: 32, offset: 608)
!1823 = !DIDerivedType(tag: DW_TAG_member, name: "swap_map", scope: !1807, file: !1808, line: 298, baseType: !1824, size: 64, offset: 640)
!1824 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !107, size: 64)
!1825 = !DIDerivedType(tag: DW_TAG_member, name: "zeromap", scope: !1807, file: !1808, line: 299, baseType: !1440, size: 64, offset: 704)
!1826 = !DIDerivedType(tag: DW_TAG_member, name: "cluster_info", scope: !1807, file: !1808, line: 300, baseType: !1827, size: 64, offset: 768)
!1827 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1828, size: 64)
!1828 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "swap_cluster_info", file: !1808, line: 249, size: 192, elements: !1829)
!1829 = !{!1830, !1831, !1832, !1833, !1834}
!1830 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !1828, file: !1808, line: 250, baseType: !79, size: 32)
!1831 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !1828, file: !1808, line: 255, baseType: !113, size: 16, offset: 32)
!1832 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !1828, file: !1808, line: 256, baseType: !103, size: 8, offset: 48)
!1833 = !DIDerivedType(tag: DW_TAG_member, name: "order", scope: !1828, file: !1808, line: 257, baseType: !103, size: 8, offset: 56)
!1834 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !1828, file: !1808, line: 258, baseType: !117, size: 128, offset: 64)
!1835 = !DIDerivedType(tag: DW_TAG_member, name: "free_clusters", scope: !1807, file: !1808, line: 301, baseType: !117, size: 128, offset: 832)
!1836 = !DIDerivedType(tag: DW_TAG_member, name: "full_clusters", scope: !1807, file: !1808, line: 302, baseType: !117, size: 128, offset: 960)
!1837 = !DIDerivedType(tag: DW_TAG_member, name: "nonfull_clusters", scope: !1807, file: !1808, line: 303, baseType: !1838, size: 128, offset: 1088)
!1838 = !DICompositeType(tag: DW_TAG_array_type, baseType: !117, size: 128, elements: !1839)
!1839 = !{!1840}
!1840 = !DISubrange(count: 1)
!1841 = !DIDerivedType(tag: DW_TAG_member, name: "frag_clusters", scope: !1807, file: !1808, line: 305, baseType: !1838, size: 128, offset: 1216)
!1842 = !DIDerivedType(tag: DW_TAG_member, name: "frag_cluster_nr", scope: !1807, file: !1808, line: 307, baseType: !1843, size: 32, offset: 1344)
!1843 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 32, elements: !1839)
!1844 = !DIDerivedType(tag: DW_TAG_member, name: "lowest_bit", scope: !1807, file: !1808, line: 308, baseType: !7, size: 32, offset: 1376)
!1845 = !DIDerivedType(tag: DW_TAG_member, name: "highest_bit", scope: !1807, file: !1808, line: 309, baseType: !7, size: 32, offset: 1408)
!1846 = !DIDerivedType(tag: DW_TAG_member, name: "pages", scope: !1807, file: !1808, line: 310, baseType: !7, size: 32, offset: 1440)
!1847 = !DIDerivedType(tag: DW_TAG_member, name: "inuse_pages", scope: !1807, file: !1808, line: 311, baseType: !7, size: 32, offset: 1472)
!1848 = !DIDerivedType(tag: DW_TAG_member, name: "cluster_next", scope: !1807, file: !1808, line: 312, baseType: !7, size: 32, offset: 1504)
!1849 = !DIDerivedType(tag: DW_TAG_member, name: "cluster_nr", scope: !1807, file: !1808, line: 313, baseType: !7, size: 32, offset: 1536)
!1850 = !DIDerivedType(tag: DW_TAG_member, name: "cluster_next_cpu", scope: !1807, file: !1808, line: 314, baseType: !1851, size: 64, offset: 1600)
!1851 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!1852 = !DIDerivedType(tag: DW_TAG_member, name: "percpu_cluster", scope: !1807, file: !1808, line: 315, baseType: !1853, size: 64, offset: 1664)
!1853 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1854, size: 64)
!1854 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "percpu_cluster", file: !1808, line: 284, size: 32, elements: !1855)
!1855 = !{!1856}
!1856 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !1854, file: !1808, line: 285, baseType: !1843, size: 32)
!1857 = !DIDerivedType(tag: DW_TAG_member, name: "swap_extent_root", scope: !1807, file: !1808, line: 316, baseType: !168, size: 64, offset: 1728)
!1858 = !DIDerivedType(tag: DW_TAG_member, name: "bdev", scope: !1807, file: !1808, line: 317, baseType: !1859, size: 64, offset: 1792)
!1859 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1860, size: 64)
!1860 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "block_device", file: !1861, line: 41, size: 7424, elements: !1862)
!1861 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/blk_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "980c1e8086848d7440e918dbb79c429f")
!1862 = !{!1863, !1864, !1865, !1868, !1872, !1875, !1876, !1877, !1878, !1879, !1880, !1881, !1882, !1883, !1887, !1888, !1889, !1890, !1891, !1892, !1895, !1896, !1897}
!1863 = !DIDerivedType(tag: DW_TAG_member, name: "bd_start_sect", scope: !1860, file: !1861, line: 42, baseType: !1670, size: 64)
!1864 = !DIDerivedType(tag: DW_TAG_member, name: "bd_nr_sectors", scope: !1860, file: !1861, line: 43, baseType: !1670, size: 64, offset: 64)
!1865 = !DIDerivedType(tag: DW_TAG_member, name: "bd_disk", scope: !1860, file: !1861, line: 44, baseType: !1866, size: 64, offset: 128)
!1866 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1867, size: 64)
!1867 = !DICompositeType(tag: DW_TAG_structure_type, name: "gendisk", file: !1435, line: 1501, flags: DIFlagFwdDecl)
!1868 = !DIDerivedType(tag: DW_TAG_member, name: "bd_queue", scope: !1860, file: !1861, line: 45, baseType: !1869, size: 64, offset: 192)
!1869 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1870, size: 64)
!1870 = !DICompositeType(tag: DW_TAG_structure_type, name: "request_queue", file: !1871, line: 74, flags: DIFlagFwdDecl)
!1871 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/iocontext.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "669329e5d8996a71da7fa0b321760590")
!1872 = !DIDerivedType(tag: DW_TAG_member, name: "bd_stats", scope: !1860, file: !1861, line: 46, baseType: !1873, size: 64, offset: 256)
!1873 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1874, size: 64)
!1874 = !DICompositeType(tag: DW_TAG_structure_type, name: "disk_stats", file: !1861, line: 46, flags: DIFlagFwdDecl)
!1875 = !DIDerivedType(tag: DW_TAG_member, name: "bd_stamp", scope: !1860, file: !1861, line: 47, baseType: !59, size: 64, offset: 320)
!1876 = !DIDerivedType(tag: DW_TAG_member, name: "__bd_flags", scope: !1860, file: !1861, line: 48, baseType: !69, size: 32, offset: 384)
!1877 = !DIDerivedType(tag: DW_TAG_member, name: "bd_dev", scope: !1860, file: !1861, line: 57, baseType: !852, size: 32, offset: 416)
!1878 = !DIDerivedType(tag: DW_TAG_member, name: "bd_mapping", scope: !1860, file: !1861, line: 58, baseType: !1030, size: 64, offset: 448)
!1879 = !DIDerivedType(tag: DW_TAG_member, name: "bd_openers", scope: !1860, file: !1861, line: 60, baseType: !69, size: 32, offset: 512)
!1880 = !DIDerivedType(tag: DW_TAG_member, name: "bd_size_lock", scope: !1860, file: !1861, line: 61, baseType: !79, size: 32, offset: 544)
!1881 = !DIDerivedType(tag: DW_TAG_member, name: "bd_claiming", scope: !1860, file: !1861, line: 62, baseType: !40, size: 64, offset: 576)
!1882 = !DIDerivedType(tag: DW_TAG_member, name: "bd_holder", scope: !1860, file: !1861, line: 63, baseType: !40, size: 64, offset: 640)
!1883 = !DIDerivedType(tag: DW_TAG_member, name: "bd_holder_ops", scope: !1860, file: !1861, line: 64, baseType: !1884, size: 64, offset: 704)
!1884 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1885, size: 64)
!1885 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1886)
!1886 = !DICompositeType(tag: DW_TAG_structure_type, name: "blk_holder_ops", file: !1861, line: 64, flags: DIFlagFwdDecl)
!1887 = !DIDerivedType(tag: DW_TAG_member, name: "bd_holder_lock", scope: !1860, file: !1861, line: 65, baseType: !1277, size: 256, offset: 768)
!1888 = !DIDerivedType(tag: DW_TAG_member, name: "bd_holders", scope: !1860, file: !1861, line: 66, baseType: !42, size: 32, offset: 1024)
!1889 = !DIDerivedType(tag: DW_TAG_member, name: "bd_holder_dir", scope: !1860, file: !1861, line: 67, baseType: !927, size: 64, offset: 1088)
!1890 = !DIDerivedType(tag: DW_TAG_member, name: "bd_fsfreeze_count", scope: !1860, file: !1861, line: 69, baseType: !69, size: 32, offset: 1152)
!1891 = !DIDerivedType(tag: DW_TAG_member, name: "bd_fsfreeze_mutex", scope: !1860, file: !1861, line: 70, baseType: !1277, size: 256, offset: 1216)
!1892 = !DIDerivedType(tag: DW_TAG_member, name: "bd_meta_info", scope: !1860, file: !1861, line: 72, baseType: !1893, size: 64, offset: 1472)
!1893 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1894, size: 64)
!1894 = !DICompositeType(tag: DW_TAG_structure_type, name: "partition_meta_info", file: !1861, line: 72, flags: DIFlagFwdDecl)
!1895 = !DIDerivedType(tag: DW_TAG_member, name: "bd_writers", scope: !1860, file: !1861, line: 73, baseType: !42, size: 32, offset: 1536)
!1896 = !DIDerivedType(tag: DW_TAG_member, name: "bd_security", scope: !1860, file: !1861, line: 75, baseType: !40, size: 64, offset: 1600)
!1897 = !DIDerivedType(tag: DW_TAG_member, name: "bd_device", scope: !1860, file: !1861, line: 81, baseType: !1898, size: 5760, offset: 1664)
!1898 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "device", file: !263, line: 721, size: 5760, elements: !1899)
!1899 = !{!1900, !1901, !1903, !1906, !1907, !1960, !2027, !2029, !2030, !2031, !2032, !2039, !2211, !2228, !2238, !2240, !2241, !2242, !2246, !2253, !2254, !2257, !2260, !2264, !2267, !2268, !2269, !2270, !2271, !2272, !2327, !2328, !2329, !2332, !2335, !2344, !2345, !2346, !2347, !2348, !2349, !2350, !2351}
!1900 = !DIDerivedType(tag: DW_TAG_member, name: "kobj", scope: !1898, file: !263, line: 722, baseType: !921, size: 512)
!1901 = !DIDerivedType(tag: DW_TAG_member, name: "parent", scope: !1898, file: !263, line: 723, baseType: !1902, size: 64, offset: 512)
!1902 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1898, size: 64)
!1903 = !DIDerivedType(tag: DW_TAG_member, name: "p", scope: !1898, file: !263, line: 725, baseType: !1904, size: 64, offset: 576)
!1904 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1905, size: 64)
!1905 = !DICompositeType(tag: DW_TAG_structure_type, name: "device_private", file: !263, line: 37, flags: DIFlagFwdDecl)
!1906 = !DIDerivedType(tag: DW_TAG_member, name: "init_name", scope: !1898, file: !263, line: 727, baseType: !36, size: 64, offset: 640)
!1907 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !1898, file: !263, line: 728, baseType: !1908, size: 64, offset: 704)
!1908 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1909, size: 64)
!1909 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1910)
!1910 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "device_type", file: !263, line: 88, size: 384, elements: !1911)
!1911 = !{!1912, !1913, !1914, !1920, !1925, !1929}
!1912 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !1910, file: !263, line: 89, baseType: !36, size: 64)
!1913 = !DIDerivedType(tag: DW_TAG_member, name: "groups", scope: !1910, file: !263, line: 90, baseType: !1006, size: 64, offset: 64)
!1914 = !DIDerivedType(tag: DW_TAG_member, name: "uevent", scope: !1910, file: !263, line: 91, baseType: !1915, size: 64, offset: 128)
!1915 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1916, size: 64)
!1916 = !DISubroutineType(types: !1917)
!1917 = !{!42, !1918, !957}
!1918 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1919, size: 64)
!1919 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1898)
!1920 = !DIDerivedType(tag: DW_TAG_member, name: "devnode", scope: !1910, file: !263, line: 92, baseType: !1921, size: 64, offset: 192)
!1921 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1922, size: 64)
!1922 = !DISubroutineType(types: !1923)
!1923 = !{!625, !1918, !1924, !187, !195}
!1924 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !44, size: 64)
!1925 = !DIDerivedType(tag: DW_TAG_member, name: "release", scope: !1910, file: !263, line: 94, baseType: !1926, size: 64, offset: 256)
!1926 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1927, size: 64)
!1927 = !DISubroutineType(types: !1928)
!1928 = !{null, !1902}
!1929 = !DIDerivedType(tag: DW_TAG_member, name: "pm", scope: !1910, file: !263, line: 96, baseType: !1930, size: 64, offset: 320)
!1930 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1931, size: 64)
!1931 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1932)
!1932 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "dev_pm_ops", file: !275, line: 286, size: 1472, elements: !1933)
!1933 = !{!1934, !1938, !1939, !1940, !1941, !1942, !1943, !1944, !1945, !1946, !1947, !1948, !1949, !1950, !1951, !1952, !1953, !1954, !1955, !1956, !1957, !1958, !1959}
!1934 = !DIDerivedType(tag: DW_TAG_member, name: "prepare", scope: !1932, file: !275, line: 287, baseType: !1935, size: 64)
!1935 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1936, size: 64)
!1936 = !DISubroutineType(types: !1937)
!1937 = !{!42, !1902}
!1938 = !DIDerivedType(tag: DW_TAG_member, name: "complete", scope: !1932, file: !275, line: 288, baseType: !1926, size: 64, offset: 64)
!1939 = !DIDerivedType(tag: DW_TAG_member, name: "suspend", scope: !1932, file: !275, line: 289, baseType: !1935, size: 64, offset: 128)
!1940 = !DIDerivedType(tag: DW_TAG_member, name: "resume", scope: !1932, file: !275, line: 290, baseType: !1935, size: 64, offset: 192)
!1941 = !DIDerivedType(tag: DW_TAG_member, name: "freeze", scope: !1932, file: !275, line: 291, baseType: !1935, size: 64, offset: 256)
!1942 = !DIDerivedType(tag: DW_TAG_member, name: "thaw", scope: !1932, file: !275, line: 292, baseType: !1935, size: 64, offset: 320)
!1943 = !DIDerivedType(tag: DW_TAG_member, name: "poweroff", scope: !1932, file: !275, line: 293, baseType: !1935, size: 64, offset: 384)
!1944 = !DIDerivedType(tag: DW_TAG_member, name: "restore", scope: !1932, file: !275, line: 294, baseType: !1935, size: 64, offset: 448)
!1945 = !DIDerivedType(tag: DW_TAG_member, name: "suspend_late", scope: !1932, file: !275, line: 295, baseType: !1935, size: 64, offset: 512)
!1946 = !DIDerivedType(tag: DW_TAG_member, name: "resume_early", scope: !1932, file: !275, line: 296, baseType: !1935, size: 64, offset: 576)
!1947 = !DIDerivedType(tag: DW_TAG_member, name: "freeze_late", scope: !1932, file: !275, line: 297, baseType: !1935, size: 64, offset: 640)
!1948 = !DIDerivedType(tag: DW_TAG_member, name: "thaw_early", scope: !1932, file: !275, line: 298, baseType: !1935, size: 64, offset: 704)
!1949 = !DIDerivedType(tag: DW_TAG_member, name: "poweroff_late", scope: !1932, file: !275, line: 299, baseType: !1935, size: 64, offset: 768)
!1950 = !DIDerivedType(tag: DW_TAG_member, name: "restore_early", scope: !1932, file: !275, line: 300, baseType: !1935, size: 64, offset: 832)
!1951 = !DIDerivedType(tag: DW_TAG_member, name: "suspend_noirq", scope: !1932, file: !275, line: 301, baseType: !1935, size: 64, offset: 896)
!1952 = !DIDerivedType(tag: DW_TAG_member, name: "resume_noirq", scope: !1932, file: !275, line: 302, baseType: !1935, size: 64, offset: 960)
!1953 = !DIDerivedType(tag: DW_TAG_member, name: "freeze_noirq", scope: !1932, file: !275, line: 303, baseType: !1935, size: 64, offset: 1024)
!1954 = !DIDerivedType(tag: DW_TAG_member, name: "thaw_noirq", scope: !1932, file: !275, line: 304, baseType: !1935, size: 64, offset: 1088)
!1955 = !DIDerivedType(tag: DW_TAG_member, name: "poweroff_noirq", scope: !1932, file: !275, line: 305, baseType: !1935, size: 64, offset: 1152)
!1956 = !DIDerivedType(tag: DW_TAG_member, name: "restore_noirq", scope: !1932, file: !275, line: 306, baseType: !1935, size: 64, offset: 1216)
!1957 = !DIDerivedType(tag: DW_TAG_member, name: "runtime_suspend", scope: !1932, file: !275, line: 307, baseType: !1935, size: 64, offset: 1280)
!1958 = !DIDerivedType(tag: DW_TAG_member, name: "runtime_resume", scope: !1932, file: !275, line: 308, baseType: !1935, size: 64, offset: 1344)
!1959 = !DIDerivedType(tag: DW_TAG_member, name: "runtime_idle", scope: !1932, file: !275, line: 309, baseType: !1935, size: 64, offset: 1408)
!1960 = !DIDerivedType(tag: DW_TAG_member, name: "bus", scope: !1898, file: !263, line: 730, baseType: !1961, size: 64, offset: 768)
!1961 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1962, size: 64)
!1962 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1963)
!1963 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bus_type", file: !1964, line: 77, size: 1280, elements: !1965)
!1964 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/device/bus.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "222bc0d86b0c180f089155f53817690d")
!1965 = !{!1966, !1967, !1968, !1969, !1970, !1971, !2013, !2014, !2015, !2016, !2017, !2018, !2019, !2020, !2021, !2022, !2023, !2024, !2025, !2026}
!1966 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !1963, file: !1964, line: 78, baseType: !36, size: 64)
!1967 = !DIDerivedType(tag: DW_TAG_member, name: "dev_name", scope: !1963, file: !1964, line: 79, baseType: !36, size: 64, offset: 64)
!1968 = !DIDerivedType(tag: DW_TAG_member, name: "bus_groups", scope: !1963, file: !1964, line: 80, baseType: !1006, size: 64, offset: 128)
!1969 = !DIDerivedType(tag: DW_TAG_member, name: "dev_groups", scope: !1963, file: !1964, line: 81, baseType: !1006, size: 64, offset: 192)
!1970 = !DIDerivedType(tag: DW_TAG_member, name: "drv_groups", scope: !1963, file: !1964, line: 82, baseType: !1006, size: 64, offset: 256)
!1971 = !DIDerivedType(tag: DW_TAG_member, name: "match", scope: !1963, file: !1964, line: 84, baseType: !1972, size: 64, offset: 320)
!1972 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1973, size: 64)
!1973 = !DISubroutineType(types: !1974)
!1974 = !{!42, !1902, !1975}
!1975 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1976, size: 64)
!1976 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1977)
!1977 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "device_driver", file: !257, line: 96, size: 1152, elements: !1978)
!1978 = !{!1979, !1980, !1981, !1982, !1983, !1984, !1985, !1989, !1993, !1994, !1995, !1996, !1997, !2005, !2006, !2007, !2008, !2009, !2010}
!1979 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !1977, file: !257, line: 97, baseType: !36, size: 64)
!1980 = !DIDerivedType(tag: DW_TAG_member, name: "bus", scope: !1977, file: !257, line: 98, baseType: !1961, size: 64, offset: 64)
!1981 = !DIDerivedType(tag: DW_TAG_member, name: "owner", scope: !1977, file: !257, line: 100, baseType: !908, size: 64, offset: 128)
!1982 = !DIDerivedType(tag: DW_TAG_member, name: "mod_name", scope: !1977, file: !257, line: 101, baseType: !36, size: 64, offset: 192)
!1983 = !DIDerivedType(tag: DW_TAG_member, name: "suppress_bind_attrs", scope: !1977, file: !257, line: 103, baseType: !614, size: 8, offset: 256)
!1984 = !DIDerivedType(tag: DW_TAG_member, name: "probe_type", scope: !1977, file: !257, line: 104, baseType: !256, size: 32, offset: 288)
!1985 = !DIDerivedType(tag: DW_TAG_member, name: "of_match_table", scope: !1977, file: !257, line: 106, baseType: !1986, size: 64, offset: 320)
!1986 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1987, size: 64)
!1987 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1988)
!1988 = !DICompositeType(tag: DW_TAG_structure_type, name: "of_device_id", file: !257, line: 106, flags: DIFlagFwdDecl)
!1989 = !DIDerivedType(tag: DW_TAG_member, name: "acpi_match_table", scope: !1977, file: !257, line: 107, baseType: !1990, size: 64, offset: 384)
!1990 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1991, size: 64)
!1991 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !1992)
!1992 = !DICompositeType(tag: DW_TAG_structure_type, name: "acpi_device_id", file: !257, line: 107, flags: DIFlagFwdDecl)
!1993 = !DIDerivedType(tag: DW_TAG_member, name: "probe", scope: !1977, file: !257, line: 109, baseType: !1935, size: 64, offset: 448)
!1994 = !DIDerivedType(tag: DW_TAG_member, name: "sync_state", scope: !1977, file: !257, line: 110, baseType: !1926, size: 64, offset: 512)
!1995 = !DIDerivedType(tag: DW_TAG_member, name: "remove", scope: !1977, file: !257, line: 111, baseType: !1935, size: 64, offset: 576)
!1996 = !DIDerivedType(tag: DW_TAG_member, name: "shutdown", scope: !1977, file: !257, line: 112, baseType: !1926, size: 64, offset: 640)
!1997 = !DIDerivedType(tag: DW_TAG_member, name: "suspend", scope: !1977, file: !257, line: 113, baseType: !1998, size: 64, offset: 704)
!1998 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1999, size: 64)
!1999 = !DISubroutineType(types: !2000)
!2000 = !{!42, !1902, !2001}
!2001 = !DIDerivedType(tag: DW_TAG_typedef, name: "pm_message_t", file: !275, line: 60, baseType: !2002)
!2002 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "pm_message", file: !275, line: 58, size: 32, elements: !2003)
!2003 = !{!2004}
!2004 = !DIDerivedType(tag: DW_TAG_member, name: "event", scope: !2002, file: !275, line: 59, baseType: !42, size: 32)
!2005 = !DIDerivedType(tag: DW_TAG_member, name: "resume", scope: !1977, file: !257, line: 114, baseType: !1935, size: 64, offset: 768)
!2006 = !DIDerivedType(tag: DW_TAG_member, name: "groups", scope: !1977, file: !257, line: 115, baseType: !1006, size: 64, offset: 832)
!2007 = !DIDerivedType(tag: DW_TAG_member, name: "dev_groups", scope: !1977, file: !257, line: 116, baseType: !1006, size: 64, offset: 896)
!2008 = !DIDerivedType(tag: DW_TAG_member, name: "pm", scope: !1977, file: !257, line: 118, baseType: !1930, size: 64, offset: 960)
!2009 = !DIDerivedType(tag: DW_TAG_member, name: "coredump", scope: !1977, file: !257, line: 119, baseType: !1926, size: 64, offset: 1024)
!2010 = !DIDerivedType(tag: DW_TAG_member, name: "p", scope: !1977, file: !257, line: 121, baseType: !2011, size: 64, offset: 1088)
!2011 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2012, size: 64)
!2012 = !DICompositeType(tag: DW_TAG_structure_type, name: "driver_private", file: !257, line: 121, flags: DIFlagFwdDecl)
!2013 = !DIDerivedType(tag: DW_TAG_member, name: "uevent", scope: !1963, file: !1964, line: 85, baseType: !1915, size: 64, offset: 384)
!2014 = !DIDerivedType(tag: DW_TAG_member, name: "probe", scope: !1963, file: !1964, line: 86, baseType: !1935, size: 64, offset: 448)
!2015 = !DIDerivedType(tag: DW_TAG_member, name: "sync_state", scope: !1963, file: !1964, line: 87, baseType: !1926, size: 64, offset: 512)
!2016 = !DIDerivedType(tag: DW_TAG_member, name: "remove", scope: !1963, file: !1964, line: 88, baseType: !1926, size: 64, offset: 576)
!2017 = !DIDerivedType(tag: DW_TAG_member, name: "shutdown", scope: !1963, file: !1964, line: 89, baseType: !1926, size: 64, offset: 640)
!2018 = !DIDerivedType(tag: DW_TAG_member, name: "online", scope: !1963, file: !1964, line: 91, baseType: !1935, size: 64, offset: 704)
!2019 = !DIDerivedType(tag: DW_TAG_member, name: "offline", scope: !1963, file: !1964, line: 92, baseType: !1935, size: 64, offset: 768)
!2020 = !DIDerivedType(tag: DW_TAG_member, name: "suspend", scope: !1963, file: !1964, line: 94, baseType: !1998, size: 64, offset: 832)
!2021 = !DIDerivedType(tag: DW_TAG_member, name: "resume", scope: !1963, file: !1964, line: 95, baseType: !1935, size: 64, offset: 896)
!2022 = !DIDerivedType(tag: DW_TAG_member, name: "num_vf", scope: !1963, file: !1964, line: 97, baseType: !1935, size: 64, offset: 960)
!2023 = !DIDerivedType(tag: DW_TAG_member, name: "dma_configure", scope: !1963, file: !1964, line: 99, baseType: !1935, size: 64, offset: 1024)
!2024 = !DIDerivedType(tag: DW_TAG_member, name: "dma_cleanup", scope: !1963, file: !1964, line: 100, baseType: !1926, size: 64, offset: 1088)
!2025 = !DIDerivedType(tag: DW_TAG_member, name: "pm", scope: !1963, file: !1964, line: 102, baseType: !1930, size: 64, offset: 1152)
!2026 = !DIDerivedType(tag: DW_TAG_member, name: "need_parent_lock", scope: !1963, file: !1964, line: 104, baseType: !614, size: 8, offset: 1216)
!2027 = !DIDerivedType(tag: DW_TAG_member, name: "driver", scope: !1898, file: !263, line: 731, baseType: !2028, size: 64, offset: 832)
!2028 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1977, size: 64)
!2029 = !DIDerivedType(tag: DW_TAG_member, name: "platform_data", scope: !1898, file: !263, line: 733, baseType: !40, size: 64, offset: 896)
!2030 = !DIDerivedType(tag: DW_TAG_member, name: "driver_data", scope: !1898, file: !263, line: 735, baseType: !40, size: 64, offset: 960)
!2031 = !DIDerivedType(tag: DW_TAG_member, name: "mutex", scope: !1898, file: !263, line: 737, baseType: !1277, size: 256, offset: 1024)
!2032 = !DIDerivedType(tag: DW_TAG_member, name: "links", scope: !1898, file: !263, line: 741, baseType: !2033, size: 448, offset: 1280)
!2033 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "dev_links_info", file: !263, line: 531, size: 448, elements: !2034)
!2034 = !{!2035, !2036, !2037, !2038}
!2035 = !DIDerivedType(tag: DW_TAG_member, name: "suppliers", scope: !2033, file: !263, line: 532, baseType: !117, size: 128)
!2036 = !DIDerivedType(tag: DW_TAG_member, name: "consumers", scope: !2033, file: !263, line: 533, baseType: !117, size: 128, offset: 128)
!2037 = !DIDerivedType(tag: DW_TAG_member, name: "defer_sync", scope: !2033, file: !263, line: 534, baseType: !117, size: 128, offset: 256)
!2038 = !DIDerivedType(tag: DW_TAG_member, name: "status", scope: !2033, file: !263, line: 535, baseType: !262, size: 32, offset: 384)
!2039 = !DIDerivedType(tag: DW_TAG_member, name: "power", scope: !1898, file: !263, line: 742, baseType: !2040, size: 2496, offset: 1728)
!2040 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "dev_pm_info", file: !275, line: 663, size: 2496, elements: !2041)
!2041 = !{!2042, !2043, !2044, !2045, !2046, !2047, !2048, !2049, !2050, !2051, !2052, !2053, !2054, !2055, !2056, !2057, !2096, !2097, !2098, !2099, !2100, !2101, !2102, !2170, !2171, !2172, !2173, !2174, !2175, !2176, !2177, !2178, !2179, !2180, !2181, !2182, !2183, !2184, !2185, !2186, !2187, !2188, !2189, !2190, !2191, !2192, !2193, !2194, !2195, !2196, !2197, !2198, !2204, !2208}
!2042 = !DIDerivedType(tag: DW_TAG_member, name: "power_state", scope: !2040, file: !275, line: 664, baseType: !2001, size: 32)
!2043 = !DIDerivedType(tag: DW_TAG_member, name: "can_wakeup", scope: !2040, file: !275, line: 665, baseType: !614, size: 1, offset: 32, flags: DIFlagBitField, extraData: i64 32)
!2044 = !DIDerivedType(tag: DW_TAG_member, name: "async_suspend", scope: !2040, file: !275, line: 666, baseType: !614, size: 1, offset: 33, flags: DIFlagBitField, extraData: i64 32)
!2045 = !DIDerivedType(tag: DW_TAG_member, name: "in_dpm_list", scope: !2040, file: !275, line: 667, baseType: !614, size: 1, offset: 34, flags: DIFlagBitField, extraData: i64 32)
!2046 = !DIDerivedType(tag: DW_TAG_member, name: "is_prepared", scope: !2040, file: !275, line: 668, baseType: !614, size: 1, offset: 35, flags: DIFlagBitField, extraData: i64 32)
!2047 = !DIDerivedType(tag: DW_TAG_member, name: "is_suspended", scope: !2040, file: !275, line: 669, baseType: !614, size: 1, offset: 36, flags: DIFlagBitField, extraData: i64 32)
!2048 = !DIDerivedType(tag: DW_TAG_member, name: "is_noirq_suspended", scope: !2040, file: !275, line: 670, baseType: !614, size: 1, offset: 37, flags: DIFlagBitField, extraData: i64 32)
!2049 = !DIDerivedType(tag: DW_TAG_member, name: "is_late_suspended", scope: !2040, file: !275, line: 671, baseType: !614, size: 1, offset: 38, flags: DIFlagBitField, extraData: i64 32)
!2050 = !DIDerivedType(tag: DW_TAG_member, name: "no_pm", scope: !2040, file: !275, line: 672, baseType: !614, size: 1, offset: 39, flags: DIFlagBitField, extraData: i64 32)
!2051 = !DIDerivedType(tag: DW_TAG_member, name: "early_init", scope: !2040, file: !275, line: 673, baseType: !614, size: 1, offset: 40, flags: DIFlagBitField, extraData: i64 32)
!2052 = !DIDerivedType(tag: DW_TAG_member, name: "direct_complete", scope: !2040, file: !275, line: 674, baseType: !614, size: 1, offset: 41, flags: DIFlagBitField, extraData: i64 32)
!2053 = !DIDerivedType(tag: DW_TAG_member, name: "driver_flags", scope: !2040, file: !275, line: 675, baseType: !578, size: 32, offset: 64)
!2054 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !2040, file: !275, line: 676, baseType: !79, size: 32, offset: 96)
!2055 = !DIDerivedType(tag: DW_TAG_member, name: "entry", scope: !2040, file: !275, line: 678, baseType: !117, size: 128, offset: 128)
!2056 = !DIDerivedType(tag: DW_TAG_member, name: "completion", scope: !2040, file: !275, line: 679, baseType: !139, size: 256, offset: 256)
!2057 = !DIDerivedType(tag: DW_TAG_member, name: "wakeup", scope: !2040, file: !275, line: 680, baseType: !2058, size: 64, offset: 512)
!2058 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2059, size: 64)
!2059 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "wakeup_source", file: !2060, line: 43, size: 1536, elements: !2061)
!2060 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/pm_wakeup.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "894fa74a68f158eeb582d89f247c5b45")
!2061 = !{!2062, !2063, !2064, !2065, !2066, !2069, !2081, !2082, !2084, !2085, !2086, !2087, !2088, !2089, !2090, !2091, !2092, !2093, !2094, !2095}
!2062 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !2059, file: !2060, line: 44, baseType: !36, size: 64)
!2063 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !2059, file: !2060, line: 45, baseType: !42, size: 32, offset: 64)
!2064 = !DIDerivedType(tag: DW_TAG_member, name: "entry", scope: !2059, file: !2060, line: 46, baseType: !117, size: 128, offset: 128)
!2065 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !2059, file: !2060, line: 47, baseType: !79, size: 32, offset: 256)
!2066 = !DIDerivedType(tag: DW_TAG_member, name: "wakeirq", scope: !2059, file: !2060, line: 48, baseType: !2067, size: 64, offset: 320)
!2067 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2068, size: 64)
!2068 = !DICompositeType(tag: DW_TAG_structure_type, name: "wake_irq", file: !275, line: 629, flags: DIFlagFwdDecl)
!2069 = !DIDerivedType(tag: DW_TAG_member, name: "timer", scope: !2059, file: !2060, line: 49, baseType: !2070, size: 320, offset: 384)
!2070 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "timer_list", file: !2071, line: 8, size: 320, elements: !2072)
!2071 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/timer_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "fc644fad45b1423081c7314a2b8c6dd2")
!2072 = !{!2073, !2074, !2075, !2080}
!2073 = !DIDerivedType(tag: DW_TAG_member, name: "entry", scope: !2070, file: !2071, line: 13, baseType: !220, size: 128)
!2074 = !DIDerivedType(tag: DW_TAG_member, name: "expires", scope: !2070, file: !2071, line: 14, baseType: !59, size: 64, offset: 128)
!2075 = !DIDerivedType(tag: DW_TAG_member, name: "function", scope: !2070, file: !2071, line: 15, baseType: !2076, size: 64, offset: 192)
!2076 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2077, size: 64)
!2077 = !DISubroutineType(types: !2078)
!2078 = !{null, !2079}
!2079 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2070, size: 64)
!2080 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !2070, file: !2071, line: 16, baseType: !578, size: 32, offset: 256)
!2081 = !DIDerivedType(tag: DW_TAG_member, name: "timer_expires", scope: !2059, file: !2060, line: 50, baseType: !59, size: 64, offset: 704)
!2082 = !DIDerivedType(tag: DW_TAG_member, name: "total_time", scope: !2059, file: !2060, line: 51, baseType: !2083, size: 64, offset: 768)
!2083 = !DIDerivedType(tag: DW_TAG_typedef, name: "ktime_t", file: !45, line: 124, baseType: !502)
!2084 = !DIDerivedType(tag: DW_TAG_member, name: "max_time", scope: !2059, file: !2060, line: 52, baseType: !2083, size: 64, offset: 832)
!2085 = !DIDerivedType(tag: DW_TAG_member, name: "last_time", scope: !2059, file: !2060, line: 53, baseType: !2083, size: 64, offset: 896)
!2086 = !DIDerivedType(tag: DW_TAG_member, name: "start_prevent_time", scope: !2059, file: !2060, line: 54, baseType: !2083, size: 64, offset: 960)
!2087 = !DIDerivedType(tag: DW_TAG_member, name: "prevent_sleep_time", scope: !2059, file: !2060, line: 55, baseType: !2083, size: 64, offset: 1024)
!2088 = !DIDerivedType(tag: DW_TAG_member, name: "event_count", scope: !2059, file: !2060, line: 56, baseType: !59, size: 64, offset: 1088)
!2089 = !DIDerivedType(tag: DW_TAG_member, name: "active_count", scope: !2059, file: !2060, line: 57, baseType: !59, size: 64, offset: 1152)
!2090 = !DIDerivedType(tag: DW_TAG_member, name: "relax_count", scope: !2059, file: !2060, line: 58, baseType: !59, size: 64, offset: 1216)
!2091 = !DIDerivedType(tag: DW_TAG_member, name: "expire_count", scope: !2059, file: !2060, line: 59, baseType: !59, size: 64, offset: 1280)
!2092 = !DIDerivedType(tag: DW_TAG_member, name: "wakeup_count", scope: !2059, file: !2060, line: 60, baseType: !59, size: 64, offset: 1344)
!2093 = !DIDerivedType(tag: DW_TAG_member, name: "dev", scope: !2059, file: !2060, line: 61, baseType: !1902, size: 64, offset: 1408)
!2094 = !DIDerivedType(tag: DW_TAG_member, name: "active", scope: !2059, file: !2060, line: 62, baseType: !614, size: 1, offset: 1472, flags: DIFlagBitField, extraData: i64 1472)
!2095 = !DIDerivedType(tag: DW_TAG_member, name: "autosleep_enabled", scope: !2059, file: !2060, line: 63, baseType: !614, size: 1, offset: 1473, flags: DIFlagBitField, extraData: i64 1472)
!2096 = !DIDerivedType(tag: DW_TAG_member, name: "wakeup_path", scope: !2040, file: !275, line: 681, baseType: !614, size: 1, offset: 576, flags: DIFlagBitField, extraData: i64 576)
!2097 = !DIDerivedType(tag: DW_TAG_member, name: "syscore", scope: !2040, file: !275, line: 682, baseType: !614, size: 1, offset: 577, flags: DIFlagBitField, extraData: i64 576)
!2098 = !DIDerivedType(tag: DW_TAG_member, name: "no_pm_callbacks", scope: !2040, file: !275, line: 683, baseType: !614, size: 1, offset: 578, flags: DIFlagBitField, extraData: i64 576)
!2099 = !DIDerivedType(tag: DW_TAG_member, name: "async_in_progress", scope: !2040, file: !275, line: 684, baseType: !614, size: 1, offset: 579, flags: DIFlagBitField, extraData: i64 576)
!2100 = !DIDerivedType(tag: DW_TAG_member, name: "must_resume", scope: !2040, file: !275, line: 685, baseType: !614, size: 1, offset: 580, flags: DIFlagBitField, extraData: i64 576)
!2101 = !DIDerivedType(tag: DW_TAG_member, name: "may_skip_resume", scope: !2040, file: !275, line: 686, baseType: !614, size: 1, offset: 581, flags: DIFlagBitField, extraData: i64 576)
!2102 = !DIDerivedType(tag: DW_TAG_member, name: "suspend_timer", scope: !2040, file: !275, line: 691, baseType: !2103, size: 512, offset: 640)
!2103 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "hrtimer", file: !270, line: 39, size: 512, elements: !2104)
!2104 = !{!2105, !2111, !2112, !2117, !2166, !2167, !2168, !2169}
!2105 = !DIDerivedType(tag: DW_TAG_member, name: "node", scope: !2103, file: !270, line: 40, baseType: !2106, size: 256)
!2106 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "timerqueue_node", file: !2107, line: 8, size: 256, elements: !2108)
!2107 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/timerqueue_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "f9375894325e953a6569222d3fba0aa9")
!2108 = !{!2109, !2110}
!2109 = !DIDerivedType(tag: DW_TAG_member, name: "node", scope: !2106, file: !2107, line: 9, baseType: !173, size: 192, align: 64)
!2110 = !DIDerivedType(tag: DW_TAG_member, name: "expires", scope: !2106, file: !2107, line: 10, baseType: !2083, size: 64, offset: 192)
!2111 = !DIDerivedType(tag: DW_TAG_member, name: "_softexpires", scope: !2103, file: !270, line: 41, baseType: !2083, size: 64, offset: 256)
!2112 = !DIDerivedType(tag: DW_TAG_member, name: "function", scope: !2103, file: !270, line: 42, baseType: !2113, size: 64, offset: 320)
!2113 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2114, size: 64)
!2114 = !DISubroutineType(types: !2115)
!2115 = !{!269, !2116}
!2116 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2103, size: 64)
!2117 = !DIDerivedType(tag: DW_TAG_member, name: "base", scope: !2103, file: !270, line: 43, baseType: !2118, size: 64, offset: 384)
!2118 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2119, size: 64)
!2119 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "hrtimer_clock_base", file: !2120, line: 47, size: 512, align: 512, elements: !2121)
!2120 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/hrtimer_defs.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "98908212b1fe86506e1c8a096da6477f")
!2121 = !{!2122, !2147, !2148, !2151, !2156, !2157, !2161, !2165}
!2122 = !DIDerivedType(tag: DW_TAG_member, name: "cpu_base", scope: !2119, file: !2120, line: 48, baseType: !2123, size: 64)
!2123 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2124, size: 64)
!2124 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "hrtimer_cpu_base", file: !2120, line: 103, size: 4608, align: 512, elements: !2125)
!2125 = !{!2126, !2127, !2128, !2129, !2130, !2131, !2132, !2133, !2134, !2135, !2136, !2137, !2138, !2139, !2140, !2141, !2142, !2143}
!2126 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !2124, file: !2120, line: 104, baseType: !148, size: 32)
!2127 = !DIDerivedType(tag: DW_TAG_member, name: "cpu", scope: !2124, file: !2120, line: 105, baseType: !7, size: 32, offset: 32)
!2128 = !DIDerivedType(tag: DW_TAG_member, name: "active_bases", scope: !2124, file: !2120, line: 106, baseType: !7, size: 32, offset: 64)
!2129 = !DIDerivedType(tag: DW_TAG_member, name: "clock_was_set_seq", scope: !2124, file: !2120, line: 107, baseType: !7, size: 32, offset: 96)
!2130 = !DIDerivedType(tag: DW_TAG_member, name: "hres_active", scope: !2124, file: !2120, line: 108, baseType: !7, size: 1, offset: 128, flags: DIFlagBitField, extraData: i64 128)
!2131 = !DIDerivedType(tag: DW_TAG_member, name: "in_hrtirq", scope: !2124, file: !2120, line: 109, baseType: !7, size: 1, offset: 129, flags: DIFlagBitField, extraData: i64 128)
!2132 = !DIDerivedType(tag: DW_TAG_member, name: "hang_detected", scope: !2124, file: !2120, line: 110, baseType: !7, size: 1, offset: 130, flags: DIFlagBitField, extraData: i64 128)
!2133 = !DIDerivedType(tag: DW_TAG_member, name: "softirq_activated", scope: !2124, file: !2120, line: 111, baseType: !7, size: 1, offset: 131, flags: DIFlagBitField, extraData: i64 128)
!2134 = !DIDerivedType(tag: DW_TAG_member, name: "online", scope: !2124, file: !2120, line: 112, baseType: !7, size: 1, offset: 132, flags: DIFlagBitField, extraData: i64 128)
!2135 = !DIDerivedType(tag: DW_TAG_member, name: "nr_events", scope: !2124, file: !2120, line: 114, baseType: !7, size: 32, offset: 160)
!2136 = !DIDerivedType(tag: DW_TAG_member, name: "nr_retries", scope: !2124, file: !2120, line: 115, baseType: !46, size: 16, offset: 192)
!2137 = !DIDerivedType(tag: DW_TAG_member, name: "nr_hangs", scope: !2124, file: !2120, line: 116, baseType: !46, size: 16, offset: 208)
!2138 = !DIDerivedType(tag: DW_TAG_member, name: "max_hang_time", scope: !2124, file: !2120, line: 117, baseType: !7, size: 32, offset: 224)
!2139 = !DIDerivedType(tag: DW_TAG_member, name: "expires_next", scope: !2124, file: !2120, line: 123, baseType: !2083, size: 64, offset: 256)
!2140 = !DIDerivedType(tag: DW_TAG_member, name: "next_timer", scope: !2124, file: !2120, line: 124, baseType: !2116, size: 64, offset: 320)
!2141 = !DIDerivedType(tag: DW_TAG_member, name: "softirq_expires_next", scope: !2124, file: !2120, line: 125, baseType: !2083, size: 64, offset: 384)
!2142 = !DIDerivedType(tag: DW_TAG_member, name: "softirq_next_timer", scope: !2124, file: !2120, line: 126, baseType: !2116, size: 64, offset: 448)
!2143 = !DIDerivedType(tag: DW_TAG_member, name: "clock_base", scope: !2124, file: !2120, line: 127, baseType: !2144, size: 4096, align: 512, offset: 512)
!2144 = !DICompositeType(tag: DW_TAG_array_type, baseType: !2119, size: 4096, align: 512, elements: !2145)
!2145 = !{!2146}
!2146 = !DISubrange(count: 8)
!2147 = !DIDerivedType(tag: DW_TAG_member, name: "index", scope: !2119, file: !2120, line: 49, baseType: !7, size: 32, offset: 64)
!2148 = !DIDerivedType(tag: DW_TAG_member, name: "clockid", scope: !2119, file: !2120, line: 50, baseType: !2149, size: 32, offset: 96)
!2149 = !DIDerivedType(tag: DW_TAG_typedef, name: "clockid_t", file: !45, line: 32, baseType: !2150)
!2150 = !DIDerivedType(tag: DW_TAG_typedef, name: "__kernel_clockid_t", file: !57, line: 96, baseType: !42)
!2151 = !DIDerivedType(tag: DW_TAG_member, name: "seq", scope: !2119, file: !2120, line: 51, baseType: !2152, size: 32, offset: 128)
!2152 = !DIDerivedType(tag: DW_TAG_typedef, name: "seqcount_raw_spinlock_t", file: !746, line: 68, baseType: !2153)
!2153 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "seqcount_raw_spinlock", file: !746, line: 68, size: 32, elements: !2154)
!2154 = !{!2155}
!2155 = !DIDerivedType(tag: DW_TAG_member, name: "seqcount", scope: !2153, file: !746, line: 68, baseType: !750, size: 32)
!2156 = !DIDerivedType(tag: DW_TAG_member, name: "running", scope: !2119, file: !2120, line: 52, baseType: !2116, size: 64, offset: 192)
!2157 = !DIDerivedType(tag: DW_TAG_member, name: "active", scope: !2119, file: !2120, line: 53, baseType: !2158, size: 128, offset: 256)
!2158 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "timerqueue_head", file: !2107, line: 13, size: 128, elements: !2159)
!2159 = !{!2160}
!2160 = !DIDerivedType(tag: DW_TAG_member, name: "rb_root", scope: !2158, file: !2107, line: 14, baseType: !1045, size: 128)
!2161 = !DIDerivedType(tag: DW_TAG_member, name: "get_time", scope: !2119, file: !2120, line: 54, baseType: !2162, size: 64, offset: 384)
!2162 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2163, size: 64)
!2163 = !DISubroutineType(types: !2164)
!2164 = !{!2083}
!2165 = !DIDerivedType(tag: DW_TAG_member, name: "offset", scope: !2119, file: !2120, line: 55, baseType: !2083, size: 64, offset: 448)
!2166 = !DIDerivedType(tag: DW_TAG_member, name: "state", scope: !2103, file: !270, line: 44, baseType: !103, size: 8, offset: 448)
!2167 = !DIDerivedType(tag: DW_TAG_member, name: "is_rel", scope: !2103, file: !270, line: 45, baseType: !103, size: 8, offset: 456)
!2168 = !DIDerivedType(tag: DW_TAG_member, name: "is_soft", scope: !2103, file: !270, line: 46, baseType: !103, size: 8, offset: 464)
!2169 = !DIDerivedType(tag: DW_TAG_member, name: "is_hard", scope: !2103, file: !270, line: 47, baseType: !103, size: 8, offset: 472)
!2170 = !DIDerivedType(tag: DW_TAG_member, name: "timer_expires", scope: !2040, file: !275, line: 692, baseType: !519, size: 64, offset: 1152)
!2171 = !DIDerivedType(tag: DW_TAG_member, name: "work", scope: !2040, file: !275, line: 693, baseType: !1337, size: 256, offset: 1216)
!2172 = !DIDerivedType(tag: DW_TAG_member, name: "wait_queue", scope: !2040, file: !275, line: 694, baseType: !74, size: 192, offset: 1472)
!2173 = !DIDerivedType(tag: DW_TAG_member, name: "wakeirq", scope: !2040, file: !275, line: 695, baseType: !2067, size: 64, offset: 1664)
!2174 = !DIDerivedType(tag: DW_TAG_member, name: "usage_count", scope: !2040, file: !275, line: 696, baseType: !69, size: 32, offset: 1728)
!2175 = !DIDerivedType(tag: DW_TAG_member, name: "child_count", scope: !2040, file: !275, line: 697, baseType: !69, size: 32, offset: 1760)
!2176 = !DIDerivedType(tag: DW_TAG_member, name: "disable_depth", scope: !2040, file: !275, line: 698, baseType: !7, size: 3, offset: 1792, flags: DIFlagBitField, extraData: i64 1792)
!2177 = !DIDerivedType(tag: DW_TAG_member, name: "idle_notification", scope: !2040, file: !275, line: 699, baseType: !614, size: 1, offset: 1795, flags: DIFlagBitField, extraData: i64 1792)
!2178 = !DIDerivedType(tag: DW_TAG_member, name: "request_pending", scope: !2040, file: !275, line: 700, baseType: !614, size: 1, offset: 1796, flags: DIFlagBitField, extraData: i64 1792)
!2179 = !DIDerivedType(tag: DW_TAG_member, name: "deferred_resume", scope: !2040, file: !275, line: 701, baseType: !614, size: 1, offset: 1797, flags: DIFlagBitField, extraData: i64 1792)
!2180 = !DIDerivedType(tag: DW_TAG_member, name: "needs_force_resume", scope: !2040, file: !275, line: 702, baseType: !614, size: 1, offset: 1798, flags: DIFlagBitField, extraData: i64 1792)
!2181 = !DIDerivedType(tag: DW_TAG_member, name: "runtime_auto", scope: !2040, file: !275, line: 703, baseType: !614, size: 1, offset: 1799, flags: DIFlagBitField, extraData: i64 1792)
!2182 = !DIDerivedType(tag: DW_TAG_member, name: "ignore_children", scope: !2040, file: !275, line: 704, baseType: !614, size: 1, offset: 1800, flags: DIFlagBitField, extraData: i64 1792)
!2183 = !DIDerivedType(tag: DW_TAG_member, name: "no_callbacks", scope: !2040, file: !275, line: 705, baseType: !614, size: 1, offset: 1801, flags: DIFlagBitField, extraData: i64 1792)
!2184 = !DIDerivedType(tag: DW_TAG_member, name: "irq_safe", scope: !2040, file: !275, line: 706, baseType: !614, size: 1, offset: 1802, flags: DIFlagBitField, extraData: i64 1792)
!2185 = !DIDerivedType(tag: DW_TAG_member, name: "use_autosuspend", scope: !2040, file: !275, line: 707, baseType: !614, size: 1, offset: 1803, flags: DIFlagBitField, extraData: i64 1792)
!2186 = !DIDerivedType(tag: DW_TAG_member, name: "timer_autosuspends", scope: !2040, file: !275, line: 708, baseType: !614, size: 1, offset: 1804, flags: DIFlagBitField, extraData: i64 1792)
!2187 = !DIDerivedType(tag: DW_TAG_member, name: "memalloc_noio", scope: !2040, file: !275, line: 709, baseType: !614, size: 1, offset: 1805, flags: DIFlagBitField, extraData: i64 1792)
!2188 = !DIDerivedType(tag: DW_TAG_member, name: "links_count", scope: !2040, file: !275, line: 710, baseType: !7, size: 32, offset: 1824)
!2189 = !DIDerivedType(tag: DW_TAG_member, name: "request", scope: !2040, file: !275, line: 711, baseType: !274, size: 32, offset: 1856)
!2190 = !DIDerivedType(tag: DW_TAG_member, name: "runtime_status", scope: !2040, file: !275, line: 712, baseType: !282, size: 32, offset: 1888)
!2191 = !DIDerivedType(tag: DW_TAG_member, name: "last_status", scope: !2040, file: !275, line: 713, baseType: !282, size: 32, offset: 1920)
!2192 = !DIDerivedType(tag: DW_TAG_member, name: "runtime_error", scope: !2040, file: !275, line: 714, baseType: !42, size: 32, offset: 1952)
!2193 = !DIDerivedType(tag: DW_TAG_member, name: "autosuspend_delay", scope: !2040, file: !275, line: 715, baseType: !42, size: 32, offset: 1984)
!2194 = !DIDerivedType(tag: DW_TAG_member, name: "last_busy", scope: !2040, file: !275, line: 716, baseType: !519, size: 64, offset: 2048)
!2195 = !DIDerivedType(tag: DW_TAG_member, name: "active_time", scope: !2040, file: !275, line: 717, baseType: !519, size: 64, offset: 2112)
!2196 = !DIDerivedType(tag: DW_TAG_member, name: "suspended_time", scope: !2040, file: !275, line: 718, baseType: !519, size: 64, offset: 2176)
!2197 = !DIDerivedType(tag: DW_TAG_member, name: "accounting_timestamp", scope: !2040, file: !275, line: 719, baseType: !519, size: 64, offset: 2240)
!2198 = !DIDerivedType(tag: DW_TAG_member, name: "subsys_data", scope: !2040, file: !275, line: 721, baseType: !2199, size: 64, offset: 2304)
!2199 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2200, size: 64)
!2200 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "pm_subsys_data", file: !275, line: 632, size: 64, elements: !2201)
!2201 = !{!2202, !2203}
!2202 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !2200, file: !275, line: 633, baseType: !79, size: 32)
!2203 = !DIDerivedType(tag: DW_TAG_member, name: "refcount", scope: !2200, file: !275, line: 634, baseType: !7, size: 32, offset: 32)
!2204 = !DIDerivedType(tag: DW_TAG_member, name: "set_latency_tolerance", scope: !2040, file: !275, line: 722, baseType: !2205, size: 64, offset: 2368)
!2205 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2206, size: 64)
!2206 = !DISubroutineType(types: !2207)
!2207 = !{null, !1902, !541}
!2208 = !DIDerivedType(tag: DW_TAG_member, name: "qos", scope: !2040, file: !275, line: 723, baseType: !2209, size: 64, offset: 2432)
!2209 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2210, size: 64)
!2210 = !DICompositeType(tag: DW_TAG_structure_type, name: "dev_pm_qos", file: !275, line: 723, flags: DIFlagFwdDecl)
!2211 = !DIDerivedType(tag: DW_TAG_member, name: "pm_domain", scope: !1898, file: !263, line: 743, baseType: !2212, size: 64, offset: 4224)
!2212 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2213, size: 64)
!2213 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "dev_pm_domain", file: !275, line: 744, size: 1856, elements: !2214)
!2214 = !{!2215, !2216, !2217, !2221, !2222, !2223, !2224}
!2215 = !DIDerivedType(tag: DW_TAG_member, name: "ops", scope: !2213, file: !275, line: 745, baseType: !1932, size: 1472)
!2216 = !DIDerivedType(tag: DW_TAG_member, name: "start", scope: !2213, file: !275, line: 746, baseType: !1935, size: 64, offset: 1472)
!2217 = !DIDerivedType(tag: DW_TAG_member, name: "detach", scope: !2213, file: !275, line: 747, baseType: !2218, size: 64, offset: 1536)
!2218 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2219, size: 64)
!2219 = !DISubroutineType(types: !2220)
!2220 = !{null, !1902, !614}
!2221 = !DIDerivedType(tag: DW_TAG_member, name: "activate", scope: !2213, file: !275, line: 748, baseType: !1935, size: 64, offset: 1600)
!2222 = !DIDerivedType(tag: DW_TAG_member, name: "sync", scope: !2213, file: !275, line: 749, baseType: !1926, size: 64, offset: 1664)
!2223 = !DIDerivedType(tag: DW_TAG_member, name: "dismiss", scope: !2213, file: !275, line: 750, baseType: !1926, size: 64, offset: 1728)
!2224 = !DIDerivedType(tag: DW_TAG_member, name: "set_performance_state", scope: !2213, file: !275, line: 751, baseType: !2225, size: 64, offset: 1792)
!2225 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2226, size: 64)
!2226 = !DISubroutineType(types: !2227)
!2227 = !{!42, !1902, !7}
!2228 = !DIDerivedType(tag: DW_TAG_member, name: "msi", scope: !1898, file: !263, line: 752, baseType: !2229, size: 128, offset: 4288)
!2229 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "dev_msi_info", file: !263, line: 543, size: 128, elements: !2230)
!2230 = !{!2231, !2235}
!2231 = !DIDerivedType(tag: DW_TAG_member, name: "domain", scope: !2229, file: !263, line: 545, baseType: !2232, size: 64)
!2232 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2233, size: 64)
!2233 = !DICompositeType(tag: DW_TAG_structure_type, name: "irq_domain", file: !2234, line: 11, flags: DIFlagFwdDecl)
!2234 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/x86_init.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "5b228dfc7e83ca06eb9386ac665c8fb4")
!2235 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !2229, file: !263, line: 546, baseType: !2236, size: 64, offset: 64)
!2236 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2237, size: 64)
!2237 = !DICompositeType(tag: DW_TAG_structure_type, name: "msi_device_data", file: !263, line: 48, flags: DIFlagFwdDecl)
!2238 = !DIDerivedType(tag: DW_TAG_member, name: "dma_mask", scope: !1898, file: !263, line: 756, baseType: !2239, size: 64, offset: 4416)
!2239 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !519, size: 64)
!2240 = !DIDerivedType(tag: DW_TAG_member, name: "coherent_dma_mask", scope: !1898, file: !263, line: 757, baseType: !519, size: 64, offset: 4480)
!2241 = !DIDerivedType(tag: DW_TAG_member, name: "bus_dma_limit", scope: !1898, file: !263, line: 762, baseType: !519, size: 64, offset: 4544)
!2242 = !DIDerivedType(tag: DW_TAG_member, name: "dma_range_map", scope: !1898, file: !263, line: 763, baseType: !2243, size: 64, offset: 4608)
!2243 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2244, size: 64)
!2244 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !2245)
!2245 = !DICompositeType(tag: DW_TAG_structure_type, name: "bus_dma_region", file: !263, line: 763, flags: DIFlagFwdDecl)
!2246 = !DIDerivedType(tag: DW_TAG_member, name: "dma_parms", scope: !1898, file: !263, line: 765, baseType: !2247, size: 64, offset: 4672)
!2247 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2248, size: 64)
!2248 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "device_dma_parameters", file: !263, line: 442, size: 128, elements: !2249)
!2249 = !{!2250, !2251, !2252}
!2250 = !DIDerivedType(tag: DW_TAG_member, name: "max_segment_size", scope: !2248, file: !263, line: 447, baseType: !7, size: 32)
!2251 = !DIDerivedType(tag: DW_TAG_member, name: "min_align_mask", scope: !2248, file: !263, line: 448, baseType: !7, size: 32, offset: 32)
!2252 = !DIDerivedType(tag: DW_TAG_member, name: "segment_boundary_mask", scope: !2248, file: !263, line: 449, baseType: !59, size: 64, offset: 64)
!2253 = !DIDerivedType(tag: DW_TAG_member, name: "dma_pools", scope: !1898, file: !263, line: 767, baseType: !117, size: 128, offset: 4736)
!2254 = !DIDerivedType(tag: DW_TAG_member, name: "dma_io_tlb_mem", scope: !1898, file: !263, line: 778, baseType: !2255, size: 64, offset: 4864)
!2255 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2256, size: 64)
!2256 = !DICompositeType(tag: DW_TAG_structure_type, name: "io_tlb_mem", file: !263, line: 778, flags: DIFlagFwdDecl)
!2257 = !DIDerivedType(tag: DW_TAG_member, name: "archdata", scope: !1898, file: !263, line: 786, baseType: !2258, offset: 4928)
!2258 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "dev_archdata", file: !2259, line: 5, elements: !1201)
!2259 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/device.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "f8292cdac17859a8f96a30fcf44b1867")
!2260 = !DIDerivedType(tag: DW_TAG_member, name: "of_node", scope: !1898, file: !263, line: 788, baseType: !2261, size: 64, offset: 4928)
!2261 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2262, size: 64)
!2262 = !DICompositeType(tag: DW_TAG_structure_type, name: "device_node", file: !2263, line: 18, flags: DIFlagFwdDecl)
!2263 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/arch_topology.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "b971b9ef95a9c0ec7b2f2b693552714a")
!2264 = !DIDerivedType(tag: DW_TAG_member, name: "fwnode", scope: !1898, file: !263, line: 789, baseType: !2265, size: 64, offset: 4992)
!2265 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2266, size: 64)
!2266 = !DICompositeType(tag: DW_TAG_structure_type, name: "fwnode_handle", file: !1964, line: 22, flags: DIFlagFwdDecl)
!2267 = !DIDerivedType(tag: DW_TAG_member, name: "numa_node", scope: !1898, file: !263, line: 792, baseType: !42, size: 32, offset: 5056)
!2268 = !DIDerivedType(tag: DW_TAG_member, name: "devt", scope: !1898, file: !263, line: 794, baseType: !852, size: 32, offset: 5088)
!2269 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !1898, file: !263, line: 795, baseType: !578, size: 32, offset: 5120)
!2270 = !DIDerivedType(tag: DW_TAG_member, name: "devres_lock", scope: !1898, file: !263, line: 797, baseType: !79, size: 32, offset: 5152)
!2271 = !DIDerivedType(tag: DW_TAG_member, name: "devres_head", scope: !1898, file: !263, line: 798, baseType: !117, size: 128, offset: 5184)
!2272 = !DIDerivedType(tag: DW_TAG_member, name: "class", scope: !1898, file: !263, line: 800, baseType: !2273, size: 64, offset: 5312)
!2273 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2274, size: 64)
!2274 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !2275)
!2275 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "class", file: !2276, line: 50, size: 768, elements: !2277)
!2276 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/device/class.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "820f9898eebaa676b0630a051415fe4a")
!2277 = !{!2278, !2279, !2280, !2281, !2282, !2286, !2290, !2291, !2292, !2318, !2322, !2326}
!2278 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !2275, file: !2276, line: 51, baseType: !36, size: 64)
!2279 = !DIDerivedType(tag: DW_TAG_member, name: "class_groups", scope: !2275, file: !2276, line: 53, baseType: !1006, size: 64, offset: 64)
!2280 = !DIDerivedType(tag: DW_TAG_member, name: "dev_groups", scope: !2275, file: !2276, line: 54, baseType: !1006, size: 64, offset: 128)
!2281 = !DIDerivedType(tag: DW_TAG_member, name: "dev_uevent", scope: !2275, file: !2276, line: 56, baseType: !1915, size: 64, offset: 192)
!2282 = !DIDerivedType(tag: DW_TAG_member, name: "devnode", scope: !2275, file: !2276, line: 57, baseType: !2283, size: 64, offset: 256)
!2283 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2284, size: 64)
!2284 = !DISubroutineType(types: !2285)
!2285 = !{!625, !1918, !1924}
!2286 = !DIDerivedType(tag: DW_TAG_member, name: "class_release", scope: !2275, file: !2276, line: 59, baseType: !2287, size: 64, offset: 320)
!2287 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2288, size: 64)
!2288 = !DISubroutineType(types: !2289)
!2289 = !{null, !2273}
!2290 = !DIDerivedType(tag: DW_TAG_member, name: "dev_release", scope: !2275, file: !2276, line: 60, baseType: !1926, size: 64, offset: 384)
!2291 = !DIDerivedType(tag: DW_TAG_member, name: "shutdown_pre", scope: !2275, file: !2276, line: 62, baseType: !1935, size: 64, offset: 448)
!2292 = !DIDerivedType(tag: DW_TAG_member, name: "ns_type", scope: !2275, file: !2276, line: 64, baseType: !2293, size: 64, offset: 512)
!2293 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2294, size: 64)
!2294 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !2295)
!2295 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kobj_ns_type_operations", file: !290, line: 39, size: 384, elements: !2296)
!2296 = !{!2297, !2298, !2302, !2306, !2313, !2317}
!2297 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !2295, file: !290, line: 40, baseType: !289, size: 32)
!2298 = !DIDerivedType(tag: DW_TAG_member, name: "current_may_mount", scope: !2295, file: !290, line: 41, baseType: !2299, size: 64, offset: 64)
!2299 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2300, size: 64)
!2300 = !DISubroutineType(types: !2301)
!2301 = !{!614}
!2302 = !DIDerivedType(tag: DW_TAG_member, name: "grab_current_ns", scope: !2295, file: !290, line: 42, baseType: !2303, size: 64, offset: 128)
!2303 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2304, size: 64)
!2304 = !DISubroutineType(types: !2305)
!2305 = !{!40}
!2306 = !DIDerivedType(tag: DW_TAG_member, name: "netlink_ns", scope: !2295, file: !290, line: 43, baseType: !2307, size: 64, offset: 192)
!2307 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2308, size: 64)
!2308 = !DISubroutineType(types: !2309)
!2309 = !{!1298, !2310}
!2310 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2311, size: 64)
!2311 = !DICompositeType(tag: DW_TAG_structure_type, name: "sock", file: !2312, line: 17, flags: DIFlagFwdDecl)
!2312 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/socket.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "336f7d78374d4dae70dccbcd043dbe60")
!2313 = !DIDerivedType(tag: DW_TAG_member, name: "initial_ns", scope: !2295, file: !290, line: 44, baseType: !2314, size: 64, offset: 256)
!2314 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2315, size: 64)
!2315 = !DISubroutineType(types: !2316)
!2316 = !{!1298}
!2317 = !DIDerivedType(tag: DW_TAG_member, name: "drop_ns", scope: !2295, file: !290, line: 45, baseType: !809, size: 64, offset: 320)
!2318 = !DIDerivedType(tag: DW_TAG_member, name: "namespace", scope: !2275, file: !2276, line: 65, baseType: !2319, size: 64, offset: 576)
!2319 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2320, size: 64)
!2320 = !DISubroutineType(types: !2321)
!2321 = !{!1298, !1918}
!2322 = !DIDerivedType(tag: DW_TAG_member, name: "get_ownership", scope: !2275, file: !2276, line: 67, baseType: !2323, size: 64, offset: 640)
!2323 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2324, size: 64)
!2324 = !DISubroutineType(types: !2325)
!2325 = !{null, !1918, !187, !195}
!2326 = !DIDerivedType(tag: DW_TAG_member, name: "pm", scope: !2275, file: !2276, line: 69, baseType: !1930, size: 64, offset: 704)
!2327 = !DIDerivedType(tag: DW_TAG_member, name: "groups", scope: !1898, file: !263, line: 801, baseType: !1006, size: 64, offset: 5376)
!2328 = !DIDerivedType(tag: DW_TAG_member, name: "release", scope: !1898, file: !263, line: 803, baseType: !1926, size: 64, offset: 5440)
!2329 = !DIDerivedType(tag: DW_TAG_member, name: "iommu_group", scope: !1898, file: !263, line: 804, baseType: !2330, size: 64, offset: 5504)
!2330 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2331, size: 64)
!2331 = !DICompositeType(tag: DW_TAG_structure_type, name: "iommu_group", file: !263, line: 45, flags: DIFlagFwdDecl)
!2332 = !DIDerivedType(tag: DW_TAG_member, name: "iommu", scope: !1898, file: !263, line: 805, baseType: !2333, size: 64, offset: 5568)
!2333 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2334, size: 64)
!2334 = !DICompositeType(tag: DW_TAG_structure_type, name: "dev_iommu", file: !263, line: 47, flags: DIFlagFwdDecl)
!2335 = !DIDerivedType(tag: DW_TAG_member, name: "physical_location", scope: !1898, file: !263, line: 807, baseType: !2336, size: 64, offset: 5632)
!2336 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2337, size: 64)
!2337 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "device_physical_location", file: !263, line: 611, size: 128, elements: !2338)
!2338 = !{!2339, !2340, !2341, !2342, !2343}
!2339 = !DIDerivedType(tag: DW_TAG_member, name: "panel", scope: !2337, file: !263, line: 612, baseType: !295, size: 32)
!2340 = !DIDerivedType(tag: DW_TAG_member, name: "vertical_position", scope: !2337, file: !263, line: 613, baseType: !304, size: 32, offset: 32)
!2341 = !DIDerivedType(tag: DW_TAG_member, name: "horizontal_position", scope: !2337, file: !263, line: 614, baseType: !309, size: 32, offset: 64)
!2342 = !DIDerivedType(tag: DW_TAG_member, name: "dock", scope: !2337, file: !263, line: 615, baseType: !614, size: 8, offset: 96)
!2343 = !DIDerivedType(tag: DW_TAG_member, name: "lid", scope: !2337, file: !263, line: 616, baseType: !614, size: 8, offset: 104)
!2344 = !DIDerivedType(tag: DW_TAG_member, name: "removable", scope: !1898, file: !263, line: 809, baseType: !314, size: 32, offset: 5696)
!2345 = !DIDerivedType(tag: DW_TAG_member, name: "offline_disabled", scope: !1898, file: !263, line: 811, baseType: !614, size: 1, offset: 5728, flags: DIFlagBitField, extraData: i64 5728)
!2346 = !DIDerivedType(tag: DW_TAG_member, name: "offline", scope: !1898, file: !263, line: 812, baseType: !614, size: 1, offset: 5729, flags: DIFlagBitField, extraData: i64 5728)
!2347 = !DIDerivedType(tag: DW_TAG_member, name: "of_node_reused", scope: !1898, file: !263, line: 813, baseType: !614, size: 1, offset: 5730, flags: DIFlagBitField, extraData: i64 5728)
!2348 = !DIDerivedType(tag: DW_TAG_member, name: "state_synced", scope: !1898, file: !263, line: 814, baseType: !614, size: 1, offset: 5731, flags: DIFlagBitField, extraData: i64 5728)
!2349 = !DIDerivedType(tag: DW_TAG_member, name: "can_match", scope: !1898, file: !263, line: 815, baseType: !614, size: 1, offset: 5732, flags: DIFlagBitField, extraData: i64 5728)
!2350 = !DIDerivedType(tag: DW_TAG_member, name: "dma_skip_sync", scope: !1898, file: !263, line: 825, baseType: !614, size: 1, offset: 5733, flags: DIFlagBitField, extraData: i64 5728)
!2351 = !DIDerivedType(tag: DW_TAG_member, name: "dma_iommu", scope: !1898, file: !263, line: 828, baseType: !614, size: 1, offset: 5734, flags: DIFlagBitField, extraData: i64 5728)
!2352 = !DIDerivedType(tag: DW_TAG_member, name: "swap_file", scope: !1807, file: !1808, line: 318, baseType: !896, size: 64, offset: 1856)
!2353 = !DIDerivedType(tag: DW_TAG_member, name: "comp", scope: !1807, file: !1808, line: 319, baseType: !139, size: 256, offset: 1920)
!2354 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !1807, file: !1808, line: 320, baseType: !79, size: 32, offset: 2176)
!2355 = !DIDerivedType(tag: DW_TAG_member, name: "cont_lock", scope: !1807, file: !1808, line: 333, baseType: !79, size: 32, offset: 2208)
!2356 = !DIDerivedType(tag: DW_TAG_member, name: "discard_work", scope: !1807, file: !1808, line: 337, baseType: !1337, size: 256, offset: 2240)
!2357 = !DIDerivedType(tag: DW_TAG_member, name: "discard_clusters", scope: !1807, file: !1808, line: 338, baseType: !117, size: 128, offset: 2496)
!2358 = !DIDerivedType(tag: DW_TAG_member, name: "avail_lists", scope: !1807, file: !1808, line: 339, baseType: !2359, offset: 2624)
!2359 = !DICompositeType(tag: DW_TAG_array_type, baseType: !1814, elements: !1353)
!2360 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1670, size: 64)
!2361 = !DIDerivedType(tag: DW_TAG_member, name: "swap_deactivate", scope: !1054, file: !342, line: 437, baseType: !2362, size: 64, offset: 1152)
!2362 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2363, size: 64)
!2363 = !DISubroutineType(types: !2364)
!2364 = !{null, !896}
!2365 = !DIDerivedType(tag: DW_TAG_member, name: "swap_rw", scope: !1054, file: !342, line: 438, baseType: !2366, size: 64, offset: 1216)
!2366 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2367, size: 64)
!2367 = !DISubroutineType(types: !2368)
!2368 = !{!42, !1687, !1725}
!2369 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !1031, file: !342, line: 479, baseType: !59, size: 64, offset: 896)
!2370 = !DIDerivedType(tag: DW_TAG_member, name: "wb_err", scope: !1031, file: !342, line: 480, baseType: !2371, size: 32, offset: 960)
!2371 = !DIDerivedType(tag: DW_TAG_typedef, name: "errseq_t", file: !2372, line: 8, baseType: !578)
!2372 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/errseq.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "4d2a47b203460bf1ca4699129d5b0ee9")
!2373 = !DIDerivedType(tag: DW_TAG_member, name: "i_private_lock", scope: !1031, file: !342, line: 481, baseType: !79, size: 32, offset: 992)
!2374 = !DIDerivedType(tag: DW_TAG_member, name: "i_private_list", scope: !1031, file: !342, line: 482, baseType: !117, size: 128, offset: 1024)
!2375 = !DIDerivedType(tag: DW_TAG_member, name: "i_mmap_rwsem", scope: !1031, file: !342, line: 483, baseType: !549, size: 320, offset: 1152)
!2376 = !DIDerivedType(tag: DW_TAG_member, name: "i_private_data", scope: !1031, file: !342, line: 484, baseType: !40, size: 64, offset: 1472)
!2377 = !DIDerivedType(tag: DW_TAG_member, name: "read", scope: !1021, file: !987, line: 298, baseType: !2378, size: 64, offset: 320)
!2378 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2379, size: 64)
!2379 = !DISubroutineType(types: !2380)
!2380 = !{!993, !896, !927, !1020, !625, !61, !55}
!2381 = !DIDerivedType(tag: DW_TAG_member, name: "write", scope: !1021, file: !987, line: 300, baseType: !2378, size: 64, offset: 384)
!2382 = !DIDerivedType(tag: DW_TAG_member, name: "llseek", scope: !1021, file: !987, line: 302, baseType: !2383, size: 64, offset: 448)
!2383 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2384, size: 64)
!2384 = !DISubroutineType(types: !2385)
!2385 = !{!61, !896, !927, !1020, !61, !42}
!2386 = !DIDerivedType(tag: DW_TAG_member, name: "mmap", scope: !1021, file: !987, line: 304, baseType: !2387, size: 64, offset: 512)
!2387 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2388, size: 64)
!2388 = !DISubroutineType(types: !2389)
!2389 = !{!42, !896, !927, !1020, !1167}
!2390 = !DIDerivedType(tag: DW_TAG_member, name: "attrs", scope: !1009, file: !987, line: 100, baseType: !2391, size: 64, offset: 192)
!2391 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !996, size: 64)
!2392 = !DIDerivedType(tag: DW_TAG_member, name: "bin_attrs", scope: !1009, file: !987, line: 101, baseType: !2393, size: 64, offset: 256)
!2393 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1020, size: 64)
!2394 = !DIDerivedType(tag: DW_TAG_member, name: "child_ns_type", scope: !977, file: !922, line: 120, baseType: !2395, size: 64, offset: 192)
!2395 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2396, size: 64)
!2396 = !DISubroutineType(types: !2397)
!2397 = !{!2293, !945}
!2398 = !DIDerivedType(tag: DW_TAG_member, name: "namespace", scope: !977, file: !922, line: 121, baseType: !2399, size: 64, offset: 256)
!2399 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2400, size: 64)
!2400 = !DISubroutineType(types: !2401)
!2401 = !{!1298, !945}
!2402 = !DIDerivedType(tag: DW_TAG_member, name: "get_ownership", scope: !977, file: !922, line: 122, baseType: !2403, size: 64, offset: 320)
!2403 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2404, size: 64)
!2404 = !DISubroutineType(types: !2405)
!2405 = !{null, !945, !187, !195}
!2406 = !DIDerivedType(tag: DW_TAG_member, name: "sd", scope: !921, file: !922, line: 70, baseType: !2407, size: 64, offset: 384)
!2407 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2408, size: 64)
!2408 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kernfs_node", file: !2409, line: 190, size: 1088, elements: !2410)
!2409 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/kernfs.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "dba4afde64a99ad0b3f541554ba8201f")
!2410 = !{!2411, !2412, !2413, !2414, !2415, !2416, !2417, !2418, !2419, !2420, !2538, !2539, !2540, !2543}
!2411 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !2408, file: !2409, line: 191, baseType: !69, size: 32)
!2412 = !DIDerivedType(tag: DW_TAG_member, name: "active", scope: !2408, file: !2409, line: 192, baseType: !69, size: 32, offset: 32)
!2413 = !DIDerivedType(tag: DW_TAG_member, name: "parent", scope: !2408, file: !2409, line: 202, baseType: !2407, size: 64, offset: 64)
!2414 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !2408, file: !2409, line: 203, baseType: !36, size: 64, offset: 128)
!2415 = !DIDerivedType(tag: DW_TAG_member, name: "rb", scope: !2408, file: !2409, line: 205, baseType: !173, size: 192, align: 64, offset: 192)
!2416 = !DIDerivedType(tag: DW_TAG_member, name: "ns", scope: !2408, file: !2409, line: 207, baseType: !1298, size: 64, offset: 384)
!2417 = !DIDerivedType(tag: DW_TAG_member, name: "hash", scope: !2408, file: !2409, line: 208, baseType: !7, size: 32, offset: 448)
!2418 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !2408, file: !2409, line: 209, baseType: !46, size: 16, offset: 480)
!2419 = !DIDerivedType(tag: DW_TAG_member, name: "mode", scope: !2408, file: !2409, line: 210, baseType: !44, size: 16, offset: 496)
!2420 = !DIDerivedType(tag: DW_TAG_member, scope: !2408, file: !2409, line: 212, baseType: !2421, size: 256, offset: 512)
!2421 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !2408, file: !2409, line: 212, size: 256, elements: !2422)
!2422 = !{!2423, !2432, !2436}
!2423 = !DIDerivedType(tag: DW_TAG_member, name: "dir", scope: !2421, file: !2409, line: 213, baseType: !2424, size: 256)
!2424 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kernfs_elem_dir", file: !2409, line: 153, size: 256, elements: !2425)
!2425 = !{!2426, !2427, !2428, !2431}
!2426 = !DIDerivedType(tag: DW_TAG_member, name: "subdirs", scope: !2424, file: !2409, line: 154, baseType: !59, size: 64)
!2427 = !DIDerivedType(tag: DW_TAG_member, name: "children", scope: !2424, file: !2409, line: 156, baseType: !168, size: 64, offset: 64)
!2428 = !DIDerivedType(tag: DW_TAG_member, name: "root", scope: !2424, file: !2409, line: 162, baseType: !2429, size: 64, offset: 128)
!2429 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2430, size: 64)
!2430 = !DICompositeType(tag: DW_TAG_structure_type, name: "kernfs_root", file: !2409, line: 162, flags: DIFlagFwdDecl)
!2431 = !DIDerivedType(tag: DW_TAG_member, name: "rev", scope: !2424, file: !2409, line: 167, baseType: !59, size: 64, offset: 192)
!2432 = !DIDerivedType(tag: DW_TAG_member, name: "symlink", scope: !2421, file: !2409, line: 214, baseType: !2433, size: 64)
!2433 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kernfs_elem_symlink", file: !2409, line: 170, size: 64, elements: !2434)
!2434 = !{!2435}
!2435 = !DIDerivedType(tag: DW_TAG_member, name: "target_kn", scope: !2433, file: !2409, line: 171, baseType: !2407, size: 64)
!2436 = !DIDerivedType(tag: DW_TAG_member, name: "attr", scope: !2421, file: !2409, line: 215, baseType: !2437, size: 256)
!2437 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kernfs_elem_attr", file: !2409, line: 174, size: 256, elements: !2438)
!2438 = !{!2439, !2533, !2536, !2537}
!2439 = !DIDerivedType(tag: DW_TAG_member, name: "ops", scope: !2437, file: !2409, line: 175, baseType: !2440, size: 64)
!2440 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2441, size: 64)
!2441 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !2442)
!2442 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kernfs_ops", file: !2409, line: 271, size: 832, elements: !2443)
!2443 = !{!2444, !2502, !2506, !2507, !2508, !2509, !2510, !2514, !2515, !2516, !2517, !2525, !2529}
!2444 = !DIDerivedType(tag: DW_TAG_member, name: "open", scope: !2442, file: !2409, line: 276, baseType: !2445, size: 64)
!2445 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2446, size: 64)
!2446 = !DISubroutineType(types: !2447)
!2447 = !{!42, !2448}
!2448 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2449, size: 64)
!2449 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kernfs_open_file", file: !2409, line: 251, size: 1216, elements: !2450)
!2450 = !{!2451, !2452, !2453, !2492, !2493, !2494, !2495, !2496, !2497, !2498, !2499, !2500, !2501}
!2451 = !DIDerivedType(tag: DW_TAG_member, name: "kn", scope: !2449, file: !2409, line: 253, baseType: !2407, size: 64)
!2452 = !DIDerivedType(tag: DW_TAG_member, name: "file", scope: !2449, file: !2409, line: 254, baseType: !896, size: 64, offset: 64)
!2453 = !DIDerivedType(tag: DW_TAG_member, name: "seq_file", scope: !2449, file: !2409, line: 255, baseType: !2454, size: 64, offset: 128)
!2454 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2455, size: 64)
!2455 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "seq_file", file: !2456, line: 16, size: 960, elements: !2457)
!2456 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/seq_file.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "cea45dd6561c49cf7dd0bbcc729e1108")
!2457 = !{!2458, !2459, !2460, !2461, !2462, !2463, !2464, !2465, !2466, !2487, !2488, !2491}
!2458 = !DIDerivedType(tag: DW_TAG_member, name: "buf", scope: !2455, file: !2456, line: 17, baseType: !625, size: 64)
!2459 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !2455, file: !2456, line: 18, baseType: !55, size: 64, offset: 64)
!2460 = !DIDerivedType(tag: DW_TAG_member, name: "from", scope: !2455, file: !2456, line: 19, baseType: !55, size: 64, offset: 128)
!2461 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !2455, file: !2456, line: 20, baseType: !55, size: 64, offset: 192)
!2462 = !DIDerivedType(tag: DW_TAG_member, name: "pad_until", scope: !2455, file: !2456, line: 21, baseType: !55, size: 64, offset: 256)
!2463 = !DIDerivedType(tag: DW_TAG_member, name: "index", scope: !2455, file: !2456, line: 22, baseType: !61, size: 64, offset: 320)
!2464 = !DIDerivedType(tag: DW_TAG_member, name: "read_pos", scope: !2455, file: !2456, line: 23, baseType: !61, size: 64, offset: 384)
!2465 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !2455, file: !2456, line: 24, baseType: !1277, size: 256, offset: 448)
!2466 = !DIDerivedType(tag: DW_TAG_member, name: "op", scope: !2455, file: !2456, line: 25, baseType: !2467, size: 64, offset: 704)
!2467 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2468, size: 64)
!2468 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !2469)
!2469 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "seq_operations", file: !2456, line: 31, size: 256, elements: !2470)
!2470 = !{!2471, !2475, !2479, !2483}
!2471 = !DIDerivedType(tag: DW_TAG_member, name: "start", scope: !2469, file: !2456, line: 32, baseType: !2472, size: 64)
!2472 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2473, size: 64)
!2473 = !DISubroutineType(types: !2474)
!2474 = !{!40, !2454, !60}
!2475 = !DIDerivedType(tag: DW_TAG_member, name: "stop", scope: !2469, file: !2456, line: 33, baseType: !2476, size: 64, offset: 64)
!2476 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2477, size: 64)
!2477 = !DISubroutineType(types: !2478)
!2478 = !{null, !2454, !40}
!2479 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !2469, file: !2456, line: 34, baseType: !2480, size: 64, offset: 128)
!2480 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2481, size: 64)
!2481 = !DISubroutineType(types: !2482)
!2482 = !{!40, !2454, !40, !60}
!2483 = !DIDerivedType(tag: DW_TAG_member, name: "show", scope: !2469, file: !2456, line: 35, baseType: !2484, size: 64, offset: 192)
!2484 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2485, size: 64)
!2485 = !DISubroutineType(types: !2486)
!2486 = !{!42, !2454, !40}
!2487 = !DIDerivedType(tag: DW_TAG_member, name: "poll_event", scope: !2455, file: !2456, line: 26, baseType: !42, size: 32, offset: 768)
!2488 = !DIDerivedType(tag: DW_TAG_member, name: "file", scope: !2455, file: !2456, line: 27, baseType: !2489, size: 64, offset: 832)
!2489 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2490, size: 64)
!2490 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !897)
!2491 = !DIDerivedType(tag: DW_TAG_member, name: "private", scope: !2455, file: !2456, line: 28, baseType: !40, size: 64, offset: 896)
!2492 = !DIDerivedType(tag: DW_TAG_member, name: "priv", scope: !2449, file: !2409, line: 256, baseType: !40, size: 64, offset: 192)
!2493 = !DIDerivedType(tag: DW_TAG_member, name: "mutex", scope: !2449, file: !2409, line: 259, baseType: !1277, size: 256, offset: 256)
!2494 = !DIDerivedType(tag: DW_TAG_member, name: "prealloc_mutex", scope: !2449, file: !2409, line: 260, baseType: !1277, size: 256, offset: 512)
!2495 = !DIDerivedType(tag: DW_TAG_member, name: "event", scope: !2449, file: !2409, line: 261, baseType: !42, size: 32, offset: 768)
!2496 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !2449, file: !2409, line: 262, baseType: !117, size: 128, offset: 832)
!2497 = !DIDerivedType(tag: DW_TAG_member, name: "prealloc_buf", scope: !2449, file: !2409, line: 263, baseType: !625, size: 64, offset: 960)
!2498 = !DIDerivedType(tag: DW_TAG_member, name: "atomic_write_len", scope: !2449, file: !2409, line: 265, baseType: !55, size: 64, offset: 1024)
!2499 = !DIDerivedType(tag: DW_TAG_member, name: "mmapped", scope: !2449, file: !2409, line: 266, baseType: !614, size: 1, offset: 1088, flags: DIFlagBitField, extraData: i64 1088)
!2500 = !DIDerivedType(tag: DW_TAG_member, name: "released", scope: !2449, file: !2409, line: 267, baseType: !614, size: 1, offset: 1089, flags: DIFlagBitField, extraData: i64 1088)
!2501 = !DIDerivedType(tag: DW_TAG_member, name: "vm_ops", scope: !2449, file: !2409, line: 268, baseType: !1385, size: 64, offset: 1152)
!2502 = !DIDerivedType(tag: DW_TAG_member, name: "release", scope: !2442, file: !2409, line: 277, baseType: !2503, size: 64, offset: 64)
!2503 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2504, size: 64)
!2504 = !DISubroutineType(types: !2505)
!2505 = !{null, !2448}
!2506 = !DIDerivedType(tag: DW_TAG_member, name: "seq_show", scope: !2442, file: !2409, line: 290, baseType: !2484, size: 64, offset: 128)
!2507 = !DIDerivedType(tag: DW_TAG_member, name: "seq_start", scope: !2442, file: !2409, line: 292, baseType: !2472, size: 64, offset: 192)
!2508 = !DIDerivedType(tag: DW_TAG_member, name: "seq_next", scope: !2442, file: !2409, line: 293, baseType: !2480, size: 64, offset: 256)
!2509 = !DIDerivedType(tag: DW_TAG_member, name: "seq_stop", scope: !2442, file: !2409, line: 294, baseType: !2476, size: 64, offset: 320)
!2510 = !DIDerivedType(tag: DW_TAG_member, name: "read", scope: !2442, file: !2409, line: 296, baseType: !2511, size: 64, offset: 384)
!2511 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2512, size: 64)
!2512 = !DISubroutineType(types: !2513)
!2513 = !{!993, !2448, !625, !55, !61}
!2514 = !DIDerivedType(tag: DW_TAG_member, name: "atomic_write_len", scope: !2442, file: !2409, line: 306, baseType: !55, size: 64, offset: 448)
!2515 = !DIDerivedType(tag: DW_TAG_member, name: "prealloc", scope: !2442, file: !2409, line: 313, baseType: !614, size: 8, offset: 512)
!2516 = !DIDerivedType(tag: DW_TAG_member, name: "write", scope: !2442, file: !2409, line: 314, baseType: !2511, size: 64, offset: 576)
!2517 = !DIDerivedType(tag: DW_TAG_member, name: "poll", scope: !2442, file: !2409, line: 317, baseType: !2518, size: 64, offset: 640)
!2518 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2519, size: 64)
!2519 = !DISubroutineType(types: !2520)
!2520 = !{!2521, !2448, !2523}
!2521 = !DIDerivedType(tag: DW_TAG_typedef, name: "__poll_t", file: !2522, line: 59, baseType: !7)
!2522 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/uapi/linux/types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "013cacf9b9a56e4327d8ec393f8d421f")
!2523 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2524, size: 64)
!2524 = !DICompositeType(tag: DW_TAG_structure_type, name: "poll_table_struct", file: !342, line: 63, flags: DIFlagFwdDecl)
!2525 = !DIDerivedType(tag: DW_TAG_member, name: "mmap", scope: !2442, file: !2409, line: 320, baseType: !2526, size: 64, offset: 704)
!2526 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2527, size: 64)
!2527 = !DISubroutineType(types: !2528)
!2528 = !{!42, !2448, !1167}
!2529 = !DIDerivedType(tag: DW_TAG_member, name: "llseek", scope: !2442, file: !2409, line: 321, baseType: !2530, size: 64, offset: 768)
!2530 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2531, size: 64)
!2531 = !DISubroutineType(types: !2532)
!2532 = !{!61, !2448, !61, !42}
!2533 = !DIDerivedType(tag: DW_TAG_member, name: "open", scope: !2437, file: !2409, line: 176, baseType: !2534, size: 64, offset: 64)
!2534 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2535, size: 64)
!2535 = !DICompositeType(tag: DW_TAG_structure_type, name: "kernfs_open_node", file: !2409, line: 35, flags: DIFlagFwdDecl)
!2536 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !2437, file: !2409, line: 177, baseType: !61, size: 64, offset: 128)
!2537 = !DIDerivedType(tag: DW_TAG_member, name: "notify_next", scope: !2437, file: !2409, line: 178, baseType: !2407, size: 64, offset: 192)
!2538 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !2408, file: !2409, line: 222, baseType: !519, size: 64, offset: 768)
!2539 = !DIDerivedType(tag: DW_TAG_member, name: "priv", scope: !2408, file: !2409, line: 224, baseType: !40, size: 64, offset: 832)
!2540 = !DIDerivedType(tag: DW_TAG_member, name: "iattr", scope: !2408, file: !2409, line: 225, baseType: !2541, size: 64, offset: 896)
!2541 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2542, size: 64)
!2542 = !DICompositeType(tag: DW_TAG_structure_type, name: "kernfs_iattrs", file: !2409, line: 36, flags: DIFlagFwdDecl)
!2543 = !DIDerivedType(tag: DW_TAG_member, name: "rcu", scope: !2408, file: !2409, line: 227, baseType: !129, size: 128, align: 64, offset: 960)
!2544 = !DIDerivedType(tag: DW_TAG_member, name: "kref", scope: !921, file: !922, line: 71, baseType: !2545, size: 32, offset: 448)
!2545 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kref", file: !2546, line: 19, size: 32, elements: !2547)
!2546 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/kref.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "1554b486522fa90d95fe0370c160b0ab")
!2547 = !{!2548}
!2548 = !DIDerivedType(tag: DW_TAG_member, name: "refcount", scope: !2545, file: !2546, line: 20, baseType: !533, size: 32)
!2549 = !DIDerivedType(tag: DW_TAG_member, name: "state_initialized", scope: !921, file: !922, line: 73, baseType: !7, size: 1, offset: 480, flags: DIFlagBitField, extraData: i64 480)
!2550 = !DIDerivedType(tag: DW_TAG_member, name: "state_in_sysfs", scope: !921, file: !922, line: 74, baseType: !7, size: 1, offset: 481, flags: DIFlagBitField, extraData: i64 480)
!2551 = !DIDerivedType(tag: DW_TAG_member, name: "state_add_uevent_sent", scope: !921, file: !922, line: 75, baseType: !7, size: 1, offset: 482, flags: DIFlagBitField, extraData: i64 480)
!2552 = !DIDerivedType(tag: DW_TAG_member, name: "state_remove_uevent_sent", scope: !921, file: !922, line: 76, baseType: !7, size: 1, offset: 483, flags: DIFlagBitField, extraData: i64 480)
!2553 = !DIDerivedType(tag: DW_TAG_member, name: "uevent_suppress", scope: !921, file: !922, line: 77, baseType: !7, size: 1, offset: 484, flags: DIFlagBitField, extraData: i64 480)
!2554 = !DIDerivedType(tag: DW_TAG_member, name: "mod", scope: !918, file: !6, line: 47, baseType: !908, size: 64, offset: 512)
!2555 = !DIDerivedType(tag: DW_TAG_member, name: "drivers_dir", scope: !918, file: !6, line: 48, baseType: !927, size: 64, offset: 576)
!2556 = !DIDerivedType(tag: DW_TAG_member, name: "mp", scope: !918, file: !6, line: 49, baseType: !2557, size: 64, offset: 640)
!2557 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2558, size: 64)
!2558 = !DICompositeType(tag: DW_TAG_structure_type, name: "module_param_attrs", file: !6, line: 49, flags: DIFlagFwdDecl)
!2559 = !DIDerivedType(tag: DW_TAG_member, name: "kobj_completion", scope: !918, file: !6, line: 50, baseType: !138, size: 64, offset: 704)
!2560 = !DIDerivedType(tag: DW_TAG_member, name: "modinfo_attrs", scope: !909, file: !6, line: 424, baseType: !2561, size: 64, offset: 1408)
!2561 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2562, size: 64)
!2562 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "module_attribute", file: !6, line: 53, size: 448, elements: !2563)
!2563 = !{!2564, !2565, !2570, !2574, !2578, !2582}
!2564 = !DIDerivedType(tag: DW_TAG_member, name: "attr", scope: !2562, file: !6, line: 54, baseType: !997, size: 128)
!2565 = !DIDerivedType(tag: DW_TAG_member, name: "show", scope: !2562, file: !6, line: 55, baseType: !2566, size: 64, offset: 128)
!2566 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2567, size: 64)
!2567 = !DISubroutineType(types: !2568)
!2568 = !{!993, !2561, !2569, !625}
!2569 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !918, size: 64)
!2570 = !DIDerivedType(tag: DW_TAG_member, name: "store", scope: !2562, file: !6, line: 57, baseType: !2571, size: 64, offset: 192)
!2571 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2572, size: 64)
!2572 = !DISubroutineType(types: !2573)
!2573 = !{!993, !2561, !2569, !36, !55}
!2574 = !DIDerivedType(tag: DW_TAG_member, name: "setup", scope: !2562, file: !6, line: 59, baseType: !2575, size: 64, offset: 256)
!2575 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2576, size: 64)
!2576 = !DISubroutineType(types: !2577)
!2577 = !{null, !908, !36}
!2578 = !DIDerivedType(tag: DW_TAG_member, name: "test", scope: !2562, file: !6, line: 60, baseType: !2579, size: 64, offset: 320)
!2579 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2580, size: 64)
!2580 = !DISubroutineType(types: !2581)
!2581 = !{!42, !908}
!2582 = !DIDerivedType(tag: DW_TAG_member, name: "free", scope: !2562, file: !6, line: 61, baseType: !2583, size: 64, offset: 384)
!2583 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2584, size: 64)
!2584 = !DISubroutineType(types: !2585)
!2585 = !{null, !908}
!2586 = !DIDerivedType(tag: DW_TAG_member, name: "version", scope: !909, file: !6, line: 425, baseType: !36, size: 64, offset: 1472)
!2587 = !DIDerivedType(tag: DW_TAG_member, name: "srcversion", scope: !909, file: !6, line: 426, baseType: !36, size: 64, offset: 1536)
!2588 = !DIDerivedType(tag: DW_TAG_member, name: "holders_dir", scope: !909, file: !6, line: 427, baseType: !927, size: 64, offset: 1600)
!2589 = !DIDerivedType(tag: DW_TAG_member, name: "syms", scope: !909, file: !6, line: 430, baseType: !2590, size: 64, offset: 1664)
!2590 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2591, size: 64)
!2591 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !2592)
!2592 = !DICompositeType(tag: DW_TAG_structure_type, name: "kernel_symbol", file: !6, line: 430, flags: DIFlagFwdDecl)
!2593 = !DIDerivedType(tag: DW_TAG_member, name: "crcs", scope: !909, file: !6, line: 431, baseType: !2594, size: 64, offset: 1728)
!2594 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2595, size: 64)
!2595 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !541)
!2596 = !DIDerivedType(tag: DW_TAG_member, name: "num_syms", scope: !909, file: !6, line: 432, baseType: !7, size: 32, offset: 1792)
!2597 = !DIDerivedType(tag: DW_TAG_member, name: "param_lock", scope: !909, file: !6, line: 441, baseType: !1277, size: 256, offset: 1856)
!2598 = !DIDerivedType(tag: DW_TAG_member, name: "kp", scope: !909, file: !6, line: 443, baseType: !2599, size: 64, offset: 2112)
!2599 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2600, size: 64)
!2600 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kernel_param", file: !2601, line: 69, size: 320, elements: !2602)
!2601 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/moduleparam.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "cdccce0302d5f7b4b20bc63b5372f477")
!2602 = !{!2603, !2604, !2605, !2622, !2624, !2627, !2628}
!2603 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !2600, file: !2601, line: 70, baseType: !36, size: 64)
!2604 = !DIDerivedType(tag: DW_TAG_member, name: "mod", scope: !2600, file: !2601, line: 71, baseType: !908, size: 64, offset: 64)
!2605 = !DIDerivedType(tag: DW_TAG_member, name: "ops", scope: !2600, file: !2601, line: 72, baseType: !2606, size: 64, offset: 128)
!2606 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2607, size: 64)
!2607 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !2608)
!2608 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kernel_param_ops", file: !2601, line: 47, size: 256, elements: !2609)
!2609 = !{!2610, !2611, !2617, !2621}
!2610 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !2608, file: !2601, line: 49, baseType: !7, size: 32)
!2611 = !DIDerivedType(tag: DW_TAG_member, name: "set", scope: !2608, file: !2601, line: 51, baseType: !2612, size: 64, offset: 64)
!2612 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2613, size: 64)
!2613 = !DISubroutineType(types: !2614)
!2614 = !{!42, !36, !2615}
!2615 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2616, size: 64)
!2616 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !2600)
!2617 = !DIDerivedType(tag: DW_TAG_member, name: "get", scope: !2608, file: !2601, line: 53, baseType: !2618, size: 64, offset: 128)
!2618 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2619, size: 64)
!2619 = !DISubroutineType(types: !2620)
!2620 = !{!42, !625, !2615}
!2621 = !DIDerivedType(tag: DW_TAG_member, name: "free", scope: !2608, file: !2601, line: 55, baseType: !809, size: 64, offset: 192)
!2622 = !DIDerivedType(tag: DW_TAG_member, name: "perm", scope: !2600, file: !2601, line: 73, baseType: !2623, size: 16, offset: 192)
!2623 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !113)
!2624 = !DIDerivedType(tag: DW_TAG_member, name: "level", scope: !2600, file: !2601, line: 74, baseType: !2625, size: 8, offset: 208)
!2625 = !DIDerivedType(tag: DW_TAG_typedef, name: "s8", file: !104, line: 16, baseType: !2626)
!2626 = !DIDerivedType(tag: DW_TAG_typedef, name: "__s8", file: !106, line: 20, baseType: !1821)
!2627 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !2600, file: !2601, line: 75, baseType: !103, size: 8, offset: 216)
!2628 = !DIDerivedType(tag: DW_TAG_member, scope: !2600, file: !2601, line: 76, baseType: !2629, size: 64, offset: 256)
!2629 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !2600, file: !2601, line: 76, size: 64, elements: !2630)
!2630 = !{!2631, !2632, !2639}
!2631 = !DIDerivedType(tag: DW_TAG_member, name: "arg", scope: !2629, file: !2601, line: 77, baseType: !40, size: 64)
!2632 = !DIDerivedType(tag: DW_TAG_member, name: "str", scope: !2629, file: !2601, line: 78, baseType: !2633, size: 64)
!2633 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2634, size: 64)
!2634 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !2635)
!2635 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kparam_string", file: !2601, line: 86, size: 128, elements: !2636)
!2636 = !{!2637, !2638}
!2637 = !DIDerivedType(tag: DW_TAG_member, name: "maxlen", scope: !2635, file: !2601, line: 87, baseType: !7, size: 32)
!2638 = !DIDerivedType(tag: DW_TAG_member, name: "string", scope: !2635, file: !2601, line: 88, baseType: !625, size: 64, offset: 64)
!2639 = !DIDerivedType(tag: DW_TAG_member, name: "arr", scope: !2629, file: !2601, line: 79, baseType: !2640, size: 64)
!2640 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2641, size: 64)
!2641 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !2642)
!2642 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kparam_array", file: !2601, line: 92, size: 256, elements: !2643)
!2643 = !{!2644, !2645, !2646, !2647, !2648}
!2644 = !DIDerivedType(tag: DW_TAG_member, name: "max", scope: !2642, file: !2601, line: 94, baseType: !7, size: 32)
!2645 = !DIDerivedType(tag: DW_TAG_member, name: "elemsize", scope: !2642, file: !2601, line: 95, baseType: !7, size: 32, offset: 32)
!2646 = !DIDerivedType(tag: DW_TAG_member, name: "num", scope: !2642, file: !2601, line: 96, baseType: !1851, size: 64, offset: 64)
!2647 = !DIDerivedType(tag: DW_TAG_member, name: "ops", scope: !2642, file: !2601, line: 97, baseType: !2606, size: 64, offset: 128)
!2648 = !DIDerivedType(tag: DW_TAG_member, name: "elem", scope: !2642, file: !2601, line: 98, baseType: !40, size: 64, offset: 192)
!2649 = !DIDerivedType(tag: DW_TAG_member, name: "num_kp", scope: !909, file: !6, line: 444, baseType: !7, size: 32, offset: 2176)
!2650 = !DIDerivedType(tag: DW_TAG_member, name: "num_gpl_syms", scope: !909, file: !6, line: 447, baseType: !7, size: 32, offset: 2208)
!2651 = !DIDerivedType(tag: DW_TAG_member, name: "gpl_syms", scope: !909, file: !6, line: 448, baseType: !2590, size: 64, offset: 2240)
!2652 = !DIDerivedType(tag: DW_TAG_member, name: "gpl_crcs", scope: !909, file: !6, line: 449, baseType: !2594, size: 64, offset: 2304)
!2653 = !DIDerivedType(tag: DW_TAG_member, name: "using_gplonly_symbols", scope: !909, file: !6, line: 450, baseType: !614, size: 8, offset: 2368)
!2654 = !DIDerivedType(tag: DW_TAG_member, name: "async_probe_requested", scope: !909, file: !6, line: 457, baseType: !614, size: 8, offset: 2376)
!2655 = !DIDerivedType(tag: DW_TAG_member, name: "num_exentries", scope: !909, file: !6, line: 460, baseType: !7, size: 32, offset: 2400)
!2656 = !DIDerivedType(tag: DW_TAG_member, name: "extable", scope: !909, file: !6, line: 461, baseType: !2657, size: 64, offset: 2432)
!2657 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2658, size: 64)
!2658 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "exception_table_entry", file: !2659, line: 23, size: 96, elements: !2660)
!2659 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/extable.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "7ac4e76e381a47ebaf36fdbb419a115d")
!2660 = !{!2661, !2662, !2663}
!2661 = !DIDerivedType(tag: DW_TAG_member, name: "insn", scope: !2658, file: !2659, line: 24, baseType: !42, size: 32)
!2662 = !DIDerivedType(tag: DW_TAG_member, name: "fixup", scope: !2658, file: !2659, line: 24, baseType: !42, size: 32, offset: 32)
!2663 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !2658, file: !2659, line: 24, baseType: !42, size: 32, offset: 64)
!2664 = !DIDerivedType(tag: DW_TAG_member, name: "init", scope: !909, file: !6, line: 464, baseType: !2665, size: 64, offset: 2496)
!2665 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2666, size: 64)
!2666 = !DISubroutineType(types: !2667)
!2667 = !{!42}
!2668 = !DIDerivedType(tag: DW_TAG_member, name: "mem", scope: !909, file: !6, line: 466, baseType: !2669, size: 4032, align: 512, offset: 2560)
!2669 = !DICompositeType(tag: DW_TAG_array_type, baseType: !2670, size: 4032, elements: !2686)
!2670 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "module_memory", file: !6, line: 368, size: 576, elements: !2671)
!2671 = !{!2672, !2673, !2674}
!2672 = !DIDerivedType(tag: DW_TAG_member, name: "base", scope: !2670, file: !6, line: 369, baseType: !40, size: 64)
!2673 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !2670, file: !6, line: 370, baseType: !7, size: 32, offset: 64)
!2674 = !DIDerivedType(tag: DW_TAG_member, name: "mtn", scope: !2670, file: !6, line: 373, baseType: !2675, size: 448, offset: 128)
!2675 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mod_tree_node", file: !6, line: 325, size: 448, elements: !2676)
!2676 = !{!2677, !2678}
!2677 = !DIDerivedType(tag: DW_TAG_member, name: "mod", scope: !2675, file: !6, line: 326, baseType: !908, size: 64)
!2678 = !DIDerivedType(tag: DW_TAG_member, name: "node", scope: !2675, file: !6, line: 327, baseType: !2679, size: 384, offset: 64)
!2679 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "latch_tree_node", file: !2680, line: 40, size: 384, elements: !2681)
!2680 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/rbtree_latch.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "298ed6c1c8090c2cce57a29615901c69")
!2681 = !{!2682}
!2682 = !DIDerivedType(tag: DW_TAG_member, name: "node", scope: !2679, file: !2680, line: 41, baseType: !2683, size: 384, align: 64)
!2683 = !DICompositeType(tag: DW_TAG_array_type, baseType: !173, size: 384, align: 64, elements: !2684)
!2684 = !{!2685}
!2685 = !DISubrange(count: 2)
!2686 = !{!2687}
!2687 = !DISubrange(count: 7)
!2688 = !DIDerivedType(tag: DW_TAG_member, name: "arch", scope: !909, file: !6, line: 469, baseType: !2689, size: 192, offset: 6592)
!2689 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mod_arch_specific", file: !2690, line: 8, size: 192, elements: !2691)
!2690 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/module.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "bc56644bb7e32f0bb7cb156e8d71b2cf")
!2691 = !{!2692, !2693, !2695}
!2692 = !DIDerivedType(tag: DW_TAG_member, name: "num_orcs", scope: !2689, file: !2690, line: 10, baseType: !7, size: 32)
!2693 = !DIDerivedType(tag: DW_TAG_member, name: "orc_unwind_ip", scope: !2689, file: !2690, line: 11, baseType: !2694, size: 64, offset: 64)
!2694 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !42, size: 64)
!2695 = !DIDerivedType(tag: DW_TAG_member, name: "orc_unwind", scope: !2689, file: !2690, line: 12, baseType: !2696, size: 64, offset: 128)
!2696 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2697, size: 64)
!2697 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "orc_entry", file: !2698, line: 59, size: 48, elements: !2699)
!2698 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/orc_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "217b54c75ff55a9bf8ecacfb45d2cb41")
!2699 = !{!2700, !2701, !2702, !2703, !2704, !2705}
!2700 = !DIDerivedType(tag: DW_TAG_member, name: "sp_offset", scope: !2697, file: !2698, line: 60, baseType: !1315, size: 16)
!2701 = !DIDerivedType(tag: DW_TAG_member, name: "bp_offset", scope: !2697, file: !2698, line: 61, baseType: !1315, size: 16, offset: 16)
!2702 = !DIDerivedType(tag: DW_TAG_member, name: "sp_reg", scope: !2697, file: !2698, line: 63, baseType: !7, size: 4, offset: 32, flags: DIFlagBitField, extraData: i64 32)
!2703 = !DIDerivedType(tag: DW_TAG_member, name: "bp_reg", scope: !2697, file: !2698, line: 64, baseType: !7, size: 4, offset: 36, flags: DIFlagBitField, extraData: i64 32)
!2704 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !2697, file: !2698, line: 65, baseType: !7, size: 3, offset: 40, flags: DIFlagBitField, extraData: i64 32)
!2705 = !DIDerivedType(tag: DW_TAG_member, name: "signal", scope: !2697, file: !2698, line: 66, baseType: !7, size: 1, offset: 43, flags: DIFlagBitField, extraData: i64 32)
!2706 = !DIDerivedType(tag: DW_TAG_member, name: "taints", scope: !909, file: !6, line: 471, baseType: !59, size: 64, offset: 6784)
!2707 = !DIDerivedType(tag: DW_TAG_member, name: "num_bugs", scope: !909, file: !6, line: 475, baseType: !7, size: 32, offset: 6848)
!2708 = !DIDerivedType(tag: DW_TAG_member, name: "bug_list", scope: !909, file: !6, line: 476, baseType: !117, size: 128, offset: 6912)
!2709 = !DIDerivedType(tag: DW_TAG_member, name: "bug_table", scope: !909, file: !6, line: 477, baseType: !2710, size: 64, offset: 7040)
!2710 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2711, size: 64)
!2711 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bug_entry", file: !2712, line: 33, size: 96, elements: !2713)
!2712 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/asm-generic/bug.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "feb7cf685e35303b6f5c15d167843099")
!2713 = !{!2714, !2715, !2716, !2717}
!2714 = !DIDerivedType(tag: DW_TAG_member, name: "bug_addr_disp", scope: !2711, file: !2712, line: 37, baseType: !42, size: 32)
!2715 = !DIDerivedType(tag: DW_TAG_member, name: "file_disp", scope: !2711, file: !2712, line: 43, baseType: !42, size: 32, offset: 32)
!2716 = !DIDerivedType(tag: DW_TAG_member, name: "line", scope: !2711, file: !2712, line: 45, baseType: !46, size: 16, offset: 64)
!2717 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !2711, file: !2712, line: 47, baseType: !46, size: 16, offset: 80)
!2718 = !DIDerivedType(tag: DW_TAG_member, name: "kallsyms", scope: !909, file: !6, line: 482, baseType: !2719, size: 64, offset: 7104)
!2719 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2720, size: 64)
!2720 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mod_kallsyms", file: !6, line: 384, size: 256, elements: !2721)
!2721 = !{!2722, !2738, !2739, !2740}
!2722 = !DIDerivedType(tag: DW_TAG_member, name: "symtab", scope: !2720, file: !6, line: 385, baseType: !2723, size: 64)
!2723 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2724, size: 64)
!2724 = !DIDerivedType(tag: DW_TAG_typedef, name: "Elf64_Sym", file: !2725, line: 204, baseType: !2726)
!2725 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/uapi/linux/elf.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "ca95cb5dd1af02c467ca9bf28a59c0aa")
!2726 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "elf64_sym", file: !2725, line: 197, size: 192, elements: !2727)
!2727 = !{!2728, !2730, !2731, !2732, !2734, !2736}
!2728 = !DIDerivedType(tag: DW_TAG_member, name: "st_name", scope: !2726, file: !2725, line: 198, baseType: !2729, size: 32)
!2729 = !DIDerivedType(tag: DW_TAG_typedef, name: "Elf64_Word", file: !2725, line: 21, baseType: !579)
!2730 = !DIDerivedType(tag: DW_TAG_member, name: "st_info", scope: !2726, file: !2725, line: 199, baseType: !107, size: 8, offset: 32)
!2731 = !DIDerivedType(tag: DW_TAG_member, name: "st_other", scope: !2726, file: !2725, line: 200, baseType: !107, size: 8, offset: 40)
!2732 = !DIDerivedType(tag: DW_TAG_member, name: "st_shndx", scope: !2726, file: !2725, line: 201, baseType: !2733, size: 16, offset: 48)
!2733 = !DIDerivedType(tag: DW_TAG_typedef, name: "Elf64_Half", file: !2725, line: 17, baseType: !114)
!2734 = !DIDerivedType(tag: DW_TAG_member, name: "st_value", scope: !2726, file: !2725, line: 202, baseType: !2735, size: 64, offset: 64)
!2735 = !DIDerivedType(tag: DW_TAG_typedef, name: "Elf64_Addr", file: !2725, line: 16, baseType: !520)
!2736 = !DIDerivedType(tag: DW_TAG_member, name: "st_size", scope: !2726, file: !2725, line: 203, baseType: !2737, size: 64, offset: 128)
!2737 = !DIDerivedType(tag: DW_TAG_typedef, name: "Elf64_Xword", file: !2725, line: 22, baseType: !520)
!2738 = !DIDerivedType(tag: DW_TAG_member, name: "num_symtab", scope: !2720, file: !6, line: 386, baseType: !7, size: 32, offset: 64)
!2739 = !DIDerivedType(tag: DW_TAG_member, name: "strtab", scope: !2720, file: !6, line: 387, baseType: !625, size: 64, offset: 128)
!2740 = !DIDerivedType(tag: DW_TAG_member, name: "typetab", scope: !2720, file: !6, line: 388, baseType: !625, size: 64, offset: 192)
!2741 = !DIDerivedType(tag: DW_TAG_member, name: "core_kallsyms", scope: !909, file: !6, line: 483, baseType: !2720, size: 256, offset: 7168)
!2742 = !DIDerivedType(tag: DW_TAG_member, name: "sect_attrs", scope: !909, file: !6, line: 486, baseType: !2743, size: 64, offset: 7424)
!2743 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2744, size: 64)
!2744 = !DICompositeType(tag: DW_TAG_structure_type, name: "module_sect_attrs", file: !6, line: 486, flags: DIFlagFwdDecl)
!2745 = !DIDerivedType(tag: DW_TAG_member, name: "notes_attrs", scope: !909, file: !6, line: 489, baseType: !2746, size: 64, offset: 7488)
!2746 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2747, size: 64)
!2747 = !DICompositeType(tag: DW_TAG_structure_type, name: "module_notes_attrs", file: !6, line: 489, flags: DIFlagFwdDecl)
!2748 = !DIDerivedType(tag: DW_TAG_member, name: "args", scope: !909, file: !6, line: 494, baseType: !625, size: 64, offset: 7552)
!2749 = !DIDerivedType(tag: DW_TAG_member, name: "percpu", scope: !909, file: !6, line: 498, baseType: !40, size: 64, offset: 7616)
!2750 = !DIDerivedType(tag: DW_TAG_member, name: "percpu_size", scope: !909, file: !6, line: 499, baseType: !7, size: 32, offset: 7680)
!2751 = !DIDerivedType(tag: DW_TAG_member, name: "noinstr_text_start", scope: !909, file: !6, line: 501, baseType: !40, size: 64, offset: 7744)
!2752 = !DIDerivedType(tag: DW_TAG_member, name: "noinstr_text_size", scope: !909, file: !6, line: 502, baseType: !7, size: 32, offset: 7808)
!2753 = !DIDerivedType(tag: DW_TAG_member, name: "num_tracepoints", scope: !909, file: !6, line: 505, baseType: !7, size: 32, offset: 7840)
!2754 = !DIDerivedType(tag: DW_TAG_member, name: "tracepoints_ptrs", scope: !909, file: !6, line: 506, baseType: !2755, size: 64, offset: 7872)
!2755 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2756, size: 64)
!2756 = !DIDerivedType(tag: DW_TAG_typedef, name: "tracepoint_ptr_t", file: !2757, line: 45, baseType: !2758)
!2757 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/tracepoint-defs.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "9adc8bcec4aca7b10db8cb63b9f9f561")
!2758 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !42)
!2759 = !DIDerivedType(tag: DW_TAG_member, name: "num_srcu_structs", scope: !909, file: !6, line: 509, baseType: !7, size: 32, offset: 7936)
!2760 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_struct_ptrs", scope: !909, file: !6, line: 510, baseType: !2761, size: 64, offset: 8000)
!2761 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2762, size: 64)
!2762 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2763, size: 64)
!2763 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "srcu_struct", file: !2764, line: 96, size: 192, elements: !2765)
!2764 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/srcutree.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "234726095f63105bfc1f497c4058accf")
!2765 = !{!2766, !2767, !2810, !2813}
!2766 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_idx", scope: !2763, file: !2764, line: 97, baseType: !7, size: 32)
!2767 = !DIDerivedType(tag: DW_TAG_member, name: "sda", scope: !2763, file: !2764, line: 98, baseType: !2768, size: 64, offset: 64)
!2768 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2769, size: 64)
!2769 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "srcu_data", file: !2764, line: 24, size: 3072, elements: !2770)
!2770 = !{!2771, !2773, !2774, !2775, !2776, !2790, !2791, !2792, !2793, !2794, !2795, !2796, !2807, !2808, !2809}
!2771 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_lock_count", scope: !2769, file: !2764, line: 26, baseType: !2772, size: 128)
!2772 = !DICompositeType(tag: DW_TAG_array_type, baseType: !496, size: 128, elements: !2684)
!2773 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_unlock_count", scope: !2769, file: !2764, line: 27, baseType: !2772, size: 128, offset: 128)
!2774 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_nmi_safety", scope: !2769, file: !2764, line: 28, baseType: !42, size: 32, offset: 256)
!2775 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !2769, file: !2764, line: 31, baseType: !79, size: 32, align: 512, offset: 512)
!2776 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_cblist", scope: !2769, file: !2764, line: 32, baseType: !2777, size: 960, offset: 576)
!2777 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "rcu_segcblist", file: !2778, line: 190, size: 960, elements: !2779)
!2778 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/rcu_segcblist.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "79ef9db5d12cd6f3e6c0246c223590a3")
!2779 = !{!2780, !2781, !2784, !2786, !2787, !2789}
!2780 = !DIDerivedType(tag: DW_TAG_member, name: "head", scope: !2777, file: !2778, line: 191, baseType: !132, size: 64)
!2781 = !DIDerivedType(tag: DW_TAG_member, name: "tails", scope: !2777, file: !2778, line: 192, baseType: !2782, size: 256, offset: 64)
!2782 = !DICompositeType(tag: DW_TAG_array_type, baseType: !2783, size: 256, elements: !635)
!2783 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !132, size: 64)
!2784 = !DIDerivedType(tag: DW_TAG_member, name: "gp_seq", scope: !2777, file: !2778, line: 193, baseType: !2785, size: 256, offset: 320)
!2785 = !DICompositeType(tag: DW_TAG_array_type, baseType: !59, size: 256, elements: !635)
!2786 = !DIDerivedType(tag: DW_TAG_member, name: "len", scope: !2777, file: !2778, line: 197, baseType: !892, size: 64, offset: 576)
!2787 = !DIDerivedType(tag: DW_TAG_member, name: "seglen", scope: !2777, file: !2778, line: 199, baseType: !2788, size: 256, offset: 640)
!2788 = !DICompositeType(tag: DW_TAG_array_type, baseType: !892, size: 256, elements: !635)
!2789 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !2777, file: !2778, line: 200, baseType: !103, size: 8, offset: 896)
!2790 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_seq_needed", scope: !2769, file: !2764, line: 33, baseType: !59, size: 64, offset: 1536)
!2791 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_seq_needed_exp", scope: !2769, file: !2764, line: 34, baseType: !59, size: 64, offset: 1600)
!2792 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_cblist_invoking", scope: !2769, file: !2764, line: 35, baseType: !614, size: 8, offset: 1664)
!2793 = !DIDerivedType(tag: DW_TAG_member, name: "delay_work", scope: !2769, file: !2764, line: 36, baseType: !2070, size: 320, offset: 1728)
!2794 = !DIDerivedType(tag: DW_TAG_member, name: "work", scope: !2769, file: !2764, line: 37, baseType: !1337, size: 256, offset: 2048)
!2795 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_barrier_head", scope: !2769, file: !2764, line: 38, baseType: !129, size: 128, align: 64, offset: 2304)
!2796 = !DIDerivedType(tag: DW_TAG_member, name: "mynode", scope: !2769, file: !2764, line: 39, baseType: !2797, size: 64, offset: 2432)
!2797 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2798, size: 64)
!2798 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "srcu_node", file: !2764, line: 49, size: 768, elements: !2799)
!2799 = !{!2800, !2801, !2802, !2803, !2804, !2805, !2806}
!2800 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !2798, file: !2764, line: 50, baseType: !79, size: 32)
!2801 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_have_cbs", scope: !2798, file: !2764, line: 51, baseType: !2785, size: 256, offset: 64)
!2802 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_data_have_cbs", scope: !2798, file: !2764, line: 53, baseType: !2785, size: 256, offset: 320)
!2803 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_seq_needed_exp", scope: !2798, file: !2764, line: 54, baseType: !59, size: 64, offset: 576)
!2804 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_parent", scope: !2798, file: !2764, line: 55, baseType: !2797, size: 64, offset: 640)
!2805 = !DIDerivedType(tag: DW_TAG_member, name: "grplo", scope: !2798, file: !2764, line: 56, baseType: !42, size: 32, offset: 704)
!2806 = !DIDerivedType(tag: DW_TAG_member, name: "grphi", scope: !2798, file: !2764, line: 57, baseType: !42, size: 32, offset: 736)
!2807 = !DIDerivedType(tag: DW_TAG_member, name: "grpmask", scope: !2769, file: !2764, line: 40, baseType: !59, size: 64, offset: 2496)
!2808 = !DIDerivedType(tag: DW_TAG_member, name: "cpu", scope: !2769, file: !2764, line: 42, baseType: !42, size: 32, offset: 2560)
!2809 = !DIDerivedType(tag: DW_TAG_member, name: "ssp", scope: !2769, file: !2764, line: 43, baseType: !2762, size: 64, offset: 2624)
!2810 = !DIDerivedType(tag: DW_TAG_member, name: "dep_map", scope: !2763, file: !2764, line: 99, baseType: !2811, offset: 128)
!2811 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "lockdep_map", file: !2812, line: 269, elements: !1201)
!2812 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/lockdep_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "fa288ec48101ba351745640d195b6597")
!2813 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_sup", scope: !2763, file: !2764, line: 100, baseType: !2814, size: 64, offset: 128)
!2814 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2815, size: 64)
!2815 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "srcu_usage", file: !2764, line: 63, size: 3008, elements: !2816)
!2816 = !{!2817, !2818, !2820, !2821, !2822, !2823, !2824, !2825, !2826, !2827, !2828, !2829, !2830, !2831, !2832, !2833, !2834, !2835, !2836, !2837, !2838, !2839, !2848}
!2817 = !DIDerivedType(tag: DW_TAG_member, name: "node", scope: !2815, file: !2764, line: 64, baseType: !2797, size: 64)
!2818 = !DIDerivedType(tag: DW_TAG_member, name: "level", scope: !2815, file: !2764, line: 65, baseType: !2819, size: 192, offset: 64)
!2819 = !DICompositeType(tag: DW_TAG_array_type, baseType: !2797, size: 192, elements: !962)
!2820 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_size_state", scope: !2815, file: !2764, line: 67, baseType: !42, size: 32, offset: 256)
!2821 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_cb_mutex", scope: !2815, file: !2764, line: 68, baseType: !1277, size: 256, offset: 320)
!2822 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !2815, file: !2764, line: 69, baseType: !79, size: 32, offset: 576)
!2823 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_mutex", scope: !2815, file: !2764, line: 70, baseType: !1277, size: 256, offset: 640)
!2824 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_seq", scope: !2815, file: !2764, line: 71, baseType: !59, size: 64, offset: 896)
!2825 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_seq_needed", scope: !2815, file: !2764, line: 72, baseType: !59, size: 64, offset: 960)
!2826 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_seq_needed_exp", scope: !2815, file: !2764, line: 73, baseType: !59, size: 64, offset: 1024)
!2827 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_start", scope: !2815, file: !2764, line: 74, baseType: !59, size: 64, offset: 1088)
!2828 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_last_gp_end", scope: !2815, file: !2764, line: 75, baseType: !59, size: 64, offset: 1152)
!2829 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_size_jiffies", scope: !2815, file: !2764, line: 76, baseType: !59, size: 64, offset: 1216)
!2830 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_n_lock_retries", scope: !2815, file: !2764, line: 77, baseType: !59, size: 64, offset: 1280)
!2831 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_n_exp_nodelay", scope: !2815, file: !2764, line: 78, baseType: !59, size: 64, offset: 1344)
!2832 = !DIDerivedType(tag: DW_TAG_member, name: "sda_is_static", scope: !2815, file: !2764, line: 79, baseType: !614, size: 8, offset: 1408)
!2833 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_barrier_seq", scope: !2815, file: !2764, line: 80, baseType: !59, size: 64, offset: 1472)
!2834 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_barrier_mutex", scope: !2815, file: !2764, line: 81, baseType: !1277, size: 256, offset: 1536)
!2835 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_barrier_completion", scope: !2815, file: !2764, line: 82, baseType: !139, size: 256, offset: 1792)
!2836 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_barrier_cpu_cnt", scope: !2815, file: !2764, line: 84, baseType: !69, size: 32, offset: 2048)
!2837 = !DIDerivedType(tag: DW_TAG_member, name: "reschedule_jiffies", scope: !2815, file: !2764, line: 87, baseType: !59, size: 64, offset: 2112)
!2838 = !DIDerivedType(tag: DW_TAG_member, name: "reschedule_count", scope: !2815, file: !2764, line: 88, baseType: !59, size: 64, offset: 2176)
!2839 = !DIDerivedType(tag: DW_TAG_member, name: "work", scope: !2815, file: !2764, line: 89, baseType: !2840, size: 704, offset: 2240)
!2840 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "delayed_work", file: !466, line: 113, size: 704, elements: !2841)
!2841 = !{!2842, !2843, !2844, !2847}
!2842 = !DIDerivedType(tag: DW_TAG_member, name: "work", scope: !2840, file: !466, line: 114, baseType: !1337, size: 256)
!2843 = !DIDerivedType(tag: DW_TAG_member, name: "timer", scope: !2840, file: !466, line: 115, baseType: !2070, size: 320, offset: 256)
!2844 = !DIDerivedType(tag: DW_TAG_member, name: "wq", scope: !2840, file: !466, line: 118, baseType: !2845, size: 64, offset: 576)
!2845 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2846, size: 64)
!2846 = !DICompositeType(tag: DW_TAG_structure_type, name: "workqueue_struct", file: !1338, line: 10, flags: DIFlagFwdDecl)
!2847 = !DIDerivedType(tag: DW_TAG_member, name: "cpu", scope: !2840, file: !466, line: 119, baseType: !42, size: 32, offset: 640)
!2848 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_ssp", scope: !2815, file: !2764, line: 90, baseType: !2762, size: 64, offset: 2944)
!2849 = !DIDerivedType(tag: DW_TAG_member, name: "jump_entries", scope: !909, file: !6, line: 523, baseType: !2850, size: 64, offset: 8064)
!2850 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2851, size: 64)
!2851 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "jump_entry", file: !2852, line: 117, size: 128, elements: !2853)
!2852 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/jump_label.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "cbd26689c4d41035d788a70ac7ef10a9")
!2853 = !{!2854, !2855, !2856}
!2854 = !DIDerivedType(tag: DW_TAG_member, name: "code", scope: !2851, file: !2852, line: 118, baseType: !541, size: 32)
!2855 = !DIDerivedType(tag: DW_TAG_member, name: "target", scope: !2851, file: !2852, line: 119, baseType: !541, size: 32, offset: 32)
!2856 = !DIDerivedType(tag: DW_TAG_member, name: "key", scope: !2851, file: !2852, line: 120, baseType: !892, size: 64, offset: 64)
!2857 = !DIDerivedType(tag: DW_TAG_member, name: "num_jump_entries", scope: !909, file: !6, line: 524, baseType: !7, size: 32, offset: 8128)
!2858 = !DIDerivedType(tag: DW_TAG_member, name: "num_trace_bprintk_fmt", scope: !909, file: !6, line: 527, baseType: !7, size: 32, offset: 8160)
!2859 = !DIDerivedType(tag: DW_TAG_member, name: "trace_bprintk_fmt_start", scope: !909, file: !6, line: 528, baseType: !2860, size: 64, offset: 8192)
!2860 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !36, size: 64)
!2861 = !DIDerivedType(tag: DW_TAG_member, name: "trace_events", scope: !909, file: !6, line: 531, baseType: !2862, size: 64, offset: 8256)
!2862 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2863, size: 64)
!2863 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2864, size: 64)
!2864 = !DICompositeType(tag: DW_TAG_structure_type, name: "trace_event_call", file: !6, line: 531, flags: DIFlagFwdDecl)
!2865 = !DIDerivedType(tag: DW_TAG_member, name: "num_trace_events", scope: !909, file: !6, line: 532, baseType: !7, size: 32, offset: 8320)
!2866 = !DIDerivedType(tag: DW_TAG_member, name: "trace_evals", scope: !909, file: !6, line: 533, baseType: !2867, size: 64, offset: 8384)
!2867 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2868, size: 64)
!2868 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2869, size: 64)
!2869 = !DICompositeType(tag: DW_TAG_structure_type, name: "trace_eval_map", file: !6, line: 533, flags: DIFlagFwdDecl)
!2870 = !DIDerivedType(tag: DW_TAG_member, name: "num_trace_evals", scope: !909, file: !6, line: 534, baseType: !7, size: 32, offset: 8448)
!2871 = !DIDerivedType(tag: DW_TAG_member, name: "kprobes_text_start", scope: !909, file: !6, line: 541, baseType: !40, size: 64, offset: 8512)
!2872 = !DIDerivedType(tag: DW_TAG_member, name: "kprobes_text_size", scope: !909, file: !6, line: 542, baseType: !7, size: 32, offset: 8576)
!2873 = !DIDerivedType(tag: DW_TAG_member, name: "kprobe_blacklist", scope: !909, file: !6, line: 543, baseType: !1440, size: 64, offset: 8640)
!2874 = !DIDerivedType(tag: DW_TAG_member, name: "num_kprobe_blacklist", scope: !909, file: !6, line: 544, baseType: !7, size: 32, offset: 8704)
!2875 = !DIDerivedType(tag: DW_TAG_member, name: "num_static_call_sites", scope: !909, file: !6, line: 547, baseType: !42, size: 32, offset: 8736)
!2876 = !DIDerivedType(tag: DW_TAG_member, name: "static_call_sites", scope: !909, file: !6, line: 548, baseType: !2877, size: 64, offset: 8768)
!2877 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2878, size: 64)
!2878 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "static_call_site", file: !2879, line: 32, size: 64, elements: !2880)
!2879 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/static_call_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "d6c482788c28845cbc4205380e7cd7fd")
!2880 = !{!2881, !2882}
!2881 = !DIDerivedType(tag: DW_TAG_member, name: "addr", scope: !2878, file: !2879, line: 33, baseType: !541, size: 32)
!2882 = !DIDerivedType(tag: DW_TAG_member, name: "key", scope: !2878, file: !2879, line: 34, baseType: !541, size: 32, offset: 32)
!2883 = !DIDerivedType(tag: DW_TAG_member, name: "source_list", scope: !909, file: !6, line: 573, baseType: !117, size: 128, offset: 8832)
!2884 = !DIDerivedType(tag: DW_TAG_member, name: "target_list", scope: !909, file: !6, line: 575, baseType: !117, size: 128, offset: 8960)
!2885 = !DIDerivedType(tag: DW_TAG_member, name: "exit", scope: !909, file: !6, line: 578, baseType: !2886, size: 64, offset: 9088)
!2886 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2887, size: 64)
!2887 = !DISubroutineType(types: !2888)
!2888 = !{null}
!2889 = !DIDerivedType(tag: DW_TAG_member, name: "refcnt", scope: !909, file: !6, line: 580, baseType: !69, size: 32, offset: 9152)
!2890 = !DIDerivedType(tag: DW_TAG_member, name: "fop_flags", scope: !905, file: !342, line: 2064, baseType: !2891, size: 32, offset: 64)
!2891 = !DIDerivedType(tag: DW_TAG_typedef, name: "fop_flags_t", file: !342, line: 2060, baseType: !7)
!2892 = !DIDerivedType(tag: DW_TAG_member, name: "llseek", scope: !905, file: !342, line: 2065, baseType: !2893, size: 64, offset: 128)
!2893 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2894, size: 64)
!2894 = !DISubroutineType(types: !2895)
!2895 = !{!61, !896, !61, !42}
!2896 = !DIDerivedType(tag: DW_TAG_member, name: "read", scope: !905, file: !342, line: 2066, baseType: !2897, size: 64, offset: 192)
!2897 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2898, size: 64)
!2898 = !DISubroutineType(types: !2899)
!2899 = !{!993, !896, !625, !55, !60}
!2900 = !DIDerivedType(tag: DW_TAG_member, name: "write", scope: !905, file: !342, line: 2067, baseType: !2901, size: 64, offset: 256)
!2901 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2902, size: 64)
!2902 = !DISubroutineType(types: !2903)
!2903 = !{!993, !896, !36, !55, !60}
!2904 = !DIDerivedType(tag: DW_TAG_member, name: "read_iter", scope: !905, file: !342, line: 2068, baseType: !1684, size: 64, offset: 320)
!2905 = !DIDerivedType(tag: DW_TAG_member, name: "write_iter", scope: !905, file: !342, line: 2069, baseType: !1684, size: 64, offset: 384)
!2906 = !DIDerivedType(tag: DW_TAG_member, name: "iopoll", scope: !905, file: !342, line: 2070, baseType: !2907, size: 64, offset: 448)
!2907 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2908, size: 64)
!2908 = !DISubroutineType(types: !2909)
!2909 = !{!42, !1687, !2910, !7}
!2910 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2911, size: 64)
!2911 = !DICompositeType(tag: DW_TAG_structure_type, name: "io_comp_batch", file: !342, line: 55, flags: DIFlagFwdDecl)
!2912 = !DIDerivedType(tag: DW_TAG_member, name: "iterate_shared", scope: !905, file: !342, line: 2072, baseType: !2913, size: 64, offset: 512)
!2913 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2914, size: 64)
!2914 = !DISubroutineType(types: !2915)
!2915 = !{!42, !896, !2916}
!2916 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2917, size: 64)
!2917 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "dir_context", file: !342, line: 2004, size: 128, elements: !2918)
!2918 = !{!2919, !2924}
!2919 = !DIDerivedType(tag: DW_TAG_member, name: "actor", scope: !2917, file: !342, line: 2005, baseType: !2920, size: 64)
!2920 = !DIDerivedType(tag: DW_TAG_typedef, name: "filldir_t", file: !342, line: 2001, baseType: !2921)
!2921 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2922, size: 64)
!2922 = !DISubroutineType(types: !2923)
!2923 = !{!614, !2916, !36, !42, !61, !519, !7}
!2924 = !DIDerivedType(tag: DW_TAG_member, name: "pos", scope: !2917, file: !342, line: 2006, baseType: !61, size: 64, offset: 64)
!2925 = !DIDerivedType(tag: DW_TAG_member, name: "poll", scope: !905, file: !342, line: 2073, baseType: !2926, size: 64, offset: 576)
!2926 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2927, size: 64)
!2927 = !DISubroutineType(types: !2928)
!2928 = !{!2521, !896, !2523}
!2929 = !DIDerivedType(tag: DW_TAG_member, name: "unlocked_ioctl", scope: !905, file: !342, line: 2074, baseType: !2930, size: 64, offset: 640)
!2930 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2931, size: 64)
!2931 = !DISubroutineType(types: !2932)
!2932 = !{!892, !896, !7, !59}
!2933 = !DIDerivedType(tag: DW_TAG_member, name: "compat_ioctl", scope: !905, file: !342, line: 2075, baseType: !2930, size: 64, offset: 704)
!2934 = !DIDerivedType(tag: DW_TAG_member, name: "mmap", scope: !905, file: !342, line: 2076, baseType: !2935, size: 64, offset: 768)
!2935 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2936, size: 64)
!2936 = !DISubroutineType(types: !2937)
!2937 = !{!42, !896, !1167}
!2938 = !DIDerivedType(tag: DW_TAG_member, name: "open", scope: !905, file: !342, line: 2077, baseType: !2939, size: 64, offset: 832)
!2939 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2940, size: 64)
!2940 = !DISubroutineType(types: !2941)
!2941 = !{!42, !779, !896}
!2942 = !DIDerivedType(tag: DW_TAG_member, name: "flush", scope: !905, file: !342, line: 2078, baseType: !2943, size: 64, offset: 896)
!2943 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2944, size: 64)
!2944 = !DISubroutineType(types: !2945)
!2945 = !{!42, !896, !2946}
!2946 = !DIDerivedType(tag: DW_TAG_typedef, name: "fl_owner_t", file: !342, line: 1102, baseType: !40)
!2947 = !DIDerivedType(tag: DW_TAG_member, name: "release", scope: !905, file: !342, line: 2079, baseType: !2939, size: 64, offset: 960)
!2948 = !DIDerivedType(tag: DW_TAG_member, name: "fsync", scope: !905, file: !342, line: 2080, baseType: !2949, size: 64, offset: 1024)
!2949 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2950, size: 64)
!2950 = !DISubroutineType(types: !2951)
!2951 = !{!42, !896, !61, !61, !42}
!2952 = !DIDerivedType(tag: DW_TAG_member, name: "fasync", scope: !905, file: !342, line: 2081, baseType: !2953, size: 64, offset: 1088)
!2953 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2954, size: 64)
!2954 = !DISubroutineType(types: !2955)
!2955 = !{!42, !42, !896, !42}
!2956 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !905, file: !342, line: 2082, baseType: !2957, size: 64, offset: 1152)
!2957 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2958, size: 64)
!2958 = !DISubroutineType(types: !2959)
!2959 = !{!42, !896, !42, !2960}
!2960 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2961, size: 64)
!2961 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "file_lock", file: !2962, line: 112, size: 1536, elements: !2963)
!2962 = !DIFile(filename: "LLM4Con/kernel_experiment/SYZBOT-3b6b32dc50537a49/src/include/linux/filelock.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "281697e55743fa0cc193620e99e911b9")
!2963 = !{!2964, !2982, !2983, !2984, !2997, !3021}
!2964 = !DIDerivedType(tag: DW_TAG_member, name: "c", scope: !2961, file: !2962, line: 113, baseType: !2965, size: 1024)
!2965 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "file_lock_core", file: !2962, line: 93, size: 1024, elements: !2966)
!2966 = !{!2967, !2969, !2970, !2971, !2972, !2973, !2974, !2975, !2976, !2979, !2980, !2981}
!2967 = !DIDerivedType(tag: DW_TAG_member, name: "flc_blocker", scope: !2965, file: !2962, line: 94, baseType: !2968, size: 64)
!2968 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2965, size: 64)
!2969 = !DIDerivedType(tag: DW_TAG_member, name: "flc_list", scope: !2965, file: !2962, line: 95, baseType: !117, size: 128, offset: 64)
!2970 = !DIDerivedType(tag: DW_TAG_member, name: "flc_link", scope: !2965, file: !2962, line: 96, baseType: !220, size: 128, offset: 192)
!2971 = !DIDerivedType(tag: DW_TAG_member, name: "flc_blocked_requests", scope: !2965, file: !2962, line: 97, baseType: !117, size: 128, offset: 320)
!2972 = !DIDerivedType(tag: DW_TAG_member, name: "flc_blocked_member", scope: !2965, file: !2962, line: 100, baseType: !117, size: 128, offset: 448)
!2973 = !DIDerivedType(tag: DW_TAG_member, name: "flc_owner", scope: !2965, file: !2962, line: 103, baseType: !2946, size: 64, offset: 576)
!2974 = !DIDerivedType(tag: DW_TAG_member, name: "flc_flags", scope: !2965, file: !2962, line: 104, baseType: !7, size: 32, offset: 640)
!2975 = !DIDerivedType(tag: DW_TAG_member, name: "flc_type", scope: !2965, file: !2962, line: 105, baseType: !107, size: 8, offset: 672)
!2976 = !DIDerivedType(tag: DW_TAG_member, name: "flc_pid", scope: !2965, file: !2962, line: 106, baseType: !2977, size: 32, offset: 704)
!2977 = !DIDerivedType(tag: DW_TAG_typedef, name: "pid_t", file: !45, line: 27, baseType: !2978)
!2978 = !DIDerivedType(tag: DW_TAG_typedef, name: "__kernel_pid_t", file: !57, line: 28, baseType: !42)
!2979 = !DIDerivedType(tag: DW_TAG_member, name: "flc_link_cpu", scope: !2965, file: !2962, line: 107, baseType: !42, size: 32, offset: 736)
!2980 = !DIDerivedType(tag: DW_TAG_member, name: "flc_wait", scope: !2965, file: !2962, line: 108, baseType: !74, size: 192, offset: 768)
!2981 = !DIDerivedType(tag: DW_TAG_member, name: "flc_file", scope: !2965, file: !2962, line: 109, baseType: !896, size: 64, offset: 960)
!2982 = !DIDerivedType(tag: DW_TAG_member, name: "fl_start", scope: !2961, file: !2962, line: 114, baseType: !61, size: 64, offset: 1024)
!2983 = !DIDerivedType(tag: DW_TAG_member, name: "fl_end", scope: !2961, file: !2962, line: 115, baseType: !61, size: 64, offset: 1088)
!2984 = !DIDerivedType(tag: DW_TAG_member, name: "fl_ops", scope: !2961, file: !2962, line: 117, baseType: !2985, size: 64, offset: 1152)
!2985 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2986, size: 64)
!2986 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !2987)
!2987 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "file_lock_operations", file: !2962, line: 32, size: 128, elements: !2988)
!2988 = !{!2989, !2993}
!2989 = !DIDerivedType(tag: DW_TAG_member, name: "fl_copy_lock", scope: !2987, file: !2962, line: 33, baseType: !2990, size: 64)
!2990 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2991, size: 64)
!2991 = !DISubroutineType(types: !2992)
!2992 = !{null, !2960, !2960}
!2993 = !DIDerivedType(tag: DW_TAG_member, name: "fl_release_private", scope: !2987, file: !2962, line: 34, baseType: !2994, size: 64, offset: 64)
!2994 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2995, size: 64)
!2995 = !DISubroutineType(types: !2996)
!2996 = !{null, !2960}
!2997 = !DIDerivedType(tag: DW_TAG_member, name: "fl_lmops", scope: !2961, file: !2962, line: 118, baseType: !2998, size: 64, offset: 1216)
!2998 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2999, size: 64)
!2999 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !3000)
!3000 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "lock_manager_operations", file: !2962, line: 37, size: 448, elements: !3001)
!3001 = !{!3002, !3003, !3007, !3011, !3012, !3016, !3020}
!3002 = !DIDerivedType(tag: DW_TAG_member, name: "lm_mod_owner", scope: !3000, file: !2962, line: 38, baseType: !40, size: 64)
!3003 = !DIDerivedType(tag: DW_TAG_member, name: "lm_get_owner", scope: !3000, file: !2962, line: 39, baseType: !3004, size: 64, offset: 64)
!3004 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3005, size: 64)
!3005 = !DISubroutineType(types: !3006)
!3006 = !{!2946, !2946}
!3007 = !DIDerivedType(tag: DW_TAG_member, name: "lm_put_owner", scope: !3000, file: !2962, line: 40, baseType: !3008, size: 64, offset: 128)
!3008 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3009, size: 64)
!3009 = !DISubroutineType(types: !3010)
!3010 = !{null, !2946}
!3011 = !DIDerivedType(tag: DW_TAG_member, name: "lm_notify", scope: !3000, file: !2962, line: 41, baseType: !2994, size: 64, offset: 192)
!3012 = !DIDerivedType(tag: DW_TAG_member, name: "lm_grant", scope: !3000, file: !2962, line: 42, baseType: !3013, size: 64, offset: 256)
!3013 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3014, size: 64)
!3014 = !DISubroutineType(types: !3015)
!3015 = !{!42, !2960, !42}
!3016 = !DIDerivedType(tag: DW_TAG_member, name: "lm_lock_expirable", scope: !3000, file: !2962, line: 43, baseType: !3017, size: 64, offset: 320)
!3017 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3018, size: 64)
!3018 = !DISubroutineType(types: !3019)
!3019 = !{!614, !2960}
!3020 = !DIDerivedType(tag: DW_TAG_member, name: "lm_expire_lock", scope: !3000, file: !2962, line: 44, baseType: !2886, size: 64, offset: 384)
!3021 = !DIDerivedType(tag: DW_TAG_member, name: "fl_u", scope: !2961, file: !2962, line: 130, baseType: !3022, size: 256, offset: 1280)
!3022 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !2961, file: !2962, line: 119, size: 256, elements: !3023)
!3023 = !{!3024, !3033, !3039, !3045}
!3024 = !DIDerivedType(tag: DW_TAG_member, name: "nfs_fl", scope: !3022, file: !2962, line: 120, baseType: !3025, size: 256)
!3025 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "nfs_lock_info", file: !3026, line: 10, size: 256, elements: !3027)
!3026 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/nfs_fs_i.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "c146eb6cc450a23c1b61e3c824cbd799")
!3027 = !{!3028, !3029, !3032}
!3028 = !DIDerivedType(tag: DW_TAG_member, name: "state", scope: !3025, file: !3026, line: 11, baseType: !578, size: 32)
!3029 = !DIDerivedType(tag: DW_TAG_member, name: "owner", scope: !3025, file: !3026, line: 12, baseType: !3030, size: 64, offset: 64)
!3030 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3031, size: 64)
!3031 = !DICompositeType(tag: DW_TAG_structure_type, name: "nlm_lockowner", file: !3026, line: 5, flags: DIFlagFwdDecl)
!3032 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !3025, file: !3026, line: 13, baseType: !117, size: 128, offset: 128)
!3033 = !DIDerivedType(tag: DW_TAG_member, name: "nfs4_fl", scope: !3022, file: !2962, line: 121, baseType: !3034, size: 64)
!3034 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "nfs4_lock_info", file: !3026, line: 17, size: 64, elements: !3035)
!3035 = !{!3036}
!3036 = !DIDerivedType(tag: DW_TAG_member, name: "owner", scope: !3034, file: !3026, line: 18, baseType: !3037, size: 64)
!3037 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3038, size: 64)
!3038 = !DICompositeType(tag: DW_TAG_structure_type, name: "nfs4_lock_state", file: !3026, line: 16, flags: DIFlagFwdDecl)
!3039 = !DIDerivedType(tag: DW_TAG_member, name: "afs", scope: !3022, file: !2962, line: 126, baseType: !3040, size: 192)
!3040 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !3022, file: !2962, line: 122, size: 192, elements: !3041)
!3041 = !{!3042, !3043, !3044}
!3042 = !DIDerivedType(tag: DW_TAG_member, name: "link", scope: !3040, file: !2962, line: 123, baseType: !117, size: 128)
!3043 = !DIDerivedType(tag: DW_TAG_member, name: "state", scope: !3040, file: !2962, line: 124, baseType: !42, size: 32, offset: 128)
!3044 = !DIDerivedType(tag: DW_TAG_member, name: "debug_id", scope: !3040, file: !2962, line: 125, baseType: !7, size: 32, offset: 160)
!3045 = !DIDerivedType(tag: DW_TAG_member, name: "ceph", scope: !3022, file: !2962, line: 129, baseType: !3046, size: 64)
!3046 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !3022, file: !2962, line: 127, size: 64, elements: !3047)
!3047 = !{!3048}
!3048 = !DIDerivedType(tag: DW_TAG_member, name: "inode", scope: !3046, file: !2962, line: 128, baseType: !779, size: 64)
!3049 = !DIDerivedType(tag: DW_TAG_member, name: "get_unmapped_area", scope: !905, file: !342, line: 2083, baseType: !3050, size: 64, offset: 1216)
!3050 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3051, size: 64)
!3051 = !DISubroutineType(types: !3052)
!3052 = !{!59, !896, !59, !59, !59, !59}
!3053 = !DIDerivedType(tag: DW_TAG_member, name: "check_flags", scope: !905, file: !342, line: 2084, baseType: !3054, size: 64, offset: 1280)
!3054 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3055, size: 64)
!3055 = !DISubroutineType(types: !3056)
!3056 = !{!42, !42}
!3057 = !DIDerivedType(tag: DW_TAG_member, name: "flock", scope: !905, file: !342, line: 2085, baseType: !2957, size: 64, offset: 1344)
!3058 = !DIDerivedType(tag: DW_TAG_member, name: "splice_write", scope: !905, file: !342, line: 2086, baseType: !3059, size: 64, offset: 1408)
!3059 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3060, size: 64)
!3060 = !DISubroutineType(types: !3061)
!3061 = !{!993, !3062, !896, !60, !55, !7}
!3062 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3063, size: 64)
!3063 = !DICompositeType(tag: DW_TAG_structure_type, name: "pipe_inode_info", file: !1435, line: 69, flags: DIFlagFwdDecl)
!3064 = !DIDerivedType(tag: DW_TAG_member, name: "splice_read", scope: !905, file: !342, line: 2087, baseType: !3065, size: 64, offset: 1472)
!3065 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3066, size: 64)
!3066 = !DISubroutineType(types: !3067)
!3067 = !{!993, !896, !60, !3062, !55, !7}
!3068 = !DIDerivedType(tag: DW_TAG_member, name: "splice_eof", scope: !905, file: !342, line: 2088, baseType: !2362, size: 64, offset: 1536)
!3069 = !DIDerivedType(tag: DW_TAG_member, name: "setlease", scope: !905, file: !342, line: 2089, baseType: !3070, size: 64, offset: 1600)
!3070 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3071, size: 64)
!3071 = !DISubroutineType(types: !3072)
!3072 = !{!42, !896, !42, !3073, !1661}
!3073 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3074, size: 64)
!3074 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3075, size: 64)
!3075 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "file_lease", file: !2962, line: 133, size: 1280, elements: !3076)
!3076 = !{!3077, !3078, !3108, !3109, !3110}
!3077 = !DIDerivedType(tag: DW_TAG_member, name: "c", scope: !3075, file: !2962, line: 134, baseType: !2965, size: 1024)
!3078 = !DIDerivedType(tag: DW_TAG_member, name: "fl_fasync", scope: !3075, file: !2962, line: 135, baseType: !3079, size: 64, offset: 1024)
!3079 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3080, size: 64)
!3080 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "fasync_struct", file: !342, line: 1142, size: 384, elements: !3081)
!3081 = !{!3082, !3103, !3104, !3105, !3106, !3107}
!3082 = !DIDerivedType(tag: DW_TAG_member, name: "fa_lock", scope: !3080, file: !342, line: 1143, baseType: !3083, size: 64)
!3083 = !DIDerivedType(tag: DW_TAG_typedef, name: "rwlock_t", file: !3084, line: 34, baseType: !3085)
!3084 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/rwlock_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "dff80c89ec6551b7d56ae9ff5387a240")
!3085 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !3084, line: 25, size: 64, elements: !3086)
!3086 = !{!3087}
!3087 = !DIDerivedType(tag: DW_TAG_member, name: "raw_lock", scope: !3085, file: !3084, line: 26, baseType: !3088, size: 64)
!3088 = !DIDerivedType(tag: DW_TAG_typedef, name: "arch_rwlock_t", file: !3089, line: 27, baseType: !3090)
!3089 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/asm-generic/qrwlock_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "724e2bc1afabb3b7b6860c9799b9cd27")
!3090 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "qrwlock", file: !3089, line: 13, size: 64, elements: !3091)
!3091 = !{!3092, !3102}
!3092 = !DIDerivedType(tag: DW_TAG_member, scope: !3090, file: !3089, line: 14, baseType: !3093, size: 32)
!3093 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !3090, file: !3089, line: 14, size: 32, elements: !3094)
!3094 = !{!3095, !3096}
!3095 = !DIDerivedType(tag: DW_TAG_member, name: "cnts", scope: !3093, file: !3089, line: 15, baseType: !69, size: 32)
!3096 = !DIDerivedType(tag: DW_TAG_member, scope: !3093, file: !3089, line: 16, baseType: !3097, size: 32)
!3097 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !3093, file: !3089, line: 16, size: 32, elements: !3098)
!3098 = !{!3099, !3100}
!3099 = !DIDerivedType(tag: DW_TAG_member, name: "wlocked", scope: !3097, file: !3089, line: 18, baseType: !103, size: 8)
!3100 = !DIDerivedType(tag: DW_TAG_member, name: "__lstate", scope: !3097, file: !3089, line: 19, baseType: !3101, size: 24, offset: 8)
!3101 = !DICompositeType(tag: DW_TAG_array_type, baseType: !103, size: 24, elements: !962)
!3102 = !DIDerivedType(tag: DW_TAG_member, name: "wait_lock", scope: !3090, file: !3089, line: 26, baseType: !91, size: 32, offset: 32)
!3103 = !DIDerivedType(tag: DW_TAG_member, name: "magic", scope: !3080, file: !342, line: 1144, baseType: !42, size: 32, offset: 64)
!3104 = !DIDerivedType(tag: DW_TAG_member, name: "fa_fd", scope: !3080, file: !342, line: 1145, baseType: !42, size: 32, offset: 96)
!3105 = !DIDerivedType(tag: DW_TAG_member, name: "fa_next", scope: !3080, file: !342, line: 1146, baseType: !3079, size: 64, offset: 128)
!3106 = !DIDerivedType(tag: DW_TAG_member, name: "fa_file", scope: !3080, file: !342, line: 1147, baseType: !896, size: 64, offset: 192)
!3107 = !DIDerivedType(tag: DW_TAG_member, name: "fa_rcu", scope: !3080, file: !342, line: 1148, baseType: !129, size: 128, align: 64, offset: 256)
!3108 = !DIDerivedType(tag: DW_TAG_member, name: "fl_break_time", scope: !3075, file: !2962, line: 137, baseType: !59, size: 64, offset: 1088)
!3109 = !DIDerivedType(tag: DW_TAG_member, name: "fl_downgrade_time", scope: !3075, file: !2962, line: 138, baseType: !59, size: 64, offset: 1152)
!3110 = !DIDerivedType(tag: DW_TAG_member, name: "fl_lmops", scope: !3075, file: !2962, line: 139, baseType: !3111, size: 64, offset: 1216)
!3111 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3112, size: 64)
!3112 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !3113)
!3113 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "lease_manager_operations", file: !2962, line: 47, size: 256, elements: !3114)
!3114 = !{!3115, !3119, !3123, !3127}
!3115 = !DIDerivedType(tag: DW_TAG_member, name: "lm_break", scope: !3113, file: !2962, line: 48, baseType: !3116, size: 64)
!3116 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3117, size: 64)
!3117 = !DISubroutineType(types: !3118)
!3118 = !{!614, !3074}
!3119 = !DIDerivedType(tag: DW_TAG_member, name: "lm_change", scope: !3113, file: !2962, line: 49, baseType: !3120, size: 64, offset: 64)
!3120 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3121, size: 64)
!3121 = !DISubroutineType(types: !3122)
!3122 = !{!42, !3074, !42, !120}
!3123 = !DIDerivedType(tag: DW_TAG_member, name: "lm_setup", scope: !3113, file: !2962, line: 50, baseType: !3124, size: 64, offset: 128)
!3124 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3125, size: 64)
!3125 = !DISubroutineType(types: !3126)
!3126 = !{null, !3074, !1661}
!3127 = !DIDerivedType(tag: DW_TAG_member, name: "lm_breaker_owns_lease", scope: !3113, file: !2962, line: 51, baseType: !3116, size: 64, offset: 192)
!3128 = !DIDerivedType(tag: DW_TAG_member, name: "fallocate", scope: !905, file: !342, line: 2090, baseType: !3129, size: 64, offset: 1664)
!3129 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3130, size: 64)
!3130 = !DISubroutineType(types: !3131)
!3131 = !{!892, !896, !42, !61, !61}
!3132 = !DIDerivedType(tag: DW_TAG_member, name: "show_fdinfo", scope: !905, file: !342, line: 2092, baseType: !3133, size: 64, offset: 1728)
!3133 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3134, size: 64)
!3134 = !DISubroutineType(types: !3135)
!3135 = !{null, !2454, !896}
!3136 = !DIDerivedType(tag: DW_TAG_member, name: "copy_file_range", scope: !905, file: !342, line: 2096, baseType: !3137, size: 64, offset: 1792)
!3137 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3138, size: 64)
!3138 = !DISubroutineType(types: !3139)
!3139 = !{!993, !896, !61, !896, !61, !55, !7}
!3140 = !DIDerivedType(tag: DW_TAG_member, name: "remap_file_range", scope: !905, file: !342, line: 2098, baseType: !3141, size: 64, offset: 1856)
!3141 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3142, size: 64)
!3142 = !DISubroutineType(types: !3143)
!3143 = !{!61, !896, !61, !896, !61, !61, !7}
!3144 = !DIDerivedType(tag: DW_TAG_member, name: "fadvise", scope: !905, file: !342, line: 2101, baseType: !2949, size: 64, offset: 1920)
!3145 = !DIDerivedType(tag: DW_TAG_member, name: "uring_cmd", scope: !905, file: !342, line: 2102, baseType: !3146, size: 64, offset: 1984)
!3146 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3147, size: 64)
!3147 = !DISubroutineType(types: !3148)
!3148 = !{!42, !3149, !7}
!3149 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3150, size: 64)
!3150 = !DICompositeType(tag: DW_TAG_structure_type, name: "io_uring_cmd", file: !342, line: 2057, flags: DIFlagFwdDecl)
!3151 = !DIDerivedType(tag: DW_TAG_member, name: "uring_cmd_iopoll", scope: !905, file: !342, line: 2103, baseType: !3152, size: 64, offset: 2048)
!3152 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3153, size: 64)
!3153 = !DISubroutineType(types: !3154)
!3154 = !{!42, !3149, !2910, !7}
!3155 = !DIDerivedType(tag: DW_TAG_member, name: "f_mapping", scope: !897, file: !342, line: 1037, baseType: !1030, size: 64, offset: 192)
!3156 = !DIDerivedType(tag: DW_TAG_member, name: "private_data", scope: !897, file: !342, line: 1038, baseType: !40, size: 64, offset: 256)
!3157 = !DIDerivedType(tag: DW_TAG_member, name: "f_inode", scope: !897, file: !342, line: 1039, baseType: !779, size: 64, offset: 320)
!3158 = !DIDerivedType(tag: DW_TAG_member, name: "f_flags", scope: !897, file: !342, line: 1040, baseType: !7, size: 32, offset: 384)
!3159 = !DIDerivedType(tag: DW_TAG_member, name: "f_iocb_flags", scope: !897, file: !342, line: 1041, baseType: !7, size: 32, offset: 416)
!3160 = !DIDerivedType(tag: DW_TAG_member, name: "f_cred", scope: !897, file: !342, line: 1042, baseType: !490, size: 64, offset: 448)
!3161 = !DIDerivedType(tag: DW_TAG_member, name: "f_path", scope: !897, file: !342, line: 1044, baseType: !3162, size: 128, offset: 512)
!3162 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "path", file: !3163, line: 8, size: 128, elements: !3164)
!3163 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/path.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "ca447ee15c3dc940e817ab4116354108")
!3164 = !{!3165, !5363}
!3165 = !DIDerivedType(tag: DW_TAG_member, name: "mnt", scope: !3162, file: !3163, line: 9, baseType: !3166, size: 64)
!3166 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3167, size: 64)
!3167 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "vfsmount", file: !3168, line: 69, size: 256, elements: !3169)
!3168 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/mount.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "5f7989542e995cf404c30dba38fb6b86")
!3169 = !{!3170, !3171, !5361, !5362}
!3170 = !DIDerivedType(tag: DW_TAG_member, name: "mnt_root", scope: !3167, file: !3168, line: 70, baseType: !740, size: 64)
!3171 = !DIDerivedType(tag: DW_TAG_member, name: "mnt_sb", scope: !3167, file: !3168, line: 71, baseType: !3172, size: 64, offset: 64)
!3172 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3173, size: 64)
!3173 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "super_block", file: !342, line: 1253, size: 10240, elements: !3174)
!3174 = !{!3175, !3176, !3177, !3178, !3179, !3180, !3217, !3349, !3389, !3474, !3478, !3479, !3480, !3481, !3482, !3483, !3484, !3485, !3486, !3492, !3496, !3497, !3498, !3499, !3557, !3560, !3561, !3562, !3603, !5214, !5215, !5216, !5217, !5218, !5219, !5239, !5240, !5247, !5248, !5252, !5253, !5254, !5255, !5312, !5331, !5332, !5333, !5334, !5335, !5336, !5337, !5352, !5353, !5354, !5355, !5356, !5357, !5358, !5359, !5360}
!3175 = !DIDerivedType(tag: DW_TAG_member, name: "s_list", scope: !3173, file: !342, line: 1254, baseType: !117, size: 128)
!3176 = !DIDerivedType(tag: DW_TAG_member, name: "s_dev", scope: !3173, file: !342, line: 1255, baseType: !852, size: 32, offset: 128)
!3177 = !DIDerivedType(tag: DW_TAG_member, name: "s_blocksize_bits", scope: !3173, file: !342, line: 1256, baseType: !107, size: 8, offset: 160)
!3178 = !DIDerivedType(tag: DW_TAG_member, name: "s_blocksize", scope: !3173, file: !342, line: 1257, baseType: !59, size: 64, offset: 192)
!3179 = !DIDerivedType(tag: DW_TAG_member, name: "s_maxbytes", scope: !3173, file: !342, line: 1258, baseType: !61, size: 64, offset: 256)
!3180 = !DIDerivedType(tag: DW_TAG_member, name: "s_type", scope: !3173, file: !342, line: 1259, baseType: !3181, size: 64, offset: 320)
!3181 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3182, size: 64)
!3182 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "file_system_type", file: !342, line: 2538, size: 576, elements: !3183)
!3183 = !{!3184, !3185, !3186, !3192, !3196, !3200, !3204, !3205, !3206, !3207, !3209, !3210, !3211, !3213, !3214, !3215, !3216}
!3184 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !3182, file: !342, line: 2539, baseType: !36, size: 64)
!3185 = !DIDerivedType(tag: DW_TAG_member, name: "fs_flags", scope: !3182, file: !342, line: 2540, baseType: !42, size: 32, offset: 64)
!3186 = !DIDerivedType(tag: DW_TAG_member, name: "init_fs_context", scope: !3182, file: !342, line: 2548, baseType: !3187, size: 64, offset: 128)
!3187 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3188, size: 64)
!3188 = !DISubroutineType(types: !3189)
!3189 = !{!42, !3190}
!3190 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3191, size: 64)
!3191 = !DICompositeType(tag: DW_TAG_structure_type, name: "fs_context", file: !3168, line: 21, flags: DIFlagFwdDecl)
!3192 = !DIDerivedType(tag: DW_TAG_member, name: "parameters", scope: !3182, file: !342, line: 2549, baseType: !3193, size: 64, offset: 192)
!3193 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3194, size: 64)
!3194 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !3195)
!3195 = !DICompositeType(tag: DW_TAG_structure_type, name: "fs_parameter_spec", file: !342, line: 79, flags: DIFlagFwdDecl)
!3196 = !DIDerivedType(tag: DW_TAG_member, name: "mount", scope: !3182, file: !342, line: 2550, baseType: !3197, size: 64, offset: 256)
!3197 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3198, size: 64)
!3198 = !DISubroutineType(types: !3199)
!3199 = !{!740, !3181, !42, !36, !40}
!3200 = !DIDerivedType(tag: DW_TAG_member, name: "kill_sb", scope: !3182, file: !342, line: 2552, baseType: !3201, size: 64, offset: 320)
!3201 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3202, size: 64)
!3202 = !DISubroutineType(types: !3203)
!3203 = !{null, !3172}
!3204 = !DIDerivedType(tag: DW_TAG_member, name: "owner", scope: !3182, file: !342, line: 2553, baseType: !908, size: 64, offset: 384)
!3205 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !3182, file: !342, line: 2554, baseType: !3181, size: 64, offset: 448)
!3206 = !DIDerivedType(tag: DW_TAG_member, name: "fs_supers", scope: !3182, file: !342, line: 2555, baseType: !216, size: 64, offset: 512)
!3207 = !DIDerivedType(tag: DW_TAG_member, name: "s_lock_key", scope: !3182, file: !342, line: 2557, baseType: !3208, offset: 576)
!3208 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "lock_class_key", file: !2812, line: 264, elements: !1201)
!3209 = !DIDerivedType(tag: DW_TAG_member, name: "s_umount_key", scope: !3182, file: !342, line: 2558, baseType: !3208, offset: 576)
!3210 = !DIDerivedType(tag: DW_TAG_member, name: "s_vfs_rename_key", scope: !3182, file: !342, line: 2559, baseType: !3208, offset: 576)
!3211 = !DIDerivedType(tag: DW_TAG_member, name: "s_writers_key", scope: !3182, file: !342, line: 2560, baseType: !3212, offset: 576)
!3212 = !DICompositeType(tag: DW_TAG_array_type, baseType: !3208, elements: !962)
!3213 = !DIDerivedType(tag: DW_TAG_member, name: "i_lock_key", scope: !3182, file: !342, line: 2562, baseType: !3208, offset: 576)
!3214 = !DIDerivedType(tag: DW_TAG_member, name: "i_mutex_key", scope: !3182, file: !342, line: 2563, baseType: !3208, offset: 576)
!3215 = !DIDerivedType(tag: DW_TAG_member, name: "invalidate_lock_key", scope: !3182, file: !342, line: 2564, baseType: !3208, offset: 576)
!3216 = !DIDerivedType(tag: DW_TAG_member, name: "i_mutex_dir_key", scope: !3182, file: !342, line: 2565, baseType: !3208, offset: 576)
!3217 = !DIDerivedType(tag: DW_TAG_member, name: "s_op", scope: !3173, file: !342, line: 1260, baseType: !3218, size: 64, offset: 384)
!3218 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3219, size: 64)
!3219 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !3220)
!3220 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "super_operations", file: !342, line: 2218, size: 1664, elements: !3221)
!3221 = !{!3222, !3226, !3230, !3231, !3235, !3239, !3243, !3244, !3245, !3249, !3253, !3257, !3258, !3259, !3265, !3269, !3270, !3274, !3275, !3276, !3277, !3281, !3285, !3332, !3347, !3348}
!3222 = !DIDerivedType(tag: DW_TAG_member, name: "alloc_inode", scope: !3220, file: !342, line: 2219, baseType: !3223, size: 64)
!3223 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3224, size: 64)
!3224 = !DISubroutineType(types: !3225)
!3225 = !{!779, !3172}
!3226 = !DIDerivedType(tag: DW_TAG_member, name: "destroy_inode", scope: !3220, file: !342, line: 2220, baseType: !3227, size: 64, offset: 64)
!3227 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3228, size: 64)
!3228 = !DISubroutineType(types: !3229)
!3229 = !{null, !779}
!3230 = !DIDerivedType(tag: DW_TAG_member, name: "free_inode", scope: !3220, file: !342, line: 2221, baseType: !3227, size: 64, offset: 128)
!3231 = !DIDerivedType(tag: DW_TAG_member, name: "dirty_inode", scope: !3220, file: !342, line: 2223, baseType: !3232, size: 64, offset: 192)
!3232 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3233, size: 64)
!3233 = !DISubroutineType(types: !3234)
!3234 = !{null, !779, !42}
!3235 = !DIDerivedType(tag: DW_TAG_member, name: "write_inode", scope: !3220, file: !342, line: 2224, baseType: !3236, size: 64, offset: 256)
!3236 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3237, size: 64)
!3237 = !DISubroutineType(types: !3238)
!3238 = !{!42, !779, !1519}
!3239 = !DIDerivedType(tag: DW_TAG_member, name: "drop_inode", scope: !3220, file: !342, line: 2225, baseType: !3240, size: 64, offset: 320)
!3240 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3241, size: 64)
!3241 = !DISubroutineType(types: !3242)
!3242 = !{!42, !779}
!3243 = !DIDerivedType(tag: DW_TAG_member, name: "evict_inode", scope: !3220, file: !342, line: 2226, baseType: !3227, size: 64, offset: 384)
!3244 = !DIDerivedType(tag: DW_TAG_member, name: "put_super", scope: !3220, file: !342, line: 2227, baseType: !3201, size: 64, offset: 448)
!3245 = !DIDerivedType(tag: DW_TAG_member, name: "sync_fs", scope: !3220, file: !342, line: 2228, baseType: !3246, size: 64, offset: 512)
!3246 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3247, size: 64)
!3247 = !DISubroutineType(types: !3248)
!3248 = !{!42, !3172, !42}
!3249 = !DIDerivedType(tag: DW_TAG_member, name: "freeze_super", scope: !3220, file: !342, line: 2229, baseType: !3250, size: 64, offset: 576)
!3250 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3251, size: 64)
!3251 = !DISubroutineType(types: !3252)
!3252 = !{!42, !3172, !341}
!3253 = !DIDerivedType(tag: DW_TAG_member, name: "freeze_fs", scope: !3220, file: !342, line: 2230, baseType: !3254, size: 64, offset: 640)
!3254 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3255, size: 64)
!3255 = !DISubroutineType(types: !3256)
!3256 = !{!42, !3172}
!3257 = !DIDerivedType(tag: DW_TAG_member, name: "thaw_super", scope: !3220, file: !342, line: 2231, baseType: !3250, size: 64, offset: 704)
!3258 = !DIDerivedType(tag: DW_TAG_member, name: "unfreeze_fs", scope: !3220, file: !342, line: 2232, baseType: !3254, size: 64, offset: 768)
!3259 = !DIDerivedType(tag: DW_TAG_member, name: "statfs", scope: !3220, file: !342, line: 2233, baseType: !3260, size: 64, offset: 832)
!3260 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3261, size: 64)
!3261 = !DISubroutineType(types: !3262)
!3262 = !{!42, !740, !3263}
!3263 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3264, size: 64)
!3264 = !DICompositeType(tag: DW_TAG_structure_type, name: "kstatfs", file: !342, line: 64, flags: DIFlagFwdDecl)
!3265 = !DIDerivedType(tag: DW_TAG_member, name: "remount_fs", scope: !3220, file: !342, line: 2234, baseType: !3266, size: 64, offset: 896)
!3266 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3267, size: 64)
!3267 = !DISubroutineType(types: !3268)
!3268 = !{!42, !3172, !2694, !625}
!3269 = !DIDerivedType(tag: DW_TAG_member, name: "umount_begin", scope: !3220, file: !342, line: 2235, baseType: !3201, size: 64, offset: 960)
!3270 = !DIDerivedType(tag: DW_TAG_member, name: "show_options", scope: !3220, file: !342, line: 2237, baseType: !3271, size: 64, offset: 1024)
!3271 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3272, size: 64)
!3272 = !DISubroutineType(types: !3273)
!3273 = !{!42, !2454, !740}
!3274 = !DIDerivedType(tag: DW_TAG_member, name: "show_devname", scope: !3220, file: !342, line: 2238, baseType: !3271, size: 64, offset: 1088)
!3275 = !DIDerivedType(tag: DW_TAG_member, name: "show_path", scope: !3220, file: !342, line: 2239, baseType: !3271, size: 64, offset: 1152)
!3276 = !DIDerivedType(tag: DW_TAG_member, name: "show_stats", scope: !3220, file: !342, line: 2240, baseType: !3271, size: 64, offset: 1216)
!3277 = !DIDerivedType(tag: DW_TAG_member, name: "quota_read", scope: !3220, file: !342, line: 2242, baseType: !3278, size: 64, offset: 1280)
!3278 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3279, size: 64)
!3279 = !DISubroutineType(types: !3280)
!3280 = !{!993, !3172, !42, !625, !55, !61}
!3281 = !DIDerivedType(tag: DW_TAG_member, name: "quota_write", scope: !3220, file: !342, line: 2243, baseType: !3282, size: 64, offset: 1344)
!3282 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3283, size: 64)
!3283 = !DISubroutineType(types: !3284)
!3284 = !{!993, !3172, !42, !36, !55, !61}
!3285 = !DIDerivedType(tag: DW_TAG_member, name: "get_dquots", scope: !3220, file: !342, line: 2244, baseType: !3286, size: 64, offset: 1408)
!3286 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3287, size: 64)
!3287 = !DISubroutineType(types: !3288)
!3288 = !{!3289, !779}
!3289 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3290, size: 64)
!3290 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3291, size: 64)
!3291 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "dquot", file: !348, line: 296, size: 1664, elements: !3292)
!3292 = !{!3293, !3294, !3295, !3296, !3297, !3298, !3299, !3300, !3301, !3317, !3318, !3319}
!3293 = !DIDerivedType(tag: DW_TAG_member, name: "dq_hash", scope: !3291, file: !348, line: 297, baseType: !220, size: 128)
!3294 = !DIDerivedType(tag: DW_TAG_member, name: "dq_inuse", scope: !3291, file: !348, line: 298, baseType: !117, size: 128, offset: 128)
!3295 = !DIDerivedType(tag: DW_TAG_member, name: "dq_free", scope: !3291, file: !348, line: 299, baseType: !117, size: 128, offset: 256)
!3296 = !DIDerivedType(tag: DW_TAG_member, name: "dq_dirty", scope: !3291, file: !348, line: 300, baseType: !117, size: 128, offset: 384)
!3297 = !DIDerivedType(tag: DW_TAG_member, name: "dq_lock", scope: !3291, file: !348, line: 301, baseType: !1277, size: 256, offset: 512)
!3298 = !DIDerivedType(tag: DW_TAG_member, name: "dq_dqb_lock", scope: !3291, file: !348, line: 302, baseType: !79, size: 32, offset: 768)
!3299 = !DIDerivedType(tag: DW_TAG_member, name: "dq_count", scope: !3291, file: !348, line: 303, baseType: !69, size: 32, offset: 800)
!3300 = !DIDerivedType(tag: DW_TAG_member, name: "dq_sb", scope: !3291, file: !348, line: 304, baseType: !3172, size: 64, offset: 832)
!3301 = !DIDerivedType(tag: DW_TAG_member, name: "dq_id", scope: !3291, file: !348, line: 305, baseType: !3302, size: 64, offset: 896)
!3302 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kqid", file: !348, line: 68, size: 64, elements: !3303)
!3303 = !{!3304, !3316}
!3304 = !DIDerivedType(tag: DW_TAG_member, scope: !3302, file: !348, line: 69, baseType: !3305, size: 32)
!3305 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !3302, file: !348, line: 69, size: 32, elements: !3306)
!3306 = !{!3307, !3308, !3309}
!3307 = !DIDerivedType(tag: DW_TAG_member, name: "uid", scope: !3305, file: !348, line: 70, baseType: !188, size: 32)
!3308 = !DIDerivedType(tag: DW_TAG_member, name: "gid", scope: !3305, file: !348, line: 71, baseType: !196, size: 32)
!3309 = !DIDerivedType(tag: DW_TAG_member, name: "projid", scope: !3305, file: !348, line: 72, baseType: !3310, size: 32)
!3310 = !DIDerivedType(tag: DW_TAG_typedef, name: "kprojid_t", file: !3311, line: 24, baseType: !3312)
!3311 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/projid.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "05e7f99607decf45353adf862137f54e")
!3312 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !3311, line: 22, size: 32, elements: !3313)
!3313 = !{!3314}
!3314 = !DIDerivedType(tag: DW_TAG_member, name: "val", scope: !3312, file: !3311, line: 23, baseType: !3315, size: 32)
!3315 = !DIDerivedType(tag: DW_TAG_typedef, name: "projid_t", file: !3311, line: 20, baseType: !194)
!3316 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !3302, file: !348, line: 74, baseType: !347, size: 32, offset: 32)
!3317 = !DIDerivedType(tag: DW_TAG_member, name: "dq_off", scope: !3291, file: !348, line: 306, baseType: !61, size: 64, offset: 960)
!3318 = !DIDerivedType(tag: DW_TAG_member, name: "dq_flags", scope: !3291, file: !348, line: 307, baseType: !59, size: 64, offset: 1024)
!3319 = !DIDerivedType(tag: DW_TAG_member, name: "dq_dqb", scope: !3291, file: !348, line: 308, baseType: !3320, size: 576, offset: 1088)
!3320 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mem_dqblk", file: !348, line: 205, size: 576, elements: !3321)
!3321 = !{!3322, !3324, !3325, !3326, !3327, !3328, !3329, !3330, !3331}
!3322 = !DIDerivedType(tag: DW_TAG_member, name: "dqb_bhardlimit", scope: !3320, file: !348, line: 206, baseType: !3323, size: 64)
!3323 = !DIDerivedType(tag: DW_TAG_typedef, name: "qsize_t", file: !348, line: 66, baseType: !63)
!3324 = !DIDerivedType(tag: DW_TAG_member, name: "dqb_bsoftlimit", scope: !3320, file: !348, line: 207, baseType: !3323, size: 64, offset: 64)
!3325 = !DIDerivedType(tag: DW_TAG_member, name: "dqb_curspace", scope: !3320, file: !348, line: 208, baseType: !3323, size: 64, offset: 128)
!3326 = !DIDerivedType(tag: DW_TAG_member, name: "dqb_rsvspace", scope: !3320, file: !348, line: 209, baseType: !3323, size: 64, offset: 192)
!3327 = !DIDerivedType(tag: DW_TAG_member, name: "dqb_ihardlimit", scope: !3320, file: !348, line: 210, baseType: !3323, size: 64, offset: 256)
!3328 = !DIDerivedType(tag: DW_TAG_member, name: "dqb_isoftlimit", scope: !3320, file: !348, line: 211, baseType: !3323, size: 64, offset: 320)
!3329 = !DIDerivedType(tag: DW_TAG_member, name: "dqb_curinodes", scope: !3320, file: !348, line: 212, baseType: !3323, size: 64, offset: 384)
!3330 = !DIDerivedType(tag: DW_TAG_member, name: "dqb_btime", scope: !3320, file: !348, line: 213, baseType: !569, size: 64, offset: 448)
!3331 = !DIDerivedType(tag: DW_TAG_member, name: "dqb_itime", scope: !3320, file: !348, line: 214, baseType: !569, size: 64, offset: 512)
!3332 = !DIDerivedType(tag: DW_TAG_member, name: "nr_cached_objects", scope: !3220, file: !342, line: 2246, baseType: !3333, size: 64, offset: 1472)
!3333 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3334, size: 64)
!3334 = !DISubroutineType(types: !3335)
!3335 = !{!892, !3172, !3336}
!3336 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3337, size: 64)
!3337 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "shrink_control", file: !3338, line: 34, size: 256, elements: !3339)
!3338 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/shrinker.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "70d14e58095faf53e3e8bf1729fa8967")
!3339 = !{!3340, !3341, !3342, !3343, !3344}
!3340 = !DIDerivedType(tag: DW_TAG_member, name: "gfp_mask", scope: !3337, file: !3338, line: 35, baseType: !488, size: 32)
!3341 = !DIDerivedType(tag: DW_TAG_member, name: "nid", scope: !3337, file: !3338, line: 38, baseType: !42, size: 32, offset: 32)
!3342 = !DIDerivedType(tag: DW_TAG_member, name: "nr_to_scan", scope: !3337, file: !3338, line: 45, baseType: !59, size: 64, offset: 64)
!3343 = !DIDerivedType(tag: DW_TAG_member, name: "nr_scanned", scope: !3337, file: !3338, line: 52, baseType: !59, size: 64, offset: 128)
!3344 = !DIDerivedType(tag: DW_TAG_member, name: "memcg", scope: !3337, file: !3338, line: 55, baseType: !3345, size: 64, offset: 192)
!3345 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3346, size: 64)
!3346 = !DICompositeType(tag: DW_TAG_structure_type, name: "mem_cgroup", file: !230, line: 33, flags: DIFlagFwdDecl)
!3347 = !DIDerivedType(tag: DW_TAG_member, name: "free_cached_objects", scope: !3220, file: !342, line: 2248, baseType: !3333, size: 64, offset: 1536)
!3348 = !DIDerivedType(tag: DW_TAG_member, name: "shutdown", scope: !3220, file: !342, line: 2250, baseType: !3201, size: 64, offset: 1600)
!3349 = !DIDerivedType(tag: DW_TAG_member, name: "dq_op", scope: !3173, file: !342, line: 1261, baseType: !3350, size: 64, offset: 448)
!3350 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3351, size: 64)
!3351 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !3352)
!3352 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "dquot_operations", file: !348, line: 324, size: 704, elements: !3353)
!3353 = !{!3354, !3358, !3362, !3366, !3367, !3368, !3369, !3370, !3375, !3380, !3384}
!3354 = !DIDerivedType(tag: DW_TAG_member, name: "write_dquot", scope: !3352, file: !348, line: 325, baseType: !3355, size: 64)
!3355 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3356, size: 64)
!3356 = !DISubroutineType(types: !3357)
!3357 = !{!42, !3290}
!3358 = !DIDerivedType(tag: DW_TAG_member, name: "alloc_dquot", scope: !3352, file: !348, line: 326, baseType: !3359, size: 64, offset: 64)
!3359 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3360, size: 64)
!3360 = !DISubroutineType(types: !3361)
!3361 = !{!3290, !3172, !42}
!3362 = !DIDerivedType(tag: DW_TAG_member, name: "destroy_dquot", scope: !3352, file: !348, line: 327, baseType: !3363, size: 64, offset: 128)
!3363 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3364, size: 64)
!3364 = !DISubroutineType(types: !3365)
!3365 = !{null, !3290}
!3366 = !DIDerivedType(tag: DW_TAG_member, name: "acquire_dquot", scope: !3352, file: !348, line: 328, baseType: !3355, size: 64, offset: 192)
!3367 = !DIDerivedType(tag: DW_TAG_member, name: "release_dquot", scope: !3352, file: !348, line: 329, baseType: !3355, size: 64, offset: 256)
!3368 = !DIDerivedType(tag: DW_TAG_member, name: "mark_dirty", scope: !3352, file: !348, line: 330, baseType: !3355, size: 64, offset: 320)
!3369 = !DIDerivedType(tag: DW_TAG_member, name: "write_info", scope: !3352, file: !348, line: 331, baseType: !3246, size: 64, offset: 384)
!3370 = !DIDerivedType(tag: DW_TAG_member, name: "get_reserved_space", scope: !3352, file: !348, line: 334, baseType: !3371, size: 64, offset: 448)
!3371 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3372, size: 64)
!3372 = !DISubroutineType(types: !3373)
!3373 = !{!3374, !779}
!3374 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3323, size: 64)
!3375 = !DIDerivedType(tag: DW_TAG_member, name: "get_projid", scope: !3352, file: !348, line: 335, baseType: !3376, size: 64, offset: 512)
!3376 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3377, size: 64)
!3377 = !DISubroutineType(types: !3378)
!3378 = !{!42, !779, !3379}
!3379 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3310, size: 64)
!3380 = !DIDerivedType(tag: DW_TAG_member, name: "get_inode_usage", scope: !3352, file: !348, line: 337, baseType: !3381, size: 64, offset: 576)
!3381 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3382, size: 64)
!3382 = !DISubroutineType(types: !3383)
!3383 = !{!42, !779, !3374}
!3384 = !DIDerivedType(tag: DW_TAG_member, name: "get_next_id", scope: !3352, file: !348, line: 339, baseType: !3385, size: 64, offset: 640)
!3385 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3386, size: 64)
!3386 = !DISubroutineType(types: !3387)
!3387 = !{!42, !3172, !3388}
!3388 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3302, size: 64)
!3389 = !DIDerivedType(tag: DW_TAG_member, name: "s_qcop", scope: !3173, file: !342, line: 1262, baseType: !3390, size: 64, offset: 512)
!3390 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3391, size: 64)
!3391 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !3392)
!3392 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "quotactl_ops", file: !348, line: 430, size: 704, elements: !3393)
!3393 = !{!3394, !3400, !3401, !3405, !3406, !3407, !3422, !3445, !3449, !3450, !3473}
!3394 = !DIDerivedType(tag: DW_TAG_member, name: "quota_on", scope: !3392, file: !348, line: 431, baseType: !3395, size: 64)
!3395 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3396, size: 64)
!3396 = !DISubroutineType(types: !3397)
!3397 = !{!42, !3172, !42, !42, !3398}
!3398 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3399, size: 64)
!3399 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !3162)
!3400 = !DIDerivedType(tag: DW_TAG_member, name: "quota_off", scope: !3392, file: !348, line: 432, baseType: !3246, size: 64, offset: 64)
!3401 = !DIDerivedType(tag: DW_TAG_member, name: "quota_enable", scope: !3392, file: !348, line: 433, baseType: !3402, size: 64, offset: 128)
!3402 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3403, size: 64)
!3403 = !DISubroutineType(types: !3404)
!3404 = !{!42, !3172, !7}
!3405 = !DIDerivedType(tag: DW_TAG_member, name: "quota_disable", scope: !3392, file: !348, line: 434, baseType: !3402, size: 64, offset: 192)
!3406 = !DIDerivedType(tag: DW_TAG_member, name: "quota_sync", scope: !3392, file: !348, line: 435, baseType: !3246, size: 64, offset: 256)
!3407 = !DIDerivedType(tag: DW_TAG_member, name: "set_info", scope: !3392, file: !348, line: 436, baseType: !3408, size: 64, offset: 320)
!3408 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3409, size: 64)
!3409 = !DISubroutineType(types: !3410)
!3410 = !{!42, !3172, !42, !3411}
!3411 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3412, size: 64)
!3412 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "qc_info", file: !348, line: 417, size: 256, elements: !3413)
!3413 = !{!3414, !3415, !3416, !3417, !3418, !3419, !3420, !3421}
!3414 = !DIDerivedType(tag: DW_TAG_member, name: "i_fieldmask", scope: !3412, file: !348, line: 418, baseType: !42, size: 32)
!3415 = !DIDerivedType(tag: DW_TAG_member, name: "i_flags", scope: !3412, file: !348, line: 419, baseType: !7, size: 32, offset: 32)
!3416 = !DIDerivedType(tag: DW_TAG_member, name: "i_spc_timelimit", scope: !3412, file: !348, line: 420, baseType: !7, size: 32, offset: 64)
!3417 = !DIDerivedType(tag: DW_TAG_member, name: "i_ino_timelimit", scope: !3412, file: !348, line: 422, baseType: !7, size: 32, offset: 96)
!3418 = !DIDerivedType(tag: DW_TAG_member, name: "i_rt_spc_timelimit", scope: !3412, file: !348, line: 423, baseType: !7, size: 32, offset: 128)
!3419 = !DIDerivedType(tag: DW_TAG_member, name: "i_spc_warnlimit", scope: !3412, file: !348, line: 424, baseType: !7, size: 32, offset: 160)
!3420 = !DIDerivedType(tag: DW_TAG_member, name: "i_ino_warnlimit", scope: !3412, file: !348, line: 425, baseType: !7, size: 32, offset: 192)
!3421 = !DIDerivedType(tag: DW_TAG_member, name: "i_rt_spc_warnlimit", scope: !3412, file: !348, line: 426, baseType: !7, size: 32, offset: 224)
!3422 = !DIDerivedType(tag: DW_TAG_member, name: "get_dqblk", scope: !3392, file: !348, line: 437, baseType: !3423, size: 64, offset: 384)
!3423 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3424, size: 64)
!3424 = !DISubroutineType(types: !3425)
!3425 = !{!42, !3172, !3302, !3426}
!3426 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3427, size: 64)
!3427 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "qc_dqblk", file: !348, line: 345, size: 960, elements: !3428)
!3428 = !{!3429, !3430, !3431, !3432, !3433, !3434, !3435, !3436, !3437, !3438, !3439, !3440, !3441, !3442, !3443, !3444}
!3429 = !DIDerivedType(tag: DW_TAG_member, name: "d_fieldmask", scope: !3427, file: !348, line: 346, baseType: !42, size: 32)
!3430 = !DIDerivedType(tag: DW_TAG_member, name: "d_spc_hardlimit", scope: !3427, file: !348, line: 347, baseType: !519, size: 64, offset: 64)
!3431 = !DIDerivedType(tag: DW_TAG_member, name: "d_spc_softlimit", scope: !3427, file: !348, line: 348, baseType: !519, size: 64, offset: 128)
!3432 = !DIDerivedType(tag: DW_TAG_member, name: "d_ino_hardlimit", scope: !3427, file: !348, line: 349, baseType: !519, size: 64, offset: 192)
!3433 = !DIDerivedType(tag: DW_TAG_member, name: "d_ino_softlimit", scope: !3427, file: !348, line: 350, baseType: !519, size: 64, offset: 256)
!3434 = !DIDerivedType(tag: DW_TAG_member, name: "d_space", scope: !3427, file: !348, line: 351, baseType: !519, size: 64, offset: 320)
!3435 = !DIDerivedType(tag: DW_TAG_member, name: "d_ino_count", scope: !3427, file: !348, line: 352, baseType: !519, size: 64, offset: 384)
!3436 = !DIDerivedType(tag: DW_TAG_member, name: "d_ino_timer", scope: !3427, file: !348, line: 353, baseType: !502, size: 64, offset: 448)
!3437 = !DIDerivedType(tag: DW_TAG_member, name: "d_spc_timer", scope: !3427, file: !348, line: 355, baseType: !502, size: 64, offset: 512)
!3438 = !DIDerivedType(tag: DW_TAG_member, name: "d_ino_warns", scope: !3427, file: !348, line: 356, baseType: !42, size: 32, offset: 576)
!3439 = !DIDerivedType(tag: DW_TAG_member, name: "d_spc_warns", scope: !3427, file: !348, line: 357, baseType: !42, size: 32, offset: 608)
!3440 = !DIDerivedType(tag: DW_TAG_member, name: "d_rt_spc_hardlimit", scope: !3427, file: !348, line: 358, baseType: !519, size: 64, offset: 640)
!3441 = !DIDerivedType(tag: DW_TAG_member, name: "d_rt_spc_softlimit", scope: !3427, file: !348, line: 359, baseType: !519, size: 64, offset: 704)
!3442 = !DIDerivedType(tag: DW_TAG_member, name: "d_rt_space", scope: !3427, file: !348, line: 360, baseType: !519, size: 64, offset: 768)
!3443 = !DIDerivedType(tag: DW_TAG_member, name: "d_rt_spc_timer", scope: !3427, file: !348, line: 361, baseType: !502, size: 64, offset: 832)
!3444 = !DIDerivedType(tag: DW_TAG_member, name: "d_rt_spc_warns", scope: !3427, file: !348, line: 362, baseType: !42, size: 32, offset: 896)
!3445 = !DIDerivedType(tag: DW_TAG_member, name: "get_nextdqblk", scope: !3392, file: !348, line: 438, baseType: !3446, size: 64, offset: 448)
!3446 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3447, size: 64)
!3447 = !DISubroutineType(types: !3448)
!3448 = !{!42, !3172, !3388, !3426}
!3449 = !DIDerivedType(tag: DW_TAG_member, name: "set_dqblk", scope: !3392, file: !348, line: 440, baseType: !3423, size: 64, offset: 512)
!3450 = !DIDerivedType(tag: DW_TAG_member, name: "get_state", scope: !3392, file: !348, line: 441, baseType: !3451, size: 64, offset: 576)
!3451 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3452, size: 64)
!3452 = !DISubroutineType(types: !3453)
!3453 = !{!42, !3172, !3454}
!3454 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3455, size: 64)
!3455 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "qc_state", file: !348, line: 411, size: 1408, elements: !3456)
!3456 = !{!3457, !3458}
!3457 = !DIDerivedType(tag: DW_TAG_member, name: "s_incoredqs", scope: !3455, file: !348, line: 412, baseType: !7, size: 32)
!3458 = !DIDerivedType(tag: DW_TAG_member, name: "s_state", scope: !3455, file: !348, line: 413, baseType: !3459, size: 1344, offset: 64)
!3459 = !DICompositeType(tag: DW_TAG_array_type, baseType: !3460, size: 1344, elements: !962)
!3460 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "qc_type_state", file: !348, line: 397, size: 448, elements: !3461)
!3461 = !{!3462, !3463, !3464, !3465, !3466, !3467, !3468, !3469, !3470, !3472}
!3462 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !3460, file: !348, line: 398, baseType: !7, size: 32)
!3463 = !DIDerivedType(tag: DW_TAG_member, name: "spc_timelimit", scope: !3460, file: !348, line: 399, baseType: !7, size: 32, offset: 32)
!3464 = !DIDerivedType(tag: DW_TAG_member, name: "ino_timelimit", scope: !3460, file: !348, line: 401, baseType: !7, size: 32, offset: 64)
!3465 = !DIDerivedType(tag: DW_TAG_member, name: "rt_spc_timelimit", scope: !3460, file: !348, line: 402, baseType: !7, size: 32, offset: 96)
!3466 = !DIDerivedType(tag: DW_TAG_member, name: "spc_warnlimit", scope: !3460, file: !348, line: 403, baseType: !7, size: 32, offset: 128)
!3467 = !DIDerivedType(tag: DW_TAG_member, name: "ino_warnlimit", scope: !3460, file: !348, line: 404, baseType: !7, size: 32, offset: 160)
!3468 = !DIDerivedType(tag: DW_TAG_member, name: "rt_spc_warnlimit", scope: !3460, file: !348, line: 405, baseType: !7, size: 32, offset: 192)
!3469 = !DIDerivedType(tag: DW_TAG_member, name: "ino", scope: !3460, file: !348, line: 406, baseType: !521, size: 64, offset: 256)
!3470 = !DIDerivedType(tag: DW_TAG_member, name: "blocks", scope: !3460, file: !348, line: 407, baseType: !3471, size: 64, offset: 320)
!3471 = !DIDerivedType(tag: DW_TAG_typedef, name: "blkcnt_t", file: !45, line: 135, baseType: !519)
!3472 = !DIDerivedType(tag: DW_TAG_member, name: "nextents", scope: !3460, file: !348, line: 408, baseType: !3471, size: 64, offset: 384)
!3473 = !DIDerivedType(tag: DW_TAG_member, name: "rm_xquota", scope: !3392, file: !348, line: 442, baseType: !3402, size: 64, offset: 640)
!3474 = !DIDerivedType(tag: DW_TAG_member, name: "s_export_op", scope: !3173, file: !342, line: 1263, baseType: !3475, size: 64, offset: 576)
!3475 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3476, size: 64)
!3476 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !3477)
!3477 = !DICompositeType(tag: DW_TAG_structure_type, name: "export_operations", file: !342, line: 56, flags: DIFlagFwdDecl)
!3478 = !DIDerivedType(tag: DW_TAG_member, name: "s_flags", scope: !3173, file: !342, line: 1264, baseType: !59, size: 64, offset: 640)
!3479 = !DIDerivedType(tag: DW_TAG_member, name: "s_iflags", scope: !3173, file: !342, line: 1265, baseType: !59, size: 64, offset: 704)
!3480 = !DIDerivedType(tag: DW_TAG_member, name: "s_magic", scope: !3173, file: !342, line: 1266, baseType: !59, size: 64, offset: 768)
!3481 = !DIDerivedType(tag: DW_TAG_member, name: "s_root", scope: !3173, file: !342, line: 1267, baseType: !740, size: 64, offset: 832)
!3482 = !DIDerivedType(tag: DW_TAG_member, name: "s_umount", scope: !3173, file: !342, line: 1268, baseType: !549, size: 320, offset: 896)
!3483 = !DIDerivedType(tag: DW_TAG_member, name: "s_count", scope: !3173, file: !342, line: 1269, baseType: !42, size: 32, offset: 1216)
!3484 = !DIDerivedType(tag: DW_TAG_member, name: "s_active", scope: !3173, file: !342, line: 1270, baseType: !69, size: 32, offset: 1248)
!3485 = !DIDerivedType(tag: DW_TAG_member, name: "s_security", scope: !3173, file: !342, line: 1272, baseType: !40, size: 64, offset: 1280)
!3486 = !DIDerivedType(tag: DW_TAG_member, name: "s_xattr", scope: !3173, file: !342, line: 1274, baseType: !3487, size: 64, offset: 1344)
!3487 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3488, size: 64)
!3488 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !3489)
!3489 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3490, size: 64)
!3490 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !3491)
!3491 = !DICompositeType(tag: DW_TAG_structure_type, name: "xattr_handler", file: !342, line: 1274, flags: DIFlagFwdDecl)
!3492 = !DIDerivedType(tag: DW_TAG_member, name: "s_roots", scope: !3173, file: !342, line: 1286, baseType: !3493, size: 64, offset: 1408)
!3493 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "hlist_bl_head", file: !756, line: 34, size: 64, elements: !3494)
!3494 = !{!3495}
!3495 = !DIDerivedType(tag: DW_TAG_member, name: "first", scope: !3493, file: !756, line: 35, baseType: !759, size: 64)
!3496 = !DIDerivedType(tag: DW_TAG_member, name: "s_mounts", scope: !3173, file: !342, line: 1287, baseType: !117, size: 128, offset: 1472)
!3497 = !DIDerivedType(tag: DW_TAG_member, name: "s_bdev", scope: !3173, file: !342, line: 1288, baseType: !1859, size: 64, offset: 1600)
!3498 = !DIDerivedType(tag: DW_TAG_member, name: "s_bdev_file", scope: !3173, file: !342, line: 1289, baseType: !896, size: 64, offset: 1664)
!3499 = !DIDerivedType(tag: DW_TAG_member, name: "s_bdi", scope: !3173, file: !342, line: 1290, baseType: !3500, size: 64, offset: 1728)
!3500 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3501, size: 64)
!3501 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "backing_dev_info", file: !354, line: 163, size: 6848, elements: !3502)
!3502 = !{!3503, !3504, !3505, !3506, !3507, !3508, !3509, !3510, !3511, !3512, !3513, !3514, !3515, !3549, !3550, !3551, !3552, !3554, !3555, !3556}
!3503 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !3501, file: !354, line: 164, baseType: !519, size: 64)
!3504 = !DIDerivedType(tag: DW_TAG_member, name: "rb_node", scope: !3501, file: !354, line: 165, baseType: !173, size: 192, align: 64, offset: 64)
!3505 = !DIDerivedType(tag: DW_TAG_member, name: "bdi_list", scope: !3501, file: !354, line: 166, baseType: !117, size: 128, offset: 256)
!3506 = !DIDerivedType(tag: DW_TAG_member, name: "ra_pages", scope: !3501, file: !354, line: 167, baseType: !59, size: 64, offset: 384)
!3507 = !DIDerivedType(tag: DW_TAG_member, name: "io_pages", scope: !3501, file: !354, line: 168, baseType: !59, size: 64, offset: 448)
!3508 = !DIDerivedType(tag: DW_TAG_member, name: "refcnt", scope: !3501, file: !354, line: 170, baseType: !2545, size: 32, offset: 512)
!3509 = !DIDerivedType(tag: DW_TAG_member, name: "capabilities", scope: !3501, file: !354, line: 171, baseType: !7, size: 32, offset: 544)
!3510 = !DIDerivedType(tag: DW_TAG_member, name: "min_ratio", scope: !3501, file: !354, line: 172, baseType: !7, size: 32, offset: 576)
!3511 = !DIDerivedType(tag: DW_TAG_member, name: "max_ratio", scope: !3501, file: !354, line: 173, baseType: !7, size: 32, offset: 608)
!3512 = !DIDerivedType(tag: DW_TAG_member, name: "max_prop_frac", scope: !3501, file: !354, line: 173, baseType: !7, size: 32, offset: 640)
!3513 = !DIDerivedType(tag: DW_TAG_member, name: "tot_write_bandwidth", scope: !3501, file: !354, line: 179, baseType: !496, size: 64, offset: 704)
!3514 = !DIDerivedType(tag: DW_TAG_member, name: "last_bdp_sleep", scope: !3501, file: !354, line: 184, baseType: !59, size: 64, offset: 768)
!3515 = !DIDerivedType(tag: DW_TAG_member, name: "wb", scope: !3501, file: !354, line: 186, baseType: !3516, size: 4672, offset: 832)
!3516 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bdi_writeback", file: !354, line: 105, size: 4672, elements: !3517)
!3517 = !{!3518, !3519, !3520, !3521, !3522, !3523, !3524, !3525, !3526, !3527, !3528, !3529, !3530, !3531, !3532, !3533, !3534, !3535, !3542, !3543, !3544, !3545, !3546, !3547, !3548}
!3518 = !DIDerivedType(tag: DW_TAG_member, name: "bdi", scope: !3516, file: !354, line: 106, baseType: !3500, size: 64)
!3519 = !DIDerivedType(tag: DW_TAG_member, name: "state", scope: !3516, file: !354, line: 108, baseType: !59, size: 64, offset: 64)
!3520 = !DIDerivedType(tag: DW_TAG_member, name: "last_old_flush", scope: !3516, file: !354, line: 109, baseType: !59, size: 64, offset: 128)
!3521 = !DIDerivedType(tag: DW_TAG_member, name: "b_dirty", scope: !3516, file: !354, line: 111, baseType: !117, size: 128, offset: 192)
!3522 = !DIDerivedType(tag: DW_TAG_member, name: "b_io", scope: !3516, file: !354, line: 112, baseType: !117, size: 128, offset: 320)
!3523 = !DIDerivedType(tag: DW_TAG_member, name: "b_more_io", scope: !3516, file: !354, line: 113, baseType: !117, size: 128, offset: 448)
!3524 = !DIDerivedType(tag: DW_TAG_member, name: "b_dirty_time", scope: !3516, file: !354, line: 114, baseType: !117, size: 128, offset: 576)
!3525 = !DIDerivedType(tag: DW_TAG_member, name: "list_lock", scope: !3516, file: !354, line: 115, baseType: !79, size: 32, offset: 704)
!3526 = !DIDerivedType(tag: DW_TAG_member, name: "writeback_inodes", scope: !3516, file: !354, line: 117, baseType: !69, size: 32, offset: 736)
!3527 = !DIDerivedType(tag: DW_TAG_member, name: "stat", scope: !3516, file: !354, line: 118, baseType: !1260, size: 1280, offset: 768)
!3528 = !DIDerivedType(tag: DW_TAG_member, name: "bw_time_stamp", scope: !3516, file: !354, line: 120, baseType: !59, size: 64, offset: 2048)
!3529 = !DIDerivedType(tag: DW_TAG_member, name: "dirtied_stamp", scope: !3516, file: !354, line: 121, baseType: !59, size: 64, offset: 2112)
!3530 = !DIDerivedType(tag: DW_TAG_member, name: "written_stamp", scope: !3516, file: !354, line: 122, baseType: !59, size: 64, offset: 2176)
!3531 = !DIDerivedType(tag: DW_TAG_member, name: "write_bandwidth", scope: !3516, file: !354, line: 123, baseType: !59, size: 64, offset: 2240)
!3532 = !DIDerivedType(tag: DW_TAG_member, name: "avg_write_bandwidth", scope: !3516, file: !354, line: 124, baseType: !59, size: 64, offset: 2304)
!3533 = !DIDerivedType(tag: DW_TAG_member, name: "dirty_ratelimit", scope: !3516, file: !354, line: 132, baseType: !59, size: 64, offset: 2368)
!3534 = !DIDerivedType(tag: DW_TAG_member, name: "balanced_dirty_ratelimit", scope: !3516, file: !354, line: 133, baseType: !59, size: 64, offset: 2432)
!3535 = !DIDerivedType(tag: DW_TAG_member, name: "completions", scope: !3516, file: !354, line: 135, baseType: !3536, size: 384, offset: 2496)
!3536 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "fprop_local_percpu", file: !3537, line: 44, size: 384, elements: !3538)
!3537 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/flex_proportions.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "533c3a2d07885ffd17541e6a62e0a11f")
!3538 = !{!3539, !3540, !3541}
!3539 = !DIDerivedType(tag: DW_TAG_member, name: "events", scope: !3536, file: !3537, line: 46, baseType: !675, size: 320)
!3540 = !DIDerivedType(tag: DW_TAG_member, name: "period", scope: !3536, file: !3537, line: 48, baseType: !7, size: 32, offset: 320)
!3541 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !3536, file: !3537, line: 49, baseType: !148, size: 32, offset: 352)
!3542 = !DIDerivedType(tag: DW_TAG_member, name: "dirty_exceeded", scope: !3516, file: !354, line: 136, baseType: !42, size: 32, offset: 2880)
!3543 = !DIDerivedType(tag: DW_TAG_member, name: "start_all_reason", scope: !3516, file: !354, line: 137, baseType: !353, size: 32, offset: 2912)
!3544 = !DIDerivedType(tag: DW_TAG_member, name: "work_lock", scope: !3516, file: !354, line: 139, baseType: !79, size: 32, offset: 2944)
!3545 = !DIDerivedType(tag: DW_TAG_member, name: "work_list", scope: !3516, file: !354, line: 140, baseType: !117, size: 128, offset: 3008)
!3546 = !DIDerivedType(tag: DW_TAG_member, name: "dwork", scope: !3516, file: !354, line: 141, baseType: !2840, size: 704, offset: 3136)
!3547 = !DIDerivedType(tag: DW_TAG_member, name: "bw_dwork", scope: !3516, file: !354, line: 142, baseType: !2840, size: 704, offset: 3840)
!3548 = !DIDerivedType(tag: DW_TAG_member, name: "bdi_node", scope: !3516, file: !354, line: 144, baseType: !117, size: 128, offset: 4544)
!3549 = !DIDerivedType(tag: DW_TAG_member, name: "wb_list", scope: !3501, file: !354, line: 187, baseType: !117, size: 128, offset: 5504)
!3550 = !DIDerivedType(tag: DW_TAG_member, name: "wb_waitq", scope: !3501, file: !354, line: 193, baseType: !74, size: 192, offset: 5632)
!3551 = !DIDerivedType(tag: DW_TAG_member, name: "dev", scope: !3501, file: !354, line: 195, baseType: !1902, size: 64, offset: 5824)
!3552 = !DIDerivedType(tag: DW_TAG_member, name: "dev_name", scope: !3501, file: !354, line: 196, baseType: !3553, size: 512, offset: 5888)
!3553 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 512, elements: !966)
!3554 = !DIDerivedType(tag: DW_TAG_member, name: "owner", scope: !3501, file: !354, line: 197, baseType: !1902, size: 64, offset: 6400)
!3555 = !DIDerivedType(tag: DW_TAG_member, name: "laptop_mode_wb_timer", scope: !3501, file: !354, line: 199, baseType: !2070, size: 320, offset: 6464)
!3556 = !DIDerivedType(tag: DW_TAG_member, name: "debug_dir", scope: !3501, file: !354, line: 202, baseType: !740, size: 64, offset: 6784)
!3557 = !DIDerivedType(tag: DW_TAG_member, name: "s_mtd", scope: !3173, file: !342, line: 1291, baseType: !3558, size: 64, offset: 1792)
!3558 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3559, size: 64)
!3559 = !DICompositeType(tag: DW_TAG_structure_type, name: "mtd_info", file: !342, line: 1291, flags: DIFlagFwdDecl)
!3560 = !DIDerivedType(tag: DW_TAG_member, name: "s_instances", scope: !3173, file: !342, line: 1292, baseType: !220, size: 128, offset: 1856)
!3561 = !DIDerivedType(tag: DW_TAG_member, name: "s_quota_types", scope: !3173, file: !342, line: 1293, baseType: !7, size: 32, offset: 1984)
!3562 = !DIDerivedType(tag: DW_TAG_member, name: "s_dquot", scope: !3173, file: !342, line: 1294, baseType: !3563, size: 2496, offset: 2048)
!3563 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "quota_info", file: !348, line: 521, size: 2496, elements: !3564)
!3564 = !{!3565, !3566, !3567, !3569, !3601}
!3565 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !3563, file: !348, line: 522, baseType: !7, size: 32)
!3566 = !DIDerivedType(tag: DW_TAG_member, name: "dqio_sem", scope: !3563, file: !348, line: 523, baseType: !549, size: 320, offset: 64)
!3567 = !DIDerivedType(tag: DW_TAG_member, name: "files", scope: !3563, file: !348, line: 524, baseType: !3568, size: 192, offset: 384)
!3568 = !DICompositeType(tag: DW_TAG_array_type, baseType: !779, size: 192, elements: !962)
!3569 = !DIDerivedType(tag: DW_TAG_member, name: "info", scope: !3563, file: !348, line: 525, baseType: !3570, size: 1728, offset: 576)
!3570 = !DICompositeType(tag: DW_TAG_array_type, baseType: !3571, size: 1728, elements: !962)
!3571 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mem_dqinfo", file: !348, line: 222, size: 576, elements: !3572)
!3572 = !{!3573, !3593, !3594, !3595, !3596, !3597, !3598, !3599, !3600}
!3573 = !DIDerivedType(tag: DW_TAG_member, name: "dqi_format", scope: !3571, file: !348, line: 223, baseType: !3574, size: 64)
!3574 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3575, size: 64)
!3575 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "quota_format_type", file: !348, line: 445, size: 256, elements: !3576)
!3576 = !{!3577, !3578, !3591, !3592}
!3577 = !DIDerivedType(tag: DW_TAG_member, name: "qf_fmt_id", scope: !3575, file: !348, line: 446, baseType: !42, size: 32)
!3578 = !DIDerivedType(tag: DW_TAG_member, name: "qf_ops", scope: !3575, file: !348, line: 447, baseType: !3579, size: 64, offset: 64)
!3579 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3580, size: 64)
!3580 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !3581)
!3581 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "quota_format_ops", file: !348, line: 312, size: 512, elements: !3582)
!3582 = !{!3583, !3584, !3585, !3586, !3587, !3588, !3589, !3590}
!3583 = !DIDerivedType(tag: DW_TAG_member, name: "check_quota_file", scope: !3581, file: !348, line: 313, baseType: !3246, size: 64)
!3584 = !DIDerivedType(tag: DW_TAG_member, name: "read_file_info", scope: !3581, file: !348, line: 314, baseType: !3246, size: 64, offset: 64)
!3585 = !DIDerivedType(tag: DW_TAG_member, name: "write_file_info", scope: !3581, file: !348, line: 315, baseType: !3246, size: 64, offset: 128)
!3586 = !DIDerivedType(tag: DW_TAG_member, name: "free_file_info", scope: !3581, file: !348, line: 316, baseType: !3246, size: 64, offset: 192)
!3587 = !DIDerivedType(tag: DW_TAG_member, name: "read_dqblk", scope: !3581, file: !348, line: 317, baseType: !3355, size: 64, offset: 256)
!3588 = !DIDerivedType(tag: DW_TAG_member, name: "commit_dqblk", scope: !3581, file: !348, line: 318, baseType: !3355, size: 64, offset: 320)
!3589 = !DIDerivedType(tag: DW_TAG_member, name: "release_dqblk", scope: !3581, file: !348, line: 319, baseType: !3355, size: 64, offset: 384)
!3590 = !DIDerivedType(tag: DW_TAG_member, name: "get_next_id", scope: !3581, file: !348, line: 320, baseType: !3385, size: 64, offset: 448)
!3591 = !DIDerivedType(tag: DW_TAG_member, name: "qf_owner", scope: !3575, file: !348, line: 448, baseType: !908, size: 64, offset: 128)
!3592 = !DIDerivedType(tag: DW_TAG_member, name: "qf_next", scope: !3575, file: !348, line: 449, baseType: !3574, size: 64, offset: 192)
!3593 = !DIDerivedType(tag: DW_TAG_member, name: "dqi_fmt_id", scope: !3571, file: !348, line: 224, baseType: !42, size: 32, offset: 64)
!3594 = !DIDerivedType(tag: DW_TAG_member, name: "dqi_dirty_list", scope: !3571, file: !348, line: 226, baseType: !117, size: 128, offset: 128)
!3595 = !DIDerivedType(tag: DW_TAG_member, name: "dqi_flags", scope: !3571, file: !348, line: 227, baseType: !59, size: 64, offset: 256)
!3596 = !DIDerivedType(tag: DW_TAG_member, name: "dqi_bgrace", scope: !3571, file: !348, line: 228, baseType: !7, size: 32, offset: 320)
!3597 = !DIDerivedType(tag: DW_TAG_member, name: "dqi_igrace", scope: !3571, file: !348, line: 229, baseType: !7, size: 32, offset: 352)
!3598 = !DIDerivedType(tag: DW_TAG_member, name: "dqi_max_spc_limit", scope: !3571, file: !348, line: 230, baseType: !3323, size: 64, offset: 384)
!3599 = !DIDerivedType(tag: DW_TAG_member, name: "dqi_max_ino_limit", scope: !3571, file: !348, line: 231, baseType: !3323, size: 64, offset: 448)
!3600 = !DIDerivedType(tag: DW_TAG_member, name: "dqi_priv", scope: !3571, file: !348, line: 232, baseType: !40, size: 64, offset: 512)
!3601 = !DIDerivedType(tag: DW_TAG_member, name: "ops", scope: !3563, file: !348, line: 526, baseType: !3602, size: 192, offset: 2304)
!3602 = !DICompositeType(tag: DW_TAG_array_type, baseType: !3579, size: 192, elements: !962)
!3603 = !DIDerivedType(tag: DW_TAG_member, name: "s_writers", scope: !3173, file: !342, line: 1296, baseType: !3604, size: 2432, offset: 4544)
!3604 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "sb_writers", file: !342, line: 1246, size: 2432, elements: !3605)
!3605 = !{!3606, !3607, !3608, !3609}
!3606 = !DIDerivedType(tag: DW_TAG_member, name: "frozen", scope: !3604, file: !342, line: 1247, baseType: !46, size: 16)
!3607 = !DIDerivedType(tag: DW_TAG_member, name: "freeze_kcount", scope: !3604, file: !342, line: 1248, baseType: !42, size: 32, offset: 32)
!3608 = !DIDerivedType(tag: DW_TAG_member, name: "freeze_ucount", scope: !3604, file: !342, line: 1249, baseType: !42, size: 32, offset: 64)
!3609 = !DIDerivedType(tag: DW_TAG_member, name: "rw_sem", scope: !3604, file: !342, line: 1250, baseType: !3610, size: 2304, offset: 128)
!3610 = !DICompositeType(tag: DW_TAG_array_type, baseType: !3611, size: 2304, elements: !962)
!3611 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "percpu_rw_semaphore", file: !3612, line: 12, size: 768, elements: !3613)
!3612 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/percpu-rwsem.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "ecddd19ede40c4830088fe5b2fd3d159")
!3613 = !{!3614, !3622, !3623, !5212, !5213}
!3614 = !DIDerivedType(tag: DW_TAG_member, name: "rss", scope: !3611, file: !3612, line: 13, baseType: !3615, size: 384)
!3615 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "rcu_sync", file: !3616, line: 17, size: 384, elements: !3617)
!3616 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/rcu_sync.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "0faca0baf4d2599bdab3652fbd51da1f")
!3617 = !{!3618, !3619, !3620, !3621}
!3618 = !DIDerivedType(tag: DW_TAG_member, name: "gp_state", scope: !3615, file: !3616, line: 18, baseType: !42, size: 32)
!3619 = !DIDerivedType(tag: DW_TAG_member, name: "gp_count", scope: !3615, file: !3616, line: 19, baseType: !42, size: 32, offset: 32)
!3620 = !DIDerivedType(tag: DW_TAG_member, name: "gp_wait", scope: !3615, file: !3616, line: 20, baseType: !74, size: 192, offset: 64)
!3621 = !DIDerivedType(tag: DW_TAG_member, name: "cb_head", scope: !3615, file: !3616, line: 22, baseType: !129, size: 128, align: 64, offset: 256)
!3622 = !DIDerivedType(tag: DW_TAG_member, name: "read_count", scope: !3611, file: !3612, line: 14, baseType: !1851, size: 64, offset: 384)
!3623 = !DIDerivedType(tag: DW_TAG_member, name: "writer", scope: !3611, file: !3612, line: 15, baseType: !3624, size: 64, offset: 448)
!3624 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "rcuwait", file: !3625, line: 16, size: 64, elements: !3626)
!3625 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/rcuwait.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "9d16d79f1814717edcda6c891eddf7a7")
!3626 = !{!3627}
!3627 = !DIDerivedType(tag: DW_TAG_member, name: "task", scope: !3624, file: !3625, line: 17, baseType: !3628, size: 64)
!3628 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3629, size: 64)
!3629 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "task_struct", file: !1435, line: 778, size: 58368, elements: !3630)
!3630 = !{!3631, !3639, !3640, !3641, !3642, !3643, !3644, !3645, !3646, !3663, !3664, !3665, !3666, !3667, !3668, !3669, !3670, !3671, !3672, !3673, !3717, !3728, !3765, !3766, !3770, !3773, !3804, !3805, !3806, !3807, !3808, !3817, !3819, !3820, !3821, !3822, !3823, !3824, !3835, !3836, !3839, !3840, !3841, !3842, !3843, !3844, !3845, !3846, !3853, !3854, !3855, !3856, !3857, !3858, !3859, !3860, !3861, !3862, !3863, !3864, !3865, !3866, !3867, !3868, !3869, !3870, !3871, !3872, !3873, !3874, !3875, !3876, !3877, !3878, !3879, !3880, !3881, !3938, !3939, !3940, !3941, !3942, !3943, !3944, !3945, !3946, !3947, !3948, !4010, !4012, !4013, !4014, !4015, !4016, !4017, !4018, !4019, !4020, !4026, !4027, !4028, !4029, !4030, !4031, !4032, !4044, !4050, !4051, !4052, !4053, !4054, !4058, !4061, !4068, !4073, !4076, !4103, !4106, !4401, !4602, !4629, !4630, !4631, !4632, !4633, !4634, !4635, !4636, !4637, !4640, !4641, !4642, !4651, !4659, !4660, !4661, !4662, !4663, !4668, !4669, !4670, !4673, !4674, !4677, !4680, !4685, !4692, !4695, !4696, !4782, !4783, !4784, !4785, !4786, !4792, !4793, !4794, !4795, !4796, !4799, !4813, !4814, !4817, !4818, !4819, !4821, !4824, !4825, !4826, !4827, !4828, !4829, !4830, !4843, !4844, !4845, !4846, !4847, !4848, !4849, !4850, !4851, !4862, !4863, !4869, !4872, !4873, !4874, !4875, !4876, !4877, !4878, !4879, !4951, !4953, !4954, !4955, !4956, !4957, !4958, !4974, !4975, !4976, !4979, !4980, !4981, !4982, !4983, !4984, !4985, !4986, !4987, !4991, !4992, !4993}
!3631 = !DIDerivedType(tag: DW_TAG_member, name: "thread_info", scope: !3629, file: !1435, line: 784, baseType: !3632, size: 192)
!3632 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "thread_info", file: !3633, line: 62, size: 192, elements: !3634)
!3633 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/thread_info.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "7db5f9e8078dfb9855aa0f01ed8a7a03")
!3634 = !{!3635, !3636, !3637, !3638}
!3635 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !3632, file: !3633, line: 63, baseType: !59, size: 64)
!3636 = !DIDerivedType(tag: DW_TAG_member, name: "syscall_work", scope: !3632, file: !3633, line: 64, baseType: !59, size: 64, offset: 64)
!3637 = !DIDerivedType(tag: DW_TAG_member, name: "status", scope: !3632, file: !3633, line: 65, baseType: !578, size: 32, offset: 128)
!3638 = !DIDerivedType(tag: DW_TAG_member, name: "cpu", scope: !3632, file: !3633, line: 67, baseType: !578, size: 32, offset: 160)
!3639 = !DIDerivedType(tag: DW_TAG_member, name: "__state", scope: !3629, file: !1435, line: 786, baseType: !7, size: 32, offset: 192)
!3640 = !DIDerivedType(tag: DW_TAG_member, name: "saved_state", scope: !3629, file: !1435, line: 789, baseType: !7, size: 32, offset: 224)
!3641 = !DIDerivedType(tag: DW_TAG_member, name: "stack", scope: !3629, file: !1435, line: 797, baseType: !40, size: 64, offset: 256)
!3642 = !DIDerivedType(tag: DW_TAG_member, name: "usage", scope: !3629, file: !1435, line: 798, baseType: !533, size: 32, offset: 320)
!3643 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !3629, file: !1435, line: 800, baseType: !7, size: 32, offset: 352)
!3644 = !DIDerivedType(tag: DW_TAG_member, name: "ptrace", scope: !3629, file: !1435, line: 801, baseType: !7, size: 32, offset: 384)
!3645 = !DIDerivedType(tag: DW_TAG_member, name: "on_cpu", scope: !3629, file: !1435, line: 808, baseType: !42, size: 32, offset: 416)
!3646 = !DIDerivedType(tag: DW_TAG_member, name: "wake_entry", scope: !3629, file: !1435, line: 809, baseType: !3647, size: 128, offset: 448)
!3647 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__call_single_node", file: !3648, line: 58, size: 128, elements: !3649)
!3648 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/smp_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "5419d178c26889551dcae04751edcc21")
!3649 = !{!3650, !3656, !3661, !3662}
!3650 = !DIDerivedType(tag: DW_TAG_member, name: "llist", scope: !3647, file: !3648, line: 59, baseType: !3651, size: 64)
!3651 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "llist_node", file: !3652, line: 60, size: 64, elements: !3653)
!3652 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/llist.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "d23d40c14174148f540e8d27c8603f36")
!3653 = !{!3654}
!3654 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !3651, file: !3652, line: 61, baseType: !3655, size: 64)
!3655 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3651, size: 64)
!3656 = !DIDerivedType(tag: DW_TAG_member, scope: !3647, file: !3648, line: 60, baseType: !3657, size: 32, offset: 64)
!3657 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !3647, file: !3648, line: 60, size: 32, elements: !3658)
!3658 = !{!3659, !3660}
!3659 = !DIDerivedType(tag: DW_TAG_member, name: "u_flags", scope: !3657, file: !3648, line: 61, baseType: !7, size: 32)
!3660 = !DIDerivedType(tag: DW_TAG_member, name: "a_flags", scope: !3657, file: !3648, line: 62, baseType: !69, size: 32)
!3661 = !DIDerivedType(tag: DW_TAG_member, name: "src", scope: !3647, file: !3648, line: 65, baseType: !113, size: 16, offset: 96)
!3662 = !DIDerivedType(tag: DW_TAG_member, name: "dst", scope: !3647, file: !3648, line: 65, baseType: !113, size: 16, offset: 112)
!3663 = !DIDerivedType(tag: DW_TAG_member, name: "wakee_flips", scope: !3629, file: !1435, line: 810, baseType: !7, size: 32, offset: 576)
!3664 = !DIDerivedType(tag: DW_TAG_member, name: "wakee_flip_decay_ts", scope: !3629, file: !1435, line: 811, baseType: !59, size: 64, offset: 640)
!3665 = !DIDerivedType(tag: DW_TAG_member, name: "last_wakee", scope: !3629, file: !1435, line: 812, baseType: !3628, size: 64, offset: 704)
!3666 = !DIDerivedType(tag: DW_TAG_member, name: "recent_used_cpu", scope: !3629, file: !1435, line: 821, baseType: !42, size: 32, offset: 768)
!3667 = !DIDerivedType(tag: DW_TAG_member, name: "wake_cpu", scope: !3629, file: !1435, line: 822, baseType: !42, size: 32, offset: 800)
!3668 = !DIDerivedType(tag: DW_TAG_member, name: "on_rq", scope: !3629, file: !1435, line: 824, baseType: !42, size: 32, offset: 832)
!3669 = !DIDerivedType(tag: DW_TAG_member, name: "prio", scope: !3629, file: !1435, line: 826, baseType: !42, size: 32, offset: 864)
!3670 = !DIDerivedType(tag: DW_TAG_member, name: "static_prio", scope: !3629, file: !1435, line: 827, baseType: !42, size: 32, offset: 896)
!3671 = !DIDerivedType(tag: DW_TAG_member, name: "normal_prio", scope: !3629, file: !1435, line: 828, baseType: !42, size: 32, offset: 928)
!3672 = !DIDerivedType(tag: DW_TAG_member, name: "rt_priority", scope: !3629, file: !1435, line: 829, baseType: !7, size: 32, offset: 960)
!3673 = !DIDerivedType(tag: DW_TAG_member, name: "se", scope: !3629, file: !1435, line: 831, baseType: !3674, size: 2048, offset: 1024)
!3674 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "sched_entity", file: !1435, line: 541, size: 2048, elements: !3675)
!3675 = !{!3676, !3681, !3682, !3683, !3684, !3685, !3686, !3687, !3688, !3689, !3690, !3691, !3692, !3693, !3694, !3695, !3696, !3697, !3698, !3700, !3703, !3704, !3705}
!3676 = !DIDerivedType(tag: DW_TAG_member, name: "load", scope: !3674, file: !1435, line: 543, baseType: !3677, size: 128)
!3677 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "load_weight", file: !1435, line: 426, size: 128, elements: !3678)
!3678 = !{!3679, !3680}
!3679 = !DIDerivedType(tag: DW_TAG_member, name: "weight", scope: !3677, file: !1435, line: 427, baseType: !59, size: 64)
!3680 = !DIDerivedType(tag: DW_TAG_member, name: "inv_weight", scope: !3677, file: !1435, line: 428, baseType: !578, size: 32, offset: 64)
!3681 = !DIDerivedType(tag: DW_TAG_member, name: "run_node", scope: !3674, file: !1435, line: 544, baseType: !173, size: 192, align: 64, offset: 128)
!3682 = !DIDerivedType(tag: DW_TAG_member, name: "deadline", scope: !3674, file: !1435, line: 545, baseType: !519, size: 64, offset: 320)
!3683 = !DIDerivedType(tag: DW_TAG_member, name: "min_vruntime", scope: !3674, file: !1435, line: 546, baseType: !519, size: 64, offset: 384)
!3684 = !DIDerivedType(tag: DW_TAG_member, name: "min_slice", scope: !3674, file: !1435, line: 547, baseType: !519, size: 64, offset: 448)
!3685 = !DIDerivedType(tag: DW_TAG_member, name: "group_node", scope: !3674, file: !1435, line: 549, baseType: !117, size: 128, offset: 512)
!3686 = !DIDerivedType(tag: DW_TAG_member, name: "on_rq", scope: !3674, file: !1435, line: 550, baseType: !107, size: 8, offset: 640)
!3687 = !DIDerivedType(tag: DW_TAG_member, name: "sched_delayed", scope: !3674, file: !1435, line: 551, baseType: !107, size: 8, offset: 648)
!3688 = !DIDerivedType(tag: DW_TAG_member, name: "rel_deadline", scope: !3674, file: !1435, line: 552, baseType: !107, size: 8, offset: 656)
!3689 = !DIDerivedType(tag: DW_TAG_member, name: "custom_slice", scope: !3674, file: !1435, line: 553, baseType: !107, size: 8, offset: 664)
!3690 = !DIDerivedType(tag: DW_TAG_member, name: "exec_start", scope: !3674, file: !1435, line: 556, baseType: !519, size: 64, offset: 704)
!3691 = !DIDerivedType(tag: DW_TAG_member, name: "sum_exec_runtime", scope: !3674, file: !1435, line: 557, baseType: !519, size: 64, offset: 768)
!3692 = !DIDerivedType(tag: DW_TAG_member, name: "prev_sum_exec_runtime", scope: !3674, file: !1435, line: 558, baseType: !519, size: 64, offset: 832)
!3693 = !DIDerivedType(tag: DW_TAG_member, name: "vruntime", scope: !3674, file: !1435, line: 559, baseType: !519, size: 64, offset: 896)
!3694 = !DIDerivedType(tag: DW_TAG_member, name: "vlag", scope: !3674, file: !1435, line: 560, baseType: !502, size: 64, offset: 960)
!3695 = !DIDerivedType(tag: DW_TAG_member, name: "slice", scope: !3674, file: !1435, line: 561, baseType: !519, size: 64, offset: 1024)
!3696 = !DIDerivedType(tag: DW_TAG_member, name: "nr_migrations", scope: !3674, file: !1435, line: 563, baseType: !519, size: 64, offset: 1088)
!3697 = !DIDerivedType(tag: DW_TAG_member, name: "depth", scope: !3674, file: !1435, line: 566, baseType: !42, size: 32, offset: 1152)
!3698 = !DIDerivedType(tag: DW_TAG_member, name: "parent", scope: !3674, file: !1435, line: 567, baseType: !3699, size: 64, offset: 1216)
!3699 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3674, size: 64)
!3700 = !DIDerivedType(tag: DW_TAG_member, name: "cfs_rq", scope: !3674, file: !1435, line: 569, baseType: !3701, size: 64, offset: 1280)
!3701 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3702, size: 64)
!3702 = !DICompositeType(tag: DW_TAG_structure_type, name: "cfs_rq", file: !1435, line: 59, flags: DIFlagFwdDecl)
!3703 = !DIDerivedType(tag: DW_TAG_member, name: "my_q", scope: !3674, file: !1435, line: 571, baseType: !3701, size: 64, offset: 1344)
!3704 = !DIDerivedType(tag: DW_TAG_member, name: "runnable_weight", scope: !3674, file: !1435, line: 573, baseType: !59, size: 64, offset: 1408)
!3705 = !DIDerivedType(tag: DW_TAG_member, name: "avg", scope: !3674, file: !1435, line: 583, baseType: !3706, size: 512, align: 512, offset: 1536)
!3706 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "sched_avg", file: !1435, line: 476, size: 512, align: 512, elements: !3707)
!3707 = !{!3708, !3709, !3710, !3711, !3712, !3713, !3714, !3715, !3716}
!3708 = !DIDerivedType(tag: DW_TAG_member, name: "last_update_time", scope: !3706, file: !1435, line: 477, baseType: !519, size: 64)
!3709 = !DIDerivedType(tag: DW_TAG_member, name: "load_sum", scope: !3706, file: !1435, line: 478, baseType: !519, size: 64, offset: 64)
!3710 = !DIDerivedType(tag: DW_TAG_member, name: "runnable_sum", scope: !3706, file: !1435, line: 479, baseType: !519, size: 64, offset: 128)
!3711 = !DIDerivedType(tag: DW_TAG_member, name: "util_sum", scope: !3706, file: !1435, line: 480, baseType: !578, size: 32, offset: 192)
!3712 = !DIDerivedType(tag: DW_TAG_member, name: "period_contrib", scope: !3706, file: !1435, line: 481, baseType: !578, size: 32, offset: 224)
!3713 = !DIDerivedType(tag: DW_TAG_member, name: "load_avg", scope: !3706, file: !1435, line: 482, baseType: !59, size: 64, offset: 256)
!3714 = !DIDerivedType(tag: DW_TAG_member, name: "runnable_avg", scope: !3706, file: !1435, line: 483, baseType: !59, size: 64, offset: 320)
!3715 = !DIDerivedType(tag: DW_TAG_member, name: "util_avg", scope: !3706, file: !1435, line: 484, baseType: !59, size: 64, offset: 384)
!3716 = !DIDerivedType(tag: DW_TAG_member, name: "util_est", scope: !3706, file: !1435, line: 485, baseType: !7, size: 32, offset: 448)
!3717 = !DIDerivedType(tag: DW_TAG_member, name: "rt", scope: !3629, file: !1435, line: 832, baseType: !3718, size: 384, offset: 3072)
!3718 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "sched_rt_entity", file: !1435, line: 587, size: 384, elements: !3719)
!3719 = !{!3720, !3721, !3722, !3723, !3724, !3725, !3726}
!3720 = !DIDerivedType(tag: DW_TAG_member, name: "run_list", scope: !3718, file: !1435, line: 588, baseType: !117, size: 128)
!3721 = !DIDerivedType(tag: DW_TAG_member, name: "timeout", scope: !3718, file: !1435, line: 589, baseType: !59, size: 64, offset: 128)
!3722 = !DIDerivedType(tag: DW_TAG_member, name: "watchdog_stamp", scope: !3718, file: !1435, line: 590, baseType: !59, size: 64, offset: 192)
!3723 = !DIDerivedType(tag: DW_TAG_member, name: "time_slice", scope: !3718, file: !1435, line: 591, baseType: !7, size: 32, offset: 256)
!3724 = !DIDerivedType(tag: DW_TAG_member, name: "on_rq", scope: !3718, file: !1435, line: 592, baseType: !46, size: 16, offset: 288)
!3725 = !DIDerivedType(tag: DW_TAG_member, name: "on_list", scope: !3718, file: !1435, line: 593, baseType: !46, size: 16, offset: 304)
!3726 = !DIDerivedType(tag: DW_TAG_member, name: "back", scope: !3718, file: !1435, line: 595, baseType: !3727, size: 64, offset: 320)
!3727 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3718, size: 64)
!3728 = !DIDerivedType(tag: DW_TAG_member, name: "dl", scope: !3629, file: !1435, line: 833, baseType: !3729, size: 1984, offset: 3456)
!3729 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "sched_dl_entity", file: !1435, line: 608, size: 1984, elements: !3730)
!3730 = !{!3731, !3732, !3733, !3734, !3735, !3736, !3737, !3738, !3739, !3740, !3741, !3742, !3743, !3744, !3745, !3746, !3747, !3748, !3749, !3750, !3753, !3759, !3764}
!3731 = !DIDerivedType(tag: DW_TAG_member, name: "rb_node", scope: !3729, file: !1435, line: 609, baseType: !173, size: 192, align: 64)
!3732 = !DIDerivedType(tag: DW_TAG_member, name: "dl_runtime", scope: !3729, file: !1435, line: 616, baseType: !519, size: 64, offset: 192)
!3733 = !DIDerivedType(tag: DW_TAG_member, name: "dl_deadline", scope: !3729, file: !1435, line: 617, baseType: !519, size: 64, offset: 256)
!3734 = !DIDerivedType(tag: DW_TAG_member, name: "dl_period", scope: !3729, file: !1435, line: 618, baseType: !519, size: 64, offset: 320)
!3735 = !DIDerivedType(tag: DW_TAG_member, name: "dl_bw", scope: !3729, file: !1435, line: 619, baseType: !519, size: 64, offset: 384)
!3736 = !DIDerivedType(tag: DW_TAG_member, name: "dl_density", scope: !3729, file: !1435, line: 620, baseType: !519, size: 64, offset: 448)
!3737 = !DIDerivedType(tag: DW_TAG_member, name: "runtime", scope: !3729, file: !1435, line: 627, baseType: !502, size: 64, offset: 512)
!3738 = !DIDerivedType(tag: DW_TAG_member, name: "deadline", scope: !3729, file: !1435, line: 628, baseType: !519, size: 64, offset: 576)
!3739 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !3729, file: !1435, line: 629, baseType: !7, size: 32, offset: 640)
!3740 = !DIDerivedType(tag: DW_TAG_member, name: "dl_throttled", scope: !3729, file: !1435, line: 662, baseType: !7, size: 1, offset: 672, flags: DIFlagBitField, extraData: i64 672)
!3741 = !DIDerivedType(tag: DW_TAG_member, name: "dl_yielded", scope: !3729, file: !1435, line: 663, baseType: !7, size: 1, offset: 673, flags: DIFlagBitField, extraData: i64 672)
!3742 = !DIDerivedType(tag: DW_TAG_member, name: "dl_non_contending", scope: !3729, file: !1435, line: 664, baseType: !7, size: 1, offset: 674, flags: DIFlagBitField, extraData: i64 672)
!3743 = !DIDerivedType(tag: DW_TAG_member, name: "dl_overrun", scope: !3729, file: !1435, line: 665, baseType: !7, size: 1, offset: 675, flags: DIFlagBitField, extraData: i64 672)
!3744 = !DIDerivedType(tag: DW_TAG_member, name: "dl_server", scope: !3729, file: !1435, line: 666, baseType: !7, size: 1, offset: 676, flags: DIFlagBitField, extraData: i64 672)
!3745 = !DIDerivedType(tag: DW_TAG_member, name: "dl_defer", scope: !3729, file: !1435, line: 667, baseType: !7, size: 1, offset: 677, flags: DIFlagBitField, extraData: i64 672)
!3746 = !DIDerivedType(tag: DW_TAG_member, name: "dl_defer_armed", scope: !3729, file: !1435, line: 668, baseType: !7, size: 1, offset: 678, flags: DIFlagBitField, extraData: i64 672)
!3747 = !DIDerivedType(tag: DW_TAG_member, name: "dl_defer_running", scope: !3729, file: !1435, line: 669, baseType: !7, size: 1, offset: 679, flags: DIFlagBitField, extraData: i64 672)
!3748 = !DIDerivedType(tag: DW_TAG_member, name: "dl_timer", scope: !3729, file: !1435, line: 675, baseType: !2103, size: 512, offset: 704)
!3749 = !DIDerivedType(tag: DW_TAG_member, name: "inactive_timer", scope: !3729, file: !1435, line: 684, baseType: !2103, size: 512, offset: 1216)
!3750 = !DIDerivedType(tag: DW_TAG_member, name: "rq", scope: !3729, file: !1435, line: 695, baseType: !3751, size: 64, offset: 1728)
!3751 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3752, size: 64)
!3752 = !DICompositeType(tag: DW_TAG_structure_type, name: "rq", file: !1435, line: 74, flags: DIFlagFwdDecl)
!3753 = !DIDerivedType(tag: DW_TAG_member, name: "server_has_tasks", scope: !3729, file: !1435, line: 696, baseType: !3754, size: 64, offset: 1792)
!3754 = !DIDerivedType(tag: DW_TAG_typedef, name: "dl_server_has_tasks_f", file: !1435, line: 605, baseType: !3755)
!3755 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3756, size: 64)
!3756 = !DISubroutineType(types: !3757)
!3757 = !{!614, !3758}
!3758 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3729, size: 64)
!3759 = !DIDerivedType(tag: DW_TAG_member, name: "server_pick_task", scope: !3729, file: !1435, line: 697, baseType: !3760, size: 64, offset: 1856)
!3760 = !DIDerivedType(tag: DW_TAG_typedef, name: "dl_server_pick_f", file: !1435, line: 606, baseType: !3761)
!3761 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3762, size: 64)
!3762 = !DISubroutineType(types: !3763)
!3763 = !{!3628, !3758}
!3764 = !DIDerivedType(tag: DW_TAG_member, name: "pi_se", scope: !3729, file: !1435, line: 705, baseType: !3758, size: 64, offset: 1920)
!3765 = !DIDerivedType(tag: DW_TAG_member, name: "dl_server", scope: !3629, file: !1435, line: 834, baseType: !3758, size: 64, offset: 5440)
!3766 = !DIDerivedType(tag: DW_TAG_member, name: "sched_class", scope: !3629, file: !1435, line: 838, baseType: !3767, size: 64, offset: 5504)
!3767 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3768, size: 64)
!3768 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !3769)
!3769 = !DICompositeType(tag: DW_TAG_structure_type, name: "sched_class", file: !1435, line: 838, flags: DIFlagFwdDecl)
!3770 = !DIDerivedType(tag: DW_TAG_member, name: "sched_task_group", scope: !3629, file: !1435, line: 847, baseType: !3771, size: 64, offset: 5568)
!3771 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3772, size: 64)
!3772 = !DICompositeType(tag: DW_TAG_structure_type, name: "task_group", file: !1435, line: 81, flags: DIFlagFwdDecl)
!3773 = !DIDerivedType(tag: DW_TAG_member, name: "stats", scope: !3629, file: !1435, line: 864, baseType: !3774, size: 2048, align: 512, offset: 5632)
!3774 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "sched_statistics", file: !1435, line: 499, size: 2048, align: 512, elements: !3775)
!3775 = !{!3776, !3777, !3778, !3779, !3780, !3781, !3782, !3783, !3784, !3785, !3786, !3787, !3788, !3789, !3790, !3791, !3792, !3793, !3794, !3795, !3796, !3797, !3798, !3799, !3800, !3801, !3802, !3803}
!3776 = !DIDerivedType(tag: DW_TAG_member, name: "wait_start", scope: !3774, file: !1435, line: 501, baseType: !519, size: 64)
!3777 = !DIDerivedType(tag: DW_TAG_member, name: "wait_max", scope: !3774, file: !1435, line: 502, baseType: !519, size: 64, offset: 64)
!3778 = !DIDerivedType(tag: DW_TAG_member, name: "wait_count", scope: !3774, file: !1435, line: 503, baseType: !519, size: 64, offset: 128)
!3779 = !DIDerivedType(tag: DW_TAG_member, name: "wait_sum", scope: !3774, file: !1435, line: 504, baseType: !519, size: 64, offset: 192)
!3780 = !DIDerivedType(tag: DW_TAG_member, name: "iowait_count", scope: !3774, file: !1435, line: 505, baseType: !519, size: 64, offset: 256)
!3781 = !DIDerivedType(tag: DW_TAG_member, name: "iowait_sum", scope: !3774, file: !1435, line: 506, baseType: !519, size: 64, offset: 320)
!3782 = !DIDerivedType(tag: DW_TAG_member, name: "sleep_start", scope: !3774, file: !1435, line: 508, baseType: !519, size: 64, offset: 384)
!3783 = !DIDerivedType(tag: DW_TAG_member, name: "sleep_max", scope: !3774, file: !1435, line: 509, baseType: !519, size: 64, offset: 448)
!3784 = !DIDerivedType(tag: DW_TAG_member, name: "sum_sleep_runtime", scope: !3774, file: !1435, line: 510, baseType: !502, size: 64, offset: 512)
!3785 = !DIDerivedType(tag: DW_TAG_member, name: "block_start", scope: !3774, file: !1435, line: 512, baseType: !519, size: 64, offset: 576)
!3786 = !DIDerivedType(tag: DW_TAG_member, name: "block_max", scope: !3774, file: !1435, line: 513, baseType: !519, size: 64, offset: 640)
!3787 = !DIDerivedType(tag: DW_TAG_member, name: "sum_block_runtime", scope: !3774, file: !1435, line: 514, baseType: !502, size: 64, offset: 704)
!3788 = !DIDerivedType(tag: DW_TAG_member, name: "exec_max", scope: !3774, file: !1435, line: 516, baseType: !502, size: 64, offset: 768)
!3789 = !DIDerivedType(tag: DW_TAG_member, name: "slice_max", scope: !3774, file: !1435, line: 517, baseType: !519, size: 64, offset: 832)
!3790 = !DIDerivedType(tag: DW_TAG_member, name: "nr_migrations_cold", scope: !3774, file: !1435, line: 519, baseType: !519, size: 64, offset: 896)
!3791 = !DIDerivedType(tag: DW_TAG_member, name: "nr_failed_migrations_affine", scope: !3774, file: !1435, line: 520, baseType: !519, size: 64, offset: 960)
!3792 = !DIDerivedType(tag: DW_TAG_member, name: "nr_failed_migrations_running", scope: !3774, file: !1435, line: 521, baseType: !519, size: 64, offset: 1024)
!3793 = !DIDerivedType(tag: DW_TAG_member, name: "nr_failed_migrations_hot", scope: !3774, file: !1435, line: 522, baseType: !519, size: 64, offset: 1088)
!3794 = !DIDerivedType(tag: DW_TAG_member, name: "nr_forced_migrations", scope: !3774, file: !1435, line: 523, baseType: !519, size: 64, offset: 1152)
!3795 = !DIDerivedType(tag: DW_TAG_member, name: "nr_wakeups", scope: !3774, file: !1435, line: 525, baseType: !519, size: 64, offset: 1216)
!3796 = !DIDerivedType(tag: DW_TAG_member, name: "nr_wakeups_sync", scope: !3774, file: !1435, line: 526, baseType: !519, size: 64, offset: 1280)
!3797 = !DIDerivedType(tag: DW_TAG_member, name: "nr_wakeups_migrate", scope: !3774, file: !1435, line: 527, baseType: !519, size: 64, offset: 1344)
!3798 = !DIDerivedType(tag: DW_TAG_member, name: "nr_wakeups_local", scope: !3774, file: !1435, line: 528, baseType: !519, size: 64, offset: 1408)
!3799 = !DIDerivedType(tag: DW_TAG_member, name: "nr_wakeups_remote", scope: !3774, file: !1435, line: 529, baseType: !519, size: 64, offset: 1472)
!3800 = !DIDerivedType(tag: DW_TAG_member, name: "nr_wakeups_affine", scope: !3774, file: !1435, line: 530, baseType: !519, size: 64, offset: 1536)
!3801 = !DIDerivedType(tag: DW_TAG_member, name: "nr_wakeups_affine_attempts", scope: !3774, file: !1435, line: 531, baseType: !519, size: 64, offset: 1600)
!3802 = !DIDerivedType(tag: DW_TAG_member, name: "nr_wakeups_passive", scope: !3774, file: !1435, line: 532, baseType: !519, size: 64, offset: 1664)
!3803 = !DIDerivedType(tag: DW_TAG_member, name: "nr_wakeups_idle", scope: !3774, file: !1435, line: 533, baseType: !519, size: 64, offset: 1728)
!3804 = !DIDerivedType(tag: DW_TAG_member, name: "btrace_seq", scope: !3629, file: !1435, line: 872, baseType: !7, size: 32, offset: 7680)
!3805 = !DIDerivedType(tag: DW_TAG_member, name: "policy", scope: !3629, file: !1435, line: 875, baseType: !7, size: 32, offset: 7712)
!3806 = !DIDerivedType(tag: DW_TAG_member, name: "max_allowed_capacity", scope: !3629, file: !1435, line: 876, baseType: !59, size: 64, offset: 7744)
!3807 = !DIDerivedType(tag: DW_TAG_member, name: "nr_cpus_allowed", scope: !3629, file: !1435, line: 877, baseType: !42, size: 32, offset: 7808)
!3808 = !DIDerivedType(tag: DW_TAG_member, name: "cpus_ptr", scope: !3629, file: !1435, line: 878, baseType: !3809, size: 64, offset: 7872)
!3809 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3810, size: 64)
!3810 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !3811)
!3811 = !DIDerivedType(tag: DW_TAG_typedef, name: "cpumask_t", file: !3812, line: 9, baseType: !3813)
!3812 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/cpumask_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "140007f380056ed76e870d516586e9f0")
!3813 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cpumask", file: !3812, line: 9, size: 64, elements: !3814)
!3814 = !{!3815}
!3815 = !DIDerivedType(tag: DW_TAG_member, name: "bits", scope: !3813, file: !3812, line: 9, baseType: !3816, size: 64)
!3816 = !DICompositeType(tag: DW_TAG_array_type, baseType: !59, size: 64, elements: !1839)
!3817 = !DIDerivedType(tag: DW_TAG_member, name: "user_cpus_ptr", scope: !3629, file: !1435, line: 879, baseType: !3818, size: 64, offset: 7936)
!3818 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3811, size: 64)
!3819 = !DIDerivedType(tag: DW_TAG_member, name: "cpus_mask", scope: !3629, file: !1435, line: 880, baseType: !3811, size: 64, offset: 8000)
!3820 = !DIDerivedType(tag: DW_TAG_member, name: "migration_pending", scope: !3629, file: !1435, line: 881, baseType: !40, size: 64, offset: 8064)
!3821 = !DIDerivedType(tag: DW_TAG_member, name: "migration_disabled", scope: !3629, file: !1435, line: 883, baseType: !46, size: 16, offset: 8128)
!3822 = !DIDerivedType(tag: DW_TAG_member, name: "migration_flags", scope: !3629, file: !1435, line: 885, baseType: !46, size: 16, offset: 8144)
!3823 = !DIDerivedType(tag: DW_TAG_member, name: "rcu_read_lock_nesting", scope: !3629, file: !1435, line: 888, baseType: !42, size: 32, offset: 8160)
!3824 = !DIDerivedType(tag: DW_TAG_member, name: "rcu_read_unlock_special", scope: !3629, file: !1435, line: 889, baseType: !3825, size: 32, offset: 8192)
!3825 = distinct !DICompositeType(tag: DW_TAG_union_type, name: "rcu_special", file: !1435, line: 744, size: 32, elements: !3826)
!3826 = !{!3827, !3834}
!3827 = !DIDerivedType(tag: DW_TAG_member, name: "b", scope: !3825, file: !1435, line: 750, baseType: !3828, size: 32)
!3828 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !3825, file: !1435, line: 745, size: 32, elements: !3829)
!3829 = !{!3830, !3831, !3832, !3833}
!3830 = !DIDerivedType(tag: DW_TAG_member, name: "blocked", scope: !3828, file: !1435, line: 746, baseType: !103, size: 8)
!3831 = !DIDerivedType(tag: DW_TAG_member, name: "need_qs", scope: !3828, file: !1435, line: 747, baseType: !103, size: 8, offset: 8)
!3832 = !DIDerivedType(tag: DW_TAG_member, name: "exp_hint", scope: !3828, file: !1435, line: 748, baseType: !103, size: 8, offset: 16)
!3833 = !DIDerivedType(tag: DW_TAG_member, name: "need_mb", scope: !3828, file: !1435, line: 749, baseType: !103, size: 8, offset: 24)
!3834 = !DIDerivedType(tag: DW_TAG_member, name: "s", scope: !3825, file: !1435, line: 751, baseType: !578, size: 32)
!3835 = !DIDerivedType(tag: DW_TAG_member, name: "rcu_node_entry", scope: !3629, file: !1435, line: 890, baseType: !117, size: 128, offset: 8256)
!3836 = !DIDerivedType(tag: DW_TAG_member, name: "rcu_blocked_node", scope: !3629, file: !1435, line: 891, baseType: !3837, size: 64, offset: 8384)
!3837 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3838, size: 64)
!3838 = !DICompositeType(tag: DW_TAG_structure_type, name: "rcu_node", file: !1435, line: 70, flags: DIFlagFwdDecl)
!3839 = !DIDerivedType(tag: DW_TAG_member, name: "rcu_tasks_nvcsw", scope: !3629, file: !1435, line: 895, baseType: !59, size: 64, offset: 8448)
!3840 = !DIDerivedType(tag: DW_TAG_member, name: "rcu_tasks_holdout", scope: !3629, file: !1435, line: 896, baseType: !103, size: 8, offset: 8512)
!3841 = !DIDerivedType(tag: DW_TAG_member, name: "rcu_tasks_idx", scope: !3629, file: !1435, line: 897, baseType: !103, size: 8, offset: 8520)
!3842 = !DIDerivedType(tag: DW_TAG_member, name: "rcu_tasks_idle_cpu", scope: !3629, file: !1435, line: 898, baseType: !42, size: 32, offset: 8544)
!3843 = !DIDerivedType(tag: DW_TAG_member, name: "rcu_tasks_holdout_list", scope: !3629, file: !1435, line: 899, baseType: !117, size: 128, offset: 8576)
!3844 = !DIDerivedType(tag: DW_TAG_member, name: "rcu_tasks_exit_cpu", scope: !3629, file: !1435, line: 900, baseType: !42, size: 32, offset: 8704)
!3845 = !DIDerivedType(tag: DW_TAG_member, name: "rcu_tasks_exit_list", scope: !3629, file: !1435, line: 901, baseType: !117, size: 128, offset: 8768)
!3846 = !DIDerivedType(tag: DW_TAG_member, name: "sched_info", scope: !3629, file: !1435, line: 913, baseType: !3847, size: 256, offset: 8896)
!3847 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "sched_info", file: !1435, line: 391, size: 256, elements: !3848)
!3848 = !{!3849, !3850, !3851, !3852}
!3849 = !DIDerivedType(tag: DW_TAG_member, name: "pcount", scope: !3847, file: !1435, line: 396, baseType: !59, size: 64)
!3850 = !DIDerivedType(tag: DW_TAG_member, name: "run_delay", scope: !3847, file: !1435, line: 399, baseType: !521, size: 64, offset: 64)
!3851 = !DIDerivedType(tag: DW_TAG_member, name: "last_arrival", scope: !3847, file: !1435, line: 404, baseType: !521, size: 64, offset: 128)
!3852 = !DIDerivedType(tag: DW_TAG_member, name: "last_queued", scope: !3847, file: !1435, line: 407, baseType: !521, size: 64, offset: 192)
!3853 = !DIDerivedType(tag: DW_TAG_member, name: "tasks", scope: !3629, file: !1435, line: 915, baseType: !117, size: 128, offset: 9152)
!3854 = !DIDerivedType(tag: DW_TAG_member, name: "pushable_tasks", scope: !3629, file: !1435, line: 917, baseType: !1814, size: 320, offset: 9280)
!3855 = !DIDerivedType(tag: DW_TAG_member, name: "pushable_dl_tasks", scope: !3629, file: !1435, line: 918, baseType: !173, size: 192, align: 64, offset: 9600)
!3856 = !DIDerivedType(tag: DW_TAG_member, name: "mm", scope: !3629, file: !1435, line: 921, baseType: !1180, size: 64, offset: 9792)
!3857 = !DIDerivedType(tag: DW_TAG_member, name: "active_mm", scope: !3629, file: !1435, line: 922, baseType: !1180, size: 64, offset: 9856)
!3858 = !DIDerivedType(tag: DW_TAG_member, name: "faults_disabled_mapping", scope: !3629, file: !1435, line: 923, baseType: !1030, size: 64, offset: 9920)
!3859 = !DIDerivedType(tag: DW_TAG_member, name: "exit_state", scope: !3629, file: !1435, line: 925, baseType: !42, size: 32, offset: 9984)
!3860 = !DIDerivedType(tag: DW_TAG_member, name: "exit_code", scope: !3629, file: !1435, line: 926, baseType: !42, size: 32, offset: 10016)
!3861 = !DIDerivedType(tag: DW_TAG_member, name: "exit_signal", scope: !3629, file: !1435, line: 927, baseType: !42, size: 32, offset: 10048)
!3862 = !DIDerivedType(tag: DW_TAG_member, name: "pdeath_signal", scope: !3629, file: !1435, line: 929, baseType: !42, size: 32, offset: 10080)
!3863 = !DIDerivedType(tag: DW_TAG_member, name: "jobctl", scope: !3629, file: !1435, line: 931, baseType: !59, size: 64, offset: 10112)
!3864 = !DIDerivedType(tag: DW_TAG_member, name: "personality", scope: !3629, file: !1435, line: 934, baseType: !7, size: 32, offset: 10176)
!3865 = !DIDerivedType(tag: DW_TAG_member, name: "sched_reset_on_fork", scope: !3629, file: !1435, line: 937, baseType: !7, size: 1, offset: 10208, flags: DIFlagBitField, extraData: i64 10208)
!3866 = !DIDerivedType(tag: DW_TAG_member, name: "sched_contributes_to_load", scope: !3629, file: !1435, line: 938, baseType: !7, size: 1, offset: 10209, flags: DIFlagBitField, extraData: i64 10208)
!3867 = !DIDerivedType(tag: DW_TAG_member, name: "sched_migrated", scope: !3629, file: !1435, line: 939, baseType: !7, size: 1, offset: 10210, flags: DIFlagBitField, extraData: i64 10208)
!3868 = !DIDerivedType(tag: DW_TAG_member, name: "sched_remote_wakeup", scope: !3629, file: !1435, line: 959, baseType: !7, size: 1, offset: 10240, flags: DIFlagBitField, extraData: i64 10240)
!3869 = !DIDerivedType(tag: DW_TAG_member, name: "sched_rt_mutex", scope: !3629, file: !1435, line: 961, baseType: !7, size: 1, offset: 10241, flags: DIFlagBitField, extraData: i64 10240)
!3870 = !DIDerivedType(tag: DW_TAG_member, name: "in_execve", scope: !3629, file: !1435, line: 965, baseType: !7, size: 1, offset: 10242, flags: DIFlagBitField, extraData: i64 10240)
!3871 = !DIDerivedType(tag: DW_TAG_member, name: "in_iowait", scope: !3629, file: !1435, line: 966, baseType: !7, size: 1, offset: 10243, flags: DIFlagBitField, extraData: i64 10240)
!3872 = !DIDerivedType(tag: DW_TAG_member, name: "restore_sigmask", scope: !3629, file: !1435, line: 968, baseType: !7, size: 1, offset: 10244, flags: DIFlagBitField, extraData: i64 10240)
!3873 = !DIDerivedType(tag: DW_TAG_member, name: "no_cgroup_migration", scope: !3629, file: !1435, line: 982, baseType: !7, size: 1, offset: 10245, flags: DIFlagBitField, extraData: i64 10240)
!3874 = !DIDerivedType(tag: DW_TAG_member, name: "frozen", scope: !3629, file: !1435, line: 984, baseType: !7, size: 1, offset: 10246, flags: DIFlagBitField, extraData: i64 10240)
!3875 = !DIDerivedType(tag: DW_TAG_member, name: "use_memdelay", scope: !3629, file: !1435, line: 987, baseType: !7, size: 1, offset: 10247, flags: DIFlagBitField, extraData: i64 10240)
!3876 = !DIDerivedType(tag: DW_TAG_member, name: "in_eventfd", scope: !3629, file: !1435, line: 999, baseType: !7, size: 1, offset: 10248, flags: DIFlagBitField, extraData: i64 10240)
!3877 = !DIDerivedType(tag: DW_TAG_member, name: "pasid_activated", scope: !3629, file: !1435, line: 1002, baseType: !7, size: 1, offset: 10249, flags: DIFlagBitField, extraData: i64 10240)
!3878 = !DIDerivedType(tag: DW_TAG_member, name: "reported_split_lock", scope: !3629, file: !1435, line: 1005, baseType: !7, size: 1, offset: 10250, flags: DIFlagBitField, extraData: i64 10240)
!3879 = !DIDerivedType(tag: DW_TAG_member, name: "in_thrashing", scope: !3629, file: !1435, line: 1009, baseType: !7, size: 1, offset: 10251, flags: DIFlagBitField, extraData: i64 10240)
!3880 = !DIDerivedType(tag: DW_TAG_member, name: "atomic_flags", scope: !3629, file: !1435, line: 1014, baseType: !59, size: 64, offset: 10304)
!3881 = !DIDerivedType(tag: DW_TAG_member, name: "restart_block", scope: !3629, file: !1435, line: 1016, baseType: !3882, size: 448, offset: 10368)
!3882 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "restart_block", file: !321, line: 25, size: 448, elements: !3883)
!3883 = !{!3884, !3885, !3890}
!3884 = !DIDerivedType(tag: DW_TAG_member, name: "arch_data", scope: !3882, file: !321, line: 26, baseType: !59, size: 64)
!3885 = !DIDerivedType(tag: DW_TAG_member, name: "fn", scope: !3882, file: !321, line: 27, baseType: !3886, size: 64, offset: 64)
!3886 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3887, size: 64)
!3887 = !DISubroutineType(types: !3888)
!3888 = !{!892, !3889}
!3889 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3882, size: 64)
!3890 = !DIDerivedType(tag: DW_TAG_member, scope: !3882, file: !321, line: 28, baseType: !3891, size: 320, offset: 128)
!3891 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !3882, file: !321, line: 28, size: 320, elements: !3892)
!3892 = !{!3893, !3903, !3928}
!3893 = !DIDerivedType(tag: DW_TAG_member, name: "futex", scope: !3891, file: !321, line: 37, baseType: !3894, size: 320)
!3894 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !3891, file: !321, line: 30, size: 320, elements: !3895)
!3895 = !{!3896, !3898, !3899, !3900, !3901, !3902}
!3896 = !DIDerivedType(tag: DW_TAG_member, name: "uaddr", scope: !3894, file: !321, line: 31, baseType: !3897, size: 64)
!3897 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !578, size: 64)
!3898 = !DIDerivedType(tag: DW_TAG_member, name: "val", scope: !3894, file: !321, line: 32, baseType: !578, size: 32, offset: 64)
!3899 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !3894, file: !321, line: 33, baseType: !578, size: 32, offset: 96)
!3900 = !DIDerivedType(tag: DW_TAG_member, name: "bitset", scope: !3894, file: !321, line: 34, baseType: !578, size: 32, offset: 128)
!3901 = !DIDerivedType(tag: DW_TAG_member, name: "time", scope: !3894, file: !321, line: 35, baseType: !519, size: 64, offset: 192)
!3902 = !DIDerivedType(tag: DW_TAG_member, name: "uaddr2", scope: !3894, file: !321, line: 36, baseType: !3897, size: 64, offset: 256)
!3903 = !DIDerivedType(tag: DW_TAG_member, name: "nanosleep", scope: !3891, file: !321, line: 47, baseType: !3904, size: 192)
!3904 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !3891, file: !321, line: 39, size: 192, elements: !3905)
!3905 = !{!3906, !3907, !3908, !3927}
!3906 = !DIDerivedType(tag: DW_TAG_member, name: "clockid", scope: !3904, file: !321, line: 40, baseType: !2149, size: 32)
!3907 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !3904, file: !321, line: 41, baseType: !320, size: 32, offset: 32)
!3908 = !DIDerivedType(tag: DW_TAG_member, scope: !3904, file: !321, line: 42, baseType: !3909, size: 64, offset: 64)
!3909 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !3904, file: !321, line: 42, size: 64, elements: !3910)
!3910 = !{!3911, !3919}
!3911 = !DIDerivedType(tag: DW_TAG_member, name: "rmtp", scope: !3909, file: !321, line: 43, baseType: !3912, size: 64)
!3912 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3913, size: 64)
!3913 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__kernel_timespec", file: !3914, line: 7, size: 128, elements: !3915)
!3914 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/uapi/linux/time_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "caebe0f0ae09abba9cc01ca1749c16bf")
!3915 = !{!3916, !3918}
!3916 = !DIDerivedType(tag: DW_TAG_member, name: "tv_sec", scope: !3913, file: !3914, line: 8, baseType: !3917, size: 64)
!3917 = !DIDerivedType(tag: DW_TAG_typedef, name: "__kernel_time64_t", file: !57, line: 93, baseType: !63)
!3918 = !DIDerivedType(tag: DW_TAG_member, name: "tv_nsec", scope: !3913, file: !3914, line: 9, baseType: !63, size: 64, offset: 64)
!3919 = !DIDerivedType(tag: DW_TAG_member, name: "compat_rmtp", scope: !3909, file: !321, line: 44, baseType: !3920, size: 64)
!3920 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3921, size: 64)
!3921 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "old_timespec32", file: !3922, line: 7, size: 64, elements: !3923)
!3922 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/vdso/time32.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "72b5ce349b0f5d7430b5761f0416ad5e")
!3923 = !{!3924, !3926}
!3924 = !DIDerivedType(tag: DW_TAG_member, name: "tv_sec", scope: !3921, file: !3922, line: 8, baseType: !3925, size: 32)
!3925 = !DIDerivedType(tag: DW_TAG_typedef, name: "old_time32_t", file: !3922, line: 5, baseType: !541)
!3926 = !DIDerivedType(tag: DW_TAG_member, name: "tv_nsec", scope: !3921, file: !3922, line: 9, baseType: !541, size: 32, offset: 32)
!3927 = !DIDerivedType(tag: DW_TAG_member, name: "expires", scope: !3904, file: !321, line: 46, baseType: !519, size: 64, offset: 128)
!3928 = !DIDerivedType(tag: DW_TAG_member, name: "poll", scope: !3891, file: !321, line: 55, baseType: !3929, size: 256)
!3929 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !3891, file: !321, line: 49, size: 256, elements: !3930)
!3930 = !{!3931, !3934, !3935, !3936, !3937}
!3931 = !DIDerivedType(tag: DW_TAG_member, name: "ufds", scope: !3929, file: !321, line: 50, baseType: !3932, size: 64)
!3932 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3933, size: 64)
!3933 = !DICompositeType(tag: DW_TAG_structure_type, name: "pollfd", file: !321, line: 14, flags: DIFlagFwdDecl)
!3934 = !DIDerivedType(tag: DW_TAG_member, name: "nfds", scope: !3929, file: !321, line: 51, baseType: !42, size: 32, offset: 64)
!3935 = !DIDerivedType(tag: DW_TAG_member, name: "has_timeout", scope: !3929, file: !321, line: 52, baseType: !42, size: 32, offset: 96)
!3936 = !DIDerivedType(tag: DW_TAG_member, name: "tv_sec", scope: !3929, file: !321, line: 53, baseType: !59, size: 64, offset: 128)
!3937 = !DIDerivedType(tag: DW_TAG_member, name: "tv_nsec", scope: !3929, file: !321, line: 54, baseType: !59, size: 64, offset: 192)
!3938 = !DIDerivedType(tag: DW_TAG_member, name: "pid", scope: !3629, file: !1435, line: 1018, baseType: !2977, size: 32, offset: 10816)
!3939 = !DIDerivedType(tag: DW_TAG_member, name: "tgid", scope: !3629, file: !1435, line: 1019, baseType: !2977, size: 32, offset: 10848)
!3940 = !DIDerivedType(tag: DW_TAG_member, name: "stack_canary", scope: !3629, file: !1435, line: 1023, baseType: !59, size: 64, offset: 10880)
!3941 = !DIDerivedType(tag: DW_TAG_member, name: "real_parent", scope: !3629, file: !1435, line: 1032, baseType: !3628, size: 64, offset: 10944)
!3942 = !DIDerivedType(tag: DW_TAG_member, name: "parent", scope: !3629, file: !1435, line: 1035, baseType: !3628, size: 64, offset: 11008)
!3943 = !DIDerivedType(tag: DW_TAG_member, name: "children", scope: !3629, file: !1435, line: 1040, baseType: !117, size: 128, offset: 11072)
!3944 = !DIDerivedType(tag: DW_TAG_member, name: "sibling", scope: !3629, file: !1435, line: 1041, baseType: !117, size: 128, offset: 11200)
!3945 = !DIDerivedType(tag: DW_TAG_member, name: "group_leader", scope: !3629, file: !1435, line: 1042, baseType: !3628, size: 64, offset: 11328)
!3946 = !DIDerivedType(tag: DW_TAG_member, name: "ptraced", scope: !3629, file: !1435, line: 1050, baseType: !117, size: 128, offset: 11392)
!3947 = !DIDerivedType(tag: DW_TAG_member, name: "ptrace_entry", scope: !3629, file: !1435, line: 1051, baseType: !117, size: 128, offset: 11520)
!3948 = !DIDerivedType(tag: DW_TAG_member, name: "thread_pid", scope: !3629, file: !1435, line: 1054, baseType: !3949, size: 64, offset: 11648)
!3949 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3950, size: 64)
!3950 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "pid", file: !3951, line: 55, size: 896, elements: !3952)
!3951 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/pid.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "c7e21dbe45a193165eae6b93f0dc9651")
!3952 = !{!3953, !3954, !3955, !3956, !3957, !3958, !3960, !3961, !3962, !3963}
!3953 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !3950, file: !3951, line: 57, baseType: !533, size: 32)
!3954 = !DIDerivedType(tag: DW_TAG_member, name: "level", scope: !3950, file: !3951, line: 58, baseType: !7, size: 32, offset: 32)
!3955 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !3950, file: !3951, line: 59, baseType: !79, size: 32, offset: 64)
!3956 = !DIDerivedType(tag: DW_TAG_member, name: "stashed", scope: !3950, file: !3951, line: 60, baseType: !740, size: 64, offset: 128)
!3957 = !DIDerivedType(tag: DW_TAG_member, name: "ino", scope: !3950, file: !3951, line: 61, baseType: !519, size: 64, offset: 192)
!3958 = !DIDerivedType(tag: DW_TAG_member, name: "tasks", scope: !3950, file: !3951, line: 63, baseType: !3959, size: 256, offset: 256)
!3959 = !DICompositeType(tag: DW_TAG_array_type, baseType: !216, size: 256, elements: !635)
!3960 = !DIDerivedType(tag: DW_TAG_member, name: "inodes", scope: !3950, file: !3951, line: 64, baseType: !216, size: 64, offset: 512)
!3961 = !DIDerivedType(tag: DW_TAG_member, name: "wait_pidfd", scope: !3950, file: !3951, line: 66, baseType: !74, size: 192, offset: 576)
!3962 = !DIDerivedType(tag: DW_TAG_member, name: "rcu", scope: !3950, file: !3951, line: 67, baseType: !129, size: 128, align: 64, offset: 768)
!3963 = !DIDerivedType(tag: DW_TAG_member, name: "numbers", scope: !3950, file: !3951, line: 68, baseType: !3964, offset: 896)
!3964 = !DICompositeType(tag: DW_TAG_array_type, baseType: !3965, elements: !1353)
!3965 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "upid", file: !3951, line: 50, size: 128, elements: !3966)
!3966 = !{!3967, !3968}
!3967 = !DIDerivedType(tag: DW_TAG_member, name: "nr", scope: !3965, file: !3951, line: 51, baseType: !42, size: 32)
!3968 = !DIDerivedType(tag: DW_TAG_member, name: "ns", scope: !3965, file: !3951, line: 52, baseType: !3969, size: 64, offset: 64)
!3969 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3970, size: 64)
!3970 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "pid_namespace", file: !3971, line: 26, size: 1152, elements: !3972)
!3971 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/pid_namespace.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "3221cc45d98b4ae2705e7ef463aa36b0")
!3972 = !{!3973, !3980, !3981, !3982, !3983, !3987, !3988, !3989, !3992, !3993, !4007, !4008, !4009}
!3973 = !DIDerivedType(tag: DW_TAG_member, name: "idr", scope: !3970, file: !3971, line: 27, baseType: !3974, size: 192)
!3974 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "idr", file: !3975, line: 19, size: 192, elements: !3976)
!3975 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/idr.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "8f4a216f458b933152d42423d3daf6f8")
!3976 = !{!3977, !3978, !3979}
!3977 = !DIDerivedType(tag: DW_TAG_member, name: "idr_rt", scope: !3974, file: !3975, line: 20, baseType: !1035, size: 128)
!3978 = !DIDerivedType(tag: DW_TAG_member, name: "idr_base", scope: !3974, file: !3975, line: 21, baseType: !7, size: 32, offset: 128)
!3979 = !DIDerivedType(tag: DW_TAG_member, name: "idr_next", scope: !3974, file: !3975, line: 22, baseType: !7, size: 32, offset: 160)
!3980 = !DIDerivedType(tag: DW_TAG_member, name: "rcu", scope: !3970, file: !3971, line: 28, baseType: !129, size: 128, align: 64, offset: 192)
!3981 = !DIDerivedType(tag: DW_TAG_member, name: "pid_allocated", scope: !3970, file: !3971, line: 29, baseType: !7, size: 32, offset: 320)
!3982 = !DIDerivedType(tag: DW_TAG_member, name: "child_reaper", scope: !3970, file: !3971, line: 30, baseType: !3628, size: 64, offset: 384)
!3983 = !DIDerivedType(tag: DW_TAG_member, name: "pid_cachep", scope: !3970, file: !3971, line: 31, baseType: !3984, size: 64, offset: 448)
!3984 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3985, size: 64)
!3985 = !DICompositeType(tag: DW_TAG_structure_type, name: "kmem_cache", file: !3986, line: 12, flags: DIFlagFwdDecl)
!3986 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/kasan.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "4d051858be54f1c7c9b34ae809b000b7")
!3987 = !DIDerivedType(tag: DW_TAG_member, name: "level", scope: !3970, file: !3971, line: 32, baseType: !7, size: 32, offset: 512)
!3988 = !DIDerivedType(tag: DW_TAG_member, name: "parent", scope: !3970, file: !3971, line: 33, baseType: !3969, size: 64, offset: 576)
!3989 = !DIDerivedType(tag: DW_TAG_member, name: "bacct", scope: !3970, file: !3971, line: 35, baseType: !3990, size: 64, offset: 640)
!3990 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3991, size: 64)
!3991 = !DICompositeType(tag: DW_TAG_structure_type, name: "fs_pin", file: !3971, line: 17, flags: DIFlagFwdDecl)
!3992 = !DIDerivedType(tag: DW_TAG_member, name: "user_ns", scope: !3970, file: !3971, line: 37, baseType: !700, size: 64, offset: 704)
!3993 = !DIDerivedType(tag: DW_TAG_member, name: "ucounts", scope: !3970, file: !3971, line: 38, baseType: !3994, size: 64, offset: 768)
!3994 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3995, size: 64)
!3995 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "ucounts", file: !702, line: 117, size: 1152, elements: !3996)
!3996 = !{!3997, !3998, !3999, !4000, !4001, !4005}
!3997 = !DIDerivedType(tag: DW_TAG_member, name: "node", scope: !3995, file: !702, line: 118, baseType: !220, size: 128)
!3998 = !DIDerivedType(tag: DW_TAG_member, name: "ns", scope: !3995, file: !702, line: 119, baseType: !700, size: 64, offset: 128)
!3999 = !DIDerivedType(tag: DW_TAG_member, name: "uid", scope: !3995, file: !702, line: 120, baseType: !188, size: 32, offset: 192)
!4000 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !3995, file: !702, line: 121, baseType: !69, size: 32, offset: 224)
!4001 = !DIDerivedType(tag: DW_TAG_member, name: "ucount", scope: !3995, file: !702, line: 122, baseType: !4002, size: 640, offset: 256)
!4002 = !DICompositeType(tag: DW_TAG_array_type, baseType: !496, size: 640, elements: !4003)
!4003 = !{!4004}
!4004 = !DISubrange(count: 10)
!4005 = !DIDerivedType(tag: DW_TAG_member, name: "rlimit", scope: !3995, file: !702, line: 123, baseType: !4006, size: 256, offset: 896)
!4006 = !DICompositeType(tag: DW_TAG_array_type, baseType: !496, size: 256, elements: !635)
!4007 = !DIDerivedType(tag: DW_TAG_member, name: "reboot", scope: !3970, file: !3971, line: 39, baseType: !42, size: 32, offset: 832)
!4008 = !DIDerivedType(tag: DW_TAG_member, name: "ns", scope: !3970, file: !3971, line: 40, baseType: !736, size: 192, offset: 896)
!4009 = !DIDerivedType(tag: DW_TAG_member, name: "memfd_noexec_scope", scope: !3970, file: !3971, line: 42, baseType: !42, size: 32, offset: 1088)
!4010 = !DIDerivedType(tag: DW_TAG_member, name: "pid_links", scope: !3629, file: !1435, line: 1055, baseType: !4011, size: 512, offset: 11712)
!4011 = !DICompositeType(tag: DW_TAG_array_type, baseType: !220, size: 512, elements: !635)
!4012 = !DIDerivedType(tag: DW_TAG_member, name: "thread_node", scope: !3629, file: !1435, line: 1056, baseType: !117, size: 128, offset: 12224)
!4013 = !DIDerivedType(tag: DW_TAG_member, name: "vfork_done", scope: !3629, file: !1435, line: 1058, baseType: !138, size: 64, offset: 12352)
!4014 = !DIDerivedType(tag: DW_TAG_member, name: "set_child_tid", scope: !3629, file: !1435, line: 1061, baseType: !2694, size: 64, offset: 12416)
!4015 = !DIDerivedType(tag: DW_TAG_member, name: "clear_child_tid", scope: !3629, file: !1435, line: 1064, baseType: !2694, size: 64, offset: 12480)
!4016 = !DIDerivedType(tag: DW_TAG_member, name: "worker_private", scope: !3629, file: !1435, line: 1067, baseType: !40, size: 64, offset: 12544)
!4017 = !DIDerivedType(tag: DW_TAG_member, name: "utime", scope: !3629, file: !1435, line: 1069, baseType: !519, size: 64, offset: 12608)
!4018 = !DIDerivedType(tag: DW_TAG_member, name: "stime", scope: !3629, file: !1435, line: 1070, baseType: !519, size: 64, offset: 12672)
!4019 = !DIDerivedType(tag: DW_TAG_member, name: "gtime", scope: !3629, file: !1435, line: 1075, baseType: !519, size: 64, offset: 12736)
!4020 = !DIDerivedType(tag: DW_TAG_member, name: "prev_cputime", scope: !3629, file: !1435, line: 1076, baseType: !4021, size: 192, offset: 12800)
!4021 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "prev_cputime", file: !1435, line: 339, size: 192, elements: !4022)
!4022 = !{!4023, !4024, !4025}
!4023 = !DIDerivedType(tag: DW_TAG_member, name: "utime", scope: !4021, file: !1435, line: 341, baseType: !519, size: 64)
!4024 = !DIDerivedType(tag: DW_TAG_member, name: "stime", scope: !4021, file: !1435, line: 342, baseType: !519, size: 64, offset: 64)
!4025 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !4021, file: !1435, line: 343, baseType: !148, size: 32, offset: 128)
!4026 = !DIDerivedType(tag: DW_TAG_member, name: "nvcsw", scope: !3629, file: !1435, line: 1085, baseType: !59, size: 64, offset: 12992)
!4027 = !DIDerivedType(tag: DW_TAG_member, name: "nivcsw", scope: !3629, file: !1435, line: 1086, baseType: !59, size: 64, offset: 13056)
!4028 = !DIDerivedType(tag: DW_TAG_member, name: "start_time", scope: !3629, file: !1435, line: 1089, baseType: !519, size: 64, offset: 13120)
!4029 = !DIDerivedType(tag: DW_TAG_member, name: "start_boottime", scope: !3629, file: !1435, line: 1092, baseType: !519, size: 64, offset: 13184)
!4030 = !DIDerivedType(tag: DW_TAG_member, name: "min_flt", scope: !3629, file: !1435, line: 1095, baseType: !59, size: 64, offset: 13248)
!4031 = !DIDerivedType(tag: DW_TAG_member, name: "maj_flt", scope: !3629, file: !1435, line: 1096, baseType: !59, size: 64, offset: 13312)
!4032 = !DIDerivedType(tag: DW_TAG_member, name: "posix_cputimers", scope: !3629, file: !1435, line: 1099, baseType: !4033, size: 640, offset: 13376)
!4033 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "posix_cputimers", file: !4034, line: 56, size: 640, elements: !4035)
!4034 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/posix-timers_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "478c00aaac76bc36014472955c37c1b7")
!4035 = !{!4036, !4042, !4043}
!4036 = !DIDerivedType(tag: DW_TAG_member, name: "bases", scope: !4033, file: !4034, line: 57, baseType: !4037, size: 576)
!4037 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4038, size: 576, elements: !962)
!4038 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "posix_cputimer_base", file: !4034, line: 41, size: 192, elements: !4039)
!4039 = !{!4040, !4041}
!4040 = !DIDerivedType(tag: DW_TAG_member, name: "nextevt", scope: !4038, file: !4034, line: 42, baseType: !519, size: 64)
!4041 = !DIDerivedType(tag: DW_TAG_member, name: "tqhead", scope: !4038, file: !4034, line: 43, baseType: !2158, size: 128, offset: 64)
!4042 = !DIDerivedType(tag: DW_TAG_member, name: "timers_active", scope: !4033, file: !4034, line: 58, baseType: !7, size: 32, offset: 576)
!4043 = !DIDerivedType(tag: DW_TAG_member, name: "expiry_active", scope: !4033, file: !4034, line: 59, baseType: !7, size: 32, offset: 608)
!4044 = !DIDerivedType(tag: DW_TAG_member, name: "posix_cputimers_work", scope: !3629, file: !1435, line: 1102, baseType: !4045, size: 448, offset: 14016)
!4045 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "posix_cputimers_work", file: !4034, line: 68, size: 448, elements: !4046)
!4046 = !{!4047, !4048, !4049}
!4047 = !DIDerivedType(tag: DW_TAG_member, name: "work", scope: !4045, file: !4034, line: 69, baseType: !129, size: 128, align: 64)
!4048 = !DIDerivedType(tag: DW_TAG_member, name: "mutex", scope: !4045, file: !4034, line: 70, baseType: !1277, size: 256, offset: 128)
!4049 = !DIDerivedType(tag: DW_TAG_member, name: "scheduled", scope: !4045, file: !4034, line: 71, baseType: !7, size: 32, offset: 384)
!4050 = !DIDerivedType(tag: DW_TAG_member, name: "ptracer_cred", scope: !3629, file: !1435, line: 1108, baseType: !490, size: 64, offset: 14464)
!4051 = !DIDerivedType(tag: DW_TAG_member, name: "real_cred", scope: !3629, file: !1435, line: 1111, baseType: !490, size: 64, offset: 14528)
!4052 = !DIDerivedType(tag: DW_TAG_member, name: "cred", scope: !3629, file: !1435, line: 1114, baseType: !490, size: 64, offset: 14592)
!4053 = !DIDerivedType(tag: DW_TAG_member, name: "cached_requested_key", scope: !3629, file: !1435, line: 1118, baseType: !528, size: 64, offset: 14656)
!4054 = !DIDerivedType(tag: DW_TAG_member, name: "comm", scope: !3629, file: !1435, line: 1128, baseType: !4055, size: 128, offset: 14720)
!4055 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 128, elements: !4056)
!4056 = !{!4057}
!4057 = !DISubrange(count: 16)
!4058 = !DIDerivedType(tag: DW_TAG_member, name: "nameidata", scope: !3629, file: !1435, line: 1130, baseType: !4059, size: 64, offset: 14848)
!4059 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4060, size: 64)
!4060 = !DICompositeType(tag: DW_TAG_structure_type, name: "nameidata", file: !1435, line: 65, flags: DIFlagFwdDecl)
!4061 = !DIDerivedType(tag: DW_TAG_member, name: "sysvsem", scope: !3629, file: !1435, line: 1133, baseType: !4062, size: 64, offset: 14912)
!4062 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "sysv_sem", file: !4063, line: 7, size: 64, elements: !4064)
!4063 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/sem_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "875896245c6421e1467036eee1d8c509")
!4064 = !{!4065}
!4065 = !DIDerivedType(tag: DW_TAG_member, name: "undo_list", scope: !4062, file: !4063, line: 9, baseType: !4066, size: 64)
!4066 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4067, size: 64)
!4067 = !DICompositeType(tag: DW_TAG_structure_type, name: "sem_undo_list", file: !4063, line: 5, flags: DIFlagFwdDecl)
!4068 = !DIDerivedType(tag: DW_TAG_member, name: "sysvshm", scope: !3629, file: !1435, line: 1134, baseType: !4069, size: 128, offset: 14976)
!4069 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "sysv_shm", file: !4070, line: 13, size: 128, elements: !4071)
!4070 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/shm.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "d87ad9c7b5e326eb79f2c39bcba5660a")
!4071 = !{!4072}
!4072 = !DIDerivedType(tag: DW_TAG_member, name: "shm_clist", scope: !4069, file: !4070, line: 14, baseType: !117, size: 128)
!4073 = !DIDerivedType(tag: DW_TAG_member, name: "fs", scope: !3629, file: !1435, line: 1141, baseType: !4074, size: 64, offset: 15104)
!4074 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4075, size: 64)
!4075 = !DICompositeType(tag: DW_TAG_structure_type, name: "fs_struct", file: !1435, line: 60, flags: DIFlagFwdDecl)
!4076 = !DIDerivedType(tag: DW_TAG_member, name: "files", scope: !3629, file: !1435, line: 1144, baseType: !4077, size: 64, offset: 15168)
!4077 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4078, size: 64)
!4078 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "files_struct", file: !4079, line: 39, size: 5632, elements: !4080)
!4079 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/fdtable.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "9e53ca8332c568627b5298785865fbfd")
!4080 = !{!4081, !4082, !4083, !4084, !4095, !4096, !4097, !4098, !4099, !4100, !4101}
!4081 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !4078, file: !4079, line: 43, baseType: !69, size: 32)
!4082 = !DIDerivedType(tag: DW_TAG_member, name: "resize_in_progress", scope: !4078, file: !4079, line: 44, baseType: !614, size: 8, offset: 32)
!4083 = !DIDerivedType(tag: DW_TAG_member, name: "resize_wait", scope: !4078, file: !4079, line: 45, baseType: !74, size: 192, offset: 64)
!4084 = !DIDerivedType(tag: DW_TAG_member, name: "fdt", scope: !4078, file: !4079, line: 47, baseType: !4085, size: 64, offset: 256)
!4085 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4086, size: 64)
!4086 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "fdtable", file: !4079, line: 27, size: 448, elements: !4087)
!4087 = !{!4088, !4089, !4091, !4092, !4093, !4094}
!4088 = !DIDerivedType(tag: DW_TAG_member, name: "max_fds", scope: !4086, file: !4079, line: 28, baseType: !7, size: 32)
!4089 = !DIDerivedType(tag: DW_TAG_member, name: "fd", scope: !4086, file: !4079, line: 29, baseType: !4090, size: 64, offset: 64)
!4090 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !896, size: 64)
!4091 = !DIDerivedType(tag: DW_TAG_member, name: "close_on_exec", scope: !4086, file: !4079, line: 30, baseType: !1440, size: 64, offset: 128)
!4092 = !DIDerivedType(tag: DW_TAG_member, name: "open_fds", scope: !4086, file: !4079, line: 31, baseType: !1440, size: 64, offset: 192)
!4093 = !DIDerivedType(tag: DW_TAG_member, name: "full_fds_bits", scope: !4086, file: !4079, line: 32, baseType: !1440, size: 64, offset: 256)
!4094 = !DIDerivedType(tag: DW_TAG_member, name: "rcu", scope: !4086, file: !4079, line: 33, baseType: !129, size: 128, align: 64, offset: 320)
!4095 = !DIDerivedType(tag: DW_TAG_member, name: "fdtab", scope: !4078, file: !4079, line: 48, baseType: !4086, size: 448, offset: 320)
!4096 = !DIDerivedType(tag: DW_TAG_member, name: "file_lock", scope: !4078, file: !4079, line: 52, baseType: !79, size: 32, align: 512, offset: 1024)
!4097 = !DIDerivedType(tag: DW_TAG_member, name: "next_fd", scope: !4078, file: !4079, line: 53, baseType: !7, size: 32, offset: 1056)
!4098 = !DIDerivedType(tag: DW_TAG_member, name: "close_on_exec_init", scope: !4078, file: !4079, line: 54, baseType: !3816, size: 64, offset: 1088)
!4099 = !DIDerivedType(tag: DW_TAG_member, name: "open_fds_init", scope: !4078, file: !4079, line: 55, baseType: !3816, size: 64, offset: 1152)
!4100 = !DIDerivedType(tag: DW_TAG_member, name: "full_fds_bits_init", scope: !4078, file: !4079, line: 56, baseType: !3816, size: 64, offset: 1216)
!4101 = !DIDerivedType(tag: DW_TAG_member, name: "fd_array", scope: !4078, file: !4079, line: 57, baseType: !4102, size: 4096, offset: 1280)
!4102 = !DICompositeType(tag: DW_TAG_array_type, baseType: !896, size: 4096, elements: !966)
!4103 = !DIDerivedType(tag: DW_TAG_member, name: "io_uring", scope: !3629, file: !1435, line: 1147, baseType: !4104, size: 64, offset: 15232)
!4104 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4105, size: 64)
!4105 = !DICompositeType(tag: DW_TAG_structure_type, name: "io_uring_task", file: !1435, line: 63, flags: DIFlagFwdDecl)
!4106 = !DIDerivedType(tag: DW_TAG_member, name: "nsproxy", scope: !3629, file: !1435, line: 1151, baseType: !4107, size: 64, offset: 15296)
!4107 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4108, size: 64)
!4108 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "nsproxy", file: !4109, line: 32, size: 576, elements: !4110)
!4109 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/nsproxy.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "8f32d31547745b266f91897a79f572e7")
!4110 = !{!4111, !4112, !4115, !4118, !4121, !4122, !4125, !4128, !4129}
!4111 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !4108, file: !4109, line: 33, baseType: !533, size: 32)
!4112 = !DIDerivedType(tag: DW_TAG_member, name: "uts_ns", scope: !4108, file: !4109, line: 34, baseType: !4113, size: 64, offset: 64)
!4113 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4114, size: 64)
!4114 = !DICompositeType(tag: DW_TAG_structure_type, name: "uts_namespace", file: !4109, line: 10, flags: DIFlagFwdDecl)
!4115 = !DIDerivedType(tag: DW_TAG_member, name: "ipc_ns", scope: !4108, file: !4109, line: 35, baseType: !4116, size: 64, offset: 128)
!4116 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4117, size: 64)
!4117 = !DICompositeType(tag: DW_TAG_structure_type, name: "ipc_namespace", file: !4109, line: 11, flags: DIFlagFwdDecl)
!4118 = !DIDerivedType(tag: DW_TAG_member, name: "mnt_ns", scope: !4108, file: !4109, line: 36, baseType: !4119, size: 64, offset: 192)
!4119 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4120, size: 64)
!4120 = !DICompositeType(tag: DW_TAG_structure_type, name: "mnt_namespace", file: !4109, line: 9, flags: DIFlagFwdDecl)
!4121 = !DIDerivedType(tag: DW_TAG_member, name: "pid_ns_for_children", scope: !4108, file: !4109, line: 37, baseType: !3969, size: 64, offset: 256)
!4122 = !DIDerivedType(tag: DW_TAG_member, name: "net_ns", scope: !4108, file: !4109, line: 38, baseType: !4123, size: 64, offset: 320)
!4123 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4124, size: 64)
!4124 = !DICompositeType(tag: DW_TAG_structure_type, name: "net", file: !530, line: 34, flags: DIFlagFwdDecl)
!4125 = !DIDerivedType(tag: DW_TAG_member, name: "time_ns", scope: !4108, file: !4109, line: 39, baseType: !4126, size: 64, offset: 384)
!4126 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4127, size: 64)
!4127 = !DICompositeType(tag: DW_TAG_structure_type, name: "time_namespace", file: !4109, line: 39, flags: DIFlagFwdDecl)
!4128 = !DIDerivedType(tag: DW_TAG_member, name: "time_ns_for_children", scope: !4108, file: !4109, line: 40, baseType: !4126, size: 64, offset: 448)
!4129 = !DIDerivedType(tag: DW_TAG_member, name: "cgroup_ns", scope: !4108, file: !4109, line: 41, baseType: !4130, size: 64, offset: 512)
!4130 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4131, size: 64)
!4131 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cgroup_namespace", file: !4132, line: 769, size: 384, elements: !4133)
!4132 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/cgroup.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "12817dfe0eaecd41341d07df7e9444c0")
!4133 = !{!4134, !4135, !4136, !4137}
!4134 = !DIDerivedType(tag: DW_TAG_member, name: "ns", scope: !4131, file: !4132, line: 770, baseType: !736, size: 192)
!4135 = !DIDerivedType(tag: DW_TAG_member, name: "user_ns", scope: !4131, file: !4132, line: 771, baseType: !700, size: 64, offset: 192)
!4136 = !DIDerivedType(tag: DW_TAG_member, name: "ucounts", scope: !4131, file: !4132, line: 772, baseType: !3994, size: 64, offset: 256)
!4137 = !DIDerivedType(tag: DW_TAG_member, name: "root_cset", scope: !4131, file: !4132, line: 773, baseType: !4138, size: 64, offset: 320)
!4138 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4139, size: 64)
!4139 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "css_set", file: !4140, line: 234, size: 4736, elements: !4141)
!4140 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/cgroup-defs.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "5eff08e9aabaa2b129a830ada3937f40")
!4141 = !{!4142, !4380, !4381, !4382, !4383, !4384, !4385, !4386, !4387, !4388, !4389, !4390, !4391, !4392, !4393, !4394, !4395, !4396, !4397, !4398, !4399, !4400}
!4142 = !DIDerivedType(tag: DW_TAG_member, name: "subsys", scope: !4139, file: !4140, line: 240, baseType: !4143, size: 896)
!4143 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4144, size: 896, elements: !4181)
!4144 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4145, size: 64)
!4145 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cgroup_subsys_state", file: !4140, line: 165, size: 1664, elements: !4146)
!4146 = !{!4147, !4259, !4363, !4364, !4365, !4366, !4367, !4368, !4369, !4370, !4371, !4372, !4378, !4379}
!4147 = !DIDerivedType(tag: DW_TAG_member, name: "cgroup", scope: !4145, file: !4140, line: 167, baseType: !4148, size: 64)
!4148 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4149, size: 64)
!4149 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cgroup", file: !4140, line: 414, size: 9216, elements: !4150)
!4150 = !{!4151, !4152, !4153, !4154, !4155, !4156, !4157, !4158, !4159, !4160, !4161, !4162, !4163, !4169, !4170, !4174, !4175, !4176, !4177, !4178, !4179, !4183, !4201, !4202, !4204, !4205, !4206, !4228, !4229, !4235, !4236, !4237, !4238, !4239, !4240, !4241, !4242, !4243, !4247, !4250, !4257}
!4151 = !DIDerivedType(tag: DW_TAG_member, name: "self", scope: !4149, file: !4140, line: 416, baseType: !4145, size: 1664)
!4152 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !4149, file: !4140, line: 418, baseType: !59, size: 64, offset: 1664)
!4153 = !DIDerivedType(tag: DW_TAG_member, name: "level", scope: !4149, file: !4140, line: 426, baseType: !42, size: 32, offset: 1728)
!4154 = !DIDerivedType(tag: DW_TAG_member, name: "max_depth", scope: !4149, file: !4140, line: 429, baseType: !42, size: 32, offset: 1760)
!4155 = !DIDerivedType(tag: DW_TAG_member, name: "nr_descendants", scope: !4149, file: !4140, line: 442, baseType: !42, size: 32, offset: 1792)
!4156 = !DIDerivedType(tag: DW_TAG_member, name: "nr_dying_descendants", scope: !4149, file: !4140, line: 443, baseType: !42, size: 32, offset: 1824)
!4157 = !DIDerivedType(tag: DW_TAG_member, name: "max_descendants", scope: !4149, file: !4140, line: 444, baseType: !42, size: 32, offset: 1856)
!4158 = !DIDerivedType(tag: DW_TAG_member, name: "nr_populated_csets", scope: !4149, file: !4140, line: 457, baseType: !42, size: 32, offset: 1888)
!4159 = !DIDerivedType(tag: DW_TAG_member, name: "nr_populated_domain_children", scope: !4149, file: !4140, line: 458, baseType: !42, size: 32, offset: 1920)
!4160 = !DIDerivedType(tag: DW_TAG_member, name: "nr_populated_threaded_children", scope: !4149, file: !4140, line: 459, baseType: !42, size: 32, offset: 1952)
!4161 = !DIDerivedType(tag: DW_TAG_member, name: "nr_threaded_children", scope: !4149, file: !4140, line: 461, baseType: !42, size: 32, offset: 1984)
!4162 = !DIDerivedType(tag: DW_TAG_member, name: "kn", scope: !4149, file: !4140, line: 463, baseType: !2407, size: 64, offset: 2048)
!4163 = !DIDerivedType(tag: DW_TAG_member, name: "procs_file", scope: !4149, file: !4140, line: 464, baseType: !4164, size: 448, offset: 2112)
!4164 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cgroup_file", file: !4140, line: 151, size: 448, elements: !4165)
!4165 = !{!4166, !4167, !4168}
!4166 = !DIDerivedType(tag: DW_TAG_member, name: "kn", scope: !4164, file: !4140, line: 153, baseType: !2407, size: 64)
!4167 = !DIDerivedType(tag: DW_TAG_member, name: "notified_at", scope: !4164, file: !4140, line: 154, baseType: !59, size: 64, offset: 64)
!4168 = !DIDerivedType(tag: DW_TAG_member, name: "notify_timer", scope: !4164, file: !4140, line: 155, baseType: !2070, size: 320, offset: 128)
!4169 = !DIDerivedType(tag: DW_TAG_member, name: "events_file", scope: !4149, file: !4140, line: 465, baseType: !4164, size: 448, offset: 2560)
!4170 = !DIDerivedType(tag: DW_TAG_member, name: "psi_files", scope: !4149, file: !4140, line: 468, baseType: !4171, offset: 3008)
!4171 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4164, elements: !4172)
!4172 = !{!4173}
!4173 = !DISubrange(count: 0)
!4174 = !DIDerivedType(tag: DW_TAG_member, name: "subtree_control", scope: !4149, file: !4140, line: 477, baseType: !113, size: 16, offset: 3008)
!4175 = !DIDerivedType(tag: DW_TAG_member, name: "subtree_ss_mask", scope: !4149, file: !4140, line: 478, baseType: !113, size: 16, offset: 3024)
!4176 = !DIDerivedType(tag: DW_TAG_member, name: "old_subtree_control", scope: !4149, file: !4140, line: 479, baseType: !113, size: 16, offset: 3040)
!4177 = !DIDerivedType(tag: DW_TAG_member, name: "old_subtree_ss_mask", scope: !4149, file: !4140, line: 480, baseType: !113, size: 16, offset: 3056)
!4178 = !DIDerivedType(tag: DW_TAG_member, name: "subsys", scope: !4149, file: !4140, line: 483, baseType: !4143, size: 896, offset: 3072)
!4179 = !DIDerivedType(tag: DW_TAG_member, name: "nr_dying_subsys", scope: !4149, file: !4140, line: 489, baseType: !4180, size: 448, offset: 3968)
!4180 = !DICompositeType(tag: DW_TAG_array_type, baseType: !42, size: 448, elements: !4181)
!4181 = !{!4182}
!4182 = !DISubrange(count: 14)
!4183 = !DIDerivedType(tag: DW_TAG_member, name: "root", scope: !4149, file: !4140, line: 491, baseType: !4184, size: 64, offset: 4416)
!4184 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4185, size: 64)
!4185 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cgroup_root", file: !4140, line: 576, size: 43520, elements: !4186)
!4186 = !{!4187, !4188, !4189, !4190, !4191, !4192, !4193, !4194, !4195, !4196, !4200}
!4187 = !DIDerivedType(tag: DW_TAG_member, name: "kf_root", scope: !4185, file: !4140, line: 577, baseType: !2429, size: 64)
!4188 = !DIDerivedType(tag: DW_TAG_member, name: "subsys_mask", scope: !4185, file: !4140, line: 580, baseType: !7, size: 32, offset: 64)
!4189 = !DIDerivedType(tag: DW_TAG_member, name: "hierarchy_id", scope: !4185, file: !4140, line: 583, baseType: !42, size: 32, offset: 96)
!4190 = !DIDerivedType(tag: DW_TAG_member, name: "root_list", scope: !4185, file: !4140, line: 586, baseType: !117, size: 128, offset: 128)
!4191 = !DIDerivedType(tag: DW_TAG_member, name: "rcu", scope: !4185, file: !4140, line: 587, baseType: !129, size: 128, align: 64, offset: 256)
!4192 = !DIDerivedType(tag: DW_TAG_member, name: "cgrp", scope: !4185, file: !4140, line: 594, baseType: !4149, size: 9216, offset: 512)
!4193 = !DIDerivedType(tag: DW_TAG_member, name: "cgrp_ancestor_storage", scope: !4185, file: !4140, line: 597, baseType: !4148, size: 64, offset: 9728)
!4194 = !DIDerivedType(tag: DW_TAG_member, name: "nr_cgrps", scope: !4185, file: !4140, line: 600, baseType: !69, size: 32, offset: 9792)
!4195 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !4185, file: !4140, line: 603, baseType: !7, size: 32, offset: 9824)
!4196 = !DIDerivedType(tag: DW_TAG_member, name: "release_agent_path", scope: !4185, file: !4140, line: 606, baseType: !4197, size: 32768, offset: 9856)
!4197 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 32768, elements: !4198)
!4198 = !{!4199}
!4199 = !DISubrange(count: 4096)
!4200 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !4185, file: !4140, line: 609, baseType: !3553, size: 512, offset: 42624)
!4201 = !DIDerivedType(tag: DW_TAG_member, name: "cset_links", scope: !4149, file: !4140, line: 497, baseType: !117, size: 128, offset: 4480)
!4202 = !DIDerivedType(tag: DW_TAG_member, name: "e_csets", scope: !4149, file: !4140, line: 506, baseType: !4203, size: 1792, offset: 4608)
!4203 = !DICompositeType(tag: DW_TAG_array_type, baseType: !117, size: 1792, elements: !4181)
!4204 = !DIDerivedType(tag: DW_TAG_member, name: "dom_cgrp", scope: !4149, file: !4140, line: 515, baseType: !4148, size: 64, offset: 6400)
!4205 = !DIDerivedType(tag: DW_TAG_member, name: "old_dom_cgrp", scope: !4149, file: !4140, line: 516, baseType: !4148, size: 64, offset: 6464)
!4206 = !DIDerivedType(tag: DW_TAG_member, name: "rstat_cpu", scope: !4149, file: !4140, line: 519, baseType: !4207, size: 64, offset: 6528)
!4207 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4208, size: 64)
!4208 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cgroup_rstat_cpu", file: !4140, line: 352, size: 896, elements: !4209)
!4209 = !{!4210, !4213, !4223, !4224, !4225, !4226, !4227}
!4210 = !DIDerivedType(tag: DW_TAG_member, name: "bsync", scope: !4208, file: !4140, line: 357, baseType: !4211)
!4211 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "u64_stats_sync", file: !4212, line: 64, elements: !1201)
!4212 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/u64_stats_sync.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "3da4ff7bace7f53a3466d26e2eb40342")
!4213 = !DIDerivedType(tag: DW_TAG_member, name: "bstat", scope: !4208, file: !4140, line: 358, baseType: !4214, size: 192)
!4214 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cgroup_base_stat", file: !4140, line: 324, size: 192, elements: !4215)
!4215 = !{!4216}
!4216 = !DIDerivedType(tag: DW_TAG_member, name: "cputime", scope: !4214, file: !4140, line: 325, baseType: !4217, size: 192)
!4217 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "task_cputime", file: !4218, line: 17, size: 192, elements: !4219)
!4218 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/sched/types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "d1c674001293f15f4be48357be6a131f")
!4219 = !{!4220, !4221, !4222}
!4220 = !DIDerivedType(tag: DW_TAG_member, name: "stime", scope: !4217, file: !4218, line: 18, baseType: !519, size: 64)
!4221 = !DIDerivedType(tag: DW_TAG_member, name: "utime", scope: !4217, file: !4218, line: 19, baseType: !519, size: 64, offset: 64)
!4222 = !DIDerivedType(tag: DW_TAG_member, name: "sum_exec_runtime", scope: !4217, file: !4218, line: 20, baseType: !521, size: 64, offset: 128)
!4223 = !DIDerivedType(tag: DW_TAG_member, name: "last_bstat", scope: !4208, file: !4140, line: 364, baseType: !4214, size: 192, offset: 192)
!4224 = !DIDerivedType(tag: DW_TAG_member, name: "subtree_bstat", scope: !4208, file: !4140, line: 372, baseType: !4214, size: 192, offset: 384)
!4225 = !DIDerivedType(tag: DW_TAG_member, name: "last_subtree_bstat", scope: !4208, file: !4140, line: 378, baseType: !4214, size: 192, offset: 576)
!4226 = !DIDerivedType(tag: DW_TAG_member, name: "updated_children", scope: !4208, file: !4140, line: 391, baseType: !4148, size: 64, offset: 768)
!4227 = !DIDerivedType(tag: DW_TAG_member, name: "updated_next", scope: !4208, file: !4140, line: 392, baseType: !4148, size: 64, offset: 832)
!4228 = !DIDerivedType(tag: DW_TAG_member, name: "rstat_css_list", scope: !4149, file: !4140, line: 520, baseType: !117, size: 128, offset: 6592)
!4229 = !DIDerivedType(tag: DW_TAG_member, name: "_pad_", scope: !4149, file: !4140, line: 527, baseType: !4230, align: 512, offset: 7168)
!4230 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cacheline_padding", file: !4231, line: 177, align: 512, elements: !4232)
!4231 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/cache.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "ae227c6fac452bfaabdd20bfbbdeab2c")
!4232 = !{!4233}
!4233 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !4230, file: !4231, line: 178, baseType: !4234)
!4234 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, elements: !4172)
!4235 = !DIDerivedType(tag: DW_TAG_member, name: "rstat_flush_next", scope: !4149, file: !4140, line: 534, baseType: !4148, size: 64, offset: 7168)
!4236 = !DIDerivedType(tag: DW_TAG_member, name: "last_bstat", scope: !4149, file: !4140, line: 537, baseType: !4214, size: 192, offset: 7232)
!4237 = !DIDerivedType(tag: DW_TAG_member, name: "bstat", scope: !4149, file: !4140, line: 538, baseType: !4214, size: 192, offset: 7424)
!4238 = !DIDerivedType(tag: DW_TAG_member, name: "prev_cputime", scope: !4149, file: !4140, line: 539, baseType: !4021, size: 192, offset: 7616)
!4239 = !DIDerivedType(tag: DW_TAG_member, name: "pidlists", scope: !4149, file: !4140, line: 545, baseType: !117, size: 128, offset: 7808)
!4240 = !DIDerivedType(tag: DW_TAG_member, name: "pidlist_mutex", scope: !4149, file: !4140, line: 546, baseType: !1277, size: 256, offset: 7936)
!4241 = !DIDerivedType(tag: DW_TAG_member, name: "offline_waitq", scope: !4149, file: !4140, line: 549, baseType: !74, size: 192, offset: 8192)
!4242 = !DIDerivedType(tag: DW_TAG_member, name: "release_agent_work", scope: !4149, file: !4140, line: 552, baseType: !1337, size: 256, offset: 8384)
!4243 = !DIDerivedType(tag: DW_TAG_member, name: "psi", scope: !4149, file: !4140, line: 555, baseType: !4244, size: 64, offset: 8640)
!4244 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4245, size: 64)
!4245 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "psi_group", file: !4246, line: 214, elements: !1201)
!4246 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/psi_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "a3153bfbcc4370f75208e4d12aca55f1")
!4247 = !DIDerivedType(tag: DW_TAG_member, name: "bpf", scope: !4149, file: !4140, line: 558, baseType: !4248, offset: 8704)
!4248 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cgroup_bpf", file: !4249, line: 81, elements: !1201)
!4249 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/bpf-cgroup-defs.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "7d774d8633eef98595a27b77eafab0a5")
!4250 = !DIDerivedType(tag: DW_TAG_member, name: "freezer", scope: !4149, file: !4140, line: 561, baseType: !4251, size: 128, offset: 8704)
!4251 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cgroup_freezer_state", file: !4140, line: 395, size: 128, elements: !4252)
!4252 = !{!4253, !4254, !4255, !4256}
!4253 = !DIDerivedType(tag: DW_TAG_member, name: "freeze", scope: !4251, file: !4140, line: 397, baseType: !614, size: 8)
!4254 = !DIDerivedType(tag: DW_TAG_member, name: "e_freeze", scope: !4251, file: !4140, line: 400, baseType: !42, size: 32, offset: 32)
!4255 = !DIDerivedType(tag: DW_TAG_member, name: "nr_frozen_descendants", scope: !4251, file: !4140, line: 405, baseType: !42, size: 32, offset: 64)
!4256 = !DIDerivedType(tag: DW_TAG_member, name: "nr_frozen_tasks", scope: !4251, file: !4140, line: 411, baseType: !42, size: 32, offset: 96)
!4257 = !DIDerivedType(tag: DW_TAG_member, name: "ancestors", scope: !4149, file: !4140, line: 568, baseType: !4258, offset: 8832)
!4258 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4148, elements: !1353)
!4259 = !DIDerivedType(tag: DW_TAG_member, name: "ss", scope: !4145, file: !4140, line: 170, baseType: !4260, size: 64, offset: 64)
!4260 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4261, size: 64)
!4261 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cgroup_subsys", file: !4140, line: 706, size: 1984, elements: !4262)
!4262 = !{!4263, !4267, !4271, !4275, !4276, !4277, !4278, !4282, !4286, !4287, !4293, !4297, !4298, !4299, !4303, !4307, !4311, !4312, !4313, !4314, !4315, !4316, !4317, !4318, !4319, !4320, !4321, !4322, !4323, !4361, !4362}
!4263 = !DIDerivedType(tag: DW_TAG_member, name: "css_alloc", scope: !4261, file: !4140, line: 707, baseType: !4264, size: 64)
!4264 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4265, size: 64)
!4265 = !DISubroutineType(types: !4266)
!4266 = !{!4144, !4144}
!4267 = !DIDerivedType(tag: DW_TAG_member, name: "css_online", scope: !4261, file: !4140, line: 708, baseType: !4268, size: 64, offset: 64)
!4268 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4269, size: 64)
!4269 = !DISubroutineType(types: !4270)
!4270 = !{!42, !4144}
!4271 = !DIDerivedType(tag: DW_TAG_member, name: "css_offline", scope: !4261, file: !4140, line: 709, baseType: !4272, size: 64, offset: 128)
!4272 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4273, size: 64)
!4273 = !DISubroutineType(types: !4274)
!4274 = !{null, !4144}
!4275 = !DIDerivedType(tag: DW_TAG_member, name: "css_released", scope: !4261, file: !4140, line: 710, baseType: !4272, size: 64, offset: 192)
!4276 = !DIDerivedType(tag: DW_TAG_member, name: "css_free", scope: !4261, file: !4140, line: 711, baseType: !4272, size: 64, offset: 256)
!4277 = !DIDerivedType(tag: DW_TAG_member, name: "css_reset", scope: !4261, file: !4140, line: 712, baseType: !4272, size: 64, offset: 320)
!4278 = !DIDerivedType(tag: DW_TAG_member, name: "css_rstat_flush", scope: !4261, file: !4140, line: 713, baseType: !4279, size: 64, offset: 384)
!4279 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4280, size: 64)
!4280 = !DISubroutineType(types: !4281)
!4281 = !{null, !4144, !42}
!4282 = !DIDerivedType(tag: DW_TAG_member, name: "css_extra_stat_show", scope: !4261, file: !4140, line: 714, baseType: !4283, size: 64, offset: 448)
!4283 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4284, size: 64)
!4284 = !DISubroutineType(types: !4285)
!4285 = !{!42, !2454, !4144}
!4286 = !DIDerivedType(tag: DW_TAG_member, name: "css_local_stat_show", scope: !4261, file: !4140, line: 716, baseType: !4283, size: 64, offset: 512)
!4287 = !DIDerivedType(tag: DW_TAG_member, name: "can_attach", scope: !4261, file: !4140, line: 719, baseType: !4288, size: 64, offset: 576)
!4288 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4289, size: 64)
!4289 = !DISubroutineType(types: !4290)
!4290 = !{!42, !4291}
!4291 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4292, size: 64)
!4292 = !DICompositeType(tag: DW_TAG_structure_type, name: "cgroup_taskset", file: !4140, line: 30, flags: DIFlagFwdDecl)
!4293 = !DIDerivedType(tag: DW_TAG_member, name: "cancel_attach", scope: !4261, file: !4140, line: 720, baseType: !4294, size: 64, offset: 640)
!4294 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4295, size: 64)
!4295 = !DISubroutineType(types: !4296)
!4296 = !{null, !4291}
!4297 = !DIDerivedType(tag: DW_TAG_member, name: "attach", scope: !4261, file: !4140, line: 721, baseType: !4294, size: 64, offset: 704)
!4298 = !DIDerivedType(tag: DW_TAG_member, name: "post_attach", scope: !4261, file: !4140, line: 722, baseType: !2886, size: 64, offset: 768)
!4299 = !DIDerivedType(tag: DW_TAG_member, name: "can_fork", scope: !4261, file: !4140, line: 723, baseType: !4300, size: 64, offset: 832)
!4300 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4301, size: 64)
!4301 = !DISubroutineType(types: !4302)
!4302 = !{!42, !3628, !4138}
!4303 = !DIDerivedType(tag: DW_TAG_member, name: "cancel_fork", scope: !4261, file: !4140, line: 725, baseType: !4304, size: 64, offset: 896)
!4304 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4305, size: 64)
!4305 = !DISubroutineType(types: !4306)
!4306 = !{null, !3628, !4138}
!4307 = !DIDerivedType(tag: DW_TAG_member, name: "fork", scope: !4261, file: !4140, line: 726, baseType: !4308, size: 64, offset: 960)
!4308 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4309, size: 64)
!4309 = !DISubroutineType(types: !4310)
!4310 = !{null, !3628}
!4311 = !DIDerivedType(tag: DW_TAG_member, name: "exit", scope: !4261, file: !4140, line: 727, baseType: !4308, size: 64, offset: 1024)
!4312 = !DIDerivedType(tag: DW_TAG_member, name: "release", scope: !4261, file: !4140, line: 728, baseType: !4308, size: 64, offset: 1088)
!4313 = !DIDerivedType(tag: DW_TAG_member, name: "bind", scope: !4261, file: !4140, line: 729, baseType: !4272, size: 64, offset: 1152)
!4314 = !DIDerivedType(tag: DW_TAG_member, name: "early_init", scope: !4261, file: !4140, line: 731, baseType: !614, size: 1, offset: 1216, flags: DIFlagBitField, extraData: i64 1216)
!4315 = !DIDerivedType(tag: DW_TAG_member, name: "implicit_on_dfl", scope: !4261, file: !4140, line: 744, baseType: !614, size: 1, offset: 1217, flags: DIFlagBitField, extraData: i64 1216)
!4316 = !DIDerivedType(tag: DW_TAG_member, name: "threaded", scope: !4261, file: !4140, line: 756, baseType: !614, size: 1, offset: 1218, flags: DIFlagBitField, extraData: i64 1216)
!4317 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !4261, file: !4140, line: 759, baseType: !42, size: 32, offset: 1248)
!4318 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !4261, file: !4140, line: 760, baseType: !36, size: 64, offset: 1280)
!4319 = !DIDerivedType(tag: DW_TAG_member, name: "legacy_name", scope: !4261, file: !4140, line: 763, baseType: !36, size: 64, offset: 1344)
!4320 = !DIDerivedType(tag: DW_TAG_member, name: "root", scope: !4261, file: !4140, line: 766, baseType: !4184, size: 64, offset: 1408)
!4321 = !DIDerivedType(tag: DW_TAG_member, name: "css_idr", scope: !4261, file: !4140, line: 769, baseType: !3974, size: 192, offset: 1472)
!4322 = !DIDerivedType(tag: DW_TAG_member, name: "cfts", scope: !4261, file: !4140, line: 775, baseType: !117, size: 128, offset: 1664)
!4323 = !DIDerivedType(tag: DW_TAG_member, name: "dfl_cftypes", scope: !4261, file: !4140, line: 781, baseType: !4324, size: 64, offset: 1792)
!4324 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4325, size: 64)
!4325 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cftype", file: !4140, line: 619, size: 1728, elements: !4326)
!4326 = !{!4327, !4328, !4329, !4330, !4331, !4332, !4333, !4334, !4336, !4337, !4338, !4342, !4346, !4347, !4348, !4349, !4350, !4354, !4358, !4359, !4360}
!4327 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !4325, file: !4140, line: 625, baseType: !3553, size: 512)
!4328 = !DIDerivedType(tag: DW_TAG_member, name: "private", scope: !4325, file: !4140, line: 626, baseType: !59, size: 64, offset: 512)
!4329 = !DIDerivedType(tag: DW_TAG_member, name: "max_write_len", scope: !4325, file: !4140, line: 632, baseType: !55, size: 64, offset: 576)
!4330 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !4325, file: !4140, line: 635, baseType: !7, size: 32, offset: 640)
!4331 = !DIDerivedType(tag: DW_TAG_member, name: "file_offset", scope: !4325, file: !4140, line: 643, baseType: !7, size: 32, offset: 672)
!4332 = !DIDerivedType(tag: DW_TAG_member, name: "ss", scope: !4325, file: !4140, line: 649, baseType: !4260, size: 64, offset: 704)
!4333 = !DIDerivedType(tag: DW_TAG_member, name: "node", scope: !4325, file: !4140, line: 650, baseType: !117, size: 128, offset: 768)
!4334 = !DIDerivedType(tag: DW_TAG_member, name: "kf_ops", scope: !4325, file: !4140, line: 651, baseType: !4335, size: 64, offset: 896)
!4335 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2442, size: 64)
!4336 = !DIDerivedType(tag: DW_TAG_member, name: "open", scope: !4325, file: !4140, line: 653, baseType: !2445, size: 64, offset: 960)
!4337 = !DIDerivedType(tag: DW_TAG_member, name: "release", scope: !4325, file: !4140, line: 654, baseType: !2503, size: 64, offset: 1024)
!4338 = !DIDerivedType(tag: DW_TAG_member, name: "read_u64", scope: !4325, file: !4140, line: 660, baseType: !4339, size: 64, offset: 1088)
!4339 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4340, size: 64)
!4340 = !DISubroutineType(types: !4341)
!4341 = !{!519, !4144, !4324}
!4342 = !DIDerivedType(tag: DW_TAG_member, name: "read_s64", scope: !4325, file: !4140, line: 664, baseType: !4343, size: 64, offset: 1152)
!4343 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4344, size: 64)
!4344 = !DISubroutineType(types: !4345)
!4345 = !{!502, !4144, !4324}
!4346 = !DIDerivedType(tag: DW_TAG_member, name: "seq_show", scope: !4325, file: !4140, line: 667, baseType: !2484, size: 64, offset: 1216)
!4347 = !DIDerivedType(tag: DW_TAG_member, name: "seq_start", scope: !4325, file: !4140, line: 670, baseType: !2472, size: 64, offset: 1280)
!4348 = !DIDerivedType(tag: DW_TAG_member, name: "seq_next", scope: !4325, file: !4140, line: 671, baseType: !2480, size: 64, offset: 1344)
!4349 = !DIDerivedType(tag: DW_TAG_member, name: "seq_stop", scope: !4325, file: !4140, line: 672, baseType: !2476, size: 64, offset: 1408)
!4350 = !DIDerivedType(tag: DW_TAG_member, name: "write_u64", scope: !4325, file: !4140, line: 679, baseType: !4351, size: 64, offset: 1472)
!4351 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4352, size: 64)
!4352 = !DISubroutineType(types: !4353)
!4353 = !{!42, !4144, !4324, !519}
!4354 = !DIDerivedType(tag: DW_TAG_member, name: "write_s64", scope: !4325, file: !4140, line: 684, baseType: !4355, size: 64, offset: 1536)
!4355 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4356, size: 64)
!4356 = !DISubroutineType(types: !4357)
!4357 = !{!42, !4144, !4324, !502}
!4358 = !DIDerivedType(tag: DW_TAG_member, name: "write", scope: !4325, file: !4140, line: 693, baseType: !2511, size: 64, offset: 1600)
!4359 = !DIDerivedType(tag: DW_TAG_member, name: "poll", scope: !4325, file: !4140, line: 696, baseType: !2518, size: 64, offset: 1664)
!4360 = !DIDerivedType(tag: DW_TAG_member, name: "lockdep_key", scope: !4325, file: !4140, line: 699, baseType: !3208, offset: 1728)
!4361 = !DIDerivedType(tag: DW_TAG_member, name: "legacy_cftypes", scope: !4261, file: !4140, line: 782, baseType: !4324, size: 64, offset: 1856)
!4362 = !DIDerivedType(tag: DW_TAG_member, name: "depends_on", scope: !4261, file: !4140, line: 791, baseType: !7, size: 32, offset: 1920)
!4363 = !DIDerivedType(tag: DW_TAG_member, name: "refcnt", scope: !4145, file: !4140, line: 173, baseType: !1121, size: 128, offset: 128)
!4364 = !DIDerivedType(tag: DW_TAG_member, name: "sibling", scope: !4145, file: !4140, line: 180, baseType: !117, size: 128, offset: 256)
!4365 = !DIDerivedType(tag: DW_TAG_member, name: "children", scope: !4145, file: !4140, line: 181, baseType: !117, size: 128, offset: 384)
!4366 = !DIDerivedType(tag: DW_TAG_member, name: "rstat_css_node", scope: !4145, file: !4140, line: 184, baseType: !117, size: 128, offset: 512)
!4367 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !4145, file: !4140, line: 190, baseType: !42, size: 32, offset: 640)
!4368 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !4145, file: !4140, line: 192, baseType: !7, size: 32, offset: 672)
!4369 = !DIDerivedType(tag: DW_TAG_member, name: "serial_nr", scope: !4145, file: !4140, line: 200, baseType: !519, size: 64, offset: 704)
!4370 = !DIDerivedType(tag: DW_TAG_member, name: "online_cnt", scope: !4145, file: !4140, line: 206, baseType: !69, size: 32, offset: 768)
!4371 = !DIDerivedType(tag: DW_TAG_member, name: "destroy_work", scope: !4145, file: !4140, line: 209, baseType: !1337, size: 256, offset: 832)
!4372 = !DIDerivedType(tag: DW_TAG_member, name: "destroy_rwork", scope: !4145, file: !4140, line: 210, baseType: !4373, size: 448, offset: 1088)
!4373 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "rcu_work", file: !466, line: 122, size: 448, elements: !4374)
!4374 = !{!4375, !4376, !4377}
!4375 = !DIDerivedType(tag: DW_TAG_member, name: "work", scope: !4373, file: !466, line: 123, baseType: !1337, size: 256)
!4376 = !DIDerivedType(tag: DW_TAG_member, name: "rcu", scope: !4373, file: !466, line: 124, baseType: !129, size: 128, align: 64, offset: 256)
!4377 = !DIDerivedType(tag: DW_TAG_member, name: "wq", scope: !4373, file: !466, line: 127, baseType: !2845, size: 64, offset: 384)
!4378 = !DIDerivedType(tag: DW_TAG_member, name: "parent", scope: !4145, file: !4140, line: 216, baseType: !4144, size: 64, offset: 1536)
!4379 = !DIDerivedType(tag: DW_TAG_member, name: "nr_descendants", scope: !4145, file: !4140, line: 224, baseType: !42, size: 32, offset: 1600)
!4380 = !DIDerivedType(tag: DW_TAG_member, name: "refcount", scope: !4139, file: !4140, line: 243, baseType: !533, size: 32, offset: 896)
!4381 = !DIDerivedType(tag: DW_TAG_member, name: "dom_cset", scope: !4139, file: !4140, line: 251, baseType: !4138, size: 64, offset: 960)
!4382 = !DIDerivedType(tag: DW_TAG_member, name: "dfl_cgrp", scope: !4139, file: !4140, line: 254, baseType: !4148, size: 64, offset: 1024)
!4383 = !DIDerivedType(tag: DW_TAG_member, name: "nr_tasks", scope: !4139, file: !4140, line: 257, baseType: !42, size: 32, offset: 1088)
!4384 = !DIDerivedType(tag: DW_TAG_member, name: "tasks", scope: !4139, file: !4140, line: 266, baseType: !117, size: 128, offset: 1152)
!4385 = !DIDerivedType(tag: DW_TAG_member, name: "mg_tasks", scope: !4139, file: !4140, line: 267, baseType: !117, size: 128, offset: 1280)
!4386 = !DIDerivedType(tag: DW_TAG_member, name: "dying_tasks", scope: !4139, file: !4140, line: 268, baseType: !117, size: 128, offset: 1408)
!4387 = !DIDerivedType(tag: DW_TAG_member, name: "task_iters", scope: !4139, file: !4140, line: 271, baseType: !117, size: 128, offset: 1536)
!4388 = !DIDerivedType(tag: DW_TAG_member, name: "e_cset_node", scope: !4139, file: !4140, line: 280, baseType: !4203, size: 1792, offset: 1664)
!4389 = !DIDerivedType(tag: DW_TAG_member, name: "threaded_csets", scope: !4139, file: !4140, line: 283, baseType: !117, size: 128, offset: 3456)
!4390 = !DIDerivedType(tag: DW_TAG_member, name: "threaded_csets_node", scope: !4139, file: !4140, line: 284, baseType: !117, size: 128, offset: 3584)
!4391 = !DIDerivedType(tag: DW_TAG_member, name: "hlist", scope: !4139, file: !4140, line: 290, baseType: !220, size: 128, offset: 3712)
!4392 = !DIDerivedType(tag: DW_TAG_member, name: "cgrp_links", scope: !4139, file: !4140, line: 296, baseType: !117, size: 128, offset: 3840)
!4393 = !DIDerivedType(tag: DW_TAG_member, name: "mg_src_preload_node", scope: !4139, file: !4140, line: 302, baseType: !117, size: 128, offset: 3968)
!4394 = !DIDerivedType(tag: DW_TAG_member, name: "mg_dst_preload_node", scope: !4139, file: !4140, line: 303, baseType: !117, size: 128, offset: 4096)
!4395 = !DIDerivedType(tag: DW_TAG_member, name: "mg_node", scope: !4139, file: !4140, line: 304, baseType: !117, size: 128, offset: 4224)
!4396 = !DIDerivedType(tag: DW_TAG_member, name: "mg_src_cgrp", scope: !4139, file: !4140, line: 313, baseType: !4148, size: 64, offset: 4352)
!4397 = !DIDerivedType(tag: DW_TAG_member, name: "mg_dst_cgrp", scope: !4139, file: !4140, line: 314, baseType: !4148, size: 64, offset: 4416)
!4398 = !DIDerivedType(tag: DW_TAG_member, name: "mg_dst_cset", scope: !4139, file: !4140, line: 315, baseType: !4138, size: 64, offset: 4480)
!4399 = !DIDerivedType(tag: DW_TAG_member, name: "dead", scope: !4139, file: !4140, line: 318, baseType: !614, size: 8, offset: 4544)
!4400 = !DIDerivedType(tag: DW_TAG_member, name: "callback_head", scope: !4139, file: !4140, line: 321, baseType: !129, size: 128, align: 64, offset: 4608)
!4401 = !DIDerivedType(tag: DW_TAG_member, name: "signal", scope: !3629, file: !1435, line: 1154, baseType: !4402, size: 64, offset: 15360)
!4402 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4403, size: 64)
!4403 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "signal_struct", file: !4404, line: 94, size: 8704, elements: !4405)
!4404 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/sched/signal.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "beefa6dad20f483f16328258137a3325")
!4405 = !{!4406, !4407, !4408, !4409, !4410, !4411, !4412, !4413, !4424, !4425, !4426, !4427, !4428, !4429, !4430, !4442, !4443, !4444, !4445, !4446, !4447, !4448, !4454, !4463, !4464, !4466, !4467, !4468, !4471, !4477, !4478, !4479, !4480, !4481, !4482, !4483, !4484, !4485, !4486, !4487, !4488, !4489, !4490, !4491, !4492, !4493, !4494, !4495, !4496, !4497, !4498, !4509, !4510, !4517, !4527, !4592, !4593, !4596, !4597, !4598, !4599, !4600, !4601}
!4406 = !DIDerivedType(tag: DW_TAG_member, name: "sigcnt", scope: !4403, file: !4404, line: 95, baseType: !533, size: 32)
!4407 = !DIDerivedType(tag: DW_TAG_member, name: "live", scope: !4403, file: !4404, line: 96, baseType: !69, size: 32, offset: 32)
!4408 = !DIDerivedType(tag: DW_TAG_member, name: "nr_threads", scope: !4403, file: !4404, line: 97, baseType: !42, size: 32, offset: 64)
!4409 = !DIDerivedType(tag: DW_TAG_member, name: "quick_threads", scope: !4403, file: !4404, line: 98, baseType: !42, size: 32, offset: 96)
!4410 = !DIDerivedType(tag: DW_TAG_member, name: "thread_head", scope: !4403, file: !4404, line: 99, baseType: !117, size: 128, offset: 128)
!4411 = !DIDerivedType(tag: DW_TAG_member, name: "wait_chldexit", scope: !4403, file: !4404, line: 101, baseType: !74, size: 192, offset: 256)
!4412 = !DIDerivedType(tag: DW_TAG_member, name: "curr_target", scope: !4403, file: !4404, line: 104, baseType: !3628, size: 64, offset: 448)
!4413 = !DIDerivedType(tag: DW_TAG_member, name: "shared_pending", scope: !4403, file: !4404, line: 107, baseType: !4414, size: 192, offset: 512)
!4414 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "sigpending", file: !4415, line: 32, size: 192, elements: !4416)
!4415 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/signal_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "95be1cb46239cfd09f9f00972d225a32")
!4416 = !{!4417, !4418}
!4417 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !4414, file: !4415, line: 33, baseType: !117, size: 128)
!4418 = !DIDerivedType(tag: DW_TAG_member, name: "signal", scope: !4414, file: !4415, line: 34, baseType: !4419, size: 64, offset: 128)
!4419 = !DIDerivedType(tag: DW_TAG_typedef, name: "sigset_t", file: !4420, line: 25, baseType: !4421)
!4420 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/signal.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "17c5937f64ff6bd1434a69ceb9a6c563")
!4421 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !4420, line: 23, size: 64, elements: !4422)
!4422 = !{!4423}
!4423 = !DIDerivedType(tag: DW_TAG_member, name: "sig", scope: !4421, file: !4420, line: 24, baseType: !3816, size: 64)
!4424 = !DIDerivedType(tag: DW_TAG_member, name: "multiprocess", scope: !4403, file: !4404, line: 110, baseType: !216, size: 64, offset: 704)
!4425 = !DIDerivedType(tag: DW_TAG_member, name: "group_exit_code", scope: !4403, file: !4404, line: 113, baseType: !42, size: 32, offset: 768)
!4426 = !DIDerivedType(tag: DW_TAG_member, name: "notify_count", scope: !4403, file: !4404, line: 115, baseType: !42, size: 32, offset: 800)
!4427 = !DIDerivedType(tag: DW_TAG_member, name: "group_exec_task", scope: !4403, file: !4404, line: 116, baseType: !3628, size: 64, offset: 832)
!4428 = !DIDerivedType(tag: DW_TAG_member, name: "group_stop_count", scope: !4403, file: !4404, line: 119, baseType: !42, size: 32, offset: 896)
!4429 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !4403, file: !4404, line: 120, baseType: !7, size: 32, offset: 928)
!4430 = !DIDerivedType(tag: DW_TAG_member, name: "core_state", scope: !4403, file: !4404, line: 122, baseType: !4431, size: 64, offset: 960)
!4431 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4432, size: 64)
!4432 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "core_state", file: !4404, line: 81, size: 448, elements: !4433)
!4433 = !{!4434, !4435, !4441}
!4434 = !DIDerivedType(tag: DW_TAG_member, name: "nr_threads", scope: !4432, file: !4404, line: 82, baseType: !69, size: 32)
!4435 = !DIDerivedType(tag: DW_TAG_member, name: "dumper", scope: !4432, file: !4404, line: 83, baseType: !4436, size: 128, offset: 64)
!4436 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "core_thread", file: !4404, line: 76, size: 128, elements: !4437)
!4437 = !{!4438, !4439}
!4438 = !DIDerivedType(tag: DW_TAG_member, name: "task", scope: !4436, file: !4404, line: 77, baseType: !3628, size: 64)
!4439 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !4436, file: !4404, line: 78, baseType: !4440, size: 64, offset: 64)
!4440 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4436, size: 64)
!4441 = !DIDerivedType(tag: DW_TAG_member, name: "startup", scope: !4432, file: !4404, line: 84, baseType: !139, size: 256, offset: 192)
!4442 = !DIDerivedType(tag: DW_TAG_member, name: "is_child_subreaper", scope: !4403, file: !4404, line: 133, baseType: !7, size: 1, offset: 1024, flags: DIFlagBitField, extraData: i64 1024)
!4443 = !DIDerivedType(tag: DW_TAG_member, name: "has_child_subreaper", scope: !4403, file: !4404, line: 134, baseType: !7, size: 1, offset: 1025, flags: DIFlagBitField, extraData: i64 1024)
!4444 = !DIDerivedType(tag: DW_TAG_member, name: "next_posix_timer_id", scope: !4403, file: !4404, line: 139, baseType: !7, size: 32, offset: 1056)
!4445 = !DIDerivedType(tag: DW_TAG_member, name: "posix_timers", scope: !4403, file: !4404, line: 140, baseType: !216, size: 64, offset: 1088)
!4446 = !DIDerivedType(tag: DW_TAG_member, name: "real_timer", scope: !4403, file: !4404, line: 143, baseType: !2103, size: 512, offset: 1152)
!4447 = !DIDerivedType(tag: DW_TAG_member, name: "it_real_incr", scope: !4403, file: !4404, line: 144, baseType: !2083, size: 64, offset: 1664)
!4448 = !DIDerivedType(tag: DW_TAG_member, name: "it", scope: !4403, file: !4404, line: 151, baseType: !4449, size: 256, offset: 1728)
!4449 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4450, size: 256, elements: !2684)
!4450 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cpu_itimer", file: !4404, line: 39, size: 128, elements: !4451)
!4451 = !{!4452, !4453}
!4452 = !DIDerivedType(tag: DW_TAG_member, name: "expires", scope: !4450, file: !4404, line: 40, baseType: !519, size: 64)
!4453 = !DIDerivedType(tag: DW_TAG_member, name: "incr", scope: !4450, file: !4404, line: 41, baseType: !519, size: 64, offset: 64)
!4454 = !DIDerivedType(tag: DW_TAG_member, name: "cputimer", scope: !4403, file: !4404, line: 157, baseType: !4455, size: 192, offset: 1984)
!4455 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "thread_group_cputimer", file: !4404, line: 67, size: 192, elements: !4456)
!4456 = !{!4457}
!4457 = !DIDerivedType(tag: DW_TAG_member, name: "cputime_atomic", scope: !4455, file: !4404, line: 68, baseType: !4458, size: 192)
!4458 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "task_cputime_atomic", file: !4404, line: 48, size: 192, elements: !4459)
!4459 = !{!4460, !4461, !4462}
!4460 = !DIDerivedType(tag: DW_TAG_member, name: "utime", scope: !4458, file: !4404, line: 49, baseType: !498, size: 64)
!4461 = !DIDerivedType(tag: DW_TAG_member, name: "stime", scope: !4458, file: !4404, line: 50, baseType: !498, size: 64, offset: 64)
!4462 = !DIDerivedType(tag: DW_TAG_member, name: "sum_exec_runtime", scope: !4458, file: !4404, line: 51, baseType: !498, size: 64, offset: 128)
!4463 = !DIDerivedType(tag: DW_TAG_member, name: "posix_cputimers", scope: !4403, file: !4404, line: 161, baseType: !4033, size: 640, offset: 2176)
!4464 = !DIDerivedType(tag: DW_TAG_member, name: "pids", scope: !4403, file: !4404, line: 164, baseType: !4465, size: 256, offset: 2816)
!4465 = !DICompositeType(tag: DW_TAG_array_type, baseType: !3949, size: 256, elements: !635)
!4466 = !DIDerivedType(tag: DW_TAG_member, name: "tty_old_pgrp", scope: !4403, file: !4404, line: 170, baseType: !3949, size: 64, offset: 3072)
!4467 = !DIDerivedType(tag: DW_TAG_member, name: "leader", scope: !4403, file: !4404, line: 173, baseType: !42, size: 32, offset: 3136)
!4468 = !DIDerivedType(tag: DW_TAG_member, name: "tty", scope: !4403, file: !4404, line: 175, baseType: !4469, size: 64, offset: 3200)
!4469 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4470, size: 64)
!4470 = !DICompositeType(tag: DW_TAG_structure_type, name: "tty_struct", file: !4404, line: 175, flags: DIFlagFwdDecl)
!4471 = !DIDerivedType(tag: DW_TAG_member, name: "stats_lock", scope: !4403, file: !4404, line: 186, baseType: !4472, size: 64, offset: 3264)
!4472 = !DIDerivedType(tag: DW_TAG_typedef, name: "seqlock_t", file: !746, line: 91, baseType: !4473)
!4473 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !746, line: 84, size: 64, elements: !4474)
!4474 = !{!4475, !4476}
!4475 = !DIDerivedType(tag: DW_TAG_member, name: "seqcount", scope: !4473, file: !746, line: 89, baseType: !745, size: 32)
!4476 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !4473, file: !746, line: 90, baseType: !79, size: 32, offset: 32)
!4477 = !DIDerivedType(tag: DW_TAG_member, name: "utime", scope: !4403, file: !4404, line: 187, baseType: !519, size: 64, offset: 3328)
!4478 = !DIDerivedType(tag: DW_TAG_member, name: "stime", scope: !4403, file: !4404, line: 187, baseType: !519, size: 64, offset: 3392)
!4479 = !DIDerivedType(tag: DW_TAG_member, name: "cutime", scope: !4403, file: !4404, line: 187, baseType: !519, size: 64, offset: 3456)
!4480 = !DIDerivedType(tag: DW_TAG_member, name: "cstime", scope: !4403, file: !4404, line: 187, baseType: !519, size: 64, offset: 3520)
!4481 = !DIDerivedType(tag: DW_TAG_member, name: "gtime", scope: !4403, file: !4404, line: 188, baseType: !519, size: 64, offset: 3584)
!4482 = !DIDerivedType(tag: DW_TAG_member, name: "cgtime", scope: !4403, file: !4404, line: 189, baseType: !519, size: 64, offset: 3648)
!4483 = !DIDerivedType(tag: DW_TAG_member, name: "prev_cputime", scope: !4403, file: !4404, line: 190, baseType: !4021, size: 192, offset: 3712)
!4484 = !DIDerivedType(tag: DW_TAG_member, name: "nvcsw", scope: !4403, file: !4404, line: 191, baseType: !59, size: 64, offset: 3904)
!4485 = !DIDerivedType(tag: DW_TAG_member, name: "nivcsw", scope: !4403, file: !4404, line: 191, baseType: !59, size: 64, offset: 3968)
!4486 = !DIDerivedType(tag: DW_TAG_member, name: "cnvcsw", scope: !4403, file: !4404, line: 191, baseType: !59, size: 64, offset: 4032)
!4487 = !DIDerivedType(tag: DW_TAG_member, name: "cnivcsw", scope: !4403, file: !4404, line: 191, baseType: !59, size: 64, offset: 4096)
!4488 = !DIDerivedType(tag: DW_TAG_member, name: "min_flt", scope: !4403, file: !4404, line: 192, baseType: !59, size: 64, offset: 4160)
!4489 = !DIDerivedType(tag: DW_TAG_member, name: "maj_flt", scope: !4403, file: !4404, line: 192, baseType: !59, size: 64, offset: 4224)
!4490 = !DIDerivedType(tag: DW_TAG_member, name: "cmin_flt", scope: !4403, file: !4404, line: 192, baseType: !59, size: 64, offset: 4288)
!4491 = !DIDerivedType(tag: DW_TAG_member, name: "cmaj_flt", scope: !4403, file: !4404, line: 192, baseType: !59, size: 64, offset: 4352)
!4492 = !DIDerivedType(tag: DW_TAG_member, name: "inblock", scope: !4403, file: !4404, line: 193, baseType: !59, size: 64, offset: 4416)
!4493 = !DIDerivedType(tag: DW_TAG_member, name: "oublock", scope: !4403, file: !4404, line: 193, baseType: !59, size: 64, offset: 4480)
!4494 = !DIDerivedType(tag: DW_TAG_member, name: "cinblock", scope: !4403, file: !4404, line: 193, baseType: !59, size: 64, offset: 4544)
!4495 = !DIDerivedType(tag: DW_TAG_member, name: "coublock", scope: !4403, file: !4404, line: 193, baseType: !59, size: 64, offset: 4608)
!4496 = !DIDerivedType(tag: DW_TAG_member, name: "maxrss", scope: !4403, file: !4404, line: 194, baseType: !59, size: 64, offset: 4672)
!4497 = !DIDerivedType(tag: DW_TAG_member, name: "cmaxrss", scope: !4403, file: !4404, line: 194, baseType: !59, size: 64, offset: 4736)
!4498 = !DIDerivedType(tag: DW_TAG_member, name: "ioac", scope: !4403, file: !4404, line: 195, baseType: !4499, size: 448, offset: 4800)
!4499 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "task_io_accounting", file: !4500, line: 12, size: 448, elements: !4501)
!4500 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/task_io_accounting.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "f1163883a6b1a5e3392a18ac19b36a07")
!4501 = !{!4502, !4503, !4504, !4505, !4506, !4507, !4508}
!4502 = !DIDerivedType(tag: DW_TAG_member, name: "rchar", scope: !4499, file: !4500, line: 15, baseType: !519, size: 64)
!4503 = !DIDerivedType(tag: DW_TAG_member, name: "wchar", scope: !4499, file: !4500, line: 17, baseType: !519, size: 64, offset: 64)
!4504 = !DIDerivedType(tag: DW_TAG_member, name: "syscr", scope: !4499, file: !4500, line: 19, baseType: !519, size: 64, offset: 128)
!4505 = !DIDerivedType(tag: DW_TAG_member, name: "syscw", scope: !4499, file: !4500, line: 21, baseType: !519, size: 64, offset: 192)
!4506 = !DIDerivedType(tag: DW_TAG_member, name: "read_bytes", scope: !4499, file: !4500, line: 29, baseType: !519, size: 64, offset: 256)
!4507 = !DIDerivedType(tag: DW_TAG_member, name: "write_bytes", scope: !4499, file: !4500, line: 35, baseType: !519, size: 64, offset: 320)
!4508 = !DIDerivedType(tag: DW_TAG_member, name: "cancelled_write_bytes", scope: !4499, file: !4500, line: 44, baseType: !519, size: 64, offset: 384)
!4509 = !DIDerivedType(tag: DW_TAG_member, name: "sum_sched_runtime", scope: !4403, file: !4404, line: 203, baseType: !521, size: 64, offset: 5248)
!4510 = !DIDerivedType(tag: DW_TAG_member, name: "rlim", scope: !4403, file: !4404, line: 214, baseType: !4511, size: 2048, offset: 5312)
!4511 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4512, size: 2048, elements: !4056)
!4512 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "rlimit", file: !4513, line: 43, size: 128, elements: !4514)
!4513 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/uapi/linux/resource.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "c8cfde99fa9f783d888356610fa3e082")
!4514 = !{!4515, !4516}
!4515 = !DIDerivedType(tag: DW_TAG_member, name: "rlim_cur", scope: !4512, file: !4513, line: 44, baseType: !58, size: 64)
!4516 = !DIDerivedType(tag: DW_TAG_member, name: "rlim_max", scope: !4512, file: !4513, line: 45, baseType: !58, size: 64, offset: 64)
!4517 = !DIDerivedType(tag: DW_TAG_member, name: "pacct", scope: !4403, file: !4404, line: 217, baseType: !4518, size: 448, offset: 7360)
!4518 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "pacct_struct", file: !4404, line: 31, size: 448, elements: !4519)
!4519 = !{!4520, !4521, !4522, !4523, !4524, !4525, !4526}
!4520 = !DIDerivedType(tag: DW_TAG_member, name: "ac_flag", scope: !4518, file: !4404, line: 32, baseType: !42, size: 32)
!4521 = !DIDerivedType(tag: DW_TAG_member, name: "ac_exitcode", scope: !4518, file: !4404, line: 33, baseType: !892, size: 64, offset: 64)
!4522 = !DIDerivedType(tag: DW_TAG_member, name: "ac_mem", scope: !4518, file: !4404, line: 34, baseType: !59, size: 64, offset: 128)
!4523 = !DIDerivedType(tag: DW_TAG_member, name: "ac_utime", scope: !4518, file: !4404, line: 35, baseType: !519, size: 64, offset: 192)
!4524 = !DIDerivedType(tag: DW_TAG_member, name: "ac_stime", scope: !4518, file: !4404, line: 35, baseType: !519, size: 64, offset: 256)
!4525 = !DIDerivedType(tag: DW_TAG_member, name: "ac_minflt", scope: !4518, file: !4404, line: 36, baseType: !59, size: 64, offset: 320)
!4526 = !DIDerivedType(tag: DW_TAG_member, name: "ac_majflt", scope: !4518, file: !4404, line: 36, baseType: !59, size: 64, offset: 384)
!4527 = !DIDerivedType(tag: DW_TAG_member, name: "stats", scope: !4403, file: !4404, line: 220, baseType: !4528, size: 64, offset: 7808)
!4528 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4529, size: 64)
!4529 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "taskstats", file: !4530, line: 41, size: 3456, elements: !4531)
!4530 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/uapi/linux/taskstats.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "406c164c2424c0b62d3cc8974eb1f3c8")
!4531 = !{!4532, !4533, !4534, !4535, !4536, !4537, !4538, !4539, !4540, !4541, !4542, !4543, !4544, !4548, !4549, !4551, !4552, !4553, !4554, !4555, !4556, !4557, !4558, !4559, !4560, !4561, !4562, !4563, !4564, !4565, !4566, !4567, !4568, !4569, !4570, !4571, !4572, !4573, !4574, !4575, !4576, !4577, !4578, !4579, !4580, !4581, !4582, !4583, !4584, !4585, !4586, !4587, !4588, !4589, !4590, !4591}
!4532 = !DIDerivedType(tag: DW_TAG_member, name: "version", scope: !4529, file: !4530, line: 47, baseType: !114, size: 16)
!4533 = !DIDerivedType(tag: DW_TAG_member, name: "ac_exitcode", scope: !4529, file: !4530, line: 48, baseType: !579, size: 32, offset: 32)
!4534 = !DIDerivedType(tag: DW_TAG_member, name: "ac_flag", scope: !4529, file: !4530, line: 54, baseType: !105, size: 8, offset: 64)
!4535 = !DIDerivedType(tag: DW_TAG_member, name: "ac_nice", scope: !4529, file: !4530, line: 55, baseType: !105, size: 8, offset: 72)
!4536 = !DIDerivedType(tag: DW_TAG_member, name: "cpu_count", scope: !4529, file: !4530, line: 73, baseType: !520, size: 64, align: 64, offset: 128)
!4537 = !DIDerivedType(tag: DW_TAG_member, name: "cpu_delay_total", scope: !4529, file: !4530, line: 74, baseType: !520, size: 64, offset: 192)
!4538 = !DIDerivedType(tag: DW_TAG_member, name: "blkio_count", scope: !4529, file: !4530, line: 81, baseType: !520, size: 64, offset: 256)
!4539 = !DIDerivedType(tag: DW_TAG_member, name: "blkio_delay_total", scope: !4529, file: !4530, line: 82, baseType: !520, size: 64, offset: 320)
!4540 = !DIDerivedType(tag: DW_TAG_member, name: "swapin_count", scope: !4529, file: !4530, line: 85, baseType: !520, size: 64, offset: 384)
!4541 = !DIDerivedType(tag: DW_TAG_member, name: "swapin_delay_total", scope: !4529, file: !4530, line: 86, baseType: !520, size: 64, offset: 448)
!4542 = !DIDerivedType(tag: DW_TAG_member, name: "cpu_run_real_total", scope: !4529, file: !4530, line: 94, baseType: !520, size: 64, offset: 512)
!4543 = !DIDerivedType(tag: DW_TAG_member, name: "cpu_run_virtual_total", scope: !4529, file: !4530, line: 102, baseType: !520, size: 64, offset: 576)
!4544 = !DIDerivedType(tag: DW_TAG_member, name: "ac_comm", scope: !4529, file: !4530, line: 107, baseType: !4545, size: 256, offset: 640)
!4545 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 256, elements: !4546)
!4546 = !{!4547}
!4547 = !DISubrange(count: 32)
!4548 = !DIDerivedType(tag: DW_TAG_member, name: "ac_sched", scope: !4529, file: !4530, line: 108, baseType: !105, size: 8, align: 64, offset: 896)
!4549 = !DIDerivedType(tag: DW_TAG_member, name: "ac_pad", scope: !4529, file: !4530, line: 110, baseType: !4550, size: 24, offset: 904)
!4550 = !DICompositeType(tag: DW_TAG_array_type, baseType: !105, size: 24, elements: !962)
!4551 = !DIDerivedType(tag: DW_TAG_member, name: "ac_uid", scope: !4529, file: !4530, line: 111, baseType: !579, size: 32, align: 64, offset: 960)
!4552 = !DIDerivedType(tag: DW_TAG_member, name: "ac_gid", scope: !4529, file: !4530, line: 113, baseType: !579, size: 32, offset: 992)
!4553 = !DIDerivedType(tag: DW_TAG_member, name: "ac_pid", scope: !4529, file: !4530, line: 114, baseType: !579, size: 32, offset: 1024)
!4554 = !DIDerivedType(tag: DW_TAG_member, name: "ac_ppid", scope: !4529, file: !4530, line: 115, baseType: !579, size: 32, offset: 1056)
!4555 = !DIDerivedType(tag: DW_TAG_member, name: "ac_btime", scope: !4529, file: !4530, line: 117, baseType: !579, size: 32, offset: 1088)
!4556 = !DIDerivedType(tag: DW_TAG_member, name: "ac_etime", scope: !4529, file: !4530, line: 118, baseType: !520, size: 64, align: 64, offset: 1152)
!4557 = !DIDerivedType(tag: DW_TAG_member, name: "ac_utime", scope: !4529, file: !4530, line: 120, baseType: !520, size: 64, offset: 1216)
!4558 = !DIDerivedType(tag: DW_TAG_member, name: "ac_stime", scope: !4529, file: !4530, line: 121, baseType: !520, size: 64, offset: 1280)
!4559 = !DIDerivedType(tag: DW_TAG_member, name: "ac_minflt", scope: !4529, file: !4530, line: 122, baseType: !520, size: 64, offset: 1344)
!4560 = !DIDerivedType(tag: DW_TAG_member, name: "ac_majflt", scope: !4529, file: !4530, line: 123, baseType: !520, size: 64, offset: 1408)
!4561 = !DIDerivedType(tag: DW_TAG_member, name: "coremem", scope: !4529, file: !4530, line: 133, baseType: !520, size: 64, offset: 1472)
!4562 = !DIDerivedType(tag: DW_TAG_member, name: "virtmem", scope: !4529, file: !4530, line: 137, baseType: !520, size: 64, offset: 1536)
!4563 = !DIDerivedType(tag: DW_TAG_member, name: "hiwater_rss", scope: !4529, file: !4530, line: 142, baseType: !520, size: 64, offset: 1600)
!4564 = !DIDerivedType(tag: DW_TAG_member, name: "hiwater_vm", scope: !4529, file: !4530, line: 143, baseType: !520, size: 64, offset: 1664)
!4565 = !DIDerivedType(tag: DW_TAG_member, name: "read_char", scope: !4529, file: !4530, line: 146, baseType: !520, size: 64, offset: 1728)
!4566 = !DIDerivedType(tag: DW_TAG_member, name: "write_char", scope: !4529, file: !4530, line: 147, baseType: !520, size: 64, offset: 1792)
!4567 = !DIDerivedType(tag: DW_TAG_member, name: "read_syscalls", scope: !4529, file: !4530, line: 148, baseType: !520, size: 64, offset: 1856)
!4568 = !DIDerivedType(tag: DW_TAG_member, name: "write_syscalls", scope: !4529, file: !4530, line: 149, baseType: !520, size: 64, offset: 1920)
!4569 = !DIDerivedType(tag: DW_TAG_member, name: "read_bytes", scope: !4529, file: !4530, line: 154, baseType: !520, size: 64, offset: 1984)
!4570 = !DIDerivedType(tag: DW_TAG_member, name: "write_bytes", scope: !4529, file: !4530, line: 155, baseType: !520, size: 64, offset: 2048)
!4571 = !DIDerivedType(tag: DW_TAG_member, name: "cancelled_write_bytes", scope: !4529, file: !4530, line: 156, baseType: !520, size: 64, offset: 2112)
!4572 = !DIDerivedType(tag: DW_TAG_member, name: "nvcsw", scope: !4529, file: !4530, line: 158, baseType: !520, size: 64, offset: 2176)
!4573 = !DIDerivedType(tag: DW_TAG_member, name: "nivcsw", scope: !4529, file: !4530, line: 159, baseType: !520, size: 64, offset: 2240)
!4574 = !DIDerivedType(tag: DW_TAG_member, name: "ac_utimescaled", scope: !4529, file: !4530, line: 162, baseType: !520, size: 64, offset: 2304)
!4575 = !DIDerivedType(tag: DW_TAG_member, name: "ac_stimescaled", scope: !4529, file: !4530, line: 163, baseType: !520, size: 64, offset: 2368)
!4576 = !DIDerivedType(tag: DW_TAG_member, name: "cpu_scaled_run_real_total", scope: !4529, file: !4530, line: 164, baseType: !520, size: 64, offset: 2432)
!4577 = !DIDerivedType(tag: DW_TAG_member, name: "freepages_count", scope: !4529, file: !4530, line: 167, baseType: !520, size: 64, offset: 2496)
!4578 = !DIDerivedType(tag: DW_TAG_member, name: "freepages_delay_total", scope: !4529, file: !4530, line: 168, baseType: !520, size: 64, offset: 2560)
!4579 = !DIDerivedType(tag: DW_TAG_member, name: "thrashing_count", scope: !4529, file: !4530, line: 171, baseType: !520, size: 64, offset: 2624)
!4580 = !DIDerivedType(tag: DW_TAG_member, name: "thrashing_delay_total", scope: !4529, file: !4530, line: 172, baseType: !520, size: 64, offset: 2688)
!4581 = !DIDerivedType(tag: DW_TAG_member, name: "ac_btime64", scope: !4529, file: !4530, line: 175, baseType: !520, size: 64, offset: 2752)
!4582 = !DIDerivedType(tag: DW_TAG_member, name: "compact_count", scope: !4529, file: !4530, line: 178, baseType: !520, size: 64, offset: 2816)
!4583 = !DIDerivedType(tag: DW_TAG_member, name: "compact_delay_total", scope: !4529, file: !4530, line: 179, baseType: !520, size: 64, offset: 2880)
!4584 = !DIDerivedType(tag: DW_TAG_member, name: "ac_tgid", scope: !4529, file: !4530, line: 182, baseType: !579, size: 32, offset: 2944)
!4585 = !DIDerivedType(tag: DW_TAG_member, name: "ac_tgetime", scope: !4529, file: !4530, line: 186, baseType: !520, size: 64, align: 64, offset: 3008)
!4586 = !DIDerivedType(tag: DW_TAG_member, name: "ac_exe_dev", scope: !4529, file: !4530, line: 194, baseType: !520, size: 64, offset: 3072)
!4587 = !DIDerivedType(tag: DW_TAG_member, name: "ac_exe_inode", scope: !4529, file: !4530, line: 195, baseType: !520, size: 64, offset: 3136)
!4588 = !DIDerivedType(tag: DW_TAG_member, name: "wpcopy_count", scope: !4529, file: !4530, line: 199, baseType: !520, size: 64, offset: 3200)
!4589 = !DIDerivedType(tag: DW_TAG_member, name: "wpcopy_delay_total", scope: !4529, file: !4530, line: 200, baseType: !520, size: 64, offset: 3264)
!4590 = !DIDerivedType(tag: DW_TAG_member, name: "irq_count", scope: !4529, file: !4530, line: 203, baseType: !520, size: 64, offset: 3328)
!4591 = !DIDerivedType(tag: DW_TAG_member, name: "irq_delay_total", scope: !4529, file: !4530, line: 204, baseType: !520, size: 64, offset: 3392)
!4592 = !DIDerivedType(tag: DW_TAG_member, name: "audit_tty", scope: !4403, file: !4404, line: 223, baseType: !7, size: 32, offset: 7872)
!4593 = !DIDerivedType(tag: DW_TAG_member, name: "tty_audit_buf", scope: !4403, file: !4404, line: 224, baseType: !4594, size: 64, offset: 7936)
!4594 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4595, size: 64)
!4595 = !DICompositeType(tag: DW_TAG_structure_type, name: "tty_audit_buf", file: !4404, line: 224, flags: DIFlagFwdDecl)
!4596 = !DIDerivedType(tag: DW_TAG_member, name: "oom_flag_origin", scope: !4403, file: !4404, line: 231, baseType: !614, size: 8, offset: 8000)
!4597 = !DIDerivedType(tag: DW_TAG_member, name: "oom_score_adj", scope: !4403, file: !4404, line: 232, baseType: !583, size: 16, offset: 8016)
!4598 = !DIDerivedType(tag: DW_TAG_member, name: "oom_score_adj_min", scope: !4403, file: !4404, line: 233, baseType: !583, size: 16, offset: 8032)
!4599 = !DIDerivedType(tag: DW_TAG_member, name: "oom_mm", scope: !4403, file: !4404, line: 235, baseType: !1180, size: 64, offset: 8064)
!4600 = !DIDerivedType(tag: DW_TAG_member, name: "cred_guard_mutex", scope: !4403, file: !4404, line: 238, baseType: !1277, size: 256, offset: 8128)
!4601 = !DIDerivedType(tag: DW_TAG_member, name: "exec_update_lock", scope: !4403, file: !4404, line: 244, baseType: !549, size: 320, offset: 8384)
!4602 = !DIDerivedType(tag: DW_TAG_member, name: "sighand", scope: !3629, file: !1435, line: 1155, baseType: !4603, size: 64, offset: 15424)
!4603 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4604, size: 64)
!4604 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "sighand_struct", file: !4404, line: 21, size: 16640, elements: !4605)
!4605 = !{!4606, !4607, !4608, !4609}
!4606 = !DIDerivedType(tag: DW_TAG_member, name: "siglock", scope: !4604, file: !4404, line: 22, baseType: !79, size: 32)
!4607 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !4604, file: !4404, line: 23, baseType: !533, size: 32, offset: 32)
!4608 = !DIDerivedType(tag: DW_TAG_member, name: "signalfd_wqh", scope: !4604, file: !4404, line: 24, baseType: !74, size: 192, offset: 64)
!4609 = !DIDerivedType(tag: DW_TAG_member, name: "action", scope: !4604, file: !4404, line: 25, baseType: !4610, size: 16384, offset: 256)
!4610 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4611, size: 16384, elements: !966)
!4611 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "k_sigaction", file: !4415, line: 51, size: 256, elements: !4612)
!4612 = !{!4613}
!4613 = !DIDerivedType(tag: DW_TAG_member, name: "sa", scope: !4611, file: !4415, line: 52, baseType: !4614, size: 256)
!4614 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "sigaction", file: !4415, line: 37, size: 256, elements: !4615)
!4615 = !{!4616, !4623, !4624, !4628}
!4616 = !DIDerivedType(tag: DW_TAG_member, name: "sa_handler", scope: !4614, file: !4415, line: 39, baseType: !4617, size: 64)
!4617 = !DIDerivedType(tag: DW_TAG_typedef, name: "__sighandler_t", file: !4618, line: 83, baseType: !4619)
!4618 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/uapi/asm-generic/signal-defs.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "b2c8f056e35777be7e127ebbe5efe17a")
!4619 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4620, size: 64)
!4620 = !DIDerivedType(tag: DW_TAG_typedef, name: "__signalfn_t", file: !4618, line: 82, baseType: !4621)
!4621 = !DISubroutineType(types: !4622)
!4622 = !{null, !42}
!4623 = !DIDerivedType(tag: DW_TAG_member, name: "sa_flags", scope: !4614, file: !4415, line: 40, baseType: !59, size: 64, offset: 64)
!4624 = !DIDerivedType(tag: DW_TAG_member, name: "sa_restorer", scope: !4614, file: !4415, line: 46, baseType: !4625, size: 64, offset: 128)
!4625 = !DIDerivedType(tag: DW_TAG_typedef, name: "__sigrestore_t", file: !4618, line: 86, baseType: !4626)
!4626 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4627, size: 64)
!4627 = !DIDerivedType(tag: DW_TAG_typedef, name: "__restorefn_t", file: !4618, line: 85, baseType: !2887)
!4628 = !DIDerivedType(tag: DW_TAG_member, name: "sa_mask", scope: !4614, file: !4415, line: 48, baseType: !4419, size: 64, offset: 192)
!4629 = !DIDerivedType(tag: DW_TAG_member, name: "blocked", scope: !3629, file: !1435, line: 1156, baseType: !4419, size: 64, offset: 15488)
!4630 = !DIDerivedType(tag: DW_TAG_member, name: "real_blocked", scope: !3629, file: !1435, line: 1157, baseType: !4419, size: 64, offset: 15552)
!4631 = !DIDerivedType(tag: DW_TAG_member, name: "saved_sigmask", scope: !3629, file: !1435, line: 1159, baseType: !4419, size: 64, offset: 15616)
!4632 = !DIDerivedType(tag: DW_TAG_member, name: "pending", scope: !3629, file: !1435, line: 1160, baseType: !4414, size: 192, offset: 15680)
!4633 = !DIDerivedType(tag: DW_TAG_member, name: "sas_ss_sp", scope: !3629, file: !1435, line: 1161, baseType: !59, size: 64, offset: 15872)
!4634 = !DIDerivedType(tag: DW_TAG_member, name: "sas_ss_size", scope: !3629, file: !1435, line: 1162, baseType: !55, size: 64, offset: 15936)
!4635 = !DIDerivedType(tag: DW_TAG_member, name: "sas_ss_flags", scope: !3629, file: !1435, line: 1163, baseType: !7, size: 32, offset: 16000)
!4636 = !DIDerivedType(tag: DW_TAG_member, name: "task_works", scope: !3629, file: !1435, line: 1165, baseType: !132, size: 64, offset: 16064)
!4637 = !DIDerivedType(tag: DW_TAG_member, name: "audit_context", scope: !3629, file: !1435, line: 1169, baseType: !4638, size: 64, offset: 16128)
!4638 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4639, size: 64)
!4639 = !DICompositeType(tag: DW_TAG_structure_type, name: "audit_context", file: !1435, line: 52, flags: DIFlagFwdDecl)
!4640 = !DIDerivedType(tag: DW_TAG_member, name: "loginuid", scope: !3629, file: !1435, line: 1171, baseType: !188, size: 32, offset: 16192)
!4641 = !DIDerivedType(tag: DW_TAG_member, name: "sessionid", scope: !3629, file: !1435, line: 1172, baseType: !7, size: 32, offset: 16224)
!4642 = !DIDerivedType(tag: DW_TAG_member, name: "seccomp", scope: !3629, file: !1435, line: 1174, baseType: !4643, size: 128, offset: 16256)
!4643 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "seccomp", file: !4644, line: 22, size: 128, elements: !4645)
!4644 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/seccomp_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "0eff61875d111fb2d7c1fc872102f94a")
!4645 = !{!4646, !4647, !4648}
!4646 = !DIDerivedType(tag: DW_TAG_member, name: "mode", scope: !4643, file: !4644, line: 23, baseType: !42, size: 32)
!4647 = !DIDerivedType(tag: DW_TAG_member, name: "filter_count", scope: !4643, file: !4644, line: 24, baseType: !69, size: 32, offset: 32)
!4648 = !DIDerivedType(tag: DW_TAG_member, name: "filter", scope: !4643, file: !4644, line: 25, baseType: !4649, size: 64, offset: 64)
!4649 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4650, size: 64)
!4650 = !DICompositeType(tag: DW_TAG_structure_type, name: "seccomp_filter", file: !4644, line: 9, flags: DIFlagFwdDecl)
!4651 = !DIDerivedType(tag: DW_TAG_member, name: "syscall_dispatch", scope: !3629, file: !1435, line: 1175, baseType: !4652, size: 256, offset: 16384)
!4652 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "syscall_user_dispatch", file: !4653, line: 9, size: 256, elements: !4654)
!4653 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/syscall_user_dispatch_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "c2280e38e7a611cd07b1ceee357bcc0d")
!4654 = !{!4655, !4656, !4657, !4658}
!4655 = !DIDerivedType(tag: DW_TAG_member, name: "selector", scope: !4652, file: !4653, line: 10, baseType: !625, size: 64)
!4656 = !DIDerivedType(tag: DW_TAG_member, name: "offset", scope: !4652, file: !4653, line: 11, baseType: !59, size: 64, offset: 64)
!4657 = !DIDerivedType(tag: DW_TAG_member, name: "len", scope: !4652, file: !4653, line: 12, baseType: !59, size: 64, offset: 128)
!4658 = !DIDerivedType(tag: DW_TAG_member, name: "on_dispatch", scope: !4652, file: !4653, line: 13, baseType: !614, size: 8, offset: 192)
!4659 = !DIDerivedType(tag: DW_TAG_member, name: "parent_exec_id", scope: !3629, file: !1435, line: 1178, baseType: !519, size: 64, offset: 16640)
!4660 = !DIDerivedType(tag: DW_TAG_member, name: "self_exec_id", scope: !3629, file: !1435, line: 1179, baseType: !519, size: 64, offset: 16704)
!4661 = !DIDerivedType(tag: DW_TAG_member, name: "alloc_lock", scope: !3629, file: !1435, line: 1182, baseType: !79, size: 32, offset: 16768)
!4662 = !DIDerivedType(tag: DW_TAG_member, name: "pi_lock", scope: !3629, file: !1435, line: 1185, baseType: !148, size: 32, offset: 16800)
!4663 = !DIDerivedType(tag: DW_TAG_member, name: "wake_q", scope: !3629, file: !1435, line: 1187, baseType: !4664, size: 64, offset: 16832)
!4664 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "wake_q_node", file: !1435, line: 767, size: 64, elements: !4665)
!4665 = !{!4666}
!4666 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !4664, file: !1435, line: 768, baseType: !4667, size: 64)
!4667 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4664, size: 64)
!4668 = !DIDerivedType(tag: DW_TAG_member, name: "pi_waiters", scope: !3629, file: !1435, line: 1191, baseType: !1045, size: 128, offset: 16896)
!4669 = !DIDerivedType(tag: DW_TAG_member, name: "pi_top_task", scope: !3629, file: !1435, line: 1193, baseType: !3628, size: 64, offset: 17024)
!4670 = !DIDerivedType(tag: DW_TAG_member, name: "pi_blocked_on", scope: !3629, file: !1435, line: 1195, baseType: !4671, size: 64, offset: 17088)
!4671 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4672, size: 64)
!4672 = !DICompositeType(tag: DW_TAG_structure_type, name: "rt_mutex_waiter", file: !1435, line: 1195, flags: DIFlagFwdDecl)
!4673 = !DIDerivedType(tag: DW_TAG_member, name: "journal_info", scope: !3629, file: !1435, line: 1232, baseType: !40, size: 64, offset: 17152)
!4674 = !DIDerivedType(tag: DW_TAG_member, name: "bio_list", scope: !3629, file: !1435, line: 1235, baseType: !4675, size: 64, offset: 17216)
!4675 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4676, size: 64)
!4676 = !DICompositeType(tag: DW_TAG_structure_type, name: "bio_list", file: !1435, line: 53, flags: DIFlagFwdDecl)
!4677 = !DIDerivedType(tag: DW_TAG_member, name: "plug", scope: !3629, file: !1435, line: 1238, baseType: !4678, size: 64, offset: 17280)
!4678 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4679, size: 64)
!4679 = !DICompositeType(tag: DW_TAG_structure_type, name: "blk_plug", file: !1435, line: 54, flags: DIFlagFwdDecl)
!4680 = !DIDerivedType(tag: DW_TAG_member, name: "reclaim_state", scope: !3629, file: !1435, line: 1241, baseType: !4681, size: 64, offset: 17344)
!4681 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4682, size: 64)
!4682 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "reclaim_state", file: !1808, line: 155, size: 64, elements: !4683)
!4683 = !{!4684}
!4684 = !DIDerivedType(tag: DW_TAG_member, name: "reclaimed", scope: !4682, file: !1808, line: 157, baseType: !59, size: 64)
!4685 = !DIDerivedType(tag: DW_TAG_member, name: "io_context", scope: !3629, file: !1435, line: 1243, baseType: !4686, size: 64, offset: 17408)
!4686 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4687, size: 64)
!4687 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "io_context", file: !1871, line: 99, size: 128, elements: !4688)
!4688 = !{!4689, !4690, !4691}
!4689 = !DIDerivedType(tag: DW_TAG_member, name: "refcount", scope: !4687, file: !1871, line: 100, baseType: !496, size: 64)
!4690 = !DIDerivedType(tag: DW_TAG_member, name: "active_ref", scope: !4687, file: !1871, line: 101, baseType: !69, size: 32, offset: 64)
!4691 = !DIDerivedType(tag: DW_TAG_member, name: "ioprio", scope: !4687, file: !1871, line: 103, baseType: !46, size: 16, offset: 96)
!4692 = !DIDerivedType(tag: DW_TAG_member, name: "capture_control", scope: !3629, file: !1435, line: 1246, baseType: !4693, size: 64, offset: 17472)
!4693 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4694, size: 64)
!4694 = !DICompositeType(tag: DW_TAG_structure_type, name: "capture_control", file: !1435, line: 58, flags: DIFlagFwdDecl)
!4695 = !DIDerivedType(tag: DW_TAG_member, name: "ptrace_message", scope: !3629, file: !1435, line: 1249, baseType: !59, size: 64, offset: 17536)
!4696 = !DIDerivedType(tag: DW_TAG_member, name: "last_siginfo", scope: !3629, file: !1435, line: 1250, baseType: !4697, size: 64, offset: 17600)
!4697 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4698, size: 64)
!4698 = !DIDerivedType(tag: DW_TAG_typedef, name: "kernel_siginfo_t", file: !4415, line: 14, baseType: !4699)
!4699 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kernel_siginfo", file: !4415, line: 12, size: 384, elements: !4700)
!4700 = !{!4701}
!4701 = !DIDerivedType(tag: DW_TAG_member, scope: !4699, file: !4415, line: 13, baseType: !4702, size: 384)
!4702 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4699, file: !4415, line: 13, size: 384, elements: !4703)
!4703 = !{!4704, !4705, !4706, !4707}
!4704 = !DIDerivedType(tag: DW_TAG_member, name: "si_signo", scope: !4702, file: !4415, line: 13, baseType: !42, size: 32)
!4705 = !DIDerivedType(tag: DW_TAG_member, name: "si_errno", scope: !4702, file: !4415, line: 13, baseType: !42, size: 32, offset: 32)
!4706 = !DIDerivedType(tag: DW_TAG_member, name: "si_code", scope: !4702, file: !4415, line: 13, baseType: !42, size: 32, offset: 64)
!4707 = !DIDerivedType(tag: DW_TAG_member, name: "_sifields", scope: !4702, file: !4415, line: 13, baseType: !4708, size: 256, offset: 128)
!4708 = distinct !DICompositeType(tag: DW_TAG_union_type, name: "__sifields", file: !4709, line: 37, size: 256, elements: !4710)
!4709 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/uapi/asm-generic/siginfo.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "d873bdc5236a48453d7006d57ee5a24e")
!4710 = !{!4711, !4716, !4729, !4735, !4744, !4771, !4776}
!4711 = !DIDerivedType(tag: DW_TAG_member, name: "_kill", scope: !4708, file: !4709, line: 42, baseType: !4712, size: 64)
!4712 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4708, file: !4709, line: 39, size: 64, elements: !4713)
!4713 = !{!4714, !4715}
!4714 = !DIDerivedType(tag: DW_TAG_member, name: "_pid", scope: !4712, file: !4709, line: 40, baseType: !2978, size: 32)
!4715 = !DIDerivedType(tag: DW_TAG_member, name: "_uid", scope: !4712, file: !4709, line: 41, baseType: !194, size: 32, offset: 32)
!4716 = !DIDerivedType(tag: DW_TAG_member, name: "_timer", scope: !4708, file: !4709, line: 50, baseType: !4717, size: 192)
!4717 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4708, file: !4709, line: 45, size: 192, elements: !4718)
!4718 = !{!4719, !4721, !4722, !4728}
!4719 = !DIDerivedType(tag: DW_TAG_member, name: "_tid", scope: !4717, file: !4709, line: 46, baseType: !4720, size: 32)
!4720 = !DIDerivedType(tag: DW_TAG_typedef, name: "__kernel_timer_t", file: !57, line: 95, baseType: !42)
!4721 = !DIDerivedType(tag: DW_TAG_member, name: "_overrun", scope: !4717, file: !4709, line: 47, baseType: !42, size: 32, offset: 32)
!4722 = !DIDerivedType(tag: DW_TAG_member, name: "_sigval", scope: !4717, file: !4709, line: 48, baseType: !4723, size: 64, offset: 64)
!4723 = !DIDerivedType(tag: DW_TAG_typedef, name: "sigval_t", file: !4709, line: 11, baseType: !4724)
!4724 = distinct !DICompositeType(tag: DW_TAG_union_type, name: "sigval", file: !4709, line: 8, size: 64, elements: !4725)
!4725 = !{!4726, !4727}
!4726 = !DIDerivedType(tag: DW_TAG_member, name: "sival_int", scope: !4724, file: !4709, line: 9, baseType: !42, size: 32)
!4727 = !DIDerivedType(tag: DW_TAG_member, name: "sival_ptr", scope: !4724, file: !4709, line: 10, baseType: !40, size: 64)
!4728 = !DIDerivedType(tag: DW_TAG_member, name: "_sys_private", scope: !4717, file: !4709, line: 49, baseType: !42, size: 32, offset: 128)
!4729 = !DIDerivedType(tag: DW_TAG_member, name: "_rt", scope: !4708, file: !4709, line: 57, baseType: !4730, size: 128)
!4730 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4708, file: !4709, line: 53, size: 128, elements: !4731)
!4731 = !{!4732, !4733, !4734}
!4732 = !DIDerivedType(tag: DW_TAG_member, name: "_pid", scope: !4730, file: !4709, line: 54, baseType: !2978, size: 32)
!4733 = !DIDerivedType(tag: DW_TAG_member, name: "_uid", scope: !4730, file: !4709, line: 55, baseType: !194, size: 32, offset: 32)
!4734 = !DIDerivedType(tag: DW_TAG_member, name: "_sigval", scope: !4730, file: !4709, line: 56, baseType: !4723, size: 64, offset: 64)
!4735 = !DIDerivedType(tag: DW_TAG_member, name: "_sigchld", scope: !4708, file: !4709, line: 66, baseType: !4736, size: 256)
!4736 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4708, file: !4709, line: 60, size: 256, elements: !4737)
!4737 = !{!4738, !4739, !4740, !4741, !4743}
!4738 = !DIDerivedType(tag: DW_TAG_member, name: "_pid", scope: !4736, file: !4709, line: 61, baseType: !2978, size: 32)
!4739 = !DIDerivedType(tag: DW_TAG_member, name: "_uid", scope: !4736, file: !4709, line: 62, baseType: !194, size: 32, offset: 32)
!4740 = !DIDerivedType(tag: DW_TAG_member, name: "_status", scope: !4736, file: !4709, line: 63, baseType: !42, size: 32, offset: 64)
!4741 = !DIDerivedType(tag: DW_TAG_member, name: "_utime", scope: !4736, file: !4709, line: 64, baseType: !4742, size: 64, offset: 128)
!4742 = !DIDerivedType(tag: DW_TAG_typedef, name: "__kernel_clock_t", file: !57, line: 94, baseType: !995)
!4743 = !DIDerivedType(tag: DW_TAG_member, name: "_stime", scope: !4736, file: !4709, line: 65, baseType: !4742, size: 64, offset: 192)
!4744 = !DIDerivedType(tag: DW_TAG_member, name: "_sigfault", scope: !4708, file: !4709, line: 100, baseType: !4745, size: 256)
!4745 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4708, file: !4709, line: 69, size: 256, elements: !4746)
!4746 = !{!4747, !4748}
!4747 = !DIDerivedType(tag: DW_TAG_member, name: "_addr", scope: !4745, file: !4709, line: 70, baseType: !40, size: 64)
!4748 = !DIDerivedType(tag: DW_TAG_member, scope: !4745, file: !4709, line: 74, baseType: !4749, size: 192, offset: 64)
!4749 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !4745, file: !4709, line: 74, size: 192, elements: !4750)
!4750 = !{!4751, !4752, !4753, !4760, !4765}
!4751 = !DIDerivedType(tag: DW_TAG_member, name: "_trapno", scope: !4749, file: !4709, line: 76, baseType: !42, size: 32)
!4752 = !DIDerivedType(tag: DW_TAG_member, name: "_addr_lsb", scope: !4749, file: !4709, line: 81, baseType: !583, size: 16)
!4753 = !DIDerivedType(tag: DW_TAG_member, name: "_addr_bnd", scope: !4749, file: !4709, line: 87, baseType: !4754, size: 192)
!4754 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4749, file: !4709, line: 83, size: 192, elements: !4755)
!4755 = !{!4756, !4758, !4759}
!4756 = !DIDerivedType(tag: DW_TAG_member, name: "_dummy_bnd", scope: !4754, file: !4709, line: 84, baseType: !4757, size: 64)
!4757 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 64, elements: !2145)
!4758 = !DIDerivedType(tag: DW_TAG_member, name: "_lower", scope: !4754, file: !4709, line: 85, baseType: !40, size: 64, offset: 64)
!4759 = !DIDerivedType(tag: DW_TAG_member, name: "_upper", scope: !4754, file: !4709, line: 86, baseType: !40, size: 64, offset: 128)
!4760 = !DIDerivedType(tag: DW_TAG_member, name: "_addr_pkey", scope: !4749, file: !4709, line: 92, baseType: !4761, size: 96)
!4761 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4749, file: !4709, line: 89, size: 96, elements: !4762)
!4762 = !{!4763, !4764}
!4763 = !DIDerivedType(tag: DW_TAG_member, name: "_dummy_pkey", scope: !4761, file: !4709, line: 90, baseType: !4757, size: 64)
!4764 = !DIDerivedType(tag: DW_TAG_member, name: "_pkey", scope: !4761, file: !4709, line: 91, baseType: !579, size: 32, offset: 64)
!4765 = !DIDerivedType(tag: DW_TAG_member, name: "_perf", scope: !4749, file: !4709, line: 98, baseType: !4766, size: 128)
!4766 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4749, file: !4709, line: 94, size: 128, elements: !4767)
!4767 = !{!4768, !4769, !4770}
!4768 = !DIDerivedType(tag: DW_TAG_member, name: "_data", scope: !4766, file: !4709, line: 95, baseType: !59, size: 64)
!4769 = !DIDerivedType(tag: DW_TAG_member, name: "_type", scope: !4766, file: !4709, line: 96, baseType: !579, size: 32, offset: 64)
!4770 = !DIDerivedType(tag: DW_TAG_member, name: "_flags", scope: !4766, file: !4709, line: 97, baseType: !579, size: 32, offset: 96)
!4771 = !DIDerivedType(tag: DW_TAG_member, name: "_sigpoll", scope: !4708, file: !4709, line: 106, baseType: !4772, size: 128)
!4772 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4708, file: !4709, line: 103, size: 128, elements: !4773)
!4773 = !{!4774, !4775}
!4774 = !DIDerivedType(tag: DW_TAG_member, name: "_band", scope: !4772, file: !4709, line: 104, baseType: !892, size: 64)
!4775 = !DIDerivedType(tag: DW_TAG_member, name: "_fd", scope: !4772, file: !4709, line: 105, baseType: !42, size: 32, offset: 64)
!4776 = !DIDerivedType(tag: DW_TAG_member, name: "_sigsys", scope: !4708, file: !4709, line: 113, baseType: !4777, size: 128)
!4777 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4708, file: !4709, line: 109, size: 128, elements: !4778)
!4778 = !{!4779, !4780, !4781}
!4779 = !DIDerivedType(tag: DW_TAG_member, name: "_call_addr", scope: !4777, file: !4709, line: 110, baseType: !40, size: 64)
!4780 = !DIDerivedType(tag: DW_TAG_member, name: "_syscall", scope: !4777, file: !4709, line: 111, baseType: !42, size: 32, offset: 64)
!4781 = !DIDerivedType(tag: DW_TAG_member, name: "_arch", scope: !4777, file: !4709, line: 112, baseType: !7, size: 32, offset: 96)
!4782 = !DIDerivedType(tag: DW_TAG_member, name: "ioac", scope: !3629, file: !1435, line: 1252, baseType: !4499, size: 448, offset: 17664)
!4783 = !DIDerivedType(tag: DW_TAG_member, name: "acct_rss_mem1", scope: !3629, file: !1435, line: 1259, baseType: !519, size: 64, offset: 18112)
!4784 = !DIDerivedType(tag: DW_TAG_member, name: "acct_vm_mem1", scope: !3629, file: !1435, line: 1261, baseType: !519, size: 64, offset: 18176)
!4785 = !DIDerivedType(tag: DW_TAG_member, name: "acct_timexpd", scope: !3629, file: !1435, line: 1263, baseType: !519, size: 64, offset: 18240)
!4786 = !DIDerivedType(tag: DW_TAG_member, name: "mems_allowed", scope: !3629, file: !1435, line: 1267, baseType: !4787, size: 64, offset: 18304)
!4787 = !DIDerivedType(tag: DW_TAG_typedef, name: "nodemask_t", file: !4788, line: 8, baseType: !4789)
!4788 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/nodemask_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "22cb5551033d7568f08324d47ea2bf2e")
!4789 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !4788, line: 8, size: 64, elements: !4790)
!4790 = !{!4791}
!4791 = !DIDerivedType(tag: DW_TAG_member, name: "bits", scope: !4789, file: !4788, line: 8, baseType: !3816, size: 64)
!4792 = !DIDerivedType(tag: DW_TAG_member, name: "mems_allowed_seq", scope: !3629, file: !1435, line: 1269, baseType: !745, size: 32, offset: 18368)
!4793 = !DIDerivedType(tag: DW_TAG_member, name: "cpuset_mem_spread_rotor", scope: !3629, file: !1435, line: 1270, baseType: !42, size: 32, offset: 18400)
!4794 = !DIDerivedType(tag: DW_TAG_member, name: "cgroups", scope: !3629, file: !1435, line: 1274, baseType: !4138, size: 64, offset: 18432)
!4795 = !DIDerivedType(tag: DW_TAG_member, name: "cg_list", scope: !3629, file: !1435, line: 1276, baseType: !117, size: 128, offset: 18496)
!4796 = !DIDerivedType(tag: DW_TAG_member, name: "robust_list", scope: !3629, file: !1435, line: 1283, baseType: !4797, size: 64, offset: 18624)
!4797 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4798, size: 64)
!4798 = !DICompositeType(tag: DW_TAG_structure_type, name: "robust_list_head", file: !1435, line: 72, flags: DIFlagFwdDecl)
!4799 = !DIDerivedType(tag: DW_TAG_member, name: "compat_robust_list", scope: !3629, file: !1435, line: 1285, baseType: !4800, size: 64, offset: 18688)
!4800 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4801, size: 64)
!4801 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "compat_robust_list_head", file: !4802, line: 392, size: 96, elements: !4803)
!4802 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/compat.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "cae4030df273c4c0f3e2cee0efe245fb")
!4803 = !{!4804, !4810, !4812}
!4804 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !4801, file: !4802, line: 393, baseType: !4805, size: 32)
!4805 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "compat_robust_list", file: !4802, line: 388, size: 32, elements: !4806)
!4806 = !{!4807}
!4807 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !4805, file: !4802, line: 389, baseType: !4808, size: 32)
!4808 = !DIDerivedType(tag: DW_TAG_typedef, name: "compat_uptr_t", file: !4809, line: 46, baseType: !578)
!4809 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/asm-generic/compat.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "789dd20e6944b4211a8be38fb9d0b245")
!4810 = !DIDerivedType(tag: DW_TAG_member, name: "futex_offset", scope: !4801, file: !4802, line: 394, baseType: !4811, size: 32, offset: 32)
!4811 = !DIDerivedType(tag: DW_TAG_typedef, name: "compat_long_t", file: !4809, line: 42, baseType: !541)
!4812 = !DIDerivedType(tag: DW_TAG_member, name: "list_op_pending", scope: !4801, file: !4802, line: 395, baseType: !4808, size: 32, offset: 64)
!4813 = !DIDerivedType(tag: DW_TAG_member, name: "pi_state_list", scope: !3629, file: !1435, line: 1287, baseType: !117, size: 128, offset: 18752)
!4814 = !DIDerivedType(tag: DW_TAG_member, name: "pi_state_cache", scope: !3629, file: !1435, line: 1288, baseType: !4815, size: 64, offset: 18880)
!4815 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4816, size: 64)
!4816 = !DICompositeType(tag: DW_TAG_structure_type, name: "futex_pi_state", file: !1435, line: 61, flags: DIFlagFwdDecl)
!4817 = !DIDerivedType(tag: DW_TAG_member, name: "futex_exit_mutex", scope: !3629, file: !1435, line: 1289, baseType: !1277, size: 256, offset: 18944)
!4818 = !DIDerivedType(tag: DW_TAG_member, name: "futex_state", scope: !3629, file: !1435, line: 1290, baseType: !7, size: 32, offset: 19200)
!4819 = !DIDerivedType(tag: DW_TAG_member, name: "perf_recursion", scope: !3629, file: !1435, line: 1293, baseType: !4820, size: 32, offset: 19232)
!4820 = !DICompositeType(tag: DW_TAG_array_type, baseType: !103, size: 32, elements: !635)
!4821 = !DIDerivedType(tag: DW_TAG_member, name: "perf_event_ctxp", scope: !3629, file: !1435, line: 1294, baseType: !4822, size: 64, offset: 19264)
!4822 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4823, size: 64)
!4823 = !DICompositeType(tag: DW_TAG_structure_type, name: "perf_event_context", file: !1435, line: 67, flags: DIFlagFwdDecl)
!4824 = !DIDerivedType(tag: DW_TAG_member, name: "perf_event_mutex", scope: !3629, file: !1435, line: 1295, baseType: !1277, size: 256, offset: 19328)
!4825 = !DIDerivedType(tag: DW_TAG_member, name: "perf_event_list", scope: !3629, file: !1435, line: 1296, baseType: !117, size: 128, offset: 19584)
!4826 = !DIDerivedType(tag: DW_TAG_member, name: "mempolicy", scope: !3629, file: !1435, line: 1303, baseType: !1433, size: 64, offset: 19712)
!4827 = !DIDerivedType(tag: DW_TAG_member, name: "il_prev", scope: !3629, file: !1435, line: 1304, baseType: !583, size: 16, offset: 19776)
!4828 = !DIDerivedType(tag: DW_TAG_member, name: "il_weight", scope: !3629, file: !1435, line: 1305, baseType: !103, size: 8, offset: 19792)
!4829 = !DIDerivedType(tag: DW_TAG_member, name: "pref_node_fork", scope: !3629, file: !1435, line: 1306, baseType: !583, size: 16, offset: 19808)
!4830 = !DIDerivedType(tag: DW_TAG_member, name: "rseq", scope: !3629, file: !1435, line: 1359, baseType: !4831, size: 64, offset: 19840)
!4831 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4832, size: 64)
!4832 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "rseq", file: !4833, line: 62, size: 256, align: 256, elements: !4834)
!4833 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/uapi/linux/rseq.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "14ee3fd2d5d9bc34e2a36070af8ac5ed")
!4834 = !{!4835, !4836, !4837, !4838, !4839, !4840, !4841}
!4835 = !DIDerivedType(tag: DW_TAG_member, name: "cpu_id_start", scope: !4832, file: !4833, line: 75, baseType: !579, size: 32)
!4836 = !DIDerivedType(tag: DW_TAG_member, name: "cpu_id", scope: !4832, file: !4833, line: 90, baseType: !579, size: 32, offset: 32)
!4837 = !DIDerivedType(tag: DW_TAG_member, name: "rseq_cs", scope: !4832, file: !4833, line: 112, baseType: !520, size: 64, offset: 64)
!4838 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !4832, file: !4833, line: 132, baseType: !579, size: 32, offset: 128)
!4839 = !DIDerivedType(tag: DW_TAG_member, name: "node_id", scope: !4832, file: !4833, line: 140, baseType: !579, size: 32, offset: 160)
!4840 = !DIDerivedType(tag: DW_TAG_member, name: "mm_cid", scope: !4832, file: !4833, line: 149, baseType: !579, size: 32, offset: 192)
!4841 = !DIDerivedType(tag: DW_TAG_member, name: "end", scope: !4832, file: !4833, line: 154, baseType: !4842, offset: 224)
!4842 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, elements: !1353)
!4843 = !DIDerivedType(tag: DW_TAG_member, name: "rseq_len", scope: !3629, file: !1435, line: 1360, baseType: !578, size: 32, offset: 19904)
!4844 = !DIDerivedType(tag: DW_TAG_member, name: "rseq_sig", scope: !3629, file: !1435, line: 1361, baseType: !578, size: 32, offset: 19936)
!4845 = !DIDerivedType(tag: DW_TAG_member, name: "rseq_event_mask", scope: !3629, file: !1435, line: 1366, baseType: !59, size: 64, offset: 19968)
!4846 = !DIDerivedType(tag: DW_TAG_member, name: "mm_cid", scope: !3629, file: !1435, line: 1370, baseType: !42, size: 32, offset: 20032)
!4847 = !DIDerivedType(tag: DW_TAG_member, name: "last_mm_cid", scope: !3629, file: !1435, line: 1371, baseType: !42, size: 32, offset: 20064)
!4848 = !DIDerivedType(tag: DW_TAG_member, name: "migrate_from_cpu", scope: !3629, file: !1435, line: 1372, baseType: !42, size: 32, offset: 20096)
!4849 = !DIDerivedType(tag: DW_TAG_member, name: "mm_cid_active", scope: !3629, file: !1435, line: 1373, baseType: !42, size: 32, offset: 20128)
!4850 = !DIDerivedType(tag: DW_TAG_member, name: "cid_work", scope: !3629, file: !1435, line: 1374, baseType: !129, size: 128, align: 64, offset: 20160)
!4851 = !DIDerivedType(tag: DW_TAG_member, name: "tlb_ubc", scope: !3629, file: !1435, line: 1377, baseType: !4852, size: 128, offset: 20288)
!4852 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "tlbflush_unmap_batch", file: !4853, line: 47, size: 128, elements: !4854)
!4853 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/mm_types_task.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "30eaa69cd218774d20f04fbb369c3345")
!4854 = !{!4855, !4860, !4861}
!4855 = !DIDerivedType(tag: DW_TAG_member, name: "arch", scope: !4852, file: !4853, line: 56, baseType: !4856, size: 64)
!4856 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "arch_tlbflush_unmap_batch", file: !4857, line: 7, size: 64, elements: !4858)
!4857 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/tlbbatch.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "a9b5c2590bec400e339804a93b4260d5")
!4858 = !{!4859}
!4859 = !DIDerivedType(tag: DW_TAG_member, name: "cpumask", scope: !4856, file: !4857, line: 12, baseType: !3813, size: 64)
!4860 = !DIDerivedType(tag: DW_TAG_member, name: "flush_required", scope: !4852, file: !4853, line: 59, baseType: !614, size: 8, offset: 64)
!4861 = !DIDerivedType(tag: DW_TAG_member, name: "writable", scope: !4852, file: !4853, line: 66, baseType: !614, size: 8, offset: 72)
!4862 = !DIDerivedType(tag: DW_TAG_member, name: "splice_pipe", scope: !3629, file: !1435, line: 1380, baseType: !3062, size: 64, offset: 20416)
!4863 = !DIDerivedType(tag: DW_TAG_member, name: "task_frag", scope: !3629, file: !1435, line: 1382, baseType: !4864, size: 128, offset: 20480)
!4864 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "page_frag", file: !4853, line: 35, size: 128, elements: !4865)
!4865 = !{!4866, !4867, !4868}
!4866 = !DIDerivedType(tag: DW_TAG_member, name: "page", scope: !4864, file: !4853, line: 36, baseType: !1060, size: 64)
!4867 = !DIDerivedType(tag: DW_TAG_member, name: "offset", scope: !4864, file: !4853, line: 38, baseType: !579, size: 32, offset: 64)
!4868 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !4864, file: !4853, line: 39, baseType: !579, size: 32, offset: 96)
!4869 = !DIDerivedType(tag: DW_TAG_member, name: "delays", scope: !3629, file: !1435, line: 1385, baseType: !4870, size: 64, offset: 20608)
!4870 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4871, size: 64)
!4871 = !DICompositeType(tag: DW_TAG_structure_type, name: "task_delay_info", file: !1435, line: 80, flags: DIFlagFwdDecl)
!4872 = !DIDerivedType(tag: DW_TAG_member, name: "nr_dirtied", scope: !3629, file: !1435, line: 1396, baseType: !42, size: 32, offset: 20672)
!4873 = !DIDerivedType(tag: DW_TAG_member, name: "nr_dirtied_pause", scope: !3629, file: !1435, line: 1397, baseType: !42, size: 32, offset: 20704)
!4874 = !DIDerivedType(tag: DW_TAG_member, name: "dirty_paused_when", scope: !3629, file: !1435, line: 1399, baseType: !59, size: 64, offset: 20736)
!4875 = !DIDerivedType(tag: DW_TAG_member, name: "timer_slack_ns", scope: !3629, file: !1435, line: 1409, baseType: !519, size: 64, offset: 20800)
!4876 = !DIDerivedType(tag: DW_TAG_member, name: "default_timer_slack_ns", scope: !3629, file: !1435, line: 1410, baseType: !519, size: 64, offset: 20864)
!4877 = !DIDerivedType(tag: DW_TAG_member, name: "trace_recursion", scope: !3629, file: !1435, line: 1457, baseType: !59, size: 64, offset: 20928)
!4878 = !DIDerivedType(tag: DW_TAG_member, name: "throttle_disk", scope: !3629, file: !1435, line: 1501, baseType: !1866, size: 64, offset: 20992)
!4879 = !DIDerivedType(tag: DW_TAG_member, name: "utask", scope: !3629, file: !1435, line: 1505, baseType: !4880, size: 64, offset: 21056)
!4880 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4881, size: 64)
!4881 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "uprobe_task", file: !327, line: 62, size: 576, elements: !4882)
!4882 = !{!4883, !4884, !4903, !4906, !4907, !4940, !4950}
!4883 = !DIDerivedType(tag: DW_TAG_member, name: "state", scope: !4881, file: !327, line: 63, baseType: !326, size: 32)
!4884 = !DIDerivedType(tag: DW_TAG_member, scope: !4881, file: !327, line: 65, baseType: !4885, size: 192, offset: 64)
!4885 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !4881, file: !327, line: 65, size: 192, elements: !4886)
!4886 = !{!4887, !4898}
!4887 = !DIDerivedType(tag: DW_TAG_member, scope: !4885, file: !327, line: 66, baseType: !4888, size: 192)
!4888 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4885, file: !327, line: 66, size: 192, elements: !4889)
!4889 = !{!4890, !4897}
!4890 = !DIDerivedType(tag: DW_TAG_member, name: "autask", scope: !4888, file: !327, line: 67, baseType: !4891, size: 128)
!4891 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "arch_uprobe_task", file: !4892, line: 50, size: 128, elements: !4893)
!4892 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/uprobes.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "d2e96ecb2d43ead22eefb8c49da2f003")
!4893 = !{!4894, !4895, !4896}
!4894 = !DIDerivedType(tag: DW_TAG_member, name: "saved_scratch_register", scope: !4891, file: !4892, line: 52, baseType: !59, size: 64)
!4895 = !DIDerivedType(tag: DW_TAG_member, name: "saved_trap_nr", scope: !4891, file: !4892, line: 54, baseType: !7, size: 32, offset: 64)
!4896 = !DIDerivedType(tag: DW_TAG_member, name: "saved_tf", scope: !4891, file: !4892, line: 55, baseType: !7, size: 32, offset: 96)
!4897 = !DIDerivedType(tag: DW_TAG_member, name: "vaddr", scope: !4888, file: !327, line: 68, baseType: !59, size: 64, offset: 128)
!4898 = !DIDerivedType(tag: DW_TAG_member, scope: !4885, file: !327, line: 71, baseType: !4899, size: 192)
!4899 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4885, file: !327, line: 71, size: 192, elements: !4900)
!4900 = !{!4901, !4902}
!4901 = !DIDerivedType(tag: DW_TAG_member, name: "dup_xol_work", scope: !4899, file: !327, line: 72, baseType: !129, size: 128, align: 64)
!4902 = !DIDerivedType(tag: DW_TAG_member, name: "dup_xol_addr", scope: !4899, file: !327, line: 73, baseType: !59, size: 64, offset: 128)
!4903 = !DIDerivedType(tag: DW_TAG_member, name: "active_uprobe", scope: !4881, file: !327, line: 77, baseType: !4904, size: 64, offset: 256)
!4904 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4905, size: 64)
!4905 = !DICompositeType(tag: DW_TAG_structure_type, name: "uprobe", file: !327, line: 19, flags: DIFlagFwdDecl)
!4906 = !DIDerivedType(tag: DW_TAG_member, name: "xol_vaddr", scope: !4881, file: !327, line: 78, baseType: !59, size: 64, offset: 320)
!4907 = !DIDerivedType(tag: DW_TAG_member, name: "auprobe", scope: !4881, file: !327, line: 80, baseType: !4908, size: 64, offset: 384)
!4908 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4909, size: 64)
!4909 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "arch_uprobe", file: !4892, line: 25, size: 256, elements: !4910)
!4910 = !{!4911, !4917, !4921}
!4911 = !DIDerivedType(tag: DW_TAG_member, scope: !4909, file: !4892, line: 26, baseType: !4912, size: 128)
!4912 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !4909, file: !4892, line: 26, size: 128, elements: !4913)
!4913 = !{!4914, !4916}
!4914 = !DIDerivedType(tag: DW_TAG_member, name: "insn", scope: !4912, file: !4892, line: 27, baseType: !4915, size: 128)
!4915 = !DICompositeType(tag: DW_TAG_array_type, baseType: !103, size: 128, elements: !4056)
!4916 = !DIDerivedType(tag: DW_TAG_member, name: "ixol", scope: !4912, file: !4892, line: 28, baseType: !4915, size: 128)
!4917 = !DIDerivedType(tag: DW_TAG_member, name: "ops", scope: !4909, file: !4892, line: 31, baseType: !4918, size: 64, offset: 128)
!4918 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4919, size: 64)
!4919 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !4920)
!4920 = !DICompositeType(tag: DW_TAG_structure_type, name: "uprobe_xol_ops", file: !4892, line: 23, flags: DIFlagFwdDecl)
!4921 = !DIDerivedType(tag: DW_TAG_member, scope: !4909, file: !4892, line: 33, baseType: !4922, size: 64, offset: 192)
!4922 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !4909, file: !4892, line: 33, size: 64, elements: !4923)
!4923 = !{!4924, !4930, !4935}
!4924 = !DIDerivedType(tag: DW_TAG_member, name: "branch", scope: !4922, file: !4892, line: 38, baseType: !4925, size: 64)
!4925 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4922, file: !4892, line: 34, size: 64, elements: !4926)
!4926 = !{!4927, !4928, !4929}
!4927 = !DIDerivedType(tag: DW_TAG_member, name: "offs", scope: !4925, file: !4892, line: 35, baseType: !541, size: 32)
!4928 = !DIDerivedType(tag: DW_TAG_member, name: "ilen", scope: !4925, file: !4892, line: 36, baseType: !103, size: 8, offset: 32)
!4929 = !DIDerivedType(tag: DW_TAG_member, name: "opc1", scope: !4925, file: !4892, line: 37, baseType: !103, size: 8, offset: 40)
!4930 = !DIDerivedType(tag: DW_TAG_member, name: "defparam", scope: !4922, file: !4892, line: 42, baseType: !4931, size: 16)
!4931 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4922, file: !4892, line: 39, size: 16, elements: !4932)
!4932 = !{!4933, !4934}
!4933 = !DIDerivedType(tag: DW_TAG_member, name: "fixups", scope: !4931, file: !4892, line: 40, baseType: !103, size: 8)
!4934 = !DIDerivedType(tag: DW_TAG_member, name: "ilen", scope: !4931, file: !4892, line: 41, baseType: !103, size: 8, offset: 8)
!4935 = !DIDerivedType(tag: DW_TAG_member, name: "push", scope: !4922, file: !4892, line: 46, baseType: !4936, size: 16)
!4936 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !4922, file: !4892, line: 43, size: 16, elements: !4937)
!4937 = !{!4938, !4939}
!4938 = !DIDerivedType(tag: DW_TAG_member, name: "reg_offset", scope: !4936, file: !4892, line: 44, baseType: !103, size: 8)
!4939 = !DIDerivedType(tag: DW_TAG_member, name: "ilen", scope: !4936, file: !4892, line: 45, baseType: !103, size: 8, offset: 8)
!4940 = !DIDerivedType(tag: DW_TAG_member, name: "return_instances", scope: !4881, file: !327, line: 82, baseType: !4941, size: 64, offset: 448)
!4941 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4942, size: 64)
!4942 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "return_instance", file: !327, line: 86, size: 384, elements: !4943)
!4943 = !{!4944, !4945, !4946, !4947, !4948, !4949}
!4944 = !DIDerivedType(tag: DW_TAG_member, name: "uprobe", scope: !4942, file: !327, line: 87, baseType: !4904, size: 64)
!4945 = !DIDerivedType(tag: DW_TAG_member, name: "func", scope: !4942, file: !327, line: 88, baseType: !59, size: 64, offset: 64)
!4946 = !DIDerivedType(tag: DW_TAG_member, name: "stack", scope: !4942, file: !327, line: 89, baseType: !59, size: 64, offset: 128)
!4947 = !DIDerivedType(tag: DW_TAG_member, name: "orig_ret_vaddr", scope: !4942, file: !327, line: 90, baseType: !59, size: 64, offset: 192)
!4948 = !DIDerivedType(tag: DW_TAG_member, name: "chained", scope: !4942, file: !327, line: 91, baseType: !614, size: 8, offset: 256)
!4949 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !4942, file: !327, line: 93, baseType: !4941, size: 64, offset: 320)
!4950 = !DIDerivedType(tag: DW_TAG_member, name: "depth", scope: !4881, file: !327, line: 83, baseType: !7, size: 32, offset: 512)
!4951 = !DIDerivedType(tag: DW_TAG_member, name: "kmap_ctrl", scope: !3629, file: !1435, line: 1511, baseType: !4952, offset: 21120)
!4952 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kmap_ctrl", file: !1435, line: 771, elements: !1201)
!4953 = !DIDerivedType(tag: DW_TAG_member, name: "rcu", scope: !3629, file: !1435, line: 1518, baseType: !129, size: 128, align: 64, offset: 21120)
!4954 = !DIDerivedType(tag: DW_TAG_member, name: "rcu_users", scope: !3629, file: !1435, line: 1519, baseType: !533, size: 32, offset: 21248)
!4955 = !DIDerivedType(tag: DW_TAG_member, name: "pagefault_disabled", scope: !3629, file: !1435, line: 1520, baseType: !42, size: 32, offset: 21280)
!4956 = !DIDerivedType(tag: DW_TAG_member, name: "oom_reaper_list", scope: !3629, file: !1435, line: 1522, baseType: !3628, size: 64, offset: 21312)
!4957 = !DIDerivedType(tag: DW_TAG_member, name: "oom_reaper_timer", scope: !3629, file: !1435, line: 1523, baseType: !2070, size: 320, offset: 21376)
!4958 = !DIDerivedType(tag: DW_TAG_member, name: "stack_vm_area", scope: !3629, file: !1435, line: 1526, baseType: !4959, size: 64, offset: 21696)
!4959 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4960, size: 64)
!4960 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "vm_struct", file: !4961, line: 52, size: 512, elements: !4962)
!4961 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/vmalloc.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "78cccaf8d7381e263bf8e586aeae21c6")
!4962 = !{!4963, !4964, !4965, !4966, !4967, !4969, !4970, !4971, !4973}
!4963 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !4960, file: !4961, line: 53, baseType: !4959, size: 64)
!4964 = !DIDerivedType(tag: DW_TAG_member, name: "addr", scope: !4960, file: !4961, line: 54, baseType: !40, size: 64, offset: 64)
!4965 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !4960, file: !4961, line: 55, baseType: !59, size: 64, offset: 128)
!4966 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !4960, file: !4961, line: 56, baseType: !59, size: 64, offset: 192)
!4967 = !DIDerivedType(tag: DW_TAG_member, name: "pages", scope: !4960, file: !4961, line: 57, baseType: !4968, size: 64, offset: 256)
!4968 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1060, size: 64)
!4969 = !DIDerivedType(tag: DW_TAG_member, name: "page_order", scope: !4960, file: !4961, line: 59, baseType: !7, size: 32, offset: 320)
!4970 = !DIDerivedType(tag: DW_TAG_member, name: "nr_pages", scope: !4960, file: !4961, line: 61, baseType: !7, size: 32, offset: 352)
!4971 = !DIDerivedType(tag: DW_TAG_member, name: "phys_addr", scope: !4960, file: !4961, line: 62, baseType: !4972, size: 64, offset: 384)
!4972 = !DIDerivedType(tag: DW_TAG_typedef, name: "phys_addr_t", file: !45, line: 162, baseType: !519)
!4973 = !DIDerivedType(tag: DW_TAG_member, name: "caller", scope: !4960, file: !4961, line: 63, baseType: !1298, size: 64, offset: 448)
!4974 = !DIDerivedType(tag: DW_TAG_member, name: "stack_refcount", scope: !3629, file: !1435, line: 1530, baseType: !533, size: 32, offset: 21760)
!4975 = !DIDerivedType(tag: DW_TAG_member, name: "security", scope: !3629, file: !1435, line: 1537, baseType: !40, size: 64, offset: 21824)
!4976 = !DIDerivedType(tag: DW_TAG_member, name: "bpf_net_context", scope: !3629, file: !1435, line: 1546, baseType: !4977, size: 64, offset: 21888)
!4977 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4978, size: 64)
!4978 = !DICompositeType(tag: DW_TAG_structure_type, name: "bpf_net_context", file: !1435, line: 57, flags: DIFlagFwdDecl)
!4979 = !DIDerivedType(tag: DW_TAG_member, name: "mce_vaddr", scope: !3629, file: !1435, line: 1554, baseType: !40, size: 64, offset: 21952)
!4980 = !DIDerivedType(tag: DW_TAG_member, name: "mce_kflags", scope: !3629, file: !1435, line: 1555, baseType: !520, size: 64, offset: 22016)
!4981 = !DIDerivedType(tag: DW_TAG_member, name: "mce_addr", scope: !3629, file: !1435, line: 1556, baseType: !519, size: 64, offset: 22080)
!4982 = !DIDerivedType(tag: DW_TAG_member, name: "mce_ripv", scope: !3629, file: !1435, line: 1557, baseType: !520, size: 1, offset: 22144, flags: DIFlagBitField, extraData: i64 22144)
!4983 = !DIDerivedType(tag: DW_TAG_member, name: "mce_whole_page", scope: !3629, file: !1435, line: 1558, baseType: !520, size: 1, offset: 22145, flags: DIFlagBitField, extraData: i64 22144)
!4984 = !DIDerivedType(tag: DW_TAG_member, name: "__mce_reserved", scope: !3629, file: !1435, line: 1559, baseType: !520, size: 62, offset: 22146, flags: DIFlagBitField, extraData: i64 22144)
!4985 = !DIDerivedType(tag: DW_TAG_member, name: "mce_kill_me", scope: !3629, file: !1435, line: 1560, baseType: !129, size: 128, align: 64, offset: 22208)
!4986 = !DIDerivedType(tag: DW_TAG_member, name: "mce_count", scope: !3629, file: !1435, line: 1561, baseType: !42, size: 32, offset: 22336)
!4987 = !DIDerivedType(tag: DW_TAG_member, name: "kretprobe_instances", scope: !3629, file: !1435, line: 1565, baseType: !4988, size: 64, offset: 22400)
!4988 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "llist_head", file: !3652, line: 56, size: 64, elements: !4989)
!4989 = !{!4990}
!4990 = !DIDerivedType(tag: DW_TAG_member, name: "first", scope: !4988, file: !3652, line: 57, baseType: !3655, size: 64)
!4991 = !DIDerivedType(tag: DW_TAG_member, name: "rethooks", scope: !3629, file: !1435, line: 1568, baseType: !4988, size: 64, offset: 22464)
!4992 = !DIDerivedType(tag: DW_TAG_member, name: "l1d_flush_kill", scope: !3629, file: !1435, line: 1578, baseType: !129, size: 128, align: 64, offset: 22528)
!4993 = !DIDerivedType(tag: DW_TAG_member, name: "thread", scope: !3629, file: !1435, line: 1602, baseType: !4994, size: 35328, offset: 23040)
!4994 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "thread_struct", file: !4995, line: 436, size: 35328, elements: !4996)
!4995 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/processor.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "ab8cb8a39c0c5713d4568d2792a01c4e")
!4996 = !{!4997, !5015, !5016, !5017, !5018, !5019, !5020, !5021, !5022, !5026, !5027, !5028, !5029, !5030, !5031, !5034, !5035, !5036, !5037}
!4997 = !DIDerivedType(tag: DW_TAG_member, name: "tls_array", scope: !4994, file: !4995, line: 438, baseType: !4998, size: 192)
!4998 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4999, size: 192, elements: !962)
!4999 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "desc_struct", file: !5000, line: 66, size: 64, elements: !5001)
!5000 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/desc_defs.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "7fc87a91d323a41075bf2971256620a7")
!5001 = !{!5002, !5003, !5004, !5005, !5006, !5007, !5008, !5009, !5010, !5011, !5012, !5013, !5014}
!5002 = !DIDerivedType(tag: DW_TAG_member, name: "limit0", scope: !4999, file: !5000, line: 67, baseType: !113, size: 16)
!5003 = !DIDerivedType(tag: DW_TAG_member, name: "base0", scope: !4999, file: !5000, line: 68, baseType: !113, size: 16, offset: 16)
!5004 = !DIDerivedType(tag: DW_TAG_member, name: "base1", scope: !4999, file: !5000, line: 69, baseType: !113, size: 8, offset: 32, flags: DIFlagBitField, extraData: i64 32)
!5005 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !4999, file: !5000, line: 69, baseType: !113, size: 4, offset: 40, flags: DIFlagBitField, extraData: i64 32)
!5006 = !DIDerivedType(tag: DW_TAG_member, name: "s", scope: !4999, file: !5000, line: 69, baseType: !113, size: 1, offset: 44, flags: DIFlagBitField, extraData: i64 32)
!5007 = !DIDerivedType(tag: DW_TAG_member, name: "dpl", scope: !4999, file: !5000, line: 69, baseType: !113, size: 2, offset: 45, flags: DIFlagBitField, extraData: i64 32)
!5008 = !DIDerivedType(tag: DW_TAG_member, name: "p", scope: !4999, file: !5000, line: 69, baseType: !113, size: 1, offset: 47, flags: DIFlagBitField, extraData: i64 32)
!5009 = !DIDerivedType(tag: DW_TAG_member, name: "limit1", scope: !4999, file: !5000, line: 70, baseType: !113, size: 4, offset: 48, flags: DIFlagBitField, extraData: i64 32)
!5010 = !DIDerivedType(tag: DW_TAG_member, name: "avl", scope: !4999, file: !5000, line: 70, baseType: !113, size: 1, offset: 52, flags: DIFlagBitField, extraData: i64 32)
!5011 = !DIDerivedType(tag: DW_TAG_member, name: "l", scope: !4999, file: !5000, line: 70, baseType: !113, size: 1, offset: 53, flags: DIFlagBitField, extraData: i64 32)
!5012 = !DIDerivedType(tag: DW_TAG_member, name: "d", scope: !4999, file: !5000, line: 70, baseType: !113, size: 1, offset: 54, flags: DIFlagBitField, extraData: i64 32)
!5013 = !DIDerivedType(tag: DW_TAG_member, name: "g", scope: !4999, file: !5000, line: 70, baseType: !113, size: 1, offset: 55, flags: DIFlagBitField, extraData: i64 32)
!5014 = !DIDerivedType(tag: DW_TAG_member, name: "base2", scope: !4999, file: !5000, line: 70, baseType: !113, size: 8, offset: 56, flags: DIFlagBitField, extraData: i64 32)
!5015 = !DIDerivedType(tag: DW_TAG_member, name: "sp", scope: !4994, file: !4995, line: 442, baseType: !59, size: 64, offset: 192)
!5016 = !DIDerivedType(tag: DW_TAG_member, name: "es", scope: !4994, file: !4995, line: 446, baseType: !46, size: 16, offset: 256)
!5017 = !DIDerivedType(tag: DW_TAG_member, name: "ds", scope: !4994, file: !4995, line: 447, baseType: !46, size: 16, offset: 272)
!5018 = !DIDerivedType(tag: DW_TAG_member, name: "fsindex", scope: !4994, file: !4995, line: 448, baseType: !46, size: 16, offset: 288)
!5019 = !DIDerivedType(tag: DW_TAG_member, name: "gsindex", scope: !4994, file: !4995, line: 449, baseType: !46, size: 16, offset: 304)
!5020 = !DIDerivedType(tag: DW_TAG_member, name: "fsbase", scope: !4994, file: !4995, line: 453, baseType: !59, size: 64, offset: 320)
!5021 = !DIDerivedType(tag: DW_TAG_member, name: "gsbase", scope: !4994, file: !4995, line: 454, baseType: !59, size: 64, offset: 384)
!5022 = !DIDerivedType(tag: DW_TAG_member, name: "ptrace_bps", scope: !4994, file: !4995, line: 465, baseType: !5023, size: 256, offset: 448)
!5023 = !DICompositeType(tag: DW_TAG_array_type, baseType: !5024, size: 256, elements: !635)
!5024 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5025, size: 64)
!5025 = !DICompositeType(tag: DW_TAG_structure_type, name: "perf_event", file: !4995, line: 434, flags: DIFlagFwdDecl)
!5026 = !DIDerivedType(tag: DW_TAG_member, name: "virtual_dr6", scope: !4994, file: !4995, line: 467, baseType: !59, size: 64, offset: 704)
!5027 = !DIDerivedType(tag: DW_TAG_member, name: "ptrace_dr7", scope: !4994, file: !4995, line: 469, baseType: !59, size: 64, offset: 768)
!5028 = !DIDerivedType(tag: DW_TAG_member, name: "cr2", scope: !4994, file: !4995, line: 471, baseType: !59, size: 64, offset: 832)
!5029 = !DIDerivedType(tag: DW_TAG_member, name: "trap_nr", scope: !4994, file: !4995, line: 472, baseType: !59, size: 64, offset: 896)
!5030 = !DIDerivedType(tag: DW_TAG_member, name: "error_code", scope: !4994, file: !4995, line: 473, baseType: !59, size: 64, offset: 960)
!5031 = !DIDerivedType(tag: DW_TAG_member, name: "io_bitmap", scope: !4994, file: !4995, line: 479, baseType: !5032, size: 64, offset: 1024)
!5032 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5033, size: 64)
!5033 = !DICompositeType(tag: DW_TAG_structure_type, name: "io_bitmap", file: !4995, line: 10, flags: DIFlagFwdDecl)
!5034 = !DIDerivedType(tag: DW_TAG_member, name: "iopl_emul", scope: !4994, file: !4995, line: 486, baseType: !59, size: 64, offset: 1088)
!5035 = !DIDerivedType(tag: DW_TAG_member, name: "iopl_warn", scope: !4994, file: !4995, line: 488, baseType: !7, size: 1, offset: 1152, flags: DIFlagBitField, extraData: i64 1152)
!5036 = !DIDerivedType(tag: DW_TAG_member, name: "pkru", scope: !4994, file: !4995, line: 497, baseType: !578, size: 32, offset: 1184)
!5037 = !DIDerivedType(tag: DW_TAG_member, name: "fpu", scope: !4994, file: !4995, line: 507, baseType: !5038, size: 33792, offset: 1536)
!5038 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "fpu", file: !5039, line: 450, size: 33792, elements: !5040)
!5039 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/fpu/types.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "34b881908b6f8375404224504e729738")
!5040 = !{!5041, !5042, !5043, !5203, !5204, !5210, !5211}
!5041 = !DIDerivedType(tag: DW_TAG_member, name: "last_cpu", scope: !5038, file: !5039, line: 463, baseType: !7, size: 32)
!5042 = !DIDerivedType(tag: DW_TAG_member, name: "avx512_timestamp", scope: !5038, file: !5039, line: 470, baseType: !59, size: 64, offset: 64)
!5043 = !DIDerivedType(tag: DW_TAG_member, name: "fpstate", scope: !5038, file: !5039, line: 478, baseType: !5044, size: 64, offset: 128)
!5044 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5045, size: 64)
!5045 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "fpstate", file: !5039, line: 358, size: 33280, align: 512, elements: !5046)
!5046 = !{!5047, !5048, !5049, !5050, !5051, !5052, !5053, !5054, !5055, !5056}
!5047 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !5045, file: !5039, line: 360, baseType: !7, size: 32)
!5048 = !DIDerivedType(tag: DW_TAG_member, name: "user_size", scope: !5045, file: !5039, line: 363, baseType: !7, size: 32, offset: 32)
!5049 = !DIDerivedType(tag: DW_TAG_member, name: "xfeatures", scope: !5045, file: !5039, line: 366, baseType: !519, size: 64, offset: 64)
!5050 = !DIDerivedType(tag: DW_TAG_member, name: "user_xfeatures", scope: !5045, file: !5039, line: 369, baseType: !519, size: 64, offset: 128)
!5051 = !DIDerivedType(tag: DW_TAG_member, name: "xfd", scope: !5045, file: !5039, line: 372, baseType: !519, size: 64, offset: 192)
!5052 = !DIDerivedType(tag: DW_TAG_member, name: "is_valloc", scope: !5045, file: !5039, line: 375, baseType: !7, size: 1, offset: 256, flags: DIFlagBitField, extraData: i64 256)
!5053 = !DIDerivedType(tag: DW_TAG_member, name: "is_guest", scope: !5045, file: !5039, line: 378, baseType: !7, size: 1, offset: 257, flags: DIFlagBitField, extraData: i64 256)
!5054 = !DIDerivedType(tag: DW_TAG_member, name: "is_confidential", scope: !5045, file: !5039, line: 393, baseType: !7, size: 1, offset: 258, flags: DIFlagBitField, extraData: i64 256)
!5055 = !DIDerivedType(tag: DW_TAG_member, name: "in_use", scope: !5045, file: !5039, line: 396, baseType: !7, size: 1, offset: 259, flags: DIFlagBitField, extraData: i64 256)
!5056 = !DIDerivedType(tag: DW_TAG_member, name: "regs", scope: !5045, file: !5039, line: 399, baseType: !5057, size: 32768, offset: 512)
!5057 = distinct !DICompositeType(tag: DW_TAG_union_type, name: "fpregs_state", file: !5039, line: 350, size: 32768, elements: !5058)
!5058 = !{!5059, !5074, !5111, !5188, !5201}
!5059 = !DIDerivedType(tag: DW_TAG_member, name: "fsave", scope: !5057, file: !5039, line: 351, baseType: !5060, size: 896)
!5060 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "fregs_state", file: !5039, line: 14, size: 896, elements: !5061)
!5061 = !{!5062, !5063, !5064, !5065, !5066, !5067, !5068, !5069, !5073}
!5062 = !DIDerivedType(tag: DW_TAG_member, name: "cwd", scope: !5060, file: !5039, line: 15, baseType: !578, size: 32)
!5063 = !DIDerivedType(tag: DW_TAG_member, name: "swd", scope: !5060, file: !5039, line: 16, baseType: !578, size: 32, offset: 32)
!5064 = !DIDerivedType(tag: DW_TAG_member, name: "twd", scope: !5060, file: !5039, line: 17, baseType: !578, size: 32, offset: 64)
!5065 = !DIDerivedType(tag: DW_TAG_member, name: "fip", scope: !5060, file: !5039, line: 18, baseType: !578, size: 32, offset: 96)
!5066 = !DIDerivedType(tag: DW_TAG_member, name: "fcs", scope: !5060, file: !5039, line: 19, baseType: !578, size: 32, offset: 128)
!5067 = !DIDerivedType(tag: DW_TAG_member, name: "foo", scope: !5060, file: !5039, line: 20, baseType: !578, size: 32, offset: 160)
!5068 = !DIDerivedType(tag: DW_TAG_member, name: "fos", scope: !5060, file: !5039, line: 21, baseType: !578, size: 32, offset: 192)
!5069 = !DIDerivedType(tag: DW_TAG_member, name: "st_space", scope: !5060, file: !5039, line: 24, baseType: !5070, size: 640, offset: 224)
!5070 = !DICompositeType(tag: DW_TAG_array_type, baseType: !578, size: 640, elements: !5071)
!5071 = !{!5072}
!5072 = !DISubrange(count: 20)
!5073 = !DIDerivedType(tag: DW_TAG_member, name: "status", scope: !5060, file: !5039, line: 27, baseType: !578, size: 32, offset: 864)
!5074 = !DIDerivedType(tag: DW_TAG_member, name: "fxsave", scope: !5057, file: !5039, line: 352, baseType: !5075, size: 4096, align: 128)
!5075 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "fxregs_state", file: !5039, line: 36, size: 4096, align: 128, elements: !5076)
!5076 = !{!5077, !5078, !5079, !5080, !5081, !5096, !5097, !5098, !5100, !5102, !5106}
!5077 = !DIDerivedType(tag: DW_TAG_member, name: "cwd", scope: !5075, file: !5039, line: 37, baseType: !113, size: 16)
!5078 = !DIDerivedType(tag: DW_TAG_member, name: "swd", scope: !5075, file: !5039, line: 38, baseType: !113, size: 16, offset: 16)
!5079 = !DIDerivedType(tag: DW_TAG_member, name: "twd", scope: !5075, file: !5039, line: 39, baseType: !113, size: 16, offset: 32)
!5080 = !DIDerivedType(tag: DW_TAG_member, name: "fop", scope: !5075, file: !5039, line: 40, baseType: !113, size: 16, offset: 48)
!5081 = !DIDerivedType(tag: DW_TAG_member, scope: !5075, file: !5039, line: 41, baseType: !5082, size: 128, offset: 64)
!5082 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !5075, file: !5039, line: 41, size: 128, elements: !5083)
!5083 = !{!5084, !5089}
!5084 = !DIDerivedType(tag: DW_TAG_member, scope: !5082, file: !5039, line: 42, baseType: !5085, size: 128)
!5085 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !5082, file: !5039, line: 42, size: 128, elements: !5086)
!5086 = !{!5087, !5088}
!5087 = !DIDerivedType(tag: DW_TAG_member, name: "rip", scope: !5085, file: !5039, line: 43, baseType: !519, size: 64)
!5088 = !DIDerivedType(tag: DW_TAG_member, name: "rdp", scope: !5085, file: !5039, line: 44, baseType: !519, size: 64, offset: 64)
!5089 = !DIDerivedType(tag: DW_TAG_member, scope: !5082, file: !5039, line: 46, baseType: !5090, size: 128)
!5090 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !5082, file: !5039, line: 46, size: 128, elements: !5091)
!5091 = !{!5092, !5093, !5094, !5095}
!5092 = !DIDerivedType(tag: DW_TAG_member, name: "fip", scope: !5090, file: !5039, line: 47, baseType: !578, size: 32)
!5093 = !DIDerivedType(tag: DW_TAG_member, name: "fcs", scope: !5090, file: !5039, line: 48, baseType: !578, size: 32, offset: 32)
!5094 = !DIDerivedType(tag: DW_TAG_member, name: "foo", scope: !5090, file: !5039, line: 49, baseType: !578, size: 32, offset: 64)
!5095 = !DIDerivedType(tag: DW_TAG_member, name: "fos", scope: !5090, file: !5039, line: 50, baseType: !578, size: 32, offset: 96)
!5096 = !DIDerivedType(tag: DW_TAG_member, name: "mxcsr", scope: !5075, file: !5039, line: 53, baseType: !578, size: 32, offset: 192)
!5097 = !DIDerivedType(tag: DW_TAG_member, name: "mxcsr_mask", scope: !5075, file: !5039, line: 54, baseType: !578, size: 32, offset: 224)
!5098 = !DIDerivedType(tag: DW_TAG_member, name: "st_space", scope: !5075, file: !5039, line: 57, baseType: !5099, size: 1024, offset: 256)
!5099 = !DICompositeType(tag: DW_TAG_array_type, baseType: !578, size: 1024, elements: !4546)
!5100 = !DIDerivedType(tag: DW_TAG_member, name: "xmm_space", scope: !5075, file: !5039, line: 60, baseType: !5101, size: 2048, offset: 1280)
!5101 = !DICompositeType(tag: DW_TAG_array_type, baseType: !578, size: 2048, elements: !966)
!5102 = !DIDerivedType(tag: DW_TAG_member, name: "padding", scope: !5075, file: !5039, line: 62, baseType: !5103, size: 384, offset: 3328)
!5103 = !DICompositeType(tag: DW_TAG_array_type, baseType: !578, size: 384, elements: !5104)
!5104 = !{!5105}
!5105 = !DISubrange(count: 12)
!5106 = !DIDerivedType(tag: DW_TAG_member, scope: !5075, file: !5039, line: 64, baseType: !5107, size: 384, offset: 3712)
!5107 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !5075, file: !5039, line: 64, size: 384, elements: !5108)
!5108 = !{!5109, !5110}
!5109 = !DIDerivedType(tag: DW_TAG_member, name: "padding1", scope: !5107, file: !5039, line: 65, baseType: !5103, size: 384)
!5110 = !DIDerivedType(tag: DW_TAG_member, name: "sw_reserved", scope: !5107, file: !5039, line: 66, baseType: !5103, size: 384)
!5111 = !DIDerivedType(tag: DW_TAG_member, name: "soft", scope: !5057, file: !5039, line: 353, baseType: !5112, size: 1088)
!5112 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "swregs_state", file: !5039, line: 81, size: 1088, elements: !5113)
!5113 = !{!5114, !5115, !5116, !5117, !5118, !5119, !5120, !5121, !5122, !5123, !5124, !5125, !5126, !5127, !5128, !5187}
!5114 = !DIDerivedType(tag: DW_TAG_member, name: "cwd", scope: !5112, file: !5039, line: 82, baseType: !578, size: 32)
!5115 = !DIDerivedType(tag: DW_TAG_member, name: "swd", scope: !5112, file: !5039, line: 83, baseType: !578, size: 32, offset: 32)
!5116 = !DIDerivedType(tag: DW_TAG_member, name: "twd", scope: !5112, file: !5039, line: 84, baseType: !578, size: 32, offset: 64)
!5117 = !DIDerivedType(tag: DW_TAG_member, name: "fip", scope: !5112, file: !5039, line: 85, baseType: !578, size: 32, offset: 96)
!5118 = !DIDerivedType(tag: DW_TAG_member, name: "fcs", scope: !5112, file: !5039, line: 86, baseType: !578, size: 32, offset: 128)
!5119 = !DIDerivedType(tag: DW_TAG_member, name: "foo", scope: !5112, file: !5039, line: 87, baseType: !578, size: 32, offset: 160)
!5120 = !DIDerivedType(tag: DW_TAG_member, name: "fos", scope: !5112, file: !5039, line: 88, baseType: !578, size: 32, offset: 192)
!5121 = !DIDerivedType(tag: DW_TAG_member, name: "st_space", scope: !5112, file: !5039, line: 90, baseType: !5070, size: 640, offset: 224)
!5122 = !DIDerivedType(tag: DW_TAG_member, name: "ftop", scope: !5112, file: !5039, line: 91, baseType: !103, size: 8, offset: 864)
!5123 = !DIDerivedType(tag: DW_TAG_member, name: "changed", scope: !5112, file: !5039, line: 92, baseType: !103, size: 8, offset: 872)
!5124 = !DIDerivedType(tag: DW_TAG_member, name: "lookahead", scope: !5112, file: !5039, line: 93, baseType: !103, size: 8, offset: 880)
!5125 = !DIDerivedType(tag: DW_TAG_member, name: "no_update", scope: !5112, file: !5039, line: 94, baseType: !103, size: 8, offset: 888)
!5126 = !DIDerivedType(tag: DW_TAG_member, name: "rm", scope: !5112, file: !5039, line: 95, baseType: !103, size: 8, offset: 896)
!5127 = !DIDerivedType(tag: DW_TAG_member, name: "alimit", scope: !5112, file: !5039, line: 96, baseType: !103, size: 8, offset: 904)
!5128 = !DIDerivedType(tag: DW_TAG_member, name: "info", scope: !5112, file: !5039, line: 97, baseType: !5129, size: 64, offset: 960)
!5129 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5130, size: 64)
!5130 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "math_emu_info", file: !5131, line: 11, size: 128, elements: !5132)
!5131 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/math_emu.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "eb60f5d824df9ca37aa30487953996a9")
!5132 = !{!5133, !5134}
!5133 = !DIDerivedType(tag: DW_TAG_member, name: "___orig_eip", scope: !5130, file: !5131, line: 12, baseType: !892, size: 64)
!5134 = !DIDerivedType(tag: DW_TAG_member, name: "regs", scope: !5130, file: !5131, line: 13, baseType: !5135, size: 64, offset: 64)
!5135 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5136, size: 64)
!5136 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "pt_regs", file: !5137, line: 103, size: 1344, elements: !5138)
!5137 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/ptrace.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "74429e3c5dac35fb5f77bb31b19ab45c")
!5138 = !{!5139, !5140, !5141, !5142, !5143, !5144, !5145, !5146, !5147, !5148, !5149, !5150, !5151, !5152, !5153, !5154, !5155, !5156, !5167, !5168, !5169}
!5139 = !DIDerivedType(tag: DW_TAG_member, name: "r15", scope: !5136, file: !5137, line: 109, baseType: !59, size: 64)
!5140 = !DIDerivedType(tag: DW_TAG_member, name: "r14", scope: !5136, file: !5137, line: 110, baseType: !59, size: 64, offset: 64)
!5141 = !DIDerivedType(tag: DW_TAG_member, name: "r13", scope: !5136, file: !5137, line: 111, baseType: !59, size: 64, offset: 128)
!5142 = !DIDerivedType(tag: DW_TAG_member, name: "r12", scope: !5136, file: !5137, line: 112, baseType: !59, size: 64, offset: 192)
!5143 = !DIDerivedType(tag: DW_TAG_member, name: "bp", scope: !5136, file: !5137, line: 113, baseType: !59, size: 64, offset: 256)
!5144 = !DIDerivedType(tag: DW_TAG_member, name: "bx", scope: !5136, file: !5137, line: 114, baseType: !59, size: 64, offset: 320)
!5145 = !DIDerivedType(tag: DW_TAG_member, name: "r11", scope: !5136, file: !5137, line: 117, baseType: !59, size: 64, offset: 384)
!5146 = !DIDerivedType(tag: DW_TAG_member, name: "r10", scope: !5136, file: !5137, line: 118, baseType: !59, size: 64, offset: 448)
!5147 = !DIDerivedType(tag: DW_TAG_member, name: "r9", scope: !5136, file: !5137, line: 119, baseType: !59, size: 64, offset: 512)
!5148 = !DIDerivedType(tag: DW_TAG_member, name: "r8", scope: !5136, file: !5137, line: 120, baseType: !59, size: 64, offset: 576)
!5149 = !DIDerivedType(tag: DW_TAG_member, name: "ax", scope: !5136, file: !5137, line: 121, baseType: !59, size: 64, offset: 640)
!5150 = !DIDerivedType(tag: DW_TAG_member, name: "cx", scope: !5136, file: !5137, line: 122, baseType: !59, size: 64, offset: 704)
!5151 = !DIDerivedType(tag: DW_TAG_member, name: "dx", scope: !5136, file: !5137, line: 123, baseType: !59, size: 64, offset: 768)
!5152 = !DIDerivedType(tag: DW_TAG_member, name: "si", scope: !5136, file: !5137, line: 124, baseType: !59, size: 64, offset: 832)
!5153 = !DIDerivedType(tag: DW_TAG_member, name: "di", scope: !5136, file: !5137, line: 125, baseType: !59, size: 64, offset: 896)
!5154 = !DIDerivedType(tag: DW_TAG_member, name: "orig_ax", scope: !5136, file: !5137, line: 139, baseType: !59, size: 64, offset: 960)
!5155 = !DIDerivedType(tag: DW_TAG_member, name: "ip", scope: !5136, file: !5137, line: 142, baseType: !59, size: 64, offset: 1024)
!5156 = !DIDerivedType(tag: DW_TAG_member, scope: !5136, file: !5137, line: 144, baseType: !5157, size: 64, offset: 1088)
!5157 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !5136, file: !5137, line: 144, size: 64, elements: !5158)
!5158 = !{!5159, !5160, !5161}
!5159 = !DIDerivedType(tag: DW_TAG_member, name: "cs", scope: !5157, file: !5137, line: 146, baseType: !113, size: 16)
!5160 = !DIDerivedType(tag: DW_TAG_member, name: "csx", scope: !5157, file: !5137, line: 148, baseType: !519, size: 64)
!5161 = !DIDerivedType(tag: DW_TAG_member, name: "fred_cs", scope: !5157, file: !5137, line: 150, baseType: !5162, size: 64)
!5162 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "fred_cs", file: !5137, line: 59, size: 64, elements: !5163)
!5163 = !{!5164, !5165, !5166}
!5164 = !DIDerivedType(tag: DW_TAG_member, name: "cs", scope: !5162, file: !5137, line: 61, baseType: !519, size: 16, flags: DIFlagBitField, extraData: i64 0)
!5165 = !DIDerivedType(tag: DW_TAG_member, name: "sl", scope: !5162, file: !5137, line: 63, baseType: !519, size: 2, offset: 16, flags: DIFlagBitField, extraData: i64 0)
!5166 = !DIDerivedType(tag: DW_TAG_member, name: "wfe", scope: !5162, file: !5137, line: 65, baseType: !519, size: 1, offset: 18, flags: DIFlagBitField, extraData: i64 0)
!5167 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !5136, file: !5137, line: 153, baseType: !59, size: 64, offset: 1152)
!5168 = !DIDerivedType(tag: DW_TAG_member, name: "sp", scope: !5136, file: !5137, line: 154, baseType: !59, size: 64, offset: 1216)
!5169 = !DIDerivedType(tag: DW_TAG_member, scope: !5136, file: !5137, line: 156, baseType: !5170, size: 64, offset: 1280)
!5170 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !5136, file: !5137, line: 156, size: 64, elements: !5171)
!5171 = !{!5172, !5173, !5174}
!5172 = !DIDerivedType(tag: DW_TAG_member, name: "ss", scope: !5170, file: !5137, line: 158, baseType: !113, size: 16)
!5173 = !DIDerivedType(tag: DW_TAG_member, name: "ssx", scope: !5170, file: !5137, line: 160, baseType: !519, size: 64)
!5174 = !DIDerivedType(tag: DW_TAG_member, name: "fred_ss", scope: !5170, file: !5137, line: 162, baseType: !5175, size: 64)
!5175 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "fred_ss", file: !5137, line: 69, size: 64, elements: !5176)
!5176 = !{!5177, !5178, !5179, !5180, !5181, !5182, !5183, !5184, !5185, !5186}
!5177 = !DIDerivedType(tag: DW_TAG_member, name: "ss", scope: !5175, file: !5137, line: 71, baseType: !519, size: 16, flags: DIFlagBitField, extraData: i64 0)
!5178 = !DIDerivedType(tag: DW_TAG_member, name: "sti", scope: !5175, file: !5137, line: 73, baseType: !519, size: 1, offset: 16, flags: DIFlagBitField, extraData: i64 0)
!5179 = !DIDerivedType(tag: DW_TAG_member, name: "swevent", scope: !5175, file: !5137, line: 75, baseType: !519, size: 1, offset: 17, flags: DIFlagBitField, extraData: i64 0)
!5180 = !DIDerivedType(tag: DW_TAG_member, name: "nmi", scope: !5175, file: !5137, line: 77, baseType: !519, size: 1, offset: 18, flags: DIFlagBitField, extraData: i64 0)
!5181 = !DIDerivedType(tag: DW_TAG_member, name: "vector", scope: !5175, file: !5137, line: 80, baseType: !519, size: 8, offset: 32, flags: DIFlagBitField, extraData: i64 0)
!5182 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !5175, file: !5137, line: 83, baseType: !519, size: 4, offset: 48, flags: DIFlagBitField, extraData: i64 0)
!5183 = !DIDerivedType(tag: DW_TAG_member, name: "enclave", scope: !5175, file: !5137, line: 86, baseType: !519, size: 1, offset: 56, flags: DIFlagBitField, extraData: i64 0)
!5184 = !DIDerivedType(tag: DW_TAG_member, name: "lm", scope: !5175, file: !5137, line: 88, baseType: !519, size: 1, offset: 57, flags: DIFlagBitField, extraData: i64 0)
!5185 = !DIDerivedType(tag: DW_TAG_member, name: "nested", scope: !5175, file: !5137, line: 93, baseType: !519, size: 1, offset: 58, flags: DIFlagBitField, extraData: i64 0)
!5186 = !DIDerivedType(tag: DW_TAG_member, name: "insnlen", scope: !5175, file: !5137, line: 100, baseType: !519, size: 4, offset: 60, flags: DIFlagBitField, extraData: i64 0)
!5187 = !DIDerivedType(tag: DW_TAG_member, name: "entry_eip", scope: !5112, file: !5039, line: 98, baseType: !578, size: 32, offset: 1024)
!5188 = !DIDerivedType(tag: DW_TAG_member, name: "xsave", scope: !5057, file: !5039, line: 354, baseType: !5189, size: 4608, align: 512)
!5189 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "xregs_state", file: !5039, line: 335, size: 4608, align: 512, elements: !5190)
!5190 = !{!5191, !5192, !5199}
!5191 = !DIDerivedType(tag: DW_TAG_member, name: "i387", scope: !5189, file: !5039, line: 336, baseType: !5075, size: 4096, align: 128)
!5192 = !DIDerivedType(tag: DW_TAG_member, name: "header", scope: !5189, file: !5039, line: 337, baseType: !5193, size: 512, offset: 4096)
!5193 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "xstate_header", file: !5039, line: 314, size: 512, elements: !5194)
!5194 = !{!5195, !5196, !5197}
!5195 = !DIDerivedType(tag: DW_TAG_member, name: "xfeatures", scope: !5193, file: !5039, line: 315, baseType: !519, size: 64)
!5196 = !DIDerivedType(tag: DW_TAG_member, name: "xcomp_bv", scope: !5193, file: !5039, line: 316, baseType: !519, size: 64, offset: 64)
!5197 = !DIDerivedType(tag: DW_TAG_member, name: "reserved", scope: !5193, file: !5039, line: 317, baseType: !5198, size: 384, offset: 128)
!5198 = !DICompositeType(tag: DW_TAG_array_type, baseType: !519, size: 384, elements: !601)
!5199 = !DIDerivedType(tag: DW_TAG_member, name: "extended_state_area", scope: !5189, file: !5039, line: 338, baseType: !5200, offset: 4608)
!5200 = !DICompositeType(tag: DW_TAG_array_type, baseType: !103, elements: !1353)
!5201 = !DIDerivedType(tag: DW_TAG_member, name: "__padding", scope: !5057, file: !5039, line: 355, baseType: !5202, size: 32768)
!5202 = !DICompositeType(tag: DW_TAG_array_type, baseType: !103, size: 32768, elements: !4198)
!5203 = !DIDerivedType(tag: DW_TAG_member, name: "__task_fpstate", scope: !5038, file: !5039, line: 486, baseType: !5044, size: 64, offset: 192)
!5204 = !DIDerivedType(tag: DW_TAG_member, name: "perm", scope: !5038, file: !5039, line: 493, baseType: !5205, size: 128, offset: 256)
!5205 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "fpu_state_perm", file: !5039, line: 406, size: 128, elements: !5206)
!5206 = !{!5207, !5208, !5209}
!5207 = !DIDerivedType(tag: DW_TAG_member, name: "__state_perm", scope: !5205, file: !5039, line: 426, baseType: !519, size: 64)
!5208 = !DIDerivedType(tag: DW_TAG_member, name: "__state_size", scope: !5205, file: !5039, line: 434, baseType: !7, size: 32, offset: 64)
!5209 = !DIDerivedType(tag: DW_TAG_member, name: "__user_state_size", scope: !5205, file: !5039, line: 442, baseType: !7, size: 32, offset: 96)
!5210 = !DIDerivedType(tag: DW_TAG_member, name: "guest_perm", scope: !5038, file: !5039, line: 500, baseType: !5205, size: 128, offset: 384)
!5211 = !DIDerivedType(tag: DW_TAG_member, name: "__fpstate", scope: !5038, file: !5039, line: 510, baseType: !5045, size: 33280, align: 512, offset: 512)
!5212 = !DIDerivedType(tag: DW_TAG_member, name: "waiters", scope: !3611, file: !3612, line: 16, baseType: !74, size: 192, offset: 512)
!5213 = !DIDerivedType(tag: DW_TAG_member, name: "block", scope: !3611, file: !3612, line: 17, baseType: !69, size: 32, offset: 704)
!5214 = !DIDerivedType(tag: DW_TAG_member, name: "s_fs_info", scope: !3173, file: !342, line: 1303, baseType: !40, size: 64, offset: 6976)
!5215 = !DIDerivedType(tag: DW_TAG_member, name: "s_time_gran", scope: !3173, file: !342, line: 1306, baseType: !578, size: 32, offset: 7040)
!5216 = !DIDerivedType(tag: DW_TAG_member, name: "s_time_min", scope: !3173, file: !342, line: 1308, baseType: !569, size: 64, offset: 7104)
!5217 = !DIDerivedType(tag: DW_TAG_member, name: "s_time_max", scope: !3173, file: !342, line: 1309, baseType: !569, size: 64, offset: 7168)
!5218 = !DIDerivedType(tag: DW_TAG_member, name: "s_fsnotify_mask", scope: !3173, file: !342, line: 1311, baseType: !578, size: 32, offset: 7232)
!5219 = !DIDerivedType(tag: DW_TAG_member, name: "s_fsnotify_info", scope: !3173, file: !342, line: 1312, baseType: !5220, size: 64, offset: 7296)
!5220 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5221, size: 64)
!5221 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "fsnotify_sb_info", file: !474, line: 489, size: 256, elements: !5222)
!5222 = !{!5223, !5237}
!5223 = !DIDerivedType(tag: DW_TAG_member, name: "sb_marks", scope: !5221, file: !474, line: 490, baseType: !5224, size: 64)
!5224 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5225, size: 64)
!5225 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "fsnotify_mark_connector", file: !474, line: 469, size: 192, elements: !5226)
!5226 = !{!5227, !5228, !5229, !5230, !5231, !5236}
!5227 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !5225, file: !474, line: 470, baseType: !79, size: 32)
!5228 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !5225, file: !474, line: 471, baseType: !107, size: 8, offset: 32)
!5229 = !DIDerivedType(tag: DW_TAG_member, name: "prio", scope: !5225, file: !474, line: 472, baseType: !107, size: 8, offset: 40)
!5230 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !5225, file: !474, line: 475, baseType: !46, size: 16, offset: 48)
!5231 = !DIDerivedType(tag: DW_TAG_member, scope: !5225, file: !474, line: 476, baseType: !5232, size: 64, offset: 64)
!5232 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !5225, file: !474, line: 476, size: 64, elements: !5233)
!5233 = !{!5234, !5235}
!5234 = !DIDerivedType(tag: DW_TAG_member, name: "obj", scope: !5232, file: !474, line: 478, baseType: !40, size: 64)
!5235 = !DIDerivedType(tag: DW_TAG_member, name: "destroy_next", scope: !5232, file: !474, line: 480, baseType: !5224, size: 64)
!5236 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !5225, file: !474, line: 482, baseType: !216, size: 64, offset: 128)
!5237 = !DIDerivedType(tag: DW_TAG_member, name: "watched_objects", scope: !5221, file: !474, line: 499, baseType: !5238, size: 192, offset: 64)
!5238 = !DICompositeType(tag: DW_TAG_array_type, baseType: !496, size: 192, elements: !962)
!5239 = !DIDerivedType(tag: DW_TAG_member, name: "s_id", scope: !3173, file: !342, line: 1325, baseType: !4545, size: 256, offset: 7360)
!5240 = !DIDerivedType(tag: DW_TAG_member, name: "s_uuid", scope: !3173, file: !342, line: 1326, baseType: !5241, size: 128, offset: 7616)
!5241 = !DIDerivedType(tag: DW_TAG_typedef, name: "uuid_t", file: !5242, line: 21, baseType: !5243)
!5242 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/uuid.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "2e89c1c0bf06464063ae8a1a936cf772")
!5243 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !5242, line: 19, size: 128, elements: !5244)
!5244 = !{!5245}
!5245 = !DIDerivedType(tag: DW_TAG_member, name: "b", scope: !5243, file: !5242, line: 20, baseType: !5246, size: 128)
!5246 = !DICompositeType(tag: DW_TAG_array_type, baseType: !105, size: 128, elements: !4056)
!5247 = !DIDerivedType(tag: DW_TAG_member, name: "s_uuid_len", scope: !3173, file: !342, line: 1327, baseType: !103, size: 8, offset: 7744)
!5248 = !DIDerivedType(tag: DW_TAG_member, name: "s_sysfs_name", scope: !3173, file: !342, line: 1330, baseType: !5249, size: 296, offset: 7752)
!5249 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 296, elements: !5250)
!5250 = !{!5251}
!5251 = !DISubrange(count: 37)
!5252 = !DIDerivedType(tag: DW_TAG_member, name: "s_max_links", scope: !3173, file: !342, line: 1332, baseType: !7, size: 32, offset: 8064)
!5253 = !DIDerivedType(tag: DW_TAG_member, name: "s_vfs_rename_mutex", scope: !3173, file: !342, line: 1338, baseType: !1277, size: 256, offset: 8128)
!5254 = !DIDerivedType(tag: DW_TAG_member, name: "s_subtype", scope: !3173, file: !342, line: 1344, baseType: !36, size: 64, offset: 8384)
!5255 = !DIDerivedType(tag: DW_TAG_member, name: "s_d_op", scope: !3173, file: !342, line: 1346, baseType: !5256, size: 64, offset: 8448)
!5256 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5257, size: 64)
!5257 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !5258)
!5258 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "dentry_operations", file: !366, line: 138, size: 1024, align: 512, elements: !5259)
!5259 = !{!5260, !5264, !5265, !5272, !5278, !5282, !5286, !5290, !5291, !5295, !5299, !5304, !5308}
!5260 = !DIDerivedType(tag: DW_TAG_member, name: "d_revalidate", scope: !5258, file: !366, line: 139, baseType: !5261, size: 64)
!5261 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5262, size: 64)
!5262 = !DISubroutineType(types: !5263)
!5263 = !{!42, !740, !7}
!5264 = !DIDerivedType(tag: DW_TAG_member, name: "d_weak_revalidate", scope: !5258, file: !366, line: 140, baseType: !5261, size: 64, offset: 64)
!5265 = !DIDerivedType(tag: DW_TAG_member, name: "d_hash", scope: !5258, file: !366, line: 141, baseType: !5266, size: 64, offset: 128)
!5266 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5267, size: 64)
!5267 = !DISubroutineType(types: !5268)
!5268 = !{!42, !5269, !5271}
!5269 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5270, size: 64)
!5270 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !741)
!5271 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !764, size: 64)
!5272 = !DIDerivedType(tag: DW_TAG_member, name: "d_compare", scope: !5258, file: !366, line: 142, baseType: !5273, size: 64, offset: 192)
!5273 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5274, size: 64)
!5274 = !DISubroutineType(types: !5275)
!5275 = !{!42, !5269, !7, !36, !5276}
!5276 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5277, size: 64)
!5277 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !764)
!5278 = !DIDerivedType(tag: DW_TAG_member, name: "d_delete", scope: !5258, file: !366, line: 144, baseType: !5279, size: 64, offset: 256)
!5279 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5280, size: 64)
!5280 = !DISubroutineType(types: !5281)
!5281 = !{!42, !5269}
!5282 = !DIDerivedType(tag: DW_TAG_member, name: "d_init", scope: !5258, file: !366, line: 145, baseType: !5283, size: 64, offset: 320)
!5283 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5284, size: 64)
!5284 = !DISubroutineType(types: !5285)
!5285 = !{!42, !740}
!5286 = !DIDerivedType(tag: DW_TAG_member, name: "d_release", scope: !5258, file: !366, line: 146, baseType: !5287, size: 64, offset: 384)
!5287 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5288, size: 64)
!5288 = !DISubroutineType(types: !5289)
!5289 = !{null, !740}
!5290 = !DIDerivedType(tag: DW_TAG_member, name: "d_prune", scope: !5258, file: !366, line: 147, baseType: !5287, size: 64, offset: 448)
!5291 = !DIDerivedType(tag: DW_TAG_member, name: "d_iput", scope: !5258, file: !366, line: 148, baseType: !5292, size: 64, offset: 512)
!5292 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5293, size: 64)
!5293 = !DISubroutineType(types: !5294)
!5294 = !{null, !740, !779}
!5295 = !DIDerivedType(tag: DW_TAG_member, name: "d_dname", scope: !5258, file: !366, line: 149, baseType: !5296, size: 64, offset: 576)
!5296 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5297, size: 64)
!5297 = !DISubroutineType(types: !5298)
!5298 = !{!625, !740, !625, !42}
!5299 = !DIDerivedType(tag: DW_TAG_member, name: "d_automount", scope: !5258, file: !366, line: 150, baseType: !5300, size: 64, offset: 640)
!5300 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5301, size: 64)
!5301 = !DISubroutineType(types: !5302)
!5302 = !{!3166, !5303}
!5303 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3162, size: 64)
!5304 = !DIDerivedType(tag: DW_TAG_member, name: "d_manage", scope: !5258, file: !366, line: 151, baseType: !5305, size: 64, offset: 704)
!5305 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5306, size: 64)
!5306 = !DISubroutineType(types: !5307)
!5307 = !{!42, !3398, !614}
!5308 = !DIDerivedType(tag: DW_TAG_member, name: "d_real", scope: !5258, file: !366, line: 152, baseType: !5309, size: 64, offset: 768)
!5309 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5310, size: 64)
!5310 = !DISubroutineType(types: !5311)
!5311 = !{!740, !740, !365}
!5312 = !DIDerivedType(tag: DW_TAG_member, name: "s_shrink", scope: !3173, file: !342, line: 1348, baseType: !5313, size: 64, offset: 8512)
!5313 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5314, size: 64)
!5314 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "shrinker", file: !3338, line: 82, size: 960, elements: !5315)
!5315 = !{!5316, !5320, !5321, !5322, !5323, !5324, !5325, !5326, !5327, !5328, !5329}
!5316 = !DIDerivedType(tag: DW_TAG_member, name: "count_objects", scope: !5314, file: !3338, line: 83, baseType: !5317, size: 64)
!5317 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5318, size: 64)
!5318 = !DISubroutineType(types: !5319)
!5319 = !{!59, !5313, !3336}
!5320 = !DIDerivedType(tag: DW_TAG_member, name: "scan_objects", scope: !5314, file: !3338, line: 85, baseType: !5317, size: 64, offset: 64)
!5321 = !DIDerivedType(tag: DW_TAG_member, name: "batch", scope: !5314, file: !3338, line: 88, baseType: !892, size: 64, offset: 128)
!5322 = !DIDerivedType(tag: DW_TAG_member, name: "seeks", scope: !5314, file: !3338, line: 89, baseType: !42, size: 32, offset: 192)
!5323 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !5314, file: !3338, line: 90, baseType: !7, size: 32, offset: 224)
!5324 = !DIDerivedType(tag: DW_TAG_member, name: "refcount", scope: !5314, file: !3338, line: 99, baseType: !533, size: 32, offset: 256)
!5325 = !DIDerivedType(tag: DW_TAG_member, name: "done", scope: !5314, file: !3338, line: 100, baseType: !139, size: 256, offset: 320)
!5326 = !DIDerivedType(tag: DW_TAG_member, name: "rcu", scope: !5314, file: !3338, line: 101, baseType: !129, size: 128, align: 64, offset: 576)
!5327 = !DIDerivedType(tag: DW_TAG_member, name: "private_data", scope: !5314, file: !3338, line: 103, baseType: !40, size: 64, offset: 704)
!5328 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !5314, file: !3338, line: 106, baseType: !117, size: 128, offset: 768)
!5329 = !DIDerivedType(tag: DW_TAG_member, name: "nr_deferred", scope: !5314, file: !3338, line: 117, baseType: !5330, size: 64, offset: 896)
!5330 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !496, size: 64)
!5331 = !DIDerivedType(tag: DW_TAG_member, name: "s_remove_count", scope: !3173, file: !342, line: 1351, baseType: !496, size: 64, offset: 8576)
!5332 = !DIDerivedType(tag: DW_TAG_member, name: "s_readonly_remount", scope: !3173, file: !342, line: 1354, baseType: !42, size: 32, offset: 8640)
!5333 = !DIDerivedType(tag: DW_TAG_member, name: "s_wb_err", scope: !3173, file: !342, line: 1357, baseType: !2371, size: 32, offset: 8672)
!5334 = !DIDerivedType(tag: DW_TAG_member, name: "s_dio_done_wq", scope: !3173, file: !342, line: 1360, baseType: !2845, size: 64, offset: 8704)
!5335 = !DIDerivedType(tag: DW_TAG_member, name: "s_pins", scope: !3173, file: !342, line: 1361, baseType: !216, size: 64, offset: 8768)
!5336 = !DIDerivedType(tag: DW_TAG_member, name: "s_user_ns", scope: !3173, file: !342, line: 1368, baseType: !700, size: 64, offset: 8832)
!5337 = !DIDerivedType(tag: DW_TAG_member, name: "s_dentry_lru", scope: !3173, file: !342, line: 1375, baseType: !5338, size: 64, offset: 8896)
!5338 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "list_lru", file: !5339, line: 51, size: 64, elements: !5340)
!5339 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/list_lru.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "35899f4f2ed4fb744a7477af8c83bffa")
!5340 = !{!5341}
!5341 = !DIDerivedType(tag: DW_TAG_member, name: "node", scope: !5338, file: !5339, line: 52, baseType: !5342, size: 64)
!5342 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5343, size: 64)
!5343 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "list_lru_node", file: !5339, line: 43, size: 512, align: 512, elements: !5344)
!5344 = !{!5345, !5346, !5351}
!5345 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !5343, file: !5339, line: 45, baseType: !79, size: 32)
!5346 = !DIDerivedType(tag: DW_TAG_member, name: "lru", scope: !5343, file: !5339, line: 47, baseType: !5347, size: 192, offset: 64)
!5347 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "list_lru_one", file: !5339, line: 31, size: 192, elements: !5348)
!5348 = !{!5349, !5350}
!5349 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !5347, file: !5339, line: 32, baseType: !117, size: 128)
!5350 = !DIDerivedType(tag: DW_TAG_member, name: "nr_items", scope: !5347, file: !5339, line: 34, baseType: !892, size: 64, offset: 128)
!5351 = !DIDerivedType(tag: DW_TAG_member, name: "nr_items", scope: !5343, file: !5339, line: 48, baseType: !892, size: 64, offset: 256)
!5352 = !DIDerivedType(tag: DW_TAG_member, name: "s_inode_lru", scope: !3173, file: !342, line: 1376, baseType: !5338, size: 64, offset: 8960)
!5353 = !DIDerivedType(tag: DW_TAG_member, name: "rcu", scope: !3173, file: !342, line: 1377, baseType: !129, size: 128, align: 64, offset: 9024)
!5354 = !DIDerivedType(tag: DW_TAG_member, name: "destroy_work", scope: !3173, file: !342, line: 1378, baseType: !1337, size: 256, offset: 9152)
!5355 = !DIDerivedType(tag: DW_TAG_member, name: "s_sync_lock", scope: !3173, file: !342, line: 1380, baseType: !1277, size: 256, offset: 9408)
!5356 = !DIDerivedType(tag: DW_TAG_member, name: "s_stack_depth", scope: !3173, file: !342, line: 1385, baseType: !42, size: 32, offset: 9664)
!5357 = !DIDerivedType(tag: DW_TAG_member, name: "s_inode_list_lock", scope: !3173, file: !342, line: 1388, baseType: !79, size: 32, align: 512, offset: 9728)
!5358 = !DIDerivedType(tag: DW_TAG_member, name: "s_inodes", scope: !3173, file: !342, line: 1389, baseType: !117, size: 128, offset: 9792)
!5359 = !DIDerivedType(tag: DW_TAG_member, name: "s_inode_wblist_lock", scope: !3173, file: !342, line: 1391, baseType: !79, size: 32, offset: 9920)
!5360 = !DIDerivedType(tag: DW_TAG_member, name: "s_inodes_wb", scope: !3173, file: !342, line: 1392, baseType: !117, size: 128, offset: 9984)
!5361 = !DIDerivedType(tag: DW_TAG_member, name: "mnt_flags", scope: !3167, file: !3168, line: 72, baseType: !42, size: 32, offset: 128)
!5362 = !DIDerivedType(tag: DW_TAG_member, name: "mnt_idmap", scope: !3167, file: !3168, line: 73, baseType: !817, size: 64, offset: 192)
!5363 = !DIDerivedType(tag: DW_TAG_member, name: "dentry", scope: !3162, file: !3163, line: 10, baseType: !740, size: 64, offset: 64)
!5364 = !DIDerivedType(tag: DW_TAG_member, scope: !897, file: !342, line: 1045, baseType: !5365, size: 256, offset: 640)
!5365 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !897, file: !342, line: 1045, size: 256, elements: !5366)
!5366 = !{!5367, !5368}
!5367 = !DIDerivedType(tag: DW_TAG_member, name: "f_pos_lock", scope: !5365, file: !342, line: 1047, baseType: !1277, size: 256)
!5368 = !DIDerivedType(tag: DW_TAG_member, name: "f_pipe", scope: !5365, file: !342, line: 1049, baseType: !519, size: 64)
!5369 = !DIDerivedType(tag: DW_TAG_member, name: "f_pos", scope: !897, file: !342, line: 1051, baseType: !61, size: 64, offset: 896)
!5370 = !DIDerivedType(tag: DW_TAG_member, name: "f_security", scope: !897, file: !342, line: 1053, baseType: !40, size: 64, offset: 960)
!5371 = !DIDerivedType(tag: DW_TAG_member, name: "f_owner", scope: !897, file: !342, line: 1056, baseType: !5372, size: 64, offset: 1024)
!5372 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5373, size: 64)
!5373 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "fown_struct", file: !342, line: 965, size: 320, elements: !5374)
!5374 = !{!5375, !5376, !5377, !5378, !5379, !5380, !5381}
!5375 = !DIDerivedType(tag: DW_TAG_member, name: "file", scope: !5373, file: !342, line: 966, baseType: !896, size: 64)
!5376 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !5373, file: !342, line: 967, baseType: !3083, size: 64, offset: 64)
!5377 = !DIDerivedType(tag: DW_TAG_member, name: "pid", scope: !5373, file: !342, line: 968, baseType: !3949, size: 64, offset: 128)
!5378 = !DIDerivedType(tag: DW_TAG_member, name: "pid_type", scope: !5373, file: !342, line: 969, baseType: !333, size: 32, offset: 192)
!5379 = !DIDerivedType(tag: DW_TAG_member, name: "uid", scope: !5373, file: !342, line: 970, baseType: !188, size: 32, offset: 224)
!5380 = !DIDerivedType(tag: DW_TAG_member, name: "euid", scope: !5373, file: !342, line: 970, baseType: !188, size: 32, offset: 256)
!5381 = !DIDerivedType(tag: DW_TAG_member, name: "signum", scope: !5373, file: !342, line: 971, baseType: !42, size: 32, offset: 288)
!5382 = !DIDerivedType(tag: DW_TAG_member, name: "f_wb_err", scope: !897, file: !342, line: 1057, baseType: !2371, size: 32, offset: 1088)
!5383 = !DIDerivedType(tag: DW_TAG_member, name: "f_sb_err", scope: !897, file: !342, line: 1058, baseType: !2371, size: 32, offset: 1120)
!5384 = !DIDerivedType(tag: DW_TAG_member, name: "f_ep", scope: !897, file: !342, line: 1060, baseType: !5385, size: 64, offset: 1152)
!5385 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !216, size: 64)
!5386 = !DIDerivedType(tag: DW_TAG_member, scope: !897, file: !342, line: 1062, baseType: !5387, size: 256, offset: 1216)
!5387 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !897, file: !342, line: 1062, size: 256, elements: !5388)
!5388 = !{!5389, !5390, !5391, !5392}
!5389 = !DIDerivedType(tag: DW_TAG_member, name: "f_task_work", scope: !5387, file: !342, line: 1063, baseType: !129, size: 128, align: 64)
!5390 = !DIDerivedType(tag: DW_TAG_member, name: "f_llist", scope: !5387, file: !342, line: 1064, baseType: !3651, size: 64)
!5391 = !DIDerivedType(tag: DW_TAG_member, name: "f_ra", scope: !5387, file: !342, line: 1065, baseType: !1643, size: 256)
!5392 = !DIDerivedType(tag: DW_TAG_member, name: "f_freeptr", scope: !5387, file: !342, line: 1066, baseType: !5393, size: 64)
!5393 = !DIDerivedType(tag: DW_TAG_typedef, name: "freeptr_t", file: !416, line: 219, baseType: !5394)
!5394 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !416, line: 219, size: 64, elements: !5395)
!5395 = !{!5396}
!5396 = !DIDerivedType(tag: DW_TAG_member, name: "v", scope: !5394, file: !416, line: 219, baseType: !59, size: 64)
!5397 = !DIDerivedType(tag: DW_TAG_member, name: "getattr", scope: !794, file: !342, line: 2151, baseType: !5398, size: 64, offset: 896)
!5398 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5399, size: 64)
!5399 = !DISubroutineType(types: !5400)
!5400 = !{!42, !817, !3398, !5401, !578, !7}
!5401 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5402, size: 64)
!5402 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kstat", file: !5403, line: 22, size: 1472, elements: !5404)
!5403 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/stat.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "b18ff898e098675144ed2ea4bef71040")
!5404 = !{!5405, !5406, !5407, !5408, !5409, !5410, !5411, !5412, !5413, !5414, !5415, !5416, !5417, !5418, !5419, !5420, !5421, !5422, !5423, !5424, !5425, !5426, !5427, !5428, !5429}
!5405 = !DIDerivedType(tag: DW_TAG_member, name: "result_mask", scope: !5402, file: !5403, line: 23, baseType: !578, size: 32)
!5406 = !DIDerivedType(tag: DW_TAG_member, name: "mode", scope: !5402, file: !5403, line: 24, baseType: !44, size: 16, offset: 32)
!5407 = !DIDerivedType(tag: DW_TAG_member, name: "nlink", scope: !5402, file: !5403, line: 25, baseType: !7, size: 32, offset: 64)
!5408 = !DIDerivedType(tag: DW_TAG_member, name: "blksize", scope: !5402, file: !5403, line: 26, baseType: !577, size: 32, offset: 96)
!5409 = !DIDerivedType(tag: DW_TAG_member, name: "attributes", scope: !5402, file: !5403, line: 27, baseType: !519, size: 64, offset: 128)
!5410 = !DIDerivedType(tag: DW_TAG_member, name: "attributes_mask", scope: !5402, file: !5403, line: 28, baseType: !519, size: 64, offset: 192)
!5411 = !DIDerivedType(tag: DW_TAG_member, name: "ino", scope: !5402, file: !5403, line: 41, baseType: !519, size: 64, offset: 256)
!5412 = !DIDerivedType(tag: DW_TAG_member, name: "dev", scope: !5402, file: !5403, line: 42, baseType: !852, size: 32, offset: 320)
!5413 = !DIDerivedType(tag: DW_TAG_member, name: "rdev", scope: !5402, file: !5403, line: 43, baseType: !852, size: 32, offset: 352)
!5414 = !DIDerivedType(tag: DW_TAG_member, name: "uid", scope: !5402, file: !5403, line: 44, baseType: !188, size: 32, offset: 384)
!5415 = !DIDerivedType(tag: DW_TAG_member, name: "gid", scope: !5402, file: !5403, line: 45, baseType: !196, size: 32, offset: 416)
!5416 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !5402, file: !5403, line: 46, baseType: !61, size: 64, offset: 448)
!5417 = !DIDerivedType(tag: DW_TAG_member, name: "atime", scope: !5402, file: !5403, line: 47, baseType: !888, size: 128, offset: 512)
!5418 = !DIDerivedType(tag: DW_TAG_member, name: "mtime", scope: !5402, file: !5403, line: 48, baseType: !888, size: 128, offset: 640)
!5419 = !DIDerivedType(tag: DW_TAG_member, name: "ctime", scope: !5402, file: !5403, line: 49, baseType: !888, size: 128, offset: 768)
!5420 = !DIDerivedType(tag: DW_TAG_member, name: "btime", scope: !5402, file: !5403, line: 50, baseType: !888, size: 128, offset: 896)
!5421 = !DIDerivedType(tag: DW_TAG_member, name: "blocks", scope: !5402, file: !5403, line: 51, baseType: !519, size: 64, offset: 1024)
!5422 = !DIDerivedType(tag: DW_TAG_member, name: "mnt_id", scope: !5402, file: !5403, line: 52, baseType: !519, size: 64, offset: 1088)
!5423 = !DIDerivedType(tag: DW_TAG_member, name: "dio_mem_align", scope: !5402, file: !5403, line: 53, baseType: !578, size: 32, offset: 1152)
!5424 = !DIDerivedType(tag: DW_TAG_member, name: "dio_offset_align", scope: !5402, file: !5403, line: 54, baseType: !578, size: 32, offset: 1184)
!5425 = !DIDerivedType(tag: DW_TAG_member, name: "change_cookie", scope: !5402, file: !5403, line: 55, baseType: !519, size: 64, offset: 1216)
!5426 = !DIDerivedType(tag: DW_TAG_member, name: "subvol", scope: !5402, file: !5403, line: 56, baseType: !519, size: 64, offset: 1280)
!5427 = !DIDerivedType(tag: DW_TAG_member, name: "atomic_write_unit_min", scope: !5402, file: !5403, line: 57, baseType: !578, size: 32, offset: 1344)
!5428 = !DIDerivedType(tag: DW_TAG_member, name: "atomic_write_unit_max", scope: !5402, file: !5403, line: 58, baseType: !578, size: 32, offset: 1376)
!5429 = !DIDerivedType(tag: DW_TAG_member, name: "atomic_write_segments_max", scope: !5402, file: !5403, line: 59, baseType: !578, size: 32, offset: 1408)
!5430 = !DIDerivedType(tag: DW_TAG_member, name: "listxattr", scope: !794, file: !342, line: 2153, baseType: !5431, size: 64, offset: 960)
!5431 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5432, size: 64)
!5432 = !DISubroutineType(types: !5433)
!5433 = !{!993, !740, !625, !55}
!5434 = !DIDerivedType(tag: DW_TAG_member, name: "fiemap", scope: !794, file: !342, line: 2154, baseType: !5435, size: 64, offset: 1024)
!5435 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5436, size: 64)
!5436 = !DISubroutineType(types: !5437)
!5437 = !{!42, !779, !5438, !519, !519}
!5438 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5439, size: 64)
!5439 = !DICompositeType(tag: DW_TAG_structure_type, name: "fiemap_extent_info", file: !342, line: 57, flags: DIFlagFwdDecl)
!5440 = !DIDerivedType(tag: DW_TAG_member, name: "update_time", scope: !794, file: !342, line: 2156, baseType: !5441, size: 64, offset: 1088)
!5441 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5442, size: 64)
!5442 = !DISubroutineType(types: !5443)
!5443 = !{!42, !779, !42}
!5444 = !DIDerivedType(tag: DW_TAG_member, name: "atomic_open", scope: !794, file: !342, line: 2157, baseType: !5445, size: 64, offset: 1152)
!5445 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5446, size: 64)
!5446 = !DISubroutineType(types: !5447)
!5447 = !{!42, !779, !740, !896, !7, !44}
!5448 = !DIDerivedType(tag: DW_TAG_member, name: "tmpfile", scope: !794, file: !342, line: 2160, baseType: !5449, size: 64, offset: 1216)
!5449 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5450, size: 64)
!5450 = !DISubroutineType(types: !5451)
!5451 = !{!42, !817, !779, !896, !44}
!5452 = !DIDerivedType(tag: DW_TAG_member, name: "get_acl", scope: !794, file: !342, line: 2162, baseType: !5453, size: 64, offset: 1280)
!5453 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5454, size: 64)
!5454 = !DISubroutineType(types: !5455)
!5455 = !{!788, !817, !740, !42}
!5456 = !DIDerivedType(tag: DW_TAG_member, name: "set_acl", scope: !794, file: !342, line: 2164, baseType: !5457, size: 64, offset: 1344)
!5457 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5458, size: 64)
!5458 = !DISubroutineType(types: !5459)
!5459 = !{!42, !817, !740, !788, !42}
!5460 = !DIDerivedType(tag: DW_TAG_member, name: "fileattr_set", scope: !794, file: !342, line: 2166, baseType: !5461, size: 64, offset: 1408)
!5461 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5462, size: 64)
!5462 = !DISubroutineType(types: !5463)
!5463 = !{!42, !817, !740, !5464}
!5464 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5465, size: 64)
!5465 = !DICompositeType(tag: DW_TAG_structure_type, name: "fileattr", file: !342, line: 80, flags: DIFlagFwdDecl)
!5466 = !DIDerivedType(tag: DW_TAG_member, name: "fileattr_get", scope: !794, file: !342, line: 2168, baseType: !5467, size: 64, offset: 1472)
!5467 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5468, size: 64)
!5468 = !DISubroutineType(types: !5469)
!5469 = !{!42, !740, !5464}
!5470 = !DIDerivedType(tag: DW_TAG_member, name: "get_offset_ctx", scope: !794, file: !342, line: 2169, baseType: !5471, size: 64, offset: 1536)
!5471 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5472, size: 64)
!5472 = !DISubroutineType(types: !5473)
!5473 = !{!5474, !779}
!5474 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5475, size: 64)
!5475 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "offset_ctx", file: !342, line: 3426, size: 192, elements: !5476)
!5476 = !{!5477, !5478}
!5477 = !DIDerivedType(tag: DW_TAG_member, name: "mt", scope: !5475, file: !342, line: 3427, baseType: !1191, size: 128)
!5478 = !DIDerivedType(tag: DW_TAG_member, name: "next_offset", scope: !5475, file: !342, line: 3428, baseType: !59, size: 64, offset: 128)
!5479 = !DIDerivedType(tag: DW_TAG_member, name: "i_sb", scope: !780, file: !342, line: 645, baseType: !3172, size: 64, offset: 320)
!5480 = !DIDerivedType(tag: DW_TAG_member, name: "i_mapping", scope: !780, file: !342, line: 646, baseType: !1030, size: 64, offset: 384)
!5481 = !DIDerivedType(tag: DW_TAG_member, name: "i_security", scope: !780, file: !342, line: 649, baseType: !40, size: 64, offset: 448)
!5482 = !DIDerivedType(tag: DW_TAG_member, name: "i_ino", scope: !780, file: !342, line: 653, baseType: !59, size: 64, offset: 512)
!5483 = !DIDerivedType(tag: DW_TAG_member, scope: !780, file: !342, line: 661, baseType: !5484, size: 32, offset: 576)
!5484 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !780, file: !342, line: 661, size: 32, elements: !5485)
!5485 = !{!5486, !5488}
!5486 = !DIDerivedType(tag: DW_TAG_member, name: "i_nlink", scope: !5484, file: !342, line: 662, baseType: !5487, size: 32)
!5487 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !7)
!5488 = !DIDerivedType(tag: DW_TAG_member, name: "__i_nlink", scope: !5484, file: !342, line: 663, baseType: !7, size: 32)
!5489 = !DIDerivedType(tag: DW_TAG_member, name: "i_rdev", scope: !780, file: !342, line: 665, baseType: !852, size: 32, offset: 608)
!5490 = !DIDerivedType(tag: DW_TAG_member, name: "i_size", scope: !780, file: !342, line: 666, baseType: !61, size: 64, offset: 640)
!5491 = !DIDerivedType(tag: DW_TAG_member, name: "i_atime_sec", scope: !780, file: !342, line: 667, baseType: !569, size: 64, offset: 704)
!5492 = !DIDerivedType(tag: DW_TAG_member, name: "i_mtime_sec", scope: !780, file: !342, line: 668, baseType: !569, size: 64, offset: 768)
!5493 = !DIDerivedType(tag: DW_TAG_member, name: "i_ctime_sec", scope: !780, file: !342, line: 669, baseType: !569, size: 64, offset: 832)
!5494 = !DIDerivedType(tag: DW_TAG_member, name: "i_atime_nsec", scope: !780, file: !342, line: 670, baseType: !578, size: 32, offset: 896)
!5495 = !DIDerivedType(tag: DW_TAG_member, name: "i_mtime_nsec", scope: !780, file: !342, line: 671, baseType: !578, size: 32, offset: 928)
!5496 = !DIDerivedType(tag: DW_TAG_member, name: "i_ctime_nsec", scope: !780, file: !342, line: 672, baseType: !578, size: 32, offset: 960)
!5497 = !DIDerivedType(tag: DW_TAG_member, name: "i_generation", scope: !780, file: !342, line: 673, baseType: !578, size: 32, offset: 992)
!5498 = !DIDerivedType(tag: DW_TAG_member, name: "i_lock", scope: !780, file: !342, line: 674, baseType: !79, size: 32, offset: 1024)
!5499 = !DIDerivedType(tag: DW_TAG_member, name: "i_bytes", scope: !780, file: !342, line: 675, baseType: !46, size: 16, offset: 1056)
!5500 = !DIDerivedType(tag: DW_TAG_member, name: "i_blkbits", scope: !780, file: !342, line: 676, baseType: !103, size: 8, offset: 1072)
!5501 = !DIDerivedType(tag: DW_TAG_member, name: "i_write_hint", scope: !780, file: !342, line: 677, baseType: !370, size: 8, offset: 1080)
!5502 = !DIDerivedType(tag: DW_TAG_member, name: "i_blocks", scope: !780, file: !342, line: 678, baseType: !3471, size: 64, offset: 1088)
!5503 = !DIDerivedType(tag: DW_TAG_member, name: "i_state", scope: !780, file: !342, line: 685, baseType: !578, size: 32, offset: 1152)
!5504 = !DIDerivedType(tag: DW_TAG_member, name: "i_rwsem", scope: !780, file: !342, line: 687, baseType: !549, size: 320, offset: 1216)
!5505 = !DIDerivedType(tag: DW_TAG_member, name: "dirtied_when", scope: !780, file: !342, line: 689, baseType: !59, size: 64, offset: 1536)
!5506 = !DIDerivedType(tag: DW_TAG_member, name: "dirtied_time_when", scope: !780, file: !342, line: 690, baseType: !59, size: 64, offset: 1600)
!5507 = !DIDerivedType(tag: DW_TAG_member, name: "i_hash", scope: !780, file: !342, line: 692, baseType: !220, size: 128, offset: 1664)
!5508 = !DIDerivedType(tag: DW_TAG_member, name: "i_io_list", scope: !780, file: !342, line: 693, baseType: !117, size: 128, offset: 1792)
!5509 = !DIDerivedType(tag: DW_TAG_member, name: "i_lru", scope: !780, file: !342, line: 702, baseType: !117, size: 128, offset: 1920)
!5510 = !DIDerivedType(tag: DW_TAG_member, name: "i_sb_list", scope: !780, file: !342, line: 703, baseType: !117, size: 128, offset: 2048)
!5511 = !DIDerivedType(tag: DW_TAG_member, name: "i_wb_list", scope: !780, file: !342, line: 704, baseType: !117, size: 128, offset: 2176)
!5512 = !DIDerivedType(tag: DW_TAG_member, scope: !780, file: !342, line: 705, baseType: !5513, size: 128, offset: 2304)
!5513 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !780, file: !342, line: 705, size: 128, elements: !5514)
!5514 = !{!5515, !5516}
!5515 = !DIDerivedType(tag: DW_TAG_member, name: "i_dentry", scope: !5513, file: !342, line: 706, baseType: !216, size: 64)
!5516 = !DIDerivedType(tag: DW_TAG_member, name: "i_rcu", scope: !5513, file: !342, line: 707, baseType: !129, size: 128, align: 64)
!5517 = !DIDerivedType(tag: DW_TAG_member, name: "i_version", scope: !780, file: !342, line: 709, baseType: !498, size: 64, offset: 2432)
!5518 = !DIDerivedType(tag: DW_TAG_member, name: "i_sequence", scope: !780, file: !342, line: 710, baseType: !498, size: 64, offset: 2496)
!5519 = !DIDerivedType(tag: DW_TAG_member, name: "i_count", scope: !780, file: !342, line: 711, baseType: !69, size: 32, offset: 2560)
!5520 = !DIDerivedType(tag: DW_TAG_member, name: "i_dio_count", scope: !780, file: !342, line: 712, baseType: !69, size: 32, offset: 2592)
!5521 = !DIDerivedType(tag: DW_TAG_member, name: "i_writecount", scope: !780, file: !342, line: 713, baseType: !69, size: 32, offset: 2624)
!5522 = !DIDerivedType(tag: DW_TAG_member, name: "i_readcount", scope: !780, file: !342, line: 715, baseType: !69, size: 32, offset: 2656)
!5523 = !DIDerivedType(tag: DW_TAG_member, scope: !780, file: !342, line: 717, baseType: !5524, size: 64, offset: 2688)
!5524 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !780, file: !342, line: 717, size: 64, elements: !5525)
!5525 = !{!5526, !5527}
!5526 = !DIDerivedType(tag: DW_TAG_member, name: "i_fop", scope: !5524, file: !342, line: 718, baseType: !903, size: 64)
!5527 = !DIDerivedType(tag: DW_TAG_member, name: "free_inode", scope: !5524, file: !342, line: 719, baseType: !3227, size: 64)
!5528 = !DIDerivedType(tag: DW_TAG_member, name: "i_flctx", scope: !780, file: !342, line: 721, baseType: !5529, size: 64, offset: 2752)
!5529 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5530, size: 64)
!5530 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "file_lock_context", file: !2962, line: 142, size: 448, elements: !5531)
!5531 = !{!5532, !5533, !5534, !5535}
!5532 = !DIDerivedType(tag: DW_TAG_member, name: "flc_lock", scope: !5530, file: !2962, line: 143, baseType: !79, size: 32)
!5533 = !DIDerivedType(tag: DW_TAG_member, name: "flc_flock", scope: !5530, file: !2962, line: 144, baseType: !117, size: 128, offset: 64)
!5534 = !DIDerivedType(tag: DW_TAG_member, name: "flc_posix", scope: !5530, file: !2962, line: 145, baseType: !117, size: 128, offset: 192)
!5535 = !DIDerivedType(tag: DW_TAG_member, name: "flc_lease", scope: !5530, file: !2962, line: 146, baseType: !117, size: 128, offset: 320)
!5536 = !DIDerivedType(tag: DW_TAG_member, name: "i_data", scope: !780, file: !342, line: 722, baseType: !1031, size: 1536, align: 64, offset: 2816)
!5537 = !DIDerivedType(tag: DW_TAG_member, name: "i_devices", scope: !780, file: !342, line: 723, baseType: !117, size: 128, offset: 4352)
!5538 = !DIDerivedType(tag: DW_TAG_member, scope: !780, file: !342, line: 724, baseType: !5539, size: 64, offset: 4480)
!5539 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !780, file: !342, line: 724, size: 64, elements: !5540)
!5540 = !{!5541, !5542, !5553, !5554}
!5541 = !DIDerivedType(tag: DW_TAG_member, name: "i_pipe", scope: !5539, file: !342, line: 725, baseType: !3062, size: 64)
!5542 = !DIDerivedType(tag: DW_TAG_member, name: "i_cdev", scope: !5539, file: !342, line: 726, baseType: !5543, size: 64)
!5543 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5544, size: 64)
!5544 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cdev", file: !5545, line: 14, size: 832, elements: !5546)
!5545 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/cdev.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "dd3580fb19b1b4d9a4e84334e4ba7aa6")
!5546 = !{!5547, !5548, !5549, !5550, !5551, !5552}
!5547 = !DIDerivedType(tag: DW_TAG_member, name: "kobj", scope: !5544, file: !5545, line: 15, baseType: !921, size: 512)
!5548 = !DIDerivedType(tag: DW_TAG_member, name: "owner", scope: !5544, file: !5545, line: 16, baseType: !908, size: 64, offset: 512)
!5549 = !DIDerivedType(tag: DW_TAG_member, name: "ops", scope: !5544, file: !5545, line: 17, baseType: !903, size: 64, offset: 576)
!5550 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !5544, file: !5545, line: 18, baseType: !117, size: 128, offset: 640)
!5551 = !DIDerivedType(tag: DW_TAG_member, name: "dev", scope: !5544, file: !5545, line: 19, baseType: !852, size: 32, offset: 768)
!5552 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !5544, file: !5545, line: 20, baseType: !7, size: 32, offset: 800)
!5553 = !DIDerivedType(tag: DW_TAG_member, name: "i_link", scope: !5539, file: !342, line: 727, baseType: !625, size: 64)
!5554 = !DIDerivedType(tag: DW_TAG_member, name: "i_dir_seq", scope: !5539, file: !342, line: 728, baseType: !7, size: 32)
!5555 = !DIDerivedType(tag: DW_TAG_member, name: "i_fsnotify_mask", scope: !780, file: !342, line: 733, baseType: !579, size: 32, offset: 4544)
!5556 = !DIDerivedType(tag: DW_TAG_member, name: "i_fsnotify_marks", scope: !780, file: !342, line: 735, baseType: !5224, size: 64, offset: 4608)
!5557 = !DIDerivedType(tag: DW_TAG_member, name: "i_private", scope: !780, file: !342, line: 746, baseType: !40, size: 64, offset: 4672)
!5558 = !DIDerivedType(tag: DW_TAG_member, name: "d_iname", scope: !741, file: !366, line: 91, baseType: !5559, size: 320, offset: 448)
!5559 = !DICompositeType(tag: DW_TAG_array_type, baseType: !107, size: 320, elements: !5560)
!5560 = !{!5561}
!5561 = !DISubrange(count: 40)
!5562 = !DIDerivedType(tag: DW_TAG_member, name: "d_op", scope: !741, file: !366, line: 95, baseType: !5256, size: 64, offset: 768)
!5563 = !DIDerivedType(tag: DW_TAG_member, name: "d_sb", scope: !741, file: !366, line: 96, baseType: !3172, size: 64, offset: 832)
!5564 = !DIDerivedType(tag: DW_TAG_member, name: "d_time", scope: !741, file: !366, line: 97, baseType: !59, size: 64, offset: 896)
!5565 = !DIDerivedType(tag: DW_TAG_member, name: "d_fsdata", scope: !741, file: !366, line: 98, baseType: !40, size: 64, offset: 960)
!5566 = !DIDerivedType(tag: DW_TAG_member, name: "d_lockref", scope: !741, file: !366, line: 100, baseType: !5567, size: 64, offset: 1024)
!5567 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "lockref", file: !5568, line: 25, size: 64, elements: !5569)
!5568 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/lockref.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "e2e2c44a44560f0fc3c2b649fe354d0c")
!5569 = !{!5570}
!5570 = !DIDerivedType(tag: DW_TAG_member, scope: !5567, file: !5568, line: 26, baseType: !5571, size: 64)
!5571 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !5567, file: !5568, line: 26, size: 64, elements: !5572)
!5572 = !{!5573, !5574}
!5573 = !DIDerivedType(tag: DW_TAG_member, name: "lock_count", scope: !5571, file: !5568, line: 28, baseType: !520, size: 64, align: 64)
!5574 = !DIDerivedType(tag: DW_TAG_member, scope: !5571, file: !5568, line: 30, baseType: !5575, size: 64)
!5575 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !5571, file: !5568, line: 30, size: 64, elements: !5576)
!5576 = !{!5577, !5578}
!5577 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !5575, file: !5568, line: 31, baseType: !79, size: 32)
!5578 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !5575, file: !5568, line: 32, baseType: !42, size: 32, offset: 32)
!5579 = !DIDerivedType(tag: DW_TAG_member, scope: !741, file: !366, line: 105, baseType: !5580, size: 128, offset: 1088)
!5580 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !741, file: !366, line: 105, size: 128, elements: !5581)
!5581 = !{!5582, !5583}
!5582 = !DIDerivedType(tag: DW_TAG_member, name: "d_lru", scope: !5580, file: !366, line: 106, baseType: !117, size: 128)
!5583 = !DIDerivedType(tag: DW_TAG_member, name: "d_wait", scope: !5580, file: !366, line: 107, baseType: !5584, size: 64)
!5584 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !74, size: 64)
!5585 = !DIDerivedType(tag: DW_TAG_member, name: "d_sib", scope: !741, file: !366, line: 109, baseType: !220, size: 128, offset: 1216)
!5586 = !DIDerivedType(tag: DW_TAG_member, name: "d_children", scope: !741, file: !366, line: 110, baseType: !216, size: 64, offset: 1344)
!5587 = !DIDerivedType(tag: DW_TAG_member, name: "d_u", scope: !741, file: !366, line: 118, baseType: !5588, size: 128, offset: 1408)
!5588 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !741, file: !366, line: 114, size: 128, elements: !5589)
!5589 = !{!5590, !5591, !5592}
!5590 = !DIDerivedType(tag: DW_TAG_member, name: "d_alias", scope: !5588, file: !366, line: 115, baseType: !220, size: 128)
!5591 = !DIDerivedType(tag: DW_TAG_member, name: "d_in_lookup_hash", scope: !5588, file: !366, line: 116, baseType: !755, size: 128)
!5592 = !DIDerivedType(tag: DW_TAG_member, name: "d_rcu", scope: !5588, file: !366, line: 117, baseType: !129, size: 128, align: 64)
!5593 = !DIDerivedType(tag: DW_TAG_member, name: "ops", scope: !736, file: !737, line: 11, baseType: !5594, size: 64, offset: 64)
!5594 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5595, size: 64)
!5595 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !5596)
!5596 = !DICompositeType(tag: DW_TAG_structure_type, name: "proc_ns_operations", file: !737, line: 7, flags: DIFlagFwdDecl)
!5597 = !DIDerivedType(tag: DW_TAG_member, name: "inum", scope: !736, file: !737, line: 12, baseType: !7, size: 32, offset: 128)
!5598 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !736, file: !737, line: 13, baseType: !533, size: 32, offset: 160)
!5599 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !701, file: !702, line: 83, baseType: !59, size: 64, offset: 1920)
!5600 = !DIDerivedType(tag: DW_TAG_member, name: "parent_could_setfcap", scope: !701, file: !702, line: 86, baseType: !614, size: 8, offset: 1984)
!5601 = !DIDerivedType(tag: DW_TAG_member, name: "keyring_name_list", scope: !701, file: !702, line: 94, baseType: !117, size: 128, offset: 2048)
!5602 = !DIDerivedType(tag: DW_TAG_member, name: "user_keyring_register", scope: !701, file: !702, line: 95, baseType: !528, size: 64, offset: 2176)
!5603 = !DIDerivedType(tag: DW_TAG_member, name: "keyring_sem", scope: !701, file: !702, line: 96, baseType: !549, size: 320, offset: 2240)
!5604 = !DIDerivedType(tag: DW_TAG_member, name: "work", scope: !701, file: !702, line: 103, baseType: !1337, size: 256, offset: 2560)
!5605 = !DIDerivedType(tag: DW_TAG_member, name: "set", scope: !701, file: !702, line: 105, baseType: !156, size: 832, offset: 2816)
!5606 = !DIDerivedType(tag: DW_TAG_member, name: "sysctls", scope: !701, file: !702, line: 106, baseType: !186, size: 64, offset: 3648)
!5607 = !DIDerivedType(tag: DW_TAG_member, name: "ucounts", scope: !701, file: !702, line: 108, baseType: !3994, size: 64, offset: 3712)
!5608 = !DIDerivedType(tag: DW_TAG_member, name: "ucount_max", scope: !701, file: !702, line: 109, baseType: !5609, size: 640, offset: 3776)
!5609 = !DICompositeType(tag: DW_TAG_array_type, baseType: !892, size: 640, elements: !4003)
!5610 = !DIDerivedType(tag: DW_TAG_member, name: "rlimit_max", scope: !701, file: !702, line: 110, baseType: !2788, size: 256, offset: 4416)
!5611 = !DIDerivedType(tag: DW_TAG_member, name: "binfmt_misc", scope: !701, file: !702, line: 113, baseType: !5612, size: 64, offset: 4672)
!5612 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5613, size: 64)
!5613 = !DICompositeType(tag: DW_TAG_structure_type, name: "binfmt_misc", file: !702, line: 71, flags: DIFlagFwdDecl)
!5614 = !DIDerivedType(tag: DW_TAG_member, name: "ucounts", scope: !492, file: !493, line: 140, baseType: !3994, size: 64, offset: 1216)
!5615 = !DIDerivedType(tag: DW_TAG_member, name: "group_info", scope: !492, file: !493, line: 141, baseType: !5616, size: 64, offset: 1280)
!5616 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5617, size: 64)
!5617 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "group_info", file: !493, line: 26, size: 64, elements: !5618)
!5618 = !{!5619, !5620, !5621}
!5619 = !DIDerivedType(tag: DW_TAG_member, name: "usage", scope: !5617, file: !493, line: 27, baseType: !533, size: 32)
!5620 = !DIDerivedType(tag: DW_TAG_member, name: "ngroups", scope: !5617, file: !493, line: 28, baseType: !42, size: 32, offset: 32)
!5621 = !DIDerivedType(tag: DW_TAG_member, name: "gid", scope: !5617, file: !493, line: 29, baseType: !5622, offset: 64)
!5622 = !DICompositeType(tag: DW_TAG_array_type, baseType: !196, elements: !1353)
!5623 = !DIDerivedType(tag: DW_TAG_member, scope: !492, file: !493, line: 143, baseType: !5624, size: 128, offset: 1344)
!5624 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !492, file: !493, line: 143, size: 128, elements: !5625)
!5625 = !{!5626, !5627}
!5626 = !DIDerivedType(tag: DW_TAG_member, name: "non_rcu", scope: !5624, file: !493, line: 144, baseType: !42, size: 32)
!5627 = !DIDerivedType(tag: DW_TAG_member, name: "rcu", scope: !5624, file: !493, line: 145, baseType: !129, size: 128, align: 64)
!5628 = !DIDerivedType(tag: DW_TAG_typedef, name: "slab_flags_t", file: !45, line: 158, baseType: !7)
!5629 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5630, size: 64)
!5630 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "backing_file", file: !5631, line: 47, size: 1600, elements: !5632)
!5631 = !DIFile(filename: "LLM4Con/kernel_experiment/SYZBOT-3b6b32dc50537a49/src/fs/file_table.c", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "9f0464e3d7acb9de8dd666f6494b200a")
!5632 = !{!5633, !5634}
!5633 = !DIDerivedType(tag: DW_TAG_member, name: "file", scope: !5630, file: !5631, line: 48, baseType: !897, size: 1472, align: 64)
!5634 = !DIDerivedType(tag: DW_TAG_member, name: "user_path", scope: !5630, file: !5631, line: 49, baseType: !3162, size: 128, offset: 1472)
!5635 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5636, size: 64)
!5636 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !5637)
!5637 = !DIDerivedType(tag: DW_TAG_volatile_type, baseType: !63)
!5638 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !492, size: 64)
!5639 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5640, size: 64)
!5640 = !DIDerivedType(tag: DW_TAG_volatile_type, baseType: !502)
!5641 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5642, size: 64)
!5642 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !3628)
!5643 = !DIDerivedType(tag: DW_TAG_typedef, name: "uintptr_t", file: !45, line: 42, baseType: !59)
!5644 = !DIDerivedType(tag: DW_TAG_typedef, name: "__kernel_rwf_t", file: !5645, line: 312, baseType: !42)
!5645 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/uapi/linux/fs.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "88dc684d91f43677766d7d17e4018654")
!5646 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5647, size: 64)
!5647 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !5648)
!5648 = !DIDerivedType(tag: DW_TAG_volatile_type, baseType: !5220)
!5649 = !{!0, !5650, !5652, !5654, !5659, !5664, !5666, !5668, !5670, !5672, !5674, !5677, !5680, !5682, !5684, !5691, !5694, !5696, !5701, !5703, !5708, !5710, !5713, !5718, !5720, !5725, !5727, !5729}
!5650 = !DIGlobalVariableExpression(var: !5651, expr: !DIExpression())
!5651 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_get_max_files507", scope: !2, file: !5631, line: 92, type: !40, isLocal: true, isDefinition: true)
!5652 = !DIGlobalVariableExpression(var: !5653, expr: !DIExpression())
!5653 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_init_module508", scope: !2, file: !5631, line: 145, type: !40, isLocal: true, isDefinition: true)
!5654 = !DIGlobalVariableExpression(var: !5655, expr: !DIExpression())
!5655 = distinct !DIGlobalVariable(name: "old_max", scope: !5656, file: !5631, line: 193, type: !892, isLocal: true, isDefinition: true)
!5656 = distinct !DISubprogram(name: "alloc_empty_file", scope: !5631, file: !5631, line: 191, type: !5657, scopeLine: 192, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5657 = !DISubroutineType(types: !5658)
!5658 = !{!896, !42, !490}
!5659 = !DIGlobalVariableExpression(var: !5660, expr: !DIExpression())
!5660 = distinct !DIGlobalVariable(scope: null, file: !5631, line: 226, type: !5661, isLocal: true, isDefinition: true)
!5661 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 280, elements: !5662)
!5662 = !{!5663}
!5663 = !DISubrange(count: 35)
!5664 = !DIGlobalVariableExpression(var: !5665, expr: !DIExpression())
!5665 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_alloc_file_pseudo509", scope: !2, file: !5631, line: 364, type: !40, isLocal: true, isDefinition: true)
!5666 = !DIGlobalVariableExpression(var: !5667, expr: !DIExpression())
!5667 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_alloc_file_pseudo_noaccount510", scope: !2, file: !5631, line: 388, type: !40, isLocal: true, isDefinition: true)
!5668 = !DIGlobalVariableExpression(var: !5669, expr: !DIExpression())
!5669 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_flush_delayed_fput511", scope: !2, file: !5631, line: 476, type: !40, isLocal: true, isDefinition: true)
!5670 = !DIGlobalVariableExpression(var: !5671, expr: !DIExpression())
!5671 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_fput512", scope: !2, file: !5631, line: 519, type: !40, isLocal: true, isDefinition: true)
!5672 = !DIGlobalVariableExpression(var: !5673, expr: !DIExpression())
!5673 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable___fput_sync513", scope: !2, file: !5631, line: 520, type: !40, isLocal: true, isDefinition: true)
!5674 = !DIGlobalVariableExpression(var: !5675, expr: !DIExpression())
!5675 = distinct !DIGlobalVariable(scope: null, file: !5631, line: 529, type: !5676, isLocal: true, isDefinition: true)
!5676 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 40, elements: !720)
!5677 = !DIGlobalVariableExpression(var: !5678, expr: !DIExpression())
!5678 = distinct !DIGlobalVariable(name: "__key", scope: !5679, file: !5631, line: 532, type: !3208, isLocal: true, isDefinition: true)
!5679 = distinct !DISubprogram(name: "files_init", scope: !5631, file: !5631, line: 522, type: !2887, scopeLine: 523, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5680 = !DIGlobalVariableExpression(var: !5681, expr: !DIExpression())
!5681 = distinct !DIGlobalVariable(name: "filp_cachep", scope: !2, file: !5631, line: 42, type: !3984, isLocal: true, isDefinition: true)
!5682 = !DIGlobalVariableExpression(var: !5683, expr: !DIExpression())
!5683 = distinct !DIGlobalVariable(name: "nr_files", scope: !2, file: !5631, line: 44, type: !675, isLocal: true, isDefinition: true, align: 512)
!5684 = !DIGlobalVariableExpression(var: !5685, expr: !DIExpression())
!5685 = distinct !DIGlobalVariable(name: "files_stat", scope: !2, file: !5631, line: 37, type: !5686, isLocal: true, isDefinition: true)
!5686 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "files_stat_struct", file: !5645, line: 115, size: 192, elements: !5687)
!5687 = !{!5688, !5689, !5690}
!5688 = !DIDerivedType(tag: DW_TAG_member, name: "nr_files", scope: !5686, file: !5645, line: 116, baseType: !59, size: 64)
!5689 = !DIDerivedType(tag: DW_TAG_member, name: "nr_free_files", scope: !5686, file: !5645, line: 117, baseType: !59, size: 64, offset: 64)
!5690 = !DIDerivedType(tag: DW_TAG_member, name: "max_files", scope: !5686, file: !5645, line: 118, baseType: !59, size: 64, offset: 128)
!5691 = !DIGlobalVariableExpression(var: !5692, expr: !DIExpression())
!5692 = distinct !DIGlobalVariable(scope: null, file: !5631, line: 136, type: !5693, isLocal: true, isDefinition: true)
!5693 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 24, elements: !962)
!5694 = !DIGlobalVariableExpression(var: !5695, expr: !DIExpression())
!5695 = distinct !DIGlobalVariable(scope: null, file: !5631, line: 136, type: !4055, isLocal: true, isDefinition: true)
!5696 = !DIGlobalVariableExpression(var: !5697, expr: !DIExpression())
!5697 = distinct !DIGlobalVariable(scope: null, file: !5631, line: 140, type: !5698, isLocal: true, isDefinition: true)
!5698 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 120, elements: !5699)
!5699 = !{!5700}
!5700 = !DISubrange(count: 15)
!5701 = !DIGlobalVariableExpression(var: !5702, expr: !DIExpression())
!5702 = distinct !DIGlobalVariable(scope: null, file: !5631, line: 108, type: !4757, isLocal: true, isDefinition: true)
!5703 = !DIGlobalVariableExpression(var: !5704, expr: !DIExpression())
!5704 = distinct !DIGlobalVariable(scope: null, file: !5631, line: 115, type: !5705, isLocal: true, isDefinition: true)
!5705 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 72, elements: !5706)
!5706 = !{!5707}
!5707 = !DISubrange(count: 9)
!5708 = !DIGlobalVariableExpression(var: !5709, expr: !DIExpression())
!5709 = distinct !DIGlobalVariable(scope: null, file: !5631, line: 124, type: !4757, isLocal: true, isDefinition: true)
!5710 = !DIGlobalVariableExpression(var: !5711, expr: !DIExpression())
!5711 = distinct !DIGlobalVariable(name: "fs_stat_sysctls", scope: !2, file: !5631, line: 106, type: !5712, isLocal: true, isDefinition: true)
!5712 = !DICompositeType(tag: DW_TAG_array_type, baseType: !33, size: 1344, elements: !962)
!5713 = !DIGlobalVariableExpression(var: !5714, expr: !DIExpression())
!5714 = distinct !DIGlobalVariable(name: "__key", scope: !5715, file: !5631, line: 167, type: !3208, isLocal: true, isDefinition: true)
!5715 = distinct !DISubprogram(name: "init_file", scope: !5631, file: !5631, line: 148, type: !5716, scopeLine: 149, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5716 = !DISubroutineType(types: !5717)
!5717 = !{!42, !896, !42, !490}
!5718 = !DIGlobalVariableExpression(var: !5719, expr: !DIExpression())
!5719 = distinct !DIGlobalVariable(scope: null, file: !5631, line: 167, type: !5698, isLocal: true, isDefinition: true)
!5720 = !DIGlobalVariableExpression(var: !5721, expr: !DIExpression())
!5721 = distinct !DIGlobalVariable(scope: null, file: !416, line: 690, type: !5722, isLocal: true, isDefinition: true)
!5722 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 848, elements: !5723)
!5723 = !{!5724}
!5724 = !DISubrange(count: 106)
!5725 = !DIGlobalVariableExpression(var: !5726, expr: !DIExpression())
!5726 = distinct !DIGlobalVariable(name: "delayed_fput_list", scope: !2, file: !5631, line: 447, type: !4988, isLocal: true, isDefinition: true)
!5727 = !DIGlobalVariableExpression(var: !5728, expr: !DIExpression())
!5728 = distinct !DIGlobalVariable(name: "delayed_fput_work", scope: !2, file: !5631, line: 478, type: !2840, isLocal: true, isDefinition: true)
!5729 = !DIGlobalVariableExpression(var: !5730, expr: !DIExpression())
!5730 = distinct !DIGlobalVariable(scope: null, file: !342, line: 3039, type: !5731, isLocal: true, isDefinition: true)
!5731 = !DICompositeType(tag: DW_TAG_array_type, baseType: !38, size: 832, elements: !5732)
!5732 = !{!5733}
!5733 = !DISubrange(count: 104)
!5734 = !{i32 7, !"Dwarf Version", i32 5}
!5735 = !{i32 2, !"Debug Info Version", i32 3}
!5736 = !{i32 1, !"wchar_size", i32 4}
!5737 = !{i32 7, !"PIC Level", i32 2}
!5738 = !{i32 7, !"PIE Level", i32 2}
!5739 = !{i32 7, !"uwtable", i32 2}
!5740 = !{i32 7, !"frame-pointer", i32 2}
!5741 = !{!"Debian clang version 15.0.6"}
!5742 = distinct !DISubprogram(name: "backing_file_user_path", scope: !5631, file: !5631, line: 57, type: !5743, scopeLine: 58, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5743 = !DISubroutineType(types: !5744)
!5744 = !{!5303, !896}
!5745 = !DILocalVariable(name: "f", arg: 1, scope: !5742, file: !5631, line: 57, type: !896)
!5746 = !DILocation(line: 57, column: 50, scope: !5742)
!5747 = !DILocation(line: 59, column: 23, scope: !5742)
!5748 = !DILocation(line: 59, column: 10, scope: !5742)
!5749 = !DILocation(line: 59, column: 27, scope: !5742)
!5750 = !DILocation(line: 59, column: 2, scope: !5742)
!5751 = distinct !DISubprogram(name: "backing_file", scope: !5631, file: !5631, line: 52, type: !5752, scopeLine: 53, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5752 = !DISubroutineType(types: !5753)
!5753 = !{!5629, !896}
!5754 = !DILocalVariable(name: "f", arg: 1, scope: !5751, file: !5631, line: 52, type: !896)
!5755 = !DILocation(line: 52, column: 62, scope: !5751)
!5756 = !DILocalVariable(name: "__mptr", scope: !5757, file: !5631, line: 54, type: !40)
!5757 = distinct !DILexicalBlock(scope: !5751, file: !5631, line: 54, column: 9)
!5758 = !DILocation(line: 54, column: 9, scope: !5757)
!5759 = !DILocation(line: 54, column: 2, scope: !5751)
!5760 = distinct !DISubprogram(name: "get_max_files", scope: !5631, file: !5631, line: 88, type: !5761, scopeLine: 89, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5761 = !DISubroutineType(types: !5762)
!5762 = !{!59}
!5763 = !DILocation(line: 90, column: 20, scope: !5760)
!5764 = !DILocation(line: 90, column: 2, scope: !5760)
!5765 = distinct !DISubprogram(name: "init_fs_stat_sysctls", scope: !5631, file: !5631, line: 134, type: !2666, scopeLine: 135, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5766 = !DILocation(line: 136, column: 2, scope: !5765)
!5767 = !DILocalVariable(name: "hdr", scope: !5768, file: !5631, line: 138, type: !186)
!5768 = distinct !DILexicalBlock(scope: !5769, file: !5631, line: 137, column: 38)
!5769 = distinct !DILexicalBlock(scope: !5765, file: !5631, line: 137, column: 6)
!5770 = !DILocation(line: 138, column: 28, scope: !5768)
!5771 = !DILocation(line: 140, column: 9, scope: !5768)
!5772 = !DILocation(line: 140, column: 7, scope: !5768)
!5773 = !DILocation(line: 141, column: 21, scope: !5768)
!5774 = !DILocation(line: 141, column: 3, scope: !5768)
!5775 = !DILocation(line: 143, column: 2, scope: !5765)
!5776 = !DILocalVariable(name: "flags", arg: 1, scope: !5656, file: !5631, line: 191, type: !42)
!5777 = !DILocation(line: 191, column: 35, scope: !5656)
!5778 = !DILocalVariable(name: "cred", arg: 2, scope: !5656, file: !5631, line: 191, type: !490)
!5779 = !DILocation(line: 191, column: 61, scope: !5656)
!5780 = !DILocalVariable(name: "f", scope: !5656, file: !5631, line: 194, type: !896)
!5781 = !DILocation(line: 194, column: 15, scope: !5656)
!5782 = !DILocalVariable(name: "error", scope: !5656, file: !5631, line: 195, type: !42)
!5783 = !DILocation(line: 195, column: 6, scope: !5656)
!5784 = !DILocation(line: 200, column: 6, scope: !5785)
!5785 = distinct !DILexicalBlock(scope: !5656, file: !5631, line: 200, column: 6)
!5786 = !DILocation(line: 200, column: 35, scope: !5785)
!5787 = !DILocation(line: 200, column: 21, scope: !5785)
!5788 = !DILocation(line: 200, column: 45, scope: !5785)
!5789 = !DILocation(line: 200, column: 49, scope: !5785)
!5790 = !DILocation(line: 200, column: 6, scope: !5656)
!5791 = !DILocation(line: 205, column: 7, scope: !5792)
!5792 = distinct !DILexicalBlock(scope: !5793, file: !5631, line: 205, column: 7)
!5793 = distinct !DILexicalBlock(scope: !5785, file: !5631, line: 200, column: 73)
!5794 = !DILocation(line: 205, column: 60, scope: !5792)
!5795 = !DILocation(line: 205, column: 46, scope: !5792)
!5796 = !DILocation(line: 205, column: 7, scope: !5793)
!5797 = !DILocation(line: 206, column: 4, scope: !5792)
!5798 = !DILocation(line: 207, column: 2, scope: !5793)
!5799 = !DILocalVariable(name: "_old", scope: !5800, file: !5631, line: 209, type: !5802)
!5800 = distinct !DILexicalBlock(scope: !5801, file: !5631, line: 209, column: 6)
!5801 = distinct !DILexicalBlock(scope: !5656, file: !5631, line: 209, column: 6)
!5802 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5803, size: 64)
!5803 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "alloc_tag", file: !5804, line: 28, size: 320, align: 64, elements: !5805)
!5804 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/alloc_tag.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "951c2fcedf6c2e064d8f21370a82dd76")
!5805 = !{!5806, !5815}
!5806 = !DIDerivedType(tag: DW_TAG_member, name: "ct", scope: !5803, file: !5804, line: 29, baseType: !5807, size: 256, align: 64)
!5807 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "codetag", file: !5808, line: 21, size: 256, align: 64, elements: !5809)
!5808 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/codetag.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "3498931043637dd583a032ec653e6786")
!5809 = !{!5810, !5811, !5812, !5813, !5814}
!5810 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !5807, file: !5808, line: 22, baseType: !7, size: 32)
!5811 = !DIDerivedType(tag: DW_TAG_member, name: "lineno", scope: !5807, file: !5808, line: 23, baseType: !7, size: 32, offset: 32)
!5812 = !DIDerivedType(tag: DW_TAG_member, name: "modname", scope: !5807, file: !5808, line: 24, baseType: !36, size: 64, offset: 64)
!5813 = !DIDerivedType(tag: DW_TAG_member, name: "function", scope: !5807, file: !5808, line: 25, baseType: !36, size: 64, offset: 128)
!5814 = !DIDerivedType(tag: DW_TAG_member, name: "filename", scope: !5807, file: !5808, line: 26, baseType: !36, size: 64, offset: 192)
!5815 = !DIDerivedType(tag: DW_TAG_member, name: "counters", scope: !5803, file: !5804, line: 30, baseType: !5816, size: 64, offset: 256)
!5816 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5817, size: 64)
!5817 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "alloc_tag_counters", file: !5804, line: 18, size: 128, elements: !5818)
!5818 = !{!5819, !5820}
!5819 = !DIDerivedType(tag: DW_TAG_member, name: "bytes", scope: !5817, file: !5804, line: 19, baseType: !519, size: 64)
!5820 = !DIDerivedType(tag: DW_TAG_member, name: "calls", scope: !5817, file: !5804, line: 20, baseType: !519, size: 64, offset: 64)
!5821 = !DILocation(line: 209, column: 6, scope: !5800)
!5822 = !DILocalVariable(name: "_res", scope: !5800, file: !5631, line: 209, type: !40)
!5823 = !DILocation(line: 209, column: 6, scope: !5824)
!5824 = !DILexicalBlockFile(scope: !5800, file: !5631, discriminator: 0)
!5825 = !DILocation(line: 209, column: 6, scope: !5826)
!5826 = distinct !DILexicalBlock(scope: !5824, file: !5631, line: 209, column: 6)
!5827 = !DILocation(line: 209, column: 6, scope: !5656)
!5828 = !DILocation(line: 209, column: 6, scope: !5801)
!5829 = !DILocation(line: 209, column: 4, scope: !5656)
!5830 = !DILocation(line: 210, column: 6, scope: !5831)
!5831 = distinct !DILexicalBlock(scope: !5656, file: !5631, line: 210, column: 6)
!5832 = !DILocation(line: 210, column: 6, scope: !5656)
!5833 = !DILocation(line: 211, column: 10, scope: !5831)
!5834 = !DILocation(line: 211, column: 3, scope: !5831)
!5835 = !DILocation(line: 213, column: 20, scope: !5656)
!5836 = !DILocation(line: 213, column: 23, scope: !5656)
!5837 = !DILocation(line: 213, column: 30, scope: !5656)
!5838 = !DILocation(line: 213, column: 10, scope: !5656)
!5839 = !DILocation(line: 213, column: 8, scope: !5656)
!5840 = !DILocation(line: 214, column: 6, scope: !5841)
!5841 = distinct !DILexicalBlock(scope: !5656, file: !5631, line: 214, column: 6)
!5842 = !DILocation(line: 214, column: 6, scope: !5656)
!5843 = !DILocation(line: 215, column: 19, scope: !5844)
!5844 = distinct !DILexicalBlock(scope: !5841, file: !5631, line: 214, column: 23)
!5845 = !DILocation(line: 215, column: 32, scope: !5844)
!5846 = !DILocation(line: 215, column: 3, scope: !5844)
!5847 = !DILocation(line: 216, column: 18, scope: !5844)
!5848 = !DILocation(line: 216, column: 10, scope: !5844)
!5849 = !DILocation(line: 216, column: 3, scope: !5844)
!5850 = !DILocation(line: 219, column: 2, scope: !5656)
!5851 = !DILocation(line: 221, column: 9, scope: !5656)
!5852 = !DILocation(line: 221, column: 2, scope: !5656)
!5853 = !DILabel(scope: !5656, name: "over", file: !5631, line: 223)
!5854 = !DILocation(line: 223, column: 1, scope: !5656)
!5855 = !DILocation(line: 225, column: 6, scope: !5856)
!5856 = distinct !DILexicalBlock(scope: !5656, file: !5631, line: 225, column: 6)
!5857 = !DILocation(line: 225, column: 23, scope: !5856)
!5858 = !DILocation(line: 225, column: 21, scope: !5856)
!5859 = !DILocation(line: 225, column: 6, scope: !5656)
!5860 = !DILocation(line: 226, column: 3, scope: !5861)
!5861 = distinct !DILexicalBlock(scope: !5862, file: !5631, line: 226, column: 3)
!5862 = distinct !DILexicalBlock(scope: !5856, file: !5631, line: 225, column: 32)
!5863 = !DILocation(line: 226, column: 3, scope: !5864)
!5864 = distinct !DILexicalBlock(scope: !5861, file: !5631, line: 226, column: 3)
!5865 = !DILocation(line: 227, column: 13, scope: !5862)
!5866 = !DILocation(line: 227, column: 11, scope: !5862)
!5867 = !DILocation(line: 228, column: 2, scope: !5862)
!5868 = !DILocation(line: 229, column: 9, scope: !5656)
!5869 = !DILocation(line: 229, column: 2, scope: !5656)
!5870 = !DILocation(line: 230, column: 1, scope: !5656)
!5871 = distinct !DISubprogram(name: "get_nr_files", scope: !5631, file: !5631, line: 80, type: !5872, scopeLine: 81, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5872 = !DISubroutineType(types: !5873)
!5873 = !{!892}
!5874 = !DILocation(line: 82, column: 9, scope: !5871)
!5875 = !DILocation(line: 82, column: 2, scope: !5871)
!5876 = distinct !DISubprogram(name: "percpu_counter_sum_positive", scope: !676, file: !676, line: 97, type: !5877, scopeLine: 98, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5877 = !DISubroutineType(types: !5878)
!5878 = !{!502, !5879}
!5879 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !675, size: 64)
!5880 = !DILocalVariable(name: "fbc", arg: 1, scope: !5876, file: !676, line: 97, type: !5879)
!5881 = !DILocation(line: 97, column: 70, scope: !5876)
!5882 = !DILocalVariable(name: "ret", scope: !5876, file: !676, line: 99, type: !502)
!5883 = !DILocation(line: 99, column: 6, scope: !5876)
!5884 = !DILocation(line: 99, column: 33, scope: !5876)
!5885 = !DILocation(line: 99, column: 12, scope: !5876)
!5886 = !DILocation(line: 100, column: 9, scope: !5876)
!5887 = !DILocation(line: 100, column: 13, scope: !5876)
!5888 = !DILocation(line: 100, column: 23, scope: !5876)
!5889 = !DILocation(line: 100, column: 2, scope: !5876)
!5890 = distinct !DISubprogram(name: "ERR_PTR", scope: !5891, file: !5891, line: 39, type: !5892, scopeLine: 40, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5891 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/err.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "6a472f130be5f24a69e0cbbc4d28265a")
!5892 = !DISubroutineType(types: !5893)
!5893 = !{!40, !892}
!5894 = !DILocalVariable(name: "error", arg: 1, scope: !5890, file: !5891, line: 39, type: !892)
!5895 = !DILocation(line: 39, column: 48, scope: !5890)
!5896 = !DILocation(line: 41, column: 18, scope: !5890)
!5897 = !DILocation(line: 41, column: 9, scope: !5890)
!5898 = !DILocation(line: 41, column: 2, scope: !5890)
!5899 = !DILocalVariable(name: "f", arg: 1, scope: !5715, file: !5631, line: 148, type: !896)
!5900 = !DILocation(line: 148, column: 35, scope: !5715)
!5901 = !DILocalVariable(name: "flags", arg: 2, scope: !5715, file: !5631, line: 148, type: !42)
!5902 = !DILocation(line: 148, column: 42, scope: !5715)
!5903 = !DILocalVariable(name: "cred", arg: 3, scope: !5715, file: !5631, line: 148, type: !490)
!5904 = !DILocation(line: 148, column: 68, scope: !5715)
!5905 = !DILocalVariable(name: "error", scope: !5715, file: !5631, line: 150, type: !42)
!5906 = !DILocation(line: 150, column: 6, scope: !5715)
!5907 = !DILocation(line: 152, column: 23, scope: !5715)
!5908 = !DILocation(line: 152, column: 14, scope: !5715)
!5909 = !DILocation(line: 152, column: 2, scope: !5715)
!5910 = !DILocation(line: 152, column: 5, scope: !5715)
!5911 = !DILocation(line: 152, column: 12, scope: !5715)
!5912 = !DILocation(line: 153, column: 30, scope: !5715)
!5913 = !DILocation(line: 153, column: 10, scope: !5715)
!5914 = !DILocation(line: 153, column: 8, scope: !5715)
!5915 = !DILocation(line: 154, column: 6, scope: !5916)
!5916 = distinct !DILexicalBlock(scope: !5715, file: !5631, line: 154, column: 6)
!5917 = !DILocation(line: 154, column: 6, scope: !5715)
!5918 = !DILocation(line: 155, column: 12, scope: !5919)
!5919 = distinct !DILexicalBlock(scope: !5916, file: !5631, line: 154, column: 23)
!5920 = !DILocation(line: 155, column: 15, scope: !5919)
!5921 = !DILocation(line: 155, column: 3, scope: !5919)
!5922 = !DILocation(line: 156, column: 10, scope: !5919)
!5923 = !DILocation(line: 156, column: 3, scope: !5919)
!5924 = !DILocation(line: 159, column: 2, scope: !5715)
!5925 = !DILocation(line: 159, column: 2, scope: !5926)
!5926 = distinct !DILexicalBlock(scope: !5715, file: !5631, line: 159, column: 2)
!5927 = !DILocalVariable(name: "lock", arg: 1, scope: !5928, file: !5929, line: 324, type: !1486)
!5928 = distinct !DISubprogram(name: "spinlock_check", scope: !5929, file: !5929, line: 324, type: !5930, scopeLine: 325, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5929 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/spinlock.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "1219c391578d79adb6ca15b58ba8b188")
!5930 = !DISubroutineType(types: !5931)
!5931 = !{!5932, !1486}
!5932 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !148, size: 64)
!5933 = !DILocation(line: 324, column: 67, scope: !5928, inlinedAt: !5934)
!5934 = distinct !DILocation(line: 159, column: 2, scope: !5926)
!5935 = !DILocation(line: 326, column: 10, scope: !5928, inlinedAt: !5934)
!5936 = !DILocation(line: 167, column: 2, scope: !5715)
!5937 = !DILocation(line: 167, column: 2, scope: !5938)
!5938 = distinct !DILexicalBlock(scope: !5715, file: !5631, line: 167, column: 2)
!5939 = !DILocation(line: 168, column: 15, scope: !5715)
!5940 = !DILocation(line: 168, column: 2, scope: !5715)
!5941 = !DILocation(line: 168, column: 5, scope: !5715)
!5942 = !DILocation(line: 168, column: 13, scope: !5715)
!5943 = !DILocation(line: 169, column: 14, scope: !5715)
!5944 = !DILocation(line: 169, column: 2, scope: !5715)
!5945 = !DILocation(line: 169, column: 5, scope: !5715)
!5946 = !DILocation(line: 169, column: 12, scope: !5715)
!5947 = !DILocation(line: 177, column: 19, scope: !5715)
!5948 = !DILocation(line: 177, column: 22, scope: !5715)
!5949 = !DILocalVariable(name: "v", arg: 1, scope: !5950, file: !5951, line: 3221, type: !5330)
!5950 = distinct !DISubprogram(name: "atomic_long_set", scope: !5951, file: !5951, line: 3221, type: !5952, scopeLine: 3222, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5951 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/atomic/atomic-instrumented.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "cbcf28c9551c8d2b361eea57e96a90b3")
!5952 = !DISubroutineType(types: !5953)
!5953 = !{null, !5330, !892}
!5954 = !DILocation(line: 3221, column: 32, scope: !5950, inlinedAt: !5955)
!5955 = distinct !DILocation(line: 177, column: 2, scope: !5715)
!5956 = !DILocalVariable(name: "i", arg: 2, scope: !5950, file: !5951, line: 3221, type: !892)
!5957 = !DILocation(line: 3221, column: 40, scope: !5950, inlinedAt: !5955)
!5958 = !DILocation(line: 3223, column: 26, scope: !5950, inlinedAt: !5955)
!5959 = !DILocalVariable(name: "v", arg: 1, scope: !5960, file: !5961, line: 80, type: !5964)
!5960 = distinct !DISubprogram(name: "instrument_atomic_write", scope: !5961, file: !5961, line: 80, type: !5962, scopeLine: 81, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5961 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/instrumented.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "a9ea79535c8f314985e8ef947dfc4a6a")
!5962 = !DISubroutineType(types: !5963)
!5963 = !{null, !5964, !55}
!5964 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5965, size: 64)
!5965 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !5966)
!5966 = !DIDerivedType(tag: DW_TAG_volatile_type, baseType: null)
!5967 = !DILocation(line: 80, column: 74, scope: !5960, inlinedAt: !5968)
!5968 = distinct !DILocation(line: 3223, column: 2, scope: !5950, inlinedAt: !5955)
!5969 = !DILocalVariable(name: "size", arg: 2, scope: !5960, file: !5961, line: 80, type: !55)
!5970 = !DILocation(line: 80, column: 84, scope: !5960, inlinedAt: !5968)
!5971 = !DILocation(line: 82, column: 20, scope: !5960, inlinedAt: !5968)
!5972 = !DILocation(line: 82, column: 23, scope: !5960, inlinedAt: !5968)
!5973 = !DILocation(line: 82, column: 2, scope: !5960, inlinedAt: !5968)
!5974 = !DILocation(line: 83, column: 2, scope: !5960, inlinedAt: !5968)
!5975 = !DILocation(line: 3224, column: 22, scope: !5950, inlinedAt: !5955)
!5976 = !DILocation(line: 3224, column: 25, scope: !5950, inlinedAt: !5955)
!5977 = !DILocalVariable(name: "v", arg: 1, scope: !5978, file: !497, line: 76, type: !5330)
!5978 = distinct !DISubprogram(name: "raw_atomic_long_set", scope: !497, file: !497, line: 76, type: !5952, scopeLine: 77, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5979 = !DILocation(line: 76, column: 36, scope: !5978, inlinedAt: !5980)
!5980 = distinct !DILocation(line: 3224, column: 2, scope: !5950, inlinedAt: !5955)
!5981 = !DILocalVariable(name: "i", arg: 2, scope: !5978, file: !497, line: 76, type: !892)
!5982 = !DILocation(line: 76, column: 44, scope: !5978, inlinedAt: !5980)
!5983 = !DILocation(line: 79, column: 19, scope: !5978, inlinedAt: !5980)
!5984 = !DILocation(line: 79, column: 22, scope: !5978, inlinedAt: !5980)
!5985 = !DILocalVariable(name: "v", arg: 1, scope: !5986, file: !5987, line: 2627, type: !5990)
!5986 = distinct !DISubprogram(name: "raw_atomic64_set", scope: !5987, file: !5987, line: 2627, type: !5988, scopeLine: 2628, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5987 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/atomic/atomic-arch-fallback.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "4d0e2c5a75812a61e0164a1dbe3e1930")
!5988 = !DISubroutineType(types: !5989)
!5989 = !{null, !5990, !502}
!5990 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !498, size: 64)
!5991 = !DILocation(line: 2627, column: 30, scope: !5986, inlinedAt: !5992)
!5992 = distinct !DILocation(line: 79, column: 2, scope: !5978, inlinedAt: !5980)
!5993 = !DILocalVariable(name: "i", arg: 2, scope: !5986, file: !5987, line: 2627, type: !502)
!5994 = !DILocation(line: 2627, column: 37, scope: !5986, inlinedAt: !5992)
!5995 = !DILocation(line: 2629, column: 20, scope: !5986, inlinedAt: !5992)
!5996 = !DILocation(line: 2629, column: 23, scope: !5986, inlinedAt: !5992)
!5997 = !DILocalVariable(name: "v", arg: 1, scope: !5998, file: !5999, line: 18, type: !5990)
!5998 = distinct !DISubprogram(name: "arch_atomic64_set", scope: !5999, file: !5999, line: 18, type: !5988, scopeLine: 19, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!5999 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/atomic64_64.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "d369ef1daa09ba314d85268bf45a7f1d")
!6000 = !DILocation(line: 18, column: 59, scope: !5998, inlinedAt: !6001)
!6001 = distinct !DILocation(line: 2629, column: 2, scope: !5986, inlinedAt: !5992)
!6002 = !DILocalVariable(name: "i", arg: 2, scope: !5998, file: !5999, line: 18, type: !502)
!6003 = !DILocation(line: 18, column: 66, scope: !5998, inlinedAt: !6001)
!6004 = !DILocation(line: 20, column: 2, scope: !6005, inlinedAt: !6001)
!6005 = distinct !DILexicalBlock(scope: !5998, file: !5999, line: 20, column: 2)
!6006 = !DILocation(line: 178, column: 2, scope: !5715)
!6007 = !DILocation(line: 179, column: 1, scope: !5715)
!6008 = distinct !DISubprogram(name: "percpu_counter_inc", scope: !676, file: !676, line: 265, type: !6009, scopeLine: 266, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6009 = !DISubroutineType(types: !6010)
!6010 = !{null, !5879}
!6011 = !DILocalVariable(name: "fbc", arg: 1, scope: !6008, file: !676, line: 265, type: !5879)
!6012 = !DILocation(line: 265, column: 62, scope: !6008)
!6013 = !DILocation(line: 267, column: 21, scope: !6008)
!6014 = !DILocation(line: 267, column: 2, scope: !6008)
!6015 = !DILocation(line: 268, column: 1, scope: !6008)
!6016 = distinct !DISubprogram(name: "alloc_empty_file_noaccount", scope: !5631, file: !5631, line: 238, type: !5657, scopeLine: 239, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6017 = !DILocalVariable(name: "flags", arg: 1, scope: !6016, file: !5631, line: 238, type: !42)
!6018 = !DILocation(line: 238, column: 45, scope: !6016)
!6019 = !DILocalVariable(name: "cred", arg: 2, scope: !6016, file: !5631, line: 238, type: !490)
!6020 = !DILocation(line: 238, column: 71, scope: !6016)
!6021 = !DILocalVariable(name: "f", scope: !6016, file: !5631, line: 240, type: !896)
!6022 = !DILocation(line: 240, column: 15, scope: !6016)
!6023 = !DILocalVariable(name: "error", scope: !6016, file: !5631, line: 241, type: !42)
!6024 = !DILocation(line: 241, column: 6, scope: !6016)
!6025 = !DILocalVariable(name: "_old", scope: !6026, file: !5631, line: 243, type: !5802)
!6026 = distinct !DILexicalBlock(scope: !6027, file: !5631, line: 243, column: 6)
!6027 = distinct !DILexicalBlock(scope: !6016, file: !5631, line: 243, column: 6)
!6028 = !DILocation(line: 243, column: 6, scope: !6026)
!6029 = !DILocalVariable(name: "_res", scope: !6026, file: !5631, line: 243, type: !40)
!6030 = !DILocation(line: 243, column: 6, scope: !6031)
!6031 = !DILexicalBlockFile(scope: !6026, file: !5631, discriminator: 0)
!6032 = !DILocation(line: 243, column: 6, scope: !6033)
!6033 = distinct !DILexicalBlock(scope: !6031, file: !5631, line: 243, column: 6)
!6034 = !DILocation(line: 243, column: 6, scope: !6016)
!6035 = !DILocation(line: 243, column: 6, scope: !6027)
!6036 = !DILocation(line: 243, column: 4, scope: !6016)
!6037 = !DILocation(line: 244, column: 6, scope: !6038)
!6038 = distinct !DILexicalBlock(scope: !6016, file: !5631, line: 244, column: 6)
!6039 = !DILocation(line: 244, column: 6, scope: !6016)
!6040 = !DILocation(line: 245, column: 10, scope: !6038)
!6041 = !DILocation(line: 245, column: 3, scope: !6038)
!6042 = !DILocation(line: 247, column: 20, scope: !6016)
!6043 = !DILocation(line: 247, column: 23, scope: !6016)
!6044 = !DILocation(line: 247, column: 30, scope: !6016)
!6045 = !DILocation(line: 247, column: 10, scope: !6016)
!6046 = !DILocation(line: 247, column: 8, scope: !6016)
!6047 = !DILocation(line: 248, column: 6, scope: !6048)
!6048 = distinct !DILexicalBlock(scope: !6016, file: !5631, line: 248, column: 6)
!6049 = !DILocation(line: 248, column: 6, scope: !6016)
!6050 = !DILocation(line: 249, column: 19, scope: !6051)
!6051 = distinct !DILexicalBlock(scope: !6048, file: !5631, line: 248, column: 23)
!6052 = !DILocation(line: 249, column: 32, scope: !6051)
!6053 = !DILocation(line: 249, column: 3, scope: !6051)
!6054 = !DILocation(line: 250, column: 18, scope: !6051)
!6055 = !DILocation(line: 250, column: 10, scope: !6051)
!6056 = !DILocation(line: 250, column: 3, scope: !6051)
!6057 = !DILocation(line: 253, column: 2, scope: !6016)
!6058 = !DILocation(line: 253, column: 5, scope: !6016)
!6059 = !DILocation(line: 253, column: 12, scope: !6016)
!6060 = !DILocation(line: 255, column: 9, scope: !6016)
!6061 = !DILocation(line: 255, column: 2, scope: !6016)
!6062 = !DILocation(line: 256, column: 1, scope: !6016)
!6063 = distinct !DISubprogram(name: "alloc_empty_backing_file", scope: !5631, file: !5631, line: 265, type: !5657, scopeLine: 266, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6064 = !DILocalVariable(name: "flags", arg: 1, scope: !6063, file: !5631, line: 265, type: !42)
!6065 = !DILocation(line: 265, column: 43, scope: !6063)
!6066 = !DILocalVariable(name: "cred", arg: 2, scope: !6063, file: !5631, line: 265, type: !490)
!6067 = !DILocation(line: 265, column: 69, scope: !6063)
!6068 = !DILocalVariable(name: "ff", scope: !6063, file: !5631, line: 267, type: !5629)
!6069 = !DILocation(line: 267, column: 23, scope: !6063)
!6070 = !DILocalVariable(name: "error", scope: !6063, file: !5631, line: 268, type: !42)
!6071 = !DILocation(line: 268, column: 6, scope: !6063)
!6072 = !DILocalVariable(name: "_old", scope: !6073, file: !5631, line: 270, type: !5802)
!6073 = distinct !DILexicalBlock(scope: !6074, file: !5631, line: 270, column: 7)
!6074 = distinct !DILexicalBlock(scope: !6063, file: !5631, line: 270, column: 7)
!6075 = !DILocation(line: 270, column: 7, scope: !6073)
!6076 = !DILocalVariable(name: "_res", scope: !6073, file: !5631, line: 270, type: !40)
!6077 = !DILocation(line: 270, column: 7, scope: !6078)
!6078 = distinct !DILexicalBlock(scope: !6073, file: !5631, line: 270, column: 7)
!6079 = !DILocation(line: 270, column: 7, scope: !6063)
!6080 = !DILocation(line: 270, column: 7, scope: !6074)
!6081 = !DILocation(line: 270, column: 5, scope: !6063)
!6082 = !DILocation(line: 271, column: 6, scope: !6083)
!6083 = distinct !DILexicalBlock(scope: !6063, file: !5631, line: 271, column: 6)
!6084 = !DILocation(line: 271, column: 6, scope: !6063)
!6085 = !DILocation(line: 272, column: 10, scope: !6083)
!6086 = !DILocation(line: 272, column: 3, scope: !6083)
!6087 = !DILocation(line: 274, column: 21, scope: !6063)
!6088 = !DILocation(line: 274, column: 25, scope: !6063)
!6089 = !DILocation(line: 274, column: 31, scope: !6063)
!6090 = !DILocation(line: 274, column: 38, scope: !6063)
!6091 = !DILocation(line: 274, column: 10, scope: !6063)
!6092 = !DILocation(line: 274, column: 8, scope: !6063)
!6093 = !DILocation(line: 275, column: 6, scope: !6094)
!6094 = distinct !DILexicalBlock(scope: !6063, file: !5631, line: 275, column: 6)
!6095 = !DILocation(line: 275, column: 6, scope: !6063)
!6096 = !DILocation(line: 276, column: 9, scope: !6097)
!6097 = distinct !DILexicalBlock(scope: !6094, file: !5631, line: 275, column: 23)
!6098 = !DILocation(line: 276, column: 3, scope: !6097)
!6099 = !DILocation(line: 277, column: 18, scope: !6097)
!6100 = !DILocation(line: 277, column: 10, scope: !6097)
!6101 = !DILocation(line: 277, column: 3, scope: !6097)
!6102 = !DILocation(line: 280, column: 2, scope: !6063)
!6103 = !DILocation(line: 280, column: 6, scope: !6063)
!6104 = !DILocation(line: 280, column: 11, scope: !6063)
!6105 = !DILocation(line: 280, column: 18, scope: !6063)
!6106 = !DILocation(line: 281, column: 10, scope: !6063)
!6107 = !DILocation(line: 281, column: 14, scope: !6063)
!6108 = !DILocation(line: 281, column: 2, scope: !6063)
!6109 = !DILocation(line: 282, column: 1, scope: !6063)
!6110 = distinct !DISubprogram(name: "kzalloc_noprof", scope: !416, file: !416, line: 1012, type: !6111, scopeLine: 1013, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6111 = !DISubroutineType(types: !6112)
!6112 = !{!40, !55, !488}
!6113 = !DILocalVariable(name: "size", arg: 1, scope: !6110, file: !416, line: 1012, type: !55)
!6114 = !DILocation(line: 1012, column: 59, scope: !6110)
!6115 = !DILocalVariable(name: "flags", arg: 2, scope: !6110, file: !416, line: 1012, type: !488)
!6116 = !DILocation(line: 1012, column: 71, scope: !6110)
!6117 = !DILocation(line: 1014, column: 24, scope: !6110)
!6118 = !DILocation(line: 1014, column: 30, scope: !6110)
!6119 = !DILocation(line: 1014, column: 36, scope: !6110)
!6120 = !DILocalVariable(name: "size", arg: 1, scope: !6121, file: !416, line: 869, type: !55)
!6121 = distinct !DISubprogram(name: "kmalloc_noprof", scope: !416, file: !416, line: 869, type: !6111, scopeLine: 870, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6122 = !DILocation(line: 869, column: 68, scope: !6121, inlinedAt: !6123)
!6123 = distinct !DILocation(line: 1014, column: 9, scope: !6110)
!6124 = !DILocalVariable(name: "flags", arg: 2, scope: !6121, file: !416, line: 869, type: !488)
!6125 = !DILocation(line: 869, column: 80, scope: !6121, inlinedAt: !6123)
!6126 = !DILocation(line: 871, column: 27, scope: !6127, inlinedAt: !6123)
!6127 = distinct !DILexicalBlock(scope: !6121, file: !416, line: 871, column: 6)
!6128 = !DILocation(line: 871, column: 6, scope: !6127, inlinedAt: !6123)
!6129 = !DILocation(line: 871, column: 33, scope: !6127, inlinedAt: !6123)
!6130 = !DILocation(line: 871, column: 36, scope: !6127, inlinedAt: !6123)
!6131 = !DILocation(line: 871, column: 6, scope: !6121, inlinedAt: !6123)
!6132 = !DILocalVariable(name: "index", scope: !6133, file: !416, line: 872, type: !7)
!6133 = distinct !DILexicalBlock(scope: !6127, file: !416, line: 871, column: 42)
!6134 = !DILocation(line: 872, column: 16, scope: !6133, inlinedAt: !6123)
!6135 = !DILocation(line: 874, column: 7, scope: !6136, inlinedAt: !6123)
!6136 = distinct !DILexicalBlock(scope: !6133, file: !416, line: 874, column: 7)
!6137 = !DILocation(line: 874, column: 12, scope: !6136, inlinedAt: !6123)
!6138 = !DILocation(line: 874, column: 7, scope: !6133, inlinedAt: !6123)
!6139 = !DILocation(line: 875, column: 34, scope: !6136, inlinedAt: !6123)
!6140 = !DILocation(line: 875, column: 40, scope: !6136, inlinedAt: !6123)
!6141 = !DILocation(line: 875, column: 11, scope: !6136, inlinedAt: !6123)
!6142 = !DILocation(line: 875, column: 4, scope: !6136, inlinedAt: !6123)
!6143 = !DILocation(line: 877, column: 11, scope: !6133, inlinedAt: !6123)
!6144 = !DILocalVariable(name: "size", arg: 1, scope: !6145, file: !416, line: 654, type: !55)
!6145 = distinct !DISubprogram(name: "__kmalloc_index", scope: !416, file: !416, line: 654, type: !6146, scopeLine: 656, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6146 = !DISubroutineType(types: !6147)
!6147 = !{!7, !55, !614}
!6148 = !DILocation(line: 654, column: 60, scope: !6145, inlinedAt: !6149)
!6149 = distinct !DILocation(line: 877, column: 11, scope: !6133, inlinedAt: !6123)
!6150 = !DILocalVariable(name: "size_is_constant", arg: 2, scope: !6145, file: !416, line: 655, type: !614)
!6151 = !DILocation(line: 655, column: 16, scope: !6145, inlinedAt: !6149)
!6152 = !DILocation(line: 657, column: 7, scope: !6153, inlinedAt: !6149)
!6153 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 657, column: 6)
!6154 = !DILocation(line: 657, column: 6, scope: !6145, inlinedAt: !6149)
!6155 = !DILocation(line: 658, column: 3, scope: !6153, inlinedAt: !6149)
!6156 = !DILocation(line: 660, column: 6, scope: !6157, inlinedAt: !6149)
!6157 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 660, column: 6)
!6158 = !DILocation(line: 660, column: 11, scope: !6157, inlinedAt: !6149)
!6159 = !DILocation(line: 660, column: 6, scope: !6145, inlinedAt: !6149)
!6160 = !DILocation(line: 661, column: 3, scope: !6157, inlinedAt: !6149)
!6161 = !DILocation(line: 663, column: 32, scope: !6162, inlinedAt: !6149)
!6162 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 663, column: 6)
!6163 = !DILocation(line: 663, column: 37, scope: !6162, inlinedAt: !6149)
!6164 = !DILocation(line: 663, column: 42, scope: !6162, inlinedAt: !6149)
!6165 = !DILocation(line: 663, column: 45, scope: !6162, inlinedAt: !6149)
!6166 = !DILocation(line: 663, column: 50, scope: !6162, inlinedAt: !6149)
!6167 = !DILocation(line: 663, column: 6, scope: !6145, inlinedAt: !6149)
!6168 = !DILocation(line: 664, column: 3, scope: !6162, inlinedAt: !6149)
!6169 = !DILocation(line: 665, column: 32, scope: !6170, inlinedAt: !6149)
!6170 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 665, column: 6)
!6171 = !DILocation(line: 665, column: 37, scope: !6170, inlinedAt: !6149)
!6172 = !DILocation(line: 665, column: 43, scope: !6170, inlinedAt: !6149)
!6173 = !DILocation(line: 665, column: 46, scope: !6170, inlinedAt: !6149)
!6174 = !DILocation(line: 665, column: 51, scope: !6170, inlinedAt: !6149)
!6175 = !DILocation(line: 665, column: 6, scope: !6145, inlinedAt: !6149)
!6176 = !DILocation(line: 666, column: 3, scope: !6170, inlinedAt: !6149)
!6177 = !DILocation(line: 667, column: 6, scope: !6178, inlinedAt: !6149)
!6178 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 667, column: 6)
!6179 = !DILocation(line: 667, column: 11, scope: !6178, inlinedAt: !6149)
!6180 = !DILocation(line: 667, column: 6, scope: !6145, inlinedAt: !6149)
!6181 = !DILocation(line: 667, column: 26, scope: !6178, inlinedAt: !6149)
!6182 = !DILocation(line: 668, column: 6, scope: !6183, inlinedAt: !6149)
!6183 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 668, column: 6)
!6184 = !DILocation(line: 668, column: 11, scope: !6183, inlinedAt: !6149)
!6185 = !DILocation(line: 668, column: 6, scope: !6145, inlinedAt: !6149)
!6186 = !DILocation(line: 668, column: 26, scope: !6183, inlinedAt: !6149)
!6187 = !DILocation(line: 669, column: 6, scope: !6188, inlinedAt: !6149)
!6188 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 669, column: 6)
!6189 = !DILocation(line: 669, column: 11, scope: !6188, inlinedAt: !6149)
!6190 = !DILocation(line: 669, column: 6, scope: !6145, inlinedAt: !6149)
!6191 = !DILocation(line: 669, column: 26, scope: !6188, inlinedAt: !6149)
!6192 = !DILocation(line: 670, column: 6, scope: !6193, inlinedAt: !6149)
!6193 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 670, column: 6)
!6194 = !DILocation(line: 670, column: 11, scope: !6193, inlinedAt: !6149)
!6195 = !DILocation(line: 670, column: 6, scope: !6145, inlinedAt: !6149)
!6196 = !DILocation(line: 670, column: 26, scope: !6193, inlinedAt: !6149)
!6197 = !DILocation(line: 671, column: 6, scope: !6198, inlinedAt: !6149)
!6198 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 671, column: 6)
!6199 = !DILocation(line: 671, column: 11, scope: !6198, inlinedAt: !6149)
!6200 = !DILocation(line: 671, column: 6, scope: !6145, inlinedAt: !6149)
!6201 = !DILocation(line: 671, column: 26, scope: !6198, inlinedAt: !6149)
!6202 = !DILocation(line: 672, column: 6, scope: !6203, inlinedAt: !6149)
!6203 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 672, column: 6)
!6204 = !DILocation(line: 672, column: 11, scope: !6203, inlinedAt: !6149)
!6205 = !DILocation(line: 672, column: 6, scope: !6145, inlinedAt: !6149)
!6206 = !DILocation(line: 672, column: 26, scope: !6203, inlinedAt: !6149)
!6207 = !DILocation(line: 673, column: 6, scope: !6208, inlinedAt: !6149)
!6208 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 673, column: 6)
!6209 = !DILocation(line: 673, column: 11, scope: !6208, inlinedAt: !6149)
!6210 = !DILocation(line: 673, column: 6, scope: !6145, inlinedAt: !6149)
!6211 = !DILocation(line: 673, column: 26, scope: !6208, inlinedAt: !6149)
!6212 = !DILocation(line: 674, column: 6, scope: !6213, inlinedAt: !6149)
!6213 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 674, column: 6)
!6214 = !DILocation(line: 674, column: 11, scope: !6213, inlinedAt: !6149)
!6215 = !DILocation(line: 674, column: 6, scope: !6145, inlinedAt: !6149)
!6216 = !DILocation(line: 674, column: 26, scope: !6213, inlinedAt: !6149)
!6217 = !DILocation(line: 675, column: 6, scope: !6218, inlinedAt: !6149)
!6218 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 675, column: 6)
!6219 = !DILocation(line: 675, column: 11, scope: !6218, inlinedAt: !6149)
!6220 = !DILocation(line: 675, column: 6, scope: !6145, inlinedAt: !6149)
!6221 = !DILocation(line: 675, column: 26, scope: !6218, inlinedAt: !6149)
!6222 = !DILocation(line: 676, column: 6, scope: !6223, inlinedAt: !6149)
!6223 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 676, column: 6)
!6224 = !DILocation(line: 676, column: 11, scope: !6223, inlinedAt: !6149)
!6225 = !DILocation(line: 676, column: 6, scope: !6145, inlinedAt: !6149)
!6226 = !DILocation(line: 676, column: 26, scope: !6223, inlinedAt: !6149)
!6227 = !DILocation(line: 677, column: 6, scope: !6228, inlinedAt: !6149)
!6228 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 677, column: 6)
!6229 = !DILocation(line: 677, column: 11, scope: !6228, inlinedAt: !6149)
!6230 = !DILocation(line: 677, column: 6, scope: !6145, inlinedAt: !6149)
!6231 = !DILocation(line: 677, column: 26, scope: !6228, inlinedAt: !6149)
!6232 = !DILocation(line: 678, column: 6, scope: !6233, inlinedAt: !6149)
!6233 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 678, column: 6)
!6234 = !DILocation(line: 678, column: 11, scope: !6233, inlinedAt: !6149)
!6235 = !DILocation(line: 678, column: 6, scope: !6145, inlinedAt: !6149)
!6236 = !DILocation(line: 678, column: 26, scope: !6233, inlinedAt: !6149)
!6237 = !DILocation(line: 679, column: 6, scope: !6238, inlinedAt: !6149)
!6238 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 679, column: 6)
!6239 = !DILocation(line: 679, column: 11, scope: !6238, inlinedAt: !6149)
!6240 = !DILocation(line: 679, column: 6, scope: !6145, inlinedAt: !6149)
!6241 = !DILocation(line: 679, column: 26, scope: !6238, inlinedAt: !6149)
!6242 = !DILocation(line: 680, column: 6, scope: !6243, inlinedAt: !6149)
!6243 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 680, column: 6)
!6244 = !DILocation(line: 680, column: 11, scope: !6243, inlinedAt: !6149)
!6245 = !DILocation(line: 680, column: 6, scope: !6145, inlinedAt: !6149)
!6246 = !DILocation(line: 680, column: 26, scope: !6243, inlinedAt: !6149)
!6247 = !DILocation(line: 681, column: 6, scope: !6248, inlinedAt: !6149)
!6248 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 681, column: 6)
!6249 = !DILocation(line: 681, column: 11, scope: !6248, inlinedAt: !6149)
!6250 = !DILocation(line: 681, column: 6, scope: !6145, inlinedAt: !6149)
!6251 = !DILocation(line: 681, column: 26, scope: !6248, inlinedAt: !6149)
!6252 = !DILocation(line: 682, column: 6, scope: !6253, inlinedAt: !6149)
!6253 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 682, column: 6)
!6254 = !DILocation(line: 682, column: 11, scope: !6253, inlinedAt: !6149)
!6255 = !DILocation(line: 682, column: 6, scope: !6145, inlinedAt: !6149)
!6256 = !DILocation(line: 682, column: 26, scope: !6253, inlinedAt: !6149)
!6257 = !DILocation(line: 683, column: 6, scope: !6258, inlinedAt: !6149)
!6258 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 683, column: 6)
!6259 = !DILocation(line: 683, column: 11, scope: !6258, inlinedAt: !6149)
!6260 = !DILocation(line: 683, column: 6, scope: !6145, inlinedAt: !6149)
!6261 = !DILocation(line: 683, column: 26, scope: !6258, inlinedAt: !6149)
!6262 = !DILocation(line: 684, column: 6, scope: !6263, inlinedAt: !6149)
!6263 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 684, column: 6)
!6264 = !DILocation(line: 684, column: 11, scope: !6263, inlinedAt: !6149)
!6265 = !DILocation(line: 684, column: 6, scope: !6145, inlinedAt: !6149)
!6266 = !DILocation(line: 684, column: 27, scope: !6263, inlinedAt: !6149)
!6267 = !DILocation(line: 685, column: 6, scope: !6268, inlinedAt: !6149)
!6268 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 685, column: 6)
!6269 = !DILocation(line: 685, column: 11, scope: !6268, inlinedAt: !6149)
!6270 = !DILocation(line: 685, column: 6, scope: !6145, inlinedAt: !6149)
!6271 = !DILocation(line: 685, column: 32, scope: !6268, inlinedAt: !6149)
!6272 = !DILocation(line: 687, column: 50, scope: !6273, inlinedAt: !6149)
!6273 = distinct !DILexicalBlock(scope: !6145, file: !416, line: 687, column: 6)
!6274 = !DILocation(line: 687, column: 6, scope: !6145, inlinedAt: !6149)
!6275 = !DILocation(line: 693, column: 2, scope: !6145, inlinedAt: !6149)
!6276 = !DILocation(line: 690, column: 3, scope: !6277, inlinedAt: !6149)
!6277 = distinct !DILexicalBlock(scope: !6278, file: !416, line: 690, column: 3)
!6278 = distinct !DILexicalBlock(scope: !6273, file: !416, line: 690, column: 3)
!6279 = !{i64 2151598131, i64 2151597940, i64 2151597992, i64 2151598038, i64 2151598066}
!6280 = !DILocation(line: 690, column: 3, scope: !6281, inlinedAt: !6149)
!6281 = distinct !DILexicalBlock(scope: !6278, file: !416, line: 690, column: 3)
!6282 = !{i64 2151598205, i64 2151598234, i64 2151598280, i64 2151598338, i64 2151598392, i64 2151598446, i64 2151598501, i64 2151598532}
!6283 = !DILocation(line: 690, column: 3, scope: !6278, inlinedAt: !6149)
!6284 = !DILocation(line: 694, column: 1, scope: !6145, inlinedAt: !6149)
!6285 = !DILocation(line: 877, column: 9, scope: !6133, inlinedAt: !6123)
!6286 = !DILocation(line: 879, column: 33, scope: !6133, inlinedAt: !6123)
!6287 = !DILocation(line: 879, column: 40, scope: !6133, inlinedAt: !6123)
!6288 = !DILocalVariable(name: "flags", arg: 1, scope: !6289, file: !416, line: 611, type: !488)
!6289 = distinct !DISubprogram(name: "kmalloc_type", scope: !416, file: !416, line: 611, type: !6290, scopeLine: 612, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6290 = !DISubroutineType(types: !6291)
!6291 = !{!456, !488, !59}
!6292 = !DILocation(line: 611, column: 67, scope: !6289, inlinedAt: !6293)
!6293 = distinct !DILocation(line: 879, column: 20, scope: !6133, inlinedAt: !6123)
!6294 = !DILocalVariable(name: "caller", arg: 2, scope: !6289, file: !416, line: 611, type: !59)
!6295 = !DILocation(line: 611, column: 88, scope: !6289, inlinedAt: !6293)
!6296 = !DILocation(line: 617, column: 6, scope: !6297, inlinedAt: !6293)
!6297 = distinct !DILexicalBlock(scope: !6289, file: !416, line: 617, column: 6)
!6298 = !DILocation(line: 617, column: 6, scope: !6289, inlinedAt: !6293)
!6299 = !DILocation(line: 623, column: 3, scope: !6297, inlinedAt: !6293)
!6300 = !DILocation(line: 633, column: 38, scope: !6301, inlinedAt: !6293)
!6301 = distinct !DILexicalBlock(scope: !6289, file: !416, line: 633, column: 6)
!6302 = !DILocation(line: 633, column: 44, scope: !6301, inlinedAt: !6293)
!6303 = !DILocation(line: 633, column: 6, scope: !6289, inlinedAt: !6293)
!6304 = !DILocation(line: 634, column: 3, scope: !6301, inlinedAt: !6293)
!6305 = !DILocation(line: 636, column: 3, scope: !6306, inlinedAt: !6293)
!6306 = distinct !DILexicalBlock(scope: !6289, file: !416, line: 635, column: 6)
!6307 = !DILocation(line: 639, column: 1, scope: !6289, inlinedAt: !6293)
!6308 = !DILocation(line: 879, column: 5, scope: !6133, inlinedAt: !6123)
!6309 = !DILocation(line: 879, column: 51, scope: !6133, inlinedAt: !6123)
!6310 = !DILocation(line: 880, column: 5, scope: !6133, inlinedAt: !6123)
!6311 = !DILocation(line: 880, column: 12, scope: !6133, inlinedAt: !6123)
!6312 = !DILocation(line: 878, column: 10, scope: !6133, inlinedAt: !6123)
!6313 = !DILocation(line: 878, column: 3, scope: !6133, inlinedAt: !6123)
!6314 = !DILocation(line: 882, column: 26, scope: !6121, inlinedAt: !6123)
!6315 = !DILocation(line: 882, column: 32, scope: !6121, inlinedAt: !6123)
!6316 = !DILocation(line: 882, column: 9, scope: !6121, inlinedAt: !6123)
!6317 = !DILocation(line: 882, column: 2, scope: !6121, inlinedAt: !6123)
!6318 = !DILocation(line: 883, column: 1, scope: !6121, inlinedAt: !6123)
!6319 = !DILocation(line: 1014, column: 2, scope: !6110)
!6320 = distinct !DISubprogram(name: "alloc_file_pseudo", scope: !5631, file: !5631, line: 345, type: !6321, scopeLine: 348, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6321 = !DISubroutineType(types: !6322)
!6322 = !{!896, !779, !3166, !36, !42, !903}
!6323 = !DILocalVariable(name: "inode", arg: 1, scope: !6320, file: !5631, line: 345, type: !779)
!6324 = !DILocation(line: 345, column: 46, scope: !6320)
!6325 = !DILocalVariable(name: "mnt", arg: 2, scope: !6320, file: !5631, line: 345, type: !3166)
!6326 = !DILocation(line: 345, column: 70, scope: !6320)
!6327 = !DILocalVariable(name: "name", arg: 3, scope: !6320, file: !5631, line: 346, type: !36)
!6328 = !DILocation(line: 346, column: 23, scope: !6320)
!6329 = !DILocalVariable(name: "flags", arg: 4, scope: !6320, file: !5631, line: 346, type: !42)
!6330 = !DILocation(line: 346, column: 33, scope: !6320)
!6331 = !DILocalVariable(name: "fops", arg: 5, scope: !6320, file: !5631, line: 347, type: !903)
!6332 = !DILocation(line: 347, column: 41, scope: !6320)
!6333 = !DILocalVariable(name: "ret", scope: !6320, file: !5631, line: 349, type: !42)
!6334 = !DILocation(line: 349, column: 6, scope: !6320)
!6335 = !DILocalVariable(name: "path", scope: !6320, file: !5631, line: 350, type: !3162)
!6336 = !DILocation(line: 350, column: 14, scope: !6320)
!6337 = !DILocalVariable(name: "file", scope: !6320, file: !5631, line: 351, type: !896)
!6338 = !DILocation(line: 351, column: 15, scope: !6320)
!6339 = !DILocation(line: 353, column: 26, scope: !6320)
!6340 = !DILocation(line: 353, column: 32, scope: !6320)
!6341 = !DILocation(line: 353, column: 39, scope: !6320)
!6342 = !DILocation(line: 353, column: 8, scope: !6320)
!6343 = !DILocation(line: 353, column: 6, scope: !6320)
!6344 = !DILocation(line: 354, column: 6, scope: !6345)
!6345 = distinct !DILexicalBlock(scope: !6320, file: !5631, line: 354, column: 6)
!6346 = !DILocation(line: 354, column: 6, scope: !6320)
!6347 = !DILocation(line: 355, column: 18, scope: !6345)
!6348 = !DILocation(line: 355, column: 10, scope: !6345)
!6349 = !DILocation(line: 355, column: 3, scope: !6345)
!6350 = !DILocation(line: 357, column: 27, scope: !6320)
!6351 = !DILocation(line: 357, column: 34, scope: !6320)
!6352 = !DILocation(line: 357, column: 9, scope: !6320)
!6353 = !DILocation(line: 357, column: 7, scope: !6320)
!6354 = !DILocation(line: 358, column: 13, scope: !6355)
!6355 = distinct !DILexicalBlock(scope: !6320, file: !5631, line: 358, column: 6)
!6356 = !DILocation(line: 358, column: 6, scope: !6355)
!6357 = !DILocation(line: 358, column: 6, scope: !6320)
!6358 = !DILocation(line: 359, column: 9, scope: !6359)
!6359 = distinct !DILexicalBlock(scope: !6355, file: !5631, line: 358, column: 20)
!6360 = !DILocation(line: 359, column: 3, scope: !6359)
!6361 = !DILocation(line: 360, column: 3, scope: !6359)
!6362 = !DILocation(line: 361, column: 2, scope: !6359)
!6363 = !DILocation(line: 362, column: 9, scope: !6320)
!6364 = !DILocation(line: 362, column: 2, scope: !6320)
!6365 = !DILocation(line: 363, column: 1, scope: !6320)
!6366 = distinct !DISubprogram(name: "alloc_path_pseudo", scope: !5631, file: !5631, line: 332, type: !6367, scopeLine: 334, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6367 = !DISubroutineType(types: !6368)
!6368 = !{!42, !36, !779, !3166, !5303}
!6369 = !DILocalVariable(name: "name", arg: 1, scope: !6366, file: !5631, line: 332, type: !36)
!6370 = !DILocation(line: 332, column: 49, scope: !6366)
!6371 = !DILocalVariable(name: "inode", arg: 2, scope: !6366, file: !5631, line: 332, type: !779)
!6372 = !DILocation(line: 332, column: 69, scope: !6366)
!6373 = !DILocalVariable(name: "mnt", arg: 3, scope: !6366, file: !5631, line: 333, type: !3166)
!6374 = !DILocation(line: 333, column: 26, scope: !6366)
!6375 = !DILocalVariable(name: "path", arg: 4, scope: !6366, file: !5631, line: 333, type: !5303)
!6376 = !DILocation(line: 333, column: 44, scope: !6366)
!6377 = !DILocalVariable(name: "this", scope: !6366, file: !5631, line: 335, type: !764)
!6378 = !DILocation(line: 335, column: 14, scope: !6366)
!6379 = !DILocation(line: 335, column: 21, scope: !6366)
!6380 = !DILocation(line: 337, column: 32, scope: !6366)
!6381 = !DILocation(line: 337, column: 37, scope: !6366)
!6382 = !DILocation(line: 337, column: 17, scope: !6366)
!6383 = !DILocation(line: 337, column: 2, scope: !6366)
!6384 = !DILocation(line: 337, column: 8, scope: !6366)
!6385 = !DILocation(line: 337, column: 15, scope: !6366)
!6386 = !DILocation(line: 338, column: 7, scope: !6387)
!6387 = distinct !DILexicalBlock(scope: !6366, file: !5631, line: 338, column: 6)
!6388 = !DILocation(line: 338, column: 13, scope: !6387)
!6389 = !DILocation(line: 338, column: 6, scope: !6366)
!6390 = !DILocation(line: 339, column: 3, scope: !6387)
!6391 = !DILocation(line: 340, column: 21, scope: !6366)
!6392 = !DILocation(line: 340, column: 14, scope: !6366)
!6393 = !DILocation(line: 340, column: 2, scope: !6366)
!6394 = !DILocation(line: 340, column: 8, scope: !6366)
!6395 = !DILocation(line: 340, column: 12, scope: !6366)
!6396 = !DILocation(line: 341, column: 16, scope: !6366)
!6397 = !DILocation(line: 341, column: 22, scope: !6366)
!6398 = !DILocation(line: 341, column: 30, scope: !6366)
!6399 = !DILocation(line: 341, column: 2, scope: !6366)
!6400 = !DILocation(line: 342, column: 2, scope: !6366)
!6401 = !DILocation(line: 343, column: 1, scope: !6366)
!6402 = distinct !DISubprogram(name: "alloc_file", scope: !5631, file: !5631, line: 321, type: !6403, scopeLine: 323, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6403 = !DISubroutineType(types: !6404)
!6404 = !{!896, !3398, !42, !903}
!6405 = !DILocalVariable(name: "path", arg: 1, scope: !6402, file: !5631, line: 321, type: !3398)
!6406 = !DILocation(line: 321, column: 51, scope: !6402)
!6407 = !DILocalVariable(name: "flags", arg: 2, scope: !6402, file: !5631, line: 321, type: !42)
!6408 = !DILocation(line: 321, column: 61, scope: !6402)
!6409 = !DILocalVariable(name: "fop", arg: 3, scope: !6402, file: !5631, line: 322, type: !903)
!6410 = !DILocation(line: 322, column: 33, scope: !6402)
!6411 = !DILocalVariable(name: "file", scope: !6402, file: !5631, line: 324, type: !896)
!6412 = !DILocation(line: 324, column: 15, scope: !6402)
!6413 = !DILocation(line: 326, column: 26, scope: !6402)
!6414 = !DILocation(line: 326, column: 33, scope: !6415)
!6415 = distinct !DILexicalBlock(scope: !6402, file: !5631, line: 326, column: 33)
!6416 = !DILocation(line: 326, column: 33, scope: !6417)
!6417 = distinct !DILexicalBlock(scope: !6415, file: !5631, line: 326, column: 33)
!6418 = !DILocation(line: 47, column: 10, scope: !6419, inlinedAt: !6425)
!6419 = distinct !DILexicalBlock(scope: !6421, file: !6420, line: 47, column: 10)
!6420 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/current.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "27a6c0a3f97d149e7df983a0c78c4e19")
!6421 = distinct !DILexicalBlock(scope: !6422, file: !6420, line: 46, column: 6)
!6422 = distinct !DISubprogram(name: "get_current", scope: !6420, file: !6420, line: 44, type: !6423, scopeLine: 45, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6423 = !DISubroutineType(types: !6424)
!6424 = !{!3628}
!6425 = distinct !DILocation(line: 326, column: 33, scope: !6415)
!6426 = !DILocation(line: 47, column: 10, scope: !6421, inlinedAt: !6425)
!6427 = !DILocation(line: 326, column: 9, scope: !6402)
!6428 = !DILocation(line: 326, column: 7, scope: !6402)
!6429 = !DILocation(line: 327, column: 14, scope: !6430)
!6430 = distinct !DILexicalBlock(scope: !6402, file: !5631, line: 327, column: 6)
!6431 = !DILocation(line: 327, column: 7, scope: !6430)
!6432 = !DILocation(line: 327, column: 6, scope: !6402)
!6433 = !DILocation(line: 328, column: 18, scope: !6430)
!6434 = !DILocation(line: 328, column: 24, scope: !6430)
!6435 = !DILocation(line: 328, column: 30, scope: !6430)
!6436 = !DILocation(line: 328, column: 3, scope: !6430)
!6437 = !DILocation(line: 329, column: 9, scope: !6402)
!6438 = !DILocation(line: 329, column: 2, scope: !6402)
!6439 = distinct !DISubprogram(name: "IS_ERR", scope: !5891, file: !5891, line: 65, type: !6440, scopeLine: 66, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6440 = !DISubroutineType(types: !6441)
!6441 = !{!614, !1298}
!6442 = !DILocalVariable(name: "ptr", arg: 1, scope: !6439, file: !5891, line: 65, type: !1298)
!6443 = !DILocation(line: 65, column: 60, scope: !6439)
!6444 = !DILocation(line: 67, column: 9, scope: !6439)
!6445 = !DILocation(line: 67, column: 2, scope: !6439)
!6446 = distinct !DISubprogram(name: "alloc_file_pseudo_noaccount", scope: !5631, file: !5631, line: 366, type: !6321, scopeLine: 370, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6447 = !DILocalVariable(name: "inode", arg: 1, scope: !6446, file: !5631, line: 366, type: !779)
!6448 = !DILocation(line: 366, column: 56, scope: !6446)
!6449 = !DILocalVariable(name: "mnt", arg: 2, scope: !6446, file: !5631, line: 367, type: !3166)
!6450 = !DILocation(line: 367, column: 24, scope: !6446)
!6451 = !DILocalVariable(name: "name", arg: 3, scope: !6446, file: !5631, line: 367, type: !36)
!6452 = !DILocation(line: 367, column: 41, scope: !6446)
!6453 = !DILocalVariable(name: "flags", arg: 4, scope: !6446, file: !5631, line: 368, type: !42)
!6454 = !DILocation(line: 368, column: 11, scope: !6446)
!6455 = !DILocalVariable(name: "fops", arg: 5, scope: !6446, file: !5631, line: 369, type: !903)
!6456 = !DILocation(line: 369, column: 37, scope: !6446)
!6457 = !DILocalVariable(name: "ret", scope: !6446, file: !5631, line: 371, type: !42)
!6458 = !DILocation(line: 371, column: 6, scope: !6446)
!6459 = !DILocalVariable(name: "path", scope: !6446, file: !5631, line: 372, type: !3162)
!6460 = !DILocation(line: 372, column: 14, scope: !6446)
!6461 = !DILocalVariable(name: "file", scope: !6446, file: !5631, line: 373, type: !896)
!6462 = !DILocation(line: 373, column: 15, scope: !6446)
!6463 = !DILocation(line: 375, column: 26, scope: !6446)
!6464 = !DILocation(line: 375, column: 32, scope: !6446)
!6465 = !DILocation(line: 375, column: 39, scope: !6446)
!6466 = !DILocation(line: 375, column: 8, scope: !6446)
!6467 = !DILocation(line: 375, column: 6, scope: !6446)
!6468 = !DILocation(line: 376, column: 6, scope: !6469)
!6469 = distinct !DILexicalBlock(scope: !6446, file: !5631, line: 376, column: 6)
!6470 = !DILocation(line: 376, column: 6, scope: !6446)
!6471 = !DILocation(line: 377, column: 18, scope: !6469)
!6472 = !DILocation(line: 377, column: 10, scope: !6469)
!6473 = !DILocation(line: 377, column: 3, scope: !6469)
!6474 = !DILocation(line: 379, column: 36, scope: !6446)
!6475 = !DILocation(line: 379, column: 43, scope: !6476)
!6476 = distinct !DILexicalBlock(scope: !6446, file: !5631, line: 379, column: 43)
!6477 = !DILocation(line: 379, column: 43, scope: !6478)
!6478 = distinct !DILexicalBlock(scope: !6476, file: !5631, line: 379, column: 43)
!6479 = !DILocation(line: 47, column: 10, scope: !6419, inlinedAt: !6480)
!6480 = distinct !DILocation(line: 379, column: 43, scope: !6476)
!6481 = !DILocation(line: 47, column: 10, scope: !6421, inlinedAt: !6480)
!6482 = !DILocation(line: 379, column: 9, scope: !6446)
!6483 = !DILocation(line: 379, column: 7, scope: !6446)
!6484 = !DILocation(line: 380, column: 13, scope: !6485)
!6485 = distinct !DILexicalBlock(scope: !6446, file: !5631, line: 380, column: 6)
!6486 = !DILocation(line: 380, column: 6, scope: !6485)
!6487 = !DILocation(line: 380, column: 6, scope: !6446)
!6488 = !DILocation(line: 381, column: 9, scope: !6489)
!6489 = distinct !DILexicalBlock(scope: !6485, file: !5631, line: 380, column: 20)
!6490 = !DILocation(line: 381, column: 3, scope: !6489)
!6491 = !DILocation(line: 382, column: 3, scope: !6489)
!6492 = !DILocation(line: 383, column: 10, scope: !6489)
!6493 = !DILocation(line: 383, column: 3, scope: !6489)
!6494 = !DILocation(line: 385, column: 17, scope: !6446)
!6495 = !DILocation(line: 385, column: 30, scope: !6446)
!6496 = !DILocation(line: 385, column: 2, scope: !6446)
!6497 = !DILocation(line: 386, column: 9, scope: !6446)
!6498 = !DILocation(line: 386, column: 2, scope: !6446)
!6499 = !DILocation(line: 387, column: 1, scope: !6446)
!6500 = distinct !DISubprogram(name: "file_init_path", scope: !5631, file: !5631, line: 291, type: !6501, scopeLine: 293, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6501 = !DISubroutineType(types: !6502)
!6502 = !{null, !896, !3398, !903}
!6503 = !DILocalVariable(name: "file", arg: 1, scope: !6500, file: !5631, line: 291, type: !896)
!6504 = !DILocation(line: 291, column: 41, scope: !6500)
!6505 = !DILocalVariable(name: "path", arg: 2, scope: !6500, file: !5631, line: 291, type: !3398)
!6506 = !DILocation(line: 291, column: 66, scope: !6500)
!6507 = !DILocalVariable(name: "fop", arg: 3, scope: !6500, file: !5631, line: 292, type: !903)
!6508 = !DILocation(line: 292, column: 37, scope: !6500)
!6509 = !DILocation(line: 294, column: 2, scope: !6500)
!6510 = !DILocation(line: 294, column: 8, scope: !6500)
!6511 = !DILocation(line: 294, column: 18, scope: !6500)
!6512 = !DILocation(line: 294, column: 17, scope: !6500)
!6513 = !DILocation(line: 295, column: 18, scope: !6500)
!6514 = !DILocation(line: 295, column: 24, scope: !6500)
!6515 = !DILocation(line: 295, column: 32, scope: !6500)
!6516 = !DILocation(line: 295, column: 2, scope: !6500)
!6517 = !DILocation(line: 295, column: 8, scope: !6500)
!6518 = !DILocation(line: 295, column: 16, scope: !6500)
!6519 = !DILocation(line: 296, column: 20, scope: !6500)
!6520 = !DILocation(line: 296, column: 26, scope: !6500)
!6521 = !DILocation(line: 296, column: 34, scope: !6500)
!6522 = !DILocation(line: 296, column: 43, scope: !6500)
!6523 = !DILocation(line: 296, column: 2, scope: !6500)
!6524 = !DILocation(line: 296, column: 8, scope: !6500)
!6525 = !DILocation(line: 296, column: 18, scope: !6500)
!6526 = !DILocation(line: 297, column: 41, scope: !6500)
!6527 = !DILocation(line: 297, column: 47, scope: !6500)
!6528 = !DILocation(line: 297, column: 19, scope: !6500)
!6529 = !DILocation(line: 297, column: 2, scope: !6500)
!6530 = !DILocation(line: 297, column: 8, scope: !6500)
!6531 = !DILocation(line: 297, column: 17, scope: !6500)
!6532 = !DILocation(line: 298, column: 38, scope: !6500)
!6533 = !DILocation(line: 298, column: 19, scope: !6500)
!6534 = !DILocation(line: 298, column: 2, scope: !6500)
!6535 = !DILocation(line: 298, column: 8, scope: !6500)
!6536 = !DILocation(line: 298, column: 17, scope: !6500)
!6537 = !DILocation(line: 299, column: 6, scope: !6538)
!6538 = distinct !DILexicalBlock(scope: !6500, file: !5631, line: 299, column: 6)
!6539 = !DILocation(line: 299, column: 11, scope: !6538)
!6540 = !DILocation(line: 299, column: 6, scope: !6500)
!6541 = !DILocation(line: 300, column: 3, scope: !6538)
!6542 = !DILocation(line: 300, column: 9, scope: !6538)
!6543 = !DILocation(line: 300, column: 16, scope: !6538)
!6544 = !DILocation(line: 301, column: 7, scope: !6545)
!6545 = distinct !DILexicalBlock(scope: !6500, file: !5631, line: 301, column: 6)
!6546 = !DILocation(line: 301, column: 13, scope: !6545)
!6547 = !DILocation(line: 301, column: 20, scope: !6545)
!6548 = !DILocation(line: 301, column: 34, scope: !6545)
!6549 = !DILocation(line: 302, column: 7, scope: !6545)
!6550 = !DILocation(line: 301, column: 6, scope: !6500)
!6551 = !DILocation(line: 303, column: 3, scope: !6545)
!6552 = !DILocation(line: 303, column: 9, scope: !6545)
!6553 = !DILocation(line: 303, column: 16, scope: !6545)
!6554 = !DILocation(line: 304, column: 7, scope: !6555)
!6555 = distinct !DILexicalBlock(scope: !6500, file: !5631, line: 304, column: 6)
!6556 = !DILocation(line: 304, column: 13, scope: !6555)
!6557 = !DILocation(line: 304, column: 20, scope: !6555)
!6558 = !DILocation(line: 304, column: 35, scope: !6555)
!6559 = !DILocation(line: 305, column: 7, scope: !6555)
!6560 = !DILocation(line: 304, column: 6, scope: !6500)
!6561 = !DILocation(line: 306, column: 3, scope: !6555)
!6562 = !DILocation(line: 306, column: 9, scope: !6555)
!6563 = !DILocation(line: 306, column: 16, scope: !6555)
!6564 = !DILocation(line: 307, column: 34, scope: !6500)
!6565 = !DILocation(line: 307, column: 23, scope: !6500)
!6566 = !DILocation(line: 307, column: 2, scope: !6500)
!6567 = !DILocation(line: 307, column: 8, scope: !6500)
!6568 = !DILocation(line: 307, column: 21, scope: !6500)
!6569 = !DILocation(line: 308, column: 2, scope: !6500)
!6570 = !DILocation(line: 308, column: 8, scope: !6500)
!6571 = !DILocation(line: 308, column: 15, scope: !6500)
!6572 = !DILocation(line: 309, column: 15, scope: !6500)
!6573 = !DILocation(line: 309, column: 2, scope: !6500)
!6574 = !DILocation(line: 309, column: 8, scope: !6500)
!6575 = !DILocation(line: 309, column: 13, scope: !6500)
!6576 = !DILocation(line: 310, column: 7, scope: !6577)
!6577 = distinct !DILexicalBlock(scope: !6500, file: !5631, line: 310, column: 6)
!6578 = !DILocation(line: 310, column: 13, scope: !6577)
!6579 = !DILocation(line: 310, column: 20, scope: !6577)
!6580 = !DILocation(line: 310, column: 50, scope: !6577)
!6581 = !DILocation(line: 310, column: 6, scope: !6500)
!6582 = !DILocation(line: 311, column: 19, scope: !6577)
!6583 = !DILocation(line: 311, column: 25, scope: !6577)
!6584 = !DILocation(line: 311, column: 33, scope: !6577)
!6585 = !DILocation(line: 311, column: 3, scope: !6577)
!6586 = !DILocation(line: 312, column: 1, scope: !6500)
!6587 = distinct !DISubprogram(name: "alloc_file_clone", scope: !5631, file: !5631, line: 390, type: !6588, scopeLine: 392, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6588 = !DISubroutineType(types: !6589)
!6589 = !{!896, !896, !42, !903}
!6590 = !DILocalVariable(name: "base", arg: 1, scope: !6587, file: !5631, line: 390, type: !896)
!6591 = !DILocation(line: 390, column: 44, scope: !6587)
!6592 = !DILocalVariable(name: "flags", arg: 2, scope: !6587, file: !5631, line: 390, type: !42)
!6593 = !DILocation(line: 390, column: 54, scope: !6587)
!6594 = !DILocalVariable(name: "fops", arg: 3, scope: !6587, file: !5631, line: 391, type: !903)
!6595 = !DILocation(line: 391, column: 35, scope: !6587)
!6596 = !DILocalVariable(name: "f", scope: !6587, file: !5631, line: 393, type: !896)
!6597 = !DILocation(line: 393, column: 15, scope: !6587)
!6598 = !DILocation(line: 395, column: 18, scope: !6587)
!6599 = !DILocation(line: 395, column: 24, scope: !6587)
!6600 = !DILocation(line: 395, column: 32, scope: !6587)
!6601 = !DILocation(line: 395, column: 39, scope: !6587)
!6602 = !DILocation(line: 395, column: 6, scope: !6587)
!6603 = !DILocation(line: 395, column: 4, scope: !6587)
!6604 = !DILocation(line: 396, column: 14, scope: !6605)
!6605 = distinct !DILexicalBlock(scope: !6587, file: !5631, line: 396, column: 6)
!6606 = !DILocation(line: 396, column: 7, scope: !6605)
!6607 = !DILocation(line: 396, column: 6, scope: !6587)
!6608 = !DILocation(line: 397, column: 13, scope: !6609)
!6609 = distinct !DILexicalBlock(scope: !6605, file: !5631, line: 396, column: 18)
!6610 = !DILocation(line: 397, column: 16, scope: !6609)
!6611 = !DILocation(line: 397, column: 3, scope: !6609)
!6612 = !DILocation(line: 398, column: 18, scope: !6609)
!6613 = !DILocation(line: 398, column: 24, scope: !6609)
!6614 = !DILocation(line: 398, column: 3, scope: !6609)
!6615 = !DILocation(line: 398, column: 6, scope: !6609)
!6616 = !DILocation(line: 398, column: 16, scope: !6609)
!6617 = !DILocation(line: 399, column: 2, scope: !6609)
!6618 = !DILocation(line: 400, column: 9, scope: !6587)
!6619 = !DILocation(line: 400, column: 2, scope: !6587)
!6620 = distinct !DISubprogram(name: "flush_delayed_fput", scope: !5631, file: !5631, line: 472, type: !2887, scopeLine: 473, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6621 = !DILocation(line: 474, column: 2, scope: !6620)
!6622 = !DILocation(line: 475, column: 1, scope: !6620)
!6623 = distinct !DISubprogram(name: "delayed_fput", scope: !5631, file: !5631, line: 448, type: !1345, scopeLine: 449, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6624 = !DILocalVariable(name: "unused", arg: 1, scope: !6623, file: !5631, line: 448, type: !1347)
!6625 = !DILocation(line: 448, column: 46, scope: !6623)
!6626 = !DILocalVariable(name: "node", scope: !6623, file: !5631, line: 450, type: !3655)
!6627 = !DILocation(line: 450, column: 21, scope: !6623)
!6628 = !DILocation(line: 450, column: 28, scope: !6623)
!6629 = !DILocalVariable(name: "f", scope: !6623, file: !5631, line: 451, type: !896)
!6630 = !DILocation(line: 451, column: 15, scope: !6623)
!6631 = !DILocalVariable(name: "t", scope: !6623, file: !5631, line: 451, type: !896)
!6632 = !DILocation(line: 451, column: 19, scope: !6623)
!6633 = !DILocalVariable(name: "__mptr", scope: !6634, file: !5631, line: 453, type: !40)
!6634 = distinct !DILexicalBlock(scope: !6635, file: !5631, line: 453, column: 2)
!6635 = distinct !DILexicalBlock(scope: !6623, file: !5631, line: 453, column: 2)
!6636 = !DILocation(line: 453, column: 2, scope: !6634)
!6637 = !DILocation(line: 453, column: 2, scope: !6635)
!6638 = !DILocation(line: 453, column: 2, scope: !6639)
!6639 = distinct !DILexicalBlock(scope: !6635, file: !5631, line: 453, column: 2)
!6640 = !DILocalVariable(name: "__mptr", scope: !6641, file: !5631, line: 453, type: !40)
!6641 = distinct !DILexicalBlock(scope: !6639, file: !5631, line: 453, column: 2)
!6642 = !DILocation(line: 453, column: 2, scope: !6641)
!6643 = !DILocation(line: 0, scope: !6639)
!6644 = !DILocation(line: 454, column: 10, scope: !6639)
!6645 = !DILocation(line: 454, column: 3, scope: !6639)
!6646 = distinct !{!6646, !6637, !6647, !6648}
!6647 = !DILocation(line: 454, column: 11, scope: !6635)
!6648 = !{!"llvm.loop.mustprogress"}
!6649 = !DILocation(line: 455, column: 1, scope: !6623)
!6650 = distinct !DISubprogram(name: "fput", scope: !5631, file: !5631, line: 480, type: !2363, scopeLine: 481, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6651 = !DILocalVariable(name: "file", arg: 1, scope: !6650, file: !5631, line: 480, type: !896)
!6652 = !DILocation(line: 480, column: 24, scope: !6650)
!6653 = !DILocation(line: 482, column: 32, scope: !6654)
!6654 = distinct !DILexicalBlock(scope: !6650, file: !5631, line: 482, column: 6)
!6655 = !DILocation(line: 482, column: 38, scope: !6654)
!6656 = !DILocalVariable(name: "v", arg: 1, scope: !6657, file: !5951, line: 4536, type: !5330)
!6657 = distinct !DISubprogram(name: "atomic_long_dec_and_test", scope: !5951, file: !5951, line: 4536, type: !6658, scopeLine: 4537, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6658 = !DISubroutineType(types: !6659)
!6659 = !{!614, !5330}
!6660 = !DILocation(line: 4536, column: 41, scope: !6657, inlinedAt: !6661)
!6661 = distinct !DILocation(line: 482, column: 6, scope: !6654)
!6662 = !DILocation(line: 4539, column: 31, scope: !6657, inlinedAt: !6661)
!6663 = !DILocalVariable(name: "v", arg: 1, scope: !6664, file: !5961, line: 94, type: !5964)
!6664 = distinct !DISubprogram(name: "instrument_atomic_read_write", scope: !5961, file: !5961, line: 94, type: !5962, scopeLine: 95, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6665 = !DILocation(line: 94, column: 79, scope: !6664, inlinedAt: !6666)
!6666 = distinct !DILocation(line: 4539, column: 2, scope: !6657, inlinedAt: !6661)
!6667 = !DILocalVariable(name: "size", arg: 2, scope: !6664, file: !5961, line: 94, type: !55)
!6668 = !DILocation(line: 94, column: 89, scope: !6664, inlinedAt: !6666)
!6669 = !DILocation(line: 96, column: 20, scope: !6664, inlinedAt: !6666)
!6670 = !DILocation(line: 96, column: 23, scope: !6664, inlinedAt: !6666)
!6671 = !DILocation(line: 96, column: 2, scope: !6664, inlinedAt: !6666)
!6672 = !DILocation(line: 97, column: 2, scope: !6664, inlinedAt: !6666)
!6673 = !DILocation(line: 4540, column: 38, scope: !6657, inlinedAt: !6661)
!6674 = !DILocalVariable(name: "v", arg: 1, scope: !6675, file: !497, line: 1568, type: !5330)
!6675 = distinct !DISubprogram(name: "raw_atomic_long_dec_and_test", scope: !497, file: !497, line: 1568, type: !6658, scopeLine: 1569, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6676 = !DILocation(line: 1568, column: 45, scope: !6675, inlinedAt: !6677)
!6677 = distinct !DILocation(line: 4540, column: 9, scope: !6657, inlinedAt: !6661)
!6678 = !DILocation(line: 1571, column: 35, scope: !6675, inlinedAt: !6677)
!6679 = !DILocalVariable(name: "v", arg: 1, scope: !6680, file: !5987, line: 4401, type: !5990)
!6680 = distinct !DISubprogram(name: "raw_atomic64_dec_and_test", scope: !5987, file: !5987, line: 4401, type: !6681, scopeLine: 4402, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6681 = !DISubroutineType(types: !6682)
!6682 = !{!614, !5990}
!6683 = !DILocation(line: 4401, column: 39, scope: !6680, inlinedAt: !6684)
!6684 = distinct !DILocation(line: 1571, column: 9, scope: !6675, inlinedAt: !6677)
!6685 = !DILocation(line: 4404, column: 36, scope: !6680, inlinedAt: !6684)
!6686 = !DILocalVariable(name: "v", arg: 1, scope: !6687, file: !5999, line: 59, type: !5990)
!6687 = distinct !DISubprogram(name: "arch_atomic64_dec_and_test", scope: !5999, file: !5999, line: 59, type: !6681, scopeLine: 60, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6688 = !DILocation(line: 59, column: 68, scope: !6687, inlinedAt: !6689)
!6689 = distinct !DILocation(line: 4404, column: 9, scope: !6680, inlinedAt: !6684)
!6690 = !DILocalVariable(name: "c", scope: !6691, file: !5999, line: 61, type: !614)
!6691 = distinct !DILexicalBlock(scope: !6687, file: !5999, line: 61, column: 9)
!6692 = !DILocation(line: 61, column: 9, scope: !6691, inlinedAt: !6689)
!6693 = !{i64 2148868465, i64 2148868504, i64 2148868525, i64 2148868562, i64 2148868585, i64 2148868594, i64 2148868668}
!6694 = !DILocation(line: 482, column: 6, scope: !6650)
!6695 = !DILocalVariable(name: "task", scope: !6696, file: !5631, line: 483, type: !3628)
!6696 = distinct !DILexicalBlock(scope: !6654, file: !5631, line: 482, column: 48)
!6697 = !DILocation(line: 483, column: 23, scope: !6696)
!6698 = !DILocation(line: 47, column: 10, scope: !6419, inlinedAt: !6699)
!6699 = distinct !DILocation(line: 483, column: 30, scope: !6696)
!6700 = !DILocation(line: 47, column: 10, scope: !6421, inlinedAt: !6699)
!6701 = !DILocation(line: 485, column: 7, scope: !6702)
!6702 = distinct !DILexicalBlock(scope: !6696, file: !5631, line: 485, column: 7)
!6703 = !DILocation(line: 485, column: 7, scope: !6696)
!6704 = !DILocation(line: 486, column: 14, scope: !6705)
!6705 = distinct !DILexicalBlock(scope: !6702, file: !5631, line: 485, column: 67)
!6706 = !DILocation(line: 486, column: 4, scope: !6705)
!6707 = !DILocation(line: 487, column: 4, scope: !6705)
!6708 = !DILocation(line: 26, column: 9, scope: !6709, inlinedAt: !6712)
!6709 = distinct !DILexicalBlock(scope: !6711, file: !6710, line: 26, column: 9)
!6710 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/preempt.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "3f72ac4bd8fca5fb4f3278f6c7aff9f8")
!6711 = distinct !DISubprogram(name: "preempt_count", scope: !6710, file: !6710, line: 24, type: !2666, scopeLine: 25, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6712 = distinct !DILocation(line: 489, column: 7, scope: !6713)
!6713 = distinct !DILexicalBlock(scope: !6696, file: !5631, line: 489, column: 7)
!6714 = !DILocation(line: 26, column: 9, scope: !6711, inlinedAt: !6712)
!6715 = !DILocation(line: 26, column: 48, scope: !6711, inlinedAt: !6712)
!6716 = !DILocation(line: 489, column: 7, scope: !6713)
!6717 = !DILocation(line: 0, scope: !6713)
!6718 = !DILocation(line: 489, column: 7, scope: !6696)
!6719 = !DILocation(line: 490, column: 20, scope: !6720)
!6720 = distinct !DILexicalBlock(scope: !6713, file: !5631, line: 489, column: 63)
!6721 = !DILocation(line: 490, column: 26, scope: !6720)
!6722 = !DILocation(line: 490, column: 4, scope: !6720)
!6723 = !DILocation(line: 491, column: 23, scope: !6724)
!6724 = distinct !DILexicalBlock(scope: !6720, file: !5631, line: 491, column: 8)
!6725 = !DILocation(line: 491, column: 30, scope: !6724)
!6726 = !DILocation(line: 491, column: 36, scope: !6724)
!6727 = !DILocation(line: 491, column: 9, scope: !6724)
!6728 = !DILocation(line: 491, column: 8, scope: !6720)
!6729 = !DILocation(line: 492, column: 5, scope: !6724)
!6730 = !DILocation(line: 498, column: 3, scope: !6720)
!6731 = !DILocation(line: 500, column: 18, scope: !6732)
!6732 = distinct !DILexicalBlock(scope: !6696, file: !5631, line: 500, column: 7)
!6733 = !DILocation(line: 500, column: 24, scope: !6732)
!6734 = !DILocation(line: 500, column: 7, scope: !6732)
!6735 = !DILocation(line: 500, column: 7, scope: !6696)
!6736 = !DILocation(line: 501, column: 4, scope: !6732)
!6737 = !DILocation(line: 502, column: 2, scope: !6696)
!6738 = !DILocation(line: 503, column: 1, scope: !6650)
!6739 = distinct !DISubprogram(name: "file_free", scope: !5631, file: !5631, line: 63, type: !2363, scopeLine: 64, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6740 = !DILocalVariable(name: "f", arg: 1, scope: !6739, file: !5631, line: 63, type: !896)
!6741 = !DILocation(line: 63, column: 43, scope: !6739)
!6742 = !DILocation(line: 65, column: 21, scope: !6739)
!6743 = !DILocation(line: 65, column: 2, scope: !6739)
!6744 = !DILocation(line: 66, column: 6, scope: !6745)
!6745 = distinct !DILexicalBlock(scope: !6739, file: !5631, line: 66, column: 6)
!6746 = !DILocation(line: 66, column: 6, scope: !6739)
!6747 = !DILocation(line: 67, column: 3, scope: !6745)
!6748 = !DILocation(line: 68, column: 11, scope: !6739)
!6749 = !DILocation(line: 68, column: 14, scope: !6739)
!6750 = !DILocation(line: 68, column: 2, scope: !6739)
!6751 = !DILocation(line: 69, column: 6, scope: !6752)
!6752 = distinct !DILexicalBlock(scope: !6739, file: !5631, line: 69, column: 6)
!6753 = !DILocation(line: 69, column: 6, scope: !6739)
!6754 = !DILocation(line: 70, column: 35, scope: !6755)
!6755 = distinct !DILexicalBlock(scope: !6752, file: !5631, line: 69, column: 43)
!6756 = !DILocation(line: 70, column: 12, scope: !6755)
!6757 = !DILocation(line: 70, column: 3, scope: !6755)
!6758 = !DILocation(line: 71, column: 22, scope: !6755)
!6759 = !DILocation(line: 71, column: 9, scope: !6755)
!6760 = !DILocation(line: 71, column: 3, scope: !6755)
!6761 = !DILocation(line: 72, column: 2, scope: !6755)
!6762 = !DILocation(line: 73, column: 19, scope: !6763)
!6763 = distinct !DILexicalBlock(scope: !6752, file: !5631, line: 72, column: 9)
!6764 = !DILocation(line: 73, column: 32, scope: !6763)
!6765 = !DILocation(line: 73, column: 3, scope: !6763)
!6766 = !DILocation(line: 75, column: 1, scope: !6739)
!6767 = distinct !DISubprogram(name: "init_task_work", scope: !408, file: !408, line: 11, type: !6768, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6768 = !DISubroutineType(types: !6769)
!6769 = !{null, !132, !6770}
!6770 = !DIDerivedType(tag: DW_TAG_typedef, name: "task_work_func_t", file: !408, line: 8, baseType: !134)
!6771 = !DILocalVariable(name: "twork", arg: 1, scope: !6767, file: !408, line: 11, type: !132)
!6772 = !DILocation(line: 11, column: 38, scope: !6767)
!6773 = !DILocalVariable(name: "func", arg: 2, scope: !6767, file: !408, line: 11, type: !6770)
!6774 = !DILocation(line: 11, column: 62, scope: !6767)
!6775 = !DILocation(line: 13, column: 16, scope: !6767)
!6776 = !DILocation(line: 13, column: 2, scope: !6767)
!6777 = !DILocation(line: 13, column: 9, scope: !6767)
!6778 = !DILocation(line: 13, column: 14, scope: !6767)
!6779 = !DILocation(line: 14, column: 1, scope: !6767)
!6780 = distinct !DISubprogram(name: "____fput", scope: !5631, file: !5631, line: 457, type: !135, scopeLine: 458, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6781 = !DILocalVariable(name: "work", arg: 1, scope: !6780, file: !5631, line: 457, type: !132)
!6782 = !DILocation(line: 457, column: 44, scope: !6780)
!6783 = !DILocalVariable(name: "__mptr", scope: !6784, file: !5631, line: 459, type: !40)
!6784 = distinct !DILexicalBlock(scope: !6780, file: !5631, line: 459, column: 9)
!6785 = !DILocation(line: 459, column: 9, scope: !6784)
!6786 = !DILocation(line: 459, column: 2, scope: !6780)
!6787 = !DILocation(line: 460, column: 1, scope: !6780)
!6788 = distinct !DISubprogram(name: "llist_add", scope: !3652, file: !3652, line: 246, type: !6789, scopeLine: 247, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6789 = !DISubroutineType(types: !6790)
!6790 = !{!614, !3655, !6791}
!6791 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !4988, size: 64)
!6792 = !DILocalVariable(name: "new", arg: 1, scope: !6788, file: !3652, line: 246, type: !3655)
!6793 = !DILocation(line: 246, column: 49, scope: !6788)
!6794 = !DILocalVariable(name: "head", arg: 2, scope: !6788, file: !3652, line: 246, type: !6791)
!6795 = !DILocation(line: 246, column: 73, scope: !6788)
!6796 = !DILocation(line: 248, column: 25, scope: !6788)
!6797 = !DILocation(line: 248, column: 30, scope: !6788)
!6798 = !DILocation(line: 248, column: 35, scope: !6788)
!6799 = !DILocation(line: 248, column: 9, scope: !6788)
!6800 = !DILocation(line: 248, column: 2, scope: !6788)
!6801 = distinct !DISubprogram(name: "schedule_delayed_work", scope: !466, file: !466, line: 814, type: !6802, scopeLine: 816, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6802 = !DISubroutineType(types: !6803)
!6803 = !{!614, !6804, !59}
!6804 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2840, size: 64)
!6805 = !DILocalVariable(name: "dwork", arg: 1, scope: !6801, file: !466, line: 814, type: !6804)
!6806 = !DILocation(line: 814, column: 63, scope: !6801)
!6807 = !DILocalVariable(name: "delay", arg: 2, scope: !6801, file: !466, line: 815, type: !59)
!6808 = !DILocation(line: 815, column: 21, scope: !6801)
!6809 = !DILocation(line: 817, column: 28, scope: !6801)
!6810 = !DILocation(line: 817, column: 39, scope: !6801)
!6811 = !DILocation(line: 817, column: 46, scope: !6801)
!6812 = !DILocation(line: 817, column: 9, scope: !6801)
!6813 = !DILocation(line: 817, column: 2, scope: !6801)
!6814 = distinct !DISubprogram(name: "__fput_sync", scope: !5631, file: !5631, line: 513, type: !2363, scopeLine: 514, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6815 = !DILocalVariable(name: "file", arg: 1, scope: !6814, file: !5631, line: 513, type: !896)
!6816 = !DILocation(line: 513, column: 31, scope: !6814)
!6817 = !DILocation(line: 515, column: 32, scope: !6818)
!6818 = distinct !DILexicalBlock(scope: !6814, file: !5631, line: 515, column: 6)
!6819 = !DILocation(line: 515, column: 38, scope: !6818)
!6820 = !DILocation(line: 4536, column: 41, scope: !6657, inlinedAt: !6821)
!6821 = distinct !DILocation(line: 515, column: 6, scope: !6818)
!6822 = !DILocation(line: 4539, column: 31, scope: !6657, inlinedAt: !6821)
!6823 = !DILocation(line: 94, column: 79, scope: !6664, inlinedAt: !6824)
!6824 = distinct !DILocation(line: 4539, column: 2, scope: !6657, inlinedAt: !6821)
!6825 = !DILocation(line: 94, column: 89, scope: !6664, inlinedAt: !6824)
!6826 = !DILocation(line: 96, column: 20, scope: !6664, inlinedAt: !6824)
!6827 = !DILocation(line: 96, column: 23, scope: !6664, inlinedAt: !6824)
!6828 = !DILocation(line: 96, column: 2, scope: !6664, inlinedAt: !6824)
!6829 = !DILocation(line: 97, column: 2, scope: !6664, inlinedAt: !6824)
!6830 = !DILocation(line: 4540, column: 38, scope: !6657, inlinedAt: !6821)
!6831 = !DILocation(line: 1568, column: 45, scope: !6675, inlinedAt: !6832)
!6832 = distinct !DILocation(line: 4540, column: 9, scope: !6657, inlinedAt: !6821)
!6833 = !DILocation(line: 1571, column: 35, scope: !6675, inlinedAt: !6832)
!6834 = !DILocation(line: 4401, column: 39, scope: !6680, inlinedAt: !6835)
!6835 = distinct !DILocation(line: 1571, column: 9, scope: !6675, inlinedAt: !6832)
!6836 = !DILocation(line: 4404, column: 36, scope: !6680, inlinedAt: !6835)
!6837 = !DILocation(line: 59, column: 68, scope: !6687, inlinedAt: !6838)
!6838 = distinct !DILocation(line: 4404, column: 9, scope: !6680, inlinedAt: !6835)
!6839 = !DILocation(line: 61, column: 9, scope: !6691, inlinedAt: !6838)
!6840 = !DILocation(line: 515, column: 6, scope: !6814)
!6841 = !DILocation(line: 516, column: 10, scope: !6818)
!6842 = !DILocation(line: 516, column: 3, scope: !6818)
!6843 = !DILocation(line: 517, column: 1, scope: !6814)
!6844 = distinct !DISubprogram(name: "__fput", scope: !5631, file: !5631, line: 405, type: !2363, scopeLine: 406, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6845 = !DILocalVariable(name: "file", arg: 1, scope: !6844, file: !5631, line: 405, type: !896)
!6846 = !DILocation(line: 405, column: 33, scope: !6844)
!6847 = !DILocalVariable(name: "dentry", scope: !6844, file: !5631, line: 407, type: !740)
!6848 = !DILocation(line: 407, column: 17, scope: !6844)
!6849 = !DILocation(line: 407, column: 26, scope: !6844)
!6850 = !DILocation(line: 407, column: 32, scope: !6844)
!6851 = !DILocation(line: 407, column: 39, scope: !6844)
!6852 = !DILocalVariable(name: "mnt", scope: !6844, file: !5631, line: 408, type: !3166)
!6853 = !DILocation(line: 408, column: 19, scope: !6844)
!6854 = !DILocation(line: 408, column: 25, scope: !6844)
!6855 = !DILocation(line: 408, column: 31, scope: !6844)
!6856 = !DILocation(line: 408, column: 38, scope: !6844)
!6857 = !DILocalVariable(name: "inode", scope: !6844, file: !5631, line: 409, type: !779)
!6858 = !DILocation(line: 409, column: 16, scope: !6844)
!6859 = !DILocation(line: 409, column: 24, scope: !6844)
!6860 = !DILocation(line: 409, column: 30, scope: !6844)
!6861 = !DILocalVariable(name: "mode", scope: !6844, file: !5631, line: 410, type: !489)
!6862 = !DILocation(line: 410, column: 10, scope: !6844)
!6863 = !DILocation(line: 410, column: 17, scope: !6844)
!6864 = !DILocation(line: 410, column: 23, scope: !6844)
!6865 = !DILocation(line: 412, column: 6, scope: !6866)
!6866 = distinct !DILexicalBlock(scope: !6844, file: !5631, line: 412, column: 6)
!6867 = !DILocation(line: 412, column: 6, scope: !6844)
!6868 = !DILocation(line: 413, column: 3, scope: !6866)
!6869 = !DILocation(line: 415, column: 2, scope: !6844)
!6870 = !DILocation(line: 73, column: 2, scope: !6871, inlinedAt: !6873)
!6871 = distinct !DISubprogram(name: "might_resched", scope: !6872, file: !6872, line: 71, type: !2887, scopeLine: 72, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6872 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/kernel.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "b9e1eb4e98ef20c50921ec32f0d5b1d0")
!6873 = distinct !DILocation(line: 415, column: 2, scope: !6874)
!6874 = distinct !DILexicalBlock(scope: !6844, file: !5631, line: 415, column: 2)
!6875 = !DILocation(line: 415, column: 2, scope: !6874)
!6876 = !DILocation(line: 417, column: 17, scope: !6844)
!6877 = !DILocation(line: 417, column: 2, scope: !6844)
!6878 = !DILocation(line: 422, column: 20, scope: !6844)
!6879 = !DILocation(line: 422, column: 2, scope: !6844)
!6880 = !DILocation(line: 423, column: 20, scope: !6844)
!6881 = !DILocation(line: 423, column: 2, scope: !6844)
!6882 = !DILocation(line: 425, column: 24, scope: !6844)
!6883 = !DILocation(line: 425, column: 2, scope: !6844)
!6884 = !DILocation(line: 426, column: 6, scope: !6885)
!6885 = distinct !DILexicalBlock(scope: !6844, file: !5631, line: 426, column: 6)
!6886 = !DILocation(line: 426, column: 6, scope: !6844)
!6887 = !DILocation(line: 427, column: 7, scope: !6888)
!6888 = distinct !DILexicalBlock(scope: !6889, file: !5631, line: 427, column: 7)
!6889 = distinct !DILexicalBlock(scope: !6885, file: !5631, line: 426, column: 40)
!6890 = !DILocation(line: 427, column: 13, scope: !6888)
!6891 = !DILocation(line: 427, column: 19, scope: !6888)
!6892 = !DILocation(line: 427, column: 7, scope: !6889)
!6893 = !DILocation(line: 428, column: 4, scope: !6888)
!6894 = !DILocation(line: 428, column: 10, scope: !6888)
!6895 = !DILocation(line: 428, column: 16, scope: !6888)
!6896 = !DILocation(line: 428, column: 27, scope: !6888)
!6897 = !DILocation(line: 429, column: 2, scope: !6889)
!6898 = !DILocation(line: 430, column: 6, scope: !6899)
!6899 = distinct !DILexicalBlock(scope: !6844, file: !5631, line: 430, column: 6)
!6900 = !DILocation(line: 430, column: 12, scope: !6899)
!6901 = !DILocation(line: 430, column: 18, scope: !6899)
!6902 = !DILocation(line: 430, column: 6, scope: !6844)
!6903 = !DILocation(line: 431, column: 3, scope: !6899)
!6904 = !DILocation(line: 431, column: 9, scope: !6899)
!6905 = !DILocation(line: 431, column: 15, scope: !6899)
!6906 = !DILocation(line: 431, column: 23, scope: !6899)
!6907 = !DILocation(line: 431, column: 30, scope: !6899)
!6908 = !DILocation(line: 432, column: 6, scope: !6909)
!6909 = distinct !DILexicalBlock(scope: !6844, file: !5631, line: 432, column: 6)
!6910 = !DILocation(line: 0, scope: !6909)
!6911 = !DILocation(line: 432, column: 6, scope: !6844)
!6912 = !DILocation(line: 434, column: 12, scope: !6913)
!6913 = distinct !DILexicalBlock(scope: !6909, file: !5631, line: 433, column: 31)
!6914 = !DILocation(line: 434, column: 19, scope: !6913)
!6915 = !DILocation(line: 434, column: 3, scope: !6913)
!6916 = !DILocation(line: 435, column: 2, scope: !6913)
!6917 = !DILocalVariable(name: "_fops", scope: !6918, file: !5631, line: 436, type: !903)
!6918 = distinct !DILexicalBlock(scope: !6844, file: !5631, line: 436, column: 2)
!6919 = !DILocation(line: 436, column: 2, scope: !6918)
!6920 = !DILocation(line: 436, column: 2, scope: !6921)
!6921 = distinct !DILexicalBlock(scope: !6918, file: !5631, line: 436, column: 2)
!6922 = !DILocation(line: 437, column: 23, scope: !6844)
!6923 = !DILocation(line: 437, column: 2, scope: !6844)
!6924 = !DILocation(line: 438, column: 18, scope: !6844)
!6925 = !DILocation(line: 438, column: 2, scope: !6844)
!6926 = !DILocation(line: 439, column: 7, scope: !6844)
!6927 = !DILocation(line: 439, column: 2, scope: !6844)
!6928 = !DILocation(line: 440, column: 6, scope: !6929)
!6929 = distinct !DILexicalBlock(scope: !6844, file: !5631, line: 440, column: 6)
!6930 = !DILocation(line: 440, column: 6, scope: !6844)
!6931 = !DILocation(line: 441, column: 20, scope: !6929)
!6932 = !DILocation(line: 441, column: 3, scope: !6929)
!6933 = !DILocation(line: 442, column: 9, scope: !6844)
!6934 = !DILocation(line: 442, column: 2, scope: !6844)
!6935 = !DILabel(scope: !6844, name: "out", file: !5631, line: 443)
!6936 = !DILocation(line: 443, column: 1, scope: !6844)
!6937 = !DILocation(line: 444, column: 12, scope: !6844)
!6938 = !DILocation(line: 444, column: 2, scope: !6844)
!6939 = !DILocation(line: 445, column: 1, scope: !6844)
!6940 = !DILocalVariable(name: "args", scope: !5679, file: !5631, line: 524, type: !6941)
!6941 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "kmem_cache_args", file: !416, line: 255, size: 256, elements: !6942)
!6942 = !{!6943, !6944, !6945, !6946, !6947, !6948}
!6943 = !DIDerivedType(tag: DW_TAG_member, name: "align", scope: !6941, file: !416, line: 261, baseType: !7, size: 32)
!6944 = !DIDerivedType(tag: DW_TAG_member, name: "useroffset", scope: !6941, file: !416, line: 267, baseType: !7, size: 32, offset: 32)
!6945 = !DIDerivedType(tag: DW_TAG_member, name: "usersize", scope: !6941, file: !416, line: 273, baseType: !7, size: 32, offset: 64)
!6946 = !DIDerivedType(tag: DW_TAG_member, name: "freeptr_offset", scope: !6941, file: !416, line: 295, baseType: !7, size: 32, offset: 96)
!6947 = !DIDerivedType(tag: DW_TAG_member, name: "use_freeptr_offset", scope: !6941, file: !416, line: 299, baseType: !614, size: 8, offset: 128)
!6948 = !DIDerivedType(tag: DW_TAG_member, name: "ctor", scope: !6941, file: !416, line: 311, baseType: !809, size: 64, offset: 192)
!6949 = !DILocation(line: 524, column: 25, scope: !5679)
!6950 = !DILocation(line: 529, column: 16, scope: !5679)
!6951 = !DILocation(line: 529, column: 14, scope: !5679)
!6952 = !DILocation(line: 532, column: 2, scope: !6953)
!6953 = distinct !DILexicalBlock(scope: !5679, file: !5631, line: 532, column: 2)
!6954 = !DILocation(line: 533, column: 1, scope: !5679)
!6955 = distinct !DISubprogram(name: "files_maxfiles_init", scope: !5631, file: !5631, line: 539, type: !2887, scopeLine: 540, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6956 = !DILocalVariable(name: "n", scope: !6955, file: !5631, line: 541, type: !59)
!6957 = !DILocation(line: 541, column: 16, scope: !6955)
!6958 = !DILocalVariable(name: "nr_pages", scope: !6955, file: !5631, line: 542, type: !59)
!6959 = !DILocation(line: 542, column: 16, scope: !6955)
!6960 = !DILocation(line: 542, column: 27, scope: !6955)
!6961 = !DILocalVariable(name: "memreserve", scope: !6955, file: !5631, line: 543, type: !59)
!6962 = !DILocation(line: 543, column: 16, scope: !6955)
!6963 = !DILocation(line: 543, column: 30, scope: !6955)
!6964 = !DILocation(line: 543, column: 41, scope: !6955)
!6965 = !DILocation(line: 543, column: 39, scope: !6955)
!6966 = !DILocation(line: 543, column: 58, scope: !6955)
!6967 = !DILocation(line: 543, column: 61, scope: !6955)
!6968 = !DILocalVariable(name: "__UNIQUE_ID_x_514", scope: !6969, file: !5631, line: 545, type: !59)
!6969 = distinct !DILexicalBlock(scope: !6955, file: !5631, line: 545, column: 15)
!6970 = !DILocation(line: 545, column: 15, scope: !6969)
!6971 = !DILocalVariable(name: "__UNIQUE_ID_y_515", scope: !6969, file: !5631, line: 545, type: !59)
!6972 = !DILocation(line: 545, column: 15, scope: !6973)
!6973 = distinct !DILexicalBlock(scope: !6969, file: !5631, line: 545, column: 15)
!6974 = !DILocation(line: 545, column: 13, scope: !6955)
!6975 = !DILocation(line: 546, column: 8, scope: !6955)
!6976 = !DILocation(line: 546, column: 19, scope: !6955)
!6977 = !DILocation(line: 546, column: 17, scope: !6955)
!6978 = !DILocation(line: 546, column: 31, scope: !6955)
!6979 = !DILocation(line: 546, column: 53, scope: !6955)
!6980 = !DILocation(line: 546, column: 4, scope: !6955)
!6981 = !DILocalVariable(name: "__UNIQUE_ID_x_517", scope: !6982, file: !5631, line: 548, type: !59)
!6982 = distinct !DILexicalBlock(scope: !6955, file: !5631, line: 548, column: 25)
!6983 = !DILocation(line: 548, column: 25, scope: !6982)
!6984 = !DILocalVariable(name: "__UNIQUE_ID_y_518", scope: !6982, file: !5631, line: 548, type: !59)
!6985 = !DILocation(line: 548, column: 23, scope: !6955)
!6986 = !DILocation(line: 549, column: 1, scope: !6955)
!6987 = distinct !DISubprogram(name: "totalram_pages", scope: !1161, file: !1161, line: 59, type: !5761, scopeLine: 60, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6988 = !DILocalVariable(name: "v", arg: 1, scope: !6989, file: !5951, line: 3186, type: !6992)
!6989 = distinct !DISubprogram(name: "atomic_long_read", scope: !5951, file: !5951, line: 3186, type: !6990, scopeLine: 3187, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6990 = !DISubroutineType(types: !6991)
!6991 = !{!892, !6992}
!6992 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !6993, size: 64)
!6993 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !496)
!6994 = !DILocation(line: 3186, column: 39, scope: !6989, inlinedAt: !6995)
!6995 = distinct !DILocation(line: 61, column: 24, scope: !6987)
!6996 = !DILocation(line: 3188, column: 25, scope: !6989, inlinedAt: !6995)
!6997 = !DILocalVariable(name: "v", arg: 1, scope: !6998, file: !5961, line: 66, type: !5964)
!6998 = distinct !DISubprogram(name: "instrument_atomic_read", scope: !5961, file: !5961, line: 66, type: !5962, scopeLine: 67, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!6999 = !DILocation(line: 66, column: 73, scope: !6998, inlinedAt: !7000)
!7000 = distinct !DILocation(line: 3188, column: 2, scope: !6989, inlinedAt: !6995)
!7001 = !DILocalVariable(name: "size", arg: 2, scope: !6998, file: !5961, line: 66, type: !55)
!7002 = !DILocation(line: 66, column: 83, scope: !6998, inlinedAt: !7000)
!7003 = !DILocation(line: 68, column: 19, scope: !6998, inlinedAt: !7000)
!7004 = !DILocation(line: 68, column: 22, scope: !6998, inlinedAt: !7000)
!7005 = !DILocation(line: 68, column: 2, scope: !6998, inlinedAt: !7000)
!7006 = !DILocation(line: 69, column: 2, scope: !6998, inlinedAt: !7000)
!7007 = !DILocation(line: 3189, column: 30, scope: !6989, inlinedAt: !6995)
!7008 = !DILocalVariable(name: "v", arg: 1, scope: !7009, file: !497, line: 35, type: !6992)
!7009 = distinct !DISubprogram(name: "raw_atomic_long_read", scope: !497, file: !497, line: 35, type: !6990, scopeLine: 36, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7010 = !DILocation(line: 35, column: 43, scope: !7009, inlinedAt: !7011)
!7011 = distinct !DILocation(line: 3189, column: 9, scope: !6989, inlinedAt: !6995)
!7012 = !DILocation(line: 38, column: 27, scope: !7009, inlinedAt: !7011)
!7013 = !DILocalVariable(name: "v", arg: 1, scope: !7014, file: !5987, line: 2581, type: !7017)
!7014 = distinct !DISubprogram(name: "raw_atomic64_read", scope: !5987, file: !5987, line: 2581, type: !7015, scopeLine: 2582, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7015 = !DISubroutineType(types: !7016)
!7016 = !{!502, !7017}
!7017 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7018, size: 64)
!7018 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !498)
!7019 = !DILocation(line: 2581, column: 37, scope: !7014, inlinedAt: !7020)
!7020 = distinct !DILocation(line: 38, column: 9, scope: !7009, inlinedAt: !7011)
!7021 = !DILocation(line: 2583, column: 28, scope: !7014, inlinedAt: !7020)
!7022 = !DILocalVariable(name: "v", arg: 1, scope: !7023, file: !5999, line: 13, type: !7017)
!7023 = distinct !DISubprogram(name: "arch_atomic64_read", scope: !5999, file: !5999, line: 13, type: !7015, scopeLine: 14, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7024 = !DILocation(line: 13, column: 65, scope: !7023, inlinedAt: !7025)
!7025 = distinct !DILocation(line: 2583, column: 9, scope: !7014, inlinedAt: !7020)
!7026 = !DILocation(line: 15, column: 9, scope: !7023, inlinedAt: !7025)
!7027 = !DILocation(line: 61, column: 2, scope: !6987)
!7028 = distinct !DISubprogram(name: "global_zone_page_state", scope: !7029, file: !7029, line: 183, type: !7030, scopeLine: 184, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7029 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/vmstat.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "fae4458cfae675069a66b5273db31e84")
!7030 = !DISubroutineType(types: !7031)
!7031 = !{!59, !436}
!7032 = !DILocalVariable(name: "item", arg: 1, scope: !7028, file: !7029, line: 183, type: !436)
!7033 = !DILocation(line: 183, column: 72, scope: !7028)
!7034 = !DILocalVariable(name: "x", scope: !7028, file: !7029, line: 185, type: !892)
!7035 = !DILocation(line: 185, column: 7, scope: !7028)
!7036 = !DILocation(line: 185, column: 42, scope: !7028)
!7037 = !DILocation(line: 185, column: 29, scope: !7028)
!7038 = !DILocation(line: 3186, column: 39, scope: !6989, inlinedAt: !7039)
!7039 = distinct !DILocation(line: 185, column: 11, scope: !7028)
!7040 = !DILocation(line: 3188, column: 25, scope: !6989, inlinedAt: !7039)
!7041 = !DILocation(line: 66, column: 73, scope: !6998, inlinedAt: !7042)
!7042 = distinct !DILocation(line: 3188, column: 2, scope: !6989, inlinedAt: !7039)
!7043 = !DILocation(line: 66, column: 83, scope: !6998, inlinedAt: !7042)
!7044 = !DILocation(line: 68, column: 19, scope: !6998, inlinedAt: !7042)
!7045 = !DILocation(line: 68, column: 22, scope: !6998, inlinedAt: !7042)
!7046 = !DILocation(line: 68, column: 2, scope: !6998, inlinedAt: !7042)
!7047 = !DILocation(line: 69, column: 2, scope: !6998, inlinedAt: !7042)
!7048 = !DILocation(line: 3189, column: 30, scope: !6989, inlinedAt: !7039)
!7049 = !DILocation(line: 35, column: 43, scope: !7009, inlinedAt: !7050)
!7050 = distinct !DILocation(line: 3189, column: 9, scope: !6989, inlinedAt: !7039)
!7051 = !DILocation(line: 38, column: 27, scope: !7009, inlinedAt: !7050)
!7052 = !DILocation(line: 2581, column: 37, scope: !7014, inlinedAt: !7053)
!7053 = distinct !DILocation(line: 38, column: 9, scope: !7009, inlinedAt: !7050)
!7054 = !DILocation(line: 2583, column: 28, scope: !7014, inlinedAt: !7053)
!7055 = !DILocation(line: 13, column: 65, scope: !7023, inlinedAt: !7056)
!7056 = distinct !DILocation(line: 2583, column: 9, scope: !7014, inlinedAt: !7053)
!7057 = !DILocation(line: 15, column: 9, scope: !7023, inlinedAt: !7056)
!7058 = !DILocation(line: 187, column: 6, scope: !7059)
!7059 = distinct !DILexicalBlock(scope: !7028, file: !7029, line: 187, column: 6)
!7060 = !DILocation(line: 187, column: 8, scope: !7059)
!7061 = !DILocation(line: 187, column: 6, scope: !7028)
!7062 = !DILocation(line: 188, column: 5, scope: !7059)
!7063 = !DILocation(line: 188, column: 3, scope: !7059)
!7064 = !DILocation(line: 190, column: 9, scope: !7028)
!7065 = !DILocation(line: 190, column: 2, scope: !7028)
!7066 = distinct !DISubprogram(name: "kmemleak_not_leak", scope: !7067, file: !7067, line: 93, type: !7068, scopeLine: 94, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7067 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/kmemleak.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "3f4ed42ee6c6e5d06af9997953e90d37")
!7068 = !DISubroutineType(types: !7069)
!7069 = !{null, !1298}
!7070 = !DILocalVariable(name: "ptr", arg: 1, scope: !7066, file: !7067, line: 93, type: !1298)
!7071 = !DILocation(line: 93, column: 50, scope: !7066)
!7072 = !DILocation(line: 95, column: 1, scope: !7066)
!7073 = distinct !DISubprogram(name: "proc_nr_files", scope: !5631, file: !5631, line: 99, type: !50, scopeLine: 101, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7074 = !DILocalVariable(name: "table", arg: 1, scope: !7073, file: !5631, line: 99, type: !52)
!7075 = !DILocation(line: 99, column: 50, scope: !7073)
!7076 = !DILocalVariable(name: "write", arg: 2, scope: !7073, file: !5631, line: 99, type: !42)
!7077 = !DILocation(line: 99, column: 61, scope: !7073)
!7078 = !DILocalVariable(name: "buffer", arg: 3, scope: !7073, file: !5631, line: 99, type: !40)
!7079 = !DILocation(line: 99, column: 74, scope: !7073)
!7080 = !DILocalVariable(name: "lenp", arg: 4, scope: !7073, file: !5631, line: 100, type: !54)
!7081 = !DILocation(line: 100, column: 13, scope: !7073)
!7082 = !DILocalVariable(name: "ppos", arg: 5, scope: !7073, file: !5631, line: 100, type: !60)
!7083 = !DILocation(line: 100, column: 27, scope: !7073)
!7084 = !DILocation(line: 102, column: 24, scope: !7073)
!7085 = !DILocation(line: 102, column: 22, scope: !7073)
!7086 = !DILocation(line: 103, column: 32, scope: !7073)
!7087 = !DILocation(line: 103, column: 39, scope: !7073)
!7088 = !DILocation(line: 103, column: 46, scope: !7073)
!7089 = !DILocation(line: 103, column: 54, scope: !7073)
!7090 = !DILocation(line: 103, column: 60, scope: !7073)
!7091 = !DILocation(line: 103, column: 9, scope: !7073)
!7092 = !DILocation(line: 103, column: 2, scope: !7073)
!7093 = distinct !DISubprogram(name: "percpu_counter_read_positive", scope: !676, file: !676, line: 118, type: !5877, scopeLine: 119, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7094 = !DILocalVariable(name: "fbc", arg: 1, scope: !7093, file: !676, line: 118, type: !5879)
!7095 = !DILocation(line: 118, column: 71, scope: !7093)
!7096 = !DILocalVariable(name: "ret", scope: !7093, file: !676, line: 121, type: !502)
!7097 = !DILocation(line: 121, column: 6, scope: !7093)
!7098 = !DILocation(line: 121, column: 12, scope: !7099)
!7099 = distinct !DILexicalBlock(scope: !7093, file: !676, line: 121, column: 12)
!7100 = !DILocation(line: 121, column: 12, scope: !7101)
!7101 = distinct !DILexicalBlock(scope: !7099, file: !676, line: 121, column: 12)
!7102 = !DILocation(line: 123, column: 6, scope: !7103)
!7103 = distinct !DILexicalBlock(scope: !7093, file: !676, line: 123, column: 6)
!7104 = !DILocation(line: 123, column: 10, scope: !7103)
!7105 = !DILocation(line: 123, column: 6, scope: !7093)
!7106 = !DILocation(line: 124, column: 10, scope: !7103)
!7107 = !DILocation(line: 124, column: 3, scope: !7103)
!7108 = !DILocation(line: 125, column: 2, scope: !7093)
!7109 = !DILocation(line: 126, column: 1, scope: !7093)
!7110 = distinct !DISubprogram(name: "get_cred", scope: !493, file: !493, line: 233, type: !7111, scopeLine: 234, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7111 = !DISubroutineType(types: !7112)
!7112 = !{!490, !490}
!7113 = !DILocalVariable(name: "cred", arg: 1, scope: !7110, file: !493, line: 233, type: !490)
!7114 = !DILocation(line: 233, column: 62, scope: !7110)
!7115 = !DILocation(line: 235, column: 23, scope: !7110)
!7116 = !DILocation(line: 235, column: 9, scope: !7110)
!7117 = !DILocation(line: 235, column: 2, scope: !7110)
!7118 = distinct !DISubprogram(name: "put_cred", scope: !493, file: !493, line: 278, type: !7119, scopeLine: 279, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7119 = !DISubroutineType(types: !7120)
!7120 = !{null, !490}
!7121 = !DILocalVariable(name: "cred", arg: 1, scope: !7118, file: !493, line: 278, type: !490)
!7122 = !DILocation(line: 278, column: 48, scope: !7118)
!7123 = !DILocation(line: 280, column: 16, scope: !7118)
!7124 = !DILocation(line: 280, column: 2, scope: !7118)
!7125 = !DILocation(line: 281, column: 1, scope: !7118)
!7126 = distinct !DISubprogram(name: "get_cred_many", scope: !493, file: !493, line: 215, type: !7127, scopeLine: 216, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7127 = !DISubroutineType(types: !7128)
!7128 = !{!490, !490, !42}
!7129 = !DILocalVariable(name: "cred", arg: 1, scope: !7126, file: !493, line: 215, type: !490)
!7130 = !DILocation(line: 215, column: 67, scope: !7126)
!7131 = !DILocalVariable(name: "nr", arg: 2, scope: !7126, file: !493, line: 215, type: !42)
!7132 = !DILocation(line: 215, column: 77, scope: !7126)
!7133 = !DILocalVariable(name: "nonconst_cred", scope: !7126, file: !493, line: 217, type: !5638)
!7134 = !DILocation(line: 217, column: 15, scope: !7126)
!7135 = !DILocation(line: 217, column: 47, scope: !7126)
!7136 = !DILocation(line: 218, column: 7, scope: !7137)
!7137 = distinct !DILexicalBlock(scope: !7126, file: !493, line: 218, column: 6)
!7138 = !DILocation(line: 218, column: 6, scope: !7126)
!7139 = !DILocation(line: 219, column: 10, scope: !7137)
!7140 = !DILocation(line: 219, column: 3, scope: !7137)
!7141 = !DILocation(line: 220, column: 2, scope: !7126)
!7142 = !DILocation(line: 220, column: 17, scope: !7126)
!7143 = !DILocation(line: 220, column: 25, scope: !7126)
!7144 = !DILocation(line: 221, column: 27, scope: !7126)
!7145 = !DILocation(line: 221, column: 42, scope: !7126)
!7146 = !DILocation(line: 221, column: 9, scope: !7126)
!7147 = !DILocation(line: 221, column: 2, scope: !7126)
!7148 = !DILocation(line: 222, column: 1, scope: !7126)
!7149 = distinct !DISubprogram(name: "get_new_cred_many", scope: !493, file: !493, line: 183, type: !7150, scopeLine: 184, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7150 = !DISubroutineType(types: !7151)
!7151 = !{!5638, !5638, !42}
!7152 = !DILocalVariable(name: "cred", arg: 1, scope: !7149, file: !493, line: 183, type: !5638)
!7153 = !DILocation(line: 183, column: 59, scope: !7149)
!7154 = !DILocalVariable(name: "nr", arg: 2, scope: !7149, file: !493, line: 183, type: !42)
!7155 = !DILocation(line: 183, column: 69, scope: !7149)
!7156 = !DILocation(line: 185, column: 18, scope: !7149)
!7157 = !DILocation(line: 185, column: 23, scope: !7149)
!7158 = !DILocation(line: 185, column: 29, scope: !7149)
!7159 = !DILocalVariable(name: "i", arg: 1, scope: !7160, file: !5951, line: 3258, type: !892)
!7160 = distinct !DISubprogram(name: "atomic_long_add", scope: !5951, file: !5951, line: 3258, type: !7161, scopeLine: 3259, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7161 = !DISubroutineType(types: !7162)
!7162 = !{null, !892, !5330}
!7163 = !DILocation(line: 3258, column: 22, scope: !7160, inlinedAt: !7164)
!7164 = distinct !DILocation(line: 185, column: 2, scope: !7149)
!7165 = !DILocalVariable(name: "v", arg: 2, scope: !7160, file: !5951, line: 3258, type: !5330)
!7166 = !DILocation(line: 3258, column: 40, scope: !7160, inlinedAt: !7164)
!7167 = !DILocation(line: 3260, column: 31, scope: !7160, inlinedAt: !7164)
!7168 = !DILocation(line: 94, column: 79, scope: !6664, inlinedAt: !7169)
!7169 = distinct !DILocation(line: 3260, column: 2, scope: !7160, inlinedAt: !7164)
!7170 = !DILocation(line: 94, column: 89, scope: !6664, inlinedAt: !7169)
!7171 = !DILocation(line: 96, column: 20, scope: !6664, inlinedAt: !7169)
!7172 = !DILocation(line: 96, column: 23, scope: !6664, inlinedAt: !7169)
!7173 = !DILocation(line: 96, column: 2, scope: !6664, inlinedAt: !7169)
!7174 = !DILocation(line: 97, column: 2, scope: !6664, inlinedAt: !7169)
!7175 = !DILocation(line: 3261, column: 22, scope: !7160, inlinedAt: !7164)
!7176 = !DILocation(line: 3261, column: 25, scope: !7160, inlinedAt: !7164)
!7177 = !DILocalVariable(name: "i", arg: 1, scope: !7178, file: !497, line: 118, type: !892)
!7178 = distinct !DISubprogram(name: "raw_atomic_long_add", scope: !497, file: !497, line: 118, type: !7161, scopeLine: 119, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7179 = !DILocation(line: 118, column: 26, scope: !7178, inlinedAt: !7180)
!7180 = distinct !DILocation(line: 3261, column: 2, scope: !7160, inlinedAt: !7164)
!7181 = !DILocalVariable(name: "v", arg: 2, scope: !7178, file: !497, line: 118, type: !5330)
!7182 = !DILocation(line: 118, column: 44, scope: !7178, inlinedAt: !7180)
!7183 = !DILocation(line: 121, column: 19, scope: !7178, inlinedAt: !7180)
!7184 = !DILocation(line: 121, column: 22, scope: !7178, inlinedAt: !7180)
!7185 = !DILocalVariable(name: "i", arg: 1, scope: !7186, file: !5987, line: 2670, type: !502)
!7186 = distinct !DISubprogram(name: "raw_atomic64_add", scope: !5987, file: !5987, line: 2670, type: !7187, scopeLine: 2671, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7187 = !DISubroutineType(types: !7188)
!7188 = !{null, !502, !5990}
!7189 = !DILocation(line: 2670, column: 22, scope: !7186, inlinedAt: !7190)
!7190 = distinct !DILocation(line: 121, column: 2, scope: !7178, inlinedAt: !7180)
!7191 = !DILocalVariable(name: "v", arg: 2, scope: !7186, file: !5987, line: 2670, type: !5990)
!7192 = !DILocation(line: 2670, column: 37, scope: !7186, inlinedAt: !7190)
!7193 = !DILocation(line: 2672, column: 20, scope: !7186, inlinedAt: !7190)
!7194 = !DILocation(line: 2672, column: 23, scope: !7186, inlinedAt: !7190)
!7195 = !DILocalVariable(name: "i", arg: 1, scope: !7196, file: !5999, line: 23, type: !502)
!7196 = distinct !DISubprogram(name: "arch_atomic64_add", scope: !5999, file: !5999, line: 23, type: !7187, scopeLine: 24, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7197 = !DILocation(line: 23, column: 51, scope: !7196, inlinedAt: !7198)
!7198 = distinct !DILocation(line: 2672, column: 2, scope: !7186, inlinedAt: !7190)
!7199 = !DILocalVariable(name: "v", arg: 2, scope: !7196, file: !5999, line: 23, type: !5990)
!7200 = !DILocation(line: 23, column: 66, scope: !7196, inlinedAt: !7198)
!7201 = !DILocation(line: 26, column: 16, scope: !7196, inlinedAt: !7198)
!7202 = !DILocation(line: 27, column: 16, scope: !7196, inlinedAt: !7198)
!7203 = !DILocation(line: 27, column: 25, scope: !7196, inlinedAt: !7198)
!7204 = !DILocation(line: 25, column: 2, scope: !7196, inlinedAt: !7198)
!7205 = !{i64 2148863601, i64 2148863640, i64 2148863661, i64 2148863698, i64 2148863721, i64 2148863591}
!7206 = !DILocation(line: 186, column: 9, scope: !7149)
!7207 = !DILocation(line: 186, column: 2, scope: !7149)
!7208 = distinct !DISubprogram(name: "kasan_check_write", scope: !7209, file: !7209, line: 44, type: !7210, scopeLine: 45, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7209 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/kasan-checks.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "2238bcef786ad7fb484e6876008837c0")
!7210 = !DISubroutineType(types: !7211)
!7211 = !{!614, !5964, !7}
!7212 = !DILocalVariable(name: "p", arg: 1, scope: !7208, file: !7209, line: 44, type: !5964)
!7213 = !DILocation(line: 44, column: 59, scope: !7208)
!7214 = !DILocalVariable(name: "size", arg: 2, scope: !7208, file: !7209, line: 44, type: !7)
!7215 = !DILocation(line: 44, column: 75, scope: !7208)
!7216 = !DILocation(line: 46, column: 2, scope: !7208)
!7217 = distinct !DISubprogram(name: "kcsan_check_access", scope: !7218, file: !7218, line: 229, type: !7219, scopeLine: 230, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7218 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/kcsan-checks.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "2357b0a3f0b55b3e5518d3885308ba8f")
!7219 = !DISubroutineType(types: !7220)
!7220 = !{null, !5964, !55, !42}
!7221 = !DILocalVariable(name: "ptr", arg: 1, scope: !7217, file: !7218, line: 229, type: !5964)
!7222 = !DILocation(line: 229, column: 60, scope: !7217)
!7223 = !DILocalVariable(name: "size", arg: 2, scope: !7217, file: !7218, line: 229, type: !55)
!7224 = !DILocation(line: 229, column: 72, scope: !7217)
!7225 = !DILocalVariable(name: "type", arg: 3, scope: !7217, file: !7218, line: 230, type: !42)
!7226 = !DILocation(line: 230, column: 15, scope: !7217)
!7227 = !DILocation(line: 230, column: 23, scope: !7217)
!7228 = distinct !DISubprogram(name: "put_cred_many", scope: !493, file: !493, line: 261, type: !7229, scopeLine: 262, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7229 = !DISubroutineType(types: !7230)
!7230 = !{null, !490, !42}
!7231 = !DILocalVariable(name: "_cred", arg: 1, scope: !7228, file: !493, line: 261, type: !490)
!7232 = !DILocation(line: 261, column: 53, scope: !7228)
!7233 = !DILocalVariable(name: "nr", arg: 2, scope: !7228, file: !493, line: 261, type: !42)
!7234 = !DILocation(line: 261, column: 64, scope: !7228)
!7235 = !DILocalVariable(name: "cred", scope: !7228, file: !493, line: 263, type: !5638)
!7236 = !DILocation(line: 263, column: 15, scope: !7228)
!7237 = !DILocation(line: 263, column: 38, scope: !7228)
!7238 = !DILocation(line: 265, column: 6, scope: !7239)
!7239 = distinct !DILexicalBlock(scope: !7228, file: !493, line: 265, column: 6)
!7240 = !DILocation(line: 265, column: 6, scope: !7228)
!7241 = !DILocation(line: 266, column: 32, scope: !7242)
!7242 = distinct !DILexicalBlock(scope: !7243, file: !493, line: 266, column: 7)
!7243 = distinct !DILexicalBlock(scope: !7239, file: !493, line: 265, column: 12)
!7244 = !DILocation(line: 266, column: 37, scope: !7242)
!7245 = !DILocation(line: 266, column: 43, scope: !7242)
!7246 = !DILocalVariable(name: "i", arg: 1, scope: !7247, file: !5951, line: 4518, type: !892)
!7247 = distinct !DISubprogram(name: "atomic_long_sub_and_test", scope: !5951, file: !5951, line: 4518, type: !7248, scopeLine: 4519, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7248 = !DISubroutineType(types: !7249)
!7249 = !{!614, !892, !5330}
!7250 = !DILocation(line: 4518, column: 31, scope: !7247, inlinedAt: !7251)
!7251 = distinct !DILocation(line: 266, column: 7, scope: !7242)
!7252 = !DILocalVariable(name: "v", arg: 2, scope: !7247, file: !5951, line: 4518, type: !5330)
!7253 = !DILocation(line: 4518, column: 49, scope: !7247, inlinedAt: !7251)
!7254 = !DILocation(line: 4521, column: 31, scope: !7247, inlinedAt: !7251)
!7255 = !DILocation(line: 94, column: 79, scope: !6664, inlinedAt: !7256)
!7256 = distinct !DILocation(line: 4521, column: 2, scope: !7247, inlinedAt: !7251)
!7257 = !DILocation(line: 94, column: 89, scope: !6664, inlinedAt: !7256)
!7258 = !DILocation(line: 96, column: 20, scope: !6664, inlinedAt: !7256)
!7259 = !DILocation(line: 96, column: 23, scope: !6664, inlinedAt: !7256)
!7260 = !DILocation(line: 96, column: 2, scope: !6664, inlinedAt: !7256)
!7261 = !DILocation(line: 97, column: 2, scope: !6664, inlinedAt: !7256)
!7262 = !DILocation(line: 4522, column: 38, scope: !7247, inlinedAt: !7251)
!7263 = !DILocation(line: 4522, column: 41, scope: !7247, inlinedAt: !7251)
!7264 = !DILocalVariable(name: "i", arg: 1, scope: !7265, file: !497, line: 1548, type: !892)
!7265 = distinct !DISubprogram(name: "raw_atomic_long_sub_and_test", scope: !497, file: !497, line: 1548, type: !7248, scopeLine: 1549, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7266 = !DILocation(line: 1548, column: 35, scope: !7265, inlinedAt: !7267)
!7267 = distinct !DILocation(line: 4522, column: 9, scope: !7247, inlinedAt: !7251)
!7268 = !DILocalVariable(name: "v", arg: 2, scope: !7265, file: !497, line: 1548, type: !5330)
!7269 = !DILocation(line: 1548, column: 53, scope: !7265, inlinedAt: !7267)
!7270 = !DILocation(line: 1551, column: 35, scope: !7265, inlinedAt: !7267)
!7271 = !DILocation(line: 1551, column: 38, scope: !7265, inlinedAt: !7267)
!7272 = !DILocalVariable(name: "i", arg: 1, scope: !7273, file: !5987, line: 4381, type: !502)
!7273 = distinct !DISubprogram(name: "raw_atomic64_sub_and_test", scope: !5987, file: !5987, line: 4381, type: !7274, scopeLine: 4382, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7274 = !DISubroutineType(types: !7275)
!7275 = !{!614, !502, !5990}
!7276 = !DILocation(line: 4381, column: 31, scope: !7273, inlinedAt: !7277)
!7277 = distinct !DILocation(line: 1551, column: 9, scope: !7265, inlinedAt: !7267)
!7278 = !DILocalVariable(name: "v", arg: 2, scope: !7273, file: !5987, line: 4381, type: !5990)
!7279 = !DILocation(line: 4381, column: 46, scope: !7273, inlinedAt: !7277)
!7280 = !DILocation(line: 4384, column: 36, scope: !7273, inlinedAt: !7277)
!7281 = !DILocation(line: 4384, column: 39, scope: !7273, inlinedAt: !7277)
!7282 = !DILocalVariable(name: "i", arg: 1, scope: !7283, file: !5999, line: 37, type: !502)
!7283 = distinct !DISubprogram(name: "arch_atomic64_sub_and_test", scope: !5999, file: !5999, line: 37, type: !7274, scopeLine: 38, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7284 = !DILocation(line: 37, column: 60, scope: !7283, inlinedAt: !7285)
!7285 = distinct !DILocation(line: 4384, column: 9, scope: !7273, inlinedAt: !7277)
!7286 = !DILocalVariable(name: "v", arg: 2, scope: !7283, file: !5999, line: 37, type: !5990)
!7287 = !DILocation(line: 37, column: 75, scope: !7283, inlinedAt: !7285)
!7288 = !DILocalVariable(name: "c", scope: !7289, file: !5999, line: 39, type: !614)
!7289 = distinct !DILexicalBlock(scope: !7283, file: !5999, line: 39, column: 9)
!7290 = !DILocation(line: 39, column: 9, scope: !7289, inlinedAt: !7285)
!7291 = !{i64 2148865819, i64 2148865858, i64 2148865879, i64 2148865916, i64 2148865939, i64 2148865948, i64 2148866047}
!7292 = !DILocation(line: 266, column: 7, scope: !7243)
!7293 = !DILocation(line: 267, column: 15, scope: !7242)
!7294 = !DILocation(line: 267, column: 4, scope: !7242)
!7295 = !DILocation(line: 268, column: 2, scope: !7243)
!7296 = !DILocation(line: 269, column: 1, scope: !7228)
!7297 = distinct !DISubprogram(name: "percpu_counter_add", scope: !676, file: !676, line: 69, type: !7298, scopeLine: 70, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7298 = !DISubroutineType(types: !7299)
!7299 = !{null, !5879, !502}
!7300 = !DILocalVariable(name: "fbc", arg: 1, scope: !7297, file: !676, line: 69, type: !5879)
!7301 = !DILocation(line: 69, column: 62, scope: !7297)
!7302 = !DILocalVariable(name: "amount", arg: 2, scope: !7297, file: !676, line: 69, type: !502)
!7303 = !DILocation(line: 69, column: 71, scope: !7297)
!7304 = !DILocation(line: 71, column: 27, scope: !7297)
!7305 = !DILocation(line: 71, column: 32, scope: !7297)
!7306 = !DILocation(line: 71, column: 40, scope: !7297)
!7307 = !DILocation(line: 71, column: 2, scope: !7297)
!7308 = !DILocation(line: 72, column: 1, scope: !7297)
!7309 = distinct !DISubprogram(name: "filemap_sample_wb_err", scope: !1637, file: !1637, line: 115, type: !7310, scopeLine: 116, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7310 = !DISubroutineType(types: !7311)
!7311 = !{!2371, !1030}
!7312 = !DILocalVariable(name: "mapping", arg: 1, scope: !7309, file: !1637, line: 115, type: !1030)
!7313 = !DILocation(line: 115, column: 68, scope: !7309)
!7314 = !DILocation(line: 117, column: 24, scope: !7309)
!7315 = !DILocation(line: 117, column: 33, scope: !7309)
!7316 = !DILocation(line: 117, column: 9, scope: !7309)
!7317 = !DILocation(line: 117, column: 2, scope: !7309)
!7318 = distinct !DISubprogram(name: "file_sample_sb_err", scope: !1637, file: !1637, line: 127, type: !7319, scopeLine: 128, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7319 = !DISubroutineType(types: !7320)
!7320 = !{!2371, !896}
!7321 = !DILocalVariable(name: "file", arg: 1, scope: !7318, file: !1637, line: 127, type: !896)
!7322 = !DILocation(line: 127, column: 56, scope: !7318)
!7323 = !DILocation(line: 129, column: 24, scope: !7318)
!7324 = !DILocation(line: 129, column: 30, scope: !7318)
!7325 = !DILocation(line: 129, column: 37, scope: !7318)
!7326 = !DILocation(line: 129, column: 45, scope: !7318)
!7327 = !DILocation(line: 129, column: 51, scope: !7318)
!7328 = !DILocation(line: 129, column: 9, scope: !7318)
!7329 = !DILocation(line: 129, column: 2, scope: !7318)
!7330 = distinct !DISubprogram(name: "iocb_flags", scope: !342, file: !342, line: 3493, type: !7331, scopeLine: 3494, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7331 = !DISubroutineType(types: !7332)
!7332 = !{!42, !896}
!7333 = !DILocalVariable(name: "file", arg: 1, scope: !7330, file: !342, line: 3493, type: !896)
!7334 = !DILocation(line: 3493, column: 43, scope: !7330)
!7335 = !DILocalVariable(name: "res", scope: !7330, file: !342, line: 3495, type: !42)
!7336 = !DILocation(line: 3495, column: 6, scope: !7330)
!7337 = !DILocation(line: 3496, column: 6, scope: !7338)
!7338 = distinct !DILexicalBlock(scope: !7330, file: !342, line: 3496, column: 6)
!7339 = !DILocation(line: 3496, column: 12, scope: !7338)
!7340 = !DILocation(line: 3496, column: 20, scope: !7338)
!7341 = !DILocation(line: 3496, column: 6, scope: !7330)
!7342 = !DILocation(line: 3497, column: 7, scope: !7338)
!7343 = !DILocation(line: 3497, column: 3, scope: !7338)
!7344 = !DILocation(line: 3498, column: 6, scope: !7345)
!7345 = distinct !DILexicalBlock(scope: !7330, file: !342, line: 3498, column: 6)
!7346 = !DILocation(line: 3498, column: 12, scope: !7345)
!7347 = !DILocation(line: 3498, column: 20, scope: !7345)
!7348 = !DILocation(line: 3498, column: 6, scope: !7330)
!7349 = !DILocation(line: 3499, column: 7, scope: !7345)
!7350 = !DILocation(line: 3499, column: 3, scope: !7345)
!7351 = !DILocation(line: 3500, column: 6, scope: !7352)
!7352 = distinct !DILexicalBlock(scope: !7330, file: !342, line: 3500, column: 6)
!7353 = !DILocation(line: 3500, column: 12, scope: !7352)
!7354 = !DILocation(line: 3500, column: 20, scope: !7352)
!7355 = !DILocation(line: 3500, column: 6, scope: !7330)
!7356 = !DILocation(line: 3501, column: 7, scope: !7352)
!7357 = !DILocation(line: 3501, column: 3, scope: !7352)
!7358 = !DILocation(line: 3502, column: 6, scope: !7359)
!7359 = distinct !DILexicalBlock(scope: !7330, file: !342, line: 3502, column: 6)
!7360 = !DILocation(line: 3502, column: 12, scope: !7359)
!7361 = !DILocation(line: 3502, column: 20, scope: !7359)
!7362 = !DILocation(line: 3502, column: 6, scope: !7330)
!7363 = !DILocation(line: 3503, column: 7, scope: !7359)
!7364 = !DILocation(line: 3503, column: 3, scope: !7359)
!7365 = !DILocation(line: 3504, column: 9, scope: !7330)
!7366 = !DILocation(line: 3504, column: 2, scope: !7330)
!7367 = distinct !DISubprogram(name: "i_readcount_inc", scope: !342, file: !342, line: 3041, type: !3228, scopeLine: 3042, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7368 = !DILocalVariable(name: "inode", arg: 1, scope: !7367, file: !342, line: 3041, type: !779)
!7369 = !DILocation(line: 3041, column: 50, scope: !7367)
!7370 = !DILocation(line: 3043, column: 14, scope: !7367)
!7371 = !DILocation(line: 3043, column: 21, scope: !7367)
!7372 = !DILocalVariable(name: "v", arg: 1, scope: !7373, file: !5951, line: 433, type: !7376)
!7373 = distinct !DISubprogram(name: "atomic_inc", scope: !5951, file: !5951, line: 433, type: !7374, scopeLine: 434, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7374 = !DISubroutineType(types: !7375)
!7375 = !{null, !7376}
!7376 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !69, size: 64)
!7377 = !DILocation(line: 433, column: 22, scope: !7373, inlinedAt: !7378)
!7378 = distinct !DILocation(line: 3043, column: 2, scope: !7367)
!7379 = !DILocation(line: 435, column: 31, scope: !7373, inlinedAt: !7378)
!7380 = !DILocation(line: 94, column: 79, scope: !6664, inlinedAt: !7381)
!7381 = distinct !DILocation(line: 435, column: 2, scope: !7373, inlinedAt: !7378)
!7382 = !DILocation(line: 94, column: 89, scope: !6664, inlinedAt: !7381)
!7383 = !DILocation(line: 96, column: 20, scope: !6664, inlinedAt: !7381)
!7384 = !DILocation(line: 96, column: 23, scope: !6664, inlinedAt: !7381)
!7385 = !DILocation(line: 96, column: 2, scope: !6664, inlinedAt: !7381)
!7386 = !DILocation(line: 97, column: 2, scope: !6664, inlinedAt: !7381)
!7387 = !DILocation(line: 436, column: 17, scope: !7373, inlinedAt: !7378)
!7388 = !DILocalVariable(name: "v", arg: 1, scope: !7389, file: !5987, line: 989, type: !7376)
!7389 = distinct !DISubprogram(name: "raw_atomic_inc", scope: !5987, file: !5987, line: 989, type: !7374, scopeLine: 990, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7390 = !DILocation(line: 989, column: 26, scope: !7389, inlinedAt: !7391)
!7391 = distinct !DILocation(line: 436, column: 2, scope: !7373, inlinedAt: !7378)
!7392 = !DILocation(line: 992, column: 18, scope: !7389, inlinedAt: !7391)
!7393 = !DILocalVariable(name: "v", arg: 1, scope: !7394, file: !7395, line: 51, type: !7376)
!7394 = distinct !DISubprogram(name: "arch_atomic_inc", scope: !7395, file: !7395, line: 51, type: !7374, scopeLine: 52, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7395 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/arch/x86/include/asm/atomic.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "511a2f7890c893e5268b1c706b490e03")
!7396 = !DILocation(line: 51, column: 55, scope: !7394, inlinedAt: !7397)
!7397 = distinct !DILocation(line: 992, column: 2, scope: !7389, inlinedAt: !7391)
!7398 = !DILocation(line: 54, column: 16, scope: !7394, inlinedAt: !7397)
!7399 = !DILocation(line: 53, column: 2, scope: !7394, inlinedAt: !7397)
!7400 = !{i64 2148837978, i64 2148838017, i64 2148838038, i64 2148838075, i64 2148838098, i64 2148837968}
!7401 = !DILocation(line: 3044, column: 1, scope: !7367)
!7402 = distinct !DISubprogram(name: "llist_del_all", scope: !3652, file: !3652, line: 264, type: !7403, scopeLine: 265, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7403 = !DISubroutineType(types: !7404)
!7404 = !{!3655, !6791}
!7405 = !DILocalVariable(name: "head", arg: 1, scope: !7402, file: !3652, line: 264, type: !6791)
!7406 = !DILocation(line: 264, column: 67, scope: !7402)
!7407 = !DILocalVariable(name: "__ai_ptr", scope: !7408, file: !3652, line: 266, type: !7409)
!7408 = distinct !DILexicalBlock(scope: !7402, file: !3652, line: 266, column: 9)
!7409 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !3655, size: 64)
!7410 = !DILocation(line: 266, column: 9, scope: !7408)
!7411 = !DILocation(line: 266, column: 9, scope: !7412)
!7412 = distinct !DILexicalBlock(scope: !7408, file: !3652, line: 266, column: 9)
!7413 = !DILocation(line: 94, column: 79, scope: !6664, inlinedAt: !7414)
!7414 = distinct !DILocation(line: 266, column: 9, scope: !7408)
!7415 = !DILocation(line: 94, column: 89, scope: !6664, inlinedAt: !7414)
!7416 = !DILocation(line: 96, column: 20, scope: !6664, inlinedAt: !7414)
!7417 = !DILocation(line: 96, column: 23, scope: !6664, inlinedAt: !7414)
!7418 = !DILocation(line: 96, column: 2, scope: !6664, inlinedAt: !7414)
!7419 = !DILocation(line: 97, column: 2, scope: !6664, inlinedAt: !7414)
!7420 = !DILocalVariable(name: "__ret", scope: !7421, file: !3652, line: 266, type: !3655)
!7421 = distinct !DILexicalBlock(scope: !7408, file: !3652, line: 266, column: 9)
!7422 = !DILocation(line: 266, column: 9, scope: !7421)
!7423 = !{i64 2149660981}
!7424 = !DILocation(line: 266, column: 2, scope: !7402)
!7425 = distinct !DISubprogram(name: "percpu_counter_dec", scope: !676, file: !676, line: 270, type: !6009, scopeLine: 271, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7426 = !DILocalVariable(name: "fbc", arg: 1, scope: !7425, file: !676, line: 270, type: !5879)
!7427 = !DILocation(line: 270, column: 62, scope: !7425)
!7428 = !DILocation(line: 272, column: 21, scope: !7425)
!7429 = !DILocation(line: 272, column: 2, scope: !7425)
!7430 = !DILocation(line: 273, column: 1, scope: !7425)
!7431 = distinct !DISubprogram(name: "queue_delayed_work", scope: !466, file: !466, line: 673, type: !7432, scopeLine: 676, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7432 = !DISubroutineType(types: !7433)
!7433 = !{!614, !2845, !6804, !59}
!7434 = !DILocalVariable(name: "wq", arg: 1, scope: !7431, file: !466, line: 673, type: !2845)
!7435 = !DILocation(line: 673, column: 64, scope: !7431)
!7436 = !DILocalVariable(name: "dwork", arg: 2, scope: !7431, file: !466, line: 674, type: !6804)
!7437 = !DILocation(line: 674, column: 32, scope: !7431)
!7438 = !DILocalVariable(name: "delay", arg: 3, scope: !7431, file: !466, line: 675, type: !59)
!7439 = !DILocation(line: 675, column: 25, scope: !7431)
!7440 = !DILocation(line: 677, column: 49, scope: !7431)
!7441 = !DILocation(line: 677, column: 53, scope: !7431)
!7442 = !DILocation(line: 677, column: 60, scope: !7431)
!7443 = !DILocation(line: 677, column: 9, scope: !7431)
!7444 = !DILocation(line: 677, column: 2, scope: !7431)
!7445 = distinct !DISubprogram(name: "fsnotify_close", scope: !7446, file: !7446, line: 407, type: !2363, scopeLine: 408, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7446 = !DIFile(filename: "tmp_flow_worktrees/SYZBOT-3b6b32dc50537a49/include/linux/fsnotify.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "b3d84653b58edf05274ba59feae11c67")
!7447 = !DILocalVariable(name: "file", arg: 1, scope: !7445, file: !7446, line: 407, type: !896)
!7448 = !DILocation(line: 407, column: 48, scope: !7445)
!7449 = !DILocalVariable(name: "mask", scope: !7445, file: !7446, line: 409, type: !579)
!7450 = !DILocation(line: 409, column: 8, scope: !7445)
!7451 = !DILocation(line: 409, column: 16, scope: !7445)
!7452 = !DILocation(line: 409, column: 22, scope: !7445)
!7453 = !DILocation(line: 409, column: 29, scope: !7445)
!7454 = !DILocation(line: 409, column: 15, scope: !7445)
!7455 = !DILocation(line: 412, column: 16, scope: !7445)
!7456 = !DILocation(line: 412, column: 22, scope: !7445)
!7457 = !DILocation(line: 412, column: 2, scope: !7445)
!7458 = !DILocation(line: 413, column: 1, scope: !7445)
!7459 = distinct !DISubprogram(name: "eventpoll_release", scope: !7460, file: !7460, line: 34, type: !2363, scopeLine: 35, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7460 = !DIFile(filename: "LLM4Con/kernel_experiment/SYZBOT-3b6b32dc50537a49/src/include/linux/eventpoll.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "835d7975987acdfb9bf5df3255f31a38")
!7461 = !DILocalVariable(name: "file", arg: 1, scope: !7459, file: !7460, line: 34, type: !896)
!7462 = !DILocation(line: 34, column: 51, scope: !7459)
!7463 = !DILocation(line: 45, column: 6, scope: !7464)
!7464 = distinct !DILexicalBlock(scope: !7459, file: !7460, line: 45, column: 6)
!7465 = !DILocation(line: 45, column: 6, scope: !7459)
!7466 = !DILocation(line: 46, column: 3, scope: !7464)
!7467 = !DILocation(line: 53, column: 25, scope: !7459)
!7468 = !DILocation(line: 53, column: 2, scope: !7459)
!7469 = !DILocation(line: 54, column: 1, scope: !7459)
!7470 = distinct !DISubprogram(name: "put_file_access", scope: !7471, file: !7471, line: 112, type: !2363, scopeLine: 113, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7471 = !DIFile(filename: "LLM4Con/kernel_experiment/SYZBOT-3b6b32dc50537a49/src/fs/internal.h", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "1a530ca9367fbe2a08f54ce3f55f2623")
!7472 = !DILocalVariable(name: "file", arg: 1, scope: !7470, file: !7471, line: 112, type: !896)
!7473 = !DILocation(line: 112, column: 49, scope: !7470)
!7474 = !DILocation(line: 114, column: 7, scope: !7475)
!7475 = distinct !DILexicalBlock(scope: !7470, file: !7471, line: 114, column: 6)
!7476 = !DILocation(line: 114, column: 13, scope: !7475)
!7477 = !DILocation(line: 114, column: 20, scope: !7475)
!7478 = !DILocation(line: 114, column: 50, scope: !7475)
!7479 = !DILocation(line: 114, column: 6, scope: !7470)
!7480 = !DILocation(line: 115, column: 19, scope: !7481)
!7481 = distinct !DILexicalBlock(scope: !7475, file: !7471, line: 114, column: 65)
!7482 = !DILocation(line: 115, column: 25, scope: !7481)
!7483 = !DILocation(line: 115, column: 3, scope: !7481)
!7484 = !DILocation(line: 116, column: 2, scope: !7481)
!7485 = !DILocation(line: 116, column: 13, scope: !7486)
!7486 = distinct !DILexicalBlock(scope: !7475, file: !7471, line: 116, column: 13)
!7487 = !DILocation(line: 116, column: 19, scope: !7486)
!7488 = !DILocation(line: 116, column: 26, scope: !7486)
!7489 = !DILocation(line: 116, column: 13, scope: !7475)
!7490 = !DILocation(line: 117, column: 25, scope: !7491)
!7491 = distinct !DILexicalBlock(scope: !7486, file: !7471, line: 116, column: 42)
!7492 = !DILocation(line: 117, column: 3, scope: !7491)
!7493 = !DILocation(line: 118, column: 2, scope: !7491)
!7494 = !DILocation(line: 119, column: 1, scope: !7470)
!7495 = distinct !DISubprogram(name: "fsnotify_file", scope: !7446, file: !7446, line: 111, type: !7496, scopeLine: 112, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7496 = !DISubroutineType(types: !7497)
!7497 = !{!42, !896, !579}
!7498 = !DILocalVariable(name: "file", arg: 1, scope: !7495, file: !7446, line: 111, type: !896)
!7499 = !DILocation(line: 111, column: 46, scope: !7495)
!7500 = !DILocalVariable(name: "mask", arg: 2, scope: !7495, file: !7446, line: 111, type: !579)
!7501 = !DILocation(line: 111, column: 58, scope: !7495)
!7502 = !DILocalVariable(name: "path", scope: !7495, file: !7446, line: 113, type: !3398)
!7503 = !DILocation(line: 113, column: 21, scope: !7495)
!7504 = !DILocation(line: 121, column: 6, scope: !7505)
!7505 = distinct !DILexicalBlock(scope: !7495, file: !7446, line: 121, column: 6)
!7506 = !DILocation(line: 121, column: 12, scope: !7505)
!7507 = !DILocation(line: 121, column: 19, scope: !7505)
!7508 = !DILocation(line: 121, column: 6, scope: !7495)
!7509 = !DILocation(line: 122, column: 3, scope: !7505)
!7510 = !DILocation(line: 124, column: 10, scope: !7495)
!7511 = !DILocation(line: 124, column: 16, scope: !7495)
!7512 = !DILocation(line: 124, column: 7, scope: !7495)
!7513 = !DILocation(line: 126, column: 6, scope: !7514)
!7514 = distinct !DILexicalBlock(scope: !7495, file: !7446, line: 126, column: 6)
!7515 = !DILocation(line: 126, column: 11, scope: !7514)
!7516 = !DILocation(line: 126, column: 38, scope: !7514)
!7517 = !DILocation(line: 127, column: 41, scope: !7514)
!7518 = !DILocation(line: 127, column: 47, scope: !7514)
!7519 = !DILocation(line: 127, column: 55, scope: !7514)
!7520 = !DILocation(line: 127, column: 7, scope: !7514)
!7521 = !DILocation(line: 126, column: 6, scope: !7495)
!7522 = !DILocation(line: 129, column: 3, scope: !7514)
!7523 = !DILocation(line: 131, column: 25, scope: !7495)
!7524 = !DILocation(line: 131, column: 31, scope: !7495)
!7525 = !DILocation(line: 131, column: 39, scope: !7495)
!7526 = !DILocation(line: 131, column: 45, scope: !7495)
!7527 = !DILocation(line: 131, column: 9, scope: !7495)
!7528 = !DILocation(line: 131, column: 2, scope: !7495)
!7529 = !DILocation(line: 132, column: 1, scope: !7495)
!7530 = distinct !DISubprogram(name: "fsnotify_sb_has_priority_watchers", scope: !7446, file: !7446, line: 21, type: !7531, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7531 = !DISubroutineType(types: !7532)
!7532 = !{!614, !3172, !42}
!7533 = !DILocalVariable(name: "sb", arg: 1, scope: !7530, file: !7446, line: 21, type: !3172)
!7534 = !DILocation(line: 21, column: 74, scope: !7530)
!7535 = !DILocalVariable(name: "prio", arg: 2, scope: !7530, file: !7446, line: 22, type: !42)
!7536 = !DILocation(line: 22, column: 16, scope: !7530)
!7537 = !DILocalVariable(name: "sbinfo", scope: !7530, file: !7446, line: 24, type: !5220)
!7538 = !DILocation(line: 24, column: 27, scope: !7530)
!7539 = !DILocation(line: 24, column: 53, scope: !7530)
!7540 = !DILocation(line: 24, column: 36, scope: !7530)
!7541 = !DILocation(line: 27, column: 7, scope: !7542)
!7542 = distinct !DILexicalBlock(scope: !7530, file: !7446, line: 27, column: 6)
!7543 = !DILocation(line: 27, column: 6, scope: !7530)
!7544 = !DILocation(line: 28, column: 3, scope: !7542)
!7545 = !DILocation(line: 30, column: 27, scope: !7530)
!7546 = !DILocation(line: 30, column: 35, scope: !7530)
!7547 = !DILocation(line: 30, column: 51, scope: !7530)
!7548 = !DILocation(line: 3186, column: 39, scope: !6989, inlinedAt: !7549)
!7549 = distinct !DILocation(line: 30, column: 9, scope: !7530)
!7550 = !DILocation(line: 3188, column: 25, scope: !6989, inlinedAt: !7549)
!7551 = !DILocation(line: 66, column: 73, scope: !6998, inlinedAt: !7552)
!7552 = distinct !DILocation(line: 3188, column: 2, scope: !6989, inlinedAt: !7549)
!7553 = !DILocation(line: 66, column: 83, scope: !6998, inlinedAt: !7552)
!7554 = !DILocation(line: 68, column: 19, scope: !6998, inlinedAt: !7552)
!7555 = !DILocation(line: 68, column: 22, scope: !6998, inlinedAt: !7552)
!7556 = !DILocation(line: 68, column: 2, scope: !6998, inlinedAt: !7552)
!7557 = !DILocation(line: 69, column: 2, scope: !6998, inlinedAt: !7552)
!7558 = !DILocation(line: 3189, column: 30, scope: !6989, inlinedAt: !7549)
!7559 = !DILocation(line: 35, column: 43, scope: !7009, inlinedAt: !7560)
!7560 = distinct !DILocation(line: 3189, column: 9, scope: !6989, inlinedAt: !7549)
!7561 = !DILocation(line: 38, column: 27, scope: !7009, inlinedAt: !7560)
!7562 = !DILocation(line: 2581, column: 37, scope: !7014, inlinedAt: !7563)
!7563 = distinct !DILocation(line: 38, column: 9, scope: !7009, inlinedAt: !7560)
!7564 = !DILocation(line: 2583, column: 28, scope: !7014, inlinedAt: !7563)
!7565 = !DILocation(line: 13, column: 65, scope: !7023, inlinedAt: !7566)
!7566 = distinct !DILocation(line: 2583, column: 9, scope: !7014, inlinedAt: !7563)
!7567 = !DILocation(line: 15, column: 9, scope: !7023, inlinedAt: !7566)
!7568 = !DILocation(line: 30, column: 9, scope: !7530)
!7569 = !DILocation(line: 30, column: 2, scope: !7530)
!7570 = !DILocation(line: 31, column: 1, scope: !7530)
!7571 = distinct !DISubprogram(name: "fsnotify_parent", scope: !7446, file: !7446, line: 76, type: !7572, scopeLine: 78, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7572 = !DISubroutineType(types: !7573)
!7573 = !{!42, !740, !579, !1298, !42}
!7574 = !DILocalVariable(name: "dentry", arg: 1, scope: !7571, file: !7446, line: 76, type: !740)
!7575 = !DILocation(line: 76, column: 50, scope: !7571)
!7576 = !DILocalVariable(name: "mask", arg: 2, scope: !7571, file: !7446, line: 76, type: !579)
!7577 = !DILocation(line: 76, column: 64, scope: !7571)
!7578 = !DILocalVariable(name: "data", arg: 3, scope: !7571, file: !7446, line: 77, type: !1298)
!7579 = !DILocation(line: 77, column: 19, scope: !7571)
!7580 = !DILocalVariable(name: "data_type", arg: 4, scope: !7571, file: !7446, line: 77, type: !42)
!7581 = !DILocation(line: 77, column: 29, scope: !7571)
!7582 = !DILocalVariable(name: "inode", scope: !7571, file: !7446, line: 79, type: !779)
!7583 = !DILocation(line: 79, column: 16, scope: !7571)
!7584 = !DILocation(line: 79, column: 32, scope: !7571)
!7585 = !DILocation(line: 79, column: 24, scope: !7571)
!7586 = !DILocation(line: 81, column: 32, scope: !7587)
!7587 = distinct !DILexicalBlock(scope: !7571, file: !7446, line: 81, column: 6)
!7588 = !DILocation(line: 81, column: 39, scope: !7587)
!7589 = !DILocation(line: 81, column: 7, scope: !7587)
!7590 = !DILocation(line: 81, column: 6, scope: !7571)
!7591 = !DILocation(line: 82, column: 3, scope: !7587)
!7592 = !DILocation(line: 84, column: 6, scope: !7593)
!7593 = distinct !DILexicalBlock(scope: !7571, file: !7446, line: 84, column: 6)
!7594 = !DILocation(line: 84, column: 6, scope: !7571)
!7595 = !DILocation(line: 85, column: 8, scope: !7596)
!7596 = distinct !DILexicalBlock(scope: !7593, file: !7446, line: 84, column: 30)
!7597 = !DILocation(line: 88, column: 9, scope: !7598)
!7598 = distinct !DILexicalBlock(scope: !7596, file: !7446, line: 88, column: 7)
!7599 = !DILocation(line: 88, column: 17, scope: !7598)
!7600 = !DILocation(line: 88, column: 25, scope: !7598)
!7601 = !DILocation(line: 88, column: 7, scope: !7596)
!7602 = !DILocation(line: 89, column: 4, scope: !7598)
!7603 = !DILocation(line: 90, column: 2, scope: !7596)
!7604 = !DILocation(line: 93, column: 6, scope: !7605)
!7605 = distinct !DILexicalBlock(scope: !7571, file: !7446, line: 93, column: 6)
!7606 = !DILocation(line: 93, column: 6, scope: !7571)
!7607 = !DILocation(line: 94, column: 3, scope: !7605)
!7608 = !DILocation(line: 96, column: 27, scope: !7571)
!7609 = !DILocation(line: 96, column: 35, scope: !7571)
!7610 = !DILocation(line: 96, column: 41, scope: !7571)
!7611 = !DILocation(line: 96, column: 47, scope: !7571)
!7612 = !DILocation(line: 96, column: 9, scope: !7571)
!7613 = !DILocation(line: 96, column: 2, scope: !7571)
!7614 = !DILabel(scope: !7571, name: "notify_child", file: !7446, line: 98)
!7615 = !DILocation(line: 98, column: 1, scope: !7571)
!7616 = !DILocation(line: 99, column: 18, scope: !7571)
!7617 = !DILocation(line: 99, column: 24, scope: !7571)
!7618 = !DILocation(line: 99, column: 30, scope: !7571)
!7619 = !DILocation(line: 99, column: 53, scope: !7571)
!7620 = !DILocation(line: 99, column: 9, scope: !7571)
!7621 = !DILocation(line: 99, column: 2, scope: !7571)
!7622 = !DILocation(line: 100, column: 1, scope: !7571)
!7623 = distinct !DISubprogram(name: "fsnotify_sb_info", scope: !474, file: !474, line: 502, type: !7624, scopeLine: 503, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7624 = !DISubroutineType(types: !7625)
!7625 = !{!5220, !3172}
!7626 = !DILocalVariable(name: "sb", arg: 1, scope: !7623, file: !474, line: 502, type: !3172)
!7627 = !DILocation(line: 502, column: 77, scope: !7623)
!7628 = !DILocation(line: 505, column: 9, scope: !7629)
!7629 = distinct !DILexicalBlock(scope: !7623, file: !474, line: 505, column: 9)
!7630 = !DILocation(line: 505, column: 9, scope: !7631)
!7631 = distinct !DILexicalBlock(scope: !7629, file: !474, line: 505, column: 9)
!7632 = !DILocation(line: 505, column: 2, scope: !7623)
!7633 = distinct !DISubprogram(name: "kasan_check_read", scope: !7209, file: !7209, line: 40, type: !7210, scopeLine: 41, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7634 = !DILocalVariable(name: "p", arg: 1, scope: !7633, file: !7209, line: 40, type: !5964)
!7635 = !DILocation(line: 40, column: 58, scope: !7633)
!7636 = !DILocalVariable(name: "size", arg: 2, scope: !7633, file: !7209, line: 40, type: !7)
!7637 = !DILocation(line: 40, column: 74, scope: !7633)
!7638 = !DILocation(line: 42, column: 2, scope: !7633)
!7639 = distinct !DISubprogram(name: "d_inode", scope: !366, file: !366, line: 525, type: !7640, scopeLine: 526, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7640 = !DISubroutineType(types: !7641)
!7641 = !{!779, !5269}
!7642 = !DILocalVariable(name: "dentry", arg: 1, scope: !7639, file: !366, line: 525, type: !5269)
!7643 = !DILocation(line: 525, column: 58, scope: !7639)
!7644 = !DILocation(line: 527, column: 9, scope: !7639)
!7645 = !DILocation(line: 527, column: 17, scope: !7639)
!7646 = !DILocation(line: 527, column: 2, scope: !7639)
!7647 = distinct !DISubprogram(name: "fsnotify_sb_has_watchers", scope: !7446, file: !7446, line: 34, type: !7648, scopeLine: 35, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7648 = !DISubroutineType(types: !7649)
!7649 = !{!614, !3172}
!7650 = !DILocalVariable(name: "sb", arg: 1, scope: !7647, file: !7446, line: 34, type: !3172)
!7651 = !DILocation(line: 34, column: 65, scope: !7647)
!7652 = !DILocation(line: 36, column: 43, scope: !7647)
!7653 = !DILocation(line: 36, column: 9, scope: !7647)
!7654 = !DILocation(line: 36, column: 2, scope: !7647)
!7655 = distinct !DISubprogram(name: "i_readcount_dec", scope: !342, file: !342, line: 3037, type: !3228, scopeLine: 3038, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7656 = !DILocalVariable(name: "inode", arg: 1, scope: !7655, file: !342, line: 3037, type: !779)
!7657 = !DILocation(line: 3037, column: 50, scope: !7655)
!7658 = !DILocation(line: 3039, column: 2, scope: !7655)
!7659 = !DILocation(line: 3039, column: 2, scope: !7660)
!7660 = distinct !DILexicalBlock(scope: !7661, file: !342, line: 3039, column: 2)
!7661 = distinct !DILexicalBlock(scope: !7655, file: !342, line: 3039, column: 2)
!7662 = !DILocalVariable(name: "v", arg: 1, scope: !7663, file: !5951, line: 607, type: !7376)
!7663 = distinct !DISubprogram(name: "atomic_dec_return", scope: !5951, file: !5951, line: 607, type: !7664, scopeLine: 608, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7664 = !DISubroutineType(types: !7665)
!7665 = !{!42, !7376}
!7666 = !DILocation(line: 607, column: 29, scope: !7663, inlinedAt: !7667)
!7667 = distinct !DILocation(line: 3039, column: 2, scope: !7660)
!7668 = !DILocation(line: 610, column: 31, scope: !7663, inlinedAt: !7667)
!7669 = !DILocation(line: 94, column: 79, scope: !6664, inlinedAt: !7670)
!7670 = distinct !DILocation(line: 610, column: 2, scope: !7663, inlinedAt: !7667)
!7671 = !DILocation(line: 94, column: 89, scope: !6664, inlinedAt: !7670)
!7672 = !DILocation(line: 96, column: 20, scope: !6664, inlinedAt: !7670)
!7673 = !DILocation(line: 96, column: 23, scope: !6664, inlinedAt: !7670)
!7674 = !DILocation(line: 96, column: 2, scope: !6664, inlinedAt: !7670)
!7675 = !DILocation(line: 97, column: 2, scope: !6664, inlinedAt: !7670)
!7676 = !DILocation(line: 611, column: 31, scope: !7663, inlinedAt: !7667)
!7677 = !DILocalVariable(name: "v", arg: 1, scope: !7678, file: !5987, line: 1227, type: !7376)
!7678 = distinct !DISubprogram(name: "raw_atomic_dec_return", scope: !5987, file: !5987, line: 1227, type: !7664, scopeLine: 1228, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7679 = !DILocation(line: 1227, column: 33, scope: !7678, inlinedAt: !7680)
!7680 = distinct !DILocation(line: 611, column: 9, scope: !7663, inlinedAt: !7667)
!7681 = !DILocation(line: 1238, column: 34, scope: !7678, inlinedAt: !7680)
!7682 = !DILocalVariable(name: "i", arg: 1, scope: !7683, file: !5987, line: 784, type: !42)
!7683 = distinct !DISubprogram(name: "raw_atomic_sub_return", scope: !5987, file: !5987, line: 784, type: !7684, scopeLine: 785, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7684 = !DISubroutineType(types: !7685)
!7685 = !{!42, !42, !7376}
!7686 = !DILocation(line: 784, column: 27, scope: !7683, inlinedAt: !7687)
!7687 = distinct !DILocation(line: 1238, column: 9, scope: !7678, inlinedAt: !7680)
!7688 = !DILocalVariable(name: "v", arg: 2, scope: !7683, file: !5987, line: 784, type: !7376)
!7689 = !DILocation(line: 784, column: 40, scope: !7683, inlinedAt: !7687)
!7690 = !DILocation(line: 787, column: 9, scope: !7683, inlinedAt: !7687)
!7691 = !DILocalVariable(name: "i", arg: 1, scope: !7692, file: !7395, line: 83, type: !42)
!7692 = distinct !DISubprogram(name: "arch_atomic_add_return", scope: !7395, file: !7395, line: 83, type: !7684, scopeLine: 84, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7693 = !DILocation(line: 83, column: 55, scope: !7692, inlinedAt: !7694)
!7694 = distinct !DILocation(line: 787, column: 9, scope: !7683, inlinedAt: !7687)
!7695 = !DILocalVariable(name: "v", arg: 2, scope: !7692, file: !7395, line: 83, type: !7376)
!7696 = !DILocation(line: 83, column: 68, scope: !7692, inlinedAt: !7694)
!7697 = !DILocation(line: 85, column: 9, scope: !7692, inlinedAt: !7694)
!7698 = !DILocalVariable(name: "__ret", scope: !7699, file: !7395, line: 85, type: !42)
!7699 = distinct !DILexicalBlock(scope: !7692, file: !7395, line: 85, column: 13)
!7700 = !DILocation(line: 85, column: 13, scope: !7699, inlinedAt: !7694)
!7701 = !{i64 2148846090, i64 2148846129, i64 2148846150, i64 2148846187, i64 2148846210, i64 2148846219}
!7702 = !DILocation(line: 85, column: 11, scope: !7692, inlinedAt: !7694)
!7703 = !DILocation(line: 3039, column: 2, scope: !7661)
!7704 = !DILocation(line: 3039, column: 2, scope: !7705)
!7705 = distinct !DILexicalBlock(scope: !7706, file: !342, line: 3039, column: 2)
!7706 = distinct !DILexicalBlock(scope: !7660, file: !342, line: 3039, column: 2)
!7707 = !{i64 2153279901, i64 2153279710, i64 2153279762, i64 2153279808, i64 2153279836}
!7708 = !DILocation(line: 3039, column: 2, scope: !7706)
!7709 = !DILocation(line: 3039, column: 2, scope: !7710)
!7710 = distinct !DILexicalBlock(scope: !7706, file: !342, line: 3039, column: 2)
!7711 = !{i64 2153279975, i64 2153280004, i64 2153280050, i64 2153280108, i64 2153280162, i64 2153280216, i64 2153280271, i64 2153280302}
!7712 = !DILocation(line: 3040, column: 1, scope: !7655)
!7713 = distinct !DISubprogram(name: "file_put_write_access", scope: !7471, file: !7471, line: 104, type: !2363, scopeLine: 105, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7714 = !DILocalVariable(name: "file", arg: 1, scope: !7713, file: !7471, line: 104, type: !896)
!7715 = !DILocation(line: 104, column: 55, scope: !7713)
!7716 = !DILocation(line: 106, column: 19, scope: !7713)
!7717 = !DILocation(line: 106, column: 25, scope: !7713)
!7718 = !DILocation(line: 106, column: 2, scope: !7713)
!7719 = !DILocation(line: 107, column: 23, scope: !7713)
!7720 = !DILocation(line: 107, column: 29, scope: !7713)
!7721 = !DILocation(line: 107, column: 36, scope: !7713)
!7722 = !DILocation(line: 107, column: 2, scope: !7713)
!7723 = !DILocation(line: 108, column: 6, scope: !7724)
!7724 = distinct !DILexicalBlock(scope: !7713, file: !7471, line: 108, column: 6)
!7725 = !DILocation(line: 108, column: 6, scope: !7713)
!7726 = !DILocation(line: 109, column: 47, scope: !7724)
!7727 = !DILocation(line: 109, column: 24, scope: !7724)
!7728 = !DILocation(line: 109, column: 54, scope: !7724)
!7729 = !DILocation(line: 109, column: 3, scope: !7724)
!7730 = !DILocation(line: 110, column: 1, scope: !7713)
!7731 = distinct !DISubprogram(name: "put_write_access", scope: !342, file: !342, line: 3022, type: !3228, scopeLine: 3023, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7732 = !DILocalVariable(name: "inode", arg: 1, scope: !7731, file: !342, line: 3022, type: !779)
!7733 = !DILocation(line: 3022, column: 52, scope: !7731)
!7734 = !DILocation(line: 3024, column: 14, scope: !7731)
!7735 = !DILocation(line: 3024, column: 21, scope: !7731)
!7736 = !DILocalVariable(name: "v", arg: 1, scope: !7737, file: !5951, line: 590, type: !7376)
!7737 = distinct !DISubprogram(name: "atomic_dec", scope: !5951, file: !5951, line: 590, type: !7374, scopeLine: 591, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7738 = !DILocation(line: 590, column: 22, scope: !7737, inlinedAt: !7739)
!7739 = distinct !DILocation(line: 3024, column: 2, scope: !7731)
!7740 = !DILocation(line: 592, column: 31, scope: !7737, inlinedAt: !7739)
!7741 = !DILocation(line: 94, column: 79, scope: !6664, inlinedAt: !7742)
!7742 = distinct !DILocation(line: 592, column: 2, scope: !7737, inlinedAt: !7739)
!7743 = !DILocation(line: 94, column: 89, scope: !6664, inlinedAt: !7742)
!7744 = !DILocation(line: 96, column: 20, scope: !6664, inlinedAt: !7742)
!7745 = !DILocation(line: 96, column: 23, scope: !6664, inlinedAt: !7742)
!7746 = !DILocation(line: 96, column: 2, scope: !6664, inlinedAt: !7742)
!7747 = !DILocation(line: 97, column: 2, scope: !6664, inlinedAt: !7742)
!7748 = !DILocation(line: 593, column: 17, scope: !7737, inlinedAt: !7739)
!7749 = !DILocalVariable(name: "v", arg: 1, scope: !7750, file: !5987, line: 1207, type: !7376)
!7750 = distinct !DISubprogram(name: "raw_atomic_dec", scope: !5987, file: !5987, line: 1207, type: !7374, scopeLine: 1208, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7751 = !DILocation(line: 1207, column: 26, scope: !7750, inlinedAt: !7752)
!7752 = distinct !DILocation(line: 593, column: 2, scope: !7737, inlinedAt: !7739)
!7753 = !DILocation(line: 1210, column: 18, scope: !7750, inlinedAt: !7752)
!7754 = !DILocalVariable(name: "v", arg: 1, scope: !7755, file: !7395, line: 58, type: !7376)
!7755 = distinct !DISubprogram(name: "arch_atomic_dec", scope: !7395, file: !7395, line: 58, type: !7374, scopeLine: 59, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !1201)
!7756 = !DILocation(line: 58, column: 55, scope: !7755, inlinedAt: !7757)
!7757 = distinct !DILocation(line: 1210, column: 2, scope: !7750, inlinedAt: !7752)
!7758 = !DILocation(line: 61, column: 16, scope: !7755, inlinedAt: !7757)
!7759 = !DILocation(line: 60, column: 2, scope: !7755, inlinedAt: !7757)
!7760 = !{i64 2148838341, i64 2148838380, i64 2148838401, i64 2148838438, i64 2148838461, i64 2148838331}
!7761 = !DILocation(line: 3025, column: 1, scope: !7731)
