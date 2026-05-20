# Evidence: SYZBOT-371a9ea56d82de71  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in tcp_poll / tcp_recvmsg (2)
- mainline fix commit: `7db48e983930285b765743ebd665aecf9850582b`
- candidate JSON: `A_SYZBOT-371a9ea56d82de71.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "tcp_poll", "fn_b": "tcp_recvmsg"}`

## Commit message

```
7db48e983930285b765743ebd665aecf9850582b
tcp: annotate tp->copied_seq lockless reads

There are few places where we fetch tp->copied_seq while
this field can change from IRQ or other cpu.

We need to add READ_ONCE() annotations, and also make
sure write sides use corresponding WRITE_ONCE() to avoid
store-tearing.

Note that tcp_inq_hint() was already using READ_ONCE(tp->copied_seq)

Signed-off-by: Eric Dumazet <edumazet@google.com>
Signed-off-by: David S. Miller <davem@davemloft.net>
```

## CVE / bug description

```
KCSAN: data-race in tcp_poll / tcp_recvmsg (2)
```

## Full mainline patch

```diff
commit 7db48e983930285b765743ebd665aecf9850582b
Author: Eric Dumazet <edumazet@google.com>
Date:   Thu Oct 10 20:17:40 2019 -0700

    tcp: annotate tp->copied_seq lockless reads
    
    There are few places where we fetch tp->copied_seq while
    this field can change from IRQ or other cpu.
    
    We need to add READ_ONCE() annotations, and also make
    sure write sides use corresponding WRITE_ONCE() to avoid
    store-tearing.
    
    Note that tcp_inq_hint() was already using READ_ONCE(tp->copied_seq)
    
    Signed-off-by: Eric Dumazet <edumazet@google.com>
    Signed-off-by: David S. Miller <davem@davemloft.net>

diff --git a/net/ipv4/tcp.c b/net/ipv4/tcp.c
index 883ee863db43..c322ad071e17 100644
--- a/net/ipv4/tcp.c
+++ b/net/ipv4/tcp.c
@@ -477,7 +477,7 @@ static void tcp_tx_timestamp(struct sock *sk, u16 tsflags)
 static inline bool tcp_stream_is_readable(const struct tcp_sock *tp,
 					  int target, struct sock *sk)
 {
-	return (READ_ONCE(tp->rcv_nxt) - tp->copied_seq >= target) ||
+	return (READ_ONCE(tp->rcv_nxt) - READ_ONCE(tp->copied_seq) >= target) ||
 		(sk->sk_prot->stream_memory_read ?
 		sk->sk_prot->stream_memory_read(sk) : false);
 }
@@ -546,7 +546,7 @@ __poll_t tcp_poll(struct file *file, struct socket *sock, poll_table *wait)
 	    (state != TCP_SYN_RECV || rcu_access_pointer(tp->fastopen_rsk))) {
 		int target = sock_rcvlowat(sk, 0, INT_MAX);
 
-		if (tp->urg_seq == tp->copied_seq &&
+		if (tp->urg_seq == READ_ONCE(tp->copied_seq) &&
 		    !sock_flag(sk, SOCK_URGINLINE) &&
 		    tp->urg_data)
 			target++;
@@ -607,7 +607,7 @@ int tcp_ioctl(struct sock *sk, int cmd, unsigned long arg)
 		unlock_sock_fast(sk, slow);
 		break;
 	case SIOCATMARK:
-		answ = tp->urg_data && tp->urg_seq == tp->copied_seq;
+		answ = tp->urg_data && tp->urg_seq == READ_ONCE(tp->copied_seq);
 		break;
 	case SIOCOUTQ:
 		if (sk->sk_state == TCP_LISTEN)
@@ -1668,9 +1668,9 @@ int tcp_read_sock(struct sock *sk, read_descriptor_t *desc,
 		sk_eat_skb(sk, skb);
 		if (!desc->count)
 			break;
-		tp->copied_seq = seq;
+		WRITE_ONCE(tp->copied_seq, seq);
 	}
-	tp->copied_seq = seq;
+	WRITE_ONCE(tp->copied_seq, seq);
 
 	tcp_rcv_space_adjust(sk);
 
@@ -1819,7 +1819,7 @@ static int tcp_zerocopy_receive(struct sock *sk,
 out:
 	up_read(&current->mm->mmap_sem);
 	if (length) {
-		tp->copied_seq = seq;
+		WRITE_ONCE(tp->copied_seq, seq);
 		tcp_rcv_space_adjust(sk);
 
 		/* Clean up data we have read: This will do ACK frames. */
@@ -2117,7 +2117,7 @@ int tcp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int nonblock,
 			if (urg_offset < used) {
 				if (!urg_offset) {
 					if (!sock_flag(sk, SOCK_URGINLINE)) {
-						++*seq;
+						WRITE_ONCE(*seq, *seq + 1);
 						urg_hole++;
 						offset++;
 						used--;
@@ -2139,7 +2139,7 @@ int tcp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int nonblock,
 			}
 		}
 
-		*seq += used;
+		WRITE_ONCE(*seq, *seq + used);
 		copied += used;
 		len -= used;
 
@@ -2166,7 +2166,7 @@ int tcp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int nonblock,
 
 found_fin_ok:
 		/* Process the FIN. */
-		++*seq;
+		WRITE_ONCE(*seq, *seq + 1);
 		if (!(flags & MSG_PEEK))
 			sk_eat_skb(sk, skb);
 		break;
@@ -2588,7 +2588,7 @@ int tcp_disconnect(struct sock *sk, int flags)
 		__kfree_skb(sk->sk_rx_skb_cache);
 		sk->sk_rx_skb_cache = NULL;
 	}
-	tp->copied_seq = tp->rcv_nxt;
+	WRITE_ONCE(tp->copied_seq, tp->rcv_nxt);
 	tp->urg_data = 0;
 	tcp_write_queue_purge(sk);
 	tcp_fastopen_active_disable_ofo_check(sk);
diff --git a/net/ipv4/tcp_diag.c b/net/ipv4/tcp_diag.c
index cd219161f106..66273c8a55c2 100644
--- a/net/ipv4/tcp_diag.c
+++ b/net/ipv4/tcp_diag.c
@@ -26,7 +26,8 @@ static void tcp_diag_get_info(struct sock *sk, struct inet_diag_msg *r,
 	} else if (sk->sk_type == SOCK_STREAM) {
 		const struct tcp_sock *tp = tcp_sk(sk);
 
-		r->idiag_rqueue = max_t(int, READ_ONCE(tp->rcv_nxt) - tp->copied_seq, 0);
+		r->idiag_rqueue = max_t(int, READ_ONCE(tp->rcv_nxt) -
+					     READ_ONCE(tp->copied_seq), 0);
 		r->idiag_wqueue = tp->write_seq - tp->snd_una;
 	}
 	if (info)
diff --git a/net/ipv4/tcp_input.c b/net/ipv4/tcp_input.c
index 5b7c8768ed5f..a30aae3a6a18 100644
--- a/net/ipv4/tcp_input.c
+++ b/net/ipv4/tcp_input.c
@@ -5961,7 +5961,7 @@ static int tcp_rcv_synsent_state_process(struct sock *sk, struct sk_buff *skb,
 		/* Remember, tcp_poll() does not lock socket!
 		 * Change state from SYN-SENT only after copied_seq
 		 * is initialized. */
-		tp->copied_seq = tp->rcv_nxt;
+		WRITE_ONCE(tp->copied_seq, tp->rcv_nxt);
 
 		smc_check_reset_syn(tp);
 
@@ -6036,7 +6036,7 @@ static int tcp_rcv_synsent_state_process(struct sock *sk, struct sk_buff *skb,
 		}
 
 		WRITE_ONCE(tp->rcv_nxt, TCP_SKB_CB(skb)->seq + 1);
-		tp->copied_seq = tp->rcv_nxt;
+		WRITE_ONCE(tp->copied_seq, tp->rcv_nxt);
 		tp->rcv_wup = TCP_SKB_CB(skb)->seq + 1;
 
 		/* RFC1323: The window in SYN & SYN/ACK segments is
@@ -6216,7 +6216,7 @@ int tcp_rcv_state_process(struct sock *sk, struct sk_buff *skb)
 			tcp_try_undo_spurious_syn(sk);
 			tp->retrans_stamp = 0;
 			tcp_init_transfer(sk, BPF_SOCK_OPS_PASSIVE_ESTABLISHED_CB);
-			tp->copied_seq = tp->rcv_nxt;
+			WRITE_ONCE(tp->copied_seq, tp->rcv_nxt);
 		}
 		smp_mb();
 		tcp_set_state(sk, TCP_ESTABLISHED);
diff --git a/net/ipv4/tcp_ipv4.c b/net/ipv4/tcp_ipv4.c
index 5089dd6bee0f..39560f482e0b 100644
--- a/net/ipv4/tcp_ipv4.c
+++ b/net/ipv4/tcp_ipv4.c
@@ -2456,7 +2456,7 @@ static void get_tcp4_sock(struct sock *sk, struct seq_file *f, int i)
 		 * we might find a transient negative value.
 		 */
 		rx_queue = max_t(int, READ_ONCE(tp->rcv_nxt) -
-				      tp->copied_seq, 0);
+				      READ_ONCE(tp->copied_seq), 0);
 
 	seq_printf(f, "%4d: %08X:%04X %08X:%04X %02X %08X:%08X %02X:%08lX "
 			"%08X %5u %8d %lu %d %pK %lu %lu %u %u %d",
diff --git a/net/ipv4/tcp_minisocks.c b/net/ipv4/tcp_minisocks.c
index adc6ce486a38..c4731d26ab4a 100644
--- a/net/ipv4/tcp_minisocks.c
+++ b/net/ipv4/tcp_minisocks.c
@@ -478,7 +478,7 @@ struct sock *tcp_create_openreq_child(const struct sock *sk,
 
 	seq = treq->rcv_isn + 1;
 	newtp->rcv_wup = seq;
-	newtp->copied_seq = seq;
+	WRITE_ONCE(newtp->copied_seq, seq);
 	WRITE_ONCE(newtp->rcv_nxt, seq);
 	newtp->segs_in = 1;
 
diff --git a/net/ipv4/tcp_output.c b/net/ipv4/tcp_output.c
index 84ae4d1449ea..7dda12720169 100644
--- a/net/ipv4/tcp_output.c
+++ b/net/ipv4/tcp_output.c
@@ -3433,7 +3433,7 @@ static void tcp_connect_init(struct sock *sk)
 	else
 		tp->rcv_tstamp = tcp_jiffies32;
 	tp->rcv_wup = tp->rcv_nxt;
-	tp->copied_seq = tp->rcv_nxt;
+	WRITE_ONCE(tp->copied_seq, tp->rcv_nxt);
 
 	inet_csk(sk)->icsk_rto = tcp_timeout_init(sk);
 	inet_csk(sk)->icsk_retransmits = 0;
diff --git a/net/ipv6/tcp_ipv6.c b/net/ipv6/tcp_ipv6.c
index 89ea0a7018b5..a62c7042fc4a 100644
--- a/net/ipv6/tcp_ipv6.c
+++ b/net/ipv6/tcp_ipv6.c
@@ -1896,7 +1896,7 @@ static void get_tcp6_sock(struct seq_file *seq, struct sock *sp, int i)
 		 * we might find a transient negative value.
 		 */
 		rx_queue = max_t(int, READ_ONCE(tp->rcv_nxt) -
-				      tp->copied_seq, 0);
+				      READ_ONCE(tp->copied_seq), 0);
 
 	seq_printf(seq,
 		   "%4d: %08X%08X%08X%08X:%04X %08X%08X%08X%08X:%04X "

```

