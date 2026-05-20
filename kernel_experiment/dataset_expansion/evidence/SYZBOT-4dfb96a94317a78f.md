# Evidence: SYZBOT-4dfb96a94317a78f  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in call_rcu / rcu_gp_fqs_loop
- mainline fix commit: `2431774f04d1050292054c763070021bade7b151`
- candidate JSON: `A_SYZBOT-4dfb96a94317a78f.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "call_rcu", "fn_b": "rcu_gp_fqs_loop"}`

## Commit message

```
2431774f04d1050292054c763070021bade7b151
rcu: Mark accesses to rcu_state.n_force_qs

This commit marks accesses to the rcu_state.n_force_qs.  These data
races are hard to make happen, but syzkaller was equal to the task.

Reported-by: syzbot+e08a83a1940ec3846cd5@syzkaller.appspotmail.com
Acked-by: Marco Elver <elver@google.com>
Signed-off-by: Paul E. McKenney <paulmck@kernel.org>
```

## CVE / bug description

```
KCSAN: data-race in call_rcu / rcu_gp_fqs_loop
```

## Full mainline patch

```diff
commit 2431774f04d1050292054c763070021bade7b151
Author: Paul E. McKenney <paulmck@kernel.org>
Date:   Tue Jul 20 06:16:27 2021 -0700

    rcu: Mark accesses to rcu_state.n_force_qs
    
    This commit marks accesses to the rcu_state.n_force_qs.  These data
    races are hard to make happen, but syzkaller was equal to the task.
    
    Reported-by: syzbot+e08a83a1940ec3846cd5@syzkaller.appspotmail.com
    Acked-by: Marco Elver <elver@google.com>
    Signed-off-by: Paul E. McKenney <paulmck@kernel.org>

diff --git a/kernel/rcu/tree.c b/kernel/rcu/tree.c
index bce848e50512..c89f5e6c4154 100644
--- a/kernel/rcu/tree.c
+++ b/kernel/rcu/tree.c
@@ -1907,7 +1907,7 @@ static void rcu_gp_fqs(bool first_time)
 	struct rcu_node *rnp = rcu_get_root();
 
 	WRITE_ONCE(rcu_state.gp_activity, jiffies);
-	rcu_state.n_force_qs++;
+	WRITE_ONCE(rcu_state.n_force_qs, rcu_state.n_force_qs + 1);
 	if (first_time) {
 		/* Collect dyntick-idle snapshots. */
 		force_qs_rnp(dyntick_save_progress_counter);
@@ -2550,7 +2550,7 @@ static void rcu_do_batch(struct rcu_data *rdp)
 	/* Reset ->qlen_last_fqs_check trigger if enough CBs have drained. */
 	if (count == 0 && rdp->qlen_last_fqs_check != 0) {
 		rdp->qlen_last_fqs_check = 0;
-		rdp->n_force_qs_snap = rcu_state.n_force_qs;
+		rdp->n_force_qs_snap = READ_ONCE(rcu_state.n_force_qs);
 	} else if (count < rdp->qlen_last_fqs_check - qhimark)
 		rdp->qlen_last_fqs_check = count;
 
@@ -2898,10 +2898,10 @@ static void __call_rcu_core(struct rcu_data *rdp, struct rcu_head *head,
 		} else {
 			/* Give the grace period a kick. */
 			rdp->blimit = DEFAULT_MAX_RCU_BLIMIT;
-			if (rcu_state.n_force_qs == rdp->n_force_qs_snap &&
+			if (READ_ONCE(rcu_state.n_force_qs) == rdp->n_force_qs_snap &&
 			    rcu_segcblist_first_pend_cb(&rdp->cblist) != head)
 				rcu_force_quiescent_state();
-			rdp->n_force_qs_snap = rcu_state.n_force_qs;
+			rdp->n_force_qs_snap = READ_ONCE(rcu_state.n_force_qs);
 			rdp->qlen_last_fqs_check = rcu_segcblist_n_cbs(&rdp->cblist);
 		}
 	}
@@ -4128,7 +4128,7 @@ int rcutree_prepare_cpu(unsigned int cpu)
 	/* Set up local state, ensuring consistent view of global state. */
 	raw_spin_lock_irqsave_rcu_node(rnp, flags);
 	rdp->qlen_last_fqs_check = 0;
-	rdp->n_force_qs_snap = rcu_state.n_force_qs;
+	rdp->n_force_qs_snap = READ_ONCE(rcu_state.n_force_qs);
 	rdp->blimit = blimit;
 	rdp->dynticks_nesting = 1;	/* CPU not up, no tearing. */
 	rcu_dynticks_eqs_online();

```

