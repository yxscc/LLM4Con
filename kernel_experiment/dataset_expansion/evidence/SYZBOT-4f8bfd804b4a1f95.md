# Evidence: SYZBOT-4f8bfd804b4a1f95  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in sbitmap_queue_clear / sbitmap_queue_clear (3)
- mainline fix commit: `9f8b93a7df4d8e1e8715fb2a45a893cffad9da0b`
- candidate JSON: `A_SYZBOT-4f8bfd804b4a1f95.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "sbitmap_queue_clear", "fn_b": "sbitmap_queue_clear"}`

## Commit message

```
9f8b93a7df4d8e1e8715fb2a45a893cffad9da0b
sbitmap: silence data race warning

KCSAN complaints about the sbitmap hint update:

==================================================================
BUG: KCSAN: data-race in sbitmap_queue_clear / sbitmap_queue_clear

write to 0xffffe8ffffd145b8 of 4 bytes by interrupt on cpu 1:
 sbitmap_queue_clear+0xca/0xf0 lib/sbitmap.c:606
 blk_mq_put_tag+0x82/0x90
 __blk_mq_free_request+0x114/0x180 block/blk-mq.c:507
 blk_mq_free_request+0x2c8/0x340 block/blk-mq.c:541
 __blk_mq_end_request+0x214/0x230 block/blk-mq.c:565
 blk_mq_end_request+0x37/0x50 block/blk-mq.c:574
 lo_complete_rq+0xca/0x170 drivers/block/loop.c:541
 blk_complete_reqs block/blk-mq.c:584 [inline]
 blk_done_softirq+0x69/0x90 block/blk-mq.c:589
 __do_softirq+0x12c/0x26e kernel/softirq.c:558
 run_ksoftirqd+0x13/0x20 kernel/softirq.c:920
 smpboot_thread_fn+0x22f/0x330 kernel/smpboot.c:164
 kthread+0x262/0x280 kernel/kthread.c:319
 ret_from_fork+0x1f/0x30

write to 0xffffe8ffffd145b8 of 4 bytes by interrupt on cpu 0:
 sbitmap_queue_clear+0xca/0xf0 lib/sbitmap.c:606
 blk_mq_put_tag+0x82/0x90
 __blk_mq_free_request+0x114/0x180 block/blk-mq.c:507
 blk_mq_free_request+0x2c8/0x340 block/blk-mq.c:541
 __blk_mq_end_request+0x214/0x230 block/blk-mq.c:565
 blk_mq_end_request+0x37/0x50 block/blk-mq.c:574
 lo_complete_rq+0xca/0x170 drivers/block/loop.c:541
 blk_complete_reqs block/blk-mq.c:584 [inline]
 blk_done_softirq+0x69/0x90 block/blk-mq.c:589
 __do_softirq+0x12c/0x26e kernel/softirq.c:558
 run_ksoftirqd+0x13/0x20 kernel/softirq.c:920
 smpboot_thread_fn+0x22f/0x330 kernel/smpboot.c:164
 kthread+0x262/0x280 kernel/kthread.c:319
 ret_from_fork+0x1f/0x30

value changed: 0x00000035 -> 0x00000044

Reported by Kernel Concurrency Sanitizer on:
CPU: 0 PID: 10 Comm: ksoftirqd/0 Not tainted 5.15.0-rc6-syzkaller #0
Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 01/01/2011
==================================================================

which is a data race, but not an important one. This is just updating the
percpu alloc hint, and the reader of that hint doesn't ever require it to
be valid.

Just annotate it with data_race() to silence this one.

Reported-by: syzbot+4f8bfd804b4a1f95b8f6@syzkaller.appspotmail.com
Acked-by: Marco Elver <elver@google.com>
Signed-off-by: Jens Axboe <axboe@kernel.dk>
```

## CVE / bug description

```
KCSAN: data-race in sbitmap_queue_clear / sbitmap_queue_clear (3)
```

## Full mainline patch

