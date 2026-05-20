# Evidence: SYZBOT-52e3dbded1f71729  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in kcm_rfree / unreserve_rx_kcm (3)
- mainline fix commit: `15e4dabda11b0fa31d510a915d1a580f47dfc92e`
- candidate JSON: `A_SYZBOT-52e3dbded1f71729.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "kcm_rfree", "fn_b": "unreserve_rx_kcm"}`

## Commit message

```
15e4dabda11b0fa31d510a915d1a580f47dfc92e
kcm: annotate data-races around kcm->rx_psock

kcm->rx_psock can be read locklessly in kcm_rfree().
Annotate the read and writes accordingly.

We do the same for kcm->rx_wait in the following patch.

syzbot reported:
BUG: KCSAN: data-race in kcm_rfree / unreserve_rx_kcm

write to 0xffff888123d827b8 of 8 bytes by task 2758 on cpu 1:
unreserve_rx_kcm+0x72/0x1f0 net/kcm/kcmsock.c:313
kcm_rcv_strparser+0x2b5/0x3a0 net/kcm/kcmsock.c:373
__strp_recv+0x64c/0xd20 net/strparser/strparser.c:301
strp_recv+0x6d/0x80 net/strparser/strparser.c:335
tcp_read_sock+0x13e/0x5a0 net/ipv4/tcp.c:1703
strp_read_sock net/strparser/strparser.c:358 [inline]
do_strp_work net/strparser/strparser.c:406 [inline]
strp_work+0xe8/0x180 net/strparser/strparser.c:415
process_one_work+0x3d3/0x720 kernel/workqueue.c:2289
worker_thread+0x618/0xa70 kernel/workqueue.c:2436
kthread+0x1a9/0x1e0 kernel/kthread.c:376
ret_from_fork+0x1f/0x30 arch/x86/entry/entry_64.S:306

read to 0xffff888123d827b8 of 8 bytes by task 5859 on cpu 0:
kcm_rfree+0x14c/0x220 net/kcm/kcmsock.c:181
skb_release_head_state+0x8e/0x160 net/core/skbuff.c:841
skb_release_all net/core/skbuff.c:852 [inline]
__kfree_skb net/core/skbuff.c:868 [inline]
kfree_skb_reason+0x5c/0x260 net/core/skbuff.c:891
kfree_skb include/linux/skbuff.h:1216 [inline]
kcm_recvmsg+0x226/0x2b0 net/kcm/kcmsock.c:1161
____sys_recvmsg+0x16c/0x2e0
___sys_recvmsg net/socket.c:2743 [inline]
do_recvmmsg+0x2f1/0x710 net/socket.c:2837
__sys_recvmmsg net/socket.c:2916 [inline]
__do_sys_recvmmsg net/socket.c:2939 [inline]
__se_sys_recvmmsg net/socket.c:2932 [inline]
__x64_sys_recvmmsg+0xde/0x160 net/socket.c:2932
do_syscall_x64 arch/x86/entry/common.c:50 [inline]
do_syscall_64+0x2b/0x70 arch/x86/entry/common.c:80
entry_SYSCALL_64_after_hwframe+0x63/0xcd

value changed: 0xffff88812971ce00 -> 0x0000000000000000

Reported by Kernel Concurrency Sanitizer on:
CPU: 0 PID: 5859 Comm: syz-executor.3 Not tainted 6.0.0-syzkaller-12189-g19d17ab7c68b-dirty #0
Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 09/22/2022

Fixes: ab7ac4eb9832 ("kcm: Kernel Connection Multiplexor module")
Reported-by: syzbot <syzkaller@googlegroups.com>
Signed-off-by: Eric Dumazet <edumazet@google.com>
Signed-off-by: David S. Miller <davem@davemloft.net>
```

## CVE / bug description

```
KCSAN: data-race in kcm_rfree / unreserve_rx_kcm (3)
```

## Full mainline patch

