# Evidence: SYZBOT-30c996283da1e6ce  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in tcp_chrono_stop / tcp_recvmsg
- mainline fix commit: `a5a7daa52edb5197a3b696afee13ef174dc2e993`
- candidate JSON: `A_SYZBOT-30c996283da1e6ce.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "tcp_chrono_stop", "fn_b": "tcp_recvmsg"}`

## Commit message

```
a5a7daa52edb5197a3b696afee13ef174dc2e993
tcp: fix data-race in tcp_recvmsg()

Reading tp->recvmsg_inq after socket lock is released
raises a KCSAN warning [1]

Replace has_tss & has_cmsg by cmsg_flags and make
sure to not read tp->recvmsg_inq a second time.

[1]
BUG: KCSAN: data-race in tcp_chrono_stop / tcp_recvmsg

write to 0xffff888126adef24 of 2 bytes by interrupt on cpu 0:
 tcp_chrono_set net/ipv4/tcp_output.c:2309 [inline]
 tcp_chrono_stop+0x14c/0x280 net/ipv4/tcp_output.c:2338
 tcp_clean_rtx_queue net/ipv4/tcp_input.c:3165 [inline]
 tcp_ack+0x274f/0x3170 net/ipv4/tcp_input.c:3688
 tcp_rcv_established+0x37e/0xf50 net/ipv4/tcp_input.c:5696
 tcp_v4_do_rcv+0x381/0x4e0 net/ipv4/tcp_ipv4.c:1561
 tcp_v4_rcv+0x19dc/0x1bb0 net/ipv4/tcp_ipv4.c:1942
 ip_protocol_deliver_rcu+0x4d/0x420 net/ipv4/ip_input.c:204
 ip_local_deliver_finish+0x110/0x140 net/ipv4/ip_input.c:231
 NF_HOOK include/linux/netfilter.h:305 [inline]
 NF_HOOK include/linux/netfilter.h:299 [inline]
 ip_local_deliver+0x133/0x210 net/ipv4/ip_input.c:252
 dst_input include/net/dst.h:442 [inline]
 ip_rcv_finish+0x121/0x160 net/ipv4/ip_input.c:413
 NF_HOOK include/linux/netfilter.h:305 [inline]
 NF_HOOK include/linux/netfilter.h:299 [inline]
 ip_rcv+0x18f/0x1a0 net/ipv4/ip_input.c:523
 __netif_receive_skb_one_core+0xa7/0xe0 net/core/dev.c:5010
 __netif_receive_skb+0x37/0xf0 net/core/dev.c:5124
 netif_receive_skb_internal+0x59/0x190 net/core/dev.c:5214
 napi_skb_finish net/core/dev.c:5677 [inline]
 napi_gro_receive+0x28f/0x330 net/core/dev.c:5710

read to 0xffff888126adef25 of 1 bytes by task 7275 on cpu 1:
 tcp_recvmsg+0x77b/0x1a30 net/ipv4/tcp.c:2187
 inet_recvmsg+0xbb/0x250 net/ipv4/af_inet.c:838
 sock_recvmsg_nosec net/socket.c:871 [inline]
 sock_recvmsg net/socket.c:889 [inline]
 sock_recvmsg+0x92/0xb0 net/socket.c:885
 sock_read_iter+0x15f/0x1e0 net/socket.c:967
 call_read_iter include/linux/fs.h:1889 [inline]
 new_sync_read+0x389/0x4f0 fs/read_write.c:414
 __vfs_read+0xb1/0xc0 fs/read_write.c:427
 vfs_read fs/read_write.c:461 [inline]
 vfs_read+0x143/0x2c0 fs/read_write.c:446
 ksys_read+0xd5/0x1b0 fs/read_write.c:587
 __do_sys_read fs/read_write.c:597 [inline]
 __se_sys_read fs/read_write.c:595 [inline]
 __x64_sys_read+0x4c/0x60 fs/read_write.c:595
 do_syscall_64+0xcc/0x370 arch/x86/entry/common.c:290
 entry_SYSCALL_64_after_hwframe+0x44/0xa9

Reported by Kernel Concurrency Sanitizer on:
CPU: 1 PID: 7275 Comm: sshd Not tainted 5.4.0-rc3+ #0
Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 01/01/2011

Fixes: b75eba76d3d7 ("tcp: send in-queue bytes in cmsg upon read")
Signed-off-by: Eric Dumazet <edumazet@google.com>
Acked-by: Soheil Hassas Yeganeh <soheil@google.com>
Reported-by: syzbot <syzkaller@googlegroups.com>
Signed-off-by: David S. Miller <davem@davemloft.net>
```

