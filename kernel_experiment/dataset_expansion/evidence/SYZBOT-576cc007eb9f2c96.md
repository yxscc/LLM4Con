# Evidence: SYZBOT-576cc007eb9f2c96  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in __snd_rawmidi_transmit_ack / snd_rawmidi_write
- mainline fix commit: `dfa9a5efe8b932a84b3b319250aa3ac60c20f876`
- candidate JSON: `A_SYZBOT-576cc007eb9f2c96.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "__snd_rawmidi_transmit_ack", "fn_b": "snd_rawmidi_write"}`

## Commit message

```
dfa9a5efe8b932a84b3b319250aa3ac60c20f876
ALSA: rawmidi: Avoid bit fields for state flags

The rawmidi state flags (opened, append, active_sensing) are stored in
bit fields that can be potentially racy when concurrently accessed
without any locks.  Although the current code should be fine, there is
also no any real benefit by keeping the bitfields for this kind of
short number of members.

This patch changes those bit fields flags to the simple bool fields.
There should be no size increase of the snd_rawmidi_substream by this
change.

Reported-by: syzbot+576cc007eb9f2c968200@syzkaller.appspotmail.com
Link: https://lore.kernel.org/r/20200214111316.26939-4-tiwai@suse.de
Signed-off-by: Takashi Iwai <tiwai@suse.de>
```

## CVE / bug description

```
KCSAN: data-race in __snd_rawmidi_transmit_ack / snd_rawmidi_write
```

## Full mainline patch

```diff
commit dfa9a5efe8b932a84b3b319250aa3ac60c20f876
Author: Takashi Iwai <tiwai@suse.de>
Date:   Fri Feb 14 12:13:16 2020 +0100

    ALSA: rawmidi: Avoid bit fields for state flags
    
    The rawmidi state flags (opened, append, active_sensing) are stored in
    bit fields that can be potentially racy when concurrently accessed
    without any locks.  Although the current code should be fine, there is
    also no any real benefit by keeping the bitfields for this kind of
    short number of members.
    
    This patch changes those bit fields flags to the simple bool fields.
    There should be no size increase of the snd_rawmidi_substream by this
    change.
    
    Reported-by: syzbot+576cc007eb9f2c968200@syzkaller.appspotmail.com
    Link: https://lore.kernel.org/r/20200214111316.26939-4-tiwai@suse.de
    Signed-off-by: Takashi Iwai <tiwai@suse.de>

diff --git a/include/sound/rawmidi.h b/include/sound/rawmidi.h
index 40ab20439fee..a36b7227a15a 100644
--- a/include/sound/rawmidi.h
+++ b/include/sound/rawmidi.h
@@ -77,9 +77,9 @@ struct snd_rawmidi_substream {
 	struct list_head list;		/* list of all substream for given stream */
 	int stream;			/* direction */
 	int number;			/* substream number */
-	unsigned int opened: 1,		/* open flag */
-		     append: 1,		/* append flag (merge more streams) */
-		     active_sensing: 1; /* send active sensing when close */
+	bool opened;			/* open flag */
+	bool append;			/* append flag (merge more streams) */
+	bool active_sensing;		/* send active sensing when close */
 	int use_count;			/* use counter (for output) */
 	size_t bytes;
 	struct snd_rawmidi *rmidi;

```

## File: `include/sound/rawmidi.h`

- changed old-lines: [80, 81, 82]

### Function `void` (L68–L92, 25 lines)

```c
	void (*event)(struct snd_rawmidi_substream *substream);
	/* defers calls to event [input] or ops->trigger [output] */
	struct work_struct event_work;
	/* private data */
	void *private_data;
	void (*private_free)(struct snd_rawmidi_substream *substream);
};

struct snd_rawmidi_substream {
	struct list_head list;		/* list of all substream for given stream */
	int stream;			/* direction */
	int number;			/* substream number */
	unsigned int opened: 1,		/* open flag */
		     append: 1,		/* append flag (merge more streams) */
		     active_sensing: 1; /* send active sensing when close */
	int use_count;			/* use counter (for output) */
	size_t bytes;
	struct snd_rawmidi *rmidi;
	struct snd_rawmidi_str *pstr;
	char name[32];
	struct snd_rawmidi_runtime *runtime;
	struct pid *pid;
	/* hardware layer */
	const struct snd_rawmidi_ops *ops;
};
```

**External callers of `void` at parent**

- `Documentation/scheduler/sched-pelt.c:18` — `void calc_runnable_avg_yN_inv(void)`
- `Documentation/scheduler/sched-pelt.c:36` — `void calc_runnable_avg_yN_sum(void)`
- `Documentation/scheduler/sched-pelt.c:59` — `void calc_converged_max(void)`
- `Documentation/scheduler/sched-pelt.c:82` — `void calc_accumulated_sum_32(void)`
- `Documentation/scheduler/sched-pelt.c:99` — `void main(void)`
- `Documentation/usb/usbdevfs-drop-permissions.c:19` — `void drop_privileges(int fd, uint32_t mask)`
- `Documentation/usb/usbdevfs-drop-permissions.c:30` — `void reset_device(int fd)`
- `Documentation/usb/usbdevfs-drop-permissions.c:42` — `void claim_some_intf(int fd)`
- `arch/alpha/boot/bootp.c:30` — `extern void move_stack(unsigned long new_stack);`
- `arch/alpha/boot/bootp.c:41` — `static inline void *`
- `arch/alpha/boot/bootp.c:42` — `find_pa(unsigned long *vptb, void *ptr)`
- `arch/alpha/boot/bootp.c:51` — `	return (void *) result;`
- `arch/alpha/boot/bootp.c:68` — `void`
- `arch/alpha/boot/bootp.c:69` — `pal_init(void)`
- `arch/alpha/boot/bootp.c:111` — `static inline void`
- `arch/alpha/boot/bootp.c:114` — `	memcpy((void *)dst, (void *)src, count);`
- `arch/alpha/boot/bootp.c:120` — `static inline void`
- `arch/alpha/boot/bootp.c:121` — `runkernel(void)`
- `arch/alpha/boot/bootp.c:134` — `void`
- `arch/alpha/boot/bootp.c:135` — `start_kernel(void)`
