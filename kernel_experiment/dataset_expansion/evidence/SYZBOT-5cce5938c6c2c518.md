# Evidence: SYZBOT-5cce5938c6c2c518  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in __ip_make_skb / __ip_make_skb
- mainline fix commit: `f866fbc842de5976e41ba874b76ce31710b634b5`
- candidate JSON: `A_SYZBOT-5cce5938c6c2c518.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "__ip_make_skb", "fn_b": "__ip_make_skb"}`

## Commit message

```
f866fbc842de5976e41ba874b76ce31710b634b5
ipv4: fix data-races around inet->inet_id

UDP sendmsg() is lockless, so ip_select_ident_segs()
can very well be run from multiple cpus [1]

Convert inet->inet_id to an atomic_t, but implement
a dedicated path for TCP, avoiding cost of a locked
instruction (atomic_add_return())

Note that this patch will cause a trivial merge conflict
because we added inet->flags in net-next tree.

v2: added missing change in
drivers/net/ethernet/chelsio/inline_crypto/chtls/chtls_cm.c
(David Ahern)

[1]

BUG: KCSAN: data-race in __ip_make_skb / __ip_make_skb

read-write to 0xffff888145af952a of 2 bytes by task 7803 on cpu 1:
ip_select_ident_segs include/net/ip.h:542 [inline]
ip_select_ident include/net/ip.h:556 [inline]
__ip_make_skb+0x844/0xc70 net/ipv4/ip_output.c:1446
ip_make_skb+0x233/0x2c0 net/ipv4/ip_output.c:1560
udp_sendmsg+0x1199/0x1250 net/ipv4/udp.c:1260
inet_sendmsg+0x63/0x80 net/ipv4/af_inet.c:830
sock_sendmsg_nosec net/socket.c:725 [inline]
sock_sendmsg net/socket.c:748 [inline]
____sys_sendmsg+0x37c/0x4d0 net/socket.c:2494
___sys_sendmsg net/socket.c:2548 [inline]
__sys_sendmmsg+0x269/0x500 net/socket.c:2634
__do_sys_sendmmsg net/socket.c:2663 [inline]
__se_sys_sendmmsg net/socket.c:2660 [inline]
__x64_sys_sendmmsg+0x57/0x60 net/socket.c:2660
do_syscall_x64 arch/x86/entry/common.c:50 [inline]
do_syscall_64+0x41/0xc0 arch/x86/entry/common.c:80
entry_SYSCALL_64_after_hwframe+0x63/0xcd

read to 0xffff888145af952a of 2 bytes by task 7804 on cpu 0:
ip_select_ident_segs include/net/ip.h:541 [inline]
ip_select_ident include/net/ip.h:556 [inline]
__ip_make_skb+0x817/0xc70 net/ipv4/ip_output.c:1446
ip_make_skb+0x233/0x2c0 net/ipv4/ip_output.c:1560
udp_sendmsg+0x1199/0x1250 net/ipv4/udp.c:1260
inet_sendmsg+0x63/0x80 net/ipv4/af_inet.c:830
sock_sendmsg_nosec net/socket.c:725 [inline]
sock_sendmsg net/socket.c:748 [inline]
____sys_sendmsg+0x37c/0x4d0 net/socket.c:2494
___sys_sendmsg net/socket.c:2548 [inline]
__sys_sendmmsg+0x269/0x500 net/socket.c:2634
__do_sys_sendmmsg net/socket.c:2663 [inline]
__se_sys_sendmmsg net/socket.c:2660 [inline]
__x64_sys_sendmmsg+0x57/0x60 net/socket.c:2660
do_syscall_x64 arch/x86/entry/common.c:50 [inline]
do_syscall_64+0x41/0xc0 arch/x86/entry/common.c:80
entry_SYSCALL_64_after_hwframe+0x63/0xcd

value changed: 0x184d -> 0x184e

Reported by Kernel Concurrency Sanitizer on:
CPU: 0 PID: 7804 Comm: syz-executor.1 Not tainted 6.5.0-rc6-syzkaller #0
Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 07/26/2023
==================================================================

Fixes: 23f57406b82d ("ipv4: avoid using shared IP generator for connected sockets")
Reported-by: syzbot <syzkaller@googlegroups.com>
Signed-off-by: Eric Dumazet <edumazet@google.com>
Reviewed-by: David Ahern <dsahern@kernel.org>
Signed-off-by: David S. Miller <davem@davemloft.net>
```

