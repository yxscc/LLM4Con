# Evidence: SYZBOT-250e1a870af0ced3  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in __netlink_dump_start / netlink_recvmsg (5)
- mainline fix commit: `a939d14919b799e6fff8a9c80296ca229ba2f8a4`
- candidate JSON: `A_SYZBOT-250e1a870af0ced3.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "__netlink_dump_start", "fn_b": "netlink_recvmsg"}`

## Commit message

```
a939d14919b799e6fff8a9c80296ca229ba2f8a4
netlink: annotate accesses to nlk->cb_running

Both netlink_recvmsg() and netlink_native_seq_show() read
nlk->cb_running locklessly. Use READ_ONCE() there.

Add corresponding WRITE_ONCE() to netlink_dump() and
__netlink_dump_start()

syzbot reported:
BUG: KCSAN: data-race in __netlink_dump_start / netlink_recvmsg

write to 0xffff88813ea4db59 of 1 bytes by task 28219 on cpu 0:
__netlink_dump_start+0x3af/0x4d0 net/netlink/af_netlink.c:2399
netlink_dump_start include/linux/netlink.h:308 [inline]
rtnetlink_rcv_msg+0x70f/0x8c0 net/core/rtnetlink.c:6130
netlink_rcv_skb+0x126/0x220 net/netlink/af_netlink.c:2577
rtnetlink_rcv+0x1c/0x20 net/core/rtnetlink.c:6192
netlink_unicast_kernel net/netlink/af_netlink.c:1339 [inline]
netlink_unicast+0x56f/0x640 net/netlink/af_netlink.c:1365
netlink_sendmsg+0x665/0x770 net/netlink/af_netlink.c:1942
sock_sendmsg_nosec net/socket.c:724 [inline]
sock_sendmsg net/socket.c:747 [inline]
sock_write_iter+0x1aa/0x230 net/socket.c:1138
call_write_iter include/linux/fs.h:1851 [inline]
new_sync_write fs/read_write.c:491 [inline]
vfs_write+0x463/0x760 fs/read_write.c:584
ksys_write+0xeb/0x1a0 fs/read_write.c:637
__do_sys_write fs/read_write.c:649 [inline]
__se_sys_write fs/read_write.c:646 [inline]
__x64_sys_write+0x42/0x50 fs/read_write.c:646
do_syscall_x64 arch/x86/entry/common.c:50 [inline]
do_syscall_64+0x41/0xc0 arch/x86/entry/common.c:80
entry_SYSCALL_64_after_hwframe+0x63/0xcd

read to 0xffff88813ea4db59 of 1 bytes by task 28222 on cpu 1:
netlink_recvmsg+0x3b4/0x730 net/netlink/af_netlink.c:2022
sock_recvmsg_nosec+0x4c/0x80 net/socket.c:1017
____sys_recvmsg+0x2db/0x310 net/socket.c:2718
___sys_recvmsg net/socket.c:2762 [inline]
do_recvmmsg+0x2e5/0x710 net/socket.c:2856
__sys_recvmmsg net/socket.c:2935 [inline]
__do_sys_recvmmsg net/socket.c:2958 [inline]
__se_sys_recvmmsg net/socket.c:2951 [inline]
__x64_sys_recvmmsg+0xe2/0x160 net/socket.c:2951
do_syscall_x64 arch/x86/entry/common.c:50 [inline]
do_syscall_64+0x41/0xc0 arch/x86/entry/common.c:80
entry_SYSCALL_64_after_hwframe+0x63/0xcd

value changed: 0x00 -> 0x01

Fixes: 16b304f3404f ("netlink: Eliminate kmalloc in netlink dump operation.")
Reported-by: syzbot <syzkaller@googlegroups.com>
Signed-off-by: Eric Dumazet <edumazet@google.com>
Signed-off-by: David S. Miller <davem@davemloft.net>
```

## CVE / bug description

```
KCSAN: data-race in __netlink_dump_start / netlink_recvmsg (5)
```

## Full mainline patch

