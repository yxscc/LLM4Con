# Evidence: SYZBOT-44cf88a58d91b12b  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in __ip4_datagram_connect / raw_bind (2)
- mainline fix commit: `153a0d187e767c68733b8e9f46218eb1f41ab902`
- candidate JSON: `A_SYZBOT-44cf88a58d91b12b.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "__ip4_datagram_connect", "fn_b": "raw_bind"}`

## Commit message

```
153a0d187e767c68733b8e9f46218eb1f41ab902
ipv4: raw: lock the socket in raw_bind()

For some reason, raw_bind() forgot to lock the socket.

BUG: KCSAN: data-race in __ip4_datagram_connect / raw_bind

write to 0xffff8881170d4308 of 4 bytes by task 5466 on cpu 0:
 raw_bind+0x1b0/0x250 net/ipv4/raw.c:739
 inet_bind+0x56/0xa0 net/ipv4/af_inet.c:443
 __sys_bind+0x14b/0x1b0 net/socket.c:1697
 __do_sys_bind net/socket.c:1708 [inline]
 __se_sys_bind net/socket.c:1706 [inline]
 __x64_sys_bind+0x3d/0x50 net/socket.c:1706
 do_syscall_x64 arch/x86/entry/common.c:50 [inline]
 do_syscall_64+0x44/0xd0 arch/x86/entry/common.c:80
 entry_SYSCALL_64_after_hwframe+0x44/0xae

read to 0xffff8881170d4308 of 4 bytes by task 5468 on cpu 1:
 __ip4_datagram_connect+0xb7/0x7b0 net/ipv4/datagram.c:39
 ip4_datagram_connect+0x2a/0x40 net/ipv4/datagram.c:89
 inet_dgram_connect+0x107/0x190 net/ipv4/af_inet.c:576
 __sys_connect_file net/socket.c:1900 [inline]
 __sys_connect+0x197/0x1b0 net/socket.c:1917
 __do_sys_connect net/socket.c:1927 [inline]
 __se_sys_connect net/socket.c:1924 [inline]
 __x64_sys_connect+0x3d/0x50 net/socket.c:1924
 do_syscall_x64 arch/x86/entry/common.c:50 [inline]
 do_syscall_64+0x44/0xd0 arch/x86/entry/common.c:80
 entry_SYSCALL_64_after_hwframe+0x44/0xae

value changed: 0x00000000 -> 0x0003007f

Reported by Kernel Concurrency Sanitizer on:
CPU: 1 PID: 5468 Comm: syz-executor.5 Not tainted 5.17.0-rc1-syzkaller #0
Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 01/01/2011

Fixes: 1da177e4c3f4 ("Linux-2.6.12-rc2")
Signed-off-by: Eric Dumazet <edumazet@google.com>
Reported-by: syzbot <syzkaller@googlegroups.com>
Signed-off-by: David S. Miller <davem@davemloft.net>
```

## CVE / bug description

```
KCSAN: data-race in __ip4_datagram_connect / raw_bind (2)
```

## Full mainline patch

