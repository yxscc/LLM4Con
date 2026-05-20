# Evidence: SYZBOT-011e4ea1da6692cf  (tier B, score 100)

- source archive: **syzbot**
- title: possible deadlock in pipe_write
- mainline fix commit: `055ca83559912f2cfd91c9441427bac4caf3c74e`
- candidate JSON: `B_SYZBOT-011e4ea1da6692cf.json`
- thread_hint: `{"kind": "deadlock", "raw_title": "possible deadlock in pipe_write"}`

## Commit message

```
055ca83559912f2cfd91c9441427bac4caf3c74e
fs/pipe: Fix lockdep false-positive in watchqueue pipe_write()

When you try to splice between a normal pipe and a notification pipe,
get_pipe_info(..., true) fails, so splice() falls back to treating the
notification pipe like a normal pipe - so we end up in
iter_file_splice_write(), which first locks the input pipe, then calls
vfs_iter_write(), which locks the output pipe.

Lockdep complains about that, because we're taking a pipe lock while
already holding another pipe lock.

I think this probably (?) can't actually lead to deadlocks, since you'd
need another way to nest locking a normal pipe into locking a
watch_queue pipe, but the lockdep annotations don't make that clear.

Bail out earlier in pipe_write() for notification pipes, before taking
the pipe lock.

Reported-and-tested-by: <syzbot+011e4ea1da6692cf881c@syzkaller.appspotmail.com>
Closes: https://syzkaller.appspot.com/bug?extid=011e4ea1da6692cf881c
Fixes: c73be61cede5 ("pipe: Add general notification queue support")
Signed-off-by: Jann Horn <jannh@google.com>
Link: https://lore.kernel.org/r/20231124150822.2121798-1-jannh@google.com
Signed-off-by: Christian Brauner <brauner@kernel.org>
```

## CVE / bug description

```
possible deadlock in pipe_write
```

## Full mainline patch

```diff
commit 055ca83559912f2cfd91c9441427bac4caf3c74e
Author: Jann Horn <jannh@google.com>
Date:   Fri Nov 24 16:08:22 2023 +0100

    fs/pipe: Fix lockdep false-positive in watchqueue pipe_write()
    
    When you try to splice between a normal pipe and a notification pipe,
    get_pipe_info(..., true) fails, so splice() falls back to treating the
    notification pipe like a normal pipe - so we end up in
    iter_file_splice_write(), which first locks the input pipe, then calls
    vfs_iter_write(), which locks the output pipe.
    
    Lockdep complains about that, because we're taking a pipe lock while
    already holding another pipe lock.
    
    I think this probably (?) can't actually lead to deadlocks, since you'd
    need another way to nest locking a normal pipe into locking a
    watch_queue pipe, but the lockdep annotations don't make that clear.
    
    Bail out earlier in pipe_write() for notification pipes, before taking
    the pipe lock.
    
    Reported-and-tested-by: <syzbot+011e4ea1da6692cf881c@syzkaller.appspotmail.com>
    Closes: https://syzkaller.appspot.com/bug?extid=011e4ea1da6692cf881c
    Fixes: c73be61cede5 ("pipe: Add general notification queue support")
    Signed-off-by: Jann Horn <jannh@google.com>
    Link: https://lore.kernel.org/r/20231124150822.2121798-1-jannh@google.com
    Signed-off-by: Christian Brauner <brauner@kernel.org>

diff --git a/fs/pipe.c b/fs/pipe.c
index 804a7d789452..226e7f66b590 100644
--- a/fs/pipe.c
+++ b/fs/pipe.c
@@ -446,6 +446,18 @@ pipe_write(struct kiocb *iocb, struct iov_iter *from)
 	bool was_empty = false;
 	bool wake_next_writer = false;
 
+	/*
+	 * Reject writing to watch queue pipes before the point where we lock
+	 * the pipe.
+	 * Otherwise, lockdep would be unhappy if the caller already has another
+	 * pipe locked.
+	 * If we had to support locking a normal pipe and a notification pipe at
+	 * the same time, we could set up lockdep annotations for that, but
+	 * since we don't actually need that, it's simpler to just bail here.
+	 */
+	if (pipe_has_watch_queue(pipe))
+		return -EXDEV;
+
 	/* Null write succeeds. */
 	if (unlikely(total_len == 0))
 		return 0;
@@ -458,11 +470,6 @@ pipe_write(struct kiocb *iocb, struct iov_iter *from)
 		goto out;
 	}
 
-	if (pipe_has_watch_queue(pipe)) {
-		ret = -EXDEV;
-		goto out;
-	}
-
 	/*
 	 * If it wasn't empty we try to merge new data into
 	 * the last buffer.

```

## File: `fs/pipe.c`

- changed old-lines: [461, 462, 463, 464, 465]

### Function `if` (L461–L464, 4 lines)

```c
	if (pipe_has_watch_queue(pipe)) {
		ret = -EXDEV;
		goto out;
	}
```

**External callers of `if` at parent**

- `Documentation/gpu/rfc/i915_small_bar.h:30` — `	 * Without this (or if this is an older kernel) the value here will`
- `Documentation/gpu/rfc/i915_small_bar.h:46` — `			 * remainder (if there is any) will not be CPU`
- `Documentation/gpu/rfc/i915_small_bar.h:57` — `			 * Note that if the value returned here is zero, then`
- `Documentation/gpu/rfc/i915_small_bar.h:112` — `	 * rounding up, if for example using the I915_GEM_CREATE_EXT_MEMORY_REGIONS`
- `Documentation/gpu/rfc/i915_small_bar.h:141` — `	 * determine if this system applies.`
- `Documentation/gpu/rfc/i915_small_bar.h:145` — `	 * if the object can't be allocated in the mappable part of`
- `Documentation/gpu/rfc/i915_small_bar.h:156` — `	 * resort, if userspace ever CPU faults this object, but this might be`
- `Documentation/gpu/rfc/i915_vm_bind.h:103` — ` * Error code -EINVAL will be returned if @start, @offset and @length are not`
- `Documentation/gpu/rfc/i915_vm_bind.h:105` — ` * -ENOSPC will be returned if the VA range specified can't be reserved.`
- `Documentation/gpu/rfc/i915_vm_bind.h:109` — ` * asynchronously, if valid @fence is specified.`
- `Documentation/gpu/rfc/i915_vm_bind.h:174` — ` * asynchronously, if valid @fence is specified.`
- `Documentation/scheduler/sched-pelt.c:28` — `		if (i % 6 == 0) printf("\n\t");`
- `Documentation/scheduler/sched-pelt.c:42` — `		if (i == 1)`
- `Documentation/scheduler/sched-pelt.c:47` — `		if (i % 11 == 0)`
- `Documentation/scheduler/sched-pelt.c:64` — `		if (n > -1)`
- `Documentation/scheduler/sched-pelt.c:71` — `		if (last == max)`
- `Documentation/scheduler/sched-pelt.c:88` — `		if (i > 1)`
- `Documentation/scheduler/sched-pelt.c:91` — `		if (i % 6 == 0)`
- `Documentation/usb/usbdevfs-drop-permissions.c:24` — `	if (res)`
- `Documentation/usb/usbdevfs-drop-permissions.c:35` — `	if (!res)`
