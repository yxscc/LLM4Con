# Evidence: SYZBOT-3872b8b1d5ece2a8  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in __rcu_read_unlock / sync_rcu_exp_select_node_cpus (2)
- mainline fix commit: `314eeb43e5f22856b281c91c966e51e5782a3498`
- candidate JSON: `A_SYZBOT-3872b8b1d5ece2a8.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "__rcu_read_unlock", "fn_b": "sync_rcu_exp_select_node_cpus"}`

## Commit message

```
314eeb43e5f22856b281c91c966e51e5782a3498
rcu: Add *_ONCE() and data_race() to rcu_node ->exp_tasks plus locking

There are lockless loads from the rcu_node structure's ->exp_tasks field,
so this commit causes all stores to use WRITE_ONCE() and all lockless
loads to use READ_ONCE() or data_race(), with the latter for debug
prints.  This code also did a unprotected traversal of the linked list
pointed into by ->exp_tasks, so this commit also acquires the rcu_node
structure's ->lock to properly protect this traversal.  This list was
traversed unprotected only when printing an RCU CPU stall warning for
an expedited grace period, so the odds of seeing this in production are
not all that high.

This data race was reported by KCSAN.

Signed-off-by: Paul E. McKenney <paulmck@kernel.org>
```

## CVE / bug description

```
KCSAN: data-race in __rcu_read_unlock / sync_rcu_exp_select_node_cpus (2)
```

## Full mainline patch

