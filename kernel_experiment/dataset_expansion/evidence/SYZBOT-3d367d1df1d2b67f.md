# Evidence: SYZBOT-3d367d1df1d2b67f  (tier A, score 200)

- source archive: **syzbot**
- title: KCSAN: data-race in snd_rawmidi_poll / snd_rawmidi_proceed
- mainline fix commit: `88a06d6fd6b369d88cec46c62db3e2604a2f50d5`
- candidate JSON: `A_SYZBOT-3d367d1df1d2b67f.json`
- thread_hint: `{"kind": "kcsan", "fn_a": "snd_rawmidi_poll", "fn_b": "snd_rawmidi_proceed"}`

## Commit message

```
88a06d6fd6b369d88cec46c62db3e2604a2f50d5
ALSA: rawmidi: Access runtime->avail always in spinlock

The runtime->avail field may be accessed concurrently while some
places refer to it without taking the runtime->lock spinlock, as
detected by KCSAN.  Usually this isn't a big problem, but for
consistency and safety, we should take the spinlock at each place
referencing this field.

Reported-by: syzbot+a23a6f1215c84756577c@syzkaller.appspotmail.com
Reported-by: syzbot+3d367d1df1d2b67f5c19@syzkaller.appspotmail.com
Link: https://lore.kernel.org/r/20201206083527.21163-1-tiwai@suse.de
Signed-off-by: Takashi Iwai <tiwai@suse.de>
```

## CVE / bug description

```
KCSAN: data-race in snd_rawmidi_poll / snd_rawmidi_proceed
```

## Full mainline patch

