# Evidence: SYZBOT-123b88b9ddea8e98  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in tcp_add_backlog / tcp_grow_window.isra.0
- mainline fix commit: `70c2655849a25431f31b505a07fe0c861e5e41fb`
- candidate JSON: `A_SYZBOT-123b88b9ddea8e98.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "tcp_add_backlog", "fn_b": "tcp_grow_window.isra.0"}`

## Commit message

```
70c2655849a25431f31b505a07fe0c861e5e41fb
net: silence KCSAN warnings about sk->sk_backlog.len reads

sk->sk_backlog.len can be written by BH handlers, and read
from process contexts in a lockless way.

Note the write side should also use WRITE_ONCE() or a variant.
We need some agreement about the best way to do this.

syzbot reported :

BUG: KCSAN: data-race in tcp_add_backlog / tcp_grow_window.isra.0

write to 0xffff88812665f32c of 4 bytes by interrupt on cpu 1:
 sk_add_backlog include/net/sock.h:934 [inline]
 tcp_add_backlog+0x4a0/0xcc0 net/ipv4/tcp_ipv4.c:1737
 tcp_v4_rcv+0x1aba/0x1bf0 net/ipv4/tcp_ipv4.c:1925
 ip_protocol_deliver_rcu+0x51/0x470 net/ipv4/ip_input.c:204
 ip_local_deliver_finish+0x110/0x140 net/ipv4/ip_input.c:231
 NF_HOOK include/linux/netfilter.h:305 [inline]
 NF_HOOK include/linux/netfilter.h:299 [inline]
 ip_local_deliver+0x133/0x210 net/ipv4/ip_input.c:252
 dst_input include/net/dst.h:442 [inline]
 ip_rcv_finish+0x121/0x160 net/ipv4/ip_input.c:413
 NF_HOOK include/linux/netfilter.h:305 [inline]
 NF_HOOK include/linux/netfilter.h:299 [inline]
 ip_rcv+0x18f/0x1a0 net/ipv4/ip_input.c:523
 __netif_receive_skb_one_core+0xa7/0xe0 net/core/dev.c:5004
 __netif_receive_skb+0x37/0xf0 net/core/dev.c:5118
 netif_receive_skb_internal+0x59/0x190 net/core/dev.c:5208
 napi_skb_finish net/core/dev.c:5671 [inline]
 napi_gro_receive+0x28f/0x330 net/core/dev.c:5704
 receive_buf+0x284/0x30b0 drivers/net/virtio_net.c:1061
 virtnet_receive drivers/net/virtio_net.c:1323 [inline]
 virtnet_poll+0x436/0x7d0 drivers/net/virtio_net.c:1428
 napi_poll net/core/dev.c:6352 [inline]
 net_rx_action+0x3ae/0xa50 net/core/dev.c:6418

read to 0xffff88812665f32c of 4 bytes by task 7292 on cpu 0:
 tcp_space include/net/tcp.h:1373 [inline]
 tcp_grow_window.isra.0+0x6b/0x480 net/ipv4/tcp_input.c:413
 tcp_event_data_recv+0x68f/0x990 net/ipv4/tcp_input.c:717
 tcp_rcv_established+0xbfe/0xf50 net/ipv4/tcp_input.c:5618
 tcp_v4_do_rcv+0x381/0x4e0 net/ipv4/tcp_ipv4.c:1542
 sk_backlog_rcv include/net/sock.h:945 [inline]
 __release_sock+0x135/0x1e0 net/core/sock.c:2427
 release_sock+0x61/0x160 net/core/sock.c:2943
 tcp_recvmsg+0x63b/0x1a30 net/ipv4/tcp.c:2181
 inet_recvmsg+0xbb/0x250 net/ipv4/af_inet.c:838
 sock_recvmsg_nosec net/socket.c:871 [inline]
 sock_recvmsg net/socket.c:889 [inline]
 sock_recvmsg+0x92/0xb0 net/socket.c:885
 sock_read_iter+0x15f/0x1e0 net/socket.c:967
 call_read_iter include/linux/fs.h:1864 [inline]
 new_sync_read+0x389/0x4f0 fs/read_write.c:414
 __vfs_read+0xb1/0xc0 fs/read_write.c:427
 vfs_read fs/read_write.c:461 [inline]
 vfs_read+0x143/0x2c0 fs/read_write.c:446

Reported by Kernel Concurrency Sanitizer on:
CPU: 0 PID: 7292 Comm: syz-fuzzer Not tainted 5.3.0+ #0
Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 01/01/2011

Signed-off-by: Eric Dumazet <edumazet@google.com>
Reported-by: syzbot <syzkaller@googlegroups.com>
Signed-off-by: Jakub Kicinski <jakub.kicinski@netronome.com>
```