```diff
commit a939d14919b799e6fff8a9c80296ca229ba2f8a4
Author: Eric Dumazet <edumazet@google.com>
Date:   Tue May 9 16:56:34 2023 +0000

    netlink: annotate accesses to nlk->cb_running
    
    Both netlink_recvmsg() and netlink_native_seq_show() read
    nlk->cb_running locklessly. Use READ_ONCE() there.
    
    Add corresponding WRITE_ONCE() to netlink_dump() and
    __netlink_dump_start()
    
    syzbot reported:
    BUG: KCSAN: data-race in __netlink_dump_start / netlink_recvmsg
    
    write to 0xffff88813ea4db59 of 1 bytes by task 28219 on cpu 0:
    __netlink_dump_start+0x3af/0x4d0 net/netlink/af_netlink.c:2399
    netlink_dump_start include/linux/netlink.h:308 [inline]
    rtnetlink_rcv_msg+0x70f/0x8c0 net/core/rtnetlink.c:6130
    netlink_rcv_skb+0x126/0x220 net/netlink/af_netlink.c:2577
    rtnetlink_rcv+0x1c/0x20 net/core/rtnetlink.c:6192
    netlink_unicast_kernel net/netlink/af_netlink.c:1339 [inline]
    netlink_unicast+0x56f/0x640 net/netlink/af_netlink.c:1365
    netlink_sendmsg+0x665/0x770 net/netlink/af_netlink.c:1942
    sock_sendmsg_nosec net/socket.c:724 [inline]
    sock_sendmsg net/socket.c:747 [inline]
    sock_write_iter+0x1aa/0x230 net/socket.c:1138
    call_write_iter include/linux/fs.h:1851 [inline]
    new_sync_write fs/read_write.c:491 [inline]
    vfs_write+0x463/0x760 fs/read_write.c:584
    ksys_write+0xeb/0x1a0 fs/read_write.c:637
    __do_sys_write fs/read_write.c:649 [inline]
    __se_sys_write fs/read_write.c:646 [inline]
    __x64_sys_write+0x42/0x50 fs/read_write.c:646
    do_syscall_x64 arch/x86/entry/common.c:50 [inline]
    do_syscall_64+0x41/0xc0 arch/x86/entry/common.c:80
    entry_SYSCALL_64_after_hwframe+0x63/0xcd
    
    read to 0xffff88813ea4db59 of 1 bytes by task 28222 on cpu 1:
    netlink_recvmsg+0x3b4/0x730 net/netlink/af_netlink.c:2022
    sock_recvmsg_nosec+0x4c/0x80 net/socket.c:1017
    ____sys_recvmsg+0x2db/0x310 net/socket.c:2718
    ___sys_recvmsg net/socket.c:2762 [inline]
    do_recvmmsg+0x2e5/0x710 net/socket.c:2856
    __sys_recvmmsg net/socket.c:2935 [inline]
    __do_sys_recvmmsg net/socket.c:2958 [inline]
    __se_sys_recvmmsg net/socket.c:2951 [inline]
    __x64_sys_recvmmsg+0xe2/0x160 net/socket.c:2951
    do_syscall_x64 arch/x86/entry/common.c:50 [inline]
    do_syscall_64+0x41/0xc0 arch/x86/entry/common.c:80
    entry_SYSCALL_64_after_hwframe+0x63/0xcd
    
    value changed: 0x00 -> 0x01
    
    Fixes: 16b304f3404f ("netlink: Eliminate kmalloc in netlink dump operation.")
    Reported-by: syzbot <syzkaller@googlegroups.com>
    Signed-off-by: Eric Dumazet <edumazet@google.com>
    Signed-off-by: David S. Miller <davem@davemloft.net>

diff --git a/net/netlink/af_netlink.c b/net/netlink/af_netlink.c
index 7ef8b9a1e30c..c87804112d0c 100644
--- a/net/netlink/af_netlink.c
+++ b/net/netlink/af_netlink.c
@@ -1990,7 +1990,7 @@ static int netlink_recvmsg(struct socket *sock, struct msghdr *msg, size_t len,
 
 	skb_free_datagram(sk, skb);
 
-	if (nlk->cb_running &&
+	if (READ_ONCE(nlk->cb_running) &&
 	    atomic_read(&sk->sk_rmem_alloc) <= sk->sk_rcvbuf / 2) {
 		ret = netlink_dump(sk);
 		if (ret) {
@@ -2302,7 +2302,7 @@ static int netlink_dump(struct sock *sk)
 	if (cb->done)
 		cb->done(cb);
 
-	nlk->cb_running = false;
+	WRITE_ONCE(nlk->cb_running, false);
 	module = cb->module;
 	skb = cb->skb;
 	mutex_unlock(nlk->cb_mutex);
@@ -2365,7 +2365,7 @@ int __netlink_dump_start(struct sock *ssk, struct sk_buff *skb,
 			goto error_put;
 	}
 
-	nlk->cb_running = true;
+	WRITE_ONCE(nlk->cb_running, true);
 	nlk->dump_done_errno = INT_MAX;
 
 	mutex_unlock(nlk->cb_mutex);
@@ -2703,7 +2703,7 @@ static int netlink_native_seq_show(struct seq_file *seq, void *v)
 			   nlk->groups ? (u32)nlk->groups[0] : 0,
 			   sk_rmem_alloc_get(s),
 			   sk_wmem_alloc_get(s),
-			   nlk->cb_running,
+			   READ_ONCE(nlk->cb_running),
 			   refcount_read(&s->sk_refcnt),
 			   atomic_read(&s->sk_drops),
 			   sock_i_ino(s)

```