```diff
commit 88a06d6fd6b369d88cec46c62db3e2604a2f50d5
Author: Takashi Iwai <tiwai@suse.de>
Date:   Sun Dec 6 09:35:27 2020 +0100

    ALSA: rawmidi: Access runtime->avail always in spinlock
    
    The runtime->avail field may be accessed concurrently while some
    places refer to it without taking the runtime->lock spinlock, as
    detected by KCSAN.  Usually this isn't a big problem, but for
    consistency and safety, we should take the spinlock at each place
    referencing this field.
    
    Reported-by: syzbot+a23a6f1215c84756577c@syzkaller.appspotmail.com
    Reported-by: syzbot+3d367d1df1d2b67f5c19@syzkaller.appspotmail.com
    Link: https://lore.kernel.org/r/20201206083527.21163-1-tiwai@suse.de
    Signed-off-by: Takashi Iwai <tiwai@suse.de>

diff --git a/sound/core/rawmidi.c b/sound/core/rawmidi.c
index c78720a3299c..257ad5206240 100644
--- a/sound/core/rawmidi.c
+++ b/sound/core/rawmidi.c
@@ -95,11 +95,21 @@ static inline unsigned short snd_rawmidi_file_flags(struct file *file)
 	}
 }
 
-static inline int snd_rawmidi_ready(struct snd_rawmidi_substream *substream)
+static inline bool __snd_rawmidi_ready(struct snd_rawmidi_runtime *runtime)
+{
+	return runtime->avail >= runtime->avail_min;
+}
+
+static bool snd_rawmidi_ready(struct snd_rawmidi_substream *substream)
 {
 	struct snd_rawmidi_runtime *runtime = substream->runtime;
+	unsigned long flags;
+	bool ready;
 
-	return runtime->avail >= runtime->avail_min;
+	spin_lock_irqsave(&runtime->lock, flags);
+	ready = __snd_rawmidi_ready(runtime);
+	spin_unlock_irqrestore(&runtime->lock, flags);
+	return ready;
 }
 
 static inline int snd_rawmidi_ready_append(struct snd_rawmidi_substream *substream,
@@ -1019,7 +1029,7 @@ int snd_rawmidi_receive(struct snd_rawmidi_substream *substream,
 	if (result > 0) {
 		if (runtime->event)
 			schedule_work(&runtime->event_work);
-		else if (snd_rawmidi_ready(substream))
+		else if (__snd_rawmidi_ready(runtime))
 			wake_up(&runtime->sleep);
 	}
 	spin_unlock_irqrestore(&runtime->lock, flags);
@@ -1098,7 +1108,7 @@ static ssize_t snd_rawmidi_read(struct file *file, char __user *buf, size_t coun
 	result = 0;
 	while (count > 0) {
 		spin_lock_irq(&runtime->lock);
-		while (!snd_rawmidi_ready(substream)) {
+		while (!__snd_rawmidi_ready(runtime)) {
 			wait_queue_entry_t wait;
 
 			if ((file->f_flags & O_NONBLOCK) != 0 || result > 0) {
@@ -1115,9 +1125,11 @@ static ssize_t snd_rawmidi_read(struct file *file, char __user *buf, size_t coun
 				return -ENODEV;
 			if (signal_pending(current))
 				return result > 0 ? result : -ERESTARTSYS;
-			if (!runtime->avail)
-				return result > 0 ? result : -EIO;
 			spin_lock_irq(&runtime->lock);
+			if (!runtime->avail) {
+				spin_unlock_irq(&runtime->lock);
+				return result > 0 ? result : -EIO;
+			}
 		}
 		spin_unlock_irq(&runtime->lock);
 		count1 = snd_rawmidi_kernel_read1(substream,
@@ -1255,7 +1267,7 @@ int __snd_rawmidi_transmit_ack(struct snd_rawmidi_substream *substream, int coun
 	runtime->avail += count;
 	substream->bytes += count;
 	if (count > 0) {
-		if (runtime->drain || snd_rawmidi_ready(substream))
+		if (runtime->drain || __snd_rawmidi_ready(runtime))
 			wake_up(&runtime->sleep);
 	}
 	return count;
@@ -1444,9 +1456,11 @@ static ssize_t snd_rawmidi_write(struct file *file, const char __user *buf,
 				return -ENODEV;
 			if (signal_pending(current))
 				return result > 0 ? result : -ERESTARTSYS;
-			if (!runtime->avail && !timeout)
-				return result > 0 ? result : -EIO;
 			spin_lock_irq(&runtime->lock);
+			if (!runtime->avail && !timeout) {
+				spin_unlock_irq(&runtime->lock);
+				return result > 0 ? result : -EIO;
+			}
 		}
 		spin_unlock_irq(&runtime->lock);
 		count1 = snd_rawmidi_kernel_write1(substream, buf, NULL, count);
@@ -1526,6 +1540,7 @@ static void snd_rawmidi_proc_info_read(struct snd_info_entry *entry,
 	struct snd_rawmidi *rmidi;
 	struct snd_rawmidi_substream *substream;
 	struct snd_rawmidi_runtime *runtime;
+	unsigned long buffer_size, avail, xruns;
 
 	rmidi = entry->private_data;
 	snd_iprintf(buffer, "%s\n\n", rmidi->name);
@@ -1544,13 +1559,16 @@ static void snd_rawmidi_proc_info_read(struct snd_info_entry *entry,
 				    "  Owner PID    : %d\n",
 				    pid_vnr(substream->pid));
 				runtime = substream->runtime;
+				spin_lock_irq(&runtime->lock);
+				buffer_size = runtime->buffer_size;
+				avail = runtime->avail;
+				spin_unlock_irq(&runtime->lock);
 				snd_iprintf(buffer,
 				    "  Mode         : %s\n"
 				    "  Buffer size  : %lu\n"
 				    "  Avail        : %lu\n",
 				    runtime->oss ? "OSS compatible" : "native",
-				    (unsigned long) runtime->buffer_size,
-				    (unsigned long) runtime->avail);
+				    buffer_size, avail);
 			}
 		}
 	}
@@ -1568,13 +1586,16 @@ static void snd_rawmidi_proc_info_read(struct snd_info_entry *entry,
 					    "  Owner PID    : %d\n",
 					    pid_vnr(substream->pid));
 				runtime = substream->runtime;
+				spin_lock_irq(&runtime->lock);
+				buffer_size = runtime->buffer_size;
+				avail = runtime->avail;
+				xruns = runtime->xruns;
+				spin_unlock_irq(&runtime->lock);
 				snd_iprintf(buffer,
 					    "  Buffer size  : %lu\n"
 					    "  Avail        : %lu\n"
 					    "  Overruns     : %lu\n",
-					    (unsigned long) runtime->buffer_size,
-					    (unsigned long) runtime->avail,
-					    (unsigned long) runtime->xruns);
+					    buffer_size, avail, xruns);
 			}
 		}
 	}

```

