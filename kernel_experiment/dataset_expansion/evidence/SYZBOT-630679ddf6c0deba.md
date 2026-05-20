# Evidence: SYZBOT-630679ddf6c0deba  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in tcp_add_backlog / tcp_try_rmem_schedule
- mainline fix commit: `ebb3b78db7bf842270a46fd4fe7cc45c78fa5ed6`
- candidate JSON: `A_SYZBOT-630679ddf6c0deba.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "tcp_add_backlog", "fn_b": "tcp_try_rmem_schedule"}`

## Commit message

```
ebb3b78db7bf842270a46fd4fe7cc45c78fa5ed6
tcp: annotate sk->sk_rcvbuf lockless reads

For the sake of tcp_poll(), there are few places where we fetch
sk->sk_rcvbuf while this field can change from IRQ or other cpu.

We need to add READ_ONCE() annotations, and also make sure write
sides use corresponding WRITE_ONCE() to avoid store-tearing.

Note that other transports probably need similar fixes.

Signed-off-by: Eric Dumazet <edumazet@google.com>
Signed-off-by: David S. Miller <davem@davemloft.net>
```

## CVE / bug description

```
KCSAN: data-race in tcp_add_backlog / tcp_try_rmem_schedule
```

## Full mainline patch

```diff
commit ebb3b78db7bf842270a46fd4fe7cc45c78fa5ed6
Author: Eric Dumazet <edumazet@google.com>
Date:   Thu Oct 10 20:17:44 2019 -0700

    tcp: annotate sk->sk_rcvbuf lockless reads
    
    For the sake of tcp_poll(), there are few places where we fetch
    sk->sk_rcvbuf while this field can change from IRQ or other cpu.
    
    We need to add READ_ONCE() annotations, and also make sure write
    sides use corresponding WRITE_ONCE() to avoid store-tearing.
    
    Note that other transports probably need similar fixes.
    
    Signed-off-by: Eric Dumazet <edumazet@google.com>
    Signed-off-by: David S. Miller <davem@davemloft.net>

diff --git a/include/net/tcp.h b/include/net/tcp.h
index e1d08f69fd39..ab4eb5eb5d07 100644
--- a/include/net/tcp.h
+++ b/include/net/tcp.h
@@ -1380,14 +1380,14 @@ static inline int tcp_win_from_space(const struct sock *sk, int space)
 /* Note: caller must be prepared to deal with negative returns */
 static inline int tcp_space(const struct sock *sk)
 {
-	return tcp_win_from_space(sk, sk->sk_rcvbuf -
+	return tcp_win_from_space(sk, READ_ONCE(sk->sk_rcvbuf) -
 				  READ_ONCE(sk->sk_backlog.len) -
 				  atomic_read(&sk->sk_rmem_alloc));
 }
 
 static inline int tcp_full_space(const struct sock *sk)
 {
-	return tcp_win_from_space(sk, sk->sk_rcvbuf);
+	return tcp_win_from_space(sk, READ_ONCE(sk->sk_rcvbuf));
 }
 
 extern void tcp_openreq_init_rwin(struct request_sock *req,
diff --git a/include/trace/events/sock.h b/include/trace/events/sock.h
index a0c4b8a30966..f720c32e7dfd 100644
--- a/include/trace/events/sock.h
+++ b/include/trace/events/sock.h
@@ -82,7 +82,7 @@ TRACE_EVENT(sock_rcvqueue_full,
 	TP_fast_assign(
 		__entry->rmem_alloc = atomic_read(&sk->sk_rmem_alloc);
 		__entry->truesize   = skb->truesize;
-		__entry->sk_rcvbuf  = sk->sk_rcvbuf;
+		__entry->sk_rcvbuf  = READ_ONCE(sk->sk_rcvbuf);
 	),
 
 	TP_printk("rmem_alloc=%d truesize=%u sk_rcvbuf=%d",
diff --git a/net/core/filter.c b/net/core/filter.c
index a50c0b6846f2..7deceaeeed7b 100644
--- a/net/core/filter.c
+++ b/net/core/filter.c
@@ -4252,7 +4252,8 @@ BPF_CALL_5(bpf_setsockopt, struct bpf_sock_ops_kern *, bpf_sock,
 		case SO_RCVBUF:
 			val = min_t(u32, val, sysctl_rmem_max);
 			sk->sk_userlocks |= SOCK_RCVBUF_LOCK;
-			sk->sk_rcvbuf = max_t(int, val * 2, SOCK_MIN_RCVBUF);
+			WRITE_ONCE(sk->sk_rcvbuf,
+				   max_t(int, val * 2, SOCK_MIN_RCVBUF));
 			break;
 		case SO_SNDBUF:
 			val = min_t(u32, val, sysctl_wmem_max);
diff --git a/net/core/skbuff.c b/net/core/skbuff.c
index 529133611ea2..8c178703467b 100644
--- a/net/core/skbuff.c
+++ b/net/core/skbuff.c
@@ -4415,7 +4415,7 @@ static void skb_set_err_queue(struct sk_buff *skb)
 int sock_queue_err_skb(struct sock *sk, struct sk_buff *skb)
 {
 	if (atomic_read(&sk->sk_rmem_alloc) + skb->truesize >=
-	    (unsigned int)sk->sk_rcvbuf)
+	    (unsigned int)READ_ONCE(sk->sk_rcvbuf))
 		return -ENOMEM;
 
 	skb_orphan(skb);
diff --git a/net/core/sock.c b/net/core/sock.c
index 2a053999df11..8c8f61e70141 100644
--- a/net/core/sock.c
+++ b/net/core/sock.c
@@ -831,7 +831,8 @@ int sock_setsockopt(struct socket *sock, int level, int optname,
 		 * returning the value we actually used in getsockopt
 		 * is the most desirable behavior.
 		 */
-		sk->sk_rcvbuf = max_t(int, val * 2, SOCK_MIN_RCVBUF);
+		WRITE_ONCE(sk->sk_rcvbuf,
+			   max_t(int, val * 2, SOCK_MIN_RCVBUF));
 		break;
 
 	case SO_RCVBUFFORCE:
@@ -3204,7 +3205,7 @@ void sk_get_meminfo(const struct sock *sk, u32 *mem)
 	memset(mem, 0, sizeof(*mem) * SK_MEMINFO_VARS);
 
 	mem[SK_MEMINFO_RMEM_ALLOC] = sk_rmem_alloc_get(sk);
-	mem[SK_MEMINFO_RCVBUF] = sk->sk_rcvbuf;
+	mem[SK_MEMINFO_RCVBUF] = READ_ONCE(sk->sk_rcvbuf);
 	mem[SK_MEMINFO_WMEM_ALLOC] = sk_wmem_alloc_get(sk);
 	mem[SK_MEMINFO_SNDBUF] = sk->sk_sndbuf;
 	mem[SK_MEMINFO_FWD_ALLOC] = sk->sk_forward_alloc;
diff --git a/net/ipv4/tcp.c b/net/ipv4/tcp.c
index 577a8c6eef9f..bc0481aa6633 100644
--- a/net/ipv4/tcp.c
+++ b/net/ipv4/tcp.c
@@ -451,7 +451,7 @@ void tcp_init_sock(struct sock *sk)
 	icsk->icsk_sync_mss = tcp_sync_mss;
 
 	sk->sk_sndbuf = sock_net(sk)->ipv4.sysctl_tcp_wmem[1];
-	sk->sk_rcvbuf = sock_net(sk)->ipv4.sysctl_tcp_rmem[1];
+	WRITE_ONCE(sk->sk_rcvbuf, sock_net(sk)->ipv4.sysctl_tcp_rmem[1]);
 
 	sk_sockets_allocated_inc(sk);
 	sk->sk_route_forced_caps = NETIF_F_GSO;
@@ -1711,7 +1711,7 @@ int tcp_set_rcvlowat(struct sock *sk, int val)
 
 	val <<= 1;
 	if (val > sk->sk_rcvbuf) {
-		sk->sk_rcvbuf = val;
+		WRITE_ONCE(sk->sk_rcvbuf, val);
 		tcp_sk(sk)->window_clamp = tcp_win_from_space(sk, val);
 	}
 	return 0;
diff --git a/net/ipv4/tcp_input.c b/net/ipv4/tcp_input.c
index 16342e043ab3..6995df20710a 100644
--- a/net/ipv4/tcp_input.c
+++ b/net/ipv4/tcp_input.c
@@ -483,8 +483,9 @@ static void tcp_clamp_window(struct sock *sk)
 	    !(sk->sk_userlocks & SOCK_RCVBUF_LOCK) &&
 	    !tcp_under_memory_pressure(sk) &&
 	    sk_memory_allocated(sk) < sk_prot_mem_limits(sk, 0)) {
-		sk->sk_rcvbuf = min(atomic_read(&sk->sk_rmem_alloc),
-				    net->ipv4.sysctl_tcp_rmem[2]);
+		WRITE_ONCE(sk->sk_rcvbuf,
+			   min(atomic_read(&sk->sk_rmem_alloc),
+			       net->ipv4.sysctl_tcp_rmem[2]));
 	}
 	if (atomic_read(&sk->sk_rmem_alloc) > sk->sk_rcvbuf)
 		tp->rcv_ssthresh = min(tp->window_clamp, 2U * tp->advmss);
@@ -648,7 +649,7 @@ void tcp_rcv_space_adjust(struct sock *sk)
 		rcvbuf = min_t(u64, rcvwin * rcvmem,
 			       sock_net(sk)->ipv4.sysctl_tcp_rmem[2]);
 		if (rcvbuf > sk->sk_rcvbuf) {
-			sk->sk_rcvbuf = rcvbuf;
+			WRITE_ONCE(sk->sk_rcvbuf, rcvbuf);
 
 			/* Make the window clamp follow along.  */
 			tp->window_clamp = tcp_win_from_space(sk, rcvbuf);

```

