# Evidence: SYZBOT-6a2a295ae3340e8e  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in tcp_sendmsg_locked / tcp_stream_memory_free
- mainline fix commit: `0f31746452e6793ad6271337438af8f4defb8940`
- candidate JSON: `A_SYZBOT-6a2a295ae3340e8e.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "tcp_sendmsg_locked", "fn_b": "tcp_stream_memory_free"}`

## Commit message

```
0f31746452e6793ad6271337438af8f4defb8940
tcp: annotate tp->write_seq lockless reads

There are few places where we fetch tp->write_seq while
this field can change from IRQ or other cpu.

We need to add READ_ONCE() annotations, and also make
sure write sides use corresponding WRITE_ONCE() to avoid
store-tearing.

Signed-off-by: Eric Dumazet <edumazet@google.com>
Signed-off-by: David S. Miller <davem@davemloft.net>
```

## CVE / bug description

```
KCSAN: data-race in tcp_sendmsg_locked / tcp_stream_memory_free
```

## Full mainline patch

```diff
commit 0f31746452e6793ad6271337438af8f4defb8940
Author: Eric Dumazet <edumazet@google.com>
Date:   Thu Oct 10 20:17:41 2019 -0700

    tcp: annotate tp->write_seq lockless reads
    
    There are few places where we fetch tp->write_seq while
    this field can change from IRQ or other cpu.
    
    We need to add READ_ONCE() annotations, and also make
    sure write sides use corresponding WRITE_ONCE() to avoid
    store-tearing.
    
    Signed-off-by: Eric Dumazet <edumazet@google.com>
    Signed-off-by: David S. Miller <davem@davemloft.net>

diff --git a/include/net/tcp.h b/include/net/tcp.h
index 35f6f7e0fdc2..8e7c3f6801a9 100644
--- a/include/net/tcp.h
+++ b/include/net/tcp.h
@@ -1917,7 +1917,7 @@ static inline u32 tcp_notsent_lowat(const struct tcp_sock *tp)
 static inline bool tcp_stream_memory_free(const struct sock *sk, int wake)
 {
 	const struct tcp_sock *tp = tcp_sk(sk);
-	u32 notsent_bytes = tp->write_seq - tp->snd_nxt;
+	u32 notsent_bytes = READ_ONCE(tp->write_seq) - tp->snd_nxt;
 
 	return (notsent_bytes << wake) < tcp_notsent_lowat(tp);
 }
diff --git a/net/ipv4/tcp.c b/net/ipv4/tcp.c
index c322ad071e17..96dd65cbeb85 100644
--- a/net/ipv4/tcp.c
+++ b/net/ipv4/tcp.c
@@ -616,7 +616,7 @@ int tcp_ioctl(struct sock *sk, int cmd, unsigned long arg)
 		if ((1 << sk->sk_state) & (TCPF_SYN_SENT | TCPF_SYN_RECV))
 			answ = 0;
 		else
-			answ = tp->write_seq - tp->snd_una;
+			answ = READ_ONCE(tp->write_seq) - tp->snd_una;
 		break;
 	case SIOCOUTQNSD:
 		if (sk->sk_state == TCP_LISTEN)
@@ -625,7 +625,7 @@ int tcp_ioctl(struct sock *sk, int cmd, unsigned long arg)
 		if ((1 << sk->sk_state) & (TCPF_SYN_SENT | TCPF_SYN_RECV))
 			answ = 0;
 		else
-			answ = tp->write_seq - tp->snd_nxt;
+			answ = READ_ONCE(tp->write_seq) - tp->snd_nxt;
 		break;
 	default:
 		return -ENOIOCTLCMD;
@@ -1035,7 +1035,7 @@ ssize_t do_tcp_sendpages(struct sock *sk, struct page *page, int offset,
 		sk->sk_wmem_queued += copy;
 		sk_mem_charge(sk, copy);
 		skb->ip_summed = CHECKSUM_PARTIAL;
-		tp->write_seq += copy;
+		WRITE_ONCE(tp->write_seq, tp->write_seq + copy);
 		TCP_SKB_CB(skb)->end_seq += copy;
 		tcp_skb_pcount_set(skb, 0);
 
@@ -1362,7 +1362,7 @@ int tcp_sendmsg_locked(struct sock *sk, struct msghdr *msg, size_t size)
 		if (!copied)
 			TCP_SKB_CB(skb)->tcp_flags &= ~TCPHDR_PSH;
 
-		tp->write_seq += copy;
+		WRITE_ONCE(tp->write_seq, tp->write_seq + copy);
 		TCP_SKB_CB(skb)->end_seq += copy;
 		tcp_skb_pcount_set(skb, 0);
 
@@ -2562,6 +2562,7 @@ int tcp_disconnect(struct sock *sk, int flags)
 	struct inet_connection_sock *icsk = inet_csk(sk);
 	struct tcp_sock *tp = tcp_sk(sk);
 	int old_state = sk->sk_state;
+	u32 seq;
 
 	if (old_state != TCP_CLOSE)
 		tcp_set_state(sk, TCP_CLOSE);
@@ -2604,9 +2605,12 @@ int tcp_disconnect(struct sock *sk, int flags)
 	tp->srtt_us = 0;
 	tp->mdev_us = jiffies_to_usecs(TCP_TIMEOUT_INIT);
 	tp->rcv_rtt_last_tsecr = 0;
-	tp->write_seq += tp->max_window + 2;
-	if (tp->write_seq == 0)
-		tp->write_seq = 1;
+
+	seq = tp->write_seq + tp->max_window + 2;
+	if (!seq)
+		seq = 1;
+	WRITE_ONCE(tp->write_seq, seq);
+
 	icsk->icsk_backoff = 0;
 	tp->snd_cwnd = 2;
 	icsk->icsk_probes_out = 0;
@@ -2933,7 +2937,7 @@ static int do_tcp_setsockopt(struct sock *sk, int level,
 		if (sk->sk_state != TCP_CLOSE)
 			err = -EPERM;
 		else if (tp->repair_queue == TCP_SEND_QUEUE)
-			tp->write_seq = val;
+			WRITE_ONCE(tp->write_seq, val);
 		else if (tp->repair_queue == TCP_RECV_QUEUE)
 			WRITE_ONCE(tp->rcv_nxt, val);
 		else
diff --git a/net/ipv4/tcp_diag.c b/net/ipv4/tcp_diag.c
index 66273c8a55c2..549506162dde 100644
--- a/net/ipv4/tcp_diag.c
+++ b/net/ipv4/tcp_diag.c
@@ -28,7 +28,7 @@ static void tcp_diag_get_info(struct sock *sk, struct inet_diag_msg *r,
 
 		r->idiag_rqueue = max_t(int, READ_ONCE(tp->rcv_nxt) -
 					     READ_ONCE(tp->copied_seq), 0);
-		r->idiag_wqueue = tp->write_seq - tp->snd_una;
+		r->idiag_wqueue = READ_ONCE(tp->write_seq) - tp->snd_una;
 	}
 	if (info)
 		tcp_get_info(sk, info);
diff --git a/net/ipv4/tcp_ipv4.c b/net/ipv4/tcp_ipv4.c
index 39560f482e0b..6be568334848 100644
--- a/net/ipv4/tcp_ipv4.c
+++ b/net/ipv4/tcp_ipv4.c
@@ -164,9 +164,11 @@ int tcp_twsk_unique(struct sock *sk, struct sock *sktw, void *twp)
 		 * without appearing to create any others.
 		 */
 		if (likely(!tp->repair)) {
-			tp->write_seq = tcptw->tw_snd_nxt + 65535 + 2;
-			if (tp->write_seq == 0)
-				tp->write_seq = 1;
+			u32 seq = tcptw->tw_snd_nxt + 65535 + 2;
+
+			if (!seq)
+				seq = 1;
+			WRITE_ONCE(tp->write_seq, seq);
 			tp->rx_opt.ts_recent	   = tcptw->tw_ts_recent;
 			tp->rx_opt.ts_recent_stamp = tcptw->tw_ts_recent_stamp;
 		}
@@ -253,7 +255,7 @@ int tcp_v4_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len)
 		tp->rx_opt.ts_recent	   = 0;
 		tp->rx_opt.ts_recent_stamp = 0;
 		if (likely(!tp->repair))
-			tp->write_seq	   = 0;
+			WRITE_ONCE(tp->write_seq, 0);
 	}
 
 	inet->inet_dport = usin->sin_port;
@@ -291,10 +293,11 @@ int tcp_v4_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len)
 
 	if (likely(!tp->repair)) {
 		if (!tp->write_seq)
-			tp->write_seq = secure_tcp_seq(inet->inet_saddr,
-						       inet->inet_daddr,
-						       inet->inet_sport,
-						       usin->sin_port);
+			WRITE_ONCE(tp->write_seq,
+				   secure_tcp_seq(inet->inet_saddr,
+						  inet->inet_daddr,
+						  inet->inet_sport,
+						  usin->sin_port));
 		tp->tsoffset = secure_tcp_ts_off(sock_net(sk),
 						 inet->inet_saddr,
 						 inet->inet_daddr);
@@ -2461,7 +2464,7 @@ static void get_tcp4_sock(struct sock *sk, struct seq_file *f, int i)
 	seq_printf(f, "%4d: %08X:%04X %08X:%04X %02X %08X:%08X %02X:%08lX "
 			"%08X %5u %8d %lu %d %pK %lu %lu %u %u %d",
 		i, src, srcp, dest, destp, state,
-		tp->write_seq - tp->snd_una,
+		READ_ONCE(tp->write_seq) - tp->snd_una,
 		rx_queue,
 		timer_active,
 		jiffies_delta_to_clock_t(timer_expires - jiffies),
diff --git a/net/ipv4/tcp_minisocks.c b/net/ipv4/tcp_minisocks.c
index c4731d26ab4a..339944690329 100644
--- a/net/ipv4/tcp_minisocks.c
+++ b/net/ipv4/tcp_minisocks.c
@@ -498,7 +498,7 @@ struct sock *tcp_create_openreq_child(const struct sock *sk,
 	newtp->total_retrans = req->num_retrans;
 
 	tcp_init_xmit_timers(newsk);
-	newtp->write_seq = newtp->pushed_seq = treq->snt_isn + 1;
+	WRITE_ONCE(newtp->write_seq, newtp->pushed_seq = treq->snt_isn + 1);
 
 	if (sock_flag(newsk, SOCK_KEEPOPEN))
 		inet_csk_reset_keepalive_timer(newsk,
diff --git a/net/ipv4/tcp_output.c b/net/ipv4/tcp_output.c
index 7dda12720169..c17c2a78809d 100644
--- a/net/ipv4/tcp_output.c
+++ b/net/ipv4/tcp_output.c
@@ -1196,7 +1196,7 @@ static void tcp_queue_skb(struct sock *sk, struct sk_buff *skb)
 	struct tcp_sock *tp = tcp_sk(sk);
 
 	/* Advance write_seq and place onto the write_queue. */
-	tp->write_seq = TCP_SKB_CB(skb)->end_seq;
+	WRITE_ONCE(tp->write_seq, TCP_SKB_CB(skb)->end_seq);
 	__skb_header_release(skb);
 	tcp_add_write_queue_tail(sk, skb);
 	sk->sk_wmem_queued += skb->truesize;
@@ -3449,7 +3449,7 @@ static void tcp_connect_queue_skb(struct sock *sk, struct sk_buff *skb)
 	__skb_header_release(skb);
 	sk->sk_wmem_queued += skb->truesize;
 	sk_mem_charge(sk, skb->truesize);
-	tp->write_seq = tcb->end_seq;
+	WRITE_ONCE(tp->write_seq, tcb->end_seq);
 	tp->packets_out += tcp_skb_pcount(skb);
 }
 
diff --git a/net/ipv6/tcp_ipv6.c b/net/ipv6/tcp_ipv6.c
index a62c7042fc4a..4804b6dc5e65 100644
--- a/net/ipv6/tcp_ipv6.c
+++ b/net/ipv6/tcp_ipv6.c
@@ -215,7 +215,7 @@ static int tcp_v6_connect(struct sock *sk, struct sockaddr *uaddr,
 	    !ipv6_addr_equal(&sk->sk_v6_daddr, &usin->sin6_addr)) {
 		tp->rx_opt.ts_recent = 0;
 		tp->rx_opt.ts_recent_stamp = 0;
-		tp->write_seq = 0;
+		WRITE_ONCE(tp->write_seq, 0);
 	}
 
 	sk->sk_v6_daddr = usin->sin6_addr;
@@ -311,10 +311,11 @@ static int tcp_v6_connect(struct sock *sk, struct sockaddr *uaddr,
 
 	if (likely(!tp->repair)) {
 		if (!tp->write_seq)
-			tp->write_seq = secure_tcpv6_seq(np->saddr.s6_addr32,
-							 sk->sk_v6_daddr.s6_addr32,
-							 inet->inet_sport,
-							 inet->inet_dport);
+			WRITE_ONCE(tp->write_seq,
+				   secure_tcpv6_seq(np->saddr.s6_addr32,
+						    sk->sk_v6_daddr.s6_addr32,
+						    inet->inet_sport,
+						    inet->inet_dport));
 		tp->tsoffset = secure_tcpv6_ts_off(sock_net(sk),
 						   np->saddr.s6_addr32,
 						   sk->sk_v6_daddr.s6_addr32);
@@ -1907,7 +1908,7 @@ static void get_tcp6_sock(struct seq_file *seq, struct sock *sp, int i)
 		   dest->s6_addr32[0], dest->s6_addr32[1],
 		   dest->s6_addr32[2], dest->s6_addr32[3], destp,
 		   state,
-		   tp->write_seq - tp->snd_una,
+		   READ_ONCE(tp->write_seq) - tp->snd_una,
 		   rx_queue,
 		   timer_active,
 		   jiffies_delta_to_clock_t(timer_expires - jiffies),

```

