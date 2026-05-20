# Evidence: SYZBOT-52fced2d288f8ecd  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in copy_process / copy_process (2)
- mainline fix commit: `c17d1a3a8ee4dac7539d5c976b45d9300f6f10bc`
- candidate JSON: `A_SYZBOT-52fced2d288f8ecd.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "copy_process", "fn_b": "copy_process"}`

## Commit message

```
c17d1a3a8ee4dac7539d5c976b45d9300f6f10bc
fork: annotate data race in copy_process()

KCSAN reported data race reading and writing nr_threads and max_threads.
The data race is intentional and benign. This is obvious from the comment
above it and based on general consensus when discussing this issue. So
there's no need for any heavy atomic or *_ONCE() machinery here.

In accordance with the newly introduced data_race() annotation consensus,
mark the offending line with data_race(). Here it's actually useful not
just to silence KCSAN but to also clearly communicate that the race is
intentional. This is especially helpful since nr_threads is otherwise
protected by tasklist_lock.

BUG: KCSAN: data-race in copy_process / copy_process

write to 0xffffffff86205cf8 of 4 bytes by task 14779 on cpu 1:
  copy_process+0x2eba/0x3c40 kernel/fork.c:2273
  _do_fork+0xfe/0x7a0 kernel/fork.c:2421
  __do_sys_clone kernel/fork.c:2576 [inline]
  __se_sys_clone kernel/fork.c:2557 [inline]
  __x64_sys_clone+0x130/0x170 kernel/fork.c:2557
  do_syscall_64+0xcc/0x3a0 arch/x86/entry/common.c:294
  entry_SYSCALL_64_after_hwframe+0x44/0xa9

read to 0xffffffff86205cf8 of 4 bytes by task 6944 on cpu 0:
  copy_process+0x94d/0x3c40 kernel/fork.c:1954
  _do_fork+0xfe/0x7a0 kernel/fork.c:2421
  __do_sys_clone kernel/fork.c:2576 [inline]
  __se_sys_clone kernel/fork.c:2557 [inline]
  __x64_sys_clone+0x130/0x170 kernel/fork.c:2557
  do_syscall_64+0xcc/0x3a0 arch/x86/entry/common.c:294
  entry_SYSCALL_64_after_hwframe+0x44/0xa9
Link: https://groups.google.com/forum/#!msg/syzkaller-upstream-mo
deration/thvp7AHs5Ew/aPdYLXfYBQAJ

Reported-by: syzbot+52fced2d288f8ecd2b20@syzkaller.appspotmail.com
Signed-off-by: Zefan Li <lizefan@huawei.com>
Signed-off-by: Weilong Chen <chenweilong@huawei.com>
Acked-by: Christian Brauner <christian.brauner@ubuntu.com>
Cc: Qian Cai <cai@lca.pw>
Cc: Oleg Nesterov <oleg@redhat.com>
Cc: Christian Brauner <christian.brauner@ubuntu.com>
Cc: Marco Elver <elver@google.com>
[christian.brauner@ubuntu.com: rewrite commit message]
Link: https://lore.kernel.org/r/20200623041240.154294-1-chenweilong@huawei.com
Signed-off-by: Christian Brauner <christian.brauner@ubuntu.com>
```

## CVE / bug description

```
KCSAN: data-race in copy_process / copy_process (2)
```

## Full mainline patch