## File: `include/net/tcp.h`

- changed old-lines: [1383, 1390]

### Function `tcp_space` (L1381–L1386, 6 lines)

```c
static inline int tcp_space(const struct sock *sk)
{
	return tcp_win_from_space(sk, sk->sk_rcvbuf -
				  READ_ONCE(sk->sk_backlog.len) -
				  atomic_read(&sk->sk_rmem_alloc));
}
```

**External callers of `tcp_space` at parent**

- `net/ipv4/tcp_input.c:413` — `	room = min_t(int, tp->window_clamp, tcp_space(sk)) - tp->rcv_ssthresh;`
- `net/ipv4/tcp_output.c:2696` — `	int free_space = tcp_space(sk);`

### Function `tcp_full_space` (L1388–L1391, 4 lines)

```c
static inline int tcp_full_space(const struct sock *sk)
{
	return tcp_win_from_space(sk, sk->sk_rcvbuf);
}
```

**External callers of `tcp_full_space` at parent**

- `drivers/crypto/chelsio/chtls/chtls_cm.c:1084` — `	RCV_WSCALE(tp) = select_rcv_wscale(tcp_full_space(newsk),`
- `drivers/crypto/chelsio/chtls/chtls_io.c:1314` — `	wnd = max_t(unsigned int, wnd, tcp_full_space(sk));`
- `net/ipv4/syncookies.c:390` — `	tcp_select_initial_window(sk, tcp_full_space(sk), req->mss,`
- `net/ipv4/tcp_input.c:367` — ` * All tcp_full_space() is split to two parts: "network" buffer, allocated`
- `net/ipv4/tcp_input.c:372` — ` * tcp_full_space(), in this case tcp_full_space() - window_clamp`
- `net/ipv4/tcp_input.c:452` — `	maxwin = tcp_full_space(sk);`
- `net/ipv4/tcp_minisocks.c:366` — `	int full_space = tcp_full_space(sk_listener);`
- `net/ipv4/tcp_minisocks.c:388` — `	/* tcp_full_space because it is guaranteed to be the first packet */`
- `net/ipv4/tcp_output.c:2697` — `	int allowed_space = tcp_full_space(sk);`
- `net/ipv4/tcp_output.c:3403` — `	    (tp->window_clamp > tcp_full_space(sk) || tp->window_clamp == 0))`
- `net/ipv4/tcp_output.c:3404` — `		tp->window_clamp = tcp_full_space(sk);`
- `net/ipv4/tcp_output.c:3410` — `	tcp_select_initial_window(sk, tcp_full_space(sk),`
- `net/ipv6/syncookies.c:244` — `	tcp_select_initial_window(sk, tcp_full_space(sk), req->mss,`

