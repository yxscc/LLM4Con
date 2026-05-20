# Evidence: SYZBOT-3e77fd302e99f5af  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in io_submit_sqes / io_uring_show_fdinfo
- mainline fix commit: `f024d3a8ded0d8d2129ae123d7a5305c29ca44ce`
- candidate JSON: `A_SYZBOT-3e77fd302e99f5af.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "io_submit_sqes", "fn_b": "io_uring_show_fdinfo"}`

## Commit message

```
f024d3a8ded0d8d2129ae123d7a5305c29ca44ce
io_uring/fdinfo: annotate racy sq/cq head/tail reads

syzbot complains about the cached sq head read, and it's totally right.
But we don't need to care, it's just reading fdinfo, and reading the
CQ or SQ tail/head entries are known racy in that they are just a view
into that very instant and may of course be outdated by the time they
are reported.

Annotate both the SQ head and CQ tail read with data_race() to avoid
this syzbot complaint.

Link: https://lore.kernel.org/io-uring/6811f6dc.050a0220.39e3a1.0d0e.GAE@google.com/
Reported-by: syzbot+3e77fd302e99f5af9394@syzkaller.appspotmail.com
Signed-off-by: Jens Axboe <axboe@kernel.dk>
```

## CVE / bug description

```
KCSAN: data-race in io_submit_sqes / io_uring_show_fdinfo
```

## Full mainline patch

```diff
commit f024d3a8ded0d8d2129ae123d7a5305c29ca44ce
Author: Jens Axboe <axboe@kernel.dk>
Date:   Wed Apr 30 07:17:17 2025 -0600

    io_uring/fdinfo: annotate racy sq/cq head/tail reads
    
    syzbot complains about the cached sq head read, and it's totally right.
    But we don't need to care, it's just reading fdinfo, and reading the
    CQ or SQ tail/head entries are known racy in that they are just a view
    into that very instant and may of course be outdated by the time they
    are reported.
    
    Annotate both the SQ head and CQ tail read with data_race() to avoid
    this syzbot complaint.
    
    Link: https://lore.kernel.org/io-uring/6811f6dc.050a0220.39e3a1.0d0e.GAE@google.com/
    Reported-by: syzbot+3e77fd302e99f5af9394@syzkaller.appspotmail.com
    Signed-off-by: Jens Axboe <axboe@kernel.dk>

diff --git a/io_uring/fdinfo.c b/io_uring/fdinfo.c
index f60d0a9d505e..9414ca6d101c 100644
--- a/io_uring/fdinfo.c
+++ b/io_uring/fdinfo.c
@@ -123,11 +123,11 @@ __cold void io_uring_show_fdinfo(struct seq_file *m, struct file *file)
 	seq_printf(m, "SqMask:\t0x%x\n", sq_mask);
 	seq_printf(m, "SqHead:\t%u\n", sq_head);
 	seq_printf(m, "SqTail:\t%u\n", sq_tail);
-	seq_printf(m, "CachedSqHead:\t%u\n", ctx->cached_sq_head);
+	seq_printf(m, "CachedSqHead:\t%u\n", data_race(ctx->cached_sq_head));
 	seq_printf(m, "CqMask:\t0x%x\n", cq_mask);
 	seq_printf(m, "CqHead:\t%u\n", cq_head);
 	seq_printf(m, "CqTail:\t%u\n", cq_tail);
-	seq_printf(m, "CachedCqTail:\t%u\n", ctx->cached_cq_tail);
+	seq_printf(m, "CachedCqTail:\t%u\n", data_race(ctx->cached_cq_tail));
 	seq_printf(m, "SQEs:\t%u\n", sq_tail - sq_head);
 	sq_entries = min(sq_tail - sq_head, ctx->sq_entries);
 	for (i = 0; i < sq_entries; i++) {

```

## File: `io_uring/fdinfo.c`

- changed old-lines: [126, 130]

### Function `io_uring_show_fdinfo` (L93–L264, 172 lines)