## CVE / bug description

```
KCSAN: data-race in tcp_add_backlog / tcp_grow_window.isra.0
```

## Full mainline patch

```diff
commit 70c2655849a25431f31b505a07fe0c861e5e41fb
Author: Eric Dumazet <edumazet@google.com>
Date:   Wed Oct 9 15:41:03 2019 -0700

    net: silence KCSAN warnings about sk->sk_backlog.len reads
    
    sk->sk_backlog.len can be written by BH handlers, and read
    from process contexts in a lockless way.
    
    Note the write side should also use WRITE_ONCE() or a variant.
    We need some agreement about the best way to do this.
    
    syzbot reported :
    
    BUG: KCSAN: data-race in tcp_add_backlog / tcp_grow_window.isra.0
    
    write to 0xffff88812665f32c of 4 bytes by interrupt on cpu 1:
     sk_add_backlog include/net/sock.h:934 [inline]
     tcp_add_backlog+0x4a0/0xcc0 net/ipv4/tcp_ipv4.c:1737
     tcp_v4_rcv+0x1aba/0x1bf0 net/ipv4/tcp_ipv4.c:1925
     ip_protocol_deliver_rcu+0x51/0x470 net/ipv4/ip_input.c:204
     ip_local_deliver_finish+0x110/0x140 net/ipv4/ip_input.c:231
     NF_HOOK include/linux/netfilter.h:305 [inline]
     NF_HOOK include/linux/netfilter.h:299 [inline]
     ip_local_deliver+0x133/0x210 net/ipv4/ip_input.c:252
     dst_input include/net/dst.h:442 [inline]
     ip_rcv_finish+0x121/0x160 net/ipv4/ip_input.c:413
     NF_HOOK include/linux/netfilter.h:305 [inline]
     NF_HOOK include/linux/netfilter.h:299 [inline]
     ip_rcv+0x18f/0x1a0 net/ipv4/ip_input.c:523
     __netif_receive_skb_one_core+0xa7/0xe0 net/core/dev.c:5004
     __netif_receive_skb+0x37/0xf0 net/core/dev.c:5118
     netif_receive_skb_internal+0x59/0x190 net/core/dev.c:5208
     napi_skb_finish net/core/dev.c:5671 [inline]
     napi_gro_receive+0x28f/0x330 net/core/dev.c:5704
     receive_buf+0x284/0x30b0 drivers/net/virtio_net.c:1061
     virtnet_receive drivers/net/virtio_net.c:1323 [inline]
     virtnet_poll+0x436/0x7d0 drivers/net/virtio_net.c:1428
     napi_poll net/core/dev.c:6352 [inline]
     net_rx_action+0x3ae/0xa50 net/core/dev.c:6418
    
    read to 0xffff88812665f32c of 4 bytes by task 7292 on cpu 0:
     tcp_space include/net/tcp.h:1373 [inline]
     tcp_grow_window.isra.0+0x6b/0x480 net/ipv4/tcp_input.c:413
     tcp_event_data_recv+0x68f/0x990 net/ipv4/tcp_input.c:717
     tcp_rcv_established+0xbfe/0xf50 net/ipv4/tcp_input.c:5618
     tcp_v4_do_rcv+0x381/0x4e0 net/ipv4/tcp_ipv4.c:1542
     sk_backlog_rcv include/net/sock.h:945 [inline]
     __release_sock+0x135/0x1e0 net/core/sock.c:2427
     release_sock+0x61/0x160 net/core/sock.c:2943
     tcp_recvmsg+0x63b/0x1a30 net/ipv4/tcp.c:2181
     inet_recvmsg+0xbb/0x250 net/ipv4/af_inet.c:838
     sock_recvmsg_nosec net/socket.c:871 [inline]
     sock_recvmsg net/socket.c:889 [inline]
     sock_recvmsg+0x92/0xb0 net/socket.c:885
     sock_read_iter+0x15f/0x1e0 net/socket.c:967
     call_read_iter include/linux/fs.h:1864 [inline]
     new_sync_read+0x389/0x4f0 fs/read_write.c:414
     __vfs_read+0xb1/0xc0 fs/read_write.c:427
     vfs_read fs/read_write.c:461 [inline]
     vfs_read+0x143/0x2c0 fs/read_write.c:446
    
    Reported by Kernel Concurrency Sanitizer on:
    CPU: 0 PID: 7292 Comm: syz-fuzzer Not tainted 5.3.0+ #0
    Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 01/01/2011
    
    Signed-off-by: Eric Dumazet <edumazet@google.com>
    Reported-by: syzbot <syzkaller@googlegroups.com>
    Signed-off-by: Jakub Kicinski <jakub.kicinski@netronome.com>

diff --git a/include/net/tcp.h b/include/net/tcp.h
index 88e63d64c698..35f6f7e0fdc2 100644
--- a/include/net/tcp.h
+++ b/include/net/tcp.h
@@ -1380,7 +1380,8 @@ static inline int tcp_win_from_space(const struct sock *sk, int space)
 /* Note: caller must be prepared to deal with negative returns */
 static inline int tcp_space(const struct sock *sk)
 {
-	return tcp_win_from_space(sk, sk->sk_rcvbuf - sk->sk_backlog.len -
+	return tcp_win_from_space(sk, sk->sk_rcvbuf -
+				  READ_ONCE(sk->sk_backlog.len) -
 				  atomic_read(&sk->sk_rmem_alloc));
 }
 
diff --git a/net/core/sock.c b/net/core/sock.c
index b7c5c6ea51ba..2a053999df11 100644
--- a/net/core/sock.c
+++ b/net/core/sock.c
@@ -3210,7 +3210,7 @@ void sk_get_meminfo(const struct sock *sk, u32 *mem)
 	mem[SK_MEMINFO_FWD_ALLOC] = sk->sk_forward_alloc;
 	mem[SK_MEMINFO_WMEM_QUEUED] = sk->sk_wmem_queued;
 	mem[SK_MEMINFO_OPTMEM] = atomic_read(&sk->sk_omem_alloc);
-	mem[SK_MEMINFO_BACKLOG] = sk->sk_backlog.len;
+	mem[SK_MEMINFO_BACKLOG] = READ_ONCE(sk->sk_backlog.len);
 	mem[SK_MEMINFO_DROPS] = atomic_read(&sk->sk_drops);
 }
 
diff --git a/net/sctp/diag.c b/net/sctp/diag.c
index fc9a4c6629ce..0851166b9175 100644
--- a/net/sctp/diag.c
+++ b/net/sctp/diag.c
@@ -175,7 +175,7 @@ static int inet_sctp_diag_fill(struct sock *sk, struct sctp_association *asoc,
 		mem[SK_MEMINFO_FWD_ALLOC] = sk->sk_forward_alloc;
 		mem[SK_MEMINFO_WMEM_QUEUED] = sk->sk_wmem_queued;
 		mem[SK_MEMINFO_OPTMEM] = atomic_read(&sk->sk_omem_alloc);
-		mem[SK_MEMINFO_BACKLOG] = sk->sk_backlog.len;
+		mem[SK_MEMINFO_BACKLOG] = READ_ONCE(sk->sk_backlog.len);
 		mem[SK_MEMINFO_DROPS] = atomic_read(&sk->sk_drops);
 
 		if (nla_put(skb, INET_DIAG_SKMEMINFO, sizeof(mem), &mem) < 0)
diff --git a/net/tipc/socket.c b/net/tipc/socket.c
index 7c736cfec57f..f8bbc4aab213 100644
--- a/net/tipc/socket.c
+++ b/net/tipc/socket.c
@@ -3790,7 +3790,7 @@ int tipc_sk_dump(struct sock *sk, u16 dqueues, char *buf)
 	i += scnprintf(buf + i, sz - i, " %d", sk->sk_sndbuf);
 	i += scnprintf(buf + i, sz - i, " | %d", sk_rmem_alloc_get(sk));
 	i += scnprintf(buf + i, sz - i, " %d", sk->sk_rcvbuf);
-	i += scnprintf(buf + i, sz - i, " | %d\n", sk->sk_backlog.len);
+	i += scnprintf(buf + i, sz - i, " | %d\n", READ_ONCE(sk->sk_backlog.len));
 
 	if (dqueues & TIPC_DUMP_SK_SNDQ) {
 		i += scnprintf(buf + i, sz - i, "sk_write_queue: ");

```