## File: `sound/core/rawmidi.c`

- changed old-lines: [98, 102, 1022, 1101, 1118, 1119, 1258, 1447, 1448, 1552, 1553, 1575, 1576, 1577]

### Function `snd_rawmidi_ready` (L98–L103, 6 lines)

```c
static inline int snd_rawmidi_ready(struct snd_rawmidi_substream *substream)
{
	struct snd_rawmidi_runtime *runtime = substream->runtime;

	return runtime->avail >= runtime->avail_min;
}
```

_(no external call sites found for `snd_rawmidi_ready`)_

### Function `snd_rawmidi_receive` (L966–L1027, 62 lines)

```c
int snd_rawmidi_receive(struct snd_rawmidi_substream *substream,
			const unsigned char *buffer, int count)
{
	unsigned long flags;
	int result = 0, count1;
	struct snd_rawmidi_runtime *runtime = substream->runtime;

	if (!substream->opened)
		return -EBADFD;
	if (runtime->buffer == NULL) {
		rmidi_dbg(substream->rmidi,
			  "snd_rawmidi_receive: input is not active!!!\n");
		return -EINVAL;
	}
	spin_lock_irqsave(&runtime->lock, flags);
	if (count == 1) {	/* special case, faster code */
		substream->bytes++;
		if (runtime->avail < runtime->buffer_size) {
			runtime->buffer[runtime->hw_ptr++] = buffer[0];
			runtime->hw_ptr %= runtime->buffer_size;
			runtime->avail++;
			result++;
		} else {
			runtime->xruns++;
		}
	} else {
		substream->bytes += count;
		count1 = runtime->buffer_size - runtime->hw_ptr;
		if (count1 > count)
			count1 = count;
		if (count1 > (int)(runtime->buffer_size - runtime->avail))
			count1 = runtime->buffer_size - runtime->avail;
		memcpy(runtime->buffer + runtime->hw_ptr, buffer, count1);
		runtime->hw_ptr += count1;
		runtime->hw_ptr %= runtime->buffer_size;
		runtime->avail += count1;
		count -= count1;
		result += count1;
		if (count > 0) {
			buffer += count1;
			count1 = count;
			if (count1 > (int)(runtime->buffer_size - runtime->avail)) {
				count1 = runtime->buffer_size - runtime->avail;
				runtime->xruns += count - count1;
			}
			if (count1 > 0) {
				memcpy(runtime->buffer, buffer, count1);
				runtime->hw_ptr = count1;
				runtime->avail += count1;
				result += count1;
			}
		}
	}
	if (result > 0) {
		if (runtime->event)
			schedule_work(&runtime->event_work);
		else if (snd_rawmidi_ready(substream))
			wake_up(&runtime->sleep);
	}
	spin_unlock_irqrestore(&runtime->lock, flags);
	return result;
}
```

**External callers of `snd_rawmidi_receive` at parent**