## File: `include/trace/events/sock.h`

- changed old-lines: [85]

## File: `net/core/filter.c`

- changed old-lines: [4255]

### Function `if` (L4242–L4333, 92 lines)

```c
	if (!sk_fullsock(sk))
		return -EINVAL;

	if (level == SOL_SOCKET) {
		if (optlen != sizeof(int))
			return -EINVAL;
		val = *((int *)optval);

		/* Only some socketops are supported */
		switch (optname) {
		case SO_RCVBUF:
			val = min_t(u32, val, sysctl_rmem_max);
			sk->sk_userlocks |= SOCK_RCVBUF_LOCK;
			sk->sk_rcvbuf = max_t(int, val * 2, SOCK_MIN_RCVBUF);
			break;
		case SO_SNDBUF:
			val = min_t(u32, val, sysctl_wmem_max);
			sk->sk_userlocks |= SOCK_SNDBUF_LOCK;
			sk->sk_sndbuf = max_t(int, val * 2, SOCK_MIN_SNDBUF);
			break;
		case SO_MAX_PACING_RATE: /* 32bit version */
			if (val != ~0U)
				cmpxchg(&sk->sk_pacing_status,
					SK_PACING_NONE,
					SK_PACING_NEEDED);
			sk->sk_max_pacing_rate = (val == ~0U) ? ~0UL : val;
			sk->sk_pacing_rate = min(sk->sk_pacing_rate,
						 sk->sk_max_pacing_rate);
			break;
		case SO_PRIORITY:
			sk->sk_priority = val;
			break;
		case SO_RCVLOWAT:
			if (val < 0)
				val = INT_MAX;
			WRITE_ONCE(sk->sk_rcvlowat, val ? : 1);
			break;
		case SO_MARK:
			if (sk->sk_mark != val) {
				sk->sk_mark = val;
				sk_dst_reset(sk);
			}
			break;
		default:
			ret = -EINVAL;
		}
#ifdef CONFIG_INET
	} else if (level == SOL_IP) {
		if (optlen != sizeof(int) || sk->sk_family != AF_INET)
			return -EINVAL;

		val = *((int *)optval);
		/* Only some options are supported */
		switch (optname) {
		case IP_TOS:
			if (val < -1 || val > 0xff) {
				ret = -EINVAL;
			} else {
				struct inet_sock *inet = inet_sk(sk);

				if (val == -1)
					val = 0;
				inet->tos = val;
			}
			break;
		default:
			ret = -EINVAL;
		}
#if IS_ENABLED(CONFIG_IPV6)
	} else if (level == SOL_IPV6) {
		if (optlen != sizeof(int) || sk->sk_family != AF_INET6)
			return -EINVAL;

		val = *((int *)optval);
		/* Only some options are supported */
		switch (optname) {
		case IPV6_TCLASS:
			if (val < -1 || val > 0xff) {
				ret = -EINVAL;
			} else {
				struct ipv6_pinfo *np = inet6_sk(sk);

				if (val == -1)
					val = 0;
				np->tclass = val;
			}
			break;
		default:
			ret = -EINVAL;
		}
#endif
	} else if (level == SOL_TCP &&
```

