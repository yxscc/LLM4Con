# Evidence: SYZBOT-282e13b9bc8ac284  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in sk_stream_wait_memory / tcp_ack
- mainline fix commit: `ab4e846a82d0ae00176de19f2db3c5c64f8eb5f2`
- candidate JSON: `A_SYZBOT-282e13b9bc8ac284.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "sk_stream_wait_memory", "fn_b": "tcp_ack"}`

## Commit message

```
ab4e846a82d0ae00176de19f2db3c5c64f8eb5f2
tcp: annotate sk->sk_wmem_queued lockless reads

For the sake of tcp_poll(), there are few places where we fetch
sk->sk_wmem_queued while this field can change from IRQ or other cpu.

We need to add READ_ONCE() annotations, and also make sure write
sides use corresponding WRITE_ONCE() to avoid store-tearing.

sk_wmem_queued_add() helper is added so that we can in
the future convert to ADD_ONCE() or equivalent if/when
available.

Signed-off-by: Eric Dumazet <edumazet@google.com>
Signed-off-by: David S. Miller <davem@davemloft.net>
```

## CVE / bug description

```
KCSAN: data-race in sk_stream_wait_memory / tcp_ack
```

## Full mainline patch

```diff
commit ab4e846a82d0ae00176de19f2db3c5c64f8eb5f2
Author: Eric Dumazet <edumazet@google.com>
Date:   Thu Oct 10 20:17:46 2019 -0700

    tcp: annotate sk->sk_wmem_queued lockless reads
    
    For the sake of tcp_poll(), there are few places where we fetch
    sk->sk_wmem_queued while this field can change from IRQ or other cpu.
    
    We need to add READ_ONCE() annotations, and also make sure write
    sides use corresponding WRITE_ONCE() to avoid store-tearing.
    
    sk_wmem_queued_add() helper is added so that we can in
    the future convert to ADD_ONCE() or equivalent if/when
    available.
    
    Signed-off-by: Eric Dumazet <edumazet@google.com>
    Signed-off-by: David S. Miller <davem@davemloft.net>

diff --git a/include/net/sock.h b/include/net/sock.h
index 3d1e7502333e..f69b58bff7e5 100644
--- a/include/net/sock.h
+++ b/include/net/sock.h
@@ -878,12 +878,17 @@ static inline bool sk_acceptq_is_full(const struct sock *sk)
  */
 static inline int sk_stream_min_wspace(const struct sock *sk)
 {
-	return sk->sk_wmem_queued >> 1;
+	return READ_ONCE(sk->sk_wmem_queued) >> 1;
 }
 
 static inline int sk_stream_wspace(const struct sock *sk)
 {
-	return READ_ONCE(sk->sk_sndbuf) - sk->sk_wmem_queued;
+	return READ_ONCE(sk->sk_sndbuf) - READ_ONCE(sk->sk_wmem_queued);
+}
+
+static inline void sk_wmem_queued_add(struct sock *sk, int val)
+{
+	WRITE_ONCE(sk->sk_wmem_queued, sk->sk_wmem_queued + val);
 }
 
 void sk_stream_write_space(struct sock *sk);
@@ -1207,7 +1212,7 @@ static inline void sk_refcnt_debug_release(const struct sock *sk)
 
 static inline bool __sk_stream_memory_free(const struct sock *sk, int wake)
 {
-	if (sk->sk_wmem_queued >= READ_ONCE(sk->sk_sndbuf))
+	if (READ_ONCE(sk->sk_wmem_queued) >= READ_ONCE(sk->sk_sndbuf))
 		return false;
 
 	return sk->sk_prot->stream_memory_free ?
@@ -1467,7 +1472,7 @@ DECLARE_STATIC_KEY_FALSE(tcp_tx_skb_cache_key);
 static inline void sk_wmem_free_skb(struct sock *sk, struct sk_buff *skb)
 {
 	sock_set_flag(sk, SOCK_QUEUE_SHRUNK);
-	sk->sk_wmem_queued -= skb->truesize;
+	sk_wmem_queued_add(sk, -skb->truesize);
 	sk_mem_uncharge(sk, skb->truesize);
 	if (static_branch_unlikely(&tcp_tx_skb_cache_key) &&
 	    !sk->sk_tx_skb_cache && !skb_cloned(skb)) {
@@ -2014,7 +2019,7 @@ static inline int skb_copy_to_page_nocache(struct sock *sk, struct iov_iter *fro
 	skb->len	     += copy;
 	skb->data_len	     += copy;
 	skb->truesize	     += copy;
-	sk->sk_wmem_queued   += copy;
+	sk_wmem_queued_add(sk, copy);
 	sk_mem_charge(sk, copy);
 	return 0;
 }
diff --git a/include/trace/events/sock.h b/include/trace/events/sock.h
index f720c32e7dfd..51fe9f6719eb 100644
--- a/include/trace/events/sock.h
+++ b/include/trace/events/sock.h
@@ -115,7 +115,7 @@ TRACE_EVENT(sock_exceed_buf_limit,
 		__entry->rmem_alloc = atomic_read(&sk->sk_rmem_alloc);
 		__entry->sysctl_wmem = sk_get_wmem0(sk, prot);
 		__entry->wmem_alloc = refcount_read(&sk->sk_wmem_alloc);
-		__entry->wmem_queued = sk->sk_wmem_queued;
+		__entry->wmem_queued = READ_ONCE(sk->sk_wmem_queued);
 		__entry->kind = kind;
 	),
 
diff --git a/net/core/datagram.c b/net/core/datagram.c
index 4cc8dc5db2b7..c210fc116103 100644
--- a/net/core/datagram.c
+++ b/net/core/datagram.c
@@ -640,7 +640,7 @@ int __zerocopy_sg_from_iter(struct sock *sk, struct sk_buff *skb,
 		skb->len += copied;
 		skb->truesize += truesize;
 		if (sk && sk->sk_type == SOCK_STREAM) {
-			sk->sk_wmem_queued += truesize;
+			sk_wmem_queued_add(sk, truesize);
 			sk_mem_charge(sk, truesize);
 		} else {
 			refcount_add(truesize, &skb->sk->sk_wmem_alloc);
diff --git a/net/core/sock.c b/net/core/sock.c
index cd075bc86407..a515392ba84b 100644
--- a/net/core/sock.c
+++ b/net/core/sock.c
@@ -3212,7 +3212,7 @@ void sk_get_meminfo(const struct sock *sk, u32 *mem)
 	mem[SK_MEMINFO_WMEM_ALLOC] = sk_wmem_alloc_get(sk);
 	mem[SK_MEMINFO_SNDBUF] = READ_ONCE(sk->sk_sndbuf);
 	mem[SK_MEMINFO_FWD_ALLOC] = sk->sk_forward_alloc;
-	mem[SK_MEMINFO_WMEM_QUEUED] = sk->sk_wmem_queued;
+	mem[SK_MEMINFO_WMEM_QUEUED] = READ_ONCE(sk->sk_wmem_queued);
 	mem[SK_MEMINFO_OPTMEM] = atomic_read(&sk->sk_omem_alloc);
 	mem[SK_MEMINFO_BACKLOG] = READ_ONCE(sk->sk_backlog.len);
 	mem[SK_MEMINFO_DROPS] = atomic_read(&sk->sk_drops);
diff --git a/net/ipv4/inet_diag.c b/net/ipv4/inet_diag.c
index bbb005eb5218..7dc79b973e6e 100644
--- a/net/ipv4/inet_diag.c
+++ b/net/ipv4/inet_diag.c
@@ -193,7 +193,7 @@ int inet_sk_diag_fill(struct sock *sk, struct inet_connection_sock *icsk,
 	if (ext & (1 << (INET_DIAG_MEMINFO - 1))) {
 		struct inet_diag_meminfo minfo = {
 			.idiag_rmem = sk_rmem_alloc_get(sk),
-			.idiag_wmem = sk->sk_wmem_queued,
+			.idiag_wmem = READ_ONCE(sk->sk_wmem_queued),
 			.idiag_fmem = sk->sk_forward_alloc,
 			.idiag_tmem = sk_wmem_alloc_get(sk),
 		};
diff --git a/net/ipv4/tcp.c b/net/ipv4/tcp.c
index 111853262972..b2ac4f074e2d 100644
--- a/net/ipv4/tcp.c
+++ b/net/ipv4/tcp.c
@@ -659,7 +659,7 @@ static void skb_entail(struct sock *sk, struct sk_buff *skb)
 	tcb->sacked  = 0;
 	__skb_header_release(skb);
 	tcp_add_write_queue_tail(sk, skb);
-	sk->sk_wmem_queued += skb->truesize;
+	sk_wmem_queued_add(sk, skb->truesize);
 	sk_mem_charge(sk, skb->truesize);
 	if (tp->nonagle & TCP_NAGLE_PUSH)
 		tp->nonagle &= ~TCP_NAGLE_PUSH;
@@ -1034,7 +1034,7 @@ ssize_t do_tcp_sendpages(struct sock *sk, struct page *page, int offset,
 		skb->len += copy;
 		skb->data_len += copy;
 		skb->truesize += copy;
-		sk->sk_wmem_queued += copy;
+		sk_wmem_queued_add(sk, copy);
 		sk_mem_charge(sk, copy);
 		skb->ip_summed = CHECKSUM_PARTIAL;
 		WRITE_ONCE(tp->write_seq, tp->write_seq + copy);
diff --git a/net/ipv4/tcp_output.c b/net/ipv4/tcp_output.c
index a115a991dfb5..0488607c5cd3 100644
--- a/net/ipv4/tcp_output.c
+++ b/net/ipv4/tcp_output.c
@@ -1199,7 +1199,7 @@ static void tcp_queue_skb(struct sock *sk, struct sk_buff *skb)
 	WRITE_ONCE(tp->write_seq, TCP_SKB_CB(skb)->end_seq);
 	__skb_header_release(skb);
 	tcp_add_write_queue_tail(sk, skb);
-	sk->sk_wmem_queued += skb->truesize;
+	sk_wmem_queued_add(sk, skb->truesize);
 	sk_mem_charge(sk, skb->truesize);
 }
 
@@ -1333,7 +1333,7 @@ int tcp_fragment(struct sock *sk, enum tcp_queue tcp_queue,
 		return -ENOMEM; /* We'll just try again later. */
 	skb_copy_decrypted(buff, skb);
 
-	sk->sk_wmem_queued += buff->truesize;
+	sk_wmem_queued_add(sk, buff->truesize);
 	sk_mem_charge(sk, buff->truesize);
 	nlen = skb->len - len - nsize;
 	buff->truesize += nlen;
@@ -1443,7 +1443,7 @@ int tcp_trim_head(struct sock *sk, struct sk_buff *skb, u32 len)
 
 	if (delta_truesize) {
 		skb->truesize	   -= delta_truesize;
-		sk->sk_wmem_queued -= delta_truesize;
+		sk_wmem_queued_add(sk, -delta_truesize);
 		sk_mem_uncharge(sk, delta_truesize);
 		sock_set_flag(sk, SOCK_QUEUE_SHRUNK);
 	}
@@ -1888,7 +1888,7 @@ static int tso_fragment(struct sock *sk, struct sk_buff *skb, unsigned int len,
 		return -ENOMEM;
 	skb_copy_decrypted(buff, skb);
 
-	sk->sk_wmem_queued += buff->truesize;
+	sk_wmem_queued_add(sk, buff->truesize);
 	sk_mem_charge(sk, buff->truesize);
 	buff->truesize += nlen;
 	skb->truesize -= nlen;
@@ -2152,7 +2152,7 @@ static int tcp_mtu_probe(struct sock *sk)
 	nskb = sk_stream_alloc_skb(sk, probe_size, GFP_ATOMIC, false);
 	if (!nskb)
 		return -1;
-	sk->sk_wmem_queued += nskb->truesize;
+	sk_wmem_queued_add(sk, nskb->truesize);
 	sk_mem_charge(sk, nskb->truesize);
 
 	skb = tcp_send_head(sk);
@@ -3222,7 +3222,7 @@ int tcp_send_synack(struct sock *sk)
 			tcp_rtx_queue_unlink_and_free(skb, sk);
 			__skb_header_release(nskb);
 			tcp_rbtree_insert(&sk->tcp_rtx_queue, nskb);
-			sk->sk_wmem_queued += nskb->truesize;
+			sk_wmem_queued_add(sk, nskb->truesize);
 			sk_mem_charge(sk, nskb->truesize);
 			skb = nskb;
 		}
@@ -3447,7 +3447,7 @@ static void tcp_connect_queue_skb(struct sock *sk, struct sk_buff *skb)
 
 	tcb->end_seq += skb->len;
 	__skb_header_release(skb);
-	sk->sk_wmem_queued += skb->truesize;
+	sk_wmem_queued_add(sk, skb->truesize);
 	sk_mem_charge(sk, skb->truesize);
 	WRITE_ONCE(tp->write_seq, tcb->end_seq);
 	tp->packets_out += tcp_skb_pcount(skb);
diff --git a/net/sched/em_meta.c b/net/sched/em_meta.c
index 4c9122fc35c9..3177dcb17316 100644
--- a/net/sched/em_meta.c
+++ b/net/sched/em_meta.c
@@ -446,7 +446,7 @@ META_COLLECTOR(int_sk_wmem_queued)
 		*err = -1;
 		return;
 	}
-	dst->value = sk->sk_wmem_queued;
+	dst->value = READ_ONCE(sk->sk_wmem_queued);
 }
 
 META_COLLECTOR(int_sk_fwd_alloc)

```