## CVE / bug description

```
KCSAN: data-race in __ip_make_skb / __ip_make_skb
```

## Full mainline patch

```diff
commit f866fbc842de5976e41ba874b76ce31710b634b5
Author: Eric Dumazet <edumazet@google.com>
Date:   Sat Aug 19 03:17:07 2023 +0000

    ipv4: fix data-races around inet->inet_id
    
    UDP sendmsg() is lockless, so ip_select_ident_segs()
    can very well be run from multiple cpus [1]
    
    Convert inet->inet_id to an atomic_t, but implement
    a dedicated path for TCP, avoiding cost of a locked
    instruction (atomic_add_return())
    
    Note that this patch will cause a trivial merge conflict
    because we added inet->flags in net-next tree.
    
    v2: added missing change in
    drivers/net/ethernet/chelsio/inline_crypto/chtls/chtls_cm.c
    (David Ahern)
    
    [1]
    
    BUG: KCSAN: data-race in __ip_make_skb / __ip_make_skb
    
    read-write to 0xffff888145af952a of 2 bytes by task 7803 on cpu 1:
    ip_select_ident_segs include/net/ip.h:542 [inline]
    ip_select_ident include/net/ip.h:556 [inline]
    __ip_make_skb+0x844/0xc70 net/ipv4/ip_output.c:1446
    ip_make_skb+0x233/0x2c0 net/ipv4/ip_output.c:1560
    udp_sendmsg+0x1199/0x1250 net/ipv4/udp.c:1260
    inet_sendmsg+0x63/0x80 net/ipv4/af_inet.c:830
    sock_sendmsg_nosec net/socket.c:725 [inline]
    sock_sendmsg net/socket.c:748 [inline]
    ____sys_sendmsg+0x37c/0x4d0 net/socket.c:2494
    ___sys_sendmsg net/socket.c:2548 [inline]
    __sys_sendmmsg+0x269/0x500 net/socket.c:2634
    __do_sys_sendmmsg net/socket.c:2663 [inline]
    __se_sys_sendmmsg net/socket.c:2660 [inline]
    __x64_sys_sendmmsg+0x57/0x60 net/socket.c:2660
    do_syscall_x64 arch/x86/entry/common.c:50 [inline]
    do_syscall_64+0x41/0xc0 arch/x86/entry/common.c:80
    entry_SYSCALL_64_after_hwframe+0x63/0xcd
    
    read to 0xffff888145af952a of 2 bytes by task 7804 on cpu 0:
    ip_select_ident_segs include/net/ip.h:541 [inline]
    ip_select_ident include/net/ip.h:556 [inline]
    __ip_make_skb+0x817/0xc70 net/ipv4/ip_output.c:1446
    ip_make_skb+0x233/0x2c0 net/ipv4/ip_output.c:1560
    udp_sendmsg+0x1199/0x1250 net/ipv4/udp.c:1260
    inet_sendmsg+0x63/0x80 net/ipv4/af_inet.c:830
    sock_sendmsg_nosec net/socket.c:725 [inline]
    sock_sendmsg net/socket.c:748 [inline]
    ____sys_sendmsg+0x37c/0x4d0 net/socket.c:2494
    ___sys_sendmsg net/socket.c:2548 [inline]
    __sys_sendmmsg+0x269/0x500 net/socket.c:2634
    __do_sys_sendmmsg net/socket.c:2663 [inline]
    __se_sys_sendmmsg net/socket.c:2660 [inline]
    __x64_sys_sendmmsg+0x57/0x60 net/socket.c:2660
    do_syscall_x64 arch/x86/entry/common.c:50 [inline]
    do_syscall_64+0x41/0xc0 arch/x86/entry/common.c:80
    entry_SYSCALL_64_after_hwframe+0x63/0xcd
    
    value changed: 0x184d -> 0x184e
    
    Reported by Kernel Concurrency Sanitizer on:
    CPU: 0 PID: 7804 Comm: syz-executor.1 Not tainted 6.5.0-rc6-syzkaller #0
    Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 07/26/2023
    ==================================================================
    
    Fixes: 23f57406b82d ("ipv4: avoid using shared IP generator for connected sockets")
    Reported-by: syzbot <syzkaller@googlegroups.com>
    Signed-off-by: Eric Dumazet <edumazet@google.com>
    Reviewed-by: David Ahern <dsahern@kernel.org>
    Signed-off-by: David S. Miller <davem@davemloft.net>

diff --git a/drivers/net/ethernet/chelsio/inline_crypto/chtls/chtls_cm.c b/drivers/net/ethernet/chelsio/inline_crypto/chtls/chtls_cm.c
index c2e7037c7ba1..7750702900fa 100644
--- a/drivers/net/ethernet/chelsio/inline_crypto/chtls/chtls_cm.c
+++ b/drivers/net/ethernet/chelsio/inline_crypto/chtls/chtls_cm.c
@@ -1466,7 +1466,7 @@ static void make_established(struct sock *sk, u32 snd_isn, unsigned int opt)
 	tp->write_seq = snd_isn;
 	tp->snd_nxt = snd_isn;
 	tp->snd_una = snd_isn;
-	inet_sk(sk)->inet_id = get_random_u16();
+	atomic_set(&inet_sk(sk)->inet_id, get_random_u16());
 	assign_rxopt(sk, opt);
 
 	if (tp->rcv_wnd > (RCV_BUFSIZ_M << 10))
diff --git a/include/net/inet_sock.h b/include/net/inet_sock.h
index 0bb32bfc6183..491ceb7ebe5d 100644
--- a/include/net/inet_sock.h
+++ b/include/net/inet_sock.h
@@ -222,8 +222,8 @@ struct inet_sock {
 	__s16			uc_ttl;
 	__u16			cmsg_flags;
 	struct ip_options_rcu __rcu	*inet_opt;
+	atomic_t		inet_id;
 	__be16			inet_sport;
-	__u16			inet_id;
 
 	__u8			tos;
 	__u8			min_ttl;
diff --git a/include/net/ip.h b/include/net/ip.h
index 332521170d9b..19adacd5ece0 100644
--- a/include/net/ip.h
+++ b/include/net/ip.h
@@ -538,8 +538,19 @@ static inline void ip_select_ident_segs(struct net *net, struct sk_buff *skb,
 	 * generator as much as we can.
 	 */
 	if (sk && inet_sk(sk)->inet_daddr) {
-		iph->id = htons(inet_sk(sk)->inet_id);
-		inet_sk(sk)->inet_id += segs;
+		int val;
+
+		/* avoid atomic operations for TCP,
+		 * as we hold socket lock at this point.
+		 */
+		if (sk_is_tcp(sk)) {
+			sock_owned_by_me(sk);
+			val = atomic_read(&inet_sk(sk)->inet_id);
+			atomic_set(&inet_sk(sk)->inet_id, val + segs);
+		} else {
+			val = atomic_add_return(segs, &inet_sk(sk)->inet_id);
+		}
+		iph->id = htons(val);
 		return;
 	}
 	if ((iph->frag_off & htons(IP_DF)) && !skb->ignore_df) {
diff --git a/net/dccp/ipv4.c b/net/dccp/ipv4.c
index fa8079303cb0..a545ad71201c 100644
--- a/net/dccp/ipv4.c
+++ b/net/dccp/ipv4.c
@@ -130,7 +130,7 @@ int dccp_v4_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len)
 						    inet->inet_daddr,
 						    inet->inet_sport,
 						    inet->inet_dport);
-	inet->inet_id = get_random_u16();
+	atomic_set(&inet->inet_id, get_random_u16());
 
 	err = dccp_connect(sk);
 	rt = NULL;
@@ -432,7 +432,7 @@ struct sock *dccp_v4_request_recv_sock(const struct sock *sk,
 	RCU_INIT_POINTER(newinet->inet_opt, rcu_dereference(ireq->ireq_opt));
 	newinet->mc_index  = inet_iif(skb);
 	newinet->mc_ttl	   = ip_hdr(skb)->ttl;
-	newinet->inet_id   = get_random_u16();
+	atomic_set(&newinet->inet_id, get_random_u16());
 
 	if (dst == NULL && (dst = inet_csk_route_child_sock(sk, newsk, req)) == NULL)
 		goto put_and_exit;
diff --git a/net/ipv4/af_inet.c b/net/ipv4/af_inet.c
index 9b2ca2fcc5a1..02736b83c303 100644
--- a/net/ipv4/af_inet.c
+++ b/net/ipv4/af_inet.c
@@ -340,7 +340,7 @@ static int inet_create(struct net *net, struct socket *sock, int protocol,
 	else
 		inet->pmtudisc = IP_PMTUDISC_WANT;
 
-	inet->inet_id = 0;
+	atomic_set(&inet->inet_id, 0);
 
 	sock_init_data(sock, sk);
 
diff --git a/net/ipv4/datagram.c b/net/ipv4/datagram.c
index 4d1af0cd7d99..cb5dbee9e018 100644
--- a/net/ipv4/datagram.c
+++ b/net/ipv4/datagram.c
@@ -73,7 +73,7 @@ int __ip4_datagram_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len
 	reuseport_has_conns_set(sk);
 	sk->sk_state = TCP_ESTABLISHED;
 	sk_set_txhash(sk);
-	inet->inet_id = get_random_u16();
+	atomic_set(&inet->inet_id, get_random_u16());
 
 	sk_dst_set(sk, &rt->dst);
 	err = 0;
diff --git a/net/ipv4/tcp_ipv4.c b/net/ipv4/tcp_ipv4.c
index a59cc4b83861..2dbdc26da86e 100644
--- a/net/ipv4/tcp_ipv4.c
+++ b/net/ipv4/tcp_ipv4.c
@@ -312,7 +312,7 @@ int tcp_v4_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len)
 					     inet->inet_daddr));
 	}
 
-	inet->inet_id = get_random_u16();
+	atomic_set(&inet->inet_id, get_random_u16());
 
 	if (tcp_fastopen_defer_connect(sk, &err))
 		return err;
@@ -1596,7 +1596,7 @@ struct sock *tcp_v4_syn_recv_sock(const struct sock *sk, struct sk_buff *skb,
 	inet_csk(newsk)->icsk_ext_hdr_len = 0;
 	if (inet_opt)
 		inet_csk(newsk)->icsk_ext_hdr_len = inet_opt->opt.optlen;
-	newinet->inet_id = get_random_u16();
+	atomic_set(&newinet->inet_id, get_random_u16());
 
 	/* Set ToS of the new socket based upon the value of incoming SYN.
 	 * ECT bits are set later in tcp_init_transfer().
diff --git a/net/sctp/socket.c b/net/sctp/socket.c
index 6da738f60f4b..76f1bce49a8e 100644
--- a/net/sctp/socket.c
+++ b/net/sctp/socket.c
@@ -9479,7 +9479,7 @@ void sctp_copy_sock(struct sock *newsk, struct sock *sk,
 	newinet->inet_rcv_saddr = inet->inet_rcv_saddr;
 	newinet->inet_dport = htons(asoc->peer.port);
 	newinet->pmtudisc = inet->pmtudisc;
-	newinet->inet_id = get_random_u16();
+	atomic_set(&newinet->inet_id, get_random_u16());
 
 	newinet->uc_ttl = inet->uc_ttl;
 	newinet->mc_loop = 1;

```