**External callers of `if` at parent**

- `Documentation/scheduler/sched-pelt.c:28` — `		if (i % 6 == 0) printf("\n\t");`
- `Documentation/scheduler/sched-pelt.c:42` — `		if (i == 1)`
- `Documentation/scheduler/sched-pelt.c:47` — `		if (i % 11 == 0)`
- `Documentation/scheduler/sched-pelt.c:64` — `		if (n > -1)`
- `Documentation/scheduler/sched-pelt.c:71` — `		if (last == max)`
- `Documentation/scheduler/sched-pelt.c:88` — `		if (i > 1)`
- `Documentation/scheduler/sched-pelt.c:91` — `		if (i % 6 == 0)`
- `Documentation/usb/usbdevfs-drop-permissions.c:24` — `	if (res)`
- `Documentation/usb/usbdevfs-drop-permissions.c:35` — `	if (!res)`
- `Documentation/usb/usbdevfs-drop-permissions.c:48` — `		if (!res)`
- `Documentation/usb/usbdevfs-drop-permissions.c:49` — `			printf("OK: claimed if %d\n", i);`
- `Documentation/usb/usbdevfs-drop-permissions.c:51` — `			printf("ERROR claiming if %d (%d - %s)\n",`
- `Documentation/usb/usbdevfs-drop-permissions.c:62` — `	if (fd < 0) {`
- `Documentation/usb/usbdevfs-drop-permissions.c:68` — `	 * check if dropping privileges is supported,`
- `Documentation/usb/usbdevfs-drop-permissions.c:72` — `	if (!(caps & USBDEVFS_CAP_DROP_PRIVILEGES)) {`
- `Documentation/usb/usbdevfs-drop-permissions.c:85` — `		"[1] Reset device. Should fail if device is in use\n"`
- `arch/alpha/boot/bootp.c:97` — `	if (i) {`
- `arch/alpha/boot/bootp.c:154` — `	if (INIT_HWRPB->pagesize != 8192) {`
- `arch/alpha/boot/bootp.c:159` — `	if (INIT_HWRPB->vptb != (unsigned long) VPTB) {`
- `arch/alpha/boot/bootp.c:181` — `	if (nbytes < 0 || nbytes >= sizeof(envval)) {`

