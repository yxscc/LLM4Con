# Evidence: SYZBOT-3cc3a12efa69aa6f  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in register_netdevice / type_show (2)
- mainline fix commit: `cc26c2661fefea215f41edb665193324a5f99021`
- candidate JSON: `A_SYZBOT-3cc3a12efa69aa6f.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "register_netdevice", "fn_b": "type_show"}`

## Commit message

```
cc26c2661fefea215f41edb665193324a5f99021
net: fix data-race in dev_isalive()

dev_isalive() is called under RTNL or dev_base_lock protection.

This means that changes to dev->reg_state should be done with both locks held.

syzbot reported:

BUG: KCSAN: data-race in register_netdevice / type_show

write to 0xffff888144ecf518 of 1 bytes by task 20886 on cpu 0:
register_netdevice+0xb9f/0xdf0 net/core/dev.c:10050
lapbeth_new_device drivers/net/wan/lapbether.c:414 [inline]
lapbeth_device_event+0x4a0/0x6c0 drivers/net/wan/lapbether.c:456
notifier_call_chain kernel/notifier.c:87 [inline]
raw_notifier_call_chain+0x53/0xb0 kernel/notifier.c:455
__dev_notify_flags+0x1d6/0x3a0
dev_change_flags+0xa2/0xc0 net/core/dev.c:8607
do_setlink+0x778/0x2230 net/core/rtnetlink.c:2780
__rtnl_newlink net/core/rtnetlink.c:3546 [inline]
rtnl_newlink+0x114c/0x16a0 net/core/rtnetlink.c:3593
rtnetlink_rcv_msg+0x811/0x8c0 net/core/rtnetlink.c:6089
netlink_rcv_skb+0x13e/0x240 net/netlink/af_netlink.c:2501
rtnetlink_rcv+0x18/0x20 net/core/rtnetlink.c:6107
netlink_unicast_kernel net/netlink/af_netlink.c:1319 [inline]
netlink_unicast+0x58a/0x660 net/netlink/af_netlink.c:1345
netlink_sendmsg+0x661/0x750 net/netlink/af_netlink.c:1921
sock_sendmsg_nosec net/socket.c:714 [inline]
sock_sendmsg net/socket.c:734 [inline]
__sys_sendto+0x21e/0x2c0 net/socket.c:2119
__do_sys_sendto net/socket.c:2131 [inline]
__se_sys_sendto net/socket.c:2127 [inline]
__x64_sys_sendto+0x74/0x90 net/socket.c:2127
do_syscall_x64 arch/x86/entry/common.c:50 [inline]
do_syscall_64+0x2b/0x70 arch/x86/entry/common.c:80
entry_SYSCALL_64_after_hwframe+0x46/0xb0

read to 0xffff888144ecf518 of 1 bytes by task 20423 on cpu 1:
dev_isalive net/core/net-sysfs.c:38 [inline]
netdev_show net/core/net-sysfs.c:50 [inline]
type_show+0x24/0x90 net/core/net-sysfs.c:112
dev_attr_show+0x35/0x90 drivers/base/core.c:2095
sysfs_kf_seq_show+0x175/0x240 fs/sysfs/file.c:59
kernfs_seq_show+0x75/0x80 fs/kernfs/file.c:162
seq_read_iter+0x2c3/0x8e0 fs/seq_file.c:230
kernfs_fop_read_iter+0xd1/0x2f0 fs/kernfs/file.c:235
call_read_iter include/linux/fs.h:2052 [inline]
new_sync_read fs/read_write.c:401 [inline]
vfs_read+0x5a5/0x6a0 fs/read_write.c:482
ksys_read+0xe8/0x1a0 fs/read_write.c:620
__do_sys_read fs/read_write.c:630 [inline]
__se_sys_read fs/read_write.c:628 [inline]
__x64_sys_read+0x3e/0x50 fs/read_write.c:628
do_syscall_x64 arch/x86/entry/common.c:50 [inline]
do_syscall_64+0x2b/0x70 arch/x86/entry/common.c:80
entry_SYSCALL_64_after_hwframe+0x46/0xb0

value changed: 0x00 -> 0x01

Reported by Kernel Concurrency Sanitizer on:
CPU: 1 PID: 20423 Comm: udevd Tainted: G W 5.19.0-rc2-syzkaller-dirty #0
Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 01/01/2011

Fixes: 1da177e4c3f4 ("Linux-2.6.12-rc2")
Signed-off-by: Eric Dumazet <edumazet@google.com>
Reported-by: syzbot <syzkaller@googlegroups.com>
Signed-off-by: David S. Miller <davem@davemloft.net>
```