```diff
commit c17d1a3a8ee4dac7539d5c976b45d9300f6f10bc
Author: Weilong Chen <chenweilong@huawei.com>
Date:   Tue Jun 23 12:12:40 2020 +0800

    fork: annotate data race in copy_process()
    
    KCSAN reported data race reading and writing nr_threads and max_threads.
    The data race is intentional and benign. This is obvious from the comment
    above it and based on general consensus when discussing this issue. So
    there's no need for any heavy atomic or *_ONCE() machinery here.
    
    In accordance with the newly introduced data_race() annotation consensus,
    mark the offending line with data_race(). Here it's actually useful not
    just to silence KCSAN but to also clearly communicate that the race is
    intentional. This is especially helpful since nr_threads is otherwise
    protected by tasklist_lock.
    
    BUG: KCSAN: data-race in copy_process / copy_process
    
    write to 0xffffffff86205cf8 of 4 bytes by task 14779 on cpu 1:
      copy_process+0x2eba/0x3c40 kernel/fork.c:2273
      _do_fork+0xfe/0x7a0 kernel/fork.c:2421
      __do_sys_clone kernel/fork.c:2576 [inline]
      __se_sys_clone kernel/fork.c:2557 [inline]
      __x64_sys_clone+0x130/0x170 kernel/fork.c:2557
      do_syscall_64+0xcc/0x3a0 arch/x86/entry/common.c:294
      entry_SYSCALL_64_after_hwframe+0x44/0xa9
    
    read to 0xffffffff86205cf8 of 4 bytes by task 6944 on cpu 0:
      copy_process+0x94d/0x3c40 kernel/fork.c:1954
      _do_fork+0xfe/0x7a0 kernel/fork.c:2421
      __do_sys_clone kernel/fork.c:2576 [inline]
      __se_sys_clone kernel/fork.c:2557 [inline]
      __x64_sys_clone+0x130/0x170 kernel/fork.c:2557
      do_syscall_64+0xcc/0x3a0 arch/x86/entry/common.c:294
      entry_SYSCALL_64_after_hwframe+0x44/0xa9
    Link: https://groups.google.com/forum/#!msg/syzkaller-upstream-mo
    deration/thvp7AHs5Ew/aPdYLXfYBQAJ
    
    Reported-by: syzbot+52fced2d288f8ecd2b20@syzkaller.appspotmail.com
    Signed-off-by: Zefan Li <lizefan@huawei.com>
    Signed-off-by: Weilong Chen <chenweilong@huawei.com>
    Acked-by: Christian Brauner <christian.brauner@ubuntu.com>
    Cc: Qian Cai <cai@lca.pw>
    Cc: Oleg Nesterov <oleg@redhat.com>
    Cc: Christian Brauner <christian.brauner@ubuntu.com>
    Cc: Marco Elver <elver@google.com>
    [christian.brauner@ubuntu.com: rewrite commit message]
    Link: https://lore.kernel.org/r/20200623041240.154294-1-chenweilong@huawei.com
    Signed-off-by: Christian Brauner <christian.brauner@ubuntu.com>

diff --git a/kernel/fork.c b/kernel/fork.c
index 142b23645d82..efc5493203ae 100644
--- a/kernel/fork.c
+++ b/kernel/fork.c
@@ -1977,7 +1977,7 @@ static __latent_entropy struct task_struct *copy_process(
 	 * to stop root fork bombs.
 	 */
 	retval = -EAGAIN;
-	if (nr_threads >= max_threads)
+	if (data_race(nr_threads >= max_threads))
 		goto bad_fork_cleanup_count;
 
 	delayacct_tsk_init(p);	/* Must remain after dup_task_struct() */

```

## File: `kernel/fork.c`

- changed old-lines: [1980]

### Function `copy_process` (L1841–L2375, 535 lines)