## File: `drivers/net/ethernet/chelsio/inline_crypto/chtls/chtls_cm.c`

- changed old-lines: [1469]

### Function `make_established` (L1461–L1477, 17 lines)

```c
static void make_established(struct sock *sk, u32 snd_isn, unsigned int opt)
{
	struct tcp_sock *tp = tcp_sk(sk);

	tp->pushed_seq = snd_isn;
	tp->write_seq = snd_isn;
	tp->snd_nxt = snd_isn;
	tp->snd_una = snd_isn;
	inet_sk(sk)->inet_id = get_random_u16();
	assign_rxopt(sk, opt);

	if (tp->rcv_wnd > (RCV_BUFSIZ_M << 10))
		tp->rcv_wup -= tp->rcv_wnd - (RCV_BUFSIZ_M << 10);

	smp_mb();
	tcp_set_state(sk, TCP_ESTABLISHED);
}
```

**External callers of `make_established` at parent**

- `tools/testing/selftests/bpf/prog_tests/bpf_iter_setsockopt.c:51` — `static int *make_established(int listen_fd, unsigned int nr_est,`
- `tools/testing/selftests/bpf/prog_tests/bpf_iter_setsockopt.c:125` — `	est_fds = make_established(listen_fd, nr_est, &accepted_fds);`