## File: `net/netlink/af_netlink.c`

- changed old-lines: [1993, 2305, 2368, 2706]

### Function `netlink_recvmsg` (L1920–L2006, 87 lines)

```c
static int netlink_recvmsg(struct socket *sock, struct msghdr *msg, size_t len,
			   int flags)
{
	struct scm_cookie scm;
	struct sock *sk = sock->sk;
	struct netlink_sock *nlk = nlk_sk(sk);
	size_t copied, max_recvmsg_len;
	struct sk_buff *skb, *data_skb;
	int err, ret;

	if (flags & MSG_OOB)
		return -EOPNOTSUPP;

	copied = 0;

	skb = skb_recv_datagram(sk, flags, &err);
	if (skb == NULL)
		goto out;

	data_skb = skb;

#ifdef CONFIG_COMPAT_NETLINK_MESSAGES
	if (unlikely(skb_shinfo(skb)->frag_list)) {
		/*
		 * If this skb has a frag_list, then here that means that we
		 * will have to use the frag_list skb's data for compat tasks
		 * and the regular skb's data for normal (non-compat) tasks.
		 *
		 * If we need to send the compat skb, assign it to the
		 * 'data_skb' variable so that it will be used below for data
		 * copying. We keep 'skb' for everything else, including
		 * freeing both later.
		 */
		if (flags & MSG_CMSG_COMPAT)
			data_skb = skb_shinfo(skb)->frag_list;
	}
#endif

	/* Record the max length of recvmsg() calls for future allocations */
	max_recvmsg_len = max(READ_ONCE(nlk->max_recvmsg_len), len);
	max_recvmsg_len = min_t(size_t, max_recvmsg_len,
				SKB_WITH_OVERHEAD(32768));
	WRITE_ONCE(nlk->max_recvmsg_len, max_recvmsg_len);

	copied = data_skb->len;
	if (len < copied) {
		msg->msg_flags |= MSG_TRUNC;
		copied = len;
	}

	err = skb_copy_datagram_msg(data_skb, 0, msg, copied);

	if (msg->msg_name) {
		DECLARE_SOCKADDR(struct sockaddr_nl *, addr, msg->msg_name);
		addr->nl_family = AF_NETLINK;
		addr->nl_pad    = 0;
		addr->nl_pid	= NETLINK_CB(skb).portid;
		addr->nl_groups	= netlink_group_mask(NETLINK_CB(skb).dst_group);
		msg->msg_namelen = sizeof(*addr);
	}

	if (nlk->flags & NETLINK_F_RECV_PKTINFO)
		netlink_cmsg_recv_pktinfo(msg, skb);
	if (nlk->flags & NETLINK_F_LISTEN_ALL_NSID)
		netlink_cmsg_listen_all_nsid(sk, msg, skb);

	memset(&scm, 0, sizeof(scm));
	scm.creds = *NETLINK_CREDS(skb);
	if (flags & MSG_TRUNC)
		copied = data_skb->len;

	skb_free_datagram(sk, skb);

	if (nlk->cb_running &&
	    atomic_read(&sk->sk_rmem_alloc) <= sk->sk_rcvbuf / 2) {
		ret = netlink_dump(sk);
		if (ret) {
			sk->sk_err = -ret;
			sk_error_report(sk);
		}
	}

	scm_recv(sock, msg, &scm, flags);
out:
	netlink_rcv_wake(sk);
	return err ? : copied;
}
```

**External callers of `netlink_recvmsg` at parent**