## File: `include/net/tcp.h`

- changed old-lines: [1920]

### Function `tcp_stream_memory_free` (L1917–L1923, 7 lines)

```c
static inline bool tcp_stream_memory_free(const struct sock *sk, int wake)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	u32 notsent_bytes = tp->write_seq - tp->snd_nxt;

	return (notsent_bytes << wake) < tcp_notsent_lowat(tp);
}
```

**External callers of `tcp_stream_memory_free` at parent**

- `net/ipv4/tcp_ipv4.c:2593` — `	.stream_memory_free	= tcp_stream_memory_free,`
- `net/ipv6/tcp_ipv6.c:2030` — `	.stream_memory_free	= tcp_stream_memory_free,`

## File: `net/ipv4/tcp.c`

- changed old-lines: [619, 628, 1038, 1365, 2607, 2608, 2609, 2936]

### Function `tcp_ioctl` (L594–L635, 42 lines)

```c
int tcp_ioctl(struct sock *sk, int cmd, unsigned long arg)
{
	struct tcp_sock *tp = tcp_sk(sk);
	int answ;
	bool slow;

	switch (cmd) {
	case SIOCINQ:
		if (sk->sk_state == TCP_LISTEN)
			return -EINVAL;

		slow = lock_sock_fast(sk);
		answ = tcp_inq(sk);
		unlock_sock_fast(sk, slow);
		break;
	case SIOCATMARK:
		answ = tp->urg_data && tp->urg_seq == READ_ONCE(tp->copied_seq);
		break;
	case SIOCOUTQ:
		if (sk->sk_state == TCP_LISTEN)
			return -EINVAL;

		if ((1 << sk->sk_state) & (TCPF_SYN_SENT | TCPF_SYN_RECV))
			answ = 0;
		else
			answ = tp->write_seq - tp->snd_una;
		break;
	case SIOCOUTQNSD:
		if (sk->sk_state == TCP_LISTEN)
			return -EINVAL;

		if ((1 << sk->sk_state) & (TCPF_SYN_SENT | TCPF_SYN_RECV))
			answ = 0;
		else
			answ = tp->write_seq - tp->snd_nxt;
		break;
	default:
		return -ENOIOCTLCMD;
	}

	return put_user(answ, (int __user *)arg);
}
```

