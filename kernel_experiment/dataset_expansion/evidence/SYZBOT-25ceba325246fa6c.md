# Evidence: SYZBOT-25ceba325246fa6c  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in inet6_recvmsg / ipv6_setsockopt
- mainline fix commit: `086d49058cd8471046ae9927524708820f5fd1c7`
- candidate JSON: `A_SYZBOT-25ceba325246fa6c.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "inet6_recvmsg", "fn_b": "ipv6_setsockopt"}`

## Commit message

```
086d49058cd8471046ae9927524708820f5fd1c7
ipv6: annotate some data-races around sk->sk_prot

IPv6 has this hack changing sk->sk_prot when an IPv6 socket
is 'converted' to an IPv4 one with IPV6_ADDRFORM option.

This operation is only performed for TCP and UDP, knowing
their 'struct proto' for the two network families are populated
in the same way, and can not disappear while a reader
might use and dereference sk->sk_prot.

If we think about it all reads of sk->sk_prot while
either socket lock or RTNL is not acquired should be using READ_ONCE().

Also note that other layers like MPTCP, XFRM, CHELSIO_TLS also
write over sk->sk_prot.

BUG: KCSAN: data-race in inet6_recvmsg / ipv6_setsockopt

write to 0xffff8881386f7aa8 of 8 bytes by task 26932 on cpu 0:
 do_ipv6_setsockopt net/ipv6/ipv6_sockglue.c:492 [inline]
 ipv6_setsockopt+0x3758/0x3910 net/ipv6/ipv6_sockglue.c:1019
 udpv6_setsockopt+0x85/0x90 net/ipv6/udp.c:1649
 sock_common_setsockopt+0x5d/0x70 net/core/sock.c:3489
 __sys_setsockopt+0x209/0x2a0 net/socket.c:2180
 __do_sys_setsockopt net/socket.c:2191 [inline]
 __se_sys_setsockopt net/socket.c:2188 [inline]
 __x64_sys_setsockopt+0x62/0x70 net/socket.c:2188
 do_syscall_x64 arch/x86/entry/common.c:50 [inline]
 do_syscall_64+0x44/0xd0 arch/x86/entry/common.c:80
 entry_SYSCALL_64_after_hwframe+0x44/0xae

read to 0xffff8881386f7aa8 of 8 bytes by task 26911 on cpu 1:
 inet6_recvmsg+0x7a/0x210 net/ipv6/af_inet6.c:659
 ____sys_recvmsg+0x16c/0x320
 ___sys_recvmsg net/socket.c:2674 [inline]
 do_recvmmsg+0x3f5/0xae0 net/socket.c:2768
 __sys_recvmmsg net/socket.c:2847 [inline]
 __do_sys_recvmmsg net/socket.c:2870 [inline]
 __se_sys_recvmmsg net/socket.c:2863 [inline]
 __x64_sys_recvmmsg+0xde/0x160 net/socket.c:2863
 do_syscall_x64 arch/x86/entry/common.c:50 [inline]
 do_syscall_64+0x44/0xd0 arch/x86/entry/common.c:80
 entry_SYSCALL_64_after_hwframe+0x44/0xae

value changed: 0xffffffff85e0e980 -> 0xffffffff85e01580

Reported by Kernel Concurrency Sanitizer on:
CPU: 1 PID: 26911 Comm: syz-executor.3 Not tainted 5.17.0-rc2-syzkaller-00316-g0457e5153e0e-dirty #0
Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 01/01/2011

Reported-by: syzbot <syzkaller@googlegroups.com>
Signed-off-by: Eric Dumazet <edumazet@google.com>
Signed-off-by: David S. Miller <davem@davemloft.net>
```

## CVE / bug description

```
KCSAN: data-race in inet6_recvmsg / ipv6_setsockopt
```

## Full mainline patch