## File: `include/net/tcp.h`

- changed old-lines: [1383]

### Function `tcp_space` (L1381–L1385, 5 lines)

```c
static inline int tcp_space(const struct sock *sk)
{
	return tcp_win_from_space(sk, sk->sk_rcvbuf - sk->sk_backlog.len -
				  atomic_read(&sk->sk_rmem_alloc));
}
```

**External callers of `tcp_space` at parent**

- `net/ipv4/tcp_input.c:413` — `	room = min_t(int, tp->window_clamp, tcp_space(sk)) - tp->rcv_ssthresh;`
- `net/ipv4/tcp_output.c:2696` — `	int free_space = tcp_space(sk);`

## File: `net/core/sock.c`

- changed old-lines: [3213]

### Function `sk_get_meminfo` (L3202–L3215, 14 lines)

```c
void sk_get_meminfo(const struct sock *sk, u32 *mem)
{
	memset(mem, 0, sizeof(*mem) * SK_MEMINFO_VARS);

	mem[SK_MEMINFO_RMEM_ALLOC] = sk_rmem_alloc_get(sk);
	mem[SK_MEMINFO_RCVBUF] = sk->sk_rcvbuf;
	mem[SK_MEMINFO_WMEM_ALLOC] = sk_wmem_alloc_get(sk);
	mem[SK_MEMINFO_SNDBUF] = sk->sk_sndbuf;
	mem[SK_MEMINFO_FWD_ALLOC] = sk->sk_forward_alloc;
	mem[SK_MEMINFO_WMEM_QUEUED] = sk->sk_wmem_queued;
	mem[SK_MEMINFO_OPTMEM] = atomic_read(&sk->sk_omem_alloc);
	mem[SK_MEMINFO_BACKLOG] = sk->sk_backlog.len;
	mem[SK_MEMINFO_DROPS] = atomic_read(&sk->sk_drops);
}
```

