# Evidence: SYZBOT-5b22981f6d3c7be1  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in br_handle_frame_finish / br_handle_frame_finish (8)
- mainline fix commit: `44bdb313da57322c9b3c108eb66981c6ec6509f4`
- candidate JSON: `A_SYZBOT-5b22981f6d3c7be1.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "br_handle_frame_finish", "fn_b": "br_handle_frame_finish"}`

## Commit message

```
44bdb313da57322c9b3c108eb66981c6ec6509f4
net: bridge: use DEV_STATS_INC()

syzbot/KCSAN reported data-races in br_handle_frame_finish() [1]
This function can run from multiple cpus without mutual exclusion.

Adopt SMP safe DEV_STATS_INC() to update dev->stats fields.

Handles updates to dev->stats.tx_dropped while we are at it.

[1]
BUG: KCSAN: data-race in br_handle_frame_finish / br_handle_frame_finish

read-write to 0xffff8881374b2178 of 8 bytes by interrupt on cpu 1:
br_handle_frame_finish+0xd4f/0xef0 net/bridge/br_input.c:189
br_nf_hook_thresh+0x1ed/0x220
br_nf_pre_routing_finish_ipv6+0x50f/0x540
NF_HOOK include/linux/netfilter.h:304 [inline]
br_nf_pre_routing_ipv6+0x1e3/0x2a0 net/bridge/br_netfilter_ipv6.c:178
br_nf_pre_routing+0x526/0xba0 net/bridge/br_netfilter_hooks.c:508
nf_hook_entry_hookfn include/linux/netfilter.h:144 [inline]
nf_hook_bridge_pre net/bridge/br_input.c:272 [inline]
br_handle_frame+0x4c9/0x940 net/bridge/br_input.c:417
__netif_receive_skb_core+0xa8a/0x21e0 net/core/dev.c:5417
__netif_receive_skb_one_core net/core/dev.c:5521 [inline]
__netif_receive_skb+0x57/0x1b0 net/core/dev.c:5637
process_backlog+0x21f/0x380 net/core/dev.c:5965
__napi_poll+0x60/0x3b0 net/core/dev.c:6527
napi_poll net/core/dev.c:6594 [inline]
net_rx_action+0x32b/0x750 net/core/dev.c:6727
__do_softirq+0xc1/0x265 kernel/softirq.c:553
run_ksoftirqd+0x17/0x20 kernel/softirq.c:921
smpboot_thread_fn+0x30a/0x4a0 kernel/smpboot.c:164
kthread+0x1d7/0x210 kernel/kthread.c:388
ret_from_fork+0x48/0x60 arch/x86/kernel/process.c:147
ret_from_fork_asm+0x11/0x20 arch/x86/entry/entry_64.S:304

read-write to 0xffff8881374b2178 of 8 bytes by interrupt on cpu 0:
br_handle_frame_finish+0xd4f/0xef0 net/bridge/br_input.c:189
br_nf_hook_thresh+0x1ed/0x220
br_nf_pre_routing_finish_ipv6+0x50f/0x540
NF_HOOK include/linux/netfilter.h:304 [inline]
br_nf_pre_routing_ipv6+0x1e3/0x2a0 net/bridge/br_netfilter_ipv6.c:178
br_nf_pre_routing+0x526/0xba0 net/bridge/br_netfilter_hooks.c:508
nf_hook_entry_hookfn include/linux/netfilter.h:144 [inline]
nf_hook_bridge_pre net/bridge/br_input.c:272 [inline]
br_handle_frame+0x4c9/0x940 net/bridge/br_input.c:417
__netif_receive_skb_core+0xa8a/0x21e0 net/core/dev.c:5417
__netif_receive_skb_one_core net/core/dev.c:5521 [inline]
__netif_receive_skb+0x57/0x1b0 net/core/dev.c:5637
process_backlog+0x21f/0x380 net/core/dev.c:5965
__napi_poll+0x60/0x3b0 net/core/dev.c:6527
napi_poll net/core/dev.c:6594 [inline]
net_rx_action+0x32b/0x750 net/core/dev.c:6727
__do_softirq+0xc1/0x265 kernel/softirq.c:553
do_softirq+0x5e/0x90 kernel/softirq.c:454
__local_bh_enable_ip+0x64/0x70 kernel/softirq.c:381
__raw_spin_unlock_bh include/linux/spinlock_api_smp.h:167 [inline]
_raw_spin_unlock_bh+0x36/0x40 kernel/locking/spinlock.c:210
spin_unlock_bh include/linux/spinlock.h:396 [inline]
batadv_tt_local_purge+0x1a8/0x1f0 net/batman-adv/translation-table.c:1356
batadv_tt_purge+0x2b/0x630 net/batman-adv/translation-table.c:3560
process_one_work kernel/workqueue.c:2630 [inline]
process_scheduled_works+0x5b8/0xa30 kernel/workqueue.c:2703
worker_thread+0x525/0x730 kernel/workqueue.c:2784
kthread+0x1d7/0x210 kernel/kthread.c:388
ret_from_fork+0x48/0x60 arch/x86/kernel/process.c:147
ret_from_fork_asm+0x11/0x20 arch/x86/entry/entry_64.S:304

value changed: 0x00000000000d7190 -> 0x00000000000d7191

Reported by Kernel Concurrency Sanitizer on:
CPU: 0 PID: 14848 Comm: kworker/u4:11 Not tainted 6.6.0-rc1-syzkaller-00236-gad8a69f361b9 #0

Fixes: 1c29fc4989bc ("[BRIDGE]: keep track of received multicast packets")
Reported-by: syzbot <syzkaller@googlegroups.com>
Signed-off-by: Eric Dumazet <edumazet@google.com>
Cc: Roopa Prabhu <roopa@nvidia.com>
Cc: Nikolay Aleksandrov <razor@blackwall.org>
Cc: bridge@lists.linux-foundation.org
Acked-by: Nikolay Aleksandrov <razor@blackwall.org>
Link: https://lore.kernel.org/r/20230918091351.1356153-1-edumazet@google.com
Signed-off-by: Paolo Abeni <pabeni@redhat.com>
```