## CVE / bug description

```
KCSAN: data-race in register_netdevice / type_show (2)
```

## Full mainline patch

```diff
commit cc26c2661fefea215f41edb665193324a5f99021
Author: Eric Dumazet <edumazet@google.com>
Date:   Thu Jun 16 00:34:34 2022 -0700

    net: fix data-race in dev_isalive()
    
    dev_isalive() is called under RTNL or dev_base_lock protection.
    
    This means that changes to dev->reg_state should be done with both locks held.
    
    syzbot reported:
    
    BUG: KCSAN: data-race in register_netdevice / type_show
    
    write to 0xffff888144ecf518 of 1 bytes by task 20886 on cpu 0:
    register_netdevice+0xb9f/0xdf0 net/core/dev.c:10050
    lapbeth_new_device drivers/net/wan/lapbether.c:414 [inline]
    lapbeth_device_event+0x4a0/0x6c0 drivers/net/wan/lapbether.c:456
    notifier_call_chain kernel/notifier.c:87 [inline]
    raw_notifier_call_chain+0x53/0xb0 kernel/notifier.c:455
    __dev_notify_flags+0x1d6/0x3a0
    dev_change_flags+0xa2/0xc0 net/core/dev.c:8607
    do_setlink+0x778/0x2230 net/core/rtnetlink.c:2780
    __rtnl_newlink net/core/rtnetlink.c:3546 [inline]
    rtnl_newlink+0x114c/0x16a0 net/core/rtnetlink.c:3593
    rtnetlink_rcv_msg+0x811/0x8c0 net/core/rtnetlink.c:6089
    netlink_rcv_skb+0x13e/0x240 net/netlink/af_netlink.c:2501
    rtnetlink_rcv+0x18/0x20 net/core/rtnetlink.c:6107
    netlink_unicast_kernel net/netlink/af_netlink.c:1319 [inline]
    netlink_unicast+0x58a/0x660 net/netlink/af_netlink.c:1345
    netlink_sendmsg+0x661/0x750 net/netlink/af_netlink.c:1921
    sock_sendmsg_nosec net/socket.c:714 [inline]
    sock_sendmsg net/socket.c:734 [inline]
    __sys_sendto+0x21e/0x2c0 net/socket.c:2119
    __do_sys_sendto net/socket.c:2131 [inline]
    __se_sys_sendto net/socket.c:2127 [inline]
    __x64_sys_sendto+0x74/0x90 net/socket.c:2127
    do_syscall_x64 arch/x86/entry/common.c:50 [inline]
    do_syscall_64+0x2b/0x70 arch/x86/entry/common.c:80
    entry_SYSCALL_64_after_hwframe+0x46/0xb0
    
    read to 0xffff888144ecf518 of 1 bytes by task 20423 on cpu 1:
    dev_isalive net/core/net-sysfs.c:38 [inline]
    netdev_show net/core/net-sysfs.c:50 [inline]
    type_show+0x24/0x90 net/core/net-sysfs.c:112
    dev_attr_show+0x35/0x90 drivers/base/core.c:2095
    sysfs_kf_seq_show+0x175/0x240 fs/sysfs/file.c:59
    kernfs_seq_show+0x75/0x80 fs/kernfs/file.c:162
    seq_read_iter+0x2c3/0x8e0 fs/seq_file.c:230
    kernfs_fop_read_iter+0xd1/0x2f0 fs/kernfs/file.c:235
    call_read_iter include/linux/fs.h:2052 [inline]
    new_sync_read fs/read_write.c:401 [inline]
    vfs_read+0x5a5/0x6a0 fs/read_write.c:482
    ksys_read+0xe8/0x1a0 fs/read_write.c:620
    __do_sys_read fs/read_write.c:630 [inline]
    __se_sys_read fs/read_write.c:628 [inline]
    __x64_sys_read+0x3e/0x50 fs/read_write.c:628
    do_syscall_x64 arch/x86/entry/common.c:50 [inline]
    do_syscall_64+0x2b/0x70 arch/x86/entry/common.c:80
    entry_SYSCALL_64_after_hwframe+0x46/0xb0
    
    value changed: 0x00 -> 0x01
    
    Reported by Kernel Concurrency Sanitizer on:
    CPU: 1 PID: 20423 Comm: udevd Tainted: G W 5.19.0-rc2-syzkaller-dirty #0
    Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 01/01/2011
    
    Fixes: 1da177e4c3f4 ("Linux-2.6.12-rc2")
    Signed-off-by: Eric Dumazet <edumazet@google.com>
    Reported-by: syzbot <syzkaller@googlegroups.com>
    Signed-off-by: David S. Miller <davem@davemloft.net>

diff --git a/net/core/dev.c b/net/core/dev.c
index 08ce317fcec8..8e6f22961206 100644
--- a/net/core/dev.c
+++ b/net/core/dev.c
@@ -397,16 +397,18 @@ static void list_netdevice(struct net_device *dev)
 /* Device list removal
  * caller must respect a RCU grace period before freeing/reusing dev
  */
-static void unlist_netdevice(struct net_device *dev)
+static void unlist_netdevice(struct net_device *dev, bool lock)
 {
 	ASSERT_RTNL();
 
 	/* Unlink dev from the device chain */
-	write_lock(&dev_base_lock);
+	if (lock)
+		write_lock(&dev_base_lock);
 	list_del_rcu(&dev->dev_list);
 	netdev_name_node_del(dev->name_node);
 	hlist_del_rcu(&dev->index_hlist);
-	write_unlock(&dev_base_lock);
+	if (lock)
+		write_unlock(&dev_base_lock);
 
 	dev_base_seq_inc(dev_net(dev));
 }
@@ -10043,11 +10045,11 @@ int register_netdevice(struct net_device *dev)
 		goto err_uninit;
 
 	ret = netdev_register_kobject(dev);
-	if (ret) {
-		dev->reg_state = NETREG_UNREGISTERED;
+	write_lock(&dev_base_lock);
+	dev->reg_state = ret ? NETREG_UNREGISTERED : NETREG_REGISTERED;
+	write_unlock(&dev_base_lock);
+	if (ret)
 		goto err_uninit;
-	}
-	dev->reg_state = NETREG_REGISTERED;
 
 	__netdev_update_features(dev);
 
@@ -10329,7 +10331,9 @@ void netdev_run_todo(void)
 			continue;
 		}
 
+		write_lock(&dev_base_lock);
 		dev->reg_state = NETREG_UNREGISTERED;
+		write_unlock(&dev_base_lock);
 		linkwatch_forget_dev(dev);
 	}
 
@@ -10810,9 +10814,10 @@ void unregister_netdevice_many(struct list_head *head)
 
 	list_for_each_entry(dev, head, unreg_list) {
 		/* And unlink it from device chain. */
-		unlist_netdevice(dev);
-
+		write_lock(&dev_base_lock);
+		unlist_netdevice(dev, false);
 		dev->reg_state = NETREG_UNREGISTERING;
+		write_unlock(&dev_base_lock);
 	}
 	flush_all_backlogs();
 
@@ -10959,7 +10964,7 @@ int __dev_change_net_namespace(struct net_device *dev, struct net *net,
 	dev_close(dev);
 
 	/* And unlink it from device chain */
-	unlist_netdevice(dev);
+	unlist_netdevice(dev, true);
 
 	synchronize_net();
 
diff --git a/net/core/net-sysfs.c b/net/core/net-sysfs.c
index e319e242dddf..a3642569fe53 100644
--- a/net/core/net-sysfs.c
+++ b/net/core/net-sysfs.c
@@ -33,6 +33,7 @@ static const char fmt_dec[] = "%d\n";
 static const char fmt_ulong[] = "%lu\n";
 static const char fmt_u64[] = "%llu\n";
 
+/* Caller holds RTNL or dev_base_lock */
 static inline int dev_isalive(const struct net_device *dev)
 {
 	return dev->reg_state <= NETREG_REGISTERED;

```