## File: `include/net/sock.h`

- changed old-lines: [881, 886, 1210, 1470, 2017]

### Function `sk_stream_min_wspace` (L879–L882, 4 lines)

```c
static inline int sk_stream_min_wspace(const struct sock *sk)
{
	return sk->sk_wmem_queued >> 1;
}
```

**External callers of `sk_stream_min_wspace` at parent**

- `drivers/crypto/chelsio/chtls/chtls_io.c:1161` — `		    (sk_stream_wspace(sk) < sk_stream_min_wspace(sk)))`
- `drivers/crypto/chelsio/chtls/chtls_io.c:1273` — `		    (sk_stream_wspace(sk) < sk_stream_min_wspace(sk)))`

### Function `sk_stream_wspace` (L884–L887, 4 lines)

```c
static inline int sk_stream_wspace(const struct sock *sk)
{
	return READ_ONCE(sk->sk_sndbuf) - sk->sk_wmem_queued;
}
```

**External callers of `sk_stream_wspace` at parent**

- `drivers/crypto/chelsio/chtls/chtls_io.c:1161` — `		    (sk_stream_wspace(sk) < sk_stream_min_wspace(sk)))`
- `drivers/crypto/chelsio/chtls/chtls_io.c:1273` — `		    (sk_stream_wspace(sk) < sk_stream_min_wspace(sk)))`
- `net/sctp/socket.c:110` — `				       : sk_stream_wspace(sk);`

