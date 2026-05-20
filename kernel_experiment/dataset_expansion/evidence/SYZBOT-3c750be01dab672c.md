# Evidence: SYZBOT-3c750be01dab672c  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in io_sq_thread / io_uring_try_cancel_requests (12)
- mainline fix commit: `a13030fd194c88961be4679f87a1380f1bda0ebe`
- candidate JSON: `A_SYZBOT-3c750be01dab672c.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "io_sq_thread", "fn_b": "io_uring_try_cancel_requests"}`

## Commit message

```
a13030fd194c88961be4679f87a1380f1bda0ebe
io_uring: simplify the SQPOLL thread check when cancelling requests

In io_uring_try_cancel_requests, we check whether sq_data->thread ==
current to determine if the function is called by the SQPOLL thread to do
iopoll when IORING_SETUP_SQPOLL is set. This check can race with the SQPOLL
thread termination.

io_uring_cancel_generic is used in 2 places: io_uring_cancel_generic and
io_ring_exit_work. In io_uring_cancel_generic, we have the information
whether the current is SQPOLL thread already. And the SQPOLL thread never
reaches io_ring_exit_work.

So to avoid the racy check, this commit adds a boolean flag to
io_uring_try_cancel_requests to determine if the caller is SQPOLL thread.

Reported-by: syzbot+3c750be01dab672c513d@syzkaller.appspotmail.com
Reported-by: Li Zetao <lizetao1@huawei.com>
Reviewed-by: Li Zetao <lizetao1@huawei.com>
Signed-off-by: Bui Quang Minh <minhquangbui99@gmail.com>
Reviewed-by: Pavel Begunkov <asml.silence@gmail.com>
Link: https://lore.kernel.org/r/20250113160331.44057-1-minhquangbui99@gmail.com
Signed-off-by: Jens Axboe <axboe@kernel.dk>
```

## CVE / bug description

```
KCSAN: data-race in io_sq_thread / io_uring_try_cancel_requests (12)
```

## Full mainline patch

