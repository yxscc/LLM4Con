typedef struct { int raw; } spinlock_t;
struct object { spinlock_t lock; int count; };
extern void _raw_spin_lock(spinlock_t *lock);
extern void _raw_spin_unlock(spinlock_t *lock);

static void helper(struct object *p)
{
    p->count = 1;
}

void locked(struct object *p)
{
    _raw_spin_lock(&p->lock);
    helper(p);
    _raw_spin_unlock(&p->lock);
}

void unlocked(struct object *p)
{
    helper(p);
}