### Function `__sk_stream_memory_free` (L1208–L1215, 8 lines)

```c
static inline bool __sk_stream_memory_free(const struct sock *sk, int wake)
{
	if (sk->sk_wmem_queued >= READ_ONCE(sk->sk_sndbuf))
		return false;

	return sk->sk_prot->stream_memory_free ?
		sk->sk_prot->stream_memory_free(sk, wake) : true;
}
```

_(no external call sites found for `__sk_stream_memory_free`)_

### Function `sk_wmem_free_skb` (L1467–L1479, 13 lines)

```c
static inline void sk_wmem_free_skb(struct sock *sk, struct sk_buff *skb)
{
	sock_set_flag(sk, SOCK_QUEUE_SHRUNK);
	sk->sk_wmem_queued -= skb->truesize;
	sk_mem_uncharge(sk, skb->truesize);
	if (static_branch_unlikely(&tcp_tx_skb_cache_key) &&
	    !sk->sk_tx_skb_cache && !skb_cloned(skb)) {
		skb_zcopy_clear(skb, true);
		sk->sk_tx_skb_cache = skb;
		return;
	}
	__kfree_skb(skb);
}
```

**External callers of `sk_wmem_free_skb` at parent**

- `include/net/tcp.h:1802` — `	sk_wmem_free_skb(sk, skb);`
- `net/ipv4/tcp.c:952` — `		sk_wmem_free_skb(sk, skb);`
- `net/ipv4/tcp.c:2535` — `		sk_wmem_free_skb(sk, skb);`
- `net/ipv4/tcp.c:2546` — `		sk_wmem_free_skb(sk, skb);`
- `net/ipv4/tcp_output.c:2186` — `			sk_wmem_free_skb(sk, skb);`

