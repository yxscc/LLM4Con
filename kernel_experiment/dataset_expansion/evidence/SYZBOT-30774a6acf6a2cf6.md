# Evidence: SYZBOT-30774a6acf6a2cf6  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in start_this_handle / start_this_handle
- mainline fix commit: `3b1833e92baba135923af4a07e73fe6e54be5a2f`
- candidate JSON: `A_SYZBOT-30774a6acf6a2cf6.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "start_this_handle", "fn_b": "start_this_handle"}`

## Commit message

```
3b1833e92baba135923af4a07e73fe6e54be5a2f
ext4: annotate data race in start_this_handle()

Access to journal->j_running_transaction is not protected by appropriate
lock and thus is racy. We are well aware of that and the code handles
the race properly. Just add a comment and data_race() annotation.

Cc: stable@kernel.org
Reported-by: syzbot+30774a6acf6a2cf6d535@syzkaller.appspotmail.com
Signed-off-by: Jan Kara <jack@suse.cz>
Link: https://lore.kernel.org/r/20210406161804.20150-1-jack@suse.cz
Signed-off-by: Theodore Ts'o <tytso@mit.edu>
```

## CVE / bug description

```
KCSAN: data-race in start_this_handle / start_this_handle
```

## Full mainline patch

```diff
commit 3b1833e92baba135923af4a07e73fe6e54be5a2f
Author: Jan Kara <jack@suse.cz>
Date:   Tue Apr 6 18:17:59 2021 +0200

    ext4: annotate data race in start_this_handle()
    
    Access to journal->j_running_transaction is not protected by appropriate
    lock and thus is racy. We are well aware of that and the code handles
    the race properly. Just add a comment and data_race() annotation.
    
    Cc: stable@kernel.org
    Reported-by: syzbot+30774a6acf6a2cf6d535@syzkaller.appspotmail.com
    Signed-off-by: Jan Kara <jack@suse.cz>
    Link: https://lore.kernel.org/r/20210406161804.20150-1-jack@suse.cz
    Signed-off-by: Theodore Ts'o <tytso@mit.edu>

diff --git a/fs/jbd2/transaction.c b/fs/jbd2/transaction.c
index 9396666b7314..398d1d9209e2 100644
--- a/fs/jbd2/transaction.c
+++ b/fs/jbd2/transaction.c
@@ -349,7 +349,12 @@ static int start_this_handle(journal_t *journal, handle_t *handle,
 	}
 
 alloc_transaction:
-	if (!journal->j_running_transaction) {
+	/*
+	 * This check is racy but it is just an optimization of allocating new
+	 * transaction early if there are high chances we'll need it. If we
+	 * guess wrong, we'll retry or free unused transaction.
+	 */
+	if (!data_race(journal->j_running_transaction)) {
 		/*
 		 * If __GFP_FS is not present, then we may be being called from
 		 * inside the fs writeback layer, so we MUST NOT fail.

```

## File: `fs/jbd2/transaction.c`

- changed old-lines: [352]

### Function `start_this_handle` (L325–L454, 130 lines)

