struct counter { int padding; int value; };
struct provider;
struct descriptor { int (*get_property)(struct provider *); };
struct provider { struct counter use_cnt; struct descriptor *desc; };
extern void instrument_read(const void *);

static __attribute__((always_inline)) inline int raw_read(struct counter *v) {
    volatile int scratch = 0;
    instrument_read((const void *)&scratch);
    return scratch + *(volatile int *)&v->value;
}
static __attribute__((always_inline)) inline int atomic_read(struct counter *v) {
    instrument_read(v);
    return raw_read(v);
}
static __attribute__((always_inline)) inline int atomic_dec_return(struct counter *v) {
    int delta = -1;
    __asm__ __volatile__("lock; xaddl %0, %1" : "+r" (delta), "+m" (v->value) : : "memory");
    return delta - 1;
}
int check_then_call(struct provider *psy) {
    if (atomic_read(&psy->use_cnt) <= 0) return 0; /* ATOMIC_READ */
    return psy->desc->get_property(psy); /* INDIRECT_CALL */
}
int retire(struct provider *psy) {
    return atomic_dec_return(&psy->use_cnt); /* ATOMIC_DEC */
}
int aliased(struct provider *incoming) {
    struct provider *psy = incoming;
    if (atomic_read(&psy->use_cnt) <= 0) return 0; /* ALIAS_INLINE */
    return psy->desc->get_property(psy); /* ALIAS_CALLEE */
}
int twice(struct provider *psy) {
    return atomic_read(&psy->use_cnt) + atomic_read(&psy->use_cnt); /* AMBIGUOUS_INLINE */
}
int repeated_field(struct provider *psy) {
    return psy->use_cnt.value + psy->use_cnt.value; /* AMBIGUOUS_FIELD */
}
int escaped_callback(struct provider *psy, void (*change)(int (**)(struct provider *))) {
    int (*callback)(struct provider *) = psy->desc->get_property;
    change(&callback);
    return callback(psy); /* ESCAPED_CALLBACK */
}