### Function `skb_copy_to_page_nocache` (L2002–L2020, 19 lines)

```c
static inline int skb_copy_to_page_nocache(struct sock *sk, struct iov_iter *from,
					   struct sk_buff *skb,
					   struct page *page,
					   int off, int copy)
{
	int err;

	err = skb_do_copy_data_nocache(sk, skb, from, page_address(page) + off,
				       copy, skb->len);
	if (err)
		return err;

	skb->len	     += copy;
	skb->data_len	     += copy;
	skb->truesize	     += copy;
	sk->sk_wmem_queued   += copy;
	sk_mem_charge(sk, copy);
	return 0;
}
```

**External callers of `skb_copy_to_page_nocache` at parent**

- `net/ipv4/tcp.c:1337` — `			err = skb_copy_to_page_nocache(sk, &msg->msg_iter, skb,`
- `net/kcm/kcmsock.c:991` — `		err = skb_copy_to_page_nocache(sk, &msg->msg_iter, skb,`

## File: `include/trace/events/sock.h`

- changed old-lines: [118]

## File: `net/core/datagram.c`

- changed old-lines: [643]

### Function `__zerocopy_sg_from_iter` (L615–L657, 43 lines)

```c
int __zerocopy_sg_from_iter(struct sock *sk, struct sk_buff *skb,
			    struct iov_iter *from, size_t length)
{
	int frag = skb_shinfo(skb)->nr_frags;

	while (length && iov_iter_count(from)) {
		struct page *pages[MAX_SKB_FRAGS];
		size_t start;
		ssize_t copied;
		unsigned long truesize;
		int n = 0;

		if (frag == MAX_SKB_FRAGS)
			return -EMSGSIZE;

		copied = iov_iter_get_pages(from, pages, length,
					    MAX_SKB_FRAGS - frag, &start);
		if (copied < 0)
			return -EFAULT;

		iov_iter_advance(from, copied);
		length -= copied;

		truesize = PAGE_ALIGN(copied + start);
		skb->data_len += copied;
		skb->len += copied;
		skb->truesize += truesize;
		if (sk && sk->sk_type == SOCK_STREAM) {
			sk->sk_wmem_queued += truesize;
			sk_mem_charge(sk, truesize);
		} else {
			refcount_add(truesize, &skb->sk->sk_wmem_alloc);
		}
		while (copied) {
			int size = min_t(int, copied, PAGE_SIZE - start);
			skb_fill_page_desc(skb, frag++, pages[n], start, size);
			start = 0;
			copied -= size;
			n++;
		}
	}
	return 0;
}
```