```diff
commit a13030fd194c88961be4679f87a1380f1bda0ebe
Author: Bui Quang Minh <minhquangbui99@gmail.com>
Date:   Mon Jan 13 23:03:31 2025 +0700

    io_uring: simplify the SQPOLL thread check when cancelling requests
    
    In io_uring_try_cancel_requests, we check whether sq_data->thread ==
    current to determine if the function is called by the SQPOLL thread to do
    iopoll when IORING_SETUP_SQPOLL is set. This check can race with the SQPOLL
    thread termination.
    
    io_uring_cancel_generic is used in 2 places: io_uring_cancel_generic and
    io_ring_exit_work. In io_uring_cancel_generic, we have the information
    whether the current is SQPOLL thread already. And the SQPOLL thread never
    reaches io_ring_exit_work.
    
    So to avoid the racy check, this commit adds a boolean flag to
    io_uring_try_cancel_requests to determine if the caller is SQPOLL thread.
    
    Reported-by: syzbot+3c750be01dab672c513d@syzkaller.appspotmail.com
    Reported-by: Li Zetao <lizetao1@huawei.com>
    Reviewed-by: Li Zetao <lizetao1@huawei.com>
    Signed-off-by: Bui Quang Minh <minhquangbui99@gmail.com>
    Reviewed-by: Pavel Begunkov <asml.silence@gmail.com>
    Link: https://lore.kernel.org/r/20250113160331.44057-1-minhquangbui99@gmail.com
    Signed-off-by: Jens Axboe <axboe@kernel.dk>

diff --git a/io_uring/io_uring.c b/io_uring/io_uring.c
index af03e9973b58..20a46bc671ea 100644
--- a/io_uring/io_uring.c
+++ b/io_uring/io_uring.c
@@ -143,7 +143,8 @@ struct io_defer_entry {
 
 static bool io_uring_try_cancel_requests(struct io_ring_ctx *ctx,
 					 struct io_uring_task *tctx,
-					 bool cancel_all);
+					 bool cancel_all,
+					 bool is_sqpoll_thread);
 
 static void io_queue_sqe(struct io_kiocb *req);
 
@@ -2869,7 +2870,8 @@ static __cold void io_ring_exit_work(struct work_struct *work)
 		if (ctx->flags & IORING_SETUP_DEFER_TASKRUN)
 			io_move_task_work_from_local(ctx);
 
-		while (io_uring_try_cancel_requests(ctx, NULL, true))
+		/* The SQPOLL thread never reaches this path */
+		while (io_uring_try_cancel_requests(ctx, NULL, true, false))
 			cond_resched();
 
 		if (ctx->sq_data) {
@@ -3037,7 +3039,8 @@ static __cold bool io_uring_try_cancel_iowq(struct io_ring_ctx *ctx)
 
 static __cold bool io_uring_try_cancel_requests(struct io_ring_ctx *ctx,
 						struct io_uring_task *tctx,
-						bool cancel_all)
+						bool cancel_all,
+						bool is_sqpoll_thread)
 {
 	struct io_task_cancel cancel = { .tctx = tctx, .all = cancel_all, };
 	enum io_wq_cancel cret;
@@ -3067,7 +3070,7 @@ static __cold bool io_uring_try_cancel_requests(struct io_ring_ctx *ctx,
 
 	/* SQPOLL thread does its own polling */
 	if ((!(ctx->flags & IORING_SETUP_SQPOLL) && cancel_all) ||
-	    (ctx->sq_data && ctx->sq_data->thread == current)) {
+	    is_sqpoll_thread) {
 		while (!wq_list_empty(&ctx->iopoll_list)) {
 			io_iopoll_try_reap_events(ctx);
 			ret = true;
@@ -3140,13 +3143,15 @@ __cold void io_uring_cancel_generic(bool cancel_all, struct io_sq_data *sqd)
 					continue;
 				loop |= io_uring_try_cancel_requests(node->ctx,
 							current->io_uring,
-							cancel_all);
+							cancel_all,
+							false);
 			}
 		} else {
 			list_for_each_entry(ctx, &sqd->ctx_list, sqd_list)
 				loop |= io_uring_try_cancel_requests(ctx,
 								     current->io_uring,
-								     cancel_all);
+								     cancel_all,
+								     true);
 		}
 
 		if (loop) {

```

## File: `io_uring/io_uring.c`

- changed old-lines: [146, 2872, 3040, 3070, 3143, 3149]

### Function `io_ring_exit_work` (L2847–L2940, 94 lines)