## CVE / bug description

```
KCSAN: data-race in tcp_chrono_stop / tcp_recvmsg
```

## Full mainline patch

```diff
commit a5a7daa52edb5197a3b696afee13ef174dc2e993
Author: Eric Dumazet <edumazet@google.com>
Date:   Wed Nov 6 12:59:33 2019 -0800

    tcp: fix data-race in tcp_recvmsg()
    
    Reading tp->recvmsg_inq after socket lock is released
    raises a KCSAN warning [1]
    
    Replace has_tss & has_cmsg by cmsg_flags and make
    sure to not read tp->recvmsg_inq a second time.
    
    [1]
    BUG: KCSAN: data-race in tcp_chrono_stop / tcp_recvmsg
    
    write to 0xffff888126adef24 of 2 bytes by interrupt on cpu 0:
     tcp_chrono_set net/ipv4/tcp_output.c:2309 [inline]
     tcp_chrono_stop+0x14c/0x280 net/ipv4/tcp_output.c:2338
     tcp_clean_rtx_queue net/ipv4/tcp_input.c:3165 [inline]
     tcp_ack+0x274f/0x3170 net/ipv4/tcp_input.c:3688
     tcp_rcv_established+0x37e/0xf50 net/ipv4/tcp_input.c:5696
     tcp_v4_do_rcv+0x381/0x4e0 net/ipv4/tcp_ipv4.c:1561
     tcp_v4_rcv+0x19dc/0x1bb0 net/ipv4/tcp_ipv4.c:1942
     ip_protocol_deliver_rcu+0x4d/0x420 net/ipv4/ip_input.c:204
     ip_local_deliver_finish+0x110/0x140 net/ipv4/ip_input.c:231
     NF_HOOK include/linux/netfilter.h:305 [inline]
     NF_HOOK include/linux/netfilter.h:299 [inline]
     ip_local_deliver+0x133/0x210 net/ipv4/ip_input.c:252
     dst_input include/net/dst.h:442 [inline]
     ip_rcv_finish+0x121/0x160 net/ipv4/ip_input.c:413
     NF_HOOK include/linux/netfilter.h:305 [inline]
     NF_HOOK include/linux/netfilter.h:299 [inline]
     ip_rcv+0x18f/0x1a0 net/ipv4/ip_input.c:523
     __netif_receive_skb_one_core+0xa7/0xe0 net/core/dev.c:5010
     __netif_receive_skb+0x37/0xf0 net/core/dev.c:5124
     netif_receive_skb_internal+0x59/0x190 net/core/dev.c:5214
     napi_skb_finish net/core/dev.c:5677 [inline]
     napi_gro_receive+0x28f/0x330 net/core/dev.c:5710
    
    read to 0xffff888126adef25 of 1 bytes by task 7275 on cpu 1:
     tcp_recvmsg+0x77b/0x1a30 net/ipv4/tcp.c:2187
     inet_recvmsg+0xbb/0x250 net/ipv4/af_inet.c:838
     sock_recvmsg_nosec net/socket.c:871 [inline]
     sock_recvmsg net/socket.c:889 [inline]
     sock_recvmsg+0x92/0xb0 net/socket.c:885
     sock_read_iter+0x15f/0x1e0 net/socket.c:967
     call_read_iter include/linux/fs.h:1889 [inline]
     new_sync_read+0x389/0x4f0 fs/read_write.c:414
     __vfs_read+0xb1/0xc0 fs/read_write.c:427
     vfs_read fs/read_write.c:461 [inline]
     vfs_read+0x143/0x2c0 fs/read_write.c:446
     ksys_read+0xd5/0x1b0 fs/read_write.c:587
     __do_sys_read fs/read_write.c:597 [inline]
     __se_sys_read fs/read_write.c:595 [inline]
     __x64_sys_read+0x4c/0x60 fs/read_write.c:595
     do_syscall_64+0xcc/0x370 arch/x86/entry/common.c:290
     entry_SYSCALL_64_after_hwframe+0x44/0xa9
    
    Reported by Kernel Concurrency Sanitizer on:
    CPU: 1 PID: 7275 Comm: sshd Not tainted 5.4.0-rc3+ #0
    Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 01/01/2011
    
    Fixes: b75eba76d3d7 ("tcp: send in-queue bytes in cmsg upon read")
    Signed-off-by: Eric Dumazet <edumazet@google.com>
    Acked-by: Soheil Hassas Yeganeh <soheil@google.com>
    Reported-by: syzbot <syzkaller@googlegroups.com>
    Signed-off-by: David S. Miller <davem@davemloft.net>

diff --git a/net/ipv4/tcp.c b/net/ipv4/tcp.c
index 8fb4fefcfd54..9b48aec29aca 100644
--- a/net/ipv4/tcp.c
+++ b/net/ipv4/tcp.c
@@ -1958,8 +1958,7 @@ int tcp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int nonblock,
 	struct sk_buff *skb, *last;
 	u32 urg_hole = 0;
 	struct scm_timestamping_internal tss;
-	bool has_tss = false;
-	bool has_cmsg;
+	int cmsg_flags;
 
 	if (unlikely(flags & MSG_ERRQUEUE))
 		return inet_recv_error(sk, msg, len, addr_len);
@@ -1974,7 +1973,7 @@ int tcp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int nonblock,
 	if (sk->sk_state == TCP_LISTEN)
 		goto out;
 
-	has_cmsg = tp->recvmsg_inq;
+	cmsg_flags = tp->recvmsg_inq ? 1 : 0;
 	timeo = sock_rcvtimeo(sk, nonblock);
 
 	/* Urgent data needs to be handled specially. */
@@ -2157,8 +2156,7 @@ int tcp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int nonblock,
 
 		if (TCP_SKB_CB(skb)->has_rxtstamp) {
 			tcp_update_recv_tstamps(skb, &tss);
-			has_tss = true;
-			has_cmsg = true;
+			cmsg_flags |= 2;
 		}
 		if (TCP_SKB_CB(skb)->tcp_flags & TCPHDR_FIN)
 			goto found_fin_ok;
@@ -2183,10 +2181,10 @@ int tcp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int nonblock,
 
 	release_sock(sk);
 
-	if (has_cmsg) {
-		if (has_tss)
+	if (cmsg_flags) {
+		if (cmsg_flags & 2)
 			tcp_recv_timestamp(msg, sk, &tss);
-		if (tp->recvmsg_inq) {
+		if (cmsg_flags & 1) {
 			inq = tcp_inq_hint(sk);
 			put_cmsg(msg, SOL_TCP, TCP_CM_INQ, sizeof(inq), &inq);
 		}

```