**External callers of `__zerocopy_sg_from_iter` at parent**

- `net/core/datagram.h:12` — `int __zerocopy_sg_from_iter(struct sock *sk, struct sk_buff *skb,`
- `net/core/skbuff.c:1272` — `	return __zerocopy_sg_from_iter(skb->sk, skb, &msg->msg_iter, len);`
- `net/core/skbuff.c:1290` — `	err = __zerocopy_sg_from_iter(sk, skb, &msg->msg_iter, len);`

## File: `net/core/sock.c`

- changed old-lines: [3215]

### Function `sk_get_meminfo` (L3206–L3219, 14 lines)

```c
void sk_get_meminfo(const struct sock *sk, u32 *mem)
{
	memset(mem, 0, sizeof(*mem) * SK_MEMINFO_VARS);

	mem[SK_MEMINFO_RMEM_ALLOC] = sk_rmem_alloc_get(sk);
	mem[SK_MEMINFO_RCVBUF] = READ_ONCE(sk->sk_rcvbuf);
	mem[SK_MEMINFO_WMEM_ALLOC] = sk_wmem_alloc_get(sk);
	mem[SK_MEMINFO_SNDBUF] = READ_ONCE(sk->sk_sndbuf);
	mem[SK_MEMINFO_FWD_ALLOC] = sk->sk_forward_alloc;
	mem[SK_MEMINFO_WMEM_QUEUED] = sk->sk_wmem_queued;
	mem[SK_MEMINFO_OPTMEM] = atomic_read(&sk->sk_omem_alloc);
	mem[SK_MEMINFO_BACKLOG] = READ_ONCE(sk->sk_backlog.len);
	mem[SK_MEMINFO_DROPS] = atomic_read(&sk->sk_drops);
}
```

**External callers of `sk_get_meminfo` at parent**

- `include/net/sock.h:2530` — `void sk_get_meminfo(const struct sock *sk, u32 *meminfo);`
- `net/core/sock_diag.c:64` — `	sk_get_meminfo(sk, mem);`

## File: `net/ipv4/inet_diag.c`

- changed old-lines: [196]

### Function `inet_sk_diag_fill` (L159–L312, 154 lines)