```diff
commit 15e4dabda11b0fa31d510a915d1a580f47dfc92e
Author: Eric Dumazet <edumazet@google.com>
Date:   Thu Oct 20 22:45:11 2022 +0000

    kcm: annotate data-races around kcm->rx_psock
    
    kcm->rx_psock can be read locklessly in kcm_rfree().
    Annotate the read and writes accordingly.
    
    We do the same for kcm->rx_wait in the following patch.
    
    syzbot reported:
    BUG: KCSAN: data-race in kcm_rfree / unreserve_rx_kcm
    
    write to 0xffff888123d827b8 of 8 bytes by task 2758 on cpu 1:
    unreserve_rx_kcm+0x72/0x1f0 net/kcm/kcmsock.c:313
    kcm_rcv_strparser+0x2b5/0x3a0 net/kcm/kcmsock.c:373
    __strp_recv+0x64c/0xd20 net/strparser/strparser.c:301
    strp_recv+0x6d/0x80 net/strparser/strparser.c:335
    tcp_read_sock+0x13e/0x5a0 net/ipv4/tcp.c:1703
    strp_read_sock net/strparser/strparser.c:358 [inline]
    do_strp_work net/strparser/strparser.c:406 [inline]
    strp_work+0xe8/0x180 net/strparser/strparser.c:415
    process_one_work+0x3d3/0x720 kernel/workqueue.c:2289
    worker_thread+0x618/0xa70 kernel/workqueue.c:2436
    kthread+0x1a9/0x1e0 kernel/kthread.c:376
    ret_from_fork+0x1f/0x30 arch/x86/entry/entry_64.S:306
    
    read to 0xffff888123d827b8 of 8 bytes by task 5859 on cpu 0:
    kcm_rfree+0x14c/0x220 net/kcm/kcmsock.c:181
    skb_release_head_state+0x8e/0x160 net/core/skbuff.c:841
    skb_release_all net/core/skbuff.c:852 [inline]
    __kfree_skb net/core/skbuff.c:868 [inline]
    kfree_skb_reason+0x5c/0x260 net/core/skbuff.c:891
    kfree_skb include/linux/skbuff.h:1216 [inline]
    kcm_recvmsg+0x226/0x2b0 net/kcm/kcmsock.c:1161
    ____sys_recvmsg+0x16c/0x2e0
    ___sys_recvmsg net/socket.c:2743 [inline]
    do_recvmmsg+0x2f1/0x710 net/socket.c:2837
    __sys_recvmmsg net/socket.c:2916 [inline]
    __do_sys_recvmmsg net/socket.c:2939 [inline]
    __se_sys_recvmmsg net/socket.c:2932 [inline]
    __x64_sys_recvmmsg+0xde/0x160 net/socket.c:2932
    do_syscall_x64 arch/x86/entry/common.c:50 [inline]
    do_syscall_64+0x2b/0x70 arch/x86/entry/common.c:80
    entry_SYSCALL_64_after_hwframe+0x63/0xcd
    
    value changed: 0xffff88812971ce00 -> 0x0000000000000000
    
    Reported by Kernel Concurrency Sanitizer on:
    CPU: 0 PID: 5859 Comm: syz-executor.3 Not tainted 6.0.0-syzkaller-12189-g19d17ab7c68b-dirty #0
    Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 09/22/2022
    
    Fixes: ab7ac4eb9832 ("kcm: Kernel Connection Multiplexor module")
    Reported-by: syzbot <syzkaller@googlegroups.com>
    Signed-off-by: Eric Dumazet <edumazet@google.com>
    Signed-off-by: David S. Miller <davem@davemloft.net>

diff --git a/net/kcm/kcmsock.c b/net/kcm/kcmsock.c
index 27725464ec08..0109ef6ddf9a 100644
--- a/net/kcm/kcmsock.c
+++ b/net/kcm/kcmsock.c
@@ -178,7 +178,7 @@ static void kcm_rfree(struct sk_buff *skb)
 	/* For reading rx_wait and rx_psock without holding lock */
 	smp_mb__after_atomic();
 
-	if (!kcm->rx_wait && !kcm->rx_psock &&
+	if (!kcm->rx_wait && !READ_ONCE(kcm->rx_psock) &&
 	    sk_rmem_alloc_get(sk) < sk->sk_rcvlowat) {
 		spin_lock_bh(&mux->rx_lock);
 		kcm_rcv_ready(kcm);
@@ -283,7 +283,8 @@ static struct kcm_sock *reserve_rx_kcm(struct kcm_psock *psock,
 	kcm->rx_wait = false;
 
 	psock->rx_kcm = kcm;
-	kcm->rx_psock = psock;
+	/* paired with lockless reads in kcm_rfree() */
+	WRITE_ONCE(kcm->rx_psock, psock);
 
 	spin_unlock_bh(&mux->rx_lock);
 
@@ -310,7 +311,8 @@ static void unreserve_rx_kcm(struct kcm_psock *psock,
 	spin_lock_bh(&mux->rx_lock);
 
 	psock->rx_kcm = NULL;
-	kcm->rx_psock = NULL;
+	/* paired with lockless reads in kcm_rfree() */
+	WRITE_ONCE(kcm->rx_psock, NULL);
 
 	/* Commit kcm->rx_psock before sk_rmem_alloc_get to sync with
 	 * kcm_rfree

```

