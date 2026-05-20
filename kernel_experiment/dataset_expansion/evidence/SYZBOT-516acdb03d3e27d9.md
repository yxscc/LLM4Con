# Evidence: SYZBOT-516acdb03d3e27d9  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in __bpf_lru_list_rotate / bpf_lru_push_free (2)
- mainline fix commit: `6df8fb83301d68ea0a0c0e1cbcc790fcc333ed12`
- candidate JSON: `A_SYZBOT-516acdb03d3e27d9.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "__bpf_lru_list_rotate", "fn_b": "bpf_lru_push_free"}`

## Commit message

```
6df8fb83301d68ea0a0c0e1cbcc790fcc333ed12
bpf_lru_list: Read double-checked variable once without lock

For double-checked locking in bpf_common_lru_push_free(), node->type is
read outside the critical section and then re-checked under the lock.
However, concurrent writes to node->type result in data races.

For example, the following concurrent access was observed by KCSAN:

  write to 0xffff88801521bc22 of 1 bytes by task 10038 on cpu 1:
   __bpf_lru_node_move_in        kernel/bpf/bpf_lru_list.c:91
   __local_list_flush            kernel/bpf/bpf_lru_list.c:298
   ...
  read to 0xffff88801521bc22 of 1 bytes by task 10043 on cpu 0:
   bpf_common_lru_push_free      kernel/bpf/bpf_lru_list.c:507
   bpf_lru_push_free             kernel/bpf/bpf_lru_list.c:555
   ...

Fix the data races where node->type is read outside the critical section
(for double-checked locking) by marking the access with READ_ONCE() as
well as ensuring the variable is only accessed once.

Fixes: 3a08c2fd7634 ("bpf: LRU List")
Reported-by: syzbot+3536db46dfa58c573458@syzkaller.appspotmail.com
Reported-by: syzbot+516acdb03d3e27d91bcd@syzkaller.appspotmail.com
Signed-off-by: Marco Elver <elver@google.com>
Signed-off-by: Andrii Nakryiko <andrii@kernel.org>
Acked-by: Martin KaFai Lau <kafai@fb.com>
Link: https://lore.kernel.org/bpf/20210209112701.3341724-1-elver@google.com
```

## CVE / bug description

```
KCSAN: data-race in __bpf_lru_list_rotate / bpf_lru_push_free (2)
```

## Full mainline patch

```diff
commit 6df8fb83301d68ea0a0c0e1cbcc790fcc333ed12
Author: Marco Elver <elver@google.com>
Date:   Tue Feb 9 12:27:01 2021 +0100

    bpf_lru_list: Read double-checked variable once without lock
    
    For double-checked locking in bpf_common_lru_push_free(), node->type is
    read outside the critical section and then re-checked under the lock.
    However, concurrent writes to node->type result in data races.
    
    For example, the following concurrent access was observed by KCSAN:
    
      write to 0xffff88801521bc22 of 1 bytes by task 10038 on cpu 1:
       __bpf_lru_node_move_in        kernel/bpf/bpf_lru_list.c:91
       __local_list_flush            kernel/bpf/bpf_lru_list.c:298
       ...
      read to 0xffff88801521bc22 of 1 bytes by task 10043 on cpu 0:
       bpf_common_lru_push_free      kernel/bpf/bpf_lru_list.c:507
       bpf_lru_push_free             kernel/bpf/bpf_lru_list.c:555
       ...
    
    Fix the data races where node->type is read outside the critical section
    (for double-checked locking) by marking the access with READ_ONCE() as
    well as ensuring the variable is only accessed once.
    
    Fixes: 3a08c2fd7634 ("bpf: LRU List")
    Reported-by: syzbot+3536db46dfa58c573458@syzkaller.appspotmail.com
    Reported-by: syzbot+516acdb03d3e27d91bcd@syzkaller.appspotmail.com
    Signed-off-by: Marco Elver <elver@google.com>
    Signed-off-by: Andrii Nakryiko <andrii@kernel.org>
    Acked-by: Martin KaFai Lau <kafai@fb.com>
    Link: https://lore.kernel.org/bpf/20210209112701.3341724-1-elver@google.com

diff --git a/kernel/bpf/bpf_lru_list.c b/kernel/bpf/bpf_lru_list.c
index 1b6b9349cb85..d99e89f113c4 100644
--- a/kernel/bpf/bpf_lru_list.c
+++ b/kernel/bpf/bpf_lru_list.c
@@ -502,13 +502,14 @@ struct bpf_lru_node *bpf_lru_pop_free(struct bpf_lru *lru, u32 hash)
 static void bpf_common_lru_push_free(struct bpf_lru *lru,
 				     struct bpf_lru_node *node)
 {
+	u8 node_type = READ_ONCE(node->type);
 	unsigned long flags;
 
-	if (WARN_ON_ONCE(node->type == BPF_LRU_LIST_T_FREE) ||
-	    WARN_ON_ONCE(node->type == BPF_LRU_LOCAL_LIST_T_FREE))
+	if (WARN_ON_ONCE(node_type == BPF_LRU_LIST_T_FREE) ||
+	    WARN_ON_ONCE(node_type == BPF_LRU_LOCAL_LIST_T_FREE))
 		return;
 
-	if (node->type == BPF_LRU_LOCAL_LIST_T_PENDING) {
+	if (node_type == BPF_LRU_LOCAL_LIST_T_PENDING) {
 		struct bpf_lru_locallist *loc_l;
 
 		loc_l = per_cpu_ptr(lru->common_lru.local_list, node->cpu);

```

## File: `kernel/bpf/bpf_lru_list.c`

- changed old-lines: [507, 508, 511]

### Function `bpf_common_lru_push_free` (L502–L533, 32 lines)

```c
static void bpf_common_lru_push_free(struct bpf_lru *lru,
				     struct bpf_lru_node *node)
{
	unsigned long flags;

	if (WARN_ON_ONCE(node->type == BPF_LRU_LIST_T_FREE) ||
	    WARN_ON_ONCE(node->type == BPF_LRU_LOCAL_LIST_T_FREE))
		return;

	if (node->type == BPF_LRU_LOCAL_LIST_T_PENDING) {
		struct bpf_lru_locallist *loc_l;

		loc_l = per_cpu_ptr(lru->common_lru.local_list, node->cpu);

		raw_spin_lock_irqsave(&loc_l->lock, flags);

		if (unlikely(node->type != BPF_LRU_LOCAL_LIST_T_PENDING)) {
			raw_spin_unlock_irqrestore(&loc_l->lock, flags);
			goto check_lru_list;
		}

		node->type = BPF_LRU_LOCAL_LIST_T_FREE;
		node->ref = 0;
		list_move(&node->list, local_free_list(loc_l));

		raw_spin_unlock_irqrestore(&loc_l->lock, flags);
		return;
	}

check_lru_list:
	bpf_lru_list_push_free(&lru->common_lru.lru_list, node);
}
```

_(no external call sites found for `bpf_common_lru_push_free`)_