```diff
commit 314eeb43e5f22856b281c91c966e51e5782a3498
Author: Paul E. McKenney <paulmck@kernel.org>
Date:   Fri Jan 3 14:18:12 2020 -0800

    rcu: Add *_ONCE() and data_race() to rcu_node ->exp_tasks plus locking
    
    There are lockless loads from the rcu_node structure's ->exp_tasks field,
    so this commit causes all stores to use WRITE_ONCE() and all lockless
    loads to use READ_ONCE() or data_race(), with the latter for debug
    prints.  This code also did a unprotected traversal of the linked list
    pointed into by ->exp_tasks, so this commit also acquires the rcu_node
    structure's ->lock to properly protect this traversal.  This list was
    traversed unprotected only when printing an RCU CPU stall warning for
    an expedited grace period, so the odds of seeing this in production are
    not all that high.
    
    This data race was reported by KCSAN.
    
    Signed-off-by: Paul E. McKenney <paulmck@kernel.org>

diff --git a/kernel/rcu/tree_exp.h b/kernel/rcu/tree_exp.h
index 1a617b9dffb0..c2b04daf1190 100644
--- a/kernel/rcu/tree_exp.h
+++ b/kernel/rcu/tree_exp.h
@@ -150,7 +150,7 @@ static void __maybe_unused sync_exp_reset_tree(void)
 static bool sync_rcu_exp_done(struct rcu_node *rnp)
 {
 	raw_lockdep_assert_held_rcu_node(rnp);
-	return rnp->exp_tasks == NULL &&
+	return READ_ONCE(rnp->exp_tasks) == NULL &&
 	       READ_ONCE(rnp->expmask) == 0;
 }
 
@@ -373,7 +373,7 @@ static void sync_rcu_exp_select_node_cpus(struct work_struct *wp)
 	 * until such time as the ->expmask bits are cleared.
 	 */
 	if (rcu_preempt_has_tasks(rnp))
-		rnp->exp_tasks = rnp->blkd_tasks.next;
+		WRITE_ONCE(rnp->exp_tasks, rnp->blkd_tasks.next);
 	raw_spin_unlock_irqrestore_rcu_node(rnp, flags);
 
 	/* IPI the remaining CPUs for expedited quiescent state. */
@@ -542,8 +542,8 @@ static void synchronize_rcu_expedited_wait(void)
 		}
 		pr_cont(" } %lu jiffies s: %lu root: %#lx/%c\n",
 			jiffies - jiffies_start, rcu_state.expedited_sequence,
-			READ_ONCE(rnp_root->expmask),
-			".T"[!!rnp_root->exp_tasks]);
+			data_race(rnp_root->expmask),
+			".T"[!!data_race(rnp_root->exp_tasks)]);
 		if (ndetected) {
 			pr_err("blocking rcu_node structures:");
 			rcu_for_each_node_breadth_first(rnp) {
@@ -553,8 +553,8 @@ static void synchronize_rcu_expedited_wait(void)
 					continue;
 				pr_cont(" l=%u:%d-%d:%#lx/%c",
 					rnp->level, rnp->grplo, rnp->grphi,
-					READ_ONCE(rnp->expmask),
-					".T"[!!rnp->exp_tasks]);
+					data_race(rnp->expmask),
+					".T"[!!data_race(rnp->exp_tasks)]);
 			}
 			pr_cont("\n");
 		}
@@ -721,17 +721,20 @@ static void sync_sched_exp_online_cleanup(int cpu)
  */
 static int rcu_print_task_exp_stall(struct rcu_node *rnp)
 {
-	struct task_struct *t;
+	unsigned long flags;
 	int ndetected = 0;
+	struct task_struct *t;
 
-	if (!rnp->exp_tasks)
+	if (!READ_ONCE(rnp->exp_tasks))
 		return 0;
+	raw_spin_lock_irqsave_rcu_node(rnp, flags);
 	t = list_entry(rnp->exp_tasks->prev,
 		       struct task_struct, rcu_node_entry);
 	list_for_each_entry_continue(t, &rnp->blkd_tasks, rcu_node_entry) {
 		pr_cont(" P%d", t->pid);
 		ndetected++;
 	}
+	raw_spin_unlock_irqrestore_rcu_node(rnp, flags);
 	return ndetected;
 }
 
diff --git a/kernel/rcu/tree_plugin.h b/kernel/rcu/tree_plugin.h
index 097635c41135..35d77db035bd 100644
--- a/kernel/rcu/tree_plugin.h
+++ b/kernel/rcu/tree_plugin.h
@@ -226,7 +226,7 @@ static void rcu_preempt_ctxt_queue(struct rcu_node *rnp, struct rcu_data *rdp)
 		WARN_ON_ONCE(rnp->completedqs == rnp->gp_seq);
 	}
 	if (!rnp->exp_tasks && (blkd_state & RCU_EXP_BLKD))
-		rnp->exp_tasks = &t->rcu_node_entry;
+		WRITE_ONCE(rnp->exp_tasks, &t->rcu_node_entry);
 	WARN_ON_ONCE(!(blkd_state & RCU_GP_BLKD) !=
 		     !(rnp->qsmask & rdp->grpmask));
 	WARN_ON_ONCE(!(blkd_state & RCU_EXP_BLKD) !=
@@ -500,7 +500,7 @@ rcu_preempt_deferred_qs_irqrestore(struct task_struct *t, unsigned long flags)
 		if (&t->rcu_node_entry == rnp->gp_tasks)
 			WRITE_ONCE(rnp->gp_tasks, np);
 		if (&t->rcu_node_entry == rnp->exp_tasks)
-			rnp->exp_tasks = np;
+			WRITE_ONCE(rnp->exp_tasks, np);
 		if (IS_ENABLED(CONFIG_RCU_BOOST)) {
 			/* Snapshot ->boost_mtx ownership w/rnp->lock held. */
 			drop_boost_mutex = rt_mutex_owner(&rnp->boost_mtx) == t;
@@ -761,7 +761,7 @@ dump_blkd_tasks(struct rcu_node *rnp, int ncheck)
 			__func__, rnp1->grplo, rnp1->grphi, rnp1->qsmask, rnp1->qsmaskinit, rnp1->qsmaskinitnext);
 	pr_info("%s: ->gp_tasks %p ->boost_tasks %p ->exp_tasks %p\n",
 		__func__, READ_ONCE(rnp->gp_tasks), rnp->boost_tasks,
-		rnp->exp_tasks);
+		READ_ONCE(rnp->exp_tasks));
 	pr_info("%s: ->blkd_tasks", __func__);
 	i = 0;
 	list_for_each(lhp, &rnp->blkd_tasks) {
@@ -1036,7 +1036,7 @@ static int rcu_boost_kthread(void *arg)
 	for (;;) {
 		WRITE_ONCE(rnp->boost_kthread_status, RCU_KTHREAD_WAITING);
 		trace_rcu_utilization(TPS("End boost kthread@rcu_wait"));
-		rcu_wait(rnp->boost_tasks || rnp->exp_tasks);
+		rcu_wait(rnp->boost_tasks || READ_ONCE(rnp->exp_tasks));
 		trace_rcu_utilization(TPS("Start boost kthread@rcu_wait"));
 		WRITE_ONCE(rnp->boost_kthread_status, RCU_KTHREAD_RUNNING);
 		more2boost = rcu_boost(rnp);

```