**External callers of `tcp_ioctl` at parent**

- `include/net/tcp.h:335` — `int tcp_ioctl(struct sock *sk, int cmd, unsigned long arg);`
- `net/ipv4/tcp_ipv4.c:2576` — `	.ioctl			= tcp_ioctl,`
- `net/ipv6/tcp_ipv6.c:2013` — `	.ioctl			= tcp_ioctl,`

### Function `do_tcp_sendpages` (L954–L1094, 141 lines)

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
		tp->write_seq += copy;
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

### Function `tcp_sendmsg_locked` (L1177–L1426, 250 lines)

```c
int tcp_sendmsg_locked(struct sock *sk, struct msghdr *msg, size_t size)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct ubuf_info *uarg = NULL;
	struct sk_buff *skb;
	struct sockcm_cookie sockc;
	int flags, err, copied = 0;
	int mss_now = 0, size_goal, copied_syn = 0;
	int process_backlog = 0;
	bool zc = false;
	long timeo;

	flags = msg->msg_flags;

	if (flags & MSG_ZEROCOPY && size && sock_flag(sk, SOCK_ZEROCOPY)) {
		skb = tcp_write_queue_tail(sk);
		uarg = sock_zerocopy_realloc(sk, size, skb_zcopy(skb));
		if (!uarg) {
			err = -ENOBUFS;
			goto out_err;
		}

		zc = sk->sk_route_caps & NETIF_F_SG;
		if (!zc)
			uarg->zerocopy = 0;
	}

	if (unlikely(flags & MSG_FASTOPEN || inet_sk(sk)->defer_connect) &&
	    !tp->repair) {
		err = tcp_sendmsg_fastopen(sk, msg, &copied_syn, size, uarg);
		if (err == -EINPROGRESS && copied_syn > 0)
			goto out;
		else if (err)
			goto out_err;
	}

	timeo = sock_sndtimeo(sk, flags & MSG_DONTWAIT);

	tcp_rate_check_app_limited(sk);  /* is sending application-limited? */

	/* Wait for a connection to finish. One exception is TCP Fast Open
	 * (passive side) where data is allowed to be sent before a connection
	 * is fully established.
	 */
	if (((1 << sk->sk_state) & ~(TCPF_ESTABLISHED | TCPF_CLOSE_WAIT)) &&
	    !tcp_passive_fastopen(sk)) {
		err = sk_stream_wait_connect(sk, &timeo);
		if (err != 0)
			goto do_error;
	}

	if (unlikely(tp->repair)) {
		if (tp->repair_queue == TCP_RECV_QUEUE) {
			copied = tcp_send_rcvq(sk, msg, size);
			goto out_nopush;
		}

		err = -EINVAL;
		if (tp->repair_queue == TCP_NO_QUEUE)
			goto out_err;

		/* 'common' sending to sendq */
	}

	sockcm_init(&sockc, sk);
	if (msg->msg_controllen) {
		err = sock_cmsg_send(sk, msg, &sockc);
		if (unlikely(err)) {
			err = -EINVAL;
			goto out_err;
		}
	}

	/* This should be in poll */
	sk_clear_bit(SOCKWQ_ASYNC_NOSPACE, sk);

	/* Ok commence sending. */
	copied = 0;

restart:
	mss_now = tcp_send_mss(sk, &size_goal, flags);

	err = -EPIPE;
	if (sk->sk_err || (sk->sk_shutdown & SEND_SHUTDOWN))
		goto do_error;

	while (msg_data_left(msg)) {
		int copy = 0;

		skb = tcp_write_queue_tail(sk);
		if (skb)
			copy = size_goal - skb->len;

		if (copy <= 0 || !tcp_skb_can_collapse_to(skb)) {
			bool first_skb;

new_segment:
			if (!sk_stream_memory_free(sk))
				goto wait_for_sndbuf;

			if (unlikely(process_backlog >= 16)) {
				process_backlog = 0;
				if (sk_flush_backlog(sk))
					goto restart;
			}
			first_skb = tcp_rtx_and_write_queues_empty(sk);
			skb = sk_stream_alloc_skb(sk, 0, sk->sk_allocation,
						  first_skb);
			if (!skb)
				goto wait_for_memory;

			process_backlog++;
			skb->ip_summed = CHECKSUM_PARTIAL;

			skb_entail(sk, skb);
			copy = size_goal;

			/* All packets are restored as if they have
			 * already been sent. skb_mstamp_ns isn't set to
			 * avoid wrong rtt estimation.
			 */
			if (tp->repair)
				TCP_SKB_CB(skb)->sacked |= TCPCB_REPAIRED;
		}

		/* Try to append data to the end of skb. */
		if (copy > msg_data_left(msg))
			copy = msg_data_left(msg);

		/* Where to copy to? */
		if (skb_availroom(skb) > 0 && !zc) {
			/* We have some space in skb head. Superb! */
			copy = min_t(int, copy, skb_availroom(skb));
			err = skb_add_data_nocache(sk, skb, &msg->msg_iter, copy);
			if (err)
				goto do_fault;
		} else if (!zc) {
			bool merge = true;
			int i = skb_shinfo(skb)->nr_frags;
			struct page_frag *pfrag = sk_page_frag(sk);

			if (!sk_page_frag_refill(sk, pfrag))
				goto wait_for_memory;

			if (!skb_can_coalesce(skb, i, pfrag->page,
					      pfrag->offset)) {
				if (i >= sysctl_max_skb_frags) {
					tcp_mark_push(tp, skb);
					goto new_segment;
				}
				merge = false;
			}

			copy = min_t(int, copy, pfrag->size - pfrag->offset);

			if (!sk_wmem_schedule(sk, copy))
				goto wait_for_memory;

			err = skb_copy_to_page_nocache(sk, &msg->msg_iter, skb,
						       pfrag->page,
						       pfrag->offset,
						       copy);
			if (err)
				goto do_error;

			/* Update the skb. */
			if (merge) {
				skb_frag_size_add(&skb_shinfo(skb)->frags[i - 1], copy);
			} else {
				skb_fill_page_desc(skb, i, pfrag->page,
						   pfrag->offset, copy);
				page_ref_inc(pfrag->page);
			}
			pfrag->offset += copy;
		} else {
			err = skb_zerocopy_iter_stream(sk, skb, msg, copy, uarg);
			if (err == -EMSGSIZE || err == -EEXIST) {
				tcp_mark_push(tp, skb);
				goto new_segment;
			}
			if (err < 0)
				goto do_error;
			copy = err;
		}

		if (!copied)
			TCP_SKB_CB(skb)->tcp_flags &= ~TCPHDR_PSH;

		tp->write_seq += copy;
		TCP_SKB_CB(skb)->end_seq += copy;
		tcp_skb_pcount_set(skb, 0);

		copied += copy;
		if (!msg_data_left(msg)) {
			if (unlikely(flags & MSG_EOR))
				TCP_SKB_CB(skb)->eor = 1;
			goto out;
		}

		if (skb->len < size_goal || (flags & MSG_OOB) || unlikely(tp->repair))
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
		if (copied)
			tcp_push(sk, flags & ~MSG_MORE, mss_now,
				 TCP_NAGLE_PUSH, size_goal);

		err = sk_stream_wait_memory(sk, &timeo);
		if (err != 0)
			goto do_error;

		mss_now = tcp_send_mss(sk, &size_goal, flags);
	}

out:
	if (copied) {
		tcp_tx_timestamp(sk, sockc.tsflags);
		tcp_push(sk, flags, mss_now, tp->nonagle, size_goal);
	}
out_nopush:
	sock_zerocopy_put(uarg);
	return copied + copied_syn;

do_error:
	skb = tcp_write_queue_tail(sk);
do_fault:
	tcp_remove_empty_skb(sk, skb);

	if (copied + copied_syn)
		goto out;
out_err:
	sock_zerocopy_put_abort(uarg, true);
	err = sk_stream_error(sk, flags, err);
	/* make sure we wake any epoll edge trigger waiter */
	if (unlikely(skb_queue_len(&sk->sk_write_queue) == 0 &&
		     err == -EAGAIN)) {
		sk->sk_write_space(sk);
		tcp_chrono_stop(sk, TCP_CHRONO_SNDBUF_LIMITED);
	}
	return err;
}
```