```c
static __cold void io_ring_exit_work(struct work_struct *work)
{
	struct io_ring_ctx *ctx = container_of(work, struct io_ring_ctx, exit_work);
	unsigned long timeout = jiffies + HZ * 60 * 5;
	unsigned long interval = HZ / 20;
	struct io_tctx_exit exit;
	struct io_tctx_node *node;
	int ret;

	/*
	 * If we're doing polled IO and end up having requests being
	 * submitted async (out-of-line), then completions can come in while
	 * we're waiting for refs to drop. We need to reap these manually,
	 * as nobody else will be looking for them.
	 */
	do {
		if (test_bit(IO_CHECK_CQ_OVERFLOW_BIT, &ctx->check_cq)) {
			mutex_lock(&ctx->uring_lock);
			io_cqring_overflow_kill(ctx);
			mutex_unlock(&ctx->uring_lock);
		}

		if (ctx->flags & IORING_SETUP_DEFER_TASKRUN)
			io_move_task_work_from_local(ctx);

		while (io_uring_try_cancel_requests(ctx, NULL, true))
			cond_resched();

		if (ctx->sq_data) {
			struct io_sq_data *sqd = ctx->sq_data;
			struct task_struct *tsk;

			io_sq_thread_park(sqd);
			tsk = sqd->thread;
			if (tsk && tsk->io_uring && tsk->io_uring->io_wq)
				io_wq_cancel_cb(tsk->io_uring->io_wq,
						io_cancel_ctx_cb, ctx, true);
			io_sq_thread_unpark(sqd);
		}

		io_req_caches_free(ctx);

		if (WARN_ON_ONCE(time_after(jiffies, timeout))) {
			/* there is little hope left, don't run it too often */
			interval = HZ * 60;
		}
		/*
		 * This is really an uninterruptible wait, as it has to be
		 * complete. But it's also run from a kworker, which doesn't
		 * take signals, so it's fine to make it interruptible. This
		 * avoids scenarios where we knowingly can wait much longer
		 * on completions, for example if someone does a SIGSTOP on
		 * a task that needs to finish task_work to make this loop
		 * complete. That's a synthetic situation that should not
		 * cause a stuck task backtrace, and hence a potential panic
		 * on stuck tasks if that is enabled.
		 */
	} while (!wait_for_completion_interruptible_timeout(&ctx->ref_comp, interval));

	init_completion(&exit.completion);
	init_task_work(&exit.task_work, io_tctx_exit_cb);
	exit.ctx = ctx;

	mutex_lock(&ctx->uring_lock);
	while (!list_empty(&ctx->tctx_list)) {
		WARN_ON_ONCE(time_after(jiffies, timeout));

		node = list_first_entry(&ctx->tctx_list, struct io_tctx_node,
					ctx_node);
		/* don't spin on a single task if cancellation failed */
		list_rotate_left(&ctx->tctx_list);
		ret = task_work_add(node->task, &exit.task_work, TWA_SIGNAL);
		if (WARN_ON_ONCE(ret))
			continue;

		mutex_unlock(&ctx->uring_lock);
		/*
		 * See comment above for
		 * wait_for_completion_interruptible_timeout() on why this
		 * wait is marked as interruptible.
		 */
		wait_for_completion_interruptible(&exit.completion);
		mutex_lock(&ctx->uring_lock);
	}
	mutex_unlock(&ctx->uring_lock);
	spin_lock(&ctx->completion_lock);
	spin_unlock(&ctx->completion_lock);

	/* pairs with RCU read section in io_req_local_work_add() */
	if (ctx->flags & IORING_SETUP_DEFER_TASKRUN)
		synchronize_rcu();

	io_ring_ctx_free(ctx);
}
```

**External callers of `io_ring_exit_work` at parent**

- `io_uring/sqpoll.c:185` — `		 * but also it is relied upon by io_ring_exit_work()`

### Function `io_uring_try_cancel_requests` (L3038–L3094, 57 lines)

```c
static __cold bool io_uring_try_cancel_requests(struct io_ring_ctx *ctx,
						struct io_uring_task *tctx,
						bool cancel_all)
{
	struct io_task_cancel cancel = { .tctx = tctx, .all = cancel_all, };
	enum io_wq_cancel cret;
	bool ret = false;

	/* set it so io_req_local_work_add() would wake us up */
	if (ctx->flags & IORING_SETUP_DEFER_TASKRUN) {
		atomic_set(&ctx->cq_wait_nr, 1);
		smp_mb();
	}

	/* failed during ring init, it couldn't have issued any requests */
	if (!ctx->rings)
		return false;

	if (!tctx) {
		ret |= io_uring_try_cancel_iowq(ctx);
	} else if (tctx->io_wq) {
		/*
		 * Cancels requests of all rings, not only @ctx, but
		 * it's fine as the task is in exit/exec.
		 */
		cret = io_wq_cancel_cb(tctx->io_wq, io_cancel_task_cb,
				       &cancel, true);
		ret |= (cret != IO_WQ_CANCEL_NOTFOUND);
	}

	/* SQPOLL thread does its own polling */
	if ((!(ctx->flags & IORING_SETUP_SQPOLL) && cancel_all) ||
	    (ctx->sq_data && ctx->sq_data->thread == current)) {
		while (!wq_list_empty(&ctx->iopoll_list)) {
			io_iopoll_try_reap_events(ctx);
			ret = true;
			cond_resched();
		}
	}

	if ((ctx->flags & IORING_SETUP_DEFER_TASKRUN) &&
	    io_allowed_defer_tw_run(ctx))
		ret |= io_run_local_work(ctx, INT_MAX, INT_MAX) > 0;
	ret |= io_cancel_defer_files(ctx, tctx, cancel_all);
	mutex_lock(&ctx->uring_lock);
	ret |= io_poll_remove_all(ctx, tctx, cancel_all);
	ret |= io_waitid_remove_all(ctx, tctx, cancel_all);
	ret |= io_futex_remove_all(ctx, tctx, cancel_all);
	ret |= io_uring_try_cancel_uring_cmd(ctx, tctx, cancel_all);
	mutex_unlock(&ctx->uring_lock);
	ret |= io_kill_timeouts(ctx, tctx, cancel_all);
	if (tctx)
		ret |= io_run_task_work() > 0;
	else
		ret |= flush_delayed_work(&ctx->fallback_work);
	return ret;
}
```