## File: `net/core/skbuff.c`

- changed old-lines: [4418]

### Function `sock_queue_err_skb` (L4415–L4434, 20 lines)

```c
int sock_queue_err_skb(struct sock *sk, struct sk_buff *skb)
{
	if (atomic_read(&sk->sk_rmem_alloc) + skb->truesize >=
	    (unsigned int)sk->sk_rcvbuf)
		return -ENOMEM;

	skb_orphan(skb);
	skb->sk = sk;
	skb->destructor = sock_rmem_free;
	atomic_add(skb->truesize, &sk->sk_rmem_alloc);
	skb_set_err_queue(skb);

	/* before exiting rcu section, make sure dst is refcounted */
	skb_dst_force(skb);

	skb_queue_tail(&sk->sk_error_queue, skb);
	if (!sock_flag(sk, SOCK_DEAD))
		sk->sk_error_report(sk);
	return 0;
}
```

**External callers of `sock_queue_err_skb` at parent**

- `drivers/net/arcnet/arcnet.c:435` — `	ret = sock_queue_err_skb(sk, ackskb);`
- `include/net/sock.h:2152` — `int sock_queue_err_skb(struct sock *sk, struct sk_buff *skb);`
- `net/can/j1939/socket.c:945` — `	err = sock_queue_err_skb(sk, skb);`
- `net/ipv4/ip_sockglue.c:415` — `		if (sock_queue_err_skb(sk, skb) == 0)`
- `net/ipv4/ip_sockglue.c:454` — `	if (sock_queue_err_skb(sk, skb))`
- `net/ipv6/datagram.c:318` — `	if (sock_queue_err_skb(sk, skb))`
- `net/ipv6/datagram.c:358` — `	if (sock_queue_err_skb(sk, skb))`
- `net/sched/sch_etf.c:157` — `	if (sock_queue_err_skb(skb->sk, clone))`

## File: `net/core/sock.c`

- changed old-lines: [834, 3207]

### Function `sock_setsockopt` (L722–L1184, 463 lines)