**External callers of `tcp_sendmsg_locked` at parent**

- `include/net/tcp.h:324` — `int tcp_sendmsg_locked(struct sock *sk, struct msghdr *msg, size_t size);`
- `net/ipv4/af_inet.c:1007` — `	.sendmsg_locked    = tcp_sendmsg_locked,`
- `net/ipv6/af_inet6.c:622` — `	.sendmsg_locked    = tcp_sendmsg_locked,`

### Function `tcp_disconnect` (L2559–L2674, 116 lines)

```c
int tcp_disconnect(struct sock *sk, int flags)
{
	struct inet_sock *inet = inet_sk(sk);
	struct inet_connection_sock *icsk = inet_csk(sk);
	struct tcp_sock *tp = tcp_sk(sk);
	int old_state = sk->sk_state;

	if (old_state != TCP_CLOSE)
		tcp_set_state(sk, TCP_CLOSE);

	/* ABORT function of RFC793 */
	if (old_state == TCP_LISTEN) {
		inet_csk_listen_stop(sk);
	} else if (unlikely(tp->repair)) {
		sk->sk_err = ECONNABORTED;
	} else if (tcp_need_reset(old_state) ||
		   (tp->snd_nxt != tp->write_seq &&
		    (1 << old_state) & (TCPF_CLOSING | TCPF_LAST_ACK))) {
		/* The last check adjusts for discrepancy of Linux wrt. RFC
		 * states
		 */
		tcp_send_active_reset(sk, gfp_any());
		sk->sk_err = ECONNRESET;
	} else if (old_state == TCP_SYN_SENT)
		sk->sk_err = ECONNRESET;

	tcp_clear_xmit_timers(sk);
	__skb_queue_purge(&sk->sk_receive_queue);
	if (sk->sk_rx_skb_cache) {
		__kfree_skb(sk->sk_rx_skb_cache);
		sk->sk_rx_skb_cache = NULL;
	}
	WRITE_ONCE(tp->copied_seq, tp->rcv_nxt);
	tp->urg_data = 0;
	tcp_write_queue_purge(sk);
	tcp_fastopen_active_disable_ofo_check(sk);
	skb_rbtree_purge(&tp->out_of_order_queue);

	inet->inet_dport = 0;

	if (!(sk->sk_userlocks & SOCK_BINDADDR_LOCK))
		inet_reset_saddr(sk);

	sk->sk_shutdown = 0;
	sock_reset_flag(sk, SOCK_DONE);
	tp->srtt_us = 0;
	tp->mdev_us = jiffies_to_usecs(TCP_TIMEOUT_INIT);
	tp->rcv_rtt_last_tsecr = 0;
	tp->write_seq += tp->max_window + 2;
	if (tp->write_seq == 0)
		tp->write_seq = 1;
	icsk->icsk_backoff = 0;
	tp->snd_cwnd = 2;
	icsk->icsk_probes_out = 0;
	icsk->icsk_rto = TCP_TIMEOUT_INIT;
	tp->snd_ssthresh = TCP_INFINITE_SSTHRESH;
	tp->snd_cwnd = TCP_INIT_CWND;
	tp->snd_cwnd_cnt = 0;
	tp->window_clamp = 0;
	tp->delivered_ce = 0;
	tcp_set_ca_state(sk, TCP_CA_Open);
	tp->is_sack_reneg = 0;
	tcp_clear_retrans(tp);
	inet_csk_delack_init(sk);
	/* Initialize rcv_mss to TCP_MIN_MSS to avoid division by 0
	 * issue in __tcp_select_window()
	 */
	icsk->icsk_ack.rcv_mss = TCP_MIN_MSS;
	memset(&tp->rx_opt, 0, sizeof(tp->rx_opt));
	__sk_dst_reset(sk);
	dst_release(sk->sk_rx_dst);
	sk->sk_rx_dst = NULL;
	tcp_saved_syn_free(tp);
	tp->compressed_ack = 0;
	tp->bytes_sent = 0;
	tp->bytes_acked = 0;
	tp->bytes_received = 0;
	tp->bytes_retrans = 0;
	tp->duplicate_sack[0].start_seq = 0;
	tp->duplicate_sack[0].end_seq = 0;
	tp->dsack_dups = 0;
	tp->reord_seen = 0;
	tp->retrans_out = 0;
	tp->sacked_out = 0;
	tp->tlp_high_seq = 0;
	tp->last_oow_ack_time = 0;
	/* There's a bubble in the pipe until at least the first ACK. */
	tp->app_limited = ~0U;
	tp->rack.mstamp = 0;
	tp->rack.advanced = 0;
	tp->rack.reo_wnd_steps = 1;
	tp->rack.last_delivered = 0;
	tp->rack.reo_wnd_persist = 0;
	tp->rack.dsack_seen = 0;
	tp->syn_data_acked = 0;
	tp->rx_opt.saw_tstamp = 0;
	tp->rx_opt.dsack = 0;
	tp->rx_opt.num_sacks = 0;
	tp->rcv_ooopack = 0;


	/* Clean up fastopen related fields */
	tcp_free_fastopen_req(tp);
	inet->defer_connect = 0;

	WARN_ON(inet->inet_num && !icsk->icsk_bind_hash);

	if (sk->sk_frag.page) {
		put_page(sk->sk_frag.page);
		sk->sk_frag.page = NULL;
		sk->sk_frag.offset = 0;
	}

	sk->sk_error_report(sk);
	return 0;
}
```