## File: `net/ipv4/tcp.c`

- changed old-lines: [480, 549, 610, 1671, 1673, 1822, 2120, 2142, 2169, 2591]

### Function `tcp_stream_is_readable` (L477–L483, 7 lines)

```c
static inline bool tcp_stream_is_readable(const struct tcp_sock *tp,
					  int target, struct sock *sk)
{
	return (READ_ONCE(tp->rcv_nxt) - tp->copied_seq >= target) ||
		(sk->sk_prot->stream_memory_read ?
		sk->sk_prot->stream_memory_read(sk) : false);
}
```

_(no external call sites found for `tcp_stream_is_readable`)_

### Function `tcp_poll` (L492–L591, 100 lines)

```c
__poll_t tcp_poll(struct file *file, struct socket *sock, poll_table *wait)
{
	__poll_t mask;
	struct sock *sk = sock->sk;
	const struct tcp_sock *tp = tcp_sk(sk);
	int state;

	sock_poll_wait(file, sock, wait);

	state = inet_sk_state_load(sk);
	if (state == TCP_LISTEN)
		return inet_csk_listen_poll(sk);

	/* Socket is not locked. We are protected from async events
	 * by poll logic and correct handling of state changes
	 * made by other threads is impossible in any case.
	 */

	mask = 0;

	/*
	 * EPOLLHUP is certainly not done right. But poll() doesn't
	 * have a notion of HUP in just one direction, and for a
	 * socket the read side is more interesting.
	 *
	 * Some poll() documentation says that EPOLLHUP is incompatible
	 * with the EPOLLOUT/POLLWR flags, so somebody should check this
	 * all. But careful, it tends to be safer to return too many
	 * bits than too few, and you can easily break real applications
	 * if you don't tell them that something has hung up!
	 *
	 * Check-me.
	 *
	 * Check number 1. EPOLLHUP is _UNMASKABLE_ event (see UNIX98 and
	 * our fs/select.c). It means that after we received EOF,
	 * poll always returns immediately, making impossible poll() on write()
	 * in state CLOSE_WAIT. One solution is evident --- to set EPOLLHUP
	 * if and only if shutdown has been made in both directions.
	 * Actually, it is interesting to look how Solaris and DUX
	 * solve this dilemma. I would prefer, if EPOLLHUP were maskable,
	 * then we could set it on SND_SHUTDOWN. BTW examples given
	 * in Stevens' books assume exactly this behaviour, it explains
	 * why EPOLLHUP is incompatible with EPOLLOUT.	--ANK
	 *
	 * NOTE. Check for TCP_CLOSE is added. The goal is to prevent
	 * blocking on fresh not-connected or disconnected socket. --ANK
	 */
	if (sk->sk_shutdown == SHUTDOWN_MASK || state == TCP_CLOSE)
		mask |= EPOLLHUP;
	if (sk->sk_shutdown & RCV_SHUTDOWN)
		mask |= EPOLLIN | EPOLLRDNORM | EPOLLRDHUP;

	/* Connected or passive Fast Open socket? */
	if (state != TCP_SYN_SENT &&
	    (state != TCP_SYN_RECV || rcu_access_pointer(tp->fastopen_rsk))) {
		int target = sock_rcvlowat(sk, 0, INT_MAX);

		if (tp->urg_seq == tp->copied_seq &&
		    !sock_flag(sk, SOCK_URGINLINE) &&
		    tp->urg_data)
			target++;

		if (tcp_stream_is_readable(tp, target, sk))
			mask |= EPOLLIN | EPOLLRDNORM;

		if (!(sk->sk_shutdown & SEND_SHUTDOWN)) {
			if (sk_stream_is_writeable(sk)) {
				mask |= EPOLLOUT | EPOLLWRNORM;
			} else {  /* send SIGIO later */
				sk_set_bit(SOCKWQ_ASYNC_NOSPACE, sk);
				set_bit(SOCK_NOSPACE, &sk->sk_socket->flags);

				/* Race breaker. If space is freed after
				 * wspace test but before the flags are set,
				 * IO signal will be lost. Memory barrier
				 * pairs with the input side.
				 */
				smp_mb__after_atomic();
				if (sk_stream_is_writeable(sk))
					mask |= EPOLLOUT | EPOLLWRNORM;
			}
		} else
			mask |= EPOLLOUT | EPOLLWRNORM;

		if (tp->urg_data & TCP_URG_VALID)
			mask |= EPOLLPRI;
	} else if (state == TCP_SYN_SENT && inet_sk(sk)->defer_connect) {
		/* Active TCP fastopen socket with defer_connect
		 * Return EPOLLOUT so application can call write()
		 * in order for kernel to generate SYN+data
		 */
		mask |= EPOLLOUT | EPOLLWRNORM;
	}
	/* This barrier is coupled with smp_wmb() in tcp_reset() */
	smp_rmb();
	if (sk->sk_err || !skb_queue_empty(&sk->sk_error_queue))
		mask |= EPOLLERR;

	return mask;
}
```