## File: `net/core/dev.c`

- changed old-lines: [400, 405, 409, 10046, 10047, 10049, 10050, 10813, 10814, 10962]

### Function `unlist_netdevice` (L400–L412, 13 lines)

```c
static void unlist_netdevice(struct net_device *dev)
{
	ASSERT_RTNL();

	/* Unlink dev from the device chain */
	write_lock(&dev_base_lock);
	list_del_rcu(&dev->dev_list);
	netdev_name_node_del(dev->name_node);
	hlist_del_rcu(&dev->index_hlist);
	write_unlock(&dev_base_lock);

	dev_base_seq_inc(dev_net(dev));
}
```

_(no external call sites found for `unlist_netdevice`)_

### Function `register_netdevice` (L9941–L10105, 165 lines)

```c
int register_netdevice(struct net_device *dev)
{
	int ret;
	struct net *net = dev_net(dev);

	BUILD_BUG_ON(sizeof(netdev_features_t) * BITS_PER_BYTE <
		     NETDEV_FEATURE_COUNT);
	BUG_ON(dev_boot_phase);
	ASSERT_RTNL();

	might_sleep();

	/* When net_device's are persistent, this will be fatal. */
	BUG_ON(dev->reg_state != NETREG_UNINITIALIZED);
	BUG_ON(!net);

	ret = ethtool_check_ops(dev->ethtool_ops);
	if (ret)
		return ret;

	spin_lock_init(&dev->addr_list_lock);
	netdev_set_addr_lockdep_class(dev);

	ret = dev_get_valid_name(net, dev, dev->name);
	if (ret < 0)
		goto out;

	ret = -ENOMEM;
	dev->name_node = netdev_name_node_head_alloc(dev);
	if (!dev->name_node)
		goto out;

	/* Init, if this function is available */
	if (dev->netdev_ops->ndo_init) {
		ret = dev->netdev_ops->ndo_init(dev);
		if (ret) {
			if (ret > 0)
				ret = -EIO;
			goto err_free_name;
		}
	}

	if (((dev->hw_features | dev->features) &
	     NETIF_F_HW_VLAN_CTAG_FILTER) &&
	    (!dev->netdev_ops->ndo_vlan_rx_add_vid ||
	     !dev->netdev_ops->ndo_vlan_rx_kill_vid)) {
		netdev_WARN(dev, "Buggy VLAN acceleration in driver!\n");
		ret = -EINVAL;
		goto err_uninit;
	}

	ret = -EBUSY;
	if (!dev->ifindex)
		dev->ifindex = dev_new_index(net);
	else if (__dev_get_by_index(net, dev->ifindex))
		goto err_uninit;

	/* Transfer changeable features to wanted_features and enable
	 * software offloads (GSO and GRO).
	 */
	dev->hw_features |= (NETIF_F_SOFT_FEATURES | NETIF_F_SOFT_FEATURES_OFF);
	dev->features |= NETIF_F_SOFT_FEATURES;

	if (dev->udp_tunnel_nic_info) {
		dev->features |= NETIF_F_RX_UDP_TUNNEL_PORT;
		dev->hw_features |= NETIF_F_RX_UDP_TUNNEL_PORT;
	}

	dev->wanted_features = dev->features & dev->hw_features;

	if (!(dev->flags & IFF_LOOPBACK))
		dev->hw_features |= NETIF_F_NOCACHE_COPY;

	/* If IPv4 TCP segmentation offload is supported we should also
	 * allow the device to enable segmenting the frame with the option
	 * of ignoring a static IP ID value.  This doesn't enable the
	 * feature itself but allows the user to enable it later.
	 */
	if (dev->hw_features & NETIF_F_TSO)
		dev->hw_features |= NETIF_F_TSO_MANGLEID;
	if (dev->vlan_features & NETIF_F_TSO)
		dev->vlan_features |= NETIF_F_TSO_MANGLEID;
	if (dev->mpls_features & NETIF_F_TSO)
		dev->mpls_features |= NETIF_F_TSO_MANGLEID;
	if (dev->hw_enc_features & NETIF_F_TSO)
		dev->hw_enc_features |= NETIF_F_TSO_MANGLEID;

	/* Make NETIF_F_HIGHDMA inheritable to VLAN devices.
	 */
	dev->vlan_features |= NETIF_F_HIGHDMA;

	/* Make NETIF_F_SG inheritable to tunnel devices.
	 */
	dev->hw_enc_features |= NETIF_F_SG | NETIF_F_GSO_PARTIAL;

	/* Make NETIF_F_SG inheritable to MPLS.
	 */
	dev->mpls_features |= NETIF_F_SG;

	ret = call_netdevice_notifiers(NETDEV_POST_INIT, dev);
	ret = notifier_to_errno(ret);
	if (ret)
		goto err_uninit;

	ret = netdev_register_kobject(dev);
	if (ret) {
		dev->reg_state = NETREG_UNREGISTERED;
		goto err_uninit;
	}
	dev->reg_state = NETREG_REGISTERED;

	__netdev_update_features(dev);

	/*
	 *	Default initial state at registry is that the
	 *	device is present.
	 */

	set_bit(__LINK_STATE_PRESENT, &dev->state);

	linkwatch_init_dev(dev);

	dev_init_scheduler(dev);

	dev_hold_track(dev, &dev->dev_registered_tracker, GFP_KERNEL);
	list_netdevice(dev);

	add_device_randomness(dev->dev_addr, dev->addr_len);

	/* If the device has permanent device address, driver should
	 * set dev_addr and also addr_assign_type should be set to
	 * NET_ADDR_PERM (default value).
	 */
	if (dev->addr_assign_type == NET_ADDR_PERM)
		memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len);

	/* Notify protocols, that a new device appeared. */
	ret = call_netdevice_notifiers(NETDEV_REGISTER, dev);
	ret = notifier_to_errno(ret);
	if (ret) {
		/* Expect explicit free_netdev() on failure */
		dev->needs_free_netdev = false;
		unregister_netdevice_queue(dev, NULL);
		goto out;
	}
	/*
	 *	Prevent userspace races by waiting until the network
	 *	device is fully setup before sending notifications.
	 */
	if (!dev->rtnl_link_ops ||
	    dev->rtnl_link_state == RTNL_LINK_INITIALIZED)
		rtmsg_ifinfo(RTM_NEWLINK, dev, ~0U, GFP_KERNEL);

out:
	return ret;

err_uninit:
	if (dev->netdev_ops->ndo_uninit)
		dev->netdev_ops->ndo_uninit(dev);
	if (dev->priv_destructor)
		dev->priv_destructor(dev);
err_free_name:
	netdev_name_node_free(dev->name_node);
	goto out;
}
```

