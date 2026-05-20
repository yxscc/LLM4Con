# Evidence: SYZBOT-63cbe31877bb80ef  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in snd_seq_check_queue / snd_seq_control_queue (3)
- mainline fix commit: `4ebd47037027c4beae99680bff3b20fdee5d7c1e`
- candidate JSON: `A_SYZBOT-63cbe31877bb80ef.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "snd_seq_check_queue", "fn_b": "snd_seq_control_queue"}`

## Commit message

```
4ebd47037027c4beae99680bff3b20fdee5d7c1e
ALSA: seq: Use bool for snd_seq_queue internal flags

The snd_seq_queue struct contains various flags in the bit fields.
Those are categorized to two different use cases, both of which are
protected by different spinlocks.  That implies that there are still
potential risks of the bad operations for bit fields by concurrent
accesses.

For addressing the problem, this patch rearranges those flags to be
a standard bool instead of a bit field.

Reported-by: syzbot+63cbe31877bb80ef58f5@syzkaller.appspotmail.com
Link: https://lore.kernel.org/r/20201206083456.21110-1-tiwai@suse.de
Signed-off-by: Takashi Iwai <tiwai@suse.de>
```

## CVE / bug description

```
KCSAN: data-race in snd_seq_check_queue / snd_seq_control_queue (3)
```

## Full mainline patch

```diff
commit 4ebd47037027c4beae99680bff3b20fdee5d7c1e
Author: Takashi Iwai <tiwai@suse.de>
Date:   Sun Dec 6 09:34:56 2020 +0100

    ALSA: seq: Use bool for snd_seq_queue internal flags
    
    The snd_seq_queue struct contains various flags in the bit fields.
    Those are categorized to two different use cases, both of which are
    protected by different spinlocks.  That implies that there are still
    potential risks of the bad operations for bit fields by concurrent
    accesses.
    
    For addressing the problem, this patch rearranges those flags to be
    a standard bool instead of a bit field.
    
    Reported-by: syzbot+63cbe31877bb80ef58f5@syzkaller.appspotmail.com
    Link: https://lore.kernel.org/r/20201206083456.21110-1-tiwai@suse.de
    Signed-off-by: Takashi Iwai <tiwai@suse.de>

diff --git a/sound/core/seq/seq_queue.h b/sound/core/seq/seq_queue.h
index 1c3a8d3254d9..c69105dc1a10 100644
--- a/sound/core/seq/seq_queue.h
+++ b/sound/core/seq/seq_queue.h
@@ -26,10 +26,10 @@ struct snd_seq_queue {
 	
 	struct snd_seq_timer *timer;	/* time keeper for this queue */
 	int	owner;		/* client that 'owns' the timer */
-	unsigned int	locked:1,	/* timer is only accesibble by owner if set */
-		klocked:1,	/* kernel lock (after START) */	
-		check_again:1,
-		check_blocked:1;
+	bool	locked;		/* timer is only accesibble by owner if set */
+	bool	klocked;	/* kernel lock (after START) */
+	bool	check_again;	/* concurrent access happened during check */
+	bool	check_blocked;	/* queue being checked */
 
 	unsigned int flags;		/* status flags */
 	unsigned int info_flags;	/* info for sync */

```

## File: `sound/core/seq/seq_queue.h`

- changed old-lines: [29, 30, 31, 32]