## File: `include/net/inet_sock.h`

- changed old-lines: [226]

## File: `include/net/ip.h`

- changed old-lines: [541, 542]

### Function `__ip_select_ident` (L530–L551, 22 lines)

```c
void __ip_select_ident(struct net *net, struct iphdr *iph, int segs);

static inline void ip_select_ident_segs(struct net *net, struct sk_buff *skb,
					struct sock *sk, int segs)
{
	struct iphdr *iph = ip_hdr(skb);

	/* We had many attacks based on IPID, use the private
	 * generator as much as we can.
	 */
	if (sk && inet_sk(sk)->inet_daddr) {
		iph->id = htons(inet_sk(sk)->inet_id);
		inet_sk(sk)->inet_id += segs;
		return;
	}
	if ((iph->frag_off & htons(IP_DF)) && !skb->ignore_df) {
		iph->id = 0;
	} else {
		/* Unfortunately we need the big hammer to get a suitable IPID */
		__ip_select_ident(net, iph, segs);
	}
}
```

**External callers of `__ip_select_ident` at parent**

- `drivers/infiniband/sw/rxe/rxe_net.c:252` — `	__ip_select_ident(dev_net(dst->dev), iph,`
- `net/ipv4/ip_output.c:179` — `			__ip_select_ident(net, iph, 1);`
- `net/ipv4/ip_tunnel_core.c:80` — `	__ip_select_ident(net, iph, skb_shinfo(skb)->gso_segs ?: 1);`
- `net/ipv4/route.c:483` — `void __ip_select_ident(struct net *net, struct iphdr *iph, int segs)`
- `net/ipv4/route.c:499` — `EXPORT_SYMBOL(__ip_select_ident);`