```c
int inet_sk_diag_fill(struct sock *sk, struct inet_connection_sock *icsk,
		      struct sk_buff *skb, const struct inet_diag_req_v2 *req,
		      struct user_namespace *user_ns,
		      u32 portid, u32 seq, u16 nlmsg_flags,
		      const struct nlmsghdr *unlh,
		      bool net_admin)
{
	const struct tcp_congestion_ops *ca_ops;
	const struct inet_diag_handler *handler;
	int ext = req->idiag_ext;
	struct inet_diag_msg *r;
	struct nlmsghdr  *nlh;
	struct nlattr *attr;
	void *info = NULL;

	handler = inet_diag_table[req->sdiag_protocol];
	BUG_ON(!handler);

	nlh = nlmsg_put(skb, portid, seq, unlh->nlmsg_type, sizeof(*r),
			nlmsg_flags);
	if (!nlh)
		return -EMSGSIZE;

	r = nlmsg_data(nlh);
	BUG_ON(!sk_fullsock(sk));

	inet_diag_msg_common_fill(r, sk);
	r->idiag_state = sk->sk_state;
	r->idiag_timer = 0;
	r->idiag_retrans = 0;

	if (inet_diag_msg_attrs_fill(sk, skb, r, ext, user_ns, net_admin))
		goto errout;

	if (ext & (1 << (INET_DIAG_MEMINFO - 1))) {
		struct inet_diag_meminfo minfo = {
			.idiag_rmem = sk_rmem_alloc_get(sk),
			.idiag_wmem = sk->sk_wmem_queued,
			.idiag_fmem = sk->sk_forward_alloc,
			.idiag_tmem = sk_wmem_alloc_get(sk),
		};

		if (nla_put(skb, INET_DIAG_MEMINFO, sizeof(minfo), &minfo) < 0)
			goto errout;
	}

	if (ext & (1 << (INET_DIAG_SKMEMINFO - 1)))
		if (sock_diag_put_meminfo(sk, skb, INET_DIAG_SKMEMINFO))
			goto errout;

	/*
	 * RAW sockets might have user-defined protocols assigned,
	 * so report the one supplied on socket creation.
	 */
	if (sk->sk_type == SOCK_RAW) {
		if (nla_put_u8(skb, INET_DIAG_PROTOCOL, sk->sk_protocol))
			goto errout;
	}

	if (!icsk) {
		handler->idiag_get_info(sk, r, NULL);
		goto out;
	}

	if (icsk->icsk_pending == ICSK_TIME_RETRANS ||
	    icsk->icsk_pending == ICSK_TIME_REO_TIMEOUT ||
	    icsk->icsk_pending == ICSK_TIME_LOSS_PROBE) {
		r->idiag_timer = 1;
		r->idiag_retrans = icsk->icsk_retransmits;
		r->idiag_expires =
			jiffies_to_msecs(icsk->icsk_timeout - jiffies);
	} else if (icsk->icsk_pending == ICSK_TIME_PROBE0) {
		r->idiag_timer = 4;
		r->idiag_retrans = icsk->icsk_probes_out;
		r->idiag_expires =
			jiffies_to_msecs(icsk->icsk_timeout - jiffies);
	} else if (timer_pending(&sk->sk_timer)) {
		r->idiag_timer = 2;
		r->idiag_retrans = icsk->icsk_probes_out;
		r->idiag_expires =
			jiffies_to_msecs(sk->sk_timer.expires - jiffies);
	} else {
		r->idiag_timer = 0;
		r->idiag_expires = 0;
	}

	if ((ext & (1 << (INET_DIAG_INFO - 1))) && handler->idiag_info_size) {
		attr = nla_reserve_64bit(skb, INET_DIAG_INFO,
					 handler->idiag_info_size,
					 INET_DIAG_PAD);
		if (!attr)
			goto errout;

		info = nla_data(attr);
	}

	if (ext & (1 << (INET_DIAG_CONG - 1))) {
		int err = 0;

		rcu_read_lock();
		ca_ops = READ_ONCE(icsk->icsk_ca_ops);
		if (ca_ops)
			err = nla_put_string(skb, INET_DIAG_CONG, ca_ops->name);
		rcu_read_unlock();
		if (err < 0)
			goto errout;
	}

	handler->idiag_get_info(sk, r, info);

	if (ext & (1 << (INET_DIAG_INFO - 1)) && handler->idiag_get_aux)
		if (handler->idiag_get_aux(sk, net_admin, skb) < 0)
			goto errout;

	if (sk->sk_state < TCP_TIME_WAIT) {
		union tcp_cc_info info;
		size_t sz = 0;
		int attr;

		rcu_read_lock();
		ca_ops = READ_ONCE(icsk->icsk_ca_ops);
		if (ca_ops && ca_ops->get_info)
			sz = ca_ops->get_info(sk, ext, &attr, &info);
		rcu_read_unlock();
		if (sz && nla_put(skb, attr, sz, &info) < 0)
			goto errout;
	}

	if (ext & (1 << (INET_DIAG_CLASS_ID - 1)) ||
	    ext & (1 << (INET_DIAG_TCLASS - 1))) {
		u32 classid = 0;

#ifdef CONFIG_SOCK_CGROUP_DATA
		classid = sock_cgroup_classid(&sk->sk_cgrp_data);
#endif
		/* Fallback to socket priority if class id isn't set.
		 * Classful qdiscs use it as direct reference to class.
		 * For cgroup2 classid is always zero.
		 */
		if (!classid)
			classid = sk->sk_priority;

		if (nla_put_u32(skb, INET_DIAG_CLASS_ID, classid))
			goto errout;
	}

out:
	nlmsg_end(skb, nlh);
	return 0;

errout:
	nlmsg_cancel(skb, nlh);
	return -EMSGSIZE;
}
```