**External callers of `register_netdevice` at parent**

- `arch/um/drivers/net_kern.c:466` — `	err = register_netdevice(dev);`
- `arch/um/drivers/vector_kern.c:1645` — `	err = register_netdevice(dev);`
- `arch/xtensa/platforms/iss/network.c:539` — `	err = register_netdevice(dev);`
- `drivers/infiniband/ulp/ipoib/ipoib_main.c:2246` — `	 * register_netdevice succeed'd and priv_destructor is set to`
- `drivers/infiniband/ulp/ipoib/ipoib_vlan.c:104` — `	 * We do not need to touch priv if register_netdevice fails, so just`
- `drivers/infiniband/ulp/ipoib/ipoib_vlan.c:131` — `	result = register_netdevice(ndev);`
- `drivers/infiniband/ulp/ipoib/ipoib_vlan.c:136` — `		 * register_netdevice sometimes calls priv_destructor,`
- `drivers/net/amt.c:3134` — `	err = register_netdevice(dev);`
- `drivers/net/bareudp.c:640` — `	err = register_netdevice(dev);`
- `drivers/net/bonding/bond_main.c:6246` — `	res = register_netdevice(bond_dev);`
- `drivers/net/bonding/bond_netlink.c:510` — `	err = register_netdevice(bond_dev);`
- `drivers/net/caif/caif_serial.c:351` — `	result = register_netdevice(dev);`
- `drivers/net/can/slcan.c:612` — `		err = register_netdevice(sl->dev);`
- `drivers/net/can/vxcan.c:221` — `	err = register_netdevice(peer);`
- `drivers/net/can/vxcan.c:241` — `	err = register_netdevice(dev);`
- `drivers/net/dummy.c:170` — `	err = register_netdevice(dev_dummy);`
- `drivers/net/ethernet/freescale/dpaa2/dpaa2-eth.c:4317` — `		 * register_netdevice() to properly fill up net_dev->perm_addr.`
- `drivers/net/ethernet/freescale/dpaa2/dpaa2-switch.c:998` — `		 * register_netdevice() to properly fill up net_dev->perm_addr.`
- `drivers/net/ethernet/intel/iavf/iavf_main.c:2476` — `		err = register_netdevice(netdev);`
- `drivers/net/ethernet/natsemi/ns83820.c:2173` — `	err = register_netdevice(ndev);`