## CVE / bug description

```
KCSAN: data-race in br_handle_frame_finish / br_handle_frame_finish (8)
```

## Full mainline patch

```diff
commit 44bdb313da57322c9b3c108eb66981c6ec6509f4
Author: Eric Dumazet <edumazet@google.com>
Date:   Mon Sep 18 09:13:51 2023 +0000

    net: bridge: use DEV_STATS_INC()
    
    syzbot/KCSAN reported data-races in br_handle_frame_finish() [1]
    This function can run from multiple cpus without mutual exclusion.
    
    Adopt SMP safe DEV_STATS_INC() to update dev->stats fields.
    
    Handles updates to dev->stats.tx_dropped while we are at it.
    
    [1]
    BUG: KCSAN: data-race in br_handle_frame_finish / br_handle_frame_finish
    
    read-write to 0xffff8881374b2178 of 8 bytes by interrupt on cpu 1:
    br_handle_frame_finish+0xd4f/0xef0 net/bridge/br_input.c:189
    br_nf_hook_thresh+0x1ed/0x220
    br_nf_pre_routing_finish_ipv6+0x50f/0x540
    NF_HOOK include/linux/netfilter.h:304 [inline]
    br_nf_pre_routing_ipv6+0x1e3/0x2a0 net/bridge/br_netfilter_ipv6.c:178
    br_nf_pre_routing+0x526/0xba0 net/bridge/br_netfilter_hooks.c:508
    nf_hook_entry_hookfn include/linux/netfilter.h:144 [inline]
    nf_hook_bridge_pre net/bridge/br_input.c:272 [inline]
    br_handle_frame+0x4c9/0x940 net/bridge/br_input.c:417
    __netif_receive_skb_core+0xa8a/0x21e0 net/core/dev.c:5417
    __netif_receive_skb_one_core net/core/dev.c:5521 [inline]
    __netif_receive_skb+0x57/0x1b0 net/core/dev.c:5637
    process_backlog+0x21f/0x380 net/core/dev.c:5965
    __napi_poll+0x60/0x3b0 net/core/dev.c:6527
    napi_poll net/core/dev.c:6594 [inline]
    net_rx_action+0x32b/0x750 net/core/dev.c:6727
    __do_softirq+0xc1/0x265 kernel/softirq.c:553
    run_ksoftirqd+0x17/0x20 kernel/softirq.c:921
    smpboot_thread_fn+0x30a/0x4a0 kernel/smpboot.c:164
    kthread+0x1d7/0x210 kernel/kthread.c:388
    ret_from_fork+0x48/0x60 arch/x86/kernel/process.c:147
    ret_from_fork_asm+0x11/0x20 arch/x86/entry/entry_64.S:304
    
    read-write to 0xffff8881374b2178 of 8 bytes by interrupt on cpu 0:
    br_handle_frame_finish+0xd4f/0xef0 net/bridge/br_input.c:189
    br_nf_hook_thresh+0x1ed/0x220
    br_nf_pre_routing_finish_ipv6+0x50f/0x540
    NF_HOOK include/linux/netfilter.h:304 [inline]
    br_nf_pre_routing_ipv6+0x1e3/0x2a0 net/bridge/br_netfilter_ipv6.c:178
    br_nf_pre_routing+0x526/0xba0 net/bridge/br_netfilter_hooks.c:508
    nf_hook_entry_hookfn include/linux/netfilter.h:144 [inline]
    nf_hook_bridge_pre net/bridge/br_input.c:272 [inline]
    br_handle_frame+0x4c9/0x940 net/bridge/br_input.c:417
    __netif_receive_skb_core+0xa8a/0x21e0 net/core/dev.c:5417
    __netif_receive_skb_one_core net/core/dev.c:5521 [inline]
    __netif_receive_skb+0x57/0x1b0 net/core/dev.c:5637
    process_backlog+0x21f/0x380 net/core/dev.c:5965
    __napi_poll+0x60/0x3b0 net/core/dev.c:6527
    napi_poll net/core/dev.c:6594 [inline]
    net_rx_action+0x32b/0x750 net/core/dev.c:6727
    __do_softirq+0xc1/0x265 kernel/softirq.c:553
    do_softirq+0x5e/0x90 kernel/softirq.c:454
    __local_bh_enable_ip+0x64/0x70 kernel/softirq.c:381
    __raw_spin_unlock_bh include/linux/spinlock_api_smp.h:167 [inline]
    _raw_spin_unlock_bh+0x36/0x40 kernel/locking/spinlock.c:210
    spin_unlock_bh include/linux/spinlock.h:396 [inline]
    batadv_tt_local_purge+0x1a8/0x1f0 net/batman-adv/translation-table.c:1356
    batadv_tt_purge+0x2b/0x630 net/batman-adv/translation-table.c:3560
    process_one_work kernel/workqueue.c:2630 [inline]
    process_scheduled_works+0x5b8/0xa30 kernel/workqueue.c:2703
    worker_thread+0x525/0x730 kernel/workqueue.c:2784
    kthread+0x1d7/0x210 kernel/kthread.c:388
    ret_from_fork+0x48/0x60 arch/x86/kernel/process.c:147
    ret_from_fork_asm+0x11/0x20 arch/x86/entry/entry_64.S:304
    
    value changed: 0x00000000000d7190 -> 0x00000000000d7191
    
    Reported by Kernel Concurrency Sanitizer on:
    CPU: 0 PID: 14848 Comm: kworker/u4:11 Not tainted 6.6.0-rc1-syzkaller-00236-gad8a69f361b9 #0
    
    Fixes: 1c29fc4989bc ("[BRIDGE]: keep track of received multicast packets")
    Reported-by: syzbot <syzkaller@googlegroups.com>
    Signed-off-by: Eric Dumazet <edumazet@google.com>
    Cc: Roopa Prabhu <roopa@nvidia.com>
    Cc: Nikolay Aleksandrov <razor@blackwall.org>
    Cc: bridge@lists.linux-foundation.org
    Acked-by: Nikolay Aleksandrov <razor@blackwall.org>
    Link: https://lore.kernel.org/r/20230918091351.1356153-1-edumazet@google.com
    Signed-off-by: Paolo Abeni <pabeni@redhat.com>

diff --git a/net/bridge/br_forward.c b/net/bridge/br_forward.c
index 9d7bc8b96b53..7431f89e897b 100644
--- a/net/bridge/br_forward.c
+++ b/net/bridge/br_forward.c
@@ -124,7 +124,7 @@ static int deliver_clone(const struct net_bridge_port *prev,
 
 	skb = skb_clone(skb, GFP_ATOMIC);
 	if (!skb) {
-		dev->stats.tx_dropped++;
+		DEV_STATS_INC(dev, tx_dropped);
 		return -ENOMEM;
 	}
 
@@ -268,7 +268,7 @@ static void maybe_deliver_addr(struct net_bridge_port *p, struct sk_buff *skb,
 
 	skb = skb_copy(skb, GFP_ATOMIC);
 	if (!skb) {
-		dev->stats.tx_dropped++;
+		DEV_STATS_INC(dev, tx_dropped);
 		return;
 	}
 
diff --git a/net/bridge/br_input.c b/net/bridge/br_input.c
index c34a0b0901b0..c729528b5e85 100644
--- a/net/bridge/br_input.c
+++ b/net/bridge/br_input.c
@@ -181,12 +181,12 @@ int br_handle_frame_finish(struct net *net, struct sock *sk, struct sk_buff *skb
 			if ((mdst && mdst->host_joined) ||
 			    br_multicast_is_router(brmctx, skb)) {
 				local_rcv = true;
-				br->dev->stats.multicast++;
+				DEV_STATS_INC(br->dev, multicast);
 			}
 			mcast_hit = true;
 		} else {
 			local_rcv = true;
-			br->dev->stats.multicast++;
+			DEV_STATS_INC(br->dev, multicast);
 		}
 		break;
 	case BR_PKT_UNICAST:

```