**External callers of `inet_sk_diag_fill` at parent**

- `include/linux/inet_diag.h:44` — `int inet_sk_diag_fill(struct sock *sk, struct inet_connection_sock *icsk,`
- `net/ipv4/raw_diag.c:111` — `	err = inet_sk_diag_fill(sk, NULL, rep, r,`
- `net/ipv4/raw_diag.c:139` — `	return inet_sk_diag_fill(sk, NULL, skb, r,`
- `net/ipv4/udp_diag.c:24` — `	return inet_sk_diag_fill(sk, NULL, skb, req,`
- `net/ipv4/udp_diag.c:73` — `	err = inet_sk_diag_fill(sk, NULL, rep, req,`

## File: `net/ipv4/tcp.c`

- changed old-lines: [662, 1037]

### Function `skb_entail` (L651–L668, 18 lines)

```c
static void skb_entail(struct sock *sk, struct sk_buff *skb)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct tcp_skb_cb *tcb = TCP_SKB_CB(skb);

	skb->csum    = 0;
	tcb->seq     = tcb->end_seq = tp->write_seq;
	tcb->tcp_flags = TCPHDR_ACK;
	tcb->sacked  = 0;
	__skb_header_release(skb);
	tcp_add_write_queue_tail(sk, skb);
	sk->sk_wmem_queued += skb->truesize;
	sk_mem_charge(sk, skb->truesize);
	if (tp->nonagle & TCP_NAGLE_PUSH)
		tp->nonagle &= ~TCP_NAGLE_PUSH;

	tcp_slow_start_after_idle_check(sk);
}
```

**External callers of `skb_entail` at parent**

- `drivers/crypto/chelsio/chtls/chtls.h:485` — `void skb_entail(struct sock *sk, struct sk_buff *skb, int flags);`
- `drivers/crypto/chelsio/chtls/chtls_cm.c:279` — `	skb_entail(sk, skb, ULPCB_FLAG_NO_HDR | ULPCB_FLAG_NO_APPEND);`
- `drivers/crypto/chelsio/chtls/chtls_io.c:122` — `		skb_entail(sk, skb,`
- `drivers/crypto/chelsio/chtls/chtls_io.c:819` — `void skb_entail(struct sock *sk, struct sk_buff *skb, int flags)`
- `drivers/crypto/chelsio/chtls/chtls_io.c:843` — `		skb_entail(sk, skb, ULPCB_FLAG_NEED_HDR);`
- `drivers/crypto/chelsio/chtls/chtls_io.c:860` — `		skb_entail(sk, skb, ULPCB_FLAG_NEED_HDR);`

### Function `do_tcp_sendpages` (L956–L1096, 141 lines)