## File: `kernel/rcu/tree_exp.h`

- changed old-lines: [153, 376, 545, 546, 556, 557, 724, 727]

### Function `sync_rcu_exp_done` (L150–L155, 6 lines)

```c
static bool sync_rcu_exp_done(struct rcu_node *rnp)
{
	raw_lockdep_assert_held_rcu_node(rnp);
	return rnp->exp_tasks == NULL &&
	       READ_ONCE(rnp->expmask) == 0;
}
```

**External callers of `sync_rcu_exp_done` at parent**

- `kernel/rcu/tree_plugin.h:493` — `		empty_exp = sync_rcu_exp_done(rnp);`
- `kernel/rcu/tree_plugin.h:517` — `		empty_exp_now = sync_rcu_exp_done(rnp);`

### Function `sync_rcu_exp_select_node_cpus` (L337–L417, 81 lines)

```c
static void sync_rcu_exp_select_node_cpus(struct work_struct *wp)
{
	int cpu;
	unsigned long flags;
	unsigned long mask_ofl_test;
	unsigned long mask_ofl_ipi;
	int ret;
	struct rcu_exp_work *rewp =
		container_of(wp, struct rcu_exp_work, rew_work);
	struct rcu_node *rnp = container_of(rewp, struct rcu_node, rew);

	raw_spin_lock_irqsave_rcu_node(rnp, flags);

	/* Each pass checks a CPU for identity, offline, and idle. */
	mask_ofl_test = 0;
	for_each_leaf_node_cpu_mask(rnp, cpu, rnp->expmask) {
		struct rcu_data *rdp = per_cpu_ptr(&rcu_data, cpu);
		unsigned long mask = rdp->grpmask;
		int snap;

		if (raw_smp_processor_id() == cpu ||
		    !(rnp->qsmaskinitnext & mask)) {
			mask_ofl_test |= mask;
		} else {
			snap = rcu_dynticks_snap(rdp);
			if (rcu_dynticks_in_eqs(snap))
				mask_ofl_test |= mask;
			else
				rdp->exp_dynticks_snap = snap;
		}
	}
	mask_ofl_ipi = rnp->expmask & ~mask_ofl_test;

	/*
	 * Need to wait for any blocked tasks as well.	Note that
	 * additional blocking tasks will also block the expedited GP
	 * until such time as the ->expmask bits are cleared.
	 */
	if (rcu_preempt_has_tasks(rnp))
		rnp->exp_tasks = rnp->blkd_tasks.next;
	raw_spin_unlock_irqrestore_rcu_node(rnp, flags);

	/* IPI the remaining CPUs for expedited quiescent state. */
	for_each_leaf_node_cpu_mask(rnp, cpu, mask_ofl_ipi) {
		struct rcu_data *rdp = per_cpu_ptr(&rcu_data, cpu);
		unsigned long mask = rdp->grpmask;

retry_ipi:
		if (rcu_dynticks_in_eqs_since(rdp, rdp->exp_dynticks_snap)) {
			mask_ofl_test |= mask;
			continue;
		}
		if (get_cpu() == cpu) {
			put_cpu();
			continue;
		}
		ret = smp_call_function_single(cpu, rcu_exp_handler, NULL, 0);
		put_cpu();
		/* The CPU will report the QS in response to the IPI. */
		if (!ret)
			continue;

		/* Failed, raced with CPU hotplug operation. */
		raw_spin_lock_irqsave_rcu_node(rnp, flags);
		if ((rnp->qsmaskinitnext & mask) &&
		    (rnp->expmask & mask)) {
			/* Online, so delay for a bit and try again. */
			raw_spin_unlock_irqrestore_rcu_node(rnp, flags);
			trace_rcu_exp_grace_period(rcu_state.name, rcu_exp_gp_seq_endval(), TPS("selectofl"));
			schedule_timeout_uninterruptible(1);
			goto retry_ipi;
		}
		/* CPU really is offline, so we must report its QS. */
		if (rnp->expmask & mask)
			mask_ofl_test |= mask;
		raw_spin_unlock_irqrestore_rcu_node(rnp, flags);
	}
	/* Report quiescent states for those that went offline. */
	if (mask_ofl_test)
		rcu_report_exp_cpu_mult(rnp, mask_ofl_test, false);
}
```