## File: `kernel/rcu/tree.c`

- changed old-lines: [1910, 2553, 2901, 2904, 4131]

### Function `rcu_gp_fqs` (L1905–L1925, 21 lines)

```c
static void rcu_gp_fqs(bool first_time)
{
	struct rcu_node *rnp = rcu_get_root();

	WRITE_ONCE(rcu_state.gp_activity, jiffies);
	rcu_state.n_force_qs++;
	if (first_time) {
		/* Collect dyntick-idle snapshots. */
		force_qs_rnp(dyntick_save_progress_counter);
	} else {
		/* Handle dyntick-idle and offline CPUs. */
		force_qs_rnp(rcu_implicit_dynticks_qs);
	}
	/* Clear flag to prevent immediate re-entry. */
	if (READ_ONCE(rcu_state.gp_flags) & RCU_GP_FLAG_FQS) {
		raw_spin_lock_irq_rcu_node(rnp);
		WRITE_ONCE(rcu_state.gp_flags,
			   READ_ONCE(rcu_state.gp_flags) & ~RCU_GP_FLAG_FQS);
		raw_spin_unlock_irq_rcu_node(rnp);
	}
}
```

_(no external call sites found for `rcu_gp_fqs`)_

### Function `rcu_do_batch` (L2444–L2574, 131 lines)