### Function `unregister_netdevice_many` (L10778–L10876, 99 lines)

```c
void unregister_netdevice_many(struct list_head *head)
{
	struct net_device *dev, *tmp;
	LIST_HEAD(close_head);

	BUG_ON(dev_boot_phase);
	ASSERT_RTNL();

	if (list_empty(head))
		return;

	list_for_each_entry_safe(dev, tmp, head, unreg_list) {
		/* Some devices call without registering
		 * for initialization unwind. Remove those
		 * devices and proceed with the remaining.
		 */
		if (dev->reg_state == NETREG_UNINITIALIZED) {
			pr_debug("unregister_netdevice: device %s/%p never was registered\n",
				 dev->name, dev);

			WARN_ON(1);
			list_del(&dev->unreg_list);
			continue;
		}
		dev->dismantle = true;
		BUG_ON(dev->reg_state != NETREG_REGISTERED);
	}

	/* If device is running, close it first. */
	list_for_each_entry(dev, head, unreg_list)
		list_add_tail(&dev->close_list, &close_head);
	dev_close_many(&close_head, true);

	list_for_each_entry(dev, head, unreg_list) {
		/* And unlink it from device chain. */
		unlist_netdevice(dev);

		dev->reg_state = NETREG_UNREGISTERING;
	}
	flush_all_backlogs();

	synchronize_net();

	list_for_each_entry(dev, head, unreg_list) {
		struct sk_buff *skb = NULL;

		/* Shutdown queueing discipline. */
		dev_shutdown(dev);

		dev_xdp_uninstall(dev);

		netdev_offload_xstats_disable_all(dev);

		/* Notify protocols, that we are about to destroy
		 * this device. They should clean all the things.
		 */
		call_netdevice_notifiers(NETDEV_UNREGISTER, dev);

		if (!dev->rtnl_link_ops ||
		    dev->rtnl_link_state == RTNL_LINK_INITIALIZED)
			skb = rtmsg_ifinfo_build_skb(RTM_DELLINK, dev, ~0U, 0,
						     GFP_KERNEL, NULL, 0);

		/*
		 *	Flush the unicast and multicast chains
		 */
		dev_uc_flush(dev);
		dev_mc_flush(dev);

		netdev_name_node_alt_flush(dev);
		netdev_name_node_free(dev->name_node);

		if (dev->netdev_ops->ndo_uninit)
			dev->netdev_ops->ndo_uninit(dev);

		if (skb)
			rtmsg_ifinfo_send(skb, dev, GFP_KERNEL);

		/* Notifier chain MUST detach us all upper devices. */
		WARN_ON(netdev_has_any_upper_dev(dev));
		WARN_ON(netdev_has_any_lower_dev(dev));

		/* Remove entries from kobject tree */
		netdev_unregister_kobject(dev);
#ifdef CONFIG_XPS
		/* Remove XPS queueing entries */
		netif_reset_xps_queues_gt(dev, 0);
#endif
	}

	synchronize_net();

	list_for_each_entry(dev, head, unreg_list) {
		dev_put_track(dev, &dev->dev_registered_tracker);
		net_set_todo(dev);
	}

	list_del(head);
}
```