## File: `net/ipv4/tcp.c`

- changed old-lines: [1961, 1962, 1977, 2160, 2161, 2186, 2187, 2189]

### Function `tcp_recvmsg` (L1947–L2208, 262 lines)

```c
int tcp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int nonblock,
		int flags, int *addr_len)
{
	struct tcp_sock *tp = tcp_sk(sk);
	int copied = 0;
	u32 peek_seq;
	u32 *seq;
	unsigned long used;
	int err, inq;
	int target;		/* Read at least this many bytes */
	long timeo;
	struct sk_buff *skb, *last;
	u32 urg_hole = 0;
	struct scm_timestamping_internal tss;
	bool has_tss = false;
	bool has_cmsg;

	if (unlikely(flags & MSG_ERRQUEUE))
		return inet_recv_error(sk, msg, len, addr_len);

	if (sk_can_busy_loop(sk) && skb_queue_empty_lockless(&sk->sk_receive_queue) &&
	    (sk->sk_state == TCP_ESTABLISHED))
		sk_busy_loop(sk, nonblock);

	lock_sock(sk);

	err = -ENOTCONN;
	if (sk->sk_state == TCP_LISTEN)
		goto out;

	has_cmsg = tp->recvmsg_inq;
	timeo = sock_rcvtimeo(sk, nonblock);

	/* Urgent data needs to be handled specially. */
	if (flags & MSG_OOB)
		goto recv_urg;

	if (unlikely(tp->repair)) {
		err = -EPERM;
		if (!(flags & MSG_PEEK))
			goto out;

		if (tp->repair_queue == TCP_SEND_QUEUE)
			goto recv_sndq;

		err = -EINVAL;
		if (tp->repair_queue == TCP_NO_QUEUE)
			goto out;

		/* 'common' recv queue MSG_PEEK-ing */
	}

	seq = &tp->copied_seq;
	if (flags & MSG_PEEK) {
		peek_seq = tp->copied_seq;
		seq = &peek_seq;
	}

	target = sock_rcvlowat(sk, flags & MSG_WAITALL, len);

	do {
		u32 offset;

		/* Are we at urgent data? Stop if we have read anything or have SIGURG pending. */
		if (tp->urg_data && tp->urg_seq == *seq) {
			if (copied)
				break;
			if (signal_pending(current)) {
				copied = timeo ? sock_intr_errno(timeo) : -EAGAIN;
				break;
			}
		}

		/* Next get a buffer. */

		last = skb_peek_tail(&sk->sk_receive_queue);
		skb_queue_walk(&sk->sk_receive_queue, skb) {
			last = skb;
			/* Now that we have two receive queues this
			 * shouldn't happen.
			 */
			if (WARN(before(*seq, TCP_SKB_CB(skb)->seq),
				 "TCP recvmsg seq # bug: copied %X, seq %X, rcvnxt %X, fl %X\n",
				 *seq, TCP_SKB_CB(skb)->seq, tp->rcv_nxt,
				 flags))
				break;

			offset = *seq - TCP_SKB_CB(skb)->seq;
			if (unlikely(TCP_SKB_CB(skb)->tcp_flags & TCPHDR_SYN)) {
				pr_err_once("%s: found a SYN, please report !\n", __func__);
				offset--;
			}
			if (offset < skb->len)
				goto found_ok_skb;
			if (TCP_SKB_CB(skb)->tcp_flags & TCPHDR_FIN)
				goto found_fin_ok;
			WARN(!(flags & MSG_PEEK),
			     "TCP recvmsg seq # bug 2: copied %X, seq %X, rcvnxt %X, fl %X\n",
			     *seq, TCP_SKB_CB(skb)->seq, tp->rcv_nxt, flags);
		}

		/* Well, if we have backlog, try to process it now yet. */

		if (copied >= target && !READ_ONCE(sk->sk_backlog.tail))
			break;

		if (copied) {
			if (sk->sk_err ||
			    sk->sk_state == TCP_CLOSE ||
			    (sk->sk_shutdown & RCV_SHUTDOWN) ||
			    !timeo ||
			    signal_pending(current))
				break;
		} else {
			if (sock_flag(sk, SOCK_DONE))
				break;

			if (sk->sk_err) {
				copied = sock_error(sk);
				break;
			}

			if (sk->sk_shutdown & RCV_SHUTDOWN)
				break;

			if (sk->sk_state == TCP_CLOSE) {
				/* This occurs when user tries to read
				 * from never connected socket.
				 */
				copied = -ENOTCONN;
				break;
			}

			if (!timeo) {
				copied = -EAGAIN;
				break;
			}

			if (signal_pending(current)) {
				copied = sock_intr_errno(timeo);
				break;
			}
		}

		tcp_cleanup_rbuf(sk, copied);

		if (copied >= target) {
			/* Do not sleep, just process backlog. */
			release_sock(sk);
			lock_sock(sk);
		} else {
			sk_wait_data(sk, &timeo, last);
		}

		if ((flags & MSG_PEEK) &&
		    (peek_seq - copied - urg_hole != tp->copied_seq)) {
			net_dbg_ratelimited("TCP(%s:%d): Application bug, race in MSG_PEEK\n",
					    current->comm,
					    task_pid_nr(current));
			peek_seq = tp->copied_seq;
		}
		continue;

found_ok_skb:
		/* Ok so how much can we use? */
		used = skb->len - offset;
		if (len < used)
			used = len;

		/* Do we have urgent data here? */
		if (tp->urg_data) {
			u32 urg_offset = tp->urg_seq - *seq;
			if (urg_offset < used) {
				if (!urg_offset) {
					if (!sock_flag(sk, SOCK_URGINLINE)) {
						WRITE_ONCE(*seq, *seq + 1);
						urg_hole++;
						offset++;
						used--;
						if (!used)
							goto skip_copy;
					}
				} else
					used = urg_offset;
			}
		}

		if (!(flags & MSG_TRUNC)) {
			err = skb_copy_datagram_msg(skb, offset, msg, used);
			if (err) {
				/* Exception. Bailout! */
				if (!copied)
					copied = -EFAULT;
				break;
			}
		}

		WRITE_ONCE(*seq, *seq + used);
		copied += used;
		len -= used;

		tcp_rcv_space_adjust(sk);

skip_copy:
		if (tp->urg_data && after(tp->copied_seq, tp->urg_seq)) {
			tp->urg_data = 0;
			tcp_fast_path_check(sk);
		}
		if (used + offset < skb->len)
			continue;

		if (TCP_SKB_CB(skb)->has_rxtstamp) {
			tcp_update_recv_tstamps(skb, &tss);
			has_tss = true;
			has_cmsg = true;
		}
		if (TCP_SKB_CB(skb)->tcp_flags & TCPHDR_FIN)
			goto found_fin_ok;
		if (!(flags & MSG_PEEK))
			sk_eat_skb(sk, skb);
		continue;

found_fin_ok:
		/* Process the FIN. */
		WRITE_ONCE(*seq, *seq + 1);
		if (!(flags & MSG_PEEK))
			sk_eat_skb(sk, skb);
		break;
	} while (len > 0);

	/* According to UNIX98, msg_name/msg_namelen are ignored
	 * on connected socket. I was just happy when found this 8) --ANK
	 */

	/* Clean up data we have read: This will do ACK frames. */
	tcp_cleanup_rbuf(sk, copied);

	release_sock(sk);

	if (has_cmsg) {
		if (has_tss)
			tcp_recv_timestamp(msg, sk, &tss);
		if (tp->recvmsg_inq) {
			inq = tcp_inq_hint(sk);
			put_cmsg(msg, SOL_TCP, TCP_CM_INQ, sizeof(inq), &inq);
		}
	}

	return copied;

out:
	release_sock(sk);
	return err;

recv_urg:
	err = tcp_recv_urg(sk, msg, len, flags);
	goto out;

recv_sndq:
	err = tcp_peek_sndq(sk, msg, len);
	goto out;
}
```