```diff
commit 9f8b93a7df4d8e1e8715fb2a45a893cffad9da0b
Author: Jens Axboe <axboe@kernel.dk>
Date:   Mon Oct 25 10:45:01 2021 -0600

    sbitmap: silence data race warning
    
    KCSAN complaints about the sbitmap hint update:
    
    ==================================================================
    BUG: KCSAN: data-race in sbitmap_queue_clear / sbitmap_queue_clear
    
    write to 0xffffe8ffffd145b8 of 4 bytes by interrupt on cpu 1:
     sbitmap_queue_clear+0xca/0xf0 lib/sbitmap.c:606
     blk_mq_put_tag+0x82/0x90
     __blk_mq_free_request+0x114/0x180 block/blk-mq.c:507
     blk_mq_free_request+0x2c8/0x340 block/blk-mq.c:541
     __blk_mq_end_request+0x214/0x230 block/blk-mq.c:565
     blk_mq_end_request+0x37/0x50 block/blk-mq.c:574
     lo_complete_rq+0xca/0x170 drivers/block/loop.c:541
     blk_complete_reqs block/blk-mq.c:584 [inline]
     blk_done_softirq+0x69/0x90 block/blk-mq.c:589
     __do_softirq+0x12c/0x26e kernel/softirq.c:558
     run_ksoftirqd+0x13/0x20 kernel/softirq.c:920
     smpboot_thread_fn+0x22f/0x330 kernel/smpboot.c:164
     kthread+0x262/0x280 kernel/kthread.c:319
     ret_from_fork+0x1f/0x30
    
    write to 0xffffe8ffffd145b8 of 4 bytes by interrupt on cpu 0:
     sbitmap_queue_clear+0xca/0xf0 lib/sbitmap.c:606
     blk_mq_put_tag+0x82/0x90
     __blk_mq_free_request+0x114/0x180 block/blk-mq.c:507
     blk_mq_free_request+0x2c8/0x340 block/blk-mq.c:541
     __blk_mq_end_request+0x214/0x230 block/blk-mq.c:565
     blk_mq_end_request+0x37/0x50 block/blk-mq.c:574
     lo_complete_rq+0xca/0x170 drivers/block/loop.c:541
     blk_complete_reqs block/blk-mq.c:584 [inline]
     blk_done_softirq+0x69/0x90 block/blk-mq.c:589
     __do_softirq+0x12c/0x26e kernel/softirq.c:558
     run_ksoftirqd+0x13/0x20 kernel/softirq.c:920
     smpboot_thread_fn+0x22f/0x330 kernel/smpboot.c:164
     kthread+0x262/0x280 kernel/kthread.c:319
     ret_from_fork+0x1f/0x30
    
    value changed: 0x00000035 -> 0x00000044
    
    Reported by Kernel Concurrency Sanitizer on:
    CPU: 0 PID: 10 Comm: ksoftirqd/0 Not tainted 5.15.0-rc6-syzkaller #0
    Hardware name: Google Google Compute Engine/Google Compute Engine, BIOS Google 01/01/2011
    ==================================================================
    
    which is a data race, but not an important one. This is just updating the
    percpu alloc hint, and the reader of that hint doesn't ever require it to
    be valid.
    
    Just annotate it with data_race() to silence this one.
    
    Reported-by: syzbot+4f8bfd804b4a1f95b8f6@syzkaller.appspotmail.com
    Acked-by: Marco Elver <elver@google.com>
    Signed-off-by: Jens Axboe <axboe@kernel.dk>

diff --git a/lib/sbitmap.c b/lib/sbitmap.c
index c6e2f1f2c4d2..2709ab825499 100644
--- a/lib/sbitmap.c
+++ b/lib/sbitmap.c
@@ -631,7 +631,7 @@ EXPORT_SYMBOL_GPL(sbitmap_queue_wake_up);
 static inline void sbitmap_update_cpu_hint(struct sbitmap *sb, int cpu, int tag)
 {
 	if (likely(!sb->round_robin && tag < sb->depth))
-		*per_cpu_ptr(sb->alloc_hint, cpu) = tag;
+		data_race(*per_cpu_ptr(sb->alloc_hint, cpu) = tag);
 }
 
 void sbitmap_queue_clear_batch(struct sbitmap_queue *sbq, int offset,

```

## File: `lib/sbitmap.c`

- changed old-lines: [634]

### Function `sbitmap_update_cpu_hint` (L631–L635, 5 lines)

```c
static inline void sbitmap_update_cpu_hint(struct sbitmap *sb, int cpu, int tag)
{
	if (likely(!sb->round_robin && tag < sb->depth))
		*per_cpu_ptr(sb->alloc_hint, cpu) = tag;
}
```

_(no external call sites found for `sbitmap_update_cpu_hint`)_