**External callers of `tcp_disconnect` at parent**

- `drivers/crypto/chelsio/chtls/chtls_cm.c:425` — `	return tcp_disconnect(sk, flags);`
- `include/net/request_sock.h:144` — ` *	temporarily through shutdown()->tcp_disconnect(), and re-enabled later.`
- `include/net/tcp.h:456` — `int tcp_disconnect(struct sock *sk, int flags);`
- `net/ipv4/tcp_ipv4.c:2574` — `	.disconnect		= tcp_disconnect,`
- `net/ipv6/tcp_ipv6.c:2011` — `	.disconnect		= tcp_disconnect,`

### Function `do_tcp_setsockopt` (L2782–L3143, 362 lines)

```c
static int do_tcp_setsockopt(struct sock *sk, int level,
		int optname, char __user *optval, unsigned int optlen)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct inet_connection_sock *icsk = inet_csk(sk);
	struct net *net = sock_net(sk);
	int val;
	int err = 0;

	/* These are data/string values, all the others are ints */
	switch (optname) {
	case TCP_CONGESTION: {
		char name[TCP_CA_NAME_MAX];

		if (optlen < 1)
			return -EINVAL;

		val = strncpy_from_user(name, optval,
					min_t(long, TCP_CA_NAME_MAX-1, optlen));
		if (val < 0)
			return -EFAULT;
		name[val] = 0;

		lock_sock(sk);
		err = tcp_set_congestion_control(sk, name, true, true,
						 ns_capable(sock_net(sk)->user_ns,
							    CAP_NET_ADMIN));
		release_sock(sk);
		return err;
	}
	case TCP_ULP: {
		char name[TCP_ULP_NAME_MAX];

		if (optlen < 1)
			return -EINVAL;

		val = strncpy_from_user(name, optval,
					min_t(long, TCP_ULP_NAME_MAX - 1,
					      optlen));
		if (val < 0)
			return -EFAULT;
		name[val] = 0;

		lock_sock(sk);
		err = tcp_set_ulp(sk, name);
		release_sock(sk);
		return err;
	}
	case TCP_FASTOPEN_KEY: {
		__u8 key[TCP_FASTOPEN_KEY_BUF_LENGTH];
		__u8 *backup_key = NULL;

		/* Allow a backup key as well to facilitate key rotation
		 * First key is the active one.
		 */
		if (optlen != TCP_FASTOPEN_KEY_LENGTH &&
		    optlen != TCP_FASTOPEN_KEY_BUF_LENGTH)
			return -EINVAL;

		if (copy_from_user(key, optval, optlen))
			return -EFAULT;

		if (optlen == TCP_FASTOPEN_KEY_BUF_LENGTH)
			backup_key = key + TCP_FASTOPEN_KEY_LENGTH;

		return tcp_fastopen_reset_cipher(net, sk, key, backup_key);
	}
	default:
		/* fallthru */
		break;
	}

	if (optlen < sizeof(int))
		return -EINVAL;

	if (get_user(val, (int __user *)optval))
		return -EFAULT;

	lock_sock(sk);

	switch (optname) {
	case TCP_MAXSEG:
		/* Values greater than interface MTU won't take effect. However
		 * at the point when this call is done we typically don't yet
		 * know which interface is going to be used
		 */
		if (val && (val < TCP_MIN_MSS || val > MAX_TCP_WINDOW)) {
			err = -EINVAL;
			break;
		}
		tp->rx_opt.user_mss = val;
		break;

	case TCP_NODELAY:
		if (val) {
			/* TCP_NODELAY is weaker than TCP_CORK, so that
			 * this option on corked socket is remembered, but
			 * it is not activated until cork is cleared.
			 *
			 * However, when TCP_NODELAY is set we make
			 * an explicit push, which overrides even TCP_CORK
			 * for currently queued segments.
			 */
			tp->nonagle |= TCP_NAGLE_OFF|TCP_NAGLE_PUSH;
			tcp_push_pending_frames(sk);
		} else {
			tp->nonagle &= ~TCP_NAGLE_OFF;
		}
		break;

	case TCP_THIN_LINEAR_TIMEOUTS:
		if (val < 0 || val > 1)
			err = -EINVAL;
		else
			tp->thin_lto = val;
		break;

	case TCP_THIN_DUPACK:
		if (val < 0 || val > 1)
			err = -EINVAL;
		break;

	case TCP_REPAIR:
		if (!tcp_can_repair_sock(sk))
			err = -EPERM;
		else if (val == TCP_REPAIR_ON) {
			tp->repair = 1;
			sk->sk_reuse = SK_FORCE_REUSE;
			tp->repair_queue = TCP_NO_QUEUE;
		} else if (val == TCP_REPAIR_OFF) {
			tp->repair = 0;
			sk->sk_reuse = SK_NO_REUSE;
			tcp_send_window_probe(sk);
		} else if (val == TCP_REPAIR_OFF_NO_WP) {
			tp->repair = 0;
			sk->sk_reuse = SK_NO_REUSE;
		} else
			err = -EINVAL;

		break;

	case TCP_REPAIR_QUEUE:
		if (!tp->repair)
			err = -EPERM;
		else if ((unsigned int)val < TCP_QUEUES_NR)
			tp->repair_queue = val;
		else
			err = -EINVAL;
		break;

	case TCP_QUEUE_SEQ:
		if (sk->sk_state != TCP_CLOSE)
			err = -EPERM;
		else if (tp->repair_queue == TCP_SEND_QUEUE)
			tp->write_seq = val;
		else if (tp->repair_queue == TCP_RECV_QUEUE)
			WRITE_ONCE(tp->rcv_nxt, val);
		else
			err = -EINVAL;
		break;

	case TCP_REPAIR_OPTIONS:
		if (!tp->repair)
			err = -EINVAL;
		else if (sk->sk_state == TCP_ESTABLISHED)
			err = tcp_repair_options_est(sk,
					(struct tcp_repair_opt __user *)optval,
					optlen);
		else
			err = -EPERM;
		break;

	case TCP_CORK:
		/* When set indicates to always queue non-full frames.
		 * Later the user clears this option and we transmit
		 * any pending partial frames in the queue.  This is
		 * meant to be used alongside sendfile() to get properly
		 * filled frames when the user (for example) must write
		 * out headers with a write() call first and then use
		 * sendfile to send out the data parts.
		 *
		 * TCP_CORK can be set together with TCP_NODELAY and it is
		 * stronger than TCP_NODELAY.
		 */
		if (val) {
			tp->nonagle |= TCP_NAGLE_CORK;
		} else {
			tp->nonagle &= ~TCP_NAGLE_CORK;
			if (tp->nonagle&TCP_NAGLE_OFF)
				tp->nonagle |= TCP_NAGLE_PUSH;
			tcp_push_pending_frames(sk);
		}
		break;

	case TCP_KEEPIDLE:
		if (val < 1 || val > MAX_TCP_KEEPIDLE)
			err = -EINVAL;
		else {
			tp->keepalive_time = val * HZ;
			if (sock_flag(sk, SOCK_KEEPOPEN) &&
			    !((1 << sk->sk_state) &
			      (TCPF_CLOSE | TCPF_LISTEN))) {
				u32 elapsed = keepalive_time_elapsed(tp);
				if (tp->keepalive_time > elapsed)
					elapsed = tp->keepalive_time - elapsed;
				else
					elapsed = 0;
				inet_csk_reset_keepalive_timer(sk, elapsed);
			}
		}
		break;
	case TCP_KEEPINTVL:
		if (val < 1 || val > MAX_TCP_KEEPINTVL)
			err = -EINVAL;
		else
			tp->keepalive_intvl = val * HZ;
		break;
	case TCP_KEEPCNT:
		if (val < 1 || val > MAX_TCP_KEEPCNT)
			err = -EINVAL;
		else
			tp->keepalive_probes = val;
		break;
	case TCP_SYNCNT:
		if (val < 1 || val > MAX_TCP_SYNCNT)
			err = -EINVAL;
		else
			icsk->icsk_syn_retries = val;
		break;

	case TCP_SAVE_SYN:
		if (val < 0 || val > 1)
			err = -EINVAL;
		else
			tp->save_syn = val;
		break;

	case TCP_LINGER2:
		if (val < 0)
			tp->linger2 = -1;
		else if (val > net->ipv4.sysctl_tcp_fin_timeout / HZ)
			tp->linger2 = 0;
		else
			tp->linger2 = val * HZ;
		break;

	case TCP_DEFER_ACCEPT:
		/* Translate value in seconds to number of retransmits */
		icsk->icsk_accept_queue.rskq_defer_accept =
			secs_to_retrans(val, TCP_TIMEOUT_INIT / HZ,
					TCP_RTO_MAX / HZ);
		break;

	case TCP_WINDOW_CLAMP:
		if (!val) {
			if (sk->sk_state != TCP_CLOSE) {
				e
```