```c
static __latent_entropy struct task_struct *copy_process(
					struct pid *pid,
					int trace,
					int node,
					struct kernel_clone_args *args)
{
	int pidfd = -1, retval;
	struct task_struct *p;
	struct multiprocess_signals delayed;
	struct file *pidfile = NULL;
	u64 clone_flags = args->flags;
	struct nsproxy *nsp = current->nsproxy;

	/*
	 * Don't allow sharing the root directory with processes in a different
	 * namespace
	 */
	if ((clone_flags & (CLONE_NEWNS|CLONE_FS)) == (CLONE_NEWNS|CLONE_FS))
		return ERR_PTR(-EINVAL);

	if ((clone_flags & (CLONE_NEWUSER|CLONE_FS)) == (CLONE_NEWUSER|CLONE_FS))
		return ERR_PTR(-EINVAL);

	/*
	 * Thread groups must share signals as well, and detached threads
	 * can only be started up within the thread group.
	 */
	if ((clone_flags & CLONE_THREAD) && !(clone_flags & CLONE_SIGHAND))
		return ERR_PTR(-EINVAL);

	/*
	 * Shared signal handlers imply shared VM. By way of the above,
	 * thread groups also imply shared VM. Blocking this case allows
	 * for various simplifications in other code.
	 */
	if ((clone_flags & CLONE_SIGHAND) && !(clone_flags & CLONE_VM))
		return ERR_PTR(-EINVAL);

	/*
	 * Siblings of global init remain as zombies on exit since they are
	 * not reaped by their parent (swapper). To solve this and to avoid
	 * multi-rooted process trees, prevent global and container-inits
	 * from creating siblings.
	 */
	if ((clone_flags & CLONE_PARENT) &&
				current->signal->flags & SIGNAL_UNKILLABLE)
		return ERR_PTR(-EINVAL);

	/*
	 * If the new process will be in a different pid or user namespace
	 * do not allow it to share a thread group with the forking task.
	 */
	if (clone_flags & CLONE_THREAD) {
		if ((clone_flags & (CLONE_NEWUSER | CLONE_NEWPID)) ||
		    (task_active_pid_ns(current) != nsp->pid_ns_for_children))
			return ERR_PTR(-EINVAL);
	}

	/*
	 * If the new process will be in a different time namespace
	 * do not allow it to share VM or a thread group with the forking task.
	 */
	if (clone_flags & (CLONE_THREAD | CLONE_VM)) {
		if (nsp->time_ns != nsp->time_ns_for_children)
			return ERR_PTR(-EINVAL);
	}

	if (clone_flags & CLONE_PIDFD) {
		/*
		 * - CLONE_DETACHED is blocked so that we can potentially
		 *   reuse it later for CLONE_PIDFD.
		 * - CLONE_THREAD is blocked until someone really needs it.
		 */
		if (clone_flags & (CLONE_DETACHED | CLONE_THREAD))
			return ERR_PTR(-EINVAL);
	}

	/*
	 * Force any signals received before this point to be delivered
	 * before the fork happens.  Collect up signals sent to multiple
	 * processes that happen during the fork and delay them so that
	 * they appear to happen after the fork.
	 */
	sigemptyset(&delayed.signal);
	INIT_HLIST_NODE(&delayed.node);

	spin_lock_irq(&current->sighand->siglock);
	if (!(clone_flags & CLONE_THREAD))
		hlist_add_head(&delayed.node, &current->signal->multiprocess);
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);
	retval = -ERESTARTNOINTR;
	if (signal_pending(current))
		goto fork_out;

	retval = -ENOMEM;
	p = dup_task_struct(current, node);
	if (!p)
		goto fork_out;

	/*
	 * This _must_ happen before we call free_task(), i.e. before we jump
	 * to any of the bad_fork_* labels. This is to avoid freeing
	 * p->set_child_tid which is (ab)used as a kthread's data pointer for
	 * kernel threads (PF_KTHREAD).
	 */
	p->set_child_tid = (clone_flags & CLONE_CHILD_SETTID) ? args->child_tid : NULL;
	/*
	 * Clear TID on mm_release()?
	 */
	p->clear_child_tid = (clone_flags & CLONE_CHILD_CLEARTID) ? args->child_tid : NULL;

	ftrace_graph_init_task(p);

	rt_mutex_init_task(p);

#ifdef CONFIG_PROVE_LOCKING
	DEBUG_LOCKS_WARN_ON(!p->hardirqs_enabled);
	DEBUG_LOCKS_WARN_ON(!p->softirqs_enabled);
#endif
	retval = -EAGAIN;
	if (atomic_read(&p->real_cred->user->processes) >=
			task_rlimit(p, RLIMIT_NPROC)) {
		if (p->real_cred->user != INIT_USER &&
		    !capable(CAP_SYS_RESOURCE) && !capable(CAP_SYS_ADMIN))
			goto bad_fork_free;
	}
	current->flags &= ~PF_NPROC_EXCEEDED;

	retval = copy_creds(p, clone_flags);
	if (retval < 0)
		goto bad_fork_free;

	/*
	 * If multiple threads are within copy_process(), then this check
	 * triggers too late. This doesn't hurt, the check is only there
	 * to stop root fork bombs.
	 */
	retval = -EAGAIN;
	if (nr_threads >= max_threads)
		goto bad_fork_cleanup_count;

	delayacct_tsk_init(p);	/* Must remain after dup_task_struct() */
	p->flags &= ~(PF_SUPERPRIV | PF_WQ_WORKER | PF_IDLE);
	p->flags |= PF_FORKNOEXEC;
	INIT_LIST_HEAD(&p->children);
	INIT_LIST_HEAD(&p->sibling);
	rcu_copy_process(p);
	p->vfork_done = NULL;
	spin_lock_init(&p->alloc_lock);

	init_sigpending(&p->pending);

	p->utime = p->stime = p->gtime = 0;
#ifdef CONFIG_ARCH_HAS_SCALED_CPUTIME
	p->utimescaled = p->stimescaled = 0;
#endif
	prev_cputime_init(&p->prev_cputime);

#ifdef CONFIG_VIRT_CPU_ACCOUNTING_GEN
	seqcount_init(&p->vtime.seqcount);
	p->vtime.starttime = 0;
	p->vtime.state = VTIME_INACTIVE;
#endif

#if defined(SPLIT_RSS_COUNTING)
	memset(&p->rss_stat, 0, sizeof(p->rss_stat));
#endif

	p->default_timer_slack_ns = current->timer_slack_ns;

#ifdef CONFIG_PSI
	p->psi_flags = 0;
#endif

	task_io_accounting_init(&p->ioac);
	acct_clear_integrals(p);

	posix_cputimers_init(&p->posix_cputimers);

	p->io_context = NULL;
	audit_set_context(p, NULL);
	cgroup_fork(p);
#ifdef CONFIG_NUMA
	p->mempolicy = mpol_dup(p->mempolicy);
	if (IS_ERR(p->mempolicy)) {
		retval = PTR_ERR(p->mempolicy);
		p->mempolicy = NULL;
		goto bad_fork_cleanup_threadgroup_lock;
	}
#endif
#ifdef CONFIG_CPUSETS
	p->cpuset_mem_spread_rotor = NUMA_NO_NODE;
	p->cpuset_slab_spread_rotor = NUMA_NO_NODE;
	seqcount_init(&p->mems_allowed_seq);
#endif
#ifdef CONFIG_TRACE_IRQFLAGS
	p->irq_events = 0;
	p->hardirqs_enabled = 0;
	p->hardirq_enable_ip = 0;
	p->hardirq_enable_event = 0;
	p->hardirq_disable_ip = _THIS_IP_;
	p->hardirq_disable_event = 0;
	p->softirqs_enabled = 1;
	p->softirq_enable_ip = _THIS_IP_;
	p->softirq_enable_event = 0;
	p->softirq_disable_ip = 0;
	p->softirq_disable_event = 0;

```