## File: `net/bridge/br_forward.c`

- changed old-lines: [127, 271]

### Function `deliver_clone` (L120–L133, 14 lines)

```c
static int deliver_clone(const struct net_bridge_port *prev,
			 struct sk_buff *skb, bool local_orig)
{
	struct net_device *dev = BR_INPUT_SKB_CB(skb)->brdev;

	skb = skb_clone(skb, GFP_ATOMIC);
	if (!skb) {
		dev->stats.tx_dropped++;
		return -ENOMEM;
	}

	__br_forward(prev, skb, local_orig);
	return 0;
}
```

_(no external call sites found for `deliver_clone`)_

### Function `maybe_deliver_addr` (L256–L279, 24 lines)

```c
static void maybe_deliver_addr(struct net_bridge_port *p, struct sk_buff *skb,
			       const unsigned char *addr, bool local_orig)
{
	struct net_device *dev = BR_INPUT_SKB_CB(skb)->brdev;
	const unsigned char *src = eth_hdr(skb)->h_source;

	if (!should_deliver(p, skb))
		return;

	/* Even with hairpin, no soliloquies - prevent breaking IPv6 DAD */
	if (skb->dev == p->dev && ether_addr_equal(src, addr))
		return;

	skb = skb_copy(skb, GFP_ATOMIC);
	if (!skb) {
		dev->stats.tx_dropped++;
		return;
	}

	if (!is_broadcast_ether_addr(addr))
		memcpy(eth_hdr(skb)->h_dest, addr, ETH_ALEN);

	__br_forward(p, skb, local_orig);
}
```