```c
static int start_this_handle(journal_t *journal, handle_t *handle,
			     gfp_t gfp_mask)
{
	transaction_t	*transaction, *new_transaction = NULL;
	int		blocks = handle->h_total_credits;
	int		rsv_blocks = 0;
	unsigned long ts = jiffies;

	if (handle->h_rsv_handle)
		rsv_blocks = handle->h_rsv_handle->h_total_credits;

	/*
	 * Limit the number of reserved credits to 1/2 of maximum transaction
	 * size and limit the number of total credits to not exceed maximum
	 * transaction size per operation.
	 */
	if ((rsv_blocks > journal->j_max_transaction_buffers / 2) ||
	    (rsv_blocks + blocks > journal->j_max_transaction_buffers)) {
		printk(KERN_ERR "JBD2: %s wants too many credits "
		       "credits:%d rsv_credits:%d max:%d\n",
		       current->comm, blocks, rsv_blocks,
		       journal->j_max_transaction_buffers);
		WARN_ON(1);
		return -ENOSPC;
	}

alloc_transaction:
	if (!journal->j_running_transaction) {
		/*
		 * If __GFP_FS is not present, then we may be being called from
		 * inside the fs writeback layer, so we MUST NOT fail.
		 */
		if ((gfp_mask & __GFP_FS) == 0)
			gfp_mask |= __GFP_NOFAIL;
		new_transaction = kmem_cache_zalloc(transaction_cache,
						    gfp_mask);
		if (!new_transaction)
			return -ENOMEM;
	}

	jbd_debug(3, "New handle %p going live.\n", handle);

	/*
	 * We need to hold j_state_lock until t_updates has been incremented,
	 * for proper journal barrier handling
	 */
repeat:
	read_lock(&journal->j_state_lock);
	BUG_ON(journal->j_flags & JBD2_UNMOUNT);
	if (is_journal_aborted(journal) ||
	    (journal->j_errno != 0 && !(journal->j_flags & JBD2_ACK_ERR))) {
		read_unlock(&journal->j_state_lock);
		jbd2_journal_free_transaction(new_transaction);
		return -EROFS;
	}

	/*
	 * Wait on the journal's transaction barrier if necessary. Specifically
	 * we allow reserved handles to proceed because otherwise commit could
	 * deadlock on page writeback not being able to complete.
	 */
	if (!handle->h_reserved && journal->j_barrier_count) {
		read_unlock(&journal->j_state_lock);
		wait_event(journal->j_wait_transaction_locked,
				journal->j_barrier_count == 0);
		goto repeat;
	}

	if (!journal->j_running_transaction) {
		read_unlock(&journal->j_state_lock);
		if (!new_transaction)
			goto alloc_transaction;
		write_lock(&journal->j_state_lock);
		if (!journal->j_running_transaction &&
		    (handle->h_reserved || !journal->j_barrier_count)) {
			jbd2_get_transaction(journal, new_transaction);
			new_transaction = NULL;
		}
		write_unlock(&journal->j_state_lock);
		goto repeat;
	}

	transaction = journal->j_running_transaction;

	if (!handle->h_reserved) {
		/* We may have dropped j_state_lock - restart in that case */
		if (add_transaction_credits(journal, blocks, rsv_blocks))
			goto repeat;
	} else {
		/*
		 * We have handle reserved so we are allowed to join T_LOCKED
		 * transaction and we don't have to check for transaction size
		 * and journal space. But we still have to wait while running
		 * transaction is being switched to a committing one as it
		 * won't wait for any handles anymore.
		 */
		if (transaction->t_state == T_SWITCH) {
			wait_transaction_switching(journal);
			goto repeat;
		}
		sub_reserved_credits(journal, blocks);
		handle->h_reserved = 0;
	}

	/* OK, account for the buffers that this operation expects to
	 * use and add the handle to the running transaction. 
	 */
	update_t_max_wait(transaction, ts);
	handle->h_transaction = transaction;
	handle->h_requested_credits = blocks;
	handle->h_revoke_credits_requested = handle->h_revoke_credits;
	handle->h_start_jiffies = jiffies;
	atomic_inc(&transaction->t_updates);
	atomic_inc(&transaction->t_handle_count);
	jbd_debug(4, "Handle %p given %d credits (total %d, free %lu)\n",
		  handle, blocks,
		  atomic_read(&transaction->t_outstanding_credits),
		  jbd2_log_space_left(journal));
	read_unlock(&journal->j_state_lock);
	current->journal_info = handle;

	rwsem_acquire_read(&journal->j_trans_commit_map, 0, 0, _THIS_IP_);
	jbd2_journal_free_transaction(new_transaction);
	/*
	 * Ensure that no allocations done while the transaction is open are
	 * going to recurse back to the fs layer.
	 */
	handle->saved_alloc_context = memalloc_nofs_save();
	return 0;
}
```

**External callers of `start_this_handle` at parent**

- `fs/jbd2/commit.c:693` — `		 * start_this_handle() uses t_outstanding_credits to determine`