```diff
commit 086d49058cd8471046ae9927524708820f5fd1c7
Author: Eric Dumazet <edumazet@google.com>
Date:   Thu Feb 17 15:48:41 2022 -0800

    ipv6: annotate some data-races around sk->sk_prot
    
    IPv6 has this hack changing sk->sk_prot when an IPv6 socket
    is 'converted' to an IPv4 one with IPV6_ADDRFORM option.
    
    This operation is only performed for TCP and UDP, knowing
    their 'struct proto' for the two network families are populated
    in the same way, and can not disappear while a reader
    might use and dereference sk->sk_prot.
    
    If we think about it all reads of sk->sk_prot while
    either socket lock or RTNL is not acquired should be using READ_ONCE().
    
    Also note that other layers like MPTCP, XFRM, CHELSIO_TLS also
    write over sk->sk_prot.
    
    BUG: KCSAN: data-race in inet6_recvmsg / ipv6_setsockopt
    
    write to 0xffff8881386f7aa8 of 8 bytes by task 26932 on cpu 0:
     do_ipv6_setsockopt net/ipv6/ipv6_sockglue.c:492 [inline]
     ipv6_setsockopt+0x3758/0x3910 net/ipv6/ipv6_sockglue.c:1019
     udpv6_setsockopt+0x85/0x90 net/ipv6/udp.c:1649
     sock_common_setsockopt+0x5d/0x70 net/core/sock.c:3489
     __sys_setsockopt+0x209/0x2a0 net/socket.c:2180
     __do_sys_setsockopt net/socket.c:2191 [inline]
     __se_sys_setsockopt net/socket.c:2188 [inline]
     __x64_sys_setsockopt+0x62/0x70 net/socket.c:2188
     do_syscall_x64 arch/x86/entry/common.c:50 [inline]
     do_syscall_64+0x44/0xd0 arch/x86/entry/common.c:80
     entry_SYSCALL_64_after_hwframe+0x44/0xae
    
    read to 0xffff8881386f7aa8 of 8 bytes by task 26911 on cpu 1:
     inet6_recvmsg+0x7a/0x210 net/ipv6/af_inet6.c:659
     ____sys_recvmsg+0x16c/0x320
     ___sys_recvmsg net/socket.c:2674 [inline]
     do_recvmmsg+0x3f5/0xae0 net/socket.c:2768
     __sys_recvmmsg net/socket.c:2847 [inline]
     __do_sys_recvmmsg net/socket.c:2870 [inline]
     __se_sys_recvmmsg net/socket.c:2863 [inline]
     __x64_sys_recvmmsg+0xde/0x160 net/socket.c:2863
     do_syscall_x64 arch/x86/entry/common.c:50 [inline]
     do_syscall_64+0x44/0xd0 arch/x86/entry/common.c:80
     entry_SYSCALL_64_after_hwframe+0x44/0xae
    
    value changed: 0xffffffff85e0e980 -> 0xffffffff85e01580
    
    Reported by Kernel Concurrency Sanitizer on:
    CPU: 1 PID: 26911 Comm: syz-executor.3 Not tainted 5.17.0-rc2-syzkaller-00316-g0457e5153e0e-dirty #0
    Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 01/01/2011
    
    Reported-by: syzbot <syzkaller@googlegroups.com>
    Signed-off-by: Eric Dumazet <edumazet@google.com>
    Signed-off-by: David S. Miller <davem@davemloft.net>

diff --git a/net/ipv6/af_inet6.c b/net/ipv6/af_inet6.c
index 8fe7900f1949..7d7b7523d126 100644
--- a/net/ipv6/af_inet6.c
+++ b/net/ipv6/af_inet6.c
@@ -441,11 +441,14 @@ int inet6_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
 {
 	struct sock *sk = sock->sk;
 	u32 flags = BIND_WITH_LOCK;
+	const struct proto *prot;
 	int err = 0;
 
+	/* IPV6_ADDRFORM can change sk->sk_prot under us. */
+	prot = READ_ONCE(sk->sk_prot);
 	/* If the socket has its own bind function then use it. */
-	if (sk->sk_prot->bind)
-		return sk->sk_prot->bind(sk, uaddr, addr_len);
+	if (prot->bind)
+		return prot->bind(sk, uaddr, addr_len);
 
 	if (addr_len < SIN6_LEN_RFC2133)
 		return -EINVAL;
@@ -555,6 +558,7 @@ int inet6_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
 	void __user *argp = (void __user *)arg;
 	struct sock *sk = sock->sk;
 	struct net *net = sock_net(sk);
+	const struct proto *prot;
 
 	switch (cmd) {
 	case SIOCADDRT:
@@ -572,9 +576,11 @@ int inet6_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
 	case SIOCSIFDSTADDR:
 		return addrconf_set_dstaddr(net, argp);
 	default:
-		if (!sk->sk_prot->ioctl)
+		/* IPV6_ADDRFORM can change sk->sk_prot under us. */
+		prot = READ_ONCE(sk->sk_prot);
+		if (!prot->ioctl)
 			return -ENOIOCTLCMD;
-		return sk->sk_prot->ioctl(sk, cmd, arg);
+		return prot->ioctl(sk, cmd, arg);
 	}
 	/*NOTREACHED*/
 	return 0;
@@ -636,11 +642,14 @@ INDIRECT_CALLABLE_DECLARE(int udpv6_sendmsg(struct sock *, struct msghdr *,
 int inet6_sendmsg(struct socket *sock, struct msghdr *msg, size_t size)
 {
 	struct sock *sk = sock->sk;
+	const struct proto *prot;
 
 	if (unlikely(inet_send_prepare(sk)))
 		return -EAGAIN;
 
-	return INDIRECT_CALL_2(sk->sk_prot->sendmsg, tcp_sendmsg, udpv6_sendmsg,
+	/* IPV6_ADDRFORM can change sk->sk_prot under us. */
+	prot = READ_ONCE(sk->sk_prot);
+	return INDIRECT_CALL_2(prot->sendmsg, tcp_sendmsg, udpv6_sendmsg,
 			       sk, msg, size);
 }
 
@@ -650,13 +659,16 @@ int inet6_recvmsg(struct socket *sock, struct msghdr *msg, size_t size,
 		  int flags)
 {
 	struct sock *sk = sock->sk;
+	const struct proto *prot;
 	int addr_len = 0;
 	int err;
 
 	if (likely(!(flags & MSG_ERRQUEUE)))
 		sock_rps_record_flow(sk);
 
-	err = INDIRECT_CALL_2(sk->sk_prot->recvmsg, tcp_recvmsg, udpv6_recvmsg,
+	/* IPV6_ADDRFORM can change sk->sk_prot under us. */
+	prot = READ_ONCE(sk->sk_prot);
+	err = INDIRECT_CALL_2(prot->recvmsg, tcp_recvmsg, udpv6_recvmsg,
 			      sk, msg, size, flags & MSG_DONTWAIT,
 			      flags & ~MSG_DONTWAIT, &addr_len);
 	if (err >= 0)
diff --git a/net/ipv6/ipv6_sockglue.c b/net/ipv6/ipv6_sockglue.c
index a733803a710c..222f6bf220ba 100644
--- a/net/ipv6/ipv6_sockglue.c
+++ b/net/ipv6/ipv6_sockglue.c
@@ -475,7 +475,8 @@ static int do_ipv6_setsockopt(struct sock *sk, int level, int optname,
 				sock_prot_inuse_add(net, sk->sk_prot, -1);
 				sock_prot_inuse_add(net, &tcp_prot, 1);
 
-				sk->sk_prot = &tcp_prot;
+				/* Paired with READ_ONCE(sk->sk_prot) in net/ipv6/af_inet6.c */
+				WRITE_ONCE(sk->sk_prot, &tcp_prot);
 				icsk->icsk_af_ops = &ipv4_specific;
 				sk->sk_socket->ops = &inet_stream_ops;
 				sk->sk_family = PF_INET;
@@ -489,7 +490,8 @@ static int do_ipv6_setsockopt(struct sock *sk, int level, int optname,
 				sock_prot_inuse_add(net, sk->sk_prot, -1);
 				sock_prot_inuse_add(net, prot, 1);
 
-				sk->sk_prot = prot;
+				/* Paired with READ_ONCE(sk->sk_prot) in net/ipv6/af_inet6.c */
+				WRITE_ONCE(sk->sk_prot, prot);
 				sk->sk_socket->ops = &inet_dgram_ops;
 				sk->sk_family = PF_INET;
 			}

```

