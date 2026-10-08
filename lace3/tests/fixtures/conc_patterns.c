/* Concurrency shapes the v3 index and tasks must keep intact, written without
 * kernel headers. The lock and free primitives are left undefined so they stay
 * calls in the IR, as the kernel's out-of-line _raw_spin_lock and kfree do. */

typedef struct { int raw; } spinlock_t;

extern void _raw_spin_lock(spinlock_t *l);
extern void _raw_spin_unlock(spinlock_t *l);
extern void kfree(const void *p);
extern void *kmalloc(unsigned long size, unsigned int flags);
extern void refcount_inc(int *r);
extern int refcount_dec_and_test(int *r);

struct buf {
	int len;
	int ref;
	char data[16];
};

struct obj {
	spinlock_t lock;
	struct buf *buf;
	int count;
	int state;
};

struct ops {
	long (*consume)(struct obj *o);
	void (*close)(struct obj *o);
	void (*bump)(struct obj *o);
	void (*reset)(struct obj *o);
	void (*consume_ref)(struct obj *o);
};

/* --- lifetime: every field access is under o->lock, yet the pointer taken in
 * consume() is used after the unlock while close() frees it. */
static long use_buf(struct buf *b)
{
	return b->len;
}

static long obj_consume(struct obj *o)
{
	struct buf *p;

	_raw_spin_lock(&o->lock);
	p = o->buf;
	_raw_spin_unlock(&o->lock);
	return use_buf(p);
}

static void release_buf(struct buf *b)
{
	kfree(b);
}

static void obj_close(struct obj *o)
{
	struct buf *p;

	_raw_spin_lock(&o->lock);
	p = o->buf;
	o->buf = 0;
	_raw_spin_unlock(&o->lock);
	release_buf(p);
}

/* Same take-then-use shape, but a reference is taken under the lock. */
static void obj_consume_ref(struct obj *o)
{
	struct buf *p;

	_raw_spin_lock(&o->lock);
	p = o->buf;
	if (p)
		refcount_inc(&p->ref);
	_raw_spin_unlock(&o->lock);
	if (p) {
		use_buf(p);
		if (refcount_dec_and_test(&p->ref))
			kfree(p);
	}
}

/* --- atomicity: check and act are separate critical sections. */
static void obj_bump(struct obj *o)
{
	int c;

	_raw_spin_lock(&o->lock);
	c = o->count;
	_raw_spin_unlock(&o->lock);
	if (c < 8) {
		_raw_spin_lock(&o->lock);
		o->count = c + 1;
		_raw_spin_unlock(&o->lock);
	}
}

static void obj_reset(struct obj *o)
{
	_raw_spin_lock(&o->lock);
	o->count = 0;
	o->state = 1;
	_raw_spin_unlock(&o->lock);
}

/* --- plain data race: an unlocked writer of state. */
void obj_mark(struct obj *o)
{
	o->state = 2;
}

int obj_peek(struct obj *o)
{
	return o->state;
}

/* --- publication of a fresh buffer. */
void obj_attach(struct obj *o)
{
	struct buf *b = kmalloc(sizeof(*b), 0);

	b->len = 0;
	_raw_spin_lock(&o->lock);
	o->buf = b;
	_raw_spin_unlock(&o->lock);
}

const struct ops obj_ops = {
	.consume = obj_consume,
	.close = obj_close,
	.bump = obj_bump,
	.reset = obj_reset,
	.consume_ref = obj_consume_ref,
};

/* --- one critical section, two unlocks (early return). */
long obj_len_locked(struct obj *o)
{
	long n;

	_raw_spin_lock(&o->lock);
	if (!o->buf) {
		_raw_spin_unlock(&o->lock);
		return -1;
	}
	n = o->buf->len;
	_raw_spin_unlock(&o->lock);
	return n;
}

/* --- runs once at boot: no concurrent activation of itself. */
__attribute__((section(".init.text"))) int obj_setup(struct obj *o)
{
	o->count = 1;
	return 0;
}