```diff
commit 153a0d187e767c68733b8e9f46218eb1f41ab902
Author: Eric Dumazet <edumazet@google.com>
Date:   Wed Jan 26 16:51:16 2022 -0800

    ipv4: raw: lock the socket in raw_bind()
    
    For some reason, raw_bind() forgot to lock the socket.
    
    BUG: KCSAN: data-race in __ip4_datagram_connect / raw_bind
    
    write to 0xffff8881170d4308 of 4 bytes by task 5466 on cpu 0:
     raw_bind+0x1b0/0x250 net/ipv4/raw.c:739
     inet_bind+0x56/0xa0 net/ipv4/af_inet.c:443
     __sys_bind+0x14b/0x1b0 net/socket.c:1697
     __do_sys_bind net/socket.c:1708 [inline]
     __se_sys_bind net/socket.c:1706 [inline]
     __x64_sys_bind+0x3d/0x50 net/socket.c:1706
     do_syscall_x64 arch/x86/entry/common.c:50 [inline]
     do_syscall_64+0x44/0xd0 arch/x86/entry/common.c:80
     entry_SYSCALL_64_after_hwframe+0x44/0xae
    
    read to 0xffff8881170d4308 of 4 bytes by task 5468 on cpu 1:
     __ip4_datagram_connect+0xb7/0x7b0 net/ipv4/datagram.c:39
     ip4_datagram_connect+0x2a/0x40 net/ipv4/datagram.c:89
     inet_dgram_connect+0x107/0x190 net/ipv4/af_inet.c:576
     __sys_connect_file net/socket.c:1900 [inline]
     __sys_connect+0x197/0x1b0 net/socket.c:1917
     __do_sys_connect net/socket.c:1927 [inline]
     __se_sys_connect net/socket.c:1924 [inline]
     __x64_sys_connect+0x3d/0x50 net/socket.c:1924
     do_syscall_x64 arch/x86/entry/common.c:50 [inline]
     do_syscall_64+0x44/0xd0 arch/x86/entry/common.c:80
     entry_SYSCALL_64_after_hwframe+0x44/0xae
    
    value changed: 0x00000000 -> 0x0003007f
    
    Reported by Kernel Concurrency Sanitizer on:
    CPU: 1 PID: 5468 Comm: syz-executor.5 Not tainted 5.17.0-rc1-syzkaller #0
    Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 01/01/2011
    
    Fixes: 1da177e4c3f4 ("Linux-2.6.12-rc2")
    Signed-off-by: Eric Dumazet <edumazet@google.com>
    Reported-by: syzbot <syzkaller@googlegroups.com>
    Signed-off-by: David S. Miller <davem@davemloft.net>

diff --git a/net/ipv4/raw.c b/net/ipv4/raw.c
index a53f256bf9d3..0505935b6b8c 100644
--- a/net/ipv4/raw.c
+++ b/net/ipv4/raw.c
@@ -722,6 +722,7 @@ static int raw_bind(struct sock *sk, struct sockaddr *uaddr, int addr_len)
 	int ret = -EINVAL;
 	int chk_addr_ret;
 
+	lock_sock(sk);
 	if (sk->sk_state != TCP_CLOSE || addr_len < sizeof(struct sockaddr_in))
 		goto out;
 
@@ -741,7 +742,9 @@ static int raw_bind(struct sock *sk, struct sockaddr *uaddr, int addr_len)
 		inet->inet_saddr = 0;  /* Use device */
 	sk_dst_reset(sk);
 	ret = 0;
-out:	return ret;
+out:
+	release_sock(sk);
+	return ret;
 }
 
 /*

```

## File: `net/ipv4/raw.c`

- changed old-lines: [744]

### Function `raw_bind` (L716–L745, 30 lines)

```c
static int raw_bind(struct sock *sk, struct sockaddr *uaddr, int addr_len)
{
	struct inet_sock *inet = inet_sk(sk);
	struct sockaddr_in *addr = (struct sockaddr_in *) uaddr;
	struct net *net = sock_net(sk);
	u32 tb_id = RT_TABLE_LOCAL;
	int ret = -EINVAL;
	int chk_addr_ret;

	if (sk->sk_state != TCP_CLOSE || addr_len < sizeof(struct sockaddr_in))
		goto out;

	if (sk->sk_bound_dev_if)
		tb_id = l3mdev_fib_table_by_index(net,
						  sk->sk_bound_dev_if) ? : tb_id;

	chk_addr_ret = inet_addr_type_table(net, addr->sin_addr.s_addr, tb_id);

	ret = -EADDRNOTAVAIL;
	if (!inet_addr_valid_or_nonlocal(net, inet, addr->sin_addr.s_addr,
					 chk_addr_ret))
		goto out;

	inet->inet_rcv_saddr = inet->inet_saddr = addr->sin_addr.s_addr;
	if (chk_addr_ret == RTN_MULTICAST || chk_addr_ret == RTN_BROADCAST)
		inet->inet_saddr = 0;  /* Use device */
	sk_dst_reset(sk);
	ret = 0;
out:	return ret;
}
```

**External callers of `raw_bind` at parent**

- `net/can/raw.c:417` — `static int raw_bind(struct socket *sock, struct sockaddr *uaddr, int len)`
- `net/can/raw.c:899` — `	.bind          = raw_bind,`
- `net/ieee802154/socket.c:196` — `static int raw_bind(struct sock *sk, struct sockaddr *_uaddr, int len)`
- `net/ieee802154/socket.c:394` — `	.bind		= raw_bind,`