- `drivers/block/drbd/drbd_nl.c:3721` — `	 * executes the dump callback successively from netlink_recvmsg(),`
- `tools/lib/bpf/netlink.c:105` — `static int netlink_recvmsg(int sock, struct msghdr *mhdr, int flags)`
- `tools/lib/bpf/netlink.c:152` — `		len = netlink_recvmsg(sock, &mhdr, MSG_PEEK | MSG_TRUNC);`
- `tools/lib/bpf/netlink.c:164` — `		len = netlink_recvmsg(sock, &mhdr, 0);`

### Function `netlink_dump` (L2203–L2317, 115 lines)

```c
static int netlink_dump(struct sock *sk)
{
	struct netlink_sock *nlk = nlk_sk(sk);
	struct netlink_ext_ack extack = {};
	struct netlink_callback *cb;
	struct sk_buff *skb = NULL;
	size_t max_recvmsg_len;
	struct module *module;
	int err = -ENOBUFS;
	int alloc_min_size;
	int alloc_size;

	mutex_lock(nlk->cb_mutex);
	if (!nlk->cb_running) {
		err = -EINVAL;
		goto errout_skb;
	}

	if (atomic_read(&sk->sk_rmem_alloc) >= sk->sk_rcvbuf)
		goto errout_skb;

	/* NLMSG_GOODSIZE is small to avoid high order allocations being
	 * required, but it makes sense to _attempt_ a 16K bytes allocation
	 * to reduce number of system calls on dump operations, if user
	 * ever provided a big enough buffer.
	 */
	cb = &nlk->cb;
	alloc_min_size = max_t(int, cb->min_dump_alloc, NLMSG_GOODSIZE);

	max_recvmsg_len = READ_ONCE(nlk->max_recvmsg_len);
	if (alloc_min_size < max_recvmsg_len) {
		alloc_size = max_recvmsg_len;
		skb = alloc_skb(alloc_size,
				(GFP_KERNEL & ~__GFP_DIRECT_RECLAIM) |
				__GFP_NOWARN | __GFP_NORETRY);
	}
	if (!skb) {
		alloc_size = alloc_min_size;
		skb = alloc_skb(alloc_size, GFP_KERNEL);
	}
	if (!skb)
		goto errout_skb;

	/* Trim skb to allocated size. User is expected to provide buffer as
	 * large as max(min_dump_alloc, 16KiB (mac_recvmsg_len capped at
	 * netlink_recvmsg())). dump will pack as many smaller messages as
	 * could fit within the allocated skb. skb is typically allocated
	 * with larger space than required (could be as much as near 2x the
	 * requested size with align to next power of 2 approach). Allowing
	 * dump to use the excess space makes it difficult for a user to have a
	 * reasonable static buffer based on the expected largest dump of a
	 * single netdev. The outcome is MSG_TRUNC error.
	 */
	skb_reserve(skb, skb_tailroom(skb) - alloc_size);

	/* Make sure malicious BPF programs can not read unitialized memory
	 * from skb->head -> skb->data
	 */
	skb_reset_network_header(skb);
	skb_reset_mac_header(skb);

	netlink_skb_set_owner_r(skb, sk);

	if (nlk->dump_done_errno > 0) {
		cb->extack = &extack;
		nlk->dump_done_errno = cb->dump(skb, cb);
		cb->extack = NULL;
	}

	if (nlk->dump_done_errno > 0 ||
	    skb_tailroom(skb) < nlmsg_total_size(sizeof(nlk->dump_done_errno))) {
		mutex_unlock(nlk->cb_mutex);

		if (sk_filter(sk, skb))
			kfree_skb(skb);
		else
			__netlink_sendskb(sk, skb);
		return 0;
	}

	if (netlink_dump_done(nlk, skb, cb, &extack))
		goto errout_skb;

#ifdef CONFIG_COMPAT_NETLINK_MESSAGES
	/* frag_list skb's data is used for compat tasks
	 * and the regular skb's data for normal (non-compat) tasks.
	 * See netlink_recvmsg().
	 */
	if (unlikely(skb_shinfo(skb)->frag_list)) {
		if (netlink_dump_done(nlk, skb_shinfo(skb)->frag_list, cb, &extack))
			goto errout_skb;
	}
#endif

	if (sk_filter(sk, skb))
		kfree_skb(skb);
	else
		__netlink_sendskb(sk, skb);

	if (cb->done)
		cb->done(cb);

	nlk->cb_running = false;
	module = cb->module;
	skb = cb->skb;
	mutex_unlock(nlk->cb_mutex);
	module_put(module);
	consume_skb(skb);
	return 0;

errout_skb:
	mutex_unlock(nlk->cb_mutex);
	kfree_skb(skb);
	return err;
}
```