**External callers of `sk_get_meminfo` at parent**

- `include/net/sock.h:2526` — `void sk_get_meminfo(const struct sock *sk, u32 *meminfo);`
- `net/core/sock_diag.c:64` — `	sk_get_meminfo(sk, mem);`

## File: `net/sctp/diag.c`

- changed old-lines: [178]

### Function `inet_sctp_diag_fill` (L123–L218, 96 lines)

```c
static int inet_sctp_diag_fill(struct sock *sk, struct sctp_association *asoc,
			       struct sk_buff *skb,
			       const struct inet_diag_req_v2 *req,
			       struct user_namespace *user_ns,
			       int portid, u32 seq, u16 nlmsg_flags,
			       const struct nlmsghdr *unlh,
			       bool net_admin)
{
	struct sctp_endpoint *ep = sctp_sk(sk)->ep;
	struct list_head *addr_list;
	struct inet_diag_msg *r;
	struct nlmsghdr  *nlh;
	int ext = req->idiag_ext;
	struct sctp_infox infox;
	void *info = NULL;

	nlh = nlmsg_put(skb, portid, seq, unlh->nlmsg_type, sizeof(*r),
			nlmsg_flags);
	if (!nlh)
		return -EMSGSIZE;

	r = nlmsg_data(nlh);
	BUG_ON(!sk_fullsock(sk));

	if (asoc) {
		inet_diag_msg_sctpasoc_fill(r, sk, asoc);
	} else {
		inet_diag_msg_common_fill(r, sk);
		r->idiag_state = sk->sk_state;
		r->idiag_timer = 0;
		r->idiag_retrans = 0;
	}

	if (inet_diag_msg_attrs_fill(sk, skb, r, ext, user_ns, net_admin))
		goto errout;

	if (ext & (1 << (INET_DIAG_SKMEMINFO - 1))) {
		u32 mem[SK_MEMINFO_VARS];
		int amt;

		if (asoc && asoc->ep->sndbuf_policy)
			amt = asoc->sndbuf_used;
		else
			amt = sk_wmem_alloc_get(sk);
		mem[SK_MEMINFO_WMEM_ALLOC] = amt;
		if (asoc && asoc->ep->rcvbuf_policy)
			amt = atomic_read(&asoc->rmem_alloc);
		else
			amt = sk_rmem_alloc_get(sk);
		mem[SK_MEMINFO_RMEM_ALLOC] = amt;
		mem[SK_MEMINFO_RCVBUF] = sk->sk_rcvbuf;
		mem[SK_MEMINFO_SNDBUF] = sk->sk_sndbuf;
		mem[SK_MEMINFO_FWD_ALLOC] = sk->sk_forward_alloc;
		mem[SK_MEMINFO_WMEM_QUEUED] = sk->sk_wmem_queued;
		mem[SK_MEMINFO_OPTMEM] = atomic_read(&sk->sk_omem_alloc);
		mem[SK_MEMINFO_BACKLOG] = sk->sk_backlog.len;
		mem[SK_MEMINFO_DROPS] = atomic_read(&sk->sk_drops);

		if (nla_put(skb, INET_DIAG_SKMEMINFO, sizeof(mem), &mem) < 0)
			goto errout;
	}

	if (ext & (1 << (INET_DIAG_INFO - 1))) {
		struct nlattr *attr;

		attr = nla_reserve_64bit(skb, INET_DIAG_INFO,
					 sizeof(struct sctp_info),
					 INET_DIAG_PAD);
		if (!attr)
			goto errout;

		info = nla_data(attr);
	}
	infox.sctpinfo = (struct sctp_info *)info;
	infox.asoc = asoc;
	sctp_diag_get_info(sk, r, &infox);

	addr_list = asoc ? &asoc->base.bind_addr.address_list
			 : &ep->base.bind_addr.address_list;
	if (inet_diag_msg_sctpladdrs_fill(skb, addr_list))
		goto errout;

	if (asoc && (ext & (1 << (INET_DIAG_CONG - 1))))
		if (nla_put_string(skb, INET_DIAG_CONG, "reno") < 0)
			goto errout;

	if (asoc && inet_diag_msg_sctpaddrs_fill(skb, asoc))
		goto errout;

	nlmsg_end(skb, nlh);
	return 0;

errout:
	nlmsg_cancel(skb, nlh);
	return -EMSGSIZE;
}
```