```c
__cold void io_uring_show_fdinfo(struct seq_file *m, struct file *file)
{
	struct io_ring_ctx *ctx = file->private_data;
	struct io_overflow_cqe *ocqe;
	struct io_rings *r = ctx->rings;
	struct rusage sq_usage;
	unsigned int sq_mask = ctx->sq_entries - 1, cq_mask = ctx->cq_entries - 1;
	unsigned int sq_head = READ_ONCE(r->sq.head);
	unsigned int sq_tail = READ_ONCE(r->sq.tail);
	unsigned int cq_head = READ_ONCE(r->cq.head);
	unsigned int cq_tail = READ_ONCE(r->cq.tail);
	unsigned int cq_shift = 0;
	unsigned int sq_shift = 0;
	unsigned int sq_entries, cq_entries;
	int sq_pid = -1, sq_cpu = -1;
	u64 sq_total_time = 0, sq_work_time = 0;
	bool has_lock;
	unsigned int i;

	if (ctx->flags & IORING_SETUP_CQE32)
		cq_shift = 1;
	if (ctx->flags & IORING_SETUP_SQE128)
		sq_shift = 1;

	/*
	 * we may get imprecise sqe and cqe info if uring is actively running
	 * since we get cached_sq_head and cached_cq_tail without uring_lock
	 * and sq_tail and cq_head are changed by userspace. But it's ok since
	 * we usually use these info when it is stuck.
	 */
	seq_printf(m, "SqMask:\t0x%x\n", sq_mask);
	seq_printf(m, "SqHead:\t%u\n", sq_head);
	seq_printf(m, "SqTail:\t%u\n", sq_tail);
	seq_printf(m, "CachedSqHead:\t%u\n", ctx->cached_sq_head);
	seq_printf(m, "CqMask:\t0x%x\n", cq_mask);
	seq_printf(m, "CqHead:\t%u\n", cq_head);
	seq_printf(m, "CqTail:\t%u\n", cq_tail);
	seq_printf(m, "CachedCqTail:\t%u\n", ctx->cached_cq_tail);
	seq_printf(m, "SQEs:\t%u\n", sq_tail - sq_head);
	sq_entries = min(sq_tail - sq_head, ctx->sq_entries);
	for (i = 0; i < sq_entries; i++) {
		unsigned int entry = i + sq_head;
		struct io_uring_sqe *sqe;
		unsigned int sq_idx;

		if (ctx->flags & IORING_SETUP_NO_SQARRAY)
			break;
		sq_idx = READ_ONCE(ctx->sq_array[entry & sq_mask]);
		if (sq_idx > sq_mask)
			continue;
		sqe = &ctx->sq_sqes[sq_idx << sq_shift];
		seq_printf(m, "%5u: opcode:%s, fd:%d, flags:%x, off:%llu, "
			      "addr:0x%llx, rw_flags:0x%x, buf_index:%d "
			      "user_data:%llu",
			   sq_idx, io_uring_get_opcode(sqe->opcode), sqe->fd,
			   sqe->flags, (unsigned long long) sqe->off,
			   (unsigned long long) sqe->addr, sqe->rw_flags,
			   sqe->buf_index, sqe->user_data);
		if (sq_shift) {
			u64 *sqeb = (void *) (sqe + 1);
			int size = sizeof(struct io_uring_sqe) / sizeof(u64);
			int j;

			for (j = 0; j < size; j++) {
				seq_printf(m, ", e%d:0x%llx", j,
						(unsigned long long) *sqeb);
				sqeb++;
			}
		}
		seq_printf(m, "\n");
	}
	seq_printf(m, "CQEs:\t%u\n", cq_tail - cq_head);
	cq_entries = min(cq_tail - cq_head, ctx->cq_entries);
	for (i = 0; i < cq_entries; i++) {
		unsigned int entry = i + cq_head;
		struct io_uring_cqe *cqe = &r->cqes[(entry & cq_mask) << cq_shift];

		seq_printf(m, "%5u: user_data:%llu, res:%d, flag:%x",
			   entry & cq_mask, cqe->user_data, cqe->res,
			   cqe->flags);
		if (cq_shift)
			seq_printf(m, ", extra1:%llu, extra2:%llu\n",
					cqe->big_cqe[0], cqe->big_cqe[1]);
		seq_printf(m, "\n");
	}

	/*
	 * Avoid ABBA deadlock between the seq lock and the io_uring mutex,
	 * since fdinfo case grabs it in the opposite direction of normal use
	 * cases. If we fail to get the lock, we just don't iterate any
	 * structures that could be going away outside the io_uring mutex.
	 */
	has_lock = mutex_trylock(&ctx->uring_lock);

	if (has_lock && (ctx->flags & IORING_SETUP_SQPOLL)) {
		struct io_sq_data *sq = ctx->sq_data;

		/*
		 * sq->thread might be NULL if we raced with the sqpoll
		 * thread termination.
		 */
		if (sq->thread) {
			sq_pid = sq->task_pid;
			sq_cpu = sq->sq_cpu;
			getrusage(sq->thread, RUSAGE_SELF, &sq_usage);
			sq_total_time = (sq_usage.ru_stime.tv_sec * 1000000
					 + sq_usage.ru_stime.tv_usec);
			sq_work_time = sq->work_time;
		}
	}

	seq_printf(m, "SqThread:\t%d\n", sq_pid);
	seq_printf(m, "SqThreadCpu:\t%d\n", sq_cpu);
	seq_printf(m, "SqTotalTime:\t%llu\n", sq_total_time);
	seq_printf(m, "SqWorkTime:\t%llu\n", sq_work_time);
	seq_printf(m, "UserFiles:\t%u\n", ctx->file_table.data.nr);
	for (i = 0; has_lock && i < ctx->file_table.data.nr; i++) {
		struct file *f = NULL;

		if (ctx->file_table.data.nodes[i])
			f = io_slot_file(ctx->file_table.data.nodes[i]);
		if (f) {
			seq_printf(m, "%5u: ", i);
			seq_file_path(m, f, " \t\n\\");
			seq_puts(m, "\n");
		}
	}
	seq_printf(m, "UserBufs:\t%u\n", ctx->buf_table.nr);
	for (i = 0; has_lock && i < ctx->buf_table.nr; i++) {
		struct io_mapped_ubuf *buf = NULL;

		if (ctx->buf_table.nodes[i])
			buf = ctx->buf_table.nodes[i]->buf;
		if (buf)
			seq_printf(m, "%5u: 0x%llx/%u\n", i, buf->ubuf, buf->len);
		else
			seq_printf(m, "%5u: <none>\n", i);
	}
	if (has_lock && !xa_empty(&ctx->personalities)) {
		unsigned long index;
		const struct cred *cred;

		seq_printf(m, "Personalities:\n");
		xa_for_each(&ctx->personalities, index, cred)
			io_uring_show_cred(m, index, cred);
	}

	seq_puts(m, "PollList:\n");
	for (i = 0; has_lock && i < (1U << ctx->cancel_table.hash_bits); i++) {
		struct io_hash_bucket *hb = &ctx->cancel_table.hbs[i];
		struct io_kiocb *req;

		hlist_for_each_entry(req, &hb->list, hash_node)
			seq_printf(m, "  op=%d, task_works=%d\n", req->opcode,
					task_work_pending(req->tctx->task));
	}

	if (has_lock)
		mutex_unlock(&ctx->uring_lock);

	seq_puts(m, "CqOverflowList:\n");
	spin_lock(&ctx->completion_lock);
	list_for_each_entry(ocqe, &ctx->cq_overflow_list, list) {
		struct io_uring_cqe *cqe = &ocqe->cqe;

		seq_printf(m, "  user_data=%llu, res=%d, flags=%x\n",
			   cqe->user_data, cqe->res, cqe->flags);

	}
	spin_unlock(&ctx->completion_lock);
	napi_show_fdinfo(ctx, m);
}
```

**External callers of `io_uring_show_fdinfo` at parent**

- `io_uring/fdinfo.h:3` — `void io_uring_show_fdinfo(struct seq_file *m, struct file *f);`
- `io_uring/io_uring.c:3479` — `	.show_fdinfo	= io_uring_show_fdinfo,`