_(no external call sites found for `sync_rcu_exp_select_node_cpus`)_

### Function `synchronize_rcu_expedited_wait` (L485–L571, 87 lines)

```c
static void synchronize_rcu_expedited_wait(void)
{
	int cpu;
	unsigned long j;
	unsigned long jiffies_stall;
	unsigned long jiffies_start;
	unsigned long mask;
	int ndetected;
	struct rcu_data *rdp;
	struct rcu_node *rnp;
	struct rcu_node *rnp_root = rcu_get_root();

	trace_rcu_exp_grace_period(rcu_state.name, rcu_exp_gp_seq_endval(), TPS("startwait"));
	jiffies_stall = rcu_jiffies_till_stall_check();
	jiffies_start = jiffies;
	if (tick_nohz_full_enabled() && rcu_inkernel_boot_has_ended()) {
		if (synchronize_rcu_expedited_wait_once(1))
			return;
		rcu_for_each_leaf_node(rnp) {
			for_each_leaf_node_cpu_mask(rnp, cpu, rnp->expmask) {
				rdp = per_cpu_ptr(&rcu_data, cpu);
				if (rdp->rcu_forced_tick_exp)
					continue;
				rdp->rcu_forced_tick_exp = true;
				tick_dep_set_cpu(cpu, TICK_DEP_BIT_RCU_EXP);
			}
		}
		j = READ_ONCE(jiffies_till_first_fqs);
		if (synchronize_rcu_expedited_wait_once(j + HZ))
			return;
		WARN_ON_ONCE(IS_ENABLED(CONFIG_PREEMPT_RT));
	}

	for (;;) {
		if (synchronize_rcu_expedited_wait_once(jiffies_stall))
			return;
		if (rcu_stall_is_suppressed())
			continue;
		panic_on_rcu_stall();
		pr_err("INFO: %s detected expedited stalls on CPUs/tasks: {",
		       rcu_state.name);
		ndetected = 0;
		rcu_for_each_leaf_node(rnp) {
			ndetected += rcu_print_task_exp_stall(rnp);
			for_each_leaf_node_possible_cpu(rnp, cpu) {
				struct rcu_data *rdp;

				mask = leaf_node_cpu_bit(rnp, cpu);
				if (!(READ_ONCE(rnp->expmask) & mask))
					continue;
				ndetected++;
				rdp = per_cpu_ptr(&rcu_data, cpu);
				pr_cont(" %d-%c%c%c", cpu,
					"O."[!!cpu_online(cpu)],
					"o."[!!(rdp->grpmask & rnp->expmaskinit)],
					"N."[!!(rdp->grpmask & rnp->expmaskinitnext)]);
			}
		}
		pr_cont(" } %lu jiffies s: %lu root: %#lx/%c\n",
			jiffies - jiffies_start, rcu_state.expedited_sequence,
			READ_ONCE(rnp_root->expmask),
			".T"[!!rnp_root->exp_tasks]);
		if (ndetected) {
			pr_err("blocking rcu_node structures:");
			rcu_for_each_node_breadth_first(rnp) {
				if (rnp == rnp_root)
					continue; /* printed unconditionally */
				if (sync_rcu_exp_done_unlocked(rnp))
					continue;
				pr_cont(" l=%u:%d-%d:%#lx/%c",
					rnp->level, rnp->grplo, rnp->grphi,
					READ_ONCE(rnp->expmask),
					".T"[!!rnp->exp_tasks]);
			}
			pr_cont("\n");
		}
		rcu_for_each_leaf_node(rnp) {
			for_each_leaf_node_possible_cpu(rnp, cpu) {
				mask = leaf_node_cpu_bit(rnp, cpu);
				if (!(READ_ONCE(rnp->expmask) & mask))
					continue;
				dump_cpu_task(cpu);
			}
		}
		jiffies_stall = 3 * rcu_jiffies_till_stall_check() + 3;
	}
}
```