## File: `net/ipv6/af_inet6.c`

- changed old-lines: [447, 448, 575, 577, 643, 659]

### Function `inet6_bind` (L440–L462, 23 lines)

```c
int inet6_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
	struct sock *sk = sock->sk;
	u32 flags = BIND_WITH_LOCK;
	int err = 0;

	/* If the socket has its own bind function then use it. */
	if (sk->sk_prot->bind)
		return sk->sk_prot->bind(sk, uaddr, addr_len);

	if (addr_len < SIN6_LEN_RFC2133)
		return -EINVAL;

	/* BPF prog is run before any checks are done so that if the prog
	 * changes context in a wrong way it will be caught.
	 */
	err = BPF_CGROUP_RUN_PROG_INET_BIND_LOCK(sk, uaddr,
						 CGROUP_INET6_BIND, &flags);
	if (err)
		return err;

	return __inet6_bind(sk, uaddr, addr_len, flags);
}
```

**External callers of `inet6_bind` at parent**

- `include/net/ipv6.h:1138` — `int inet6_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len);`
- `include/net/ipv6_stubs.h:77` — `	int (*inet6_bind)(struct sock *sk, struct sockaddr *uaddr, int addr_len,`
- `net/core/filter.c:5602` — `		return ipv6_bpf_stub->inet6_bind(sk, addr, addr_len, flags);`
- `net/dccp/ipv6.c:1071` — `	.bind		   = inet6_bind,`
- `net/ipv6/raw.c:1316` — `	.bind		   = inet6_bind,`
- `net/l2tp/l2tp_ip6.c:738` — `	.bind		   = inet6_bind,`
- `net/sctp/ipv6.c:1071` — `	.bind		   = inet6_bind,`