```c
int sock_setsockopt(struct socket *sock, int level, int optname,
		    char __user *optval, unsigned int optlen)
{
	struct sock_txtime sk_txtime;
	struct sock *sk = sock->sk;
	int val;
	int valbool;
	struct linger ling;
	int ret = 0;

	/*
	 *	Options without arguments
	 */

	if (optname == SO_BINDTODEVICE)
		return sock_setbindtodevice(sk, optval, optlen);

	if (optlen < sizeof(int))
		return -EINVAL;

	if (get_user(val, (int __user *)optval))
		return -EFAULT;

	valbool = val ? 1 : 0;

	lock_sock(sk);

	switch (optname) {
	case SO_DEBUG:
		if (val && !capable(CAP_NET_ADMIN))
			ret = -EACCES;
		else
			sock_valbool_flag(sk, SOCK_DBG, valbool);
		break;
	case SO_REUSEADDR:
		sk->sk_reuse = (valbool ? SK_CAN_REUSE : SK_NO_REUSE);
		break;
	case SO_REUSEPORT:
		sk->sk_reuseport = valbool;
		break;
	case SO_TYPE:
	case SO_PROTOCOL:
	case SO_DOMAIN:
	case SO_ERROR:
		ret = -ENOPROTOOPT;
		break;
	case SO_DONTROUTE:
		sock_valbool_flag(sk, SOCK_LOCALROUTE, valbool);
		sk_dst_reset(sk);
		break;
	case SO_BROADCAST:
		sock_valbool_flag(sk, SOCK_BROADCAST, valbool);
		break;
	case SO_SNDBUF:
		/* Don't error on this BSD doesn't and if you think
		 * about it this is right. Otherwise apps have to
		 * play 'guess the biggest size' games. RCVBUF/SNDBUF
		 * are treated in BSD as hints
		 */
		val = min_t(u32, val, sysctl_wmem_max);
set_sndbuf:
		/* Ensure val * 2 fits into an int, to prevent max_t()
		 * from treating it as a negative value.
		 */
		val = min_t(int, val, INT_MAX / 2);
		sk->sk_userlocks |= SOCK_SNDBUF_LOCK;
		sk->sk_sndbuf = max_t(int, val * 2, SOCK_MIN_SNDBUF);
		/* Wake up sending tasks if we upped the value. */
		sk->sk_write_space(sk);
		break;

	case SO_SNDBUFFORCE:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}

		/* No negative values (to prevent underflow, as val will be
		 * multiplied by 2).
		 */
		if (val < 0)
			val = 0;
		goto set_sndbuf;

	case SO_RCVBUF:
		/* Don't error on this BSD doesn't and if you think
		 * about it this is right. Otherwise apps have to
		 * play 'guess the biggest size' games. RCVBUF/SNDBUF
		 * are treated in BSD as hints
		 */
		val = min_t(u32, val, sysctl_rmem_max);
set_rcvbuf:
		/* Ensure val * 2 fits into an int, to prevent max_t()
		 * from treating it as a negative value.
		 */
		val = min_t(int, val, INT_MAX / 2);
		sk->sk_userlocks |= SOCK_RCVBUF_LOCK;
		/*
		 * We double it on the way in to account for
		 * "struct sk_buff" etc. overhead.   Applications
		 * assume that the SO_RCVBUF setting they make will
		 * allow that much actual data to be received on that
		 * socket.
		 *
		 * Applications are unaware that "struct sk_buff" and
		 * other overheads allocate from the receive buffer
		 * during socket buffer allocation.
		 *
		 * And after considering the possible alternatives,
		 * returning the value we actually used in getsockopt
		 * is the most desirable behavior.
		 */
		sk->sk_rcvbuf = max_t(int, val * 2, SOCK_MIN_RCVBUF);
		break;

	case SO_RCVBUFFORCE:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}

		/* No negative values (to prevent underflow, as val will be
		 * multiplied by 2).
		 */
		if (val < 0)
			val = 0;
		goto set_rcvbuf;

	case SO_KEEPALIVE:
		if (sk->sk_prot->keepalive)
			sk->sk_prot->keepalive(sk, valbool);
		sock_valbool_flag(sk, SOCK_KEEPOPEN, valbool);
		break;

	case SO_OOBINLINE:
		sock_valbool_flag(sk, SOCK_URGINLINE, valbool);
		break;

	case SO_NO_CHECK:
		sk->sk_no_check_tx = valbool;
		break;

	case SO_PRIORITY:
		if ((val >= 0 && val <= 6) ||
		    ns_capable(sock_net(sk)->user_ns, CAP_NET_ADMIN))
			sk->sk_priority = val;
		else
			ret = -EPERM;
		break;

	case SO_LINGER:
		if (optlen < sizeof(ling)) {
			ret = -EINVAL;	/* 1003.1g */
			break;
		}
		if (copy_from_user(&ling, optval, sizeof(ling))) {
			ret = -EFAULT;
			break;
		}
		if (!ling.l_onoff)
			sock_reset_flag(sk, SOCK_LINGER);
		else {
#if (BITS_PER_LONG == 32)
			if ((unsigned int)ling.l_linger >= MAX_SCHEDULE_TIMEOUT/HZ)
				sk->sk_lingertime = MAX_SCHEDULE_TIMEOUT;
			else
#endif
				sk->sk_lingertime = (unsigned int)ling.l_linger * HZ;
			sock_set_flag(sk, SOCK_LINGER);
		}
		break;

	case SO_BSDCOMPAT:
		sock_warn_obsolete_bsdism("setsockopt");
		break;

	case SO_PASSCRED:
		if (valbool)
			set_bit(SOCK_PASSCRED, &sock->flags);
		else
			clear_bit(SOCK_PASSCRED, &sock->flags);
		break;

	case SO_TIMESTAMP_OLD:
	case SO_TIMESTAMP_NEW:
	case SO_TIMESTAMPNS_OLD:
	case SO_TIMESTAMPNS_NEW:
		if (valbool)  {
			if (optname == SO_TIMESTAMP_NEW || optname == SO_TIMESTAMPNS_NEW)
				sock_set_flag(sk, SOCK_TSTAMP_NEW);
			else
				sock_reset_flag(sk, SOCK_TSTAMP_NEW);

			if (optname == SO_TIMESTAMP_OLD || optname == SO_TIMESTAMP_NEW)
				sock_reset_flag(sk, SOCK_RCVTSTAMPNS);
			else
				sock_set_flag(sk, SOCK_RCVTSTAMPNS);
			sock_set_flag(sk, SOCK_RCVTSTAMP);
			sock_enable_timestamp(sk, SOCK_TIMESTAMP);
		} else {
			sock_reset_flag(sk, SOCK_RCVTSTAMP);
			sock_reset_flag(sk, SOCK_RCVTSTAMPNS);
			sock_reset_flag(sk, SOCK_TSTAMP_NEW);
		}
		break;

	case SO_TIMESTAMPING_NEW:
		sock_set_flag(sk, SOCK_TSTAMP_NEW);
		/* fall through */
	case SO_TIMESTAMPING_OLD:
		if (val & ~SOF_TIMESTAMPING_MASK) {
			ret = -EINVAL;
			break;
		}

		if (val & SOF_TIMESTAMPING_OPT_ID &&
		    !(sk->sk_tsflags & SOF_TIMESTAMPING_OPT_ID)) {
			if (sk->sk_protocol == IPPROTO_TCP &&
			    sk->sk_type == SOCK_STREAM) {
				if ((1 << sk->sk_state) &
				    (TCPF_CLOSE | TCPF_LISTEN)) {
					ret = -EINVAL;
					break;
				}
				sk->sk_tskey = tcp_sk(sk)->snd_una;
			} else {
				sk->sk_tskey = 0;
			}
		}

		if (val & SOF_TIMESTAMPING_OPT_STATS &&
		    !(val & SOF_TIMESTAMPING_OPT_TSONLY)) {
			ret = -EINVAL;
			break;
		}

		sk->sk_tsflags = val;
		if (val & SOF_TIMESTAMPING_RX_SOFTWARE)
			sock_enable_timestamp(sk,
					      SOCK_TIMESTAMPING_RX_SOFTWARE);
		else {
			if (optname == SO_TIMESTAMPING_NEW)
				sock_reset_flag(sk, SOCK_TSTAMP_NEW);

			sock_disable_timestamp(sk,
					       (1UL << SOCK_TIMESTAMPING_RX_SOF
```