- `drivers/hid/hid-prodikeys.c:230` — `	snd_rawmidi_receive(pm->in_substream, buffer, 3);`
- `drivers/usb/gadget/function/f_midi.c:253` — `	snd_rawmidi_receive(substream, data, length);`
- `include/sound/rawmidi.h:148` — `int snd_rawmidi_receive(struct snd_rawmidi_substream *substream,`
- `sound/core/seq/seq_virmidi.c:83` — `			snd_seq_dump_var_event(ev, (snd_seq_dump_func_t)snd_rawmidi_receive, vmidi->substream);`
- `sound/core/seq/seq_virmidi.c:88` — `				snd_rawmidi_receive(vmidi->substream, msg, len);`
- `sound/drivers/mpu401/mpu401_uart.c:393` — `			snd_rawmidi_receive(mpu->substream_input, &byte, 1);`
- `sound/drivers/mtpav.c:495` — `		snd_rawmidi_receive(portp->input, &inbyte, 1);`
- `sound/drivers/mts64.c:831` — `			snd_rawmidi_receive(substream, &data, 1);`
- `sound/drivers/portman2x4.c:616` — `				snd_rawmidi_receive(pm->midi_input[0],`
- `sound/drivers/portman2x4.c:627` — `				snd_rawmidi_receive(pm->midi_input[1],`
- `sound/drivers/serial-u16550.c:222` — `				snd_rawmidi_receive(uart->midi_input[substream],`
- `sound/drivers/serial-u16550.c:226` — `			snd_rawmidi_receive(uart->midi_input[substream], &c, 1);`
- `sound/firewire/amdtp-am824.c:343` — `			snd_rawmidi_receive(p->midi[port], b + 1, len);`
- `sound/firewire/digi00x/amdtp-dot.c:315` — `				snd_rawmidi_receive(p->midi[port], b + 1, len);`
- `sound/firewire/fireface/ff-protocol-former.c:418` — `			snd_rawmidi_receive(substream, &byte, 1);`
- `sound/firewire/fireface/ff-protocol-former.c:580` — `				snd_rawmidi_receive(substream, &byte, 1);`
- `sound/firewire/fireface/ff-protocol-former.c:590` — `				snd_rawmidi_receive(substream, &byte, 1);`
- `sound/firewire/fireface/ff-protocol-latter.c:353` — `		snd_rawmidi_receive(substream, byte, len);`
- `sound/firewire/motu/amdtp-motu.c:273` — `			snd_rawmidi_receive(midi, b + p->midi_byte_offset, 1);`
- `sound/firewire/oxfw/oxfw-scs1x.c:51` — `	snd_rawmidi_receive(stream, nibbles, 2);`

### Function `snd_rawmidi_read` (L1083–L1134, 52 lines)

```c
static ssize_t snd_rawmidi_read(struct file *file, char __user *buf, size_t count,
				loff_t *offset)
{
	long result;
	int count1;
	struct snd_rawmidi_file *rfile;
	struct snd_rawmidi_substream *substream;
	struct snd_rawmidi_runtime *runtime;

	rfile = file->private_data;
	substream = rfile->input;
	if (substream == NULL)
		return -EIO;
	runtime = substream->runtime;
	snd_rawmidi_input_trigger(substream, 1);
	result = 0;
	while (count > 0) {
		spin_lock_irq(&runtime->lock);
		while (!snd_rawmidi_ready(substream)) {
			wait_queue_entry_t wait;

			if ((file->f_flags & O_NONBLOCK) != 0 || result > 0) {
				spin_unlock_irq(&runtime->lock);
				return result > 0 ? result : -EAGAIN;
			}
			init_waitqueue_entry(&wait, current);
			add_wait_queue(&runtime->sleep, &wait);
			set_current_state(TASK_INTERRUPTIBLE);
			spin_unlock_irq(&runtime->lock);
			schedule();
			remove_wait_queue(&runtime->sleep, &wait);
			if (rfile->rmidi->card->shutdown)
				return -ENODEV;
			if (signal_pending(current))
				return result > 0 ? result : -ERESTARTSYS;
			if (!runtime->avail)
				return result > 0 ? result : -EIO;
			spin_lock_irq(&runtime->lock);
		}
		spin_unlock_irq(&runtime->lock);
		count1 = snd_rawmidi_kernel_read1(substream,
						  (unsigned char __user *)buf,
						  NULL/*kernelbuf*/,
						  count);
		if (count1 < 0)
			return result > 0 ? result : count1;
		result += count1;
		buf += count1;
		count -= count1;
	}
	return result;
}
```

_(no external call sites found for `snd_rawmidi_read`)_

### Function `__snd_rawmidi_transmit_ack` (L1243–L1262, 20 lines)

