# Evidence: SYZBOT-4a06d4373fd52f0b  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in inotify_handle_inode_event / inotify_remove_from_idr
- mainline fix commit: `c915d8f5918bea7c3962b09b8884ca128bfd9b0c`
- candidate JSON: `A_SYZBOT-4a06d4373fd52f0b.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "inotify_handle_inode_event", "fn_b": "inotify_remove_from_idr"}`

## Commit message

```
c915d8f5918bea7c3962b09b8884ca128bfd9b0c
inotify: Avoid reporting event with invalid wd

When inotify_freeing_mark() races with inotify_handle_inode_event() it
can happen that inotify_handle_inode_event() sees that i_mark->wd got
already reset to -1 and reports this value to userspace which can
confuse the inotify listener. Avoid the problem by validating that wd is
sensible (and pretend the mark got removed before the event got
generated otherwise).

CC: stable@vger.kernel.org
Fixes: 7e790dd5fc93 ("inotify: fix error paths in inotify_update_watch")
Message-Id: <20230424163219.9250-1-jack@suse.cz>
Reported-by: syzbot+4a06d4373fd52f0b2f9c@syzkaller.appspotmail.com
Reviewed-by: Amir Goldstein <amir73il@gmail.com>
Signed-off-by: Jan Kara <jack@suse.cz>
```

## CVE / bug description

```
KCSAN: data-race in inotify_handle_inode_event / inotify_remove_from_idr
```

## Full mainline patch

```diff
commit c915d8f5918bea7c3962b09b8884ca128bfd9b0c
Author: Jan Kara <jack@suse.cz>
Date:   Mon Apr 24 18:32:19 2023 +0200

    inotify: Avoid reporting event with invalid wd
    
    When inotify_freeing_mark() races with inotify_handle_inode_event() it
    can happen that inotify_handle_inode_event() sees that i_mark->wd got
    already reset to -1 and reports this value to userspace which can
    confuse the inotify listener. Avoid the problem by validating that wd is
    sensible (and pretend the mark got removed before the event got
    generated otherwise).
    
    CC: stable@vger.kernel.org
    Fixes: 7e790dd5fc93 ("inotify: fix error paths in inotify_update_watch")
    Message-Id: <20230424163219.9250-1-jack@suse.cz>
    Reported-by: syzbot+4a06d4373fd52f0b2f9c@syzkaller.appspotmail.com
    Reviewed-by: Amir Goldstein <amir73il@gmail.com>
    Signed-off-by: Jan Kara <jack@suse.cz>

diff --git a/fs/notify/inotify/inotify_fsnotify.c b/fs/notify/inotify/inotify_fsnotify.c
index 49cfe2ae6d23..993375f0db67 100644
--- a/fs/notify/inotify/inotify_fsnotify.c
+++ b/fs/notify/inotify/inotify_fsnotify.c
@@ -65,7 +65,7 @@ int inotify_handle_inode_event(struct fsnotify_mark *inode_mark, u32 mask,
 	struct fsnotify_event *fsn_event;
 	struct fsnotify_group *group = inode_mark->group;
 	int ret;
-	int len = 0;
+	int len = 0, wd;
 	int alloc_len = sizeof(struct inotify_event_info);
 	struct mem_cgroup *old_memcg;
 
@@ -80,6 +80,13 @@ int inotify_handle_inode_event(struct fsnotify_mark *inode_mark, u32 mask,
 	i_mark = container_of(inode_mark, struct inotify_inode_mark,
 			      fsn_mark);
 
+	/*
+	 * We can be racing with mark being detached. Don't report event with
+	 * invalid wd.
+	 */
+	wd = READ_ONCE(i_mark->wd);
+	if (wd == -1)
+		return 0;
 	/*
 	 * Whoever is interested in the event, pays for the allocation. Do not
 	 * trigger OOM killer in the target monitoring memcg as it may have
@@ -110,7 +117,7 @@ int inotify_handle_inode_event(struct fsnotify_mark *inode_mark, u32 mask,
 	fsn_event = &event->fse;
 	fsnotify_init_event(fsn_event);
 	event->mask = mask;
-	event->wd = i_mark->wd;
+	event->wd = wd;
 	event->sync_cookie = cookie;
 	event->name_len = len;
 	if (len)

```

## File: `fs/notify/inotify/inotify_fsnotify.c`

- changed old-lines: [68, 113]

### Function `inotify_handle_inode_event` (L59–L129, 71 lines)

```c
int inotify_handle_inode_event(struct fsnotify_mark *inode_mark, u32 mask,
			       struct inode *inode, struct inode *dir,
			       const struct qstr *name, u32 cookie)
{
	struct inotify_inode_mark *i_mark;
	struct inotify_event_info *event;
	struct fsnotify_event *fsn_event;
	struct fsnotify_group *group = inode_mark->group;
	int ret;
	int len = 0;
	int alloc_len = sizeof(struct inotify_event_info);
	struct mem_cgroup *old_memcg;

	if (name) {
		len = name->len;
		alloc_len += len + 1;
	}

	pr_debug("%s: group=%p mark=%p mask=%x\n", __func__, group, inode_mark,
		 mask);

	i_mark = container_of(inode_mark, struct inotify_inode_mark,
			      fsn_mark);

	/*
	 * Whoever is interested in the event, pays for the allocation. Do not
	 * trigger OOM killer in the target monitoring memcg as it may have
	 * security repercussion.
	 */
	old_memcg = set_active_memcg(group->memcg);
	event = kmalloc(alloc_len, GFP_KERNEL_ACCOUNT | __GFP_RETRY_MAYFAIL);
	set_active_memcg(old_memcg);

	if (unlikely(!event)) {
		/*
		 * Treat lost event due to ENOMEM the same way as queue
		 * overflow to let userspace know event was lost.
		 */
		fsnotify_queue_overflow(group);
		return -ENOMEM;
	}

	/*
	 * We now report FS_ISDIR flag with MOVE_SELF and DELETE_SELF events
	 * for fanotify. inotify never reported IN_ISDIR with those events.
	 * It looks like an oversight, but to avoid the risk of breaking
	 * existing inotify programs, mask the flag out from those events.
	 */
	if (mask & (IN_MOVE_SELF | IN_DELETE_SELF))
		mask &= ~IN_ISDIR;

	fsn_event = &event->fse;
	fsnotify_init_event(fsn_event);
	event->mask = mask;
	event->wd = i_mark->wd;
	event->sync_cookie = cookie;
	event->name_len = len;
	if (len)
		strcpy(event->name, name->name);

	ret = fsnotify_add_event(group, fsn_event, inotify_merge);
	if (ret) {
		/* Our event wasn't used in the end. Free it. */
		fsnotify_destroy_event(group, fsn_event);
	}

	if (inode_mark->flags & FSNOTIFY_MARK_FLAG_IN_ONESHOT)
		fsnotify_destroy_mark(inode_mark, group);

	return 0;
}
```

**External callers of `inotify_handle_inode_event` at parent**

- `fs/notify/inotify/inotify.h:46` — `extern int inotify_handle_inode_event(struct fsnotify_mark *inode_mark,`
- `fs/notify/inotify/inotify_user.c:527` — `	inotify_handle_inode_event(fsn_mark, FS_IN_IGNORED, NULL, NULL, NULL,`