```c
ssize_t do_tcp_sendpages(struct sock *sk, struct page *page, int offset,
			 size_t size, int flags)
{
	struct tcp_sock *tp = tcp_sk(sk);
	int mss_now, size_goal;
	int err;
	ssize_t copied;
	long timeo = sock_sndtimeo(sk, flags & MSG_DONTWAIT);

	if (IS_ENABLED(CONFIG_DEBUG_VM) &&
	    WARN_ONCE(PageSlab(page), "page must not be a Slab one"))
		return -EINVAL;

	/* Wait for a connection to finish. One exception is TCP Fast Open
	 * (passive side) where data is allowed to be sent before a connection
	 * is fully established.
	 */
	if (((1 << sk->sk_state) & ~(TCPF_ESTABLISHED | TCPF_CLOSE_WAIT)) &&
	    !tcp_passive_fastopen(sk)) {
		err = sk_stream_wait_connect(sk, &timeo);
		if (err != 0)
			goto out_err;
	}

	sk_clear_bit(SOCKWQ_ASYNC_NOSPACE, sk);

	mss_now = tcp_send_mss(sk, &size_goal, flags);
	copied = 0;

	err = -EPIPE;
	if (sk->sk_err || (sk->sk_shutdown & SEND_SHUTDOWN))
		goto out_err;

	while (size > 0) {
		struct sk_buff *skb = tcp_write_queue_tail(sk);
		int copy, i;
		bool can_coalesce;

		if (!skb || (copy = size_goal - skb->len) <= 0 ||
		    !tcp_skb_can_collapse_to(skb)) {
new_segment:
			if (!sk_stream_memory_free(sk))
				goto wait_for_sndbuf;

			skb = sk_stream_alloc_skb(sk, 0, sk->sk_allocation,
					tcp_rtx_and_write_queues_empty(sk));
			if (!skb)
				goto wait_for_memory;

#ifdef CONFIG_TLS_DEVICE
			skb->decrypted = !!(flags & MSG_SENDPAGE_DECRYPTED);
#endif
			skb_entail(sk, skb);
			copy = size_goal;
		}

		if (copy > size)
			copy = size;

		i = skb_shinfo(skb)->nr_frags;
		can_coalesce = skb_can_coalesce(skb, i, page, offset);
		if (!can_coalesce && i >= sysctl_max_skb_frags) {
			tcp_mark_push(tp, skb);
			goto new_segment;
		}
		if (!sk_wmem_schedule(sk, copy))
			goto wait_for_memory;

		if (can_coalesce) {
			skb_frag_size_add(&skb_shinfo(skb)->frags[i - 1], copy);
		} else {
			get_page(page);
			skb_fill_page_desc(skb, i, page, offset, copy);
		}

		if (!(flags & MSG_NO_SHARED_FRAGS))
			skb_shinfo(skb)->tx_flags |= SKBTX_SHARED_FRAG;

		skb->len += copy;
		skb->data_len += copy;
		skb->truesize += copy;
		sk->sk_wmem_queued += copy;
		sk_mem_charge(sk, copy);
		skb->ip_summed = CHECKSUM_PARTIAL;
		WRITE_ONCE(tp->write_seq, tp->write_seq + copy);
		TCP_SKB_CB(skb)->end_seq += copy;
		tcp_skb_pcount_set(skb, 0);

		if (!copied)
			TCP_SKB_CB(skb)->tcp_flags &= ~TCPHDR_PSH;

		copied += copy;
		offset += copy;
		size -= copy;
		if (!size)
			goto out;

		if (skb->len < size_goal || (flags & MSG_OOB))
			continue;

		if (forced_push(tp)) {
			tcp_mark_push(tp, skb);
			__tcp_push_pending_frames(sk, mss_now, TCP_NAGLE_PUSH);
		} else if (skb == tcp_send_head(sk))
			tcp_push_one(sk, mss_now);
		continue;

wait_for_sndbuf:
		set_bit(SOCK_NOSPACE, &sk->sk_socket->flags);
wait_for_memory:
		tcp_push(sk, flags & ~MSG_MORE, mss_now,
			 TCP_NAGLE_PUSH, size_goal);

		err = sk_stream_wait_memory(sk, &timeo);
		if (err != 0)
			goto do_error;

		mss_now = tcp_send_mss(sk, &size_goal, flags);
	}

out:
	if (copied) {
		tcp_tx_timestamp(sk, sk->sk_tsflags);
		if (!(flags & MSG_SENDPAGE_NOTLAST))
			tcp_push(sk, flags, mss_now, tp->nonagle, size_goal);
	}
	return copied;

do_error:
	tcp_remove_empty_skb(sk, tcp_write_queue_tail(sk));
	if (copied)
		goto out;
out_err:
	/* make sure we wake any epoll edge trigger waiter */
	if (unlikely(skb_queue_len(&sk->sk_write_queue) == 0 &&
		     err == -EAGAIN)) {
		sk->sk_write_space(sk);
		tcp_chrono_stop(sk, TCP_CHRONO_SNDBUF_LIMITED);
	}
	return sk_stream_error(sk, flags, err);
}
```

**External callers of `do_tcp_sendpages` at parent**

- `drivers/infiniband/sw/siw/siw_qp_tx.c:316` — ` * 0copy TCP transmit interface: Use do_tcp_sendpages.`
- `drivers/infiniband/sw/siw/siw_qp_tx.c:340` — `		rv = do_tcp_sendpages(sk, page[i], offset, bytes, flags);`
- `include/net/tcp.h:329` — `ssize_t do_tcp_sendpages(struct sock *sk, struct page *page, int offset,`
- `net/ipv4/tcp_bpf.c:241` — `			ret = do_tcp_sendpages(sk, page, off, size, flags);`
- `net/ipv4/tcp_bpf.c:406` — `	/* Don't let internal do_tcp_sendpages() flags through */`
- `net/tls/tls_main.c:123` — `		ret = do_tcp_sendpages(sk, p, offset, size, sendpage_flags);`
- `net/tls/tls_main.c:238` — `	 * if do_tcp_sendpages where to call sk_wait_event.`