_(no external call sites found for `do_tcp_setsockopt`)_

## File: `net/ipv4/tcp_diag.c`

- changed old-lines: [31]

### Function `tcp_diag_get_info` (L18–L35, 18 lines)

```c
static void tcp_diag_get_info(struct sock *sk, struct inet_diag_msg *r,
			      void *_info)
{
	struct tcp_info *info = _info;

	if (inet_sk_state_load(sk) == TCP_LISTEN) {
		r->idiag_rqueue = sk->sk_ack_backlog;
		r->idiag_wqueue = sk->sk_max_ack_backlog;
	} else if (sk->sk_type == SOCK_STREAM) {
		const struct tcp_sock *tp = tcp_sk(sk);

		r->idiag_rqueue = max_t(int, READ_ONCE(tp->rcv_nxt) -
					     READ_ONCE(tp->copied_seq), 0);
		r->idiag_wqueue = tp->write_seq - tp->snd_una;
	}
	if (info)
		tcp_get_info(sk, info);
}
```

**External callers of `tcp_diag_get_info` at parent**

- `include/net/inet_sock.h:313` — ` * tcp_diag_get_info(), tcp_get_info(), tcp_poll(), get_tcp4_sock() ...`

## File: `net/ipv4/tcp_ipv4.c`

- changed old-lines: [167, 168, 169, 256, 294, 295, 296, 297, 2464]

### Function `tcp_twsk_unique` (L106–L178, 73 lines)

```c
int tcp_twsk_unique(struct sock *sk, struct sock *sktw, void *twp)
{
	const struct inet_timewait_sock *tw = inet_twsk(sktw);
	const struct tcp_timewait_sock *tcptw = tcp_twsk(sktw);
	struct tcp_sock *tp = tcp_sk(sk);
	int reuse = sock_net(sk)->ipv4.sysctl_tcp_tw_reuse;

	if (reuse == 2) {
		/* Still does not detect *everything* that goes through
		 * lo, since we require a loopback src or dst address
		 * or direct binding to 'lo' interface.
		 */
		bool loopback = false;
		if (tw->tw_bound_dev_if == LOOPBACK_IFINDEX)
			loopback = true;
#if IS_ENABLED(CONFIG_IPV6)
		if (tw->tw_family == AF_INET6) {
			if (ipv6_addr_loopback(&tw->tw_v6_daddr) ||
			    (ipv6_addr_v4mapped(&tw->tw_v6_daddr) &&
			     (tw->tw_v6_daddr.s6_addr[12] == 127)) ||
			    ipv6_addr_loopback(&tw->tw_v6_rcv_saddr) ||
			    (ipv6_addr_v4mapped(&tw->tw_v6_rcv_saddr) &&
			     (tw->tw_v6_rcv_saddr.s6_addr[12] == 127)))
				loopback = true;
		} else
#endif
		{
			if (ipv4_is_loopback(tw->tw_daddr) ||
			    ipv4_is_loopback(tw->tw_rcv_saddr))
				loopback = true;
		}
		if (!loopback)
			reuse = 0;
	}

	/* With PAWS, it is safe from the viewpoint
	   of data integrity. Even without PAWS it is safe provided sequence
	   spaces do not overlap i.e. at data rates <= 80Mbit/sec.

	   Actually, the idea is close to VJ's one, only timestamp cache is
	   held not per host, but per port pair and TW bucket is used as state
	   holder.

	   If TW bucket has been already destroyed we fall back to VJ's scheme
	   and use initial timestamp retrieved from peer table.
	 */
	if (tcptw->tw_ts_recent_stamp &&
	    (!twp || (reuse && time_after32(ktime_get_seconds(),
					    tcptw->tw_ts_recent_stamp)))) {
		/* In case of repair and re-using TIME-WAIT sockets we still
		 * want to be sure that it is safe as above but honor the
		 * sequence numbers and time stamps set as part of the repair
		 * process.
		 *
		 * Without this check re-using a TIME-WAIT socket with TCP
		 * repair would accumulate a -1 on the repair assigned
		 * sequence number. The first time it is reused the sequence
		 * is -1, the second time -2, etc. This fixes that issue
		 * without appearing to create any others.
		 */
		if (likely(!tp->repair)) {
			tp->write_seq = tcptw->tw_snd_nxt + 65535 + 2;
			if (tp->write_seq == 0)
				tp->write_seq = 1;
			tp->rx_opt.ts_recent	   = tcptw->tw_ts_recent;
			tp->rx_opt.ts_recent_stamp = tcptw->tw_ts_recent_stamp;
		}
		sock_hold(sktw);
		return 1;
	}

	return 0;
}
```

**External callers of `tcp_twsk_unique` at parent**

- `include/net/tcp.h:339` — `int tcp_twsk_unique(struct sock *sk, struct sock *sktw, void *twp);`
- `net/ipv6/tcp_ipv6.c:1737` — `	.twsk_unique	= tcp_twsk_unique,`

### Function `tcp_v4_connect` (L197–L327, 131 lines)