**External callers of `tcp_poll` at parent**

- `include/net/inet_sock.h:313` — ` * tcp_diag_get_info(), tcp_get_info(), tcp_poll(), get_tcp4_sock() ...`
- `include/net/tcp.h:392` — `__poll_t tcp_poll(struct file *file, struct socket *sock,`
- `net/dccp/ipv4.c:987` — `	/* FIXME: work on tcp_poll to rename it to inet_csk_poll */`
- `net/ipv4/af_inet.c:992` — `	.poll		   = tcp_poll,`
- `net/ipv4/tcp_input.c:4119` — `	/* This barrier is coupled with smp_rmb() in tcp_poll() */`
- `net/ipv4/tcp_input.c:5202` — `		/* pairs with tcp_poll() */`
- `net/ipv4/tcp_input.c:5961` — `		/* Remember, tcp_poll() does not lock socket!`
- `net/ipv6/af_inet6.c:609` — `	.poll		   = tcp_poll,			/* ok		*/`
- `net/sctp/socket.c:8448` — ` * tcp_poll().  Note that, based on these implementations, we don't`
- `net/sctp/socket.c:8504` — `		 * signal.  tcp_poll() has a race breaker for this race`

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
		answ = tp->urg_data && tp->urg_seq == tp->copied_seq;
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