### Function `inet6_ioctl` (L553–L581, 29 lines)

```c
int inet6_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct sock *sk = sock->sk;
	struct net *net = sock_net(sk);

	switch (cmd) {
	case SIOCADDRT:
	case SIOCDELRT: {
		struct in6_rtmsg rtmsg;

		if (copy_from_user(&rtmsg, argp, sizeof(rtmsg)))
			return -EFAULT;
		return ipv6_route_ioctl(net, cmd, &rtmsg);
	}
	case SIOCSIFADDR:
		return addrconf_add_ifaddr(net, argp);
	case SIOCDIFADDR:
		return addrconf_del_ifaddr(net, argp);
	case SIOCSIFDSTADDR:
		return addrconf_set_dstaddr(net, argp);
	default:
		if (!sk->sk_prot->ioctl)
			return -ENOIOCTLCMD;
		return sk->sk_prot->ioctl(sk, cmd, arg);
	}
	/*NOTREACHED*/
	return 0;
}
```

**External callers of `inet6_ioctl` at parent**

- `include/net/ipv6.h:1141` — `int inet6_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg);`
- `net/dccp/ipv6.c:1077` — `	.ioctl		   = inet6_ioctl,`
- `net/ipv6/raw.c:1322` — `	.ioctl		   = inet6_ioctl,		/* must change  */`
- `net/l2tp/l2tp_ip6.c:744` — `	.ioctl		   = inet6_ioctl,`
- `net/mptcp/protocol.c:3715` — `	.ioctl		   = inet6_ioctl,`
- `net/sctp/ipv6.c:1077` — `	.ioctl		   = inet6_ioctl,`