```c
int __snd_rawmidi_transmit_ack(struct snd_rawmidi_substream *substream, int count)
{
	struct snd_rawmidi_runtime *runtime = substream->runtime;

	if (runtime->buffer == NULL) {
		rmidi_dbg(substream->rmidi,
			  "snd_rawmidi_transmit_ack: output is not active!!!\n");
		return -EINVAL;
	}
	snd_BUG_ON(runtime->avail + count > runtime->buffer_size);
	runtime->hw_ptr += count;
	runtime->hw_ptr %= runtime->buffer_size;
	runtime->avail += count;
	substream->bytes += count;
	if (count > 0) {
		if (runtime->drain || snd_rawmidi_ready(substream))
			wake_up(&runtime->sleep);
	}
	return count;
}
```

**External callers of `__snd_rawmidi_transmit_ack` at parent**

- `include/sound/rawmidi.h:158` — `int __snd_rawmidi_transmit_ack(struct snd_rawmidi_substream *substream,`

### Function `snd_rawmidi_write` (L1412–L1482, 71 lines)

```c
static ssize_t snd_rawmidi_write(struct file *file, const char __user *buf,
				 size_t count, loff_t *offset)
{
	long result, timeout;
	int count1;
	struct snd_rawmidi_file *rfile;
	struct snd_rawmidi_runtime *runtime;
	struct snd_rawmidi_substream *substream;

	rfile = file->private_data;
	substream = rfile->output;
	runtime = substream->runtime;
	/* we cannot put an atomic message to our buffer */
	if (substream->append && count > runtime->buffer_size)
		return -EIO;
	result = 0;
	while (count > 0) {
		spin_lock_irq(&runtime->lock);
		while (!snd_rawmidi_ready_append(substream, count)) {
			wait_queue_entry_t wait;

			if (file->f_flags & O_NONBLOCK) {
				spin_unlock_irq(&runtime->lock);
				return result > 0 ? result : -EAGAIN;
			}
			init_waitqueue_entry(&wait, current);
			add_wait_queue(&runtime->sleep, &wait);
			set_current_state(TASK_INTERRUPTIBLE);
			spin_unlock_irq(&runtime->lock);
			timeout = schedule_timeout(30 * HZ);
			remove_wait_queue(&runtime->sleep, &wait);
			if (rfile->rmidi->card->shutdown)
				return -ENODEV;
			if (signal_pending(current))
				return result > 0 ? result : -ERESTARTSYS;
			if (!runtime->avail && !timeout)
				return result > 0 ? result : -EIO;
			spin_lock_irq(&runtime->lock);
		}
		spin_unlock_irq(&runtime->lock);
		count1 = snd_rawmidi_kernel_write1(substream, buf, NULL, count);
		if (count1 < 0)
			return result > 0 ? result : count1;
		result += count1;
		buf += count1;
		if ((size_t)count1 < count && (file->f_flags & O_NONBLOCK))
			break;
		count -= count1;
	}
	if (file->f_flags & O_DSYNC) {
		spin_lock_irq(&runtime->lock);
		while (runtime->avail != runtime->buffer_size) {
			wait_queue_entry_t wait;
			unsigned int last_avail = runtime->avail;

			init_waitqueue_entry(&wait, current);
			add_wait_queue(&runtime->sleep, &wait);
			set_current_state(TASK_INTERRUPTIBLE);
			spin_unlock_irq(&runtime->lock);
			timeout = schedule_timeout(30 * HZ);
			remove_wait_queue(&runtime->sleep, &wait);
			if (signal_pending(current))
				return result > 0 ? result : -ERESTARTSYS;
			if (runtime->avail == last_avail && !timeout)
				return result > 0 ? result : -EIO;
			spin_lock_irq(&runtime->lock);
		}
		spin_unlock_irq(&runtime->lock);
	}
	return result;
}
```

**External callers of `snd_rawmidi_write` at parent**

- `drivers/usb/gadget/function/f_midi.c:626` — `	 * until a) usb requests have been completed or b) snd_rawmidi_write()`