### Function `tcp_read_sock` (L1614–L1683, 70 lines)

```c
int tcp_read_sock(struct sock *sk, read_descriptor_t *desc,
		  sk_read_actor_t recv_actor)
{
	struct sk_buff *skb;
	struct tcp_sock *tp = tcp_sk(sk);
	u32 seq = tp->copied_seq;
	u32 offset;
	int copied = 0;

	if (sk->sk_state == TCP_LISTEN)
		return -ENOTCONN;
	while ((skb = tcp_recv_skb(sk, seq, &offset)) != NULL) {
		if (offset < skb->len) {
			int used;
			size_t len;

			len = skb->len - offset;
			/* Stop reading if we hit a patch of urgent data */
			if (tp->urg_data) {
				u32 urg_offset = tp->urg_seq - seq;
				if (urg_offset < len)
					len = urg_offset;
				if (!len)
					break;
			}
			used = recv_actor(desc, skb, offset, len);
			if (used <= 0) {
				if (!copied)
					copied = used;
				break;
			} else if (used <= len) {
				seq += used;
				copied += used;
				offset += used;
			}
			/* If recv_actor drops the lock (e.g. TCP splice
			 * receive) the skb pointer might be invalid when
			 * getting here: tcp_collapse might have deleted it
			 * while aggregating skbs from the socket queue.
			 */
			skb = tcp_recv_skb(sk, seq - 1, &offset);
			if (!skb)
				break;
			/* TCP coalescing might have appended data to the skb.
			 * Try to splice more frags
			 */
			if (offset + 1 != skb->len)
				continue;
		}
		if (TCP_SKB_CB(skb)->tcp_flags & TCPHDR_FIN) {
			sk_eat_skb(sk, skb);
			++seq;
			break;
		}
		sk_eat_skb(sk, skb);
		if (!desc->count)
			break;
		tp->copied_seq = seq;
	}
	tp->copied_seq = seq;

	tcp_rcv_space_adjust(sk);

	/* Clean up data we have read: This will do ACK frames. */
	if (copied > 0) {
		tcp_recv_skb(sk, seq, &offset);
		tcp_cleanup_rbuf(sk, copied);
	}
	return copied;
}
```

**External callers of `tcp_read_sock` at parent**

- `drivers/infiniband/sw/siw/siw_cm.c:125` — `	tcp_read_sock(sk, &rd_desc, siw_tcp_rx_data);`
- `drivers/infiniband/sw/siw/siw_qp.c:115` — `			tcp_read_sock(sk, &rd_desc, siw_tcp_rx_data);`
- `drivers/scsi/iscsi_tcp.c:147` — `	tcp_read_sock(sk, &rd_desc, iscsi_sw_tcp_recv);`
- `include/net/tcp.h:639` — `int tcp_read_sock(struct sock *sk, read_descriptor_t *desc,`
- `net/ipv4/af_inet.c:1006` — `	.read_sock	   = tcp_read_sock,`
- `net/ipv6/af_inet6.c:625` — `	.read_sock	   = tcp_read_sock,`
- `net/rds/tcp_recv.c:168` — `	 * tcp_read_sock() interprets partial progress as an indication to stop`
- `net/rds/tcp_recv.c:276` — `	tcp_read_sock(sock->sk, &desc, rds_tcp_data_recv);`
- `net/rds/tcp_recv.c:277` — `	rdsdebug("tcp_read_sock for tc %p gfp 0x%x returned %d\n", tc, gfp,`
- `net/rds/tcp_recv.c:284` — ` * We hold the sock lock to serialize our rds_tcp_recv->tcp_read_sock from`

### Function `tcp_zerocopy_receive` (L1738–L1837, 100 lines)

```c
static int tcp_zerocopy_receive(struct sock *sk,
				struct tcp_zerocopy_receive *zc)
{
	unsigned long address = (unsigned long)zc->address;
	const skb_frag_t *frags = NULL;
	u32 length = 0, seq, offset;
	struct vm_area_struct *vma;
	struct sk_buff *skb = NULL;
	struct tcp_sock *tp;
	int inq;
	int ret;

	if (address & (PAGE_SIZE - 1) || address != zc->address)
		return -EINVAL;

	if (sk->sk_state == TCP_LISTEN)
		return -ENOTCONN;

	sock_rps_record_flow(sk);

	down_read(&current->mm->mmap_sem);

	ret = -EINVAL;
	vma = find_vma(current->mm, address);
	if (!vma || vma->vm_start > address || vma->vm_ops != &tcp_vm_ops)
		goto out;
	zc->length = min_t(unsigned long, zc->length, vma->vm_end - address);

	tp = tcp_sk(sk);
	seq = tp->copied_seq;
	inq = tcp_inq(sk);
	zc->length = min_t(u32, zc->length, inq);
	zc->length &= ~(PAGE_SIZE - 1);
	if (zc->length) {
		zap_page_range(vma, address, zc->length);
		zc->recv_skip_hint = 0;
	} else {
		zc->recv_skip_hint = inq;
	}
	ret = 0;
	while (length + PAGE_SIZE <= zc->length) {
		if (zc->recv_skip_hint < PAGE_SIZE) {
			if (skb) {
				skb = skb->next;
				offset = seq - TCP_SKB_CB(skb)->seq;
			} else {
				skb = tcp_recv_skb(sk, seq, &offset);
			}

			zc->recv_skip_hint = skb->len - offset;
			offset -= skb_headlen(skb);
			if ((int)offset < 0 || skb_has_frag_list(skb))
				break;
			frags = skb_shinfo(skb)->frags;
			while (offset) {
				if (skb_frag_size(frags) > offset)
					goto out;
				offset -= skb_frag_size(frags);
				frags++;
			}
		}
		if (skb_frag_size(frags) != PAGE_SIZE || skb_frag_off(frags)) {
			int remaining = zc->recv_skip_hint;

			while (remaining && (skb_frag_size(frags) != PAGE_SIZE ||
					     skb_frag_off(frags))) {
				remaining -= skb_frag_size(frags);
				frags++;
			}
			zc->recv_skip_hint -= remaining;
			break;
		}
		ret = vm_insert_page(vma, address + length,
				     skb_frag_page(frags));
		if (ret)
			break;
		length += PAGE_SIZE;
		seq += PAGE_SIZE;
		zc->recv_skip_hint -= PAGE_SIZE;
		frags++;
	}
out:
	up_read(&current->mm->mmap_sem);
	if (length) {
		tp->copied_seq = seq;
		tcp_rcv_space_adjust(sk);

		/* Clean up data we have read: This will do ACK frames. */
		tcp_recv_skb(sk, seq, &offset);
		tcp_cleanup_rbuf(sk, length);
		ret = 0;
		if (length == zc->length)
			zc->recv_skip_hint = 0;
	} else {
		if (!zc->recv_skip_hint && sock_flag(sk, SOCK_DONE))
			ret = -EIO;
	}
	zc->length = length;
	return ret;
}
```