_(no external call sites found for `synchronize_rcu_expedited_wait`)_

### Function `rcu_print_task_exp_stall` (L722–L736, 15 lines)

```c
static int rcu_print_task_exp_stall(struct rcu_node *rnp)
{
	struct task_struct *t;
	int ndetected = 0;

	if (!rnp->exp_tasks)
		return 0;
	t = list_entry(rnp->exp_tasks->prev,
		       struct task_struct, rcu_node_entry);
	list_for_each_entry_continue(t, &rnp->blkd_tasks, rcu_node_entry) {
		pr_cont(" P%d", t->pid);
		ndetected++;
	}
	return ndetected;
}
```

**External callers of `rcu_print_task_exp_stall` at parent**

- `kernel/rcu/tree.h:404` — `static int rcu_print_task_exp_stall(struct rcu_node *rnp);`

## File: `kernel/rcu/tree_plugin.h`

- changed old-lines: [229, 503, 764, 1039]

### Function `rcu_preempt_ctxt_queue` (L132–L246, 115 lines)

```c
static void rcu_preempt_ctxt_queue(struct rcu_node *rnp, struct rcu_data *rdp)
	__releases(rnp->lock) /* But leaves rrupts disabled. */
{
	int blkd_state = (rnp->gp_tasks ? RCU_GP_TASKS : 0) +
			 (rnp->exp_tasks ? RCU_EXP_TASKS : 0) +
			 (rnp->qsmask & rdp->grpmask ? RCU_GP_BLKD : 0) +
			 (rnp->expmask & rdp->grpmask ? RCU_EXP_BLKD : 0);
	struct task_struct *t = current;

	raw_lockdep_assert_held_rcu_node(rnp);
	WARN_ON_ONCE(rdp->mynode != rnp);
	WARN_ON_ONCE(!rcu_is_leaf_node(rnp));
	/* RCU better not be waiting on newly onlined CPUs! */
	WARN_ON_ONCE(rnp->qsmaskinitnext & ~rnp->qsmaskinit & rnp->qsmask &
		     rdp->grpmask);

	/*
	 * Decide where to queue the newly blocked task.  In theory,
	 * this could be an if-statement.  In practice, when I tried
	 * that, it was quite messy.
	 */
	switch (blkd_state) {
	case 0:
	case                RCU_EXP_TASKS:
	case                RCU_EXP_TASKS + RCU_GP_BLKD:
	case RCU_GP_TASKS:
	case RCU_GP_TASKS + RCU_EXP_TASKS:

		/*
		 * Blocking neither GP, or first task blocking the normal
		 * GP but not blocking the already-waiting expedited GP.
		 * Queue at the head of the list to avoid unnecessarily
		 * blocking the already-waiting GPs.
		 */
		list_add(&t->rcu_node_entry, &rnp->blkd_tasks);
		break;

	case                                              RCU_EXP_BLKD:
	case                                RCU_GP_BLKD:
	case                                RCU_GP_BLKD + RCU_EXP_BLKD:
	case RCU_GP_TASKS +                               RCU_EXP_BLKD:
	case RCU_GP_TASKS +                 RCU_GP_BLKD + RCU_EXP_BLKD:
	case RCU_GP_TASKS + RCU_EXP_TASKS + RCU_GP_BLKD + RCU_EXP_BLKD:

		/*
		 * First task arriving that blocks either GP, or first task
		 * arriving that blocks the expedited GP (with the normal
		 * GP already waiting), or a task arriving that blocks
		 * both GPs with both GPs already waiting.  Queue at the
		 * tail of the list to avoid any GP waiting on any of the
		 * already queued tasks that are not blocking it.
		 */
		list_add_tail(&t->rcu_node_entry, &rnp->blkd_tasks);
		break;

	case                RCU_EXP_TASKS +               RCU_EXP_BLKD:
	case                RCU_EXP_TASKS + RCU_GP_BLKD + RCU_EXP_BLKD:
	case RCU_GP_TASKS + RCU_EXP_TASKS +               RCU_EXP_BLKD:

		/*
		 * Second or subsequent task blocking the expedited GP.
		 * The task either does not block the normal GP, or is the
		 * first task blocking the normal GP.  Queue just after
		 * the first task blocking the expedited GP.
		 */
		list_add(&t->rcu_node_entry, rnp->exp_tasks);
		break;

	case RCU_GP_TASKS +                 RCU_GP_BLKD:
	case RCU_GP_TASKS + RCU_EXP_TASKS + RCU_GP_BLKD:

		/*
		 * Second or subsequent task blocking the normal GP.
		 * The task does not block the expedited GP. Queue just
		 * after the first task blocking the normal GP.
		 */
		list_add(&t->rcu_node_entry, rnp->gp_tasks);
		break;

	default:

		/* Yet another exercise in excessive paranoia. */
		WARN_ON_ONCE(1);
		break;
	}

	/*
	 * We have now queued the task.  If it was the first one to
	 * block either grace period, update the ->gp_tasks and/or
	 * ->exp_tasks pointers, respectively, to reference the newly
	 * blocked tasks.
	 */
	if (!rnp->gp_tasks && (blkd_state & RCU_GP_BLKD)) {
		WRITE_ONCE(rnp->gp_tasks, &t->rcu_node_entry);
		WARN_ON_ONCE(rnp->completedqs == rnp->gp_seq);
	}
	if (!rnp->exp_tasks && (blkd_state & RCU_EXP_BLKD))
		rnp->exp_tasks = &t->rcu_node_entry;
	WARN_ON_ONCE(!(blkd_state & RCU_GP_BLKD) !=
		     !(rnp->qsmask & rdp->grpmask));
	WARN_ON_ONCE(!(blkd_state & RCU_EXP_BLKD) !=
		     !(rnp->expmask & rdp->grpmask));
	raw_spin_unlock_rcu_node(rnp); /* interrupts remain disabled. */

	/*
	 * Report the quiescent state for the expedited GP.  This expedited
	 * GP should not be able to end until we report, so there should be
	 * no need to check for a subsequent expedited GP.  (Though we are
	 * still in a quiescent state in any case.)
	 */
	if (blkd_state & RCU_EXP_BLKD && rdp->exp_deferred_qs)
		rcu_report_exp_rdp(rdp);
	else
		WARN_ON_ONCE(rdp->exp_deferred_qs);
}
```