```c
int tcp_v4_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len)
{
	struct sockaddr_in *usin = (struct sockaddr_in *)uaddr;
	struct inet_sock *inet = inet_sk(sk);
	struct tcp_sock *tp = tcp_sk(sk);
	__be16 orig_sport, orig_dport;
	__be32 daddr, nexthop;
	struct flowi4 *fl4;
	struct rtable *rt;
	int err;
	struct ip_options_rcu *inet_opt;
	struct inet_timewait_death_row *tcp_death_row = &sock_net(sk)->ipv4.tcp_death_row;

	if (addr_len < sizeof(struct sockaddr_in))
		return -EINVAL;

	if (usin->sin_family != AF_INET)
		return -EAFNOSUPPORT;

	nexthop = daddr = usin->sin_addr.s_addr;
	inet_opt = rcu_dereference_protected(inet->inet_opt,
					     lockdep_sock_is_held(sk));
	if (inet_opt && inet_opt->opt.srr) {
		if (!daddr)
			return -EINVAL;
		nexthop = inet_opt->opt.faddr;
	}

	orig_sport = inet->inet_sport;
	orig_dport = usin->sin_port;
	fl4 = &inet->cork.fl.u.ip4;
	rt = ip_route_connect(fl4, nexthop, inet->inet_saddr,
			      RT_CONN_FLAGS(sk), sk->sk_bound_dev_if,
			      IPPROTO_TCP,
			      orig_sport, orig_dport, sk);
	if (IS_ERR(rt)) {
		err = PTR_ERR(rt);
		if (err == -ENETUNREACH)
			IP_INC_STATS(sock_net(sk), IPSTATS_MIB_OUTNOROUTES);
		return err;
	}

	if (rt->rt_flags & (RTCF_MULTICAST | RTCF_BROADCAST)) {
		ip_rt_put(rt);
		return -ENETUNREACH;
	}

	if (!inet_opt || !inet_opt->opt.srr)
		daddr = fl4->daddr;

	if (!inet->inet_saddr)
		inet->inet_saddr = fl4->saddr;
	sk_rcv_saddr_set(sk, inet->inet_saddr);

	if (tp->rx_opt.ts_recent_stamp && inet->inet_daddr != daddr) {
		/* Reset inherited state */
		tp->rx_opt.ts_recent	   = 0;
		tp->rx_opt.ts_recent_stamp = 0;
		if (likely(!tp->repair))
			tp->write_seq	   = 0;
	}

	inet->inet_dport = usin->sin_port;
	sk_daddr_set(sk, daddr);

	inet_csk(sk)->icsk_ext_hdr_len = 0;
	if (inet_opt)
		inet_csk(sk)->icsk_ext_hdr_len = inet_opt->opt.optlen;

	tp->rx_opt.mss_clamp = TCP_MSS_DEFAULT;

	/* Socket identity is still unknown (sport may be zero).
	 * However we set state to SYN-SENT and not releasing socket
	 * lock select source port, enter ourselves into the hash tables and
	 * complete initialization after this.
	 */
	tcp_set_state(sk, TCP_SYN_SENT);
	err = inet_hash_connect(tcp_death_row, sk);
	if (err)
		goto failure;

	sk_set_txhash(sk);

	rt = ip_route_newports(fl4, rt, orig_sport, orig_dport,
			       inet->inet_sport, inet->inet_dport, sk);
	if (IS_ERR(rt)) {
		err = PTR_ERR(rt);
		rt = NULL;
		goto failure;
	}
	/* OK, now commit destination to socket.  */
	sk->sk_gso_type = SKB_GSO_TCPV4;
	sk_setup_caps(sk, &rt->dst);
	rt = NULL;

	if (likely(!tp->repair)) {
		if (!tp->write_seq)
			tp->write_seq = secure_tcp_seq(inet->inet_saddr,
						       inet->inet_daddr,
						       inet->inet_sport,
						       usin->sin_port);
		tp->tsoffset = secure_tcp_ts_off(sock_net(sk),
						 inet->inet_saddr,
						 inet->inet_daddr);
	}

	inet->inet_id = tp->write_seq ^ jiffies;

	if (tcp_fastopen_defer_connect(sk, &err))
		return err;
	if (err)
		goto failure;

	err = tcp_connect(sk);

	if (err)
		goto failure;

	return 0;

failure:
	/*
	 * This unhashes the socket and releases the local port,
	 * if necessary.
	 */
	tcp_set_state(sk, TCP_CLOSE);
	ip_rt_put(rt);
	sk->sk_route_caps = 0;
	inet->inet_dport = 0;
	return err;
}
```

**External callers of `tcp_v4_connect` at parent**

- `include/net/tcp.h:445` — `int tcp_v4_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len);`
- `net/ipv6/tcp_ipv6.c:245` — `		err = tcp_v4_connect(sk, (struct sockaddr *)&sin, sizeof(sin));`

### Function `get_tcp4_sock` (L2420–L2480, 61 lines)

```c
static void get_tcp4_sock(struct sock *sk, struct seq_file *f, int i)
{
	int timer_active;
	unsigned long timer_expires;
	const struct tcp_sock *tp = tcp_sk(sk);
	const struct inet_connection_sock *icsk = inet_csk(sk);
	const struct inet_sock *inet = inet_sk(sk);
	const struct fastopen_queue *fastopenq = &icsk->icsk_accept_queue.fastopenq;
	__be32 dest = inet->inet_daddr;
	__be32 src = inet->inet_rcv_saddr;
	__u16 destp = ntohs(inet->inet_dport);
	__u16 srcp = ntohs(inet->inet_sport);
	int rx_queue;
	int state;

	if (icsk->icsk_pending == ICSK_TIME_RETRANS ||
	    icsk->icsk_pending == ICSK_TIME_REO_TIMEOUT ||
	    icsk->icsk_pending == ICSK_TIME_LOSS_PROBE) {
		timer_active	= 1;
		timer_expires	= icsk->icsk_timeout;
	} else if (icsk->icsk_pending == ICSK_TIME_PROBE0) {
		timer_active	= 4;
		timer_expires	= icsk->icsk_timeout;
	} else if (timer_pending(&sk->sk_timer)) {
		timer_active	= 2;
		timer_expires	= sk->sk_timer.expires;
	} else {
		timer_active	= 0;
		timer_expires = jiffies;
	}

	state = inet_sk_state_load(sk);
	if (state == TCP_LISTEN)
		rx_queue = sk->sk_ack_backlog;
	else
		/* Because we don't lock the socket,
		 * we might find a transient negative value.
		 */
		rx_queue = max_t(int, READ_ONCE(tp->rcv_nxt) -
				      READ_ONCE(tp->copied_seq), 0);

	seq_printf(f, "%4d: %08X:%04X %08X:%04X %02X %08X:%08X %02X:%08lX "
			"%08X %5u %8d %lu %d %pK %lu %lu %u %u %d",
		i, src, srcp, dest, destp, state,
		tp->write_seq - tp->snd_una,
		rx_queue,
		timer_active,
		jiffies_delta_to_clock_t(timer_expires - jiffies),
		icsk->icsk_retransmits,
		from_kuid_munged(seq_user_ns(f), sock_i_uid(sk)),
		icsk->icsk_probes_out,
		sock_i_ino(sk),
		refcount_read(&sk->sk_refcnt), sk,
		jiffies_to_clock_t(icsk->icsk_rto),
		jiffies_to_clock_t(icsk->icsk_ack.ato),
		(icsk->icsk_ack.quick << 1) | inet_csk_in_pingpong_mode(sk),
		tp->snd_cwnd,
		state == TCP_LISTEN ?
		    fastopenq->max_qlen :
		    (tcp_in_initial_slowstart(tp) ? -1 : tp->snd_ssthresh));
}
```

**External callers of `get_tcp4_sock` at parent**

- `include/net/inet_sock.h:313` — ` * tcp_diag_get_info(), tcp_get_info(), tcp_poll(), get_tcp4_sock() ...`

## File: `net/ipv4/tcp_minisocks.c`

- changed old-lines: [501]

### Function `tcp_create_openreq_child` (L456–L552, 97 lines)