_(no external call sites found for `inet_sctp_diag_fill`)_

## File: `net/tipc/socket.c`

- changed old-lines: [3793]

### Function `tipc_sk_dump` (L3745–L3816, 72 lines)

```c
int tipc_sk_dump(struct sock *sk, u16 dqueues, char *buf)
{
	int i = 0;
	size_t sz = (dqueues) ? SK_LMAX : SK_LMIN;
	struct tipc_sock *tsk;
	struct publication *p;
	bool tsk_connected;

	if (!sk) {
		i += scnprintf(buf, sz, "sk data: (null)\n");
		return i;
	}

	tsk = tipc_sk(sk);
	tsk_connected = !tipc_sk_type_connectionless(sk);

	i += scnprintf(buf, sz, "sk data: %u", sk->sk_type);
	i += scnprintf(buf + i, sz - i, " %d", sk->sk_state);
	i += scnprintf(buf + i, sz - i, " %x", tsk_own_node(tsk));
	i += scnprintf(buf + i, sz - i, " %u", tsk->portid);
	i += scnprintf(buf + i, sz - i, " | %u", tsk_connected);
	if (tsk_connected) {
		i += scnprintf(buf + i, sz - i, " %x", tsk_peer_node(tsk));
		i += scnprintf(buf + i, sz - i, " %u", tsk_peer_port(tsk));
		i += scnprintf(buf + i, sz - i, " %u", tsk->conn_type);
		i += scnprintf(buf + i, sz - i, " %u", tsk->conn_instance);
	}
	i += scnprintf(buf + i, sz - i, " | %u", tsk->published);
	if (tsk->published) {
		p = list_first_entry_or_null(&tsk->publications,
					     struct publication, binding_sock);
		i += scnprintf(buf + i, sz - i, " %u", (p) ? p->type : 0);
		i += scnprintf(buf + i, sz - i, " %u", (p) ? p->lower : 0);
		i += scnprintf(buf + i, sz - i, " %u", (p) ? p->upper : 0);
	}
	i += scnprintf(buf + i, sz - i, " | %u", tsk->snd_win);
	i += scnprintf(buf + i, sz - i, " %u", tsk->rcv_win);
	i += scnprintf(buf + i, sz - i, " %u", tsk->max_pkt);
	i += scnprintf(buf + i, sz - i, " %x", tsk->peer_caps);
	i += scnprintf(buf + i, sz - i, " %u", tsk->cong_link_cnt);
	i += scnprintf(buf + i, sz - i, " %u", tsk->snt_unacked);
	i += scnprintf(buf + i, sz - i, " %u", tsk->rcv_unacked);
	i += scnprintf(buf + i, sz - i, " %u", atomic_read(&tsk->dupl_rcvcnt));
	i += scnprintf(buf + i, sz - i, " %u", sk->sk_shutdown);
	i += scnprintf(buf + i, sz - i, " | %d", sk_wmem_alloc_get(sk));
	i += scnprintf(buf + i, sz - i, " %d", sk->sk_sndbuf);
	i += scnprintf(buf + i, sz - i, " | %d", sk_rmem_alloc_get(sk));
	i += scnprintf(buf + i, sz - i, " %d", sk->sk_rcvbuf);
	i += scnprintf(buf + i, sz - i, " | %d\n", sk->sk_backlog.len);

	if (dqueues & TIPC_DUMP_SK_SNDQ) {
		i += scnprintf(buf + i, sz - i, "sk_write_queue: ");
		i += tipc_list_dump(&sk->sk_write_queue, false, buf + i);
	}

	if (dqueues & TIPC_DUMP_SK_RCVQ) {
		i += scnprintf(buf + i, sz - i, "sk_receive_queue: ");
		i += tipc_list_dump(&sk->sk_receive_queue, false, buf + i);
	}

	if (dqueues & TIPC_DUMP_SK_BKLGQ) {
		i += scnprintf(buf + i, sz - i, "sk_backlog:\n  head ");
		i += tipc_skb_dump(sk->sk_backlog.head, false, buf + i);
		if (sk->sk_backlog.tail != sk->sk_backlog.head) {
			i += scnprintf(buf + i, sz - i, "  tail ");
			i += tipc_skb_dump(sk->sk_backlog.tail, false,
					   buf + i);
		}
	}

	return i;
}
```

**External callers of `tipc_sk_dump` at parent**

- `net/tipc/trace.h:131` — `int tipc_sk_dump(struct sock *sk, u16 dqueues, char *buf);`
- `net/tipc/trace.h:205` — `		tipc_sk_dump(sk, dqueues, __get_str(buf));`
- `net/tipc/trace.h:222` — `DEFINE_SK_EVENT_FILTER(tipc_sk_dump);`