**External callers of `sock_setsockopt` at parent**

- `fs/cifs/connect.c:3856` — `	 * the default. sock_setsockopt not used because it expects`
- `include/net/sock.h:1614` — `int sock_setsockopt(struct socket *sock, int level, int op,`
- `net/compat.c:349` — `	return sock_setsockopt(sock, level, optname, (char __user *)kfprog,`
- `net/compat.c:360` — `	return sock_setsockopt(sock, level, optname, optval, optlen);`
- `net/socket.c:2080` — `			    sock_setsockopt(sock, level, optname, optval,`
- `net/socket.c:3658` — `		err = sock_setsockopt(sock, level, optname, uoptval, optlen);`

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
	mem[SK_MEMINFO_BACKLOG] = READ_ONCE(sk->sk_backlog.len);
	mem[SK_MEMINFO_DROPS] = atomic_read(&sk->sk_drops);
}
```

**External callers of `sk_get_meminfo` at parent**

- `include/net/sock.h:2526` — `void sk_get_meminfo(const struct sock *sk, u32 *meminfo);`
- `net/core/sock_diag.c:64` — `	sk_get_meminfo(sk, mem);`

## File: `net/ipv4/tcp.c`

- changed old-lines: [454, 1714]

### Function `tcp_init_sock` (L408–L458, 51 lines)

```c
void tcp_init_sock(struct sock *sk)
{
	struct inet_connection_sock *icsk = inet_csk(sk);
	struct tcp_sock *tp = tcp_sk(sk);

	tp->out_of_order_queue = RB_ROOT;
	sk->tcp_rtx_queue = RB_ROOT;
	tcp_init_xmit_timers(sk);
	INIT_LIST_HEAD(&tp->tsq_node);
	INIT_LIST_HEAD(&tp->tsorted_sent_queue);

	icsk->icsk_rto = TCP_TIMEOUT_INIT;
	tp->mdev_us = jiffies_to_usecs(TCP_TIMEOUT_INIT);
	minmax_reset(&tp->rtt_min, tcp_jiffies32, ~0U);

	/* So many TCP implementations out there (incorrectly) count the
	 * initial SYN frame in their delayed-ACK and congestion control
	 * algorithms that we must have the following bandaid to talk
	 * efficiently to them.  -DaveM
	 */
	tp->snd_cwnd = TCP_INIT_CWND;

	/* There's a bubble in the pipe until at least the first ACK. */
	tp->app_limited = ~0U;

	/* See draft-stevens-tcpca-spec-01 for discussion of the
	 * initialization of these values.
	 */
	tp->snd_ssthresh = TCP_INFINITE_SSTHRESH;
	tp->snd_cwnd_clamp = ~0;
	tp->mss_cache = TCP_MSS_DEFAULT;

	tp->reordering = sock_net(sk)->ipv4.sysctl_tcp_reordering;
	tcp_assign_congestion_control(sk);

	tp->tsoffset = 0;
	tp->rack.reo_wnd_steps = 1;

	sk->sk_state = TCP_CLOSE;

	sk->sk_write_space = sk_stream_write_space;
	sock_set_flag(sk, SOCK_USE_WRITE_QUEUE);

	icsk->icsk_sync_mss = tcp_sync_mss;

	sk->sk_sndbuf = sock_net(sk)->ipv4.sysctl_tcp_wmem[1];
	sk->sk_rcvbuf = sock_net(sk)->ipv4.sysctl_tcp_rmem[1];

	sk_sockets_allocated_inc(sk);
	sk->sk_route_forced_caps = NETIF_F_GSO;
}
```

**External callers of `tcp_init_sock` at parent**

- `include/net/tcp.h:390` — `void tcp_init_sock(struct sock *sk);`
- `net/ipv4/tcp_ipv4.c:2082` — `	tcp_init_sock(sk);`
- `net/ipv6/tcp_ipv6.c:1807` — `	tcp_init_sock(sk);`

### Function `tcp_set_rcvlowat` (L1695–L1718, 24 lines)

```c
int tcp_set_rcvlowat(struct sock *sk, int val)
{
	int cap;

	if (sk->sk_userlocks & SOCK_RCVBUF_LOCK)
		cap = sk->sk_rcvbuf >> 1;
	else
		cap = sock_net(sk)->ipv4.sysctl_tcp_rmem[2] >> 1;
	val = min(val, cap);
	WRITE_ONCE(sk->sk_rcvlowat, val ? : 1);

	/* Check if we need to signal EPOLLIN right now */
	tcp_data_ready(sk);

	if (sk->sk_userlocks & SOCK_RCVBUF_LOCK)
		return 0;

	val <<= 1;
	if (val > sk->sk_rcvbuf) {
		sk->sk_rcvbuf = val;
		tcp_sk(sk)->window_clamp = tcp_win_from_space(sk, val);
	}
	return 0;
}
```

**External callers of `tcp_set_rcvlowat` at parent**

- `include/net/tcp.h:406` — `int tcp_set_rcvlowat(struct sock *sk, int val);`
- `net/ipv4/af_inet.c:1015` — `	.set_rcvlowat	   = tcp_set_rcvlowat,`
- `net/ipv6/af_inet6.c:631` — `	.set_rcvlowat	   = tcp_set_rcvlowat,`