### Function `inet6_sendmsg` (L636–L645, 10 lines)

```c
int inet6_sendmsg(struct socket *sock, struct msghdr *msg, size_t size)
{
	struct sock *sk = sock->sk;

	if (unlikely(inet_send_prepare(sk)))
		return -EAGAIN;

	return INDIRECT_CALL_2(sk->sk_prot->sendmsg, tcp_sendmsg, udpv6_sendmsg,
			       sk, msg, size);
}
```

**External callers of `inet6_sendmsg` at parent**

- `include/net/ipv6.h:1147` — `int inet6_sendmsg(struct socket *sock, struct msghdr *msg, size_t size);`
- `net/mptcp/protocol.c:3721` — `	.sendmsg	   = inet6_sendmsg,`
- `net/socket.c:701` — `INDIRECT_CALLABLE_DECLARE(int inet6_sendmsg(struct socket *, struct msghdr *,`
- `net/socket.c:705` — `	int ret = INDIRECT_CALL_INET(sock->ops->sendmsg, inet6_sendmsg,`

### Function `inet6_recvmsg` (L649–L665, 17 lines)

```c
int inet6_recvmsg(struct socket *sock, struct msghdr *msg, size_t size,
		  int flags)
{
	struct sock *sk = sock->sk;
	int addr_len = 0;
	int err;

	if (likely(!(flags & MSG_ERRQUEUE)))
		sock_rps_record_flow(sk);

	err = INDIRECT_CALL_2(sk->sk_prot->recvmsg, tcp_recvmsg, udpv6_recvmsg,
			      sk, msg, size, flags & MSG_DONTWAIT,
			      flags & ~MSG_DONTWAIT, &addr_len);
	if (err >= 0)
		msg->msg_namelen = addr_len;
	return err;
}
```

**External callers of `inet6_recvmsg` at parent**

- `include/net/ipv6.h:1148` — `int inet6_recvmsg(struct socket *sock, struct msghdr *msg, size_t size,`
- `net/mptcp/protocol.c:3722` — `	.recvmsg	   = inet6_recvmsg,`
- `net/socket.c:943` — `INDIRECT_CALLABLE_DECLARE(int inet6_recvmsg(struct socket *, struct msghdr *,`
- `net/socket.c:948` — `	return INDIRECT_CALL_INET(sock->ops->recvmsg, inet6_recvmsg,`

## File: `net/ipv6/ipv6_sockglue.c`

- changed old-lines: [478, 492]

### Function `do_ipv6_setsockopt` (L394–L1006, 613 lines)