```c
static void rcu_do_batch(struct rcu_data *rdp)
{
	int div;
	bool __maybe_unused empty;
	unsigned long flags;
	const bool offloaded = rcu_rdp_is_offloaded(rdp);
	struct rcu_head *rhp;
	struct rcu_cblist rcl = RCU_CBLIST_INITIALIZER(rcl);
	long bl, count = 0;
	long pending, tlimit = 0;

	/* If no callbacks are ready, just return. */
	if (!rcu_segcblist_ready_cbs(&rdp->cblist)) {
		trace_rcu_batch_start(rcu_state.name,
				      rcu_segcblist_n_cbs(&rdp->cblist), 0);
		trace_rcu_batch_end(rcu_state.name, 0,
				    !rcu_segcblist_empty(&rdp->cblist),
				    need_resched(), is_idle_task(current),
				    rcu_is_callbacks_kthread());
		return;
	}

	/*
	 * Extract the list of ready callbacks, disabling to prevent
	 * races with call_rcu() from interrupt handlers.  Leave the
	 * callback counts, as rcu_barrier() needs to be conservative.
	 */
	local_irq_save(flags);
	rcu_nocb_lock(rdp);
	WARN_ON_ONCE(cpu_is_offline(smp_processor_id()));
	pending = rcu_segcblist_n_cbs(&rdp->cblist);
	div = READ_ONCE(rcu_divisor);
	div = div < 0 ? 7 : div > sizeof(long) * 8 - 2 ? sizeof(long) * 8 - 2 : div;
	bl = max(rdp->blimit, pending >> div);
	if (unlikely(bl > 100)) {
		long rrn = READ_ONCE(rcu_resched_ns);

		rrn = rrn < NSEC_PER_MSEC ? NSEC_PER_MSEC : rrn > NSEC_PER_SEC ? NSEC_PER_SEC : rrn;
		tlimit = local_clock() + rrn;
	}
	trace_rcu_batch_start(rcu_state.name,
			      rcu_segcblist_n_cbs(&rdp->cblist), bl);
	rcu_segcblist_extract_done_cbs(&rdp->cblist, &rcl);
	if (offloaded)
		rdp->qlen_last_fqs_check = rcu_segcblist_n_cbs(&rdp->cblist);

	trace_rcu_segcb_stats(&rdp->cblist, TPS("SegCbDequeued"));
	rcu_nocb_unlock_irqrestore(rdp, flags);

	/* Invoke callbacks. */
	tick_dep_set_task(current, TICK_DEP_BIT_RCU);
	rhp = rcu_cblist_dequeue(&rcl);

	for (; rhp; rhp = rcu_cblist_dequeue(&rcl)) {
		rcu_callback_t f;

		count++;
		debug_rcu_head_unqueue(rhp);

		rcu_lock_acquire(&rcu_callback_map);
		trace_rcu_invoke_callback(rcu_state.name, rhp);

		f = rhp->func;
		WRITE_ONCE(rhp->func, (rcu_callback_t)0L);
		f(rhp);

		rcu_lock_release(&rcu_callback_map);

		/*
		 * Stop only if limit reached and CPU has something to do.
		 */
		if (count >= bl && !offloaded &&
		    (need_resched() ||
		     (!is_idle_task(current) && !rcu_is_callbacks_kthread())))
			break;
		if (unlikely(tlimit)) {
			/* only call local_clock() every 32 callbacks */
			if (likely((count & 31) || local_clock() < tlimit))
				continue;
			/* Exceeded the time limit, so leave. */
			break;
		}
		if (!in_serving_softirq()) {
			local_bh_enable();
			lockdep_assert_irqs_enabled();
			cond_resched_tasks_rcu_qs();
			lockdep_assert_irqs_enabled();
			local_bh_disable();
		}
	}

	local_irq_save(flags);
	rcu_nocb_lock(rdp);
	rdp->n_cbs_invoked += count;
	trace_rcu_batch_end(rcu_state.name, count, !!rcl.head, need_resched(),
			    is_idle_task(current), rcu_is_callbacks_kthread());

	/* Update counts and requeue any remaining callbacks. */
	rcu_segcblist_insert_done_cbs(&rdp->cblist, &rcl);
	rcu_segcblist_add_len(&rdp->cblist, -count);

	/* Reinstate batch limit if we have worked down the excess. */
	count = rcu_segcblist_n_cbs(&rdp->cblist);
	if (rdp->blimit >= DEFAULT_MAX_RCU_BLIMIT && count <= qlowmark)
		rdp->blimit = blimit;

	/* Reset ->qlen_last_fqs_check trigger if enough CBs have drained. */
	if (count == 0 && rdp->qlen_last_fqs_check != 0) {
		rdp->qlen_last_fqs_check = 0;
		rdp->n_force_qs_snap = rcu_state.n_force_qs;
	} else if (count < rdp->qlen_last_fqs_check - qhimark)
		rdp->qlen_last_fqs_check = count;

	/*
	 * The following usually indicates a double call_rcu().  To track
	 * this down, try building with CONFIG_DEBUG_OBJECTS_RCU_HEAD=y.
	 */
	empty = rcu_segcblist_empty(&rdp->cblist);
	WARN_ON_ONCE(count == 0 && !empty);
	WARN_ON_ONCE(!IS_ENABLED(CONFIG_RCU_NOCB_CPU) &&
		     count != 0 && empty);
	WARN_ON_ONCE(count == 0 && rcu_segcblist_n_segment_cbs(&rdp->cblist) != 0);
	WARN_ON_ONCE(!empty && rcu_segcblist_n_segment_cbs(&rdp->cblist) == 0);

	rcu_nocb_unlock_irqrestore(rdp, flags);

	/* Re-invoke RCU core processing if there are callbacks remaining. */
	if (!offloaded && rcu_segcblist_ready_cbs(&rdp->cblist))
		invoke_rcu_core();
	tick_dep_clear_task(current, TICK_DEP_BIT_RCU);
}
```

**External callers of `rcu_do_batch` at parent**

- `include/trace/events/rcu.h:598` — ` * Tracepoint for marking the beginning rcu_do_batch, performed to start`
- `include/trace/events/rcu.h:711` — ` * Tracepoint for exiting rcu_do_batch after RCU callbacks have been`
- `kernel/rcu/tree_nocb.h:802` — `	rcu_do_batch(rdp);`

### Function `__call_rcu_core` (L2868–L2908, 41 lines)