## File: `net/dccp/ipv4.c`

- changed old-lines: [133, 435]

### Function `dccp_v4_connect` (L45–L151, 107 lines)

```c
int dccp_v4_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len)
{
	const struct sockaddr_in *usin = (struct sockaddr_in *)uaddr;
	struct inet_sock *inet = inet_sk(sk);
	struct dccp_sock *dp = dccp_sk(sk);
	__be16 orig_sport, orig_dport;
	__be32 daddr, nexthop;
	struct flowi4 *fl4;
	struct rtable *rt;
	int err;
	struct ip_options_rcu *inet_opt;

	dp->dccps_role = DCCP_ROLE_CLIENT;

	if (addr_len < sizeof(struct sockaddr_in))
		return -EINVAL;

	if (usin->sin_family != AF_INET)
		return -EAFNOSUPPORT;

	nexthop = daddr = usin->sin_addr.s_addr;

	inet_opt = rcu_dereference_protected(inet->inet_opt,
					     lockdep_sock_is_held(sk));
	if (inet_opt != NULL && inet_opt->opt.srr) {
		if (daddr == 0)
			return -EINVAL;
		nexthop = inet_opt->opt.faddr;
	}

	orig_sport = inet->inet_sport;
	orig_dport = usin->sin_port;
	fl4 = &inet->cork.fl.u.ip4;
	rt = ip_route_connect(fl4, nexthop, inet->inet_saddr,
			      sk->sk_bound_dev_if, IPPROTO_DCCP, orig_sport,
			      orig_dport, sk);
	if (IS_ERR(rt))
		return PTR_ERR(rt);

	if (rt->rt_flags & (RTCF_MULTICAST | RTCF_BROADCAST)) {
		ip_rt_put(rt);
		return -ENETUNREACH;
	}

	if (inet_opt == NULL || !inet_opt->opt.srr)
		daddr = fl4->daddr;

	if (inet->inet_saddr == 0) {
		err = inet_bhash2_update_saddr(sk,  &fl4->saddr, AF_INET);
		if (err) {
			ip_rt_put(rt);
			return err;
		}
	} else {
		sk_rcv_saddr_set(sk, inet->inet_saddr);
	}

	inet->inet_dport = usin->sin_port;
	sk_daddr_set(sk, daddr);

	inet_csk(sk)->icsk_ext_hdr_len = 0;
	if (inet_opt)
		inet_csk(sk)->icsk_ext_hdr_len = inet_opt->opt.optlen;
	/*
	 * Socket identity is still unknown (sport may be zero).
	 * However we set state to DCCP_REQUESTING and not releasing socket
	 * lock select source port, enter ourselves into the hash tables and
	 * complete initialization after this.
	 */
	dccp_set_state(sk, DCCP_REQUESTING);
	err = inet_hash_connect(&dccp_death_row, sk);
	if (err != 0)
		goto failure;

	rt = ip_route_newports(fl4, rt, orig_sport, orig_dport,
			       inet->inet_sport, inet->inet_dport, sk);
	if (IS_ERR(rt)) {
		err = PTR_ERR(rt);
		rt = NULL;
		goto failure;
	}
	/* OK, now commit destination to socket.  */
	sk_setup_caps(sk, &rt->dst);

	dp->dccps_iss = secure_dccp_sequence_number(inet->inet_saddr,
						    inet->inet_daddr,
						    inet->inet_sport,
						    inet->inet_dport);
	inet->inet_id = get_random_u16();

	err = dccp_connect(sk);
	rt = NULL;
	if (err != 0)
		goto failure;
out:
	return err;
failure:
	/*
	 * This unhashes the socket and releases the local port, if necessary.
	 */
	dccp_set_state(sk, DCCP_CLOSED);
	inet_bhash2_reset_saddr(sk);
	ip_rt_put(rt);
	sk->sk_route_caps = 0;
	inet->inet_dport = 0;
	goto out;
}
```