_(no external call sites found for `rcu_preempt_ctxt_queue`)_

### Function `if` (L474–L543, 70 lines)

```c
	if (rdp->exp_deferred_qs)
		rcu_report_exp_rdp(rdp);

	/* Clean up if blocked during RCU read-side critical section. */
	if (special.b.blocked) {

		/*
		 * Remove this task from the list it blocked on.  The task
		 * now remains queued on the rcu_node corresponding to the
		 * CPU it first blocked on, so there is no longer any need
		 * to loop.  Retain a WARN_ON_ONCE() out of sheer paranoia.
		 */
		rnp = t->rcu_blocked_node;
		raw_spin_lock_rcu_node(rnp); /* irqs already disabled. */
		WARN_ON_ONCE(rnp != t->rcu_blocked_node);
		WARN_ON_ONCE(!rcu_is_leaf_node(rnp));
		empty_norm = !rcu_preempt_blocked_readers_cgp(rnp);
		WARN_ON_ONCE(rnp->completedqs == rnp->gp_seq &&
			     (!empty_norm || rnp->qsmask));
		empty_exp = sync_rcu_exp_done(rnp);
		smp_mb(); /* ensure expedited fastpath sees end of RCU c-s. */
		np = rcu_next_node_entry(t, rnp);
		list_del_init(&t->rcu_node_entry);
		t->rcu_blocked_node = NULL;
		trace_rcu_unlock_preempted_task(TPS("rcu_preempt"),
						rnp->gp_seq, t->pid);
		if (&t->rcu_node_entry == rnp->gp_tasks)
			WRITE_ONCE(rnp->gp_tasks, np);
		if (&t->rcu_node_entry == rnp->exp_tasks)
			rnp->exp_tasks = np;
		if (IS_ENABLED(CONFIG_RCU_BOOST)) {
			/* Snapshot ->boost_mtx ownership w/rnp->lock held. */
			drop_boost_mutex = rt_mutex_owner(&rnp->boost_mtx) == t;
			if (&t->rcu_node_entry == rnp->boost_tasks)
				rnp->boost_tasks = np;
		}

		/*
		 * If this was the last task on the current list, and if
		 * we aren't waiting on any CPUs, report the quiescent state.
		 * Note that rcu_report_unblock_qs_rnp() releases rnp->lock,
		 * so we must take a snapshot of the expedited state.
		 */
		empty_exp_now = sync_rcu_exp_done(rnp);
		if (!empty_norm && !rcu_preempt_blocked_readers_cgp(rnp)) {
			trace_rcu_quiescent_state_report(TPS("preempt_rcu"),
							 rnp->gp_seq,
							 0, rnp->qsmask,
							 rnp->level,
							 rnp->grplo,
							 rnp->grphi,
							 !!rnp->gp_tasks);
			rcu_report_unblock_qs_rnp(rnp, flags);
		} else {
			raw_spin_unlock_irqrestore_rcu_node(rnp, flags);
		}

		/* Unboost if we were boosted. */
		if (IS_ENABLED(CONFIG_RCU_BOOST) && drop_boost_mutex)
			rt_mutex_futex_unlock(&rnp->boost_mtx);

		/*
		 * If this was the last task on the expedited lists,
		 * then we need to report up the rcu_node hierarchy.
		 */
		if (!empty_exp && empty_exp_now)
			rcu_report_exp_rnp(rnp, true);
	} else {
		local_irq_restore(flags);
	}
```

