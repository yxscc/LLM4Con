# Evidence: SYZBOT-00f7f5b884b117ee  (tier B, score 100)

- source archive: **syzbot**
- title: possible deadlock in __nilfs_error (3)
- mainline fix commit: `fb881cd7604536b17a1927fb0533f9a6982ffcc5`
- candidate JSON: `B_SYZBOT-00f7f5b884b117ee.json`
- thread_hint: `{"kind": "deadlock", "raw_title": "possible deadlock in __nilfs_error (3)"}`

## Commit message

```
fb881cd7604536b17a1927fb0533f9a6982ffcc5
nilfs2: fix deadlock warnings caused by lock dependency in init_nilfs()

After commit c0e473a0d226 ("block: fix race between set_blocksize and read
paths") was merged, set_blocksize() called by sb_set_blocksize() now locks
the inode of the backing device file.  As a result of this change, syzbot
started reporting deadlock warnings due to a circular dependency involving
the semaphore "ns_sem" of the nilfs object, the inode lock of the backing
device file, and the locks that this inode lock is transitively dependent
on.

This is caused by a new lock dependency added by the above change, since
init_nilfs() calls sb_set_blocksize() in the lock section of "ns_sem". 
However, these warnings are false positives because init_nilfs() is called
in the early stage of the mount operation and the filesystem has not yet
started.

The reason why "ns_sem" is locked in init_nilfs() was to avoid a race
condition in nilfs_fill_super() caused by sharing a nilfs object among
multiple filesystem instances (super block structures) in the early
implementation.  However, nilfs objects and super block structures have
long ago become one-to-one, and there is no longer any need to use the
semaphore there.

So, fix this issue by removing the use of the semaphore "ns_sem" in
init_nilfs().

Link: https://lkml.kernel.org/r/20250503053327.12294-1-konishi.ryusuke@gmail.com
Fixes: c0e473a0d226 ("block: fix race between set_blocksize and read paths")
Signed-off-by: Ryusuke Konishi <konishi.ryusuke@gmail.com>
Reported-by: syzbot+00f7f5b884b117ee6773@syzkaller.appspotmail.com
Closes: https://syzkaller.appspot.com/bug?extid=00f7f5b884b117ee6773
Tested-by: syzbot+00f7f5b884b117ee6773@syzkaller.appspotmail.com
Reported-by: syzbot+f30591e72bfc24d4715b@syzkaller.appspotmail.com
Closes: https://syzkaller.appspot.com/bug?extid=f30591e72bfc24d4715b
Tested-by: syzbot+f30591e72bfc24d4715b@syzkaller.appspotmail.com>
Signed-off-by: Andrew Morton <akpm@linux-foundation.org>
```

## CVE / bug description

```
possible deadlock in __nilfs_error (3)
```

## Full mainline patch

```diff
commit fb881cd7604536b17a1927fb0533f9a6982ffcc5
Author: Ryusuke Konishi <konishi.ryusuke@gmail.com>
Date:   Sat May 3 14:33:14 2025 +0900

    nilfs2: fix deadlock warnings caused by lock dependency in init_nilfs()
    
    After commit c0e473a0d226 ("block: fix race between set_blocksize and read
    paths") was merged, set_blocksize() called by sb_set_blocksize() now locks
    the inode of the backing device file.  As a result of this change, syzbot
    started reporting deadlock warnings due to a circular dependency involving
    the semaphore "ns_sem" of the nilfs object, the inode lock of the backing
    device file, and the locks that this inode lock is transitively dependent
    on.
    
    This is caused by a new lock dependency added by the above change, since
    init_nilfs() calls sb_set_blocksize() in the lock section of "ns_sem".
    However, these warnings are false positives because init_nilfs() is called
    in the early stage of the mount operation and the filesystem has not yet
    started.
    
    The reason why "ns_sem" is locked in init_nilfs() was to avoid a race
    condition in nilfs_fill_super() caused by sharing a nilfs object among
    multiple filesystem instances (super block structures) in the early
    implementation.  However, nilfs objects and super block structures have
    long ago become one-to-one, and there is no longer any need to use the
    semaphore there.
    
    So, fix this issue by removing the use of the semaphore "ns_sem" in
    init_nilfs().
    
    Link: https://lkml.kernel.org/r/20250503053327.12294-1-konishi.ryusuke@gmail.com
    Fixes: c0e473a0d226 ("block: fix race between set_blocksize and read paths")
    Signed-off-by: Ryusuke Konishi <konishi.ryusuke@gmail.com>
    Reported-by: syzbot+00f7f5b884b117ee6773@syzkaller.appspotmail.com
    Closes: https://syzkaller.appspot.com/bug?extid=00f7f5b884b117ee6773
    Tested-by: syzbot+00f7f5b884b117ee6773@syzkaller.appspotmail.com
    Reported-by: syzbot+f30591e72bfc24d4715b@syzkaller.appspotmail.com
    Closes: https://syzkaller.appspot.com/bug?extid=f30591e72bfc24d4715b
    Tested-by: syzbot+f30591e72bfc24d4715b@syzkaller.appspotmail.com>
    Signed-off-by: Andrew Morton <akpm@linux-foundation.org>

diff --git a/fs/nilfs2/the_nilfs.c b/fs/nilfs2/the_nilfs.c
index cb01ea81724d..d0bcf744c553 100644
--- a/fs/nilfs2/the_nilfs.c
+++ b/fs/nilfs2/the_nilfs.c
@@ -705,8 +705,6 @@ int init_nilfs(struct the_nilfs *nilfs, struct super_block *sb)
 	int blocksize;
 	int err;
 
-	down_write(&nilfs->ns_sem);
-
 	blocksize = sb_min_blocksize(sb, NILFS_MIN_BLOCK_SIZE);
 	if (!blocksize) {
 		nilfs_err(sb, "unable to set blocksize");
@@ -779,7 +777,6 @@ int init_nilfs(struct the_nilfs *nilfs, struct super_block *sb)
 	set_nilfs_init(nilfs);
 	err = 0;
  out:
-	up_write(&nilfs->ns_sem);
 	return err;
 
  failed_sbh:

```