**External callers of `netlink_dump` at parent**

- `drivers/block/drbd/drbd_nl.c:3720` — `	 * relies on the current implementation of netlink_dump(), which`

### Function `__netlink_dump_start` (L2319–L2393, 75 lines)

```c
int __netlink_dump_start(struct sock *ssk, struct sk_buff *skb,
			 const struct nlmsghdr *nlh,
			 struct netlink_dump_control *control)
{
	struct netlink_sock *nlk, *nlk2;
	struct netlink_callback *cb;
	struct sock *sk;
	int ret;

	refcount_inc(&skb->users);

	sk = netlink_lookup(sock_net(ssk), ssk->sk_protocol, NETLINK_CB(skb).portid);
	if (sk == NULL) {
		ret = -ECONNREFUSED;
		goto error_free;
	}

	nlk = nlk_sk(sk);
	mutex_lock(nlk->cb_mutex);
	/* A dump is in progress... */
	if (nlk->cb_running) {
		ret = -EBUSY;
		goto error_unlock;
	}
	/* add reference of module which cb->dump belongs to */
	if (!try_module_get(control->module)) {
		ret = -EPROTONOSUPPORT;
		goto error_unlock;
	}

	cb = &nlk->cb;
	memset(cb, 0, sizeof(*cb));
	cb->dump = control->dump;
	cb->done = control->done;
	cb->nlh = nlh;
	cb->data = control->data;
	cb->module = control->module;
	cb->min_dump_alloc = control->min_dump_alloc;
	cb->skb = skb;

	nlk2 = nlk_sk(NETLINK_CB(skb).sk);
	cb->strict_check = !!(nlk2->flags & NETLINK_F_STRICT_CHK);

	if (control->start) {
		ret = control->start(cb);
		if (ret)
			goto error_put;
	}

	nlk->cb_running = true;
	nlk->dump_done_errno = INT_MAX;

	mutex_unlock(nlk->cb_mutex);

	ret = netlink_dump(sk);

	sock_put(sk);

	if (ret)
		return ret;

	/* We successfully started a dump, by returning -EINTR we
	 * signal not to send ACK even if it was requested.
	 */
	return -EINTR;

error_put:
	module_put(control->module);
error_unlock:
	sock_put(sk);
	mutex_unlock(nlk->cb_mutex);
error_free:
	kfree_skb(skb);
	return ret;
}
```

**External callers of `__netlink_dump_start` at parent**

- `include/linux/netlink.h:319` — `int __netlink_dump_start(struct sock *ssk, struct sk_buff *skb,`
- `include/linux/netlink.h:329` — `	return __netlink_dump_start(ssk, skb, nlh, control);`
- `net/netlink/genetlink.c:918` — `		err = __netlink_dump_start(net->genl_sock, skb, nlh, &c);`
- `net/netlink/genetlink.c:929` — `		err = __netlink_dump_start(net->genl_sock, skb, nlh, &c);`

### Function `netlink_native_seq_show` (L2689–L2714, 26 lines)

```c
static int netlink_native_seq_show(struct seq_file *seq, void *v)
{
	if (v == SEQ_START_TOKEN) {
		seq_puts(seq,
			 "sk               Eth Pid        Groups   "
			 "Rmem     Wmem     Dump  Locks    Drops    Inode\n");
	} else {
		struct sock *s = v;
		struct netlink_sock *nlk = nlk_sk(s);

		seq_printf(seq, "%pK %-3d %-10u %08x %-8d %-8d %-5d %-8d %-8u %-8lu\n",
			   s,
			   s->sk_protocol,
			   nlk->portid,
			   nlk->groups ? (u32)nlk->groups[0] : 0,
			   sk_rmem_alloc_get(s),
			   sk_wmem_alloc_get(s),
			   nlk->cb_running,
			   refcount_read(&s->sk_refcnt),
			   atomic_read(&s->sk_drops),
			   sock_i_ino(s)
			);

	}
	return 0;
}
```

_(no external call sites found for `netlink_native_seq_show`)_