**External callers of `unregister_netdevice_many` at parent**

- `drivers/infiniband/ulp/ipoib/ipoib_main.c:2593` — `		unregister_netdevice_many(&head);`
- `drivers/net/amt.c:3252` — `		unregister_netdevice_many(&list);`
- `drivers/net/bareudp.c:764` — `	unregister_netdevice_many(&list);`
- `drivers/net/bonding/bond_main.c:6295` — `	unregister_netdevice_many(&list);`
- `drivers/net/ethernet/qualcomm/rmnet/rmnet_config.c:238` — `		unregister_netdevice_many(&list);`
- `drivers/net/geneve.c:1916` — `	unregister_netdevice_many(&list_kill);`
- `drivers/net/geneve.c:1978` — `	unregister_netdevice_many(&list);`
- `drivers/net/gtp.c:1884` — `	unregister_netdevice_many(&list);`
- `drivers/net/ipvlan/ipvlan_main.c:759` — `		unregister_netdevice_many(&lst_kill);`
- `drivers/net/macsec.c:4319` — `		unregister_netdevice_many(&head);`
- `drivers/net/macvlan.c:1796` — `		unregister_netdevice_many(&list_kill);`
- `drivers/net/ppp/ppp_generic.c:1140` — `	unregister_netdevice_many(&list);`
- `drivers/net/usb/qmi_wwan.c:1563` — `		unregister_netdevice_many(&list);`
- `drivers/net/vxlan/vxlan_core.c:4424` — `		unregister_netdevice_many(&list_kill);`
- `drivers/net/vxlan/vxlan_core.c:4451` — `	unregister_netdevice_many(&list_kill);`
- `drivers/net/vxlan/vxlan_core.c:4689` — `	unregister_netdevice_many(&list);`
- `drivers/net/wireless/virt_wifi.c:641` — `		unregister_netdevice_many(&list_kill);`
- `drivers/net/wwan/qcom_bam_dmux.c:862` — `	unregister_netdevice_many(&list);`
- `drivers/net/wwan/wwan_core.c:1153` — `	unregister_netdevice_many(&kill_list);`
- `include/linux/netdevice.h:3015` — `void unregister_netdevice_many(struct list_head *head);`