```c
struct sock *tcp_create_openreq_child(const struct sock *sk,
				      struct request_sock *req,
				      struct sk_buff *skb)
{
	struct sock *newsk = inet_csk_clone_lock(sk, req, GFP_ATOMIC);
	const struct inet_request_sock *ireq = inet_rsk(req);
	struct tcp_request_sock *treq = tcp_rsk(req);
	struct inet_connection_sock *newicsk;
	struct tcp_sock *oldtp, *newtp;
	u32 seq;

	if (!newsk)
		return NULL;

	newicsk = inet_csk(newsk);
	newtp = tcp_sk(newsk);
	oldtp = tcp_sk(sk);

	smc_check_reset_syn_req(oldtp, req, newtp);

	/* Now setup tcp_sock */
	newtp->pred_flags = 0;

	seq = treq->rcv_isn + 1;
	newtp->rcv_wup = seq;
	WRITE_ONCE(newtp->copied_seq, seq);
	WRITE_ONCE(newtp->rcv_nxt, seq);
	newtp->segs_in = 1;

	newtp->snd_sml = newtp->snd_una =
	newtp->snd_nxt = newtp->snd_up = treq->snt_isn + 1;

	INIT_LIST_HEAD(&newtp->tsq_node);
	INIT_LIST_HEAD(&newtp->tsorted_sent_queue);

	tcp_init_wl(newtp, treq->rcv_isn);

	minmax_reset(&newtp->rtt_min, tcp_jiffies32, ~0U);
	newicsk->icsk_ack.lrcvtime = tcp_jiffies32;

	newtp->lsndtime = tcp_jiffies32;
	newsk->sk_txhash = treq->txhash;
	newtp->total_retrans = req->num_retrans;

	tcp_init_xmit_timers(newsk);
	newtp->write_seq = newtp->pushed_seq = treq->snt_isn + 1;

	if (sock_flag(newsk, SOCK_KEEPOPEN))
		inet_csk_reset_keepalive_timer(newsk,
					       keepalive_time_when(newtp));

	newtp->rx_opt.tstamp_ok = ireq->tstamp_ok;
	newtp->rx_opt.sack_ok = ireq->sack_ok;
	newtp->window_clamp = req->rsk_window_clamp;
	newtp->rcv_ssthresh = req->rsk_rcv_wnd;
	newtp->rcv_wnd = req->rsk_rcv_wnd;
	newtp->rx_opt.wscale_ok = ireq->wscale_ok;
	if (newtp->rx_opt.wscale_ok) {
		newtp->rx_opt.snd_wscale = ireq->snd_wscale;
		newtp->rx_opt.rcv_wscale = ireq->rcv_wscale;
	} else {
		newtp->rx_opt.snd_wscale = newtp->rx_opt.rcv_wscale = 0;
		newtp->window_clamp = min(newtp->window_clamp, 65535U);
	}
	newtp->snd_wnd = ntohs(tcp_hdr(skb)->window) << newtp->rx_opt.snd_wscale;
	newtp->max_window = newtp->snd_wnd;

	if (newtp->rx_opt.tstamp_ok) {
		newtp->rx_opt.ts_recent = req->ts_recent;
		newtp->rx_opt.ts_recent_stamp = ktime_get_seconds();
		newtp->tcp_header_len = sizeof(struct tcphdr) + TCPOLEN_TSTAMP_ALIGNED;
	} else {
		newtp->rx_opt.ts_recent_stamp = 0;
		newtp->tcp_header_len = sizeof(struct tcphdr);
	}
	if (req->num_timeout) {
		newtp->undo_marker = treq->snt_isn;
		newtp->retrans_stamp = div_u64(treq->snt_synack,
					       USEC_PER_SEC / TCP_TS_HZ);
	}
	newtp->tsoffset = treq->ts_off;
#ifdef CONFIG_TCP_MD5SIG
	newtp->md5sig_info = NULL;	/*XXX*/
	if (newtp->af_specific->md5_lookup(sk, newsk))
		newtp->tcp_header_len += TCPOLEN_MD5SIG_ALIGNED;
#endif
	if (skb->len >= TCP_MSS_DEFAULT + newtp->tcp_header_len)
		newicsk->icsk_ack.last_seg_size = skb->len - newtp->tcp_header_len;
	newtp->rx_opt.mss_clamp = req->mss;
	tcp_ecn_openreq_child(newtp, req);
	newtp->fastopen_req = NULL;
	RCU_INIT_POINTER(newtp->fastopen_rsk, NULL);

	__TCP_INC_STATS(sock_net(sk), TCP_MIB_PASSIVEOPENS);

	return newsk;
}
```

**External callers of `tcp_create_openreq_child` at parent**

- `drivers/crypto/chelsio/chtls/chtls_cm.c:1028` — `	newsk = tcp_create_openreq_child(lsk, oreq, cdev->askb);`
- `drivers/crypto/chelsio/chtls/chtls_cm.c:1091` — `	bh_unlock_sock(newsk); /* tcp_create_openreq_child ->sk_clone_lock */`
- `include/net/tcp.h:435` — `struct sock *tcp_create_openreq_child(const struct sock *sk,`
- `net/core/sock.c:1887` — `		 * tcp_create_openreq_child always was incrementing the`
- `net/ipv4/tcp_fastopen.c:175` — `	/* segs_in has been initialized to 1 in tcp_create_openreq_child().`
- `net/ipv4/tcp_ipv4.c:480` — `	/* XXX (TFO) - tp->snd_una should be ISN (tcp_create_openreq_child() */`
- `net/ipv4/tcp_ipv4.c:1428` — `	newsk = tcp_create_openreq_child(sk, req, skb);`
- `net/ipv6/tcp_ipv6.c:408` — `	/* XXX (TFO) - tp->snd_una should be ISN (tcp_create_openreq_child() */`
- `net/ipv6/tcp_ipv6.c:1171` — `		 * here, tcp_create_openreq_child now does this for us, see the comment in`
- `net/ipv6/tcp_ipv6.c:1195` — `	newsk = tcp_create_openreq_child(sk, req, skb);`
- `net/ipv6/tcp_ipv6.c:1201` — `	 * count here, tcp_create_openreq_child now does this for us, see the`

## File: `net/ipv4/tcp_output.c`

- changed old-lines: [1199, 3452]

### Function `tcp_queue_skb` (L1194–L1204, 11 lines)

```c
static void tcp_queue_skb(struct sock *sk, struct sk_buff *skb)
{
	struct tcp_sock *tp = tcp_sk(sk);

	/* Advance write_seq and place onto the write_queue. */
	tp->write_seq = TCP_SKB_CB(skb)->end_seq;
	__skb_header_release(skb);
	tcp_add_write_queue_tail(sk, skb);
	sk->sk_wmem_queued += skb->truesize;
	sk_mem_charge(sk, skb->truesize);
}
```

_(no external call sites found for `tcp_queue_skb`)_

### Function `tcp_connect_queue_skb` (L3443–L3454, 12 lines)

```c
static void tcp_connect_queue_skb(struct sock *sk, struct sk_buff *skb)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct tcp_skb_cb *tcb = TCP_SKB_CB(skb);

	tcb->end_seq += skb->len;
	__skb_header_release(skb);
	sk->sk_wmem_queued += skb->truesize;
	sk_mem_charge(sk, skb->truesize);
	tp->write_seq = tcb->end_seq;
	tp->packets_out += tcp_skb_pcount(skb);
}
```

_(no external call sites found for `tcp_connect_queue_skb`)_