```c
static int do_ipv6_setsockopt(struct sock *sk, int level, int optname,
		   sockptr_t optval, unsigned int optlen)
{
	struct ipv6_pinfo *np = inet6_sk(sk);
	struct net *net = sock_net(sk);
	int val, valbool;
	int retv = -ENOPROTOOPT;
	bool needs_rtnl = setsockopt_needs_rtnl(optname);

	if (sockptr_is_null(optval))
		val = 0;
	else {
		if (optlen >= sizeof(int)) {
			if (copy_from_sockptr(&val, optval, sizeof(val)))
				return -EFAULT;
		} else
			val = 0;
	}

	valbool = (val != 0);

	if (ip6_mroute_opt(optname))
		return ip6_mroute_setsockopt(sk, optname, optval, optlen);

	if (needs_rtnl)
		rtnl_lock();
	lock_sock(sk);

	switch (optname) {

	case IPV6_ADDRFORM:
		if (optlen < sizeof(int))
			goto e_inval;
		if (val == PF_INET) {
			struct ipv6_txoptions *opt;
			struct sk_buff *pktopt;

			if (sk->sk_type == SOCK_RAW)
				break;

			if (sk->sk_protocol == IPPROTO_UDP ||
			    sk->sk_protocol == IPPROTO_UDPLITE) {
				struct udp_sock *up = udp_sk(sk);
				if (up->pending == AF_INET6) {
					retv = -EBUSY;
					break;
				}
			} else if (sk->sk_protocol == IPPROTO_TCP) {
				if (sk->sk_prot != &tcpv6_prot) {
					retv = -EBUSY;
					break;
				}
			} else {
				break;
			}

			if (sk->sk_state != TCP_ESTABLISHED) {
				retv = -ENOTCONN;
				break;
			}

			if (ipv6_only_sock(sk) ||
			    !ipv6_addr_v4mapped(&sk->sk_v6_daddr)) {
				retv = -EADDRNOTAVAIL;
				break;
			}

			fl6_free_socklist(sk);
			__ipv6_sock_mc_close(sk);
			__ipv6_sock_ac_close(sk);

			/*
			 * Sock is moving from IPv6 to IPv4 (sk_prot), so
			 * remove it from the refcnt debug socks count in the
			 * original family...
			 */
			sk_refcnt_debug_dec(sk);

			if (sk->sk_protocol == IPPROTO_TCP) {
				struct inet_connection_sock *icsk = inet_csk(sk);

				sock_prot_inuse_add(net, sk->sk_prot, -1);
				sock_prot_inuse_add(net, &tcp_prot, 1);

				sk->sk_prot = &tcp_prot;
				icsk->icsk_af_ops = &ipv4_specific;
				sk->sk_socket->ops = &inet_stream_ops;
				sk->sk_family = PF_INET;
				tcp_sync_mss(sk, icsk->icsk_pmtu_cookie);
			} else {
				struct proto *prot = &udp_prot;

				if (sk->sk_protocol == IPPROTO_UDPLITE)
					prot = &udplite_prot;

				sock_prot_inuse_add(net, sk->sk_prot, -1);
				sock_prot_inuse_add(net, prot, 1);

				sk->sk_prot = prot;
				sk->sk_socket->ops = &inet_dgram_ops;
				sk->sk_family = PF_INET;
			}
			opt = xchg((__force struct ipv6_txoptions **)&np->opt,
				   NULL);
			if (opt) {
				atomic_sub(opt->tot_len, &sk->sk_omem_alloc);
				txopt_put(opt);
			}
			pktopt = xchg(&np->pktoptions, NULL);
			kfree_skb(pktopt);

			/*
			 * ... and add it to the refcnt debug socks count
			 * in the new family. -acme
			 */
			sk_refcnt_debug_inc(sk);
			module_put(THIS_MODULE);
			retv = 0;
			break;
		}
		goto e_inval;

	case IPV6_V6ONLY:
		if (optlen < sizeof(int) ||
		    inet_sk(sk)->inet_num)
			goto e_inval;
		sk->sk_ipv6only = valbool;
		retv = 0;
		break;

	case IPV6_RECVPKTINFO:
		if (optlen < sizeof(int))
			goto e_inval;
		np->rxopt.bits.rxinfo = valbool;
		retv = 0;
		break;

	case IPV6_2292PKTINFO:
		if (optlen < sizeof(int))
			goto e_inval;
		np->rxopt.bits.rxoinfo = valbool;
		retv = 0;
		break;

	case IPV6_RECVHOPLIMIT:
		if (optlen < sizeof(int))
			goto e_inval;
		np->rxopt.bits.rxhlim = valbool;
		retv = 0;
		break;

	case IPV6_2292HOPLIMIT:
		if (optlen < sizeof(int))
			goto e_inval;
		np->rxopt.bits.rxohlim = valbool;
		retv = 0;
		break;

	case IPV6_RECVRTHDR:
		if (optlen < sizeof(int))
			goto e_inval;
		np->rxopt.bits.srcrt = valbool;
		retv = 0;
		break;

	case IPV6_2292RTHDR:
		if (optlen < sizeof(int))
			goto e_inval;
		np->rxopt.bits.osrcrt = valbool;
		retv = 0;
		break;

	case IPV6_RECVHOPOPTS:
		if (optlen < sizeof(int))
			goto e_inval;
		np->rxopt.bits.hopopts = valbool;
		retv = 0;
		break;

	case IPV6_2292HOPOPTS:
		if (optlen < sizeof(int))
			goto e_inval;
		np->rxopt.bits.ohopopts = valbool;
		retv = 0;
		break;

	case IPV6_RECVDSTOPTS:
		if (optlen < sizeof(int))
			goto e_inval;
		np->rxopt.bits.dstopts = valbool;
		retv = 0;
		break;

	case IPV6_2292DSTOPTS:
		if (optlen < sizeof(int))
			goto e_inval;
		np->rxopt.bits.odstopts = valbool;
		retv = 0;
		break;

	case IPV6_TCLASS:
		if (optlen < sizeof(int))
			goto e_inval;
		if (val < -1 || val > 0xff)
			goto e_inval;
		/* RFC 3542, 6.5: default traffic class of 0x0 */
		if (val == -1)
			val = 0;
		if (sk->sk_type == SOCK_STREAM) {
			val &= ~INET_ECN_MASK;
			val |= np->tclass & INET_ECN_MASK;
		}
		if (np->tclass != val) {
			np->tclass = val;
			sk_dst_reset(sk);
		}
		retv = 0;
		break;

	case IPV6_RECVTCLASS:
		if (optlen < sizeof(int))
			goto e_inval;
		np->rxopt.bits.rxtclass = valbool;
		retv = 0;
		break;

	case IPV6_FLOWINFO:
		if (optlen < sizeof(int))
			goto e_inval;
		np->rxopt.bits.rxflow = valbool;
		retv = 0;
		break;

	case IPV6_RECVPATHMTU:
		if (optlen < sizeof(int))
			goto e_inval;
		np->rxopt.bits.rxpmtu = valbool;
		retv = 0;
		break;

	case IPV6_TRANSPARENT:
		if (valbool && !ns_capable(net->user_ns, CAP_NET_RAW) &&
		    !ns_capable(net->user_ns, CAP_NET_ADMIN)) {
			retv = -EPERM;
			break;
		}
		if (optlen < sizeof(int))
			goto e_inval;
		/* we don't have a separate transparent bit for IPV6 we use the one in the IPv4 socket */
		inet_sk(sk)->transparent = valbool;
		retv = 0;
		break;

	case IPV6_FREEBIND:
		if (optlen < sizeof(int))
			goto e_inval;
		/* we also don't have a separate freebind bit for IPV6 */
		inet_sk(sk)->freebind = valbool;
		retv = 0;
		break;

	case IPV6_RECVORIGDSTADDR:
		if (optlen < sizeof(int))
			goto e_inval;
		np->rxopt.bits.rxorigdstaddr = valbool;
		retv = 0;
		break;

	case IPV6_HOPOPTS:
	case IPV6_RTHDRDSTOPTS:
	case IPV6_RTHDR:
	case IPV6_DSTOPTS:
		retv = ipv6_set_opt_hdr(sk, optname, optval, optlen);
		break;

	case IPV6_PKTINFO:
	{
		struct in6_pktinfo pkt;

		if (optlen == 0)
			goto e_inval;
		else if (optlen < sizeof(struct in6_pktinfo) ||
			 sockptr_is_null(optval))
			goto e_inval;

		if (copy_from_sockptr(&pkt, optval, sizeof(p
```

**External callers of `do_ipv6_setsockopt` at parent**

- `net/ipv6/tcp_ipv6.c:419` — `		/* min_hopcount can be changed concurrently from do_ipv6_setsockopt() */`
- `net/ipv6/tcp_ipv6.c:1737` — `		/* min_hopcount can be changed concurrently from do_ipv6_setsockopt() */`