**External callers of `dccp_v4_connect` at parent**

- `net/dccp/dccp.h:303` — `int dccp_v4_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len);`
- `net/dccp/ipv6.c:904` — `		err = dccp_v4_connect(sk, (struct sockaddr *)&sin, sizeof(sin));`

### Function `dccp_v4_request_recv_sock` (L409–L465, 57 lines)

```c
struct sock *dccp_v4_request_recv_sock(const struct sock *sk,
				       struct sk_buff *skb,
				       struct request_sock *req,
				       struct dst_entry *dst,
				       struct request_sock *req_unhash,
				       bool *own_req)
{
	struct inet_request_sock *ireq;
	struct inet_sock *newinet;
	struct sock *newsk;

	if (sk_acceptq_is_full(sk))
		goto exit_overflow;

	newsk = dccp_create_openreq_child(sk, req, skb);
	if (newsk == NULL)
		goto exit_nonewsk;

	newinet		   = inet_sk(newsk);
	ireq		   = inet_rsk(req);
	sk_daddr_set(newsk, ireq->ir_rmt_addr);
	sk_rcv_saddr_set(newsk, ireq->ir_loc_addr);
	newinet->inet_saddr	= ireq->ir_loc_addr;
	RCU_INIT_POINTER(newinet->inet_opt, rcu_dereference(ireq->ireq_opt));
	newinet->mc_index  = inet_iif(skb);
	newinet->mc_ttl	   = ip_hdr(skb)->ttl;
	newinet->inet_id   = get_random_u16();

	if (dst == NULL && (dst = inet_csk_route_child_sock(sk, newsk, req)) == NULL)
		goto put_and_exit;

	sk_setup_caps(newsk, dst);

	dccp_sync_mss(newsk, dst_mtu(dst));

	if (__inet_inherit_port(sk, newsk) < 0)
		goto put_and_exit;
	*own_req = inet_ehash_nolisten(newsk, req_to_sk(req_unhash), NULL);
	if (*own_req)
		ireq->ireq_opt = NULL;
	else
		newinet->inet_opt = NULL;
	return newsk;

exit_overflow:
	__NET_INC_STATS(sock_net(sk), LINUX_MIB_LISTENOVERFLOWS);
exit_nonewsk:
	dst_release(dst);
exit:
	__NET_INC_STATS(sock_net(sk), LINUX_MIB_LISTENDROPS);
	return NULL;
put_and_exit:
	newinet->inet_opt = NULL;
	inet_csk_prepare_forced_close(newsk);
	dccp_done(newsk);
	goto exit;
}
```

