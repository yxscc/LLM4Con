/* Each registration idiom the entry discovery must recognise, without kernel
 * headers so the fixture compiles with a bare clang. */

struct file;

struct demo_ops {
	int owner;
	int (*open)(struct file *f);
	void (*release)(struct file *f);
};

struct work_struct {
	long data;
	void *entry[2];
	void (*func)(struct work_struct *w);
};

struct demo_dev {
	int counter;
	struct work_struct work;
};

extern void init_timer_key(void *timer, void (*fn)(void *), unsigned int flags);

static void shared_helper(void)
{
}

static int demo_open(struct file *f)
{
	shared_helper();
	return 0;
}

static void demo_release(struct file *f)
{
}

static void demo_ioctl(struct file *f)
{
}

static const struct demo_ops demo_fops = {
	.owner = 1,
	.open = demo_open,
	.release = demo_release,
};

static const struct demo_ops demo_ops_table[2] = {
	{ .open = demo_open },
	{ .release = demo_ioctl },
};

static void demo_work_fn(struct work_struct *w)
{
}

static void demo_timer_fn(void *t)
{
}

void demo_setup(struct demo_dev *d, void *timer)
{
	d->work.func = demo_work_fn;
	init_timer_key(timer, demo_timer_fn, 0);
	shared_helper();
}

long __do_sys_demo(long x)
{
	return x;
}

int demo_exported(void)
{
	return 1;
}

void *__ksymtab_demo_exported __attribute__((used)) = (void *)demo_exported;

int demo_exported_v7(void)
{
	return 2;
}

static void *__UNIQUE_ID_addressable_demo_exported_v7_12
	__attribute__((used, section(".discard.addressable"))) = (void *)demo_exported_v7;

static void demo_discarded_fn(void)
{
}

static void *demo_discard_marker
	__attribute__((used, section(".discard.misc"))) = (void *)demo_discarded_fn;

void demo_close_all(void)
{
	demo_release(0);
}

const struct demo_ops *demo_get_ops(int i)
{
	return i < 0 ? &demo_fops : &demo_ops_table[i];
}

struct demo_buf {
	int len;
	union {
		struct {
			void *head;
			void (*destructor)(struct demo_buf *b);
		};
		long raw[2];
	};
};

static void demo_buf_free(struct demo_buf *b)
{
}

void demo_set_destructor(struct demo_buf *b)
{
	b->destructor = demo_buf_free;
}

struct demo_notifier {
	int priority;
	int (*call)(void *data);
};

static int demo_notify(void *data)
{
	return 0;
}

static struct demo_notifier demo_nb;

void demo_register_notifier(void)
{
	demo_nb.call = demo_notify;
}

static inline __attribute__((always_inline)) void demo_timer_setup(void *t)
{
	init_timer_key(t, demo_timer_fn, 0);
}

void demo_arm(void *t)
{
	demo_timer_setup(t);
}

struct demo_mod {
	int count;
	void (*modfunc[4])(struct demo_dev *d);
};

static void demo_mod_fn(struct demo_dev *d)
{
}

void demo_add_mod(struct demo_mod *m, int i)
{
	m->modfunc[i] = demo_mod_fn;
	m->modfunc[2] = demo_mod_fn;
}

void demo_dispatch(int op, struct demo_dev *d)
{
	asm goto("" : : : : out);
out:
	switch (op) {
	case 1:
		shared_helper();
		break;
	case 2:
		d->counter++;
		break;
	}
	d->work.func = demo_work_fn;
}