_(no external call sites found for `io_uring_try_cancel_requests`)_

### Function `io_uring_cancel_generic` (L3107–L3188, 82 lines)

```c
__cold void io_uring_cancel_generic(bool cancel_all, struct io_sq_data *sqd)
{
	struct io_uring_task *tctx = current->io_uring;
	struct io_ring_ctx *ctx;
	struct io_tctx_node *node;
	unsigned long index;
	s64 inflight;
	DEFINE_WAIT(wait);

	WARN_ON_ONCE(sqd && sqd->thread != current);

	if (!current->io_uring)
		return;
	if (tctx->io_wq)
		io_wq_exit_start(tctx->io_wq);

	atomic_inc(&tctx->in_cancel);
	do {
		bool loop = false;

		io_uring_drop_tctx_refs(current);
		if (!tctx_inflight(tctx, !cancel_all))
			break;

		/* read completions before cancelations */
		inflight = tctx_inflight(tctx, false);
		if (!inflight)
			break;

		if (!sqd) {
			xa_for_each(&tctx->xa, index, node) {
				/* sqpoll task will cancel all its requests */
				if (node->ctx->sq_data)
					continue;
				loop |= io_uring_try_cancel_requests(node->ctx,
							current->io_uring,
							cancel_all);
			}
		} else {
			list_for_each_entry(ctx, &sqd->ctx_list, sqd_list)
				loop |= io_uring_try_cancel_requests(ctx,
								     current->io_uring,
								     cancel_all);
		}

		if (loop) {
			cond_resched();
			continue;
		}

		prepare_to_wait(&tctx->wait, &wait, TASK_INTERRUPTIBLE);
		io_run_task_work();
		io_uring_drop_tctx_refs(current);
		xa_for_each(&tctx->xa, index, node) {
			if (io_local_work_pending(node->ctx)) {
				WARN_ON_ONCE(node->ctx->submitter_task &&
					     node->ctx->submitter_task != current);
				goto end_wait;
			}
		}
		/*
		 * If we've seen completions, retry without waiting. This
		 * avoids a race where a completion comes in before we did
		 * prepare_to_wait().
		 */
		if (inflight == tctx_inflight(tctx, !cancel_all))
			schedule();
end_wait:
		finish_wait(&tctx->wait, &wait);
	} while (1);

	io_uring_clean_tctx(tctx);
	if (cancel_all) {
		/*
		 * We shouldn't run task_works after cancel, so just leave
		 * ->in_cancel set for normal exit.
		 */
		atomic_dec(&tctx->in_cancel);
		/* for exec all current's requests should be gone, kill tctx */
		__io_uring_free(current);
	}
}
```

**External callers of `io_uring_cancel_generic` at parent**

- `io_uring/io_uring.h:99` — `__cold void io_uring_cancel_generic(bool cancel_all, struct io_sq_data *sqd);`
- `io_uring/sqpoll.c:377` — `	io_uring_cancel_generic(true, sqd);`