**External callers of `copy_process` at parent**

- `arch/arm64/kernel/process.c:366` — `	 * later in copy_process(), but to avoid tripping up future`
- `fs/coredump.c:391` — `	 *	visible to zap_threads() because copy_process() adds the new`
- `include/linux/pid.h:159` — ` * pid_ns->child_reaper is assigned in copy_process, we check`
- `kernel/auditsc.c:923` — ` * specified task.  This is called from copy_process, so no lock is`
- `kernel/auditsc.c:1588` — ` * Called from copy_process and do_exit`
- `kernel/cgroup/cgroup.c:5885` — ` * cgroup_fork - initialize cgroup related fields during copy_process()`
- `kernel/cgroup/pids.c:216` — ` * on cgroup_threadgroup_change_begin() held by the copy_process().`
- `kernel/events/core.c:12320` — `		 * Without this copy_process() will unconditionally free this`
- `kernel/events/uprobes.c:1797` — ` * Called in context of a new clone/fork from copy_process.`
- `kernel/kthread.c:74` — `	 * can't be wrongly copied by copy_process(). We also rely on fact`
- `kernel/livepatch/transition.c:611` — `/* Called from copy_process() during fork */`
- `kernel/trace/trace_uprobe.c:1237` — `		 * event->parent != NULL means copy_process(), we can avoid`