```c
static void __call_rcu_core(struct rcu_data *rdp, struct rcu_head *head,
			    unsigned long flags)
{
	/*
	 * If called from an extended quiescent state, invoke the RCU
	 * core in order to force a re-evaluation of RCU's idleness.
	 */
	if (!rcu_is_watching())
		invoke_rcu_core();

	/* If interrupts were disabled or CPU offline, don't invoke RCU core. */
	if (irqs_disabled_flags(flags) || cpu_is_offline(smp_processor_id()))
		return;

	/*
	 * Force the grace period if too many callbacks or too long waiting.
	 * Enforce hysteresis, and don't invoke rcu_force_quiescent_state()
	 * if some other CPU has recently done so.  Also, don't bother
	 * invoking rcu_force_quiescent_state() if the newly enqueued callback
	 * is the only one waiting for a grace period to complete.
	 */
	if (unlikely(rcu_segcblist_n_cbs(&rdp->cblist) >
		     rdp->qlen_last_fqs_check + qhimark)) {

		/* Are we ignoring a completed grace period? */
		note_gp_changes(rdp);

		/* Start a new grace period if one not already started. */
		if (!rcu_gp_in_progress()) {
			rcu_accelerate_cbs_unlocked(rdp->mynode, rdp);
		} else {
			/* Give the grace period a kick. */
			rdp->blimit = DEFAULT_MAX_RCU_BLIMIT;
			if (rcu_state.n_force_qs == rdp->n_force_qs_snap &&
			    rcu_segcblist_first_pend_cb(&rdp->cblist) != head)
				rcu_force_quiescent_state();
			rdp->n_force_qs_snap = rcu_state.n_force_qs;
			rdp->qlen_last_fqs_check = rcu_segcblist_n_cbs(&rdp->cblist);
		}
	}
}
```

_(no external call sites found for `__call_rcu_core`)_

### Function `rcutree_prepare_cpu` (L4122–L4166, 45 lines)

```c
int rcutree_prepare_cpu(unsigned int cpu)
{
	unsigned long flags;
	struct rcu_data *rdp = per_cpu_ptr(&rcu_data, cpu);
	struct rcu_node *rnp = rcu_get_root();

	/* Set up local state, ensuring consistent view of global state. */
	raw_spin_lock_irqsave_rcu_node(rnp, flags);
	rdp->qlen_last_fqs_check = 0;
	rdp->n_force_qs_snap = rcu_state.n_force_qs;
	rdp->blimit = blimit;
	rdp->dynticks_nesting = 1;	/* CPU not up, no tearing. */
	rcu_dynticks_eqs_online();
	raw_spin_unlock_rcu_node(rnp);		/* irqs remain disabled. */

	/*
	 * Only non-NOCB CPUs that didn't have early-boot callbacks need to be
	 * (re-)initialized.
	 */
	if (!rcu_segcblist_is_enabled(&rdp->cblist))
		rcu_segcblist_init(&rdp->cblist);  /* Re-enable callbacks. */

	/*
	 * Add CPU to leaf rcu_node pending-online bitmask.  Any needed
	 * propagation up the rcu_node tree will happen at the beginning
	 * of the next grace period.
	 */
	rnp = rdp->mynode;
	raw_spin_lock_rcu_node(rnp);		/* irqs already disabled. */
	rdp->beenonline = true;	 /* We have now been online. */
	rdp->gp_seq = READ_ONCE(rnp->gp_seq);
	rdp->gp_seq_needed = rdp->gp_seq;
	rdp->cpu_no_qs.b.norm = true;
	rdp->core_needs_qs = false;
	rdp->rcu_iw_pending = false;
	rdp->rcu_iw = IRQ_WORK_INIT_HARD(rcu_iw_handler);
	rdp->rcu_iw_gp_seq = rdp->gp_seq - 1;
	trace_rcu_grace_period(rcu_state.name, rdp->gp_seq, TPS("cpuonl"));
	raw_spin_unlock_irqrestore_rcu_node(rnp, flags);
	rcu_spawn_one_boost_kthread(rnp);
	rcu_spawn_cpu_nocb_kthread(cpu);
	WRITE_ONCE(rcu_state.n_online_cpus, rcu_state.n_online_cpus + 1);

	return 0;
}
```

**External callers of `rcutree_prepare_cpu` at parent**

- `include/linux/rcutiny.h:111` — `#define rcutree_prepare_cpu      NULL`
- `include/linux/rcutree.h:74` — `int rcutree_prepare_cpu(unsigned int cpu);`
- `kernel/cpu.c:1682` — `		.startup.single		= rcutree_prepare_cpu,`