## File: `fs/nilfs2/the_nilfs.c`

- changed old-lines: [708, 709, 782]

### Function `init_nilfs` (L702–L788, 87 lines)

```c
int init_nilfs(struct the_nilfs *nilfs, struct super_block *sb)
{
	struct nilfs_super_block *sbp;
	int blocksize;
	int err;

	down_write(&nilfs->ns_sem);

	blocksize = sb_min_blocksize(sb, NILFS_MIN_BLOCK_SIZE);
	if (!blocksize) {
		nilfs_err(sb, "unable to set blocksize");
		err = -EINVAL;
		goto out;
	}
	err = nilfs_load_super_block(nilfs, sb, blocksize, &sbp);
	if (err)
		goto out;

	err = nilfs_store_magic(sb, sbp);
	if (err)
		goto failed_sbh;

	err = nilfs_check_feature_compatibility(sb, sbp);
	if (err)
		goto failed_sbh;

	err = nilfs_get_blocksize(sb, sbp, &blocksize);
	if (err)
		goto failed_sbh;

	if (blocksize < NILFS_MIN_BLOCK_SIZE) {
		nilfs_err(sb,
			  "couldn't mount because of unsupported filesystem blocksize %d",
			  blocksize);
		err = -EINVAL;
		goto failed_sbh;
	}
	if (sb->s_blocksize != blocksize) {
		int hw_blocksize = bdev_logical_block_size(sb->s_bdev);

		if (blocksize < hw_blocksize) {
			nilfs_err(sb,
				  "blocksize %d too small for device (sector-size = %d)",
				  blocksize, hw_blocksize);
			err = -EINVAL;
			goto failed_sbh;
		}
		nilfs_release_super_block(nilfs);
		if (!sb_set_blocksize(sb, blocksize)) {
			nilfs_err(sb, "bad blocksize %d", blocksize);
			err = -EINVAL;
			goto out;
		}

		err = nilfs_load_super_block(nilfs, sb, blocksize, &sbp);
		if (err)
			goto out;
			/*
			 * Not to failed_sbh; sbh is released automatically
			 * when reloading fails.
			 */
	}
	nilfs->ns_blocksize_bits = sb->s_blocksize_bits;
	nilfs->ns_blocksize = blocksize;

	err = nilfs_store_disk_layout(nilfs, sbp);
	if (err)
		goto failed_sbh;

	sb->s_maxbytes = nilfs_max_size(sb->s_blocksize_bits);

	nilfs->ns_mount_state = le16_to_cpu(sbp->s_state);

	err = nilfs_store_log_cursor(nilfs, sbp);
	if (err)
		goto failed_sbh;

	set_nilfs_init(nilfs);
	err = 0;
 out:
	up_write(&nilfs->ns_sem);
	return err;

 failed_sbh:
	nilfs_release_super_block(nilfs);
	goto out;
}
```

**External callers of `init_nilfs` at parent**

- `fs/nilfs2/super.c:1060` — `	err = init_nilfs(nilfs, sb);`
- `fs/nilfs2/the_nilfs.h:269` — `int init_nilfs(struct the_nilfs *nilfs, struct super_block *sb);`