## File: `net/kcm/kcmsock.c`

- changed old-lines: [181, 286, 313]

### Function `kcm_rfree` (L168–L187, 20 lines)

```c
static void kcm_rfree(struct sk_buff *skb)
{
	struct sock *sk = skb->sk;
	struct kcm_sock *kcm = kcm_sk(sk);
	struct kcm_mux *mux = kcm->mux;
	unsigned int len = skb->truesize;

	sk_mem_uncharge(sk, len);
	atomic_sub(len, &sk->sk_rmem_alloc);

	/* For reading rx_wait and rx_psock without holding lock */
	smp_mb__after_atomic();

	if (!kcm->rx_wait && !kcm->rx_psock &&
	    sk_rmem_alloc_get(sk) < sk->sk_rcvlowat) {
		spin_lock_bh(&mux->rx_lock);
		kcm_rcv_ready(kcm);
		spin_unlock_bh(&mux->rx_lock);
	}
}
```

_(no external call sites found for `kcm_rfree`)_

### Function `reserve_rx_kcm` (L251–L291, 41 lines)

```c
static struct kcm_sock *reserve_rx_kcm(struct kcm_psock *psock,
				       struct sk_buff *head)
{
	struct kcm_mux *mux = psock->mux;
	struct kcm_sock *kcm;

	WARN_ON(psock->ready_rx_msg);

	if (psock->rx_kcm)
		return psock->rx_kcm;

	spin_lock_bh(&mux->rx_lock);

	if (psock->rx_kcm) {
		spin_unlock_bh(&mux->rx_lock);
		return psock->rx_kcm;
	}

	kcm_update_rx_mux_stats(mux, psock);

	if (list_empty(&mux->kcm_rx_waiters)) {
		psock->ready_rx_msg = head;
		strp_pause(&psock->strp);
		list_add_tail(&psock->psock_ready_list,
			      &mux->psocks_ready);
		spin_unlock_bh(&mux->rx_lock);
		return NULL;
	}

	kcm = list_first_entry(&mux->kcm_rx_waiters,
			       struct kcm_sock, wait_rx_list);
	list_del(&kcm->wait_rx_list);
	kcm->rx_wait = false;

	psock->rx_kcm = kcm;
	kcm->rx_psock = psock;

	spin_unlock_bh(&mux->rx_lock);

	return kcm;
}
```

_(no external call sites found for `reserve_rx_kcm`)_

### Function `unreserve_rx_kcm` (L301–L340, 40 lines)

```c
static void unreserve_rx_kcm(struct kcm_psock *psock,
			     bool rcv_ready)
{
	struct kcm_sock *kcm = psock->rx_kcm;
	struct kcm_mux *mux = psock->mux;

	if (!kcm)
		return;

	spin_lock_bh(&mux->rx_lock);

	psock->rx_kcm = NULL;
	kcm->rx_psock = NULL;

	/* Commit kcm->rx_psock before sk_rmem_alloc_get to sync with
	 * kcm_rfree
	 */
	smp_mb();

	if (unlikely(kcm->done)) {
		spin_unlock_bh(&mux->rx_lock);

		/* Need to run kcm_done in a task since we need to qcquire
		 * callback locks which may already be held here.
		 */
		INIT_WORK(&kcm->done_work, kcm_done_work);
		schedule_work(&kcm->done_work);
		return;
	}

	if (unlikely(kcm->rx_disabled)) {
		requeue_rx_msgs(mux, &kcm->sk.sk_receive_queue);
	} else if (rcv_ready || unlikely(!sk_rmem_alloc_get(&kcm->sk))) {
		/* Check for degenerative race with rx_wait that all
		 * data was dequeued (accounted for in kcm_rfree).
		 */
		kcm_rcv_ready(kcm);
	}
	spin_unlock_bh(&mux->rx_lock);
}
```

_(no external call sites found for `unreserve_rx_kcm`)_
