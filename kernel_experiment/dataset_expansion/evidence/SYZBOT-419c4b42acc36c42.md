# Evidence: SYZBOT-419c4b42acc36c42  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in try_to_migrate_one / zap_page_range_single
- mainline fix commit: `2e976567233228ff928c2405f7e03ebb7fb7aa50`
- candidate JSON: `A_SYZBOT-419c4b42acc36c42.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "try_to_migrate_one", "fn_b": "zap_page_range_single"}`

## Commit message

```
2e976567233228ff928c2405f7e03ebb7fb7aa50
mm: annotate data race in update_hiwater_rss

mm_struct.hiwater_rss can be accessed concurrently without proper
synchronization as reported by KCSAN.

This data race is benign as it only affects accounting information.
Annotate it with data_race() to make KCSAN happy.

Link: https://lkml.kernel.org/r/20250331-mm-maxrss-data-race-v2-1-cf958e6205bf@iencinas.com
Signed-off-by: Ignacio Encinas <ignacio@iencinas.com>
Reported-by: syzbot+419c4b42acc36c420ad3@syzkaller.appspotmail.com
Closes: https://lore.kernel.org/all/67e3390c.050a0220.1ec46.0001.GAE@google.com/
Suggested-by: Lorenzo Stoakes <lorenzo.stoakes@oracle.com>
Acked-by: Pedro Falcato <pfalcato@suse.de>
Cc: Liam Howlett <liam.howlett@oracle.com>
Signed-off-by: Andrew Morton <akpm@linux-foundation.org>
```

## CVE / bug description

```
KCSAN: data-race in try_to_migrate_one / zap_page_range_single
```

## Full mainline patch

```diff
commit 2e976567233228ff928c2405f7e03ebb7fb7aa50
Author: Ignacio Encinas <ignacio@iencinas.com>
Date:   Mon Mar 31 21:57:05 2025 +0200

    mm: annotate data race in update_hiwater_rss
    
    mm_struct.hiwater_rss can be accessed concurrently without proper
    synchronization as reported by KCSAN.
    
    This data race is benign as it only affects accounting information.
    Annotate it with data_race() to make KCSAN happy.
    
    Link: https://lkml.kernel.org/r/20250331-mm-maxrss-data-race-v2-1-cf958e6205bf@iencinas.com
    Signed-off-by: Ignacio Encinas <ignacio@iencinas.com>
    Reported-by: syzbot+419c4b42acc36c420ad3@syzkaller.appspotmail.com
    Closes: https://lore.kernel.org/all/67e3390c.050a0220.1ec46.0001.GAE@google.com/
    Suggested-by: Lorenzo Stoakes <lorenzo.stoakes@oracle.com>
    Acked-by: Pedro Falcato <pfalcato@suse.de>
    Cc: Liam Howlett <liam.howlett@oracle.com>
    Signed-off-by: Andrew Morton <akpm@linux-foundation.org>

diff --git a/include/linux/mm.h b/include/linux/mm.h
index dcdb798184ef..1690f21e7808 100644
--- a/include/linux/mm.h
+++ b/include/linux/mm.h
@@ -12,6 +12,7 @@
 #include <linux/rbtree.h>
 #include <linux/atomic.h>
 #include <linux/debug_locks.h>
+#include <linux/compiler.h>
 #include <linux/mm_types.h>
 #include <linux/mmap_lock.h>
 #include <linux/range.h>
@@ -2796,7 +2797,7 @@ static inline void update_hiwater_rss(struct mm_struct *mm)
 {
 	unsigned long _rss = get_mm_rss(mm);
 
-	if ((mm)->hiwater_rss < _rss)
+	if (data_race(mm->hiwater_rss) < _rss)
 		(mm)->hiwater_rss = _rss;
 }
 

```

## File: `include/linux/mm.h`

- changed old-lines: [2799]

### Function `update_hiwater_rss` (L2795–L2801, 7 lines)

```c
static inline void update_hiwater_rss(struct mm_struct *mm)
{
	unsigned long _rss = get_mm_rss(mm);

	if ((mm)->hiwater_rss < _rss)
		(mm)->hiwater_rss = _rss;
}
```

**External callers of `update_hiwater_rss` at parent**

- `mm/madvise.c:819` — `	update_hiwater_rss(mm);`
- `mm/memory.c:2012` — `	update_hiwater_rss(vma->vm_mm);`
- `mm/mmap.c:1282` — `	/* update_hiwater_rss(mm) here? but nobody should be looking */`
- `mm/rmap.c:2061` — `		update_hiwater_rss(mm);`
- `mm/rmap.c:2447` — `		update_hiwater_rss(mm);`
- `mm/vma.c:445` — `	update_hiwater_rss(mm);`
- `mm/vma.c:1183` — `	update_hiwater_rss(vms->vma->vm_mm);`
- `tools/testing/vma/vma_internal.h:712` — `static inline void update_hiwater_rss(struct mm_struct *)`