### Function `__dev_change_net_namespace` (L10914–L11038, 125 lines)

```c
int __dev_change_net_namespace(struct net_device *dev, struct net *net,
			       const char *pat, int new_ifindex)
{
	struct net *net_old = dev_net(dev);
	int err, new_nsid;

	ASSERT_RTNL();

	/* Don't allow namespace local devices to be moved. */
	err = -EINVAL;
	if (dev->features & NETIF_F_NETNS_LOCAL)
		goto out;

	/* Ensure the device has been registrered */
	if (dev->reg_state != NETREG_REGISTERED)
		goto out;

	/* Get out if there is nothing todo */
	err = 0;
	if (net_eq(net_old, net))
		goto out;

	/* Pick the destination device name, and ensure
	 * we can use it in the destination network namespace.
	 */
	err = -EEXIST;
	if (netdev_name_in_use(net, dev->name)) {
		/* We get here if we can't use the current device name */
		if (!pat)
			goto out;
		err = dev_get_valid_name(net, dev, pat);
		if (err < 0)
			goto out;
	}

	/* Check that new_ifindex isn't used yet. */
	err = -EBUSY;
	if (new_ifindex && __dev_get_by_index(net, new_ifindex))
		goto out;

	/*
	 * And now a mini version of register_netdevice unregister_netdevice.
	 */

	/* If device is running close it first. */
	dev_close(dev);

	/* And unlink it from device chain */
	unlist_netdevice(dev);

	synchronize_net();

	/* Shutdown queueing discipline. */
	dev_shutdown(dev);

	/* Notify protocols, that we are about to destroy
	 * this device. They should clean all the things.
	 *
	 * Note that dev->reg_state stays at NETREG_REGISTERED.
	 * This is wanted because this way 8021q and macvlan know
	 * the device is just moving and can keep their slaves up.
	 */
	call_netdevice_notifiers(NETDEV_UNREGISTER, dev);
	rcu_barrier();

	new_nsid = peernet2id_alloc(dev_net(dev), net, GFP_KERNEL);
	/* If there is an ifindex conflict assign a new one */
	if (!new_ifindex) {
		if (__dev_get_by_index(net, dev->ifindex))
			new_ifindex = dev_new_index(net);
		else
			new_ifindex = dev->ifindex;
	}

	rtmsg_ifinfo_newnet(RTM_DELLINK, dev, ~0U, GFP_KERNEL, &new_nsid,
			    new_ifindex);

	/*
	 *	Flush the unicast and multicast chains
	 */
	dev_uc_flush(dev);
	dev_mc_flush(dev);

	/* Send a netdev-removed uevent to the old namespace */
	kobject_uevent(&dev->dev.kobj, KOBJ_REMOVE);
	netdev_adjacent_del_links(dev);

	/* Move per-net netdevice notifiers that are following the netdevice */
	move_netdevice_notifiers_dev_net(dev, net);

	/* Actually switch the network namespace */
	dev_net_set(dev, net);
	dev->ifindex = new_ifindex;

	/* Send a netdev-add uevent to the new namespace */
	kobject_uevent(&dev->dev.kobj, KOBJ_ADD);
	netdev_adjacent_add_links(dev);

	/* Fixup kobjects */
	err = device_rename(&dev->dev, dev->name);
	WARN_ON(err);

	/* Adapt owner in case owning user namespace of target network
	 * namespace is different from the original one.
	 */
	err = netdev_change_owner(dev, net_old, net);
	WARN_ON(err);

	/* Add the device back in the hashes */
	list_netdevice(dev);

	/* Notify protocols, that a new device appeared. */
	call_netdevice_notifiers(NETDEV_REGISTER, dev);

	/*
	 *	Prevent userspace races by waiting until the network
	 *	device is fully setup before sending notifications.
	 */
	rtmsg_ifinfo(RTM_NEWLINK, dev, ~0U, GFP_KERNEL);

	synchronize_net();
	err = 0;
out:
	return err;
}
```

**External callers of `__dev_change_net_namespace` at parent**

- `include/linux/netdevice.h:3827` — `int __dev_change_net_namespace(struct net_device *dev, struct net *net,`
- `include/linux/netdevice.h:3833` — `	return __dev_change_net_namespace(dev, net, pat, 0);`
- `net/core/rtnetlink.c:2685` — `		err = __dev_change_net_namespace(dev, net, pat, new_ifindex);`

## File: `net/core/net-sysfs.c`

- changed old-lines: []