_(no external call sites found for `maybe_deliver_addr`)_

## File: `net/bridge/br_input.c`

- changed old-lines: [184, 189]

### Function `br_handle_frame_finish` (L74–L223, 150 lines)

```c
int br_handle_frame_finish(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	struct net_bridge_port *p = br_port_get_rcu(skb->dev);
	enum br_pkt_type pkt_type = BR_PKT_UNICAST;
	struct net_bridge_fdb_entry *dst = NULL;
	struct net_bridge_mcast_port *pmctx;
	struct net_bridge_mdb_entry *mdst;
	bool local_rcv, mcast_hit = false;
	struct net_bridge_mcast *brmctx;
	struct net_bridge_vlan *vlan;
	struct net_bridge *br;
	u16 vid = 0;
	u8 state;

	if (!p)
		goto drop;

	br = p->br;

	if (br_mst_is_enabled(br)) {
		state = BR_STATE_FORWARDING;
	} else {
		if (p->state == BR_STATE_DISABLED)
			goto drop;

		state = p->state;
	}

	brmctx = &p->br->multicast_ctx;
	pmctx = &p->multicast_ctx;
	if (!br_allowed_ingress(p->br, nbp_vlan_group_rcu(p), skb, &vid,
				&state, &vlan))
		goto out;

	if (p->flags & BR_PORT_LOCKED) {
		struct net_bridge_fdb_entry *fdb_src =
			br_fdb_find_rcu(br, eth_hdr(skb)->h_source, vid);

		if (!fdb_src) {
			/* FDB miss. Create locked FDB entry if MAB is enabled
			 * and drop the packet.
			 */
			if (p->flags & BR_PORT_MAB)
				br_fdb_update(br, p, eth_hdr(skb)->h_source,
					      vid, BIT(BR_FDB_LOCKED));
			goto drop;
		} else if (READ_ONCE(fdb_src->dst) != p ||
			   test_bit(BR_FDB_LOCAL, &fdb_src->flags)) {
			/* FDB mismatch. Drop the packet without roaming. */
			goto drop;
		} else if (test_bit(BR_FDB_LOCKED, &fdb_src->flags)) {
			/* FDB match, but entry is locked. Refresh it and drop
			 * the packet.
			 */
			br_fdb_update(br, p, eth_hdr(skb)->h_source, vid,
				      BIT(BR_FDB_LOCKED));
			goto drop;
		}
	}

	nbp_switchdev_frame_mark(p, skb);

	/* insert into forwarding database after filtering to avoid spoofing */
	if (p->flags & BR_LEARNING)
		br_fdb_update(br, p, eth_hdr(skb)->h_source, vid, 0);

	local_rcv = !!(br->dev->flags & IFF_PROMISC);
	if (is_multicast_ether_addr(eth_hdr(skb)->h_dest)) {
		/* by definition the broadcast is also a multicast address */
		if (is_broadcast_ether_addr(eth_hdr(skb)->h_dest)) {
			pkt_type = BR_PKT_BROADCAST;
			local_rcv = true;
		} else {
			pkt_type = BR_PKT_MULTICAST;
			if (br_multicast_rcv(&brmctx, &pmctx, vlan, skb, vid))
				goto drop;
		}
	}

	if (state == BR_STATE_LEARNING)
		goto drop;

	BR_INPUT_SKB_CB(skb)->brdev = br->dev;
	BR_INPUT_SKB_CB(skb)->src_port_isolated = !!(p->flags & BR_ISOLATED);

	if (IS_ENABLED(CONFIG_INET) &&
	    (skb->protocol == htons(ETH_P_ARP) ||
	     skb->protocol == htons(ETH_P_RARP))) {
		br_do_proxy_suppress_arp(skb, br, vid, p);
	} else if (IS_ENABLED(CONFIG_IPV6) &&
		   skb->protocol == htons(ETH_P_IPV6) &&
		   br_opt_get(br, BROPT_NEIGH_SUPPRESS_ENABLED) &&
		   pskb_may_pull(skb, sizeof(struct ipv6hdr) +
				 sizeof(struct nd_msg)) &&
		   ipv6_hdr(skb)->nexthdr == IPPROTO_ICMPV6) {
			struct nd_msg *msg, _msg;

			msg = br_is_nd_neigh_msg(skb, &_msg);
			if (msg)
				br_do_suppress_nd(skb, br, vid, p, msg);
	}

	switch (pkt_type) {
	case BR_PKT_MULTICAST:
		mdst = br_mdb_get(brmctx, skb, vid);
		if ((mdst || BR_INPUT_SKB_CB_MROUTERS_ONLY(skb)) &&
		    br_multicast_querier_exists(brmctx, eth_hdr(skb), mdst)) {
			if ((mdst && mdst->host_joined) ||
			    br_multicast_is_router(brmctx, skb)) {
				local_rcv = true;
				br->dev->stats.multicast++;
			}
			mcast_hit = true;
		} else {
			local_rcv = true;
			br->dev->stats.multicast++;
		}
		break;
	case BR_PKT_UNICAST:
		dst = br_fdb_find_rcu(br, eth_hdr(skb)->h_dest, vid);
		break;
	default:
		break;
	}

	if (dst) {
		unsigned long now = jiffies;

		if (test_bit(BR_FDB_LOCAL, &dst->flags))
			return br_pass_frame_up(skb);

		if (now != dst->used)
			dst->used = now;
		br_forward(dst->dst, skb, local_rcv, false);
	} else {
		if (!mcast_hit)
			br_flood(br, skb, pkt_type, local_rcv, false, vid);
		else
			br_multicast_flood(mdst, skb, brmctx, local_rcv, false);
	}

	if (local_rcv)
		return br_pass_frame_up(skb);

out:
	return 0;
drop:
	kfree_skb(skb);
	goto out;
}
```

**External callers of `br_handle_frame_finish` at parent**

- `include/linux/netfilter_bridge.h:17` — `int br_handle_frame_finish(struct net *net, struct sock *sk, struct sk_buff *skb);`
- `net/bridge/br_netfilter_hooks.c:264` — ` * sure that br_handle_frame_finish() is called afterwards.`
- `net/bridge/br_netfilter_hooks.c:284` — `			ret = br_handle_frame_finish(net, sk, skb);`
- `net/bridge/br_netfilter_hooks.c:426` — `			  br_handle_frame_finish);`
- `net/bridge/br_netfilter_hooks.c:912` — `	br_handle_frame_finish(dev_net(skb->dev), NULL, skb);`
- `net/bridge/br_netfilter_ipv6.c:149` — `			  skb->dev, NULL, br_handle_frame_finish);`
- `net/bridge/br_private.h:909` — `int br_handle_frame_finish(struct net *net, struct sock *sk, struct sk_buff *skb);`