**External callers of `if` at parent**

- `Documentation/scheduler/sched-pelt.c:28` — `		if (i % 6 == 0) printf("\n\t");`
- `Documentation/scheduler/sched-pelt.c:42` — `		if (i == 1)`
- `Documentation/scheduler/sched-pelt.c:47` — `		if (i % 11 == 0)`
- `Documentation/scheduler/sched-pelt.c:64` — `		if (n > -1)`
- `Documentation/scheduler/sched-pelt.c:71` — `		if (last == max)`
- `Documentation/scheduler/sched-pelt.c:88` — `		if (i > 1)`
- `Documentation/scheduler/sched-pelt.c:91` — `		if (i % 6 == 0)`
- `Documentation/usb/usbdevfs-drop-permissions.c:24` — `	if (res)`
- `Documentation/usb/usbdevfs-drop-permissions.c:35` — `	if (!res)`
- `Documentation/usb/usbdevfs-drop-permissions.c:48` — `		if (!res)`
- `Documentation/usb/usbdevfs-drop-permissions.c:49` — `			printf("OK: claimed if %d\n", i);`
- `Documentation/usb/usbdevfs-drop-permissions.c:51` — `			printf("ERROR claiming if %d (%d - %s)\n",`
- `Documentation/usb/usbdevfs-drop-permissions.c:62` — `	if (fd < 0) {`
- `Documentation/usb/usbdevfs-drop-permissions.c:68` — `	 * check if dropping privileges is supported,`
- `Documentation/usb/usbdevfs-drop-permissions.c:72` — `	if (!(caps & USBDEVFS_CAP_DROP_PRIVILEGES)) {`
- `Documentation/usb/usbdevfs-drop-permissions.c:85` — `		"[1] Reset device. Should fail if device is in use\n"`
- `arch/alpha/boot/bootp.c:97` — `	if (i) {`
- `arch/alpha/boot/bootp.c:154` — `	if (INIT_HWRPB->pagesize != 8192) {`
- `arch/alpha/boot/bootp.c:159` — `	if (INIT_HWRPB->vptb != (unsigned long) VPTB) {`
- `arch/alpha/boot/bootp.c:181` — `	if (nbytes < 0 || nbytes >= sizeof(envval)) {`

### Function `for` (L759–L771, 13 lines)

```c
	for (rnp1 = rnp; rnp1; rnp1 = rnp1->parent)
		pr_info("%s: %d:%d ->qsmask %#lx ->qsmaskinit %#lx ->qsmaskinitnext %#lx\n",
			__func__, rnp1->grplo, rnp1->grphi, rnp1->qsmask, rnp1->qsmaskinit, rnp1->qsmaskinitnext);
	pr_info("%s: ->gp_tasks %p ->boost_tasks %p ->exp_tasks %p\n",
		__func__, READ_ONCE(rnp->gp_tasks), rnp->boost_tasks,
		rnp->exp_tasks);
	pr_info("%s: ->blkd_tasks", __func__);
	i = 0;
	list_for_each(lhp, &rnp->blkd_tasks) {
		pr_cont(" %p", lhp);
		if (++i >= ncheck)
			break;
	}
```