**External callers of `tcp_zerocopy_receive` at parent**

- `include/uapi/linux/tcp.h:334` — `struct tcp_zerocopy_receive {`
- `tools/testing/selftests/net/tcp_mmap.c:124` — `	struct tcp_zerocopy_receive zc;`

## File: `net/ipv4/tcp_diag.c`

- changed old-lines: [29]

### Function `tcp_diag_get_info` (L18–L34, 17 lines)

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

		r->idiag_rqueue = max_t(int, READ_ONCE(tp->rcv_nxt) - tp->copied_seq, 0);
		r->idiag_wqueue = tp->write_seq - tp->snd_una;
	}
	if (info)
		tcp_get_info(sk, info);
}
```

**External callers of `tcp_diag_get_info` at parent**

- `include/net/inet_sock.h:313` — ` * tcp_diag_get_info(), tcp_get_info(), tcp_poll(), get_tcp4_sock() ...`

## File: `net/ipv4/tcp_input.c`

- changed old-lines: [5964, 6039, 6219]

### Function `tcp_rcv_synsent_state_process` (L5862–L6086, 225 lines)

```c
static int tcp_rcv_synsent_state_process(struct sock *sk, struct sk_buff *skb,
					 const struct tcphdr *th)
{
	struct inet_connection_sock *icsk = inet_csk(sk);
	struct tcp_sock *tp = tcp_sk(sk);
	struct tcp_fastopen_cookie foc = { .len = -1 };
	int saved_clamp = tp->rx_opt.mss_clamp;
	bool fastopen_fail;

	tcp_parse_options(sock_net(sk), skb, &tp->rx_opt, 0, &foc);
	if (tp->rx_opt.saw_tstamp && tp->rx_opt.rcv_tsecr)
		tp->rx_opt.rcv_tsecr -= tp->tsoffset;

	if (th->ack) {
		/* rfc793:
		 * "If the state is SYN-SENT then
		 *    first check the ACK bit
		 *      If the ACK bit is set
		 *	  If SEG.ACK =< ISS, or SEG.ACK > SND.NXT, send
		 *        a reset (unless the RST bit is set, if so drop
		 *        the segment and return)"
		 */
		if (!after(TCP_SKB_CB(skb)->ack_seq, tp->snd_una) ||
		    after(TCP_SKB_CB(skb)->ack_seq, tp->snd_nxt))
			goto reset_and_undo;

		if (tp->rx_opt.saw_tstamp && tp->rx_opt.rcv_tsecr &&
		    !between(tp->rx_opt.rcv_tsecr, tp->retrans_stamp,
			     tcp_time_stamp(tp))) {
			NET_INC_STATS(sock_net(sk),
					LINUX_MIB_PAWSACTIVEREJECTED);
			goto reset_and_undo;
		}

		/* Now ACK is acceptable.
		 *
		 * "If the RST bit is set
		 *    If the ACK was acceptable then signal the user "error:
		 *    connection reset", drop the segment, enter CLOSED state,
		 *    delete TCB, and return."
		 */

		if (th->rst) {
			tcp_reset(sk);
			goto discard;
		}

		/* rfc793:
		 *   "fifth, if neither of the SYN or RST bits is set then
		 *    drop the segment and return."
		 *
		 *    See note below!
		 *                                        --ANK(990513)
		 */
		if (!th->syn)
			goto discard_and_undo;

		/* rfc793:
		 *   "If the SYN bit is on ...
		 *    are acceptable then ...
		 *    (our SYN has been ACKed), change the connection
		 *    state to ESTABLISHED..."
		 */

		tcp_ecn_rcv_synack(tp, th);

		tcp_init_wl(tp, TCP_SKB_CB(skb)->seq);
		tcp_try_undo_spurious_syn(sk);
		tcp_ack(sk, skb, FLAG_SLOWPATH);

		/* Ok.. it's good. Set up sequence numbers and
		 * move to established.
		 */
		WRITE_ONCE(tp->rcv_nxt, TCP_SKB_CB(skb)->seq + 1);
		tp->rcv_wup = TCP_SKB_CB(skb)->seq + 1;

		/* RFC1323: The window in SYN & SYN/ACK segments is
		 * never scaled.
		 */
		tp->snd_wnd = ntohs(th->window);

		if (!tp->rx_opt.wscale_ok) {
			tp->rx_opt.snd_wscale = tp->rx_opt.rcv_wscale = 0;
			tp->window_clamp = min(tp->window_clamp, 65535U);
		}

		if (tp->rx_opt.saw_tstamp) {
			tp->rx_opt.tstamp_ok	   = 1;
			tp->tcp_header_len =
				sizeof(struct tcphdr) + TCPOLEN_TSTAMP_ALIGNED;
			tp->advmss	    -= TCPOLEN_TSTAMP_ALIGNED;
			tcp_store_ts_recent(tp);
		} else {
			tp->tcp_header_len = sizeof(struct tcphdr);
		}

		tcp_sync_mss(sk, icsk->icsk_pmtu_cookie);
		tcp_initialize_rcv_mss(sk);

		/* Remember, tcp_poll() does not lock socket!
		 * Change state from SYN-SENT only after copied_seq
		 * is initialized. */
		tp->copied_seq = tp->rcv_nxt;

		smc_check_reset_syn(tp);

		smp_mb();

		tcp_finish_connect(sk, skb);

		fastopen_fail = (tp->syn_fastopen || tp->syn_data) &&
				tcp_rcv_fastopen_synack(sk, skb, &foc);

		if (!sock_flag(sk, SOCK_DEAD)) {
			sk->sk_state_change(sk);
			sk_wake_async(sk, SOCK_WAKE_IO, POLL_OUT);
		}
		if (fastopen_fail)
			return -1;
		if (sk->sk_write_pending ||
		    icsk->icsk_accept_queue.rskq_defer_accept ||
		    inet_csk_in_pingpong_mode(sk)) {
			/* Save one ACK. Data will be ready after
			 * several ticks, if write_pending is set.
			 *
			 * It may be deleted, but with this feature tcpdumps
			 * look so _wonderfully_ clever, that I was not able
			 * to stand against the temptation 8)     --ANK
			 */
			inet_csk_schedule_ack(sk);
			tcp_enter_quickack_mode(sk, TCP_MAX_QUICKACKS);
			inet_csk_reset_xmit_timer(sk, ICSK_TIME_DACK,
						  TCP_DELACK_MAX, TCP_RTO_MAX);

discard:
			tcp_drop(sk, skb);
			return 0;
		} else {
			tcp_send_ack(sk);
		}
		return -1;
	}

	/* No ACK in the segment */

	if (th->rst) {
		/* rfc793:
		 * "If the RST bit is set
		 *
		 *      Otherwise (no ACK) drop the segment and return."
		 */

		goto discard_and_undo;
	}

	/* PAWS check. */
	if (tp->rx_opt.ts_recent_stamp && tp->rx_opt.saw_tstamp &&
	    tcp_paws_reject(&tp->rx_opt, 0))
		goto discard_and_undo;

	if (th->syn) {
		/* We see SYN without ACK. It is attempt of
		 * simultaneous connect with crossed SYNs.
		 * Particularly, it can be connect to self.
		 */
		tcp_set_state(sk, TCP_SYN_RECV);

		if (tp->rx_opt.saw_tstamp) {
			tp->rx_opt.tstamp_ok = 1;
			tcp_store_ts_recent(tp);
			tp->tcp_header_len =
				sizeof(struct tcphdr) + TCPOLEN_TSTAMP_ALIGNED;
		} else {
			tp->tcp_header_len = sizeof(struct tcphdr);
		}

		WRITE_ONCE(tp->rcv_nxt, TCP_SKB_CB(skb)->seq + 1);
		tp->copied_seq = tp->rcv_nxt;
		tp->rcv_wup = TCP_SKB_CB(skb)->seq + 1;

		/* RFC1323: The window in SYN & SYN/ACK segments is
		 * never scaled.
		 */
		tp->snd_wnd    = ntohs(th->window);
		tp->snd_wl1    = TCP_SKB_CB(skb)->seq;
		tp->max_window = tp->snd_wnd;

		tcp_ecn_rcv_syn(tp, th);

		tcp_mtup_init(sk);
		tcp_sync_mss(sk, icsk->icsk_pmtu_cookie);
		tcp_initialize_rcv_mss(sk);

		tcp_send_synack(sk);
#if 0
		/* Note, we could accept data and URG from this segment.
		 * There are no obstacles to make this (except that we must
		 * either change tcp_recvmsg() to prevent it from returning data
		 * before 3WHS completes per RFC793, or employ TCP Fast Open).
		 *
		 * However, if we ignore data in ACKless segments sometimes,
		 * we have no reasons to accept it sometimes.
		 * Also, seems the code doing it in step6 of tcp_rcv_state_process
		 * is not flawless. So, discard packet for sanity.
		 * Uncomment this return to process the data.
		 */
		return -1;
#else
		goto discard;
#endif
	}
	/* "fifth, if neither of the SYN or RST bits is set then
	 * drop the segment and return."
	 */

discard_and_undo:
	tcp_clear_options(&tp->rx_opt);
	tp->rx_opt.mss_clamp = saved_clamp;
	goto discard;

reset_and_undo:
	tcp_clear_options(&tp->rx_opt);
	tp->rx_opt.mss_clamp = saved_clamp;
	return 1;
}
```

_(no external call sites found for `tcp_rcv_synsent_state_process`)_

### Function `tcp_rcv_state_process` (L6123–L6361, 239 lines)

```c
int tcp_rcv_state_process(struct sock *sk, struct sk_buff *skb)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct inet_connection_sock *icsk = inet_csk(sk);
	const struct tcphdr *th = tcp_hdr(skb);
	struct request_sock *req;
	int queued = 0;
	bool acceptable;

	switch (sk->sk_state) {
	case TCP_CLOSE:
		goto discard;

	case TCP_LISTEN:
		if (th->ack)
			return 1;

		if (th->rst)
			goto discard;

		if (th->syn) {
			if (th->fin)
				goto discard;
			/* It is possible that we process SYN packets from backlog,
			 * so we need to make sure to disable BH and RCU right there.
			 */
			rcu_read_lock();
			local_bh_disable();
			acceptable = icsk->icsk_af_ops->conn_request(sk, skb) >= 0;
			local_bh_enable();
			rcu_read_unlock();

			if (!acceptable)
				return 1;
			consume_skb(skb);
			return 0;
		}
		goto discard;

	case TCP_SYN_SENT:
		tp->rx_opt.saw_tstamp = 0;
		tcp_mstamp_refresh(tp);
		queued = tcp_rcv_synsent_state_process(sk, skb, th);
		if (queued >= 0)
			return queued;

		/* Do step6 onward by hand. */
		tcp_urg(sk, skb, th);
		__kfree_skb(skb);
		tcp_data_snd_check(sk);
		return 0;
	}

	tcp_mstamp_refresh(tp);
	tp->rx_opt.saw_tstamp = 0;
	req = rcu_dereference_protected(tp->fastopen_rsk,
					lockdep_sock_is_held(sk));
	if (req) {
		bool req_stolen;

		WARN_ON_ONCE(sk->sk_state != TCP_SYN_RECV &&
		    sk->sk_state != TCP_FIN_WAIT1);

		if (!tcp_check_req(sk, skb, req, true, &req_stolen))
			goto discard;
	}

	if (!th->ack && !th->rst && !th->syn)
		goto discard;

	if (!tcp_validate_incoming(sk, skb, th, 0))
		return 0;

	/* step 5: check the ACK field */
	acceptable = tcp_ack(sk, skb, FLAG_SLOWPATH |
				      FLAG_UPDATE_TS_RECENT |
				      FLAG_NO_CHALLENGE_ACK) > 0;

	if (!acceptable) {
		if (sk->sk_state == TCP_SYN_RECV)
			return 1;	/* send one RST */
		tcp_send_challenge_ack(sk, skb);
		goto discard;
	}
	switch (sk->sk_state) {
	case TCP_SYN_RECV:
		tp->delivered++; /* SYN-ACK delivery isn't tracked in tcp_ack */
		if (!tp->srtt_us)
			tcp_synack_rtt_meas(sk, req);

		if (req) {
			tcp_rcv_synrecv_state_fastopen(sk);
		} else {
			tcp_try_undo_spurious_syn(sk);
			tp->retrans_stamp = 0;
			tcp_init_transfer(sk, BPF_SOCK_OPS_PASSIVE_ESTABLISHED_CB);
			tp->copied_seq = tp->rcv_nxt;
		}
		smp_mb();
		tcp_set_state(sk, TCP_ESTABLISHED);
		sk->sk_state_change(sk);

		/* Note, that this wakeup is only for marginal crossed SYN case.
		 * Passively open sockets are not waked up, because
		 * sk->sk_sleep == NULL and sk->sk_socket == NULL.
		 */
		if (sk->sk_socket)
			sk_wake_async(sk, SOCK_WAKE_IO, POLL_OUT);

		tp->snd_una = TCP_SKB_CB(skb)->ack_seq;
		tp->snd_wnd = ntohs(th->window) << tp->rx_opt.snd_wscale;
		tcp_init_wl(tp, TCP_SKB_CB(skb)->seq);

		if (tp->rx_opt.tstamp_ok)
			tp->advmss -= TCPOLEN_TSTAMP_ALIGNED;

		if (!inet_csk(sk)->icsk_ca_ops->cong_control)
			tcp_update_pacing_rate(sk);

		/* Prevent spurious tcp_cwnd_restart() on first data packet */
		tp->lsndtime = tcp_jiffies32;

		tcp_initialize_rcv_mss(sk);
		tcp_fast_path_on(tp);
		break;

	case TCP_FIN_WAIT1: {
		int tmo;

		if (req)
			tcp_rcv_synrecv_state_fastopen(sk);

		if (tp->snd_una != tp->write_seq)
			break;

		tcp_set_state(sk, TCP_FIN_WAIT2);
		sk->sk_shutdown |= SEND_SHUTDOWN;

		sk_dst_confirm(sk);

		if (!sock_flag(sk, SOCK_DEAD)) {
			/* Wake up lingering close() */
			sk->sk_state_change(sk);
			break;
		}

		if (tp->linger2 < 0) {
			tcp_done(sk);
			NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPABORTONDATA);
			return 1;
		}
		if (TCP_SKB_CB(skb)->end_seq != TCP_SKB_CB(skb)->seq &&
		    after(TCP_SKB_CB(skb)->end_seq - th->fin, tp->rcv_nxt)) {
			/* Receive out of order FIN after close() */
			if (tp->syn_fastopen && th->fin)
				tcp_fastopen_active_disable(sk);
			tcp_done(sk);
			NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPABORTONDATA);
			return 1;
		}

		tmo = tcp_fin_time(sk);
		if (tmo > TCP_TIMEWAIT_LEN) {
			inet_csk_reset_keepalive_timer(sk, tmo - TCP_TIMEWAIT_LEN);
		} else if (th->fin || sock_owned_by_user(sk)) {
			/* Bad case. We could lose such FIN otherwise.
			 * It is not a big problem, but it looks confusing
			 * and not so rare event. We still can lose it now,
			 * if it spins in bh_lock_sock(), but it is really
			 * marginal case.
			 */
			inet_csk_reset_keepalive_timer(sk, tmo);
		} else {
			tcp_time_wait(sk, TCP_FIN_WAIT2, tmo);
			goto discard;
		}
		break;
	}

	case TCP_CLOSING:
		if (tp->snd_una == tp->write_seq) {
			tcp_time_wait(sk, TCP_TIME_WAIT, 0);
			goto discard;
		}
		break;

	case TCP_LAST_ACK:
		if (tp->snd_una == tp->write_seq) {
			tcp_update_metrics(sk);
			tcp_done(sk);
			goto discard;
		}
		break;
	}

	/* step 6: check the URG bit */
	tcp_urg(sk, skb, th);

	/* step 7: process the segment text */
	switch (sk->sk_state) {
	case TCP_CLOSE_WAIT:
	case TCP_CLOSING:
	case TCP_LAST_ACK:
		if (!before(TCP_SKB_CB(skb)->seq, tp->rcv_nxt))
			break;
		/* fall through */
	case TCP_FIN_WAIT1:
	case TCP_FIN_WAIT2:
		/* RFC 793 says to queue data in these states,
		 * RFC 1122 says we MUST send a reset.
		 * BSD 4.4 also does reset.
		 */
		if (sk->sk_shutdown & RCV_SHUTDOWN) {
			if (TCP_SKB_CB(skb)->end_seq != TCP_SKB_CB(skb)->seq &&
			    after(TCP_SKB_CB(skb)->end_seq - th->fin, tp->rcv_nxt)) {
				NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPABORTONDATA);
				tcp_reset(sk);
				return 1;
			}
		}
		/* Fall through */
	case TCP_ESTABLISHED:
		tcp_data_queue(sk, skb);
		queued = 1;
		break;
	}

	/* tcp_data could move socket to TIME-WAIT */
	if (sk->sk_state != TCP_CLOSE) {
		tcp_data_snd_check(sk);
		tcp_ack_snd_check(sk);
	}

	if (!queued) {
discard:
		tcp_drop(sk, skb);
	}
	return 0;
}
```

**External callers of `tcp_rcv_state_process` at parent**

- `include/net/tcp.h:336` — `int tcp_rcv_state_process(struct sock *sk, struct sk_buff *skb);`
- `net/ipv4/tcp_ipv4.c:1583` — `	if (tcp_rcv_state_process(sk, skb)) {`
- `net/ipv4/tcp_minisocks.c:113` — `		/* Just repeat all the checks of tcp_rcv_state_process() */`
- `net/ipv4/tcp_minisocks.c:676` — `	   from SYNACK both here and in tcp_rcv_state_process().`
- `net/ipv4/tcp_minisocks.c:677` — `	   tcp_rcv_state_process() does not, hence, we do not too.`
- `net/ipv4/tcp_minisocks.c:827` — `		ret = tcp_rcv_state_process(child, skb);`
- `net/ipv4/tcp_output.c:3375` — `	 * See tcp_input.c:tcp_rcv_state_process case TCP_SYN_SENT.`
- `net/ipv6/tcp_ipv6.c:1402` — `	if (tcp_rcv_state_process(sk, skb))`

## File: `net/ipv4/tcp_ipv4.c`

- changed old-lines: [2459]

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
				      tp->copied_seq, 0);

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

- changed old-lines: [481]

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
	newtp->copied_seq = seq;
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

- changed old-lines: [3436]

### Function `tcp_connect_init` (L3367–L3441, 75 lines)

```c
static void tcp_connect_init(struct sock *sk)
{
	const struct dst_entry *dst = __sk_dst_get(sk);
	struct tcp_sock *tp = tcp_sk(sk);
	__u8 rcv_wscale;
	u32 rcv_wnd;

	/* We'll fix this up when we get a response from the other end.
	 * See tcp_input.c:tcp_rcv_state_process case TCP_SYN_SENT.
	 */
	tp->tcp_header_len = sizeof(struct tcphdr);
	if (sock_net(sk)->ipv4.sysctl_tcp_timestamps)
		tp->tcp_header_len += TCPOLEN_TSTAMP_ALIGNED;

#ifdef CONFIG_TCP_MD5SIG
	if (tp->af_specific->md5_lookup(sk, sk))
		tp->tcp_header_len += TCPOLEN_MD5SIG_ALIGNED;
#endif

	/* If user gave his TCP_MAXSEG, record it to clamp */
	if (tp->rx_opt.user_mss)
		tp->rx_opt.mss_clamp = tp->rx_opt.user_mss;
	tp->max_window = 0;
	tcp_mtup_init(sk);
	tcp_sync_mss(sk, dst_mtu(dst));

	tcp_ca_dst_init(sk, dst);

	if (!tp->window_clamp)
		tp->window_clamp = dst_metric(dst, RTAX_WINDOW);
	tp->advmss = tcp_mss_clamp(tp, dst_metric_advmss(dst));

	tcp_initialize_rcv_mss(sk);

	/* limit the window selection if the user enforce a smaller rx buffer */
	if (sk->sk_userlocks & SOCK_RCVBUF_LOCK &&
	    (tp->window_clamp > tcp_full_space(sk) || tp->window_clamp == 0))
		tp->window_clamp = tcp_full_space(sk);

	rcv_wnd = tcp_rwnd_init_bpf(sk);
	if (rcv_wnd == 0)
		rcv_wnd = dst_metric(dst, RTAX_INITRWND);

	tcp_select_initial_window(sk, tcp_full_space(sk),
				  tp->advmss - (tp->rx_opt.ts_recent_stamp ? tp->tcp_header_len - sizeof(struct tcphdr) : 0),
				  &tp->rcv_wnd,
				  &tp->window_clamp,
				  sock_net(sk)->ipv4.sysctl_tcp_window_scaling,
				  &rcv_wscale,
				  rcv_wnd);

	tp->rx_opt.rcv_wscale = rcv_wscale;
	tp->rcv_ssthresh = tp->rcv_wnd;

	sk->sk_err = 0;
	sock_reset_flag(sk, SOCK_DONE);
	tp->snd_wnd = 0;
	tcp_init_wl(tp, 0);
	tcp_write_queue_purge(sk);
	tp->snd_una = tp->write_seq;
	tp->snd_sml = tp->write_seq;
	tp->snd_up = tp->write_seq;
	tp->snd_nxt = tp->write_seq;

	if (likely(!tp->repair))
		tp->rcv_nxt = 0;
	else
		tp->rcv_tstamp = tcp_jiffies32;
	tp->rcv_wup = tp->rcv_nxt;
	tp->copied_seq = tp->rcv_nxt;

	inet_csk(sk)->icsk_rto = tcp_timeout_init(sk);
	inet_csk(sk)->icsk_retransmits = 0;
	tcp_clear_retrans(tp);
}
```

_(no external call sites found for `tcp_connect_init`)_