**External callers of `dccp_v4_request_recv_sock` at parent**

- `net/dccp/dccp.h:266` — `struct sock *dccp_v4_request_recv_sock(const struct sock *sk, struct sk_buff *skb,`
- `net/dccp/ipv6.c:431` — `		newsk = dccp_v4_request_recv_sock(sk, skb, req, dst,`

## File: `net/ipv4/af_inet.c`

- changed old-lines: [343]

### Function `inet_create` (L245–L395, 151 lines)

```c
static int inet_create(struct net *net, struct socket *sock, int protocol,
		       int kern)
{
	struct sock *sk;
	struct inet_protosw *answer;
	struct inet_sock *inet;
	struct proto *answer_prot;
	unsigned char answer_flags;
	int try_loading_module = 0;
	int err;

	if (protocol < 0 || protocol >= IPPROTO_MAX)
		return -EINVAL;

	sock->state = SS_UNCONNECTED;

	/* Look for the requested type/protocol pair. */
lookup_protocol:
	err = -ESOCKTNOSUPPORT;
	rcu_read_lock();
	list_for_each_entry_rcu(answer, &inetsw[sock->type], list) {

		err = 0;
		/* Check the non-wild match. */
		if (protocol == answer->protocol) {
			if (protocol != IPPROTO_IP)
				break;
		} else {
			/* Check for the two wild cases. */
			if (IPPROTO_IP == protocol) {
				protocol = answer->protocol;
				break;
			}
			if (IPPROTO_IP == answer->protocol)
				break;
		}
		err = -EPROTONOSUPPORT;
	}

	if (unlikely(err)) {
		if (try_loading_module < 2) {
			rcu_read_unlock();
			/*
			 * Be more specific, e.g. net-pf-2-proto-132-type-1
			 * (net-pf-PF_INET-proto-IPPROTO_SCTP-type-SOCK_STREAM)
			 */
			if (++try_loading_module == 1)
				request_module("net-pf-%d-proto-%d-type-%d",
					       PF_INET, protocol, sock->type);
			/*
			 * Fall back to generic, e.g. net-pf-2-proto-132
			 * (net-pf-PF_INET-proto-IPPROTO_SCTP)
			 */
			else
				request_module("net-pf-%d-proto-%d",
					       PF_INET, protocol);
			goto lookup_protocol;
		} else
			goto out_rcu_unlock;
	}

	err = -EPERM;
	if (sock->type == SOCK_RAW && !kern &&
	    !ns_capable(net->user_ns, CAP_NET_RAW))
		goto out_rcu_unlock;

	sock->ops = answer->ops;
	answer_prot = answer->prot;
	answer_flags = answer->flags;
	rcu_read_unlock();

	WARN_ON(!answer_prot->slab);

	err = -ENOMEM;
	sk = sk_alloc(net, PF_INET, GFP_KERNEL, answer_prot, kern);
	if (!sk)
		goto out;

	err = 0;
	if (INET_PROTOSW_REUSE & answer_flags)
		sk->sk_reuse = SK_CAN_REUSE;

	inet = inet_sk(sk);
	inet->is_icsk = (INET_PROTOSW_ICSK & answer_flags) != 0;

	inet->nodefrag = 0;

	if (SOCK_RAW == sock->type) {
		inet->inet_num = protocol;
		if (IPPROTO_RAW == protocol)
			inet->hdrincl = 1;
	}

	if (READ_ONCE(net->ipv4.sysctl_ip_no_pmtu_disc))
		inet->pmtudisc = IP_PMTUDISC_DONT;
	else
		inet->pmtudisc = IP_PMTUDISC_WANT;

	inet->inet_id = 0;

	sock_init_data(sock, sk);

	sk->sk_destruct	   = inet_sock_destruct;
	sk->sk_protocol	   = protocol;
	sk->sk_backlog_rcv = sk->sk_prot->backlog_rcv;
	sk->sk_txrehash = READ_ONCE(net->core.sysctl_txrehash);

	inet->uc_ttl	= -1;
	inet->mc_loop	= 1;
	inet->mc_ttl	= 1;
	inet->mc_all	= 1;
	inet->mc_index	= 0;
	inet->mc_list	= NULL;
	inet->rcv_tos	= 0;

	if (inet->inet_num) {
		/* It assumes that any protocol which allows
		 * the user to assign a number at socket
		 * creation time automatically
		 * shares.
		 */
		inet->inet_sport = htons(inet->inet_num);
		/* Add to protocol hash chains. */
		err = sk->sk_prot->hash(sk);
		if (err) {
			sk_common_release(sk);
			goto out;
		}
	}

	if (sk->sk_prot->init) {
		err = sk->sk_prot->init(sk);
		if (err) {
			sk_common_release(sk);
			goto out;
		}
	}

	if (!kern) {
		err = BPF_CGROUP_RUN_PROG_INET_SOCK(sk);
		if (err) {
			sk_common_release(sk);
			goto out;
		}
	}
out:
	return err;
out_rcu_unlock:
	rcu_read_unlock();
	goto out;
}
```