**External callers of `for` at parent**

- `Documentation/scheduler/sched-pelt.c:2` — ` * The following program is used to generate the constants for`
- `Documentation/scheduler/sched-pelt.c:25` — `	for (i = 0; i < HALFLIFE; i++) {`
- `Documentation/scheduler/sched-pelt.c:41` — `	for (i = 1; i <= HALFLIFE; i++) {`
- `Documentation/scheduler/sched-pelt.c:63` — `	for (; ; n++) {`
- `Documentation/scheduler/sched-pelt.c:87` — `	for (i = 1; i <= n/HALFLIFE+1; i++) {`
- `Documentation/usb/usbdevfs-drop-permissions.c:46` — `	for (i = 0; i < 4; i++) {`
- `arch/alpha/boot/bootp.c:7` — ` * This file is used for creating a bootp file for the Linux/AXP kernel`
- `arch/alpha/boot/bootp.c:56` — ` * PCB for that. The kernel proper should replace this PCB with`
- `arch/alpha/boot/bootp.c:153` — `	srm_printk("Linux/AXP bootp loader for Linux " UTS_RELEASE "\n");`
- `arch/alpha/boot/bootp.c:166` — `	/* The initrd must be page-aligned.  See below for the`
- `arch/alpha/boot/bootpz.c:7` — ` * This file is used for creating a compressed BOOTP file for the`
- `arch/alpha/boot/bootpz.c:29` — `#define MALLOC_AREA_SIZE 0x200000 /* 2MB for now */`
- `arch/alpha/boot/bootpz.c:86` — `	/* do some range checking for detecting an overlap... */`
- `arch/alpha/boot/bootpz.c:87` — `	for (vaddr = vstart; vaddr <= vend; vaddr += PAGE_SIZE)`
- `arch/alpha/boot/bootpz.c:105` — ` * PCB for that. The kernel proper should replace this PCB with`
- `arch/alpha/boot/bootpz.c:212` — `/* Virtual addresses for the BOOTP image. Note that this includes the`
- `arch/alpha/boot/bootpz.c:222` — `/* Virtual addresses for just the bootstrapper part of the BOOTP image. */`
- `arch/alpha/boot/bootpz.c:226` — `/* Virtual addresses for just the data part of the BOOTP`
- `arch/alpha/boot/bootpz.c:235` — `/* KSEG addresses for the uncompressed kernel.`
- `arch/alpha/boot/bootpz.c:237` — `   Note that the end address includes workspace for the decompression.`

### Function `rcu_boost_kthread` (L1029–L1058, 30 lines)

```c
static int rcu_boost_kthread(void *arg)
{
	struct rcu_node *rnp = (struct rcu_node *)arg;
	int spincnt = 0;
	int more2boost;

	trace_rcu_utilization(TPS("Start boost kthread@init"));
	for (;;) {
		WRITE_ONCE(rnp->boost_kthread_status, RCU_KTHREAD_WAITING);
		trace_rcu_utilization(TPS("End boost kthread@rcu_wait"));
		rcu_wait(rnp->boost_tasks || rnp->exp_tasks);
		trace_rcu_utilization(TPS("Start boost kthread@rcu_wait"));
		WRITE_ONCE(rnp->boost_kthread_status, RCU_KTHREAD_RUNNING);
		more2boost = rcu_boost(rnp);
		if (more2boost)
			spincnt++;
		else
			spincnt = 0;
		if (spincnt > 10) {
			WRITE_ONCE(rnp->boost_kthread_status, RCU_KTHREAD_YIELDING);
			trace_rcu_utilization(TPS("End boost kthread@rcu_yield"));
			schedule_timeout_interruptible(2);
			trace_rcu_utilization(TPS("Start boost kthread@rcu_yield"));
			spincnt = 0;
		}
	}
	/* NOTREACHED */
	trace_rcu_utilization(TPS("End boost kthread@notreached"));
	return 0;
}
```

_(no external call sites found for `rcu_boost_kthread`)_
