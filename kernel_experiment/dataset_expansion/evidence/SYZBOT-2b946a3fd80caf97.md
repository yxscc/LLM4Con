# Evidence: SYZBOT-2b946a3fd80caf97  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in io_sq_thread / io_sq_thread_park (9)
- mainline fix commit: `e4956dc7a84da074fd8dc10f7abd147f15b3ae58`
- candidate JSON: `A_SYZBOT-2b946a3fd80caf97.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "io_sq_thread", "fn_b": "io_sq_thread_park"}`

## Commit message

```
e4956dc7a84da074fd8dc10f7abd147f15b3ae58
io_uring/sqpoll: annotate debug task == current with data_race()

There's a debug check in io_sq_thread_park() checking if it's the SQPOLL
thread itself calling park. KCSAN warns about this, as we should not be
reading sqd->thread outside of sqd->lock.

Just silence this with data_race(). The pointer isn't used for anything
but this debug check.

Reported-by: syzbot+2b946a3fd80caf971b21@syzkaller.appspotmail.com
Signed-off-by: Jens Axboe <axboe@kernel.dk>
```

## CVE / bug description

```
KCSAN: data-race in io_sq_thread / io_sq_thread_park (9)
```

## Full mainline patch

```diff
commit e4956dc7a84da074fd8dc10f7abd147f15b3ae58
Author: Jens Axboe <axboe@kernel.dk>
Date:   Tue Aug 13 06:10:59 2024 -0600

    io_uring/sqpoll: annotate debug task == current with data_race()
    
    There's a debug check in io_sq_thread_park() checking if it's the SQPOLL
    thread itself calling park. KCSAN warns about this, as we should not be
    reading sqd->thread outside of sqd->lock.
    
    Just silence this with data_race(). The pointer isn't used for anything
    but this debug check.
    
    Reported-by: syzbot+2b946a3fd80caf971b21@syzkaller.appspotmail.com
    Signed-off-by: Jens Axboe <axboe@kernel.dk>

diff --git a/io_uring/sqpoll.c b/io_uring/sqpoll.c
index b3722e5275e7..3b50dc9586d1 100644
--- a/io_uring/sqpoll.c
+++ b/io_uring/sqpoll.c
@@ -44,7 +44,7 @@ void io_sq_thread_unpark(struct io_sq_data *sqd)
 void io_sq_thread_park(struct io_sq_data *sqd)
 	__acquires(&sqd->lock)
 {
-	WARN_ON_ONCE(sqd->thread == current);
+	WARN_ON_ONCE(data_race(sqd->thread) == current);
 
 	atomic_inc(&sqd->park_pending);
 	set_bit(IO_SQ_THREAD_SHOULD_PARK, &sqd->state);

```

## File: `io_uring/sqpoll.c`

- changed old-lines: [47]

### Function `io_sq_thread_park` (L44–L54, 11 lines)

```c
void io_sq_thread_park(struct io_sq_data *sqd)
	__acquires(&sqd->lock)
{
	WARN_ON_ONCE(sqd->thread == current);

	atomic_inc(&sqd->park_pending);
	set_bit(IO_SQ_THREAD_SHOULD_PARK, &sqd->state);
	mutex_lock(&sqd->lock);
	if (sqd->thread)
		wake_up_process(sqd->thread);
}
```

**External callers of `io_sq_thread_park` at parent**

- `io_uring/io_uring.c:2786` — `			io_sq_thread_park(sqd);`
- `io_uring/sqpoll.h:27` — `void io_sq_thread_park(struct io_sq_data *sqd);`