**External callers of `inet_create` at parent**

- `net/core/sock.c:51` — ` *		Anonymous	:	inet_create tidied up (sk->reuse setting)`

## File: `net/ipv4/datagram.c`

- changed old-lines: [76]

### Function `__ip4_datagram_connect` (L19–L82, 64 lines)

```c
int __ip4_datagram_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len)
{
	struct inet_sock *inet = inet_sk(sk);
	struct sockaddr_in *usin = (struct sockaddr_in *) uaddr;
	struct flowi4 *fl4;
	struct rtable *rt;
	__be32 saddr;
	int oif;
	int err;


	if (addr_len < sizeof(*usin))
		return -EINVAL;

	if (usin->sin_family != AF_INET)
		return -EAFNOSUPPORT;

	sk_dst_reset(sk);

	oif = sk->sk_bound_dev_if;
	saddr = inet->inet_saddr;
	if (ipv4_is_multicast(usin->sin_addr.s_addr)) {
		if (!oif || netif_index_is_l3_master(sock_net(sk), oif))
			oif = inet->mc_index;
		if (!saddr)
			saddr = inet->mc_addr;
	} else if (!oif) {
		oif = inet->uc_index;
	}
	fl4 = &inet->cork.fl.u.ip4;
	rt = ip_route_connect(fl4, usin->sin_addr.s_addr, saddr, oif,
			      sk->sk_protocol, inet->inet_sport,
			      usin->sin_port, sk);
	if (IS_ERR(rt)) {
		err = PTR_ERR(rt);
		if (err == -ENETUNREACH)
			IP_INC_STATS(sock_net(sk), IPSTATS_MIB_OUTNOROUTES);
		goto out;
	}

	if ((rt->rt_flags & RTCF_BROADCAST) && !sock_flag(sk, SOCK_BROADCAST)) {
		ip_rt_put(rt);
		err = -EACCES;
		goto out;
	}
	if (!inet->inet_saddr)
		inet->inet_saddr = fl4->saddr;	/* Update source address */
	if (!inet->inet_rcv_saddr) {
		inet->inet_rcv_saddr = fl4->saddr;
		if (sk->sk_prot->rehash)
			sk->sk_prot->rehash(sk);
	}
	inet->inet_daddr = fl4->daddr;
	inet->inet_dport = usin->sin_port;
	reuseport_has_conns_set(sk);
	sk->sk_state = TCP_ESTABLISHED;
	sk_set_txhash(sk);
	inet->inet_id = get_random_u16();

	sk_dst_set(sk, &rt->dst);
	err = 0;
out:
	return err;
}
```

**External callers of `__ip4_datagram_connect` at parent**

- `include/net/ip.h:264` — `int __ip4_datagram_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len);`
- `net/ipv4/ping.c:297` — `	/* This check is replicated from __ip4_datagram_connect() and`
- `net/ipv4/udp.c:1919` — `	/* This check is replicated from __ip4_datagram_connect() and`
- `net/ipv6/datagram.c:155` — `		err = __ip4_datagram_connect(sk, uaddr, addr_len);`
- `net/ipv6/datagram.c:194` — `		err = __ip4_datagram_connect(sk,`
- `net/l2tp/l2tp_ip.c:326` — `	rc = __ip4_datagram_connect(sk, uaddr, addr_len);`