**External callers of `tcp_recvmsg` at parent**

- `arch/microblaze/kernel/unwind.c:83` — `	 *	 750 picks up things like tcp_recvmsg(),`
- `include/net/tcp.h:404` — `int tcp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int nonblock,`
- `net/ipv4/af_inet.c:838` — `	err = INDIRECT_CALL_2(sk->sk_prot->recvmsg, tcp_recvmsg, udp_recvmsg,`
- `net/ipv4/tcp_bpf.c:127` — `		return tcp_recvmsg(sk, msg, len, nonblock, flags, addr_len);`
- `net/ipv4/tcp_bpf.c:131` — `		return tcp_recvmsg(sk, msg, len, nonblock, flags, addr_len);`
- `net/ipv4/tcp_bpf.c:146` — `			return tcp_recvmsg(sk, msg, len, nonblock, flags, addr_len);`
- `net/ipv4/tcp_bpf.c:661` — `	return ops->recvmsg  == tcp_recvmsg &&`
- `net/ipv4/tcp_input.c:5232` — `	      * (tcp_recvmsg() will send ACK otherwise).`
- `net/ipv4/tcp_input.c:6065` — `		 * either change tcp_recvmsg() to prevent it from returning data`
- `net/ipv4/tcp_ipv4.c:2584` — `	.recvmsg		= tcp_recvmsg,`
- `net/ipv6/af_inet6.c:592` — `	err = INDIRECT_CALL_2(sk->sk_prot->recvmsg, tcp_recvmsg, udpv6_recvmsg,`
- `net/ipv6/tcp_ipv6.c:2021` — `	.recvmsg		= tcp_recvmsg,`
