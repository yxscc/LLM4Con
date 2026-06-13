; ModuleID = '/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/kernel/notifier.c'
source_filename = "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/kernel/notifier.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_atomic_notifier_chain_register:\09\09\09\09\09"
module asm "\09.asciz \09\22atomic_notifier_chain_register\22\09\09\09\09\09"
module asm "__kstrtabns_atomic_notifier_chain_register:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+atomic_notifier_chain_register\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_atomic_notifier_chain_register:\09\09\09\09"
module asm "\09.long\09atomic_notifier_chain_register- .\09\09\09\09"
module asm "\09.long\09__kstrtab_atomic_notifier_chain_register- .\09\09\09"
module asm "\09.long\09__kstrtabns_atomic_notifier_chain_register- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_atomic_notifier_chain_unregister:\09\09\09\09\09"
module asm "\09.asciz \09\22atomic_notifier_chain_unregister\22\09\09\09\09\09"
module asm "__kstrtabns_atomic_notifier_chain_unregister:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+atomic_notifier_chain_unregister\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_atomic_notifier_chain_unregister:\09\09\09\09"
module asm "\09.long\09atomic_notifier_chain_unregister- .\09\09\09\09"
module asm "\09.long\09__kstrtab_atomic_notifier_chain_unregister- .\09\09\09"
module asm "\09.long\09__kstrtabns_atomic_notifier_chain_unregister- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_atomic_notifier_call_chain:\09\09\09\09\09"
module asm "\09.asciz \09\22atomic_notifier_call_chain\22\09\09\09\09\09"
module asm "__kstrtabns_atomic_notifier_call_chain:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+atomic_notifier_call_chain\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_atomic_notifier_call_chain:\09\09\09\09"
module asm "\09.long\09atomic_notifier_call_chain- .\09\09\09\09"
module asm "\09.long\09__kstrtab_atomic_notifier_call_chain- .\09\09\09"
module asm "\09.long\09__kstrtabns_atomic_notifier_call_chain- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_blocking_notifier_chain_register:\09\09\09\09\09"
module asm "\09.asciz \09\22blocking_notifier_chain_register\22\09\09\09\09\09"
module asm "__kstrtabns_blocking_notifier_chain_register:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+blocking_notifier_chain_register\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_blocking_notifier_chain_register:\09\09\09\09"
module asm "\09.long\09blocking_notifier_chain_register- .\09\09\09\09"
module asm "\09.long\09__kstrtab_blocking_notifier_chain_register- .\09\09\09"
module asm "\09.long\09__kstrtabns_blocking_notifier_chain_register- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_blocking_notifier_chain_unregister:\09\09\09\09\09"
module asm "\09.asciz \09\22blocking_notifier_chain_unregister\22\09\09\09\09\09"
module asm "__kstrtabns_blocking_notifier_chain_unregister:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+blocking_notifier_chain_unregister\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_blocking_notifier_chain_unregister:\09\09\09\09"
module asm "\09.long\09blocking_notifier_chain_unregister- .\09\09\09\09"
module asm "\09.long\09__kstrtab_blocking_notifier_chain_unregister- .\09\09\09"
module asm "\09.long\09__kstrtabns_blocking_notifier_chain_unregister- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_blocking_notifier_call_chain_robust:\09\09\09\09\09"
module asm "\09.asciz \09\22blocking_notifier_call_chain_robust\22\09\09\09\09\09"
module asm "__kstrtabns_blocking_notifier_call_chain_robust:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+blocking_notifier_call_chain_robust\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_blocking_notifier_call_chain_robust:\09\09\09\09"
module asm "\09.long\09blocking_notifier_call_chain_robust- .\09\09\09\09"
module asm "\09.long\09__kstrtab_blocking_notifier_call_chain_robust- .\09\09\09"
module asm "\09.long\09__kstrtabns_blocking_notifier_call_chain_robust- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_blocking_notifier_call_chain:\09\09\09\09\09"
module asm "\09.asciz \09\22blocking_notifier_call_chain\22\09\09\09\09\09"
module asm "__kstrtabns_blocking_notifier_call_chain:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+blocking_notifier_call_chain\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_blocking_notifier_call_chain:\09\09\09\09"
module asm "\09.long\09blocking_notifier_call_chain- .\09\09\09\09"
module asm "\09.long\09__kstrtab_blocking_notifier_call_chain- .\09\09\09"
module asm "\09.long\09__kstrtabns_blocking_notifier_call_chain- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_raw_notifier_chain_register:\09\09\09\09\09"
module asm "\09.asciz \09\22raw_notifier_chain_register\22\09\09\09\09\09"
module asm "__kstrtabns_raw_notifier_chain_register:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+raw_notifier_chain_register\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_raw_notifier_chain_register:\09\09\09\09"
module asm "\09.long\09raw_notifier_chain_register- .\09\09\09\09"
module asm "\09.long\09__kstrtab_raw_notifier_chain_register- .\09\09\09"
module asm "\09.long\09__kstrtabns_raw_notifier_chain_register- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_raw_notifier_chain_unregister:\09\09\09\09\09"
module asm "\09.asciz \09\22raw_notifier_chain_unregister\22\09\09\09\09\09"
module asm "__kstrtabns_raw_notifier_chain_unregister:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+raw_notifier_chain_unregister\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_raw_notifier_chain_unregister:\09\09\09\09"
module asm "\09.long\09raw_notifier_chain_unregister- .\09\09\09\09"
module asm "\09.long\09__kstrtab_raw_notifier_chain_unregister- .\09\09\09"
module asm "\09.long\09__kstrtabns_raw_notifier_chain_unregister- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_raw_notifier_call_chain_robust:\09\09\09\09\09"
module asm "\09.asciz \09\22raw_notifier_call_chain_robust\22\09\09\09\09\09"
module asm "__kstrtabns_raw_notifier_call_chain_robust:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+raw_notifier_call_chain_robust\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_raw_notifier_call_chain_robust:\09\09\09\09"
module asm "\09.long\09raw_notifier_call_chain_robust- .\09\09\09\09"
module asm "\09.long\09__kstrtab_raw_notifier_call_chain_robust- .\09\09\09"
module asm "\09.long\09__kstrtabns_raw_notifier_call_chain_robust- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_raw_notifier_call_chain:\09\09\09\09\09"
module asm "\09.asciz \09\22raw_notifier_call_chain\22\09\09\09\09\09"
module asm "__kstrtabns_raw_notifier_call_chain:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+raw_notifier_call_chain\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_raw_notifier_call_chain:\09\09\09\09"
module asm "\09.long\09raw_notifier_call_chain- .\09\09\09\09"
module asm "\09.long\09__kstrtab_raw_notifier_call_chain- .\09\09\09"
module asm "\09.long\09__kstrtabns_raw_notifier_call_chain- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_srcu_notifier_chain_register:\09\09\09\09\09"
module asm "\09.asciz \09\22srcu_notifier_chain_register\22\09\09\09\09\09"
module asm "__kstrtabns_srcu_notifier_chain_register:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+srcu_notifier_chain_register\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_srcu_notifier_chain_register:\09\09\09\09"
module asm "\09.long\09srcu_notifier_chain_register- .\09\09\09\09"
module asm "\09.long\09__kstrtab_srcu_notifier_chain_register- .\09\09\09"
module asm "\09.long\09__kstrtabns_srcu_notifier_chain_register- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_srcu_notifier_chain_unregister:\09\09\09\09\09"
module asm "\09.asciz \09\22srcu_notifier_chain_unregister\22\09\09\09\09\09"
module asm "__kstrtabns_srcu_notifier_chain_unregister:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+srcu_notifier_chain_unregister\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_srcu_notifier_chain_unregister:\09\09\09\09"
module asm "\09.long\09srcu_notifier_chain_unregister- .\09\09\09\09"
module asm "\09.long\09__kstrtab_srcu_notifier_chain_unregister- .\09\09\09"
module asm "\09.long\09__kstrtabns_srcu_notifier_chain_unregister- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_srcu_notifier_call_chain:\09\09\09\09\09"
module asm "\09.asciz \09\22srcu_notifier_call_chain\22\09\09\09\09\09"
module asm "__kstrtabns_srcu_notifier_call_chain:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+srcu_notifier_call_chain\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_srcu_notifier_call_chain:\09\09\09\09"
module asm "\09.long\09srcu_notifier_call_chain- .\09\09\09\09"
module asm "\09.long\09__kstrtab_srcu_notifier_call_chain- .\09\09\09"
module asm "\09.long\09__kstrtabns_srcu_notifier_call_chain- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_srcu_init_notifier_head:\09\09\09\09\09"
module asm "\09.asciz \09\22srcu_init_notifier_head\22\09\09\09\09\09"
module asm "__kstrtabns_srcu_init_notifier_head:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+srcu_init_notifier_head\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_srcu_init_notifier_head:\09\09\09\09"
module asm "\09.long\09srcu_init_notifier_head- .\09\09\09\09"
module asm "\09.long\09__kstrtab_srcu_init_notifier_head- .\09\09\09"
module asm "\09.long\09__kstrtabns_srcu_init_notifier_head- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_register_die_notifier:\09\09\09\09\09"
module asm "\09.asciz \09\22register_die_notifier\22\09\09\09\09\09"
module asm "__kstrtabns_register_die_notifier:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+register_die_notifier\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_register_die_notifier:\09\09\09\09"
module asm "\09.long\09register_die_notifier- .\09\09\09\09"
module asm "\09.long\09__kstrtab_register_die_notifier- .\09\09\09"
module asm "\09.long\09__kstrtabns_register_die_notifier- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"
module asm "\09.section \22__ksymtab_strings\22,\22aMS\22,%progbits,1\09"
module asm "__kstrtab_unregister_die_notifier:\09\09\09\09\09"
module asm "\09.asciz \09\22unregister_die_notifier\22\09\09\09\09\09"
module asm "__kstrtabns_unregister_die_notifier:\09\09\09\09\09"
module asm "\09.asciz \09\22\22\09\09\09\09\09"
module asm "\09.previous\09\09\09\09\09\09"
module asm "\09.section \22___ksymtab_gpl+unregister_die_notifier\22, \22a\22\09"
module asm "\09.balign\094\09\09\09\09\09"
module asm "__ksymtab_unregister_die_notifier:\09\09\09\09"
module asm "\09.long\09unregister_die_notifier- .\09\09\09\09"
module asm "\09.long\09__kstrtab_unregister_die_notifier- .\09\09\09"
module asm "\09.long\09__kstrtabns_unregister_die_notifier- .\09\09\09"
module asm "\09.previous\09\09\09\09\09"

%struct.blocking_notifier_head = type { %struct.rw_semaphore, ptr }
%struct.rw_semaphore = type { %struct.atomic64_t, %struct.atomic64_t, %struct.optimistic_spin_queue, %struct.raw_spinlock, %struct.list_head }
%struct.atomic64_t = type { i64 }
%struct.optimistic_spin_queue = type { %struct.atomic_t }
%struct.atomic_t = type { i32 }
%struct.raw_spinlock = type { %struct.qspinlock }
%struct.qspinlock = type { %union.anon }
%union.anon = type { %struct.atomic_t }
%struct.list_head = type { ptr, ptr }
%struct.lock_class_key = type {}
%struct.atomic_notifier_head = type { %struct.spinlock, ptr }
%struct.spinlock = type { %union.anon.1 }
%union.anon.1 = type { %struct.raw_spinlock }
%struct.notifier_block = type { ptr, ptr, i32 }
%struct.raw_notifier_head = type { ptr }
%struct.srcu_notifier_head = type { %struct.mutex, %struct.srcu_struct, ptr }
%struct.mutex = type { %struct.atomic64_t, %struct.raw_spinlock, %struct.optimistic_spin_queue, %struct.list_head }
%struct.srcu_struct = type { [5 x %struct.srcu_node], [3 x ptr], %struct.mutex, %struct.spinlock, %struct.mutex, i32, i64, i64, i64, i64, ptr, i64, %struct.mutex, %struct.completion, %struct.atomic_t, %struct.delayed_work, %struct.lockdep_map }
%struct.srcu_node = type { %struct.spinlock, [4 x i64], [4 x i64], i64, ptr, i32, i32 }
%struct.completion = type { i32, %struct.swait_queue_head }
%struct.swait_queue_head = type { %struct.raw_spinlock, %struct.list_head }
%struct.delayed_work = type { %struct.work_struct, %struct.timer_list, ptr, i32 }
%struct.work_struct = type { %struct.atomic64_t, %struct.list_head, ptr }
%struct.timer_list = type { %struct.hlist_node, i64, ptr, i32 }
%struct.hlist_node = type { ptr, ptr }
%struct.lockdep_map = type {}
%struct.die_args = type { ptr, ptr, i64, i32, i32 }

@reboot_notifier_list = dso_local global %struct.blocking_notifier_head { %struct.rw_semaphore { %struct.atomic64_t zeroinitializer, %struct.atomic64_t zeroinitializer, %struct.optimistic_spin_queue zeroinitializer, %struct.raw_spinlock zeroinitializer, %struct.list_head { ptr getelementptr (i8, ptr @reboot_notifier_list, i64 24), ptr getelementptr (i8, ptr @reboot_notifier_list, i64 24) } }, ptr null }, align 8, !dbg !0
@_kbl_addr_notifier_call_chain = internal global i64 ptrtoint (ptr @notifier_call_chain to i64), section "_kprobe_blacklist", align 8, !dbg !54
@__UNIQUE_ID___addressable_atomic_notifier_chain_register252 = internal global ptr @atomic_notifier_chain_register, section ".discard.addressable", align 8, !dbg !57
@__UNIQUE_ID___addressable_atomic_notifier_chain_unregister253 = internal global ptr @atomic_notifier_chain_unregister, section ".discard.addressable", align 8, !dbg !59
@__UNIQUE_ID___addressable_atomic_notifier_call_chain254 = internal global ptr @atomic_notifier_call_chain, section ".discard.addressable", align 8, !dbg !61
@_kbl_addr_atomic_notifier_call_chain = internal global i64 ptrtoint (ptr @atomic_notifier_call_chain to i64), section "_kprobe_blacklist", align 8, !dbg !63
@system_state = external dso_local global i32, align 4
@__UNIQUE_ID___addressable_blocking_notifier_chain_register255 = internal global ptr @blocking_notifier_chain_register, section ".discard.addressable", align 8, !dbg !65
@__UNIQUE_ID___addressable_blocking_notifier_chain_unregister256 = internal global ptr @blocking_notifier_chain_unregister, section ".discard.addressable", align 8, !dbg !67
@__UNIQUE_ID___addressable_blocking_notifier_call_chain_robust258 = internal global ptr @blocking_notifier_call_chain_robust, section ".discard.addressable", align 8, !dbg !69
@__UNIQUE_ID___addressable_blocking_notifier_call_chain260 = internal global ptr @blocking_notifier_call_chain, section ".discard.addressable", align 8, !dbg !71
@__UNIQUE_ID___addressable_raw_notifier_chain_register261 = internal global ptr @raw_notifier_chain_register, section ".discard.addressable", align 8, !dbg !73
@__UNIQUE_ID___addressable_raw_notifier_chain_unregister262 = internal global ptr @raw_notifier_chain_unregister, section ".discard.addressable", align 8, !dbg !75
@__UNIQUE_ID___addressable_raw_notifier_call_chain_robust263 = internal global ptr @raw_notifier_call_chain_robust, section ".discard.addressable", align 8, !dbg !77
@__UNIQUE_ID___addressable_raw_notifier_call_chain264 = internal global ptr @raw_notifier_call_chain, section ".discard.addressable", align 8, !dbg !79
@__UNIQUE_ID___addressable_srcu_notifier_chain_register265 = internal global ptr @srcu_notifier_chain_register, section ".discard.addressable", align 8, !dbg !81
@__UNIQUE_ID___addressable_srcu_notifier_chain_unregister266 = internal global ptr @srcu_notifier_chain_unregister, section ".discard.addressable", align 8, !dbg !83
@__UNIQUE_ID___addressable_srcu_notifier_call_chain267 = internal global ptr @srcu_notifier_call_chain, section ".discard.addressable", align 8, !dbg !85
@srcu_init_notifier_head.__key = internal global %struct.lock_class_key zeroinitializer, align 1, !dbg !87
@.str = private unnamed_addr constant [11 x i8] c"&nh->mutex\00", align 1, !dbg !301
@.str.1 = private unnamed_addr constant [61 x i8] c"/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/kernel/notifier.c\00", align 1, !dbg !307
@__UNIQUE_ID___addressable_srcu_init_notifier_head269 = internal global ptr @srcu_init_notifier_head, section ".discard.addressable", align 8, !dbg !312
@die_chain = internal global %struct.atomic_notifier_head zeroinitializer, align 8, !dbg !331
@_kbl_addr_notify_die = internal global i64 ptrtoint (ptr @notify_die to i64), section "_kprobe_blacklist", align 8, !dbg !314
@__UNIQUE_ID___addressable_register_die_notifier270 = internal global ptr @register_die_notifier, section ".discard.addressable", align 8, !dbg !316
@__UNIQUE_ID___addressable_unregister_die_notifier271 = internal global ptr @unregister_die_notifier, section ".discard.addressable", align 8, !dbg !318
@.str.2 = private unnamed_addr constant [41 x i8] c"notifier callback %ps already registered\00", align 1, !dbg !320
@.str.3 = private unnamed_addr constant [64 x i8] c"/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/srcu.h\00", align 1, !dbg !325
@llvm.compiler.used = appending global [20 x ptr] [ptr @_kbl_addr_notifier_call_chain, ptr @__UNIQUE_ID___addressable_atomic_notifier_chain_register252, ptr @__UNIQUE_ID___addressable_atomic_notifier_chain_unregister253, ptr @__UNIQUE_ID___addressable_atomic_notifier_call_chain254, ptr @_kbl_addr_atomic_notifier_call_chain, ptr @__UNIQUE_ID___addressable_blocking_notifier_chain_register255, ptr @__UNIQUE_ID___addressable_blocking_notifier_chain_unregister256, ptr @__UNIQUE_ID___addressable_blocking_notifier_call_chain_robust258, ptr @__UNIQUE_ID___addressable_blocking_notifier_call_chain260, ptr @__UNIQUE_ID___addressable_raw_notifier_chain_register261, ptr @__UNIQUE_ID___addressable_raw_notifier_chain_unregister262, ptr @__UNIQUE_ID___addressable_raw_notifier_call_chain_robust263, ptr @__UNIQUE_ID___addressable_raw_notifier_call_chain264, ptr @__UNIQUE_ID___addressable_srcu_notifier_chain_register265, ptr @__UNIQUE_ID___addressable_srcu_notifier_chain_unregister266, ptr @__UNIQUE_ID___addressable_srcu_notifier_call_chain267, ptr @__UNIQUE_ID___addressable_srcu_init_notifier_head269, ptr @_kbl_addr_notify_die, ptr @__UNIQUE_ID___addressable_register_die_notifier270, ptr @__UNIQUE_ID___addressable_unregister_die_notifier271], section "llvm.metadata"

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @notifier_call_chain(ptr noundef %0, i64 noundef %1, ptr noundef %2, i32 noundef %3, ptr noundef %4) #0 !dbg !355 {
  %6 = alloca ptr, align 8
  %7 = alloca i64, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i32, align 4
  %10 = alloca ptr, align 8
  %11 = alloca i32, align 4
  %12 = alloca ptr, align 8
  %13 = alloca ptr, align 8
  %14 = alloca ptr, align 8
  %15 = alloca ptr, align 8
  %16 = alloca ptr, align 8
  %17 = alloca ptr, align 8
  %18 = alloca ptr, align 8
  %19 = alloca ptr, align 8
  store ptr %0, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !360, metadata !DIExpression()), !dbg !361
  store i64 %1, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !362, metadata !DIExpression()), !dbg !363
  store ptr %2, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !364, metadata !DIExpression()), !dbg !365
  store i32 %3, ptr %9, align 4
  call void @llvm.dbg.declare(metadata ptr %9, metadata !366, metadata !DIExpression()), !dbg !367
  store ptr %4, ptr %10, align 8
  call void @llvm.dbg.declare(metadata ptr %10, metadata !368, metadata !DIExpression()), !dbg !369
  call void @llvm.dbg.declare(metadata ptr %11, metadata !370, metadata !DIExpression()), !dbg !371
  store i32 0, ptr %11, align 4, !dbg !371
  call void @llvm.dbg.declare(metadata ptr %12, metadata !372, metadata !DIExpression()), !dbg !373
  call void @llvm.dbg.declare(metadata ptr %13, metadata !374, metadata !DIExpression()), !dbg !375
  call void @llvm.dbg.declare(metadata ptr %14, metadata !376, metadata !DIExpression()), !dbg !378
  br label %20, !dbg !379

20:                                               ; preds = %5
  br label %21, !dbg !381

21:                                               ; preds = %20
  %22 = load ptr, ptr %6, align 8, !dbg !379
  %23 = load volatile ptr, ptr %22, align 8, !dbg !379
  store ptr %23, ptr %15, align 8, !dbg !381
  %24 = load ptr, ptr %15, align 8, !dbg !379
  store ptr %24, ptr %14, align 8, !dbg !378
  %25 = load ptr, ptr %14, align 8, !dbg !378
  store ptr %25, ptr %16, align 8, !dbg !378
  %26 = load ptr, ptr %16, align 8, !dbg !378
  store ptr %26, ptr %12, align 8, !dbg !383
  br label %27, !dbg !384

27:                                               ; preds = %62, %21
  %28 = load ptr, ptr %12, align 8, !dbg !385
  %29 = icmp ne ptr %28, null, !dbg !385
  br i1 %29, label %30, label %33, !dbg !386

30:                                               ; preds = %27
  %31 = load i32, ptr %9, align 4, !dbg !387
  %32 = icmp ne i32 %31, 0, !dbg !386
  br label %33

33:                                               ; preds = %30, %27
  %34 = phi i1 [ false, %27 ], [ %32, %30 ], !dbg !388
  br i1 %34, label %35, label %66, !dbg !384

35:                                               ; preds = %33
  call void @llvm.dbg.declare(metadata ptr %17, metadata !389, metadata !DIExpression()), !dbg !392
  br label %36, !dbg !393

36:                                               ; preds = %35
  br label %37, !dbg !395

37:                                               ; preds = %36
  %38 = load ptr, ptr %12, align 8, !dbg !393
  %39 = getelementptr inbounds %struct.notifier_block, ptr %38, i32 0, i32 1, !dbg !393
  %40 = load volatile ptr, ptr %39, align 8, !dbg !393
  store ptr %40, ptr %18, align 8, !dbg !395
  %41 = load ptr, ptr %18, align 8, !dbg !393
  store ptr %41, ptr %17, align 8, !dbg !392
  %42 = load ptr, ptr %17, align 8, !dbg !392
  store ptr %42, ptr %19, align 8, !dbg !392
  %43 = load ptr, ptr %19, align 8, !dbg !392
  store ptr %43, ptr %13, align 8, !dbg !397
  %44 = load ptr, ptr %12, align 8, !dbg !398
  %45 = getelementptr inbounds %struct.notifier_block, ptr %44, i32 0, i32 0, !dbg !399
  %46 = load ptr, ptr %45, align 8, !dbg !399
  %47 = load ptr, ptr %12, align 8, !dbg !400
  %48 = load i64, ptr %7, align 8, !dbg !401
  %49 = load ptr, ptr %8, align 8, !dbg !402
  %50 = call i32 %46(ptr noundef %47, i64 noundef %48, ptr noundef %49), !dbg !398
  store i32 %50, ptr %11, align 4, !dbg !403
  %51 = load ptr, ptr %10, align 8, !dbg !404
  %52 = icmp ne ptr %51, null, !dbg !404
  br i1 %52, label %53, label %57, !dbg !406

53:                                               ; preds = %37
  %54 = load ptr, ptr %10, align 8, !dbg !407
  %55 = load i32, ptr %54, align 4, !dbg !408
  %56 = add nsw i32 %55, 1, !dbg !408
  store i32 %56, ptr %54, align 4, !dbg !408
  br label %57, !dbg !409

57:                                               ; preds = %53, %37
  %58 = load i32, ptr %11, align 4, !dbg !410
  %59 = and i32 %58, 32768, !dbg !412
  %60 = icmp ne i32 %59, 0, !dbg !412
  br i1 %60, label %61, label %62, !dbg !413

61:                                               ; preds = %57
  br label %66, !dbg !414

62:                                               ; preds = %57
  %63 = load ptr, ptr %13, align 8, !dbg !415
  store ptr %63, ptr %12, align 8, !dbg !416
  %64 = load i32, ptr %9, align 4, !dbg !417
  %65 = add nsw i32 %64, -1, !dbg !417
  store i32 %65, ptr %9, align 4, !dbg !417
  br label %27, !dbg !384, !llvm.loop !418

66:                                               ; preds = %61, %33
  %67 = load i32, ptr %11, align 4, !dbg !421
  ret i32 %67, !dbg !422
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @atomic_notifier_chain_register(ptr noundef %0, ptr noundef %1) #0 !dbg !423 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i64, align 8
  %10 = alloca i32, align 4
  %11 = alloca i64, align 8
  %12 = alloca i64, align 8
  %13 = alloca i32, align 4
  store ptr %0, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !427, metadata !DIExpression()), !dbg !428
  store ptr %1, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !429, metadata !DIExpression()), !dbg !430
  call void @llvm.dbg.declare(metadata ptr %9, metadata !431, metadata !DIExpression()), !dbg !432
  call void @llvm.dbg.declare(metadata ptr %10, metadata !433, metadata !DIExpression()), !dbg !434
  br label %14, !dbg !435

14:                                               ; preds = %2
  br label %15, !dbg !436

15:                                               ; preds = %14
  call void @llvm.dbg.declare(metadata ptr %11, metadata !438, metadata !DIExpression()), !dbg !441
  call void @llvm.dbg.declare(metadata ptr %12, metadata !442, metadata !DIExpression()), !dbg !441
  %16 = icmp eq ptr %11, %12, !dbg !441
  %17 = zext i1 %16 to i32, !dbg !441
  store i32 1, ptr %13, align 4, !dbg !441
  %18 = load i32, ptr %13, align 4, !dbg !441
  %19 = load ptr, ptr %7, align 8, !dbg !443
  %20 = getelementptr inbounds %struct.atomic_notifier_head, ptr %19, i32 0, i32 0, !dbg !443
  store ptr %20, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !444, metadata !DIExpression()), !dbg !451
  %21 = load ptr, ptr %6, align 8, !dbg !453
  %22 = call i64 @_raw_spin_lock_irqsave(ptr noundef %21), !dbg !443
  store i64 %22, ptr %9, align 8, !dbg !443
  br label %23, !dbg !443

23:                                               ; preds = %15
  br label %24, !dbg !436

24:                                               ; preds = %23
  %25 = load ptr, ptr %7, align 8, !dbg !454
  %26 = getelementptr inbounds %struct.atomic_notifier_head, ptr %25, i32 0, i32 1, !dbg !455
  %27 = load ptr, ptr %8, align 8, !dbg !456
  %28 = call i32 @notifier_chain_register(ptr noundef %26, ptr noundef %27), !dbg !457
  store i32 %28, ptr %10, align 4, !dbg !458
  %29 = load ptr, ptr %7, align 8, !dbg !459
  %30 = getelementptr inbounds %struct.atomic_notifier_head, ptr %29, i32 0, i32 0, !dbg !460
  %31 = load i64, ptr %9, align 8, !dbg !461
  store ptr %30, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !462, metadata !DIExpression()), !dbg !466
  store i64 %31, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !468, metadata !DIExpression()), !dbg !469
  call void @llvm.dbg.declare(metadata ptr undef, metadata !470, metadata !DIExpression()), !dbg !473
  call void @llvm.dbg.declare(metadata ptr undef, metadata !474, metadata !DIExpression()), !dbg !473
  store i32 1, ptr %5, align 4, !dbg !473
  %32 = load i32, ptr %5, align 4, !dbg !473
  %33 = load ptr, ptr %3, align 8, !dbg !475
  %34 = load i64, ptr %4, align 8, !dbg !475
  call void @_raw_spin_unlock_irqrestore(ptr noundef %33, i64 noundef %34) #3, !dbg !475
  %35 = load i32, ptr %10, align 4, !dbg !476
  ret i32 %35, !dbg !477
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare dso_local i64 @_raw_spin_lock_irqsave(ptr noundef) #2 section ".spinlock.text"

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @notifier_chain_register(ptr noundef %0, ptr noundef %1) #0 !dbg !478 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !481, metadata !DIExpression()), !dbg !482
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !483, metadata !DIExpression()), !dbg !484
  br label %9, !dbg !485

9:                                                ; preds = %67, %2
  %10 = load ptr, ptr %4, align 8, !dbg !486
  %11 = load ptr, ptr %10, align 8, !dbg !487
  %12 = icmp ne ptr %11, null, !dbg !488
  br i1 %12, label %13, label %71, !dbg !485

13:                                               ; preds = %9
  %14 = load ptr, ptr %4, align 8, !dbg !489
  %15 = load ptr, ptr %14, align 8, !dbg !489
  %16 = load ptr, ptr %5, align 8, !dbg !489
  %17 = icmp eq ptr %15, %16, !dbg !489
  %18 = xor i1 %17, true, !dbg !489
  %19 = xor i1 %18, true, !dbg !489
  %20 = zext i1 %19 to i32, !dbg !489
  %21 = sext i32 %20 to i64, !dbg !489
  %22 = icmp ne i64 %21, 0, !dbg !489
  br i1 %22, label %23, label %57, !dbg !492

23:                                               ; preds = %13
  call void @llvm.dbg.declare(metadata ptr %6, metadata !493, metadata !DIExpression()), !dbg !496
  store i32 1, ptr %6, align 4, !dbg !496
  %24 = load i32, ptr %6, align 4, !dbg !497
  %25 = icmp ne i32 %24, 0, !dbg !497
  %26 = xor i1 %25, true, !dbg !497
  %27 = xor i1 %26, true, !dbg !497
  %28 = zext i1 %27 to i32, !dbg !497
  %29 = sext i32 %28 to i64, !dbg !497
  %30 = icmp ne i64 %29, 0, !dbg !497
  br i1 %30, label %31, label %49, !dbg !496

31:                                               ; preds = %23
  br label %32, !dbg !497

32:                                               ; preds = %31
  br label %33, !dbg !499

33:                                               ; preds = %32
  br label %34, !dbg !501

34:                                               ; preds = %33
  %35 = load ptr, ptr %5, align 8, !dbg !499
  %36 = getelementptr inbounds %struct.notifier_block, ptr %35, i32 0, i32 0, !dbg !499
  %37 = load ptr, ptr %36, align 8, !dbg !499
  call void (ptr, ...) @__warn_printk(ptr noundef @.str.2, ptr noundef %37), !dbg !499
  br label %38, !dbg !499

38:                                               ; preds = %34
  br label %39, !dbg !503

39:                                               ; preds = %38
  br label %40, !dbg !505

40:                                               ; preds = %39
  br label %41, !dbg !503

41:                                               ; preds = %40
  call void asm sideeffect "1:\09.byte 0x0f, 0x0b\0A.pushsection __bug_table,\22aw\22\0A2:\09.long 1b - 2b\09# bug_entry::bug_addr\0A\09.long ${0:c} - 2b\09# bug_entry::file\0A\09.word ${1:c}\09# bug_entry::line\0A\09.word ${2:c}\09# bug_entry::flags\0A\09.org 2b+${3:c}\0A.popsection", "i,i,i,i,~{dirflag},~{fpsr},~{flags}"(ptr @.str.1, i32 28, i32 2313, i64 12) #3, !dbg !507, !srcloc !509
  br label %42, !dbg !507

42:                                               ; preds = %41
  call void asm sideeffect "243:\0A\09.pushsection .discard.reachable\0A\09.long 243b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 243) #3, !dbg !510, !srcloc !512
  br label %43, !dbg !503

43:                                               ; preds = %42
  br label %44, !dbg !513

44:                                               ; preds = %43
  br label %45, !dbg !503

45:                                               ; preds = %44
  br label %46, !dbg !499

46:                                               ; preds = %45
  br label %47, !dbg !515

47:                                               ; preds = %46
  br label %48, !dbg !499

48:                                               ; preds = %47
  br label %49, !dbg !499

49:                                               ; preds = %48, %23
  %50 = load i32, ptr %6, align 4, !dbg !496
  %51 = icmp ne i32 %50, 0, !dbg !496
  %52 = xor i1 %51, true, !dbg !496
  %53 = xor i1 %52, true, !dbg !496
  %54 = zext i1 %53 to i32, !dbg !496
  %55 = sext i32 %54 to i64, !dbg !496
  store i64 %55, ptr %7, align 8, !dbg !497
  %56 = load i64, ptr %7, align 8, !dbg !496
  store i32 -17, ptr %3, align 4, !dbg !517
  br label %108, !dbg !517

57:                                               ; preds = %13
  %58 = load ptr, ptr %5, align 8, !dbg !518
  %59 = getelementptr inbounds %struct.notifier_block, ptr %58, i32 0, i32 2, !dbg !520
  %60 = load i32, ptr %59, align 8, !dbg !520
  %61 = load ptr, ptr %4, align 8, !dbg !521
  %62 = load ptr, ptr %61, align 8, !dbg !522
  %63 = getelementptr inbounds %struct.notifier_block, ptr %62, i32 0, i32 2, !dbg !523
  %64 = load i32, ptr %63, align 8, !dbg !523
  %65 = icmp sgt i32 %60, %64, !dbg !524
  br i1 %65, label %66, label %67, !dbg !525

66:                                               ; preds = %57
  br label %71, !dbg !526

67:                                               ; preds = %57
  %68 = load ptr, ptr %4, align 8, !dbg !527
  %69 = load ptr, ptr %68, align 8, !dbg !528
  %70 = getelementptr inbounds %struct.notifier_block, ptr %69, i32 0, i32 1, !dbg !529
  store ptr %70, ptr %4, align 8, !dbg !530
  br label %9, !dbg !485, !llvm.loop !531

71:                                               ; preds = %66, %9
  %72 = load ptr, ptr %4, align 8, !dbg !533
  %73 = load ptr, ptr %72, align 8, !dbg !534
  %74 = load ptr, ptr %5, align 8, !dbg !535
  %75 = getelementptr inbounds %struct.notifier_block, ptr %74, i32 0, i32 1, !dbg !536
  store ptr %73, ptr %75, align 8, !dbg !537
  br label %76, !dbg !538

76:                                               ; preds = %71
  call void @llvm.dbg.declare(metadata ptr %8, metadata !539, metadata !DIExpression()), !dbg !541
  %77 = load ptr, ptr %5, align 8, !dbg !541
  %78 = ptrtoint ptr %77 to i64, !dbg !541
  store i64 %78, ptr %8, align 8, !dbg !541
  br i1 false, label %79, label %92, !dbg !542

79:                                               ; preds = %76
  %80 = load i64, ptr %8, align 8, !dbg !542
  %81 = icmp eq i64 %80, 0, !dbg !542
  br i1 %81, label %82, label %92, !dbg !541

82:                                               ; preds = %79
  br label %83, !dbg !542

83:                                               ; preds = %82
  br label %84, !dbg !544

84:                                               ; preds = %83
  br label %85, !dbg !546

85:                                               ; preds = %84
  br label %86, !dbg !544

86:                                               ; preds = %85
  %87 = load i64, ptr %8, align 8, !dbg !548
  %88 = inttoptr i64 %87 to ptr, !dbg !548
  %89 = load ptr, ptr %4, align 8, !dbg !548
  store volatile ptr %88, ptr %89, align 8, !dbg !548
  br label %90, !dbg !548

90:                                               ; preds = %86
  br label %91, !dbg !544

91:                                               ; preds = %90
  br label %106, !dbg !544

92:                                               ; preds = %79, %76
  br label %93, !dbg !542

93:                                               ; preds = %92
  br label %94, !dbg !550

94:                                               ; preds = %93
  br label %95, !dbg !552

95:                                               ; preds = %94
  call void asm sideeffect "", "~{memory},~{dirflag},~{fpsr},~{flags}"() #3, !dbg !550, !srcloc !554
  br label %96, !dbg !550

96:                                               ; preds = %95
  br label %97, !dbg !555

97:                                               ; preds = %96
  br label %98, !dbg !557

98:                                               ; preds = %97
  br label %99, !dbg !555

99:                                               ; preds = %98
  %100 = load i64, ptr %8, align 8, !dbg !559
  %101 = inttoptr i64 %100 to ptr, !dbg !559
  %102 = load ptr, ptr %4, align 8, !dbg !559
  store volatile ptr %101, ptr %102, align 8, !dbg !559
  br label %103, !dbg !559

103:                                              ; preds = %99
  br label %104, !dbg !555

104:                                              ; preds = %103
  br label %105, !dbg !550

105:                                              ; preds = %104
  br label %106

106:                                              ; preds = %105, %91
  br label %107, !dbg !541

107:                                              ; preds = %106
  store i32 0, ptr %3, align 4, !dbg !561
  br label %108, !dbg !561

108:                                              ; preds = %107, %49
  %109 = load i32, ptr %3, align 4, !dbg !562
  ret i32 %109, !dbg !562
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @atomic_notifier_chain_unregister(ptr noundef %0, ptr noundef %1) #0 !dbg !563 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i64, align 8
  %10 = alloca i32, align 4
  %11 = alloca i64, align 8
  %12 = alloca i64, align 8
  %13 = alloca i32, align 4
  store ptr %0, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !564, metadata !DIExpression()), !dbg !565
  store ptr %1, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !566, metadata !DIExpression()), !dbg !567
  call void @llvm.dbg.declare(metadata ptr %9, metadata !568, metadata !DIExpression()), !dbg !569
  call void @llvm.dbg.declare(metadata ptr %10, metadata !570, metadata !DIExpression()), !dbg !571
  br label %14, !dbg !572

14:                                               ; preds = %2
  br label %15, !dbg !573

15:                                               ; preds = %14
  call void @llvm.dbg.declare(metadata ptr %11, metadata !575, metadata !DIExpression()), !dbg !578
  call void @llvm.dbg.declare(metadata ptr %12, metadata !579, metadata !DIExpression()), !dbg !578
  %16 = icmp eq ptr %11, %12, !dbg !578
  %17 = zext i1 %16 to i32, !dbg !578
  store i32 1, ptr %13, align 4, !dbg !578
  %18 = load i32, ptr %13, align 4, !dbg !578
  %19 = load ptr, ptr %7, align 8, !dbg !580
  %20 = getelementptr inbounds %struct.atomic_notifier_head, ptr %19, i32 0, i32 0, !dbg !580
  store ptr %20, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !444, metadata !DIExpression()), !dbg !581
  %21 = load ptr, ptr %6, align 8, !dbg !583
  %22 = call i64 @_raw_spin_lock_irqsave(ptr noundef %21), !dbg !580
  store i64 %22, ptr %9, align 8, !dbg !580
  br label %23, !dbg !580

23:                                               ; preds = %15
  br label %24, !dbg !573

24:                                               ; preds = %23
  %25 = load ptr, ptr %7, align 8, !dbg !584
  %26 = getelementptr inbounds %struct.atomic_notifier_head, ptr %25, i32 0, i32 1, !dbg !585
  %27 = load ptr, ptr %8, align 8, !dbg !586
  %28 = call i32 @notifier_chain_unregister(ptr noundef %26, ptr noundef %27), !dbg !587
  store i32 %28, ptr %10, align 4, !dbg !588
  %29 = load ptr, ptr %7, align 8, !dbg !589
  %30 = getelementptr inbounds %struct.atomic_notifier_head, ptr %29, i32 0, i32 0, !dbg !590
  %31 = load i64, ptr %9, align 8, !dbg !591
  store ptr %30, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !462, metadata !DIExpression()), !dbg !592
  store i64 %31, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !468, metadata !DIExpression()), !dbg !594
  call void @llvm.dbg.declare(metadata ptr undef, metadata !470, metadata !DIExpression()), !dbg !595
  call void @llvm.dbg.declare(metadata ptr undef, metadata !474, metadata !DIExpression()), !dbg !595
  store i32 1, ptr %5, align 4, !dbg !595
  %32 = load i32, ptr %5, align 4, !dbg !595
  %33 = load ptr, ptr %3, align 8, !dbg !596
  %34 = load i64, ptr %4, align 8, !dbg !596
  call void @_raw_spin_unlock_irqrestore(ptr noundef %33, i64 noundef %34) #3, !dbg !596
  call void @synchronize_rcu(), !dbg !597
  %35 = load i32, ptr %10, align 4, !dbg !598
  ret i32 %35, !dbg !599
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @notifier_chain_unregister(ptr noundef %0, ptr noundef %1) #0 !dbg !600 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !601, metadata !DIExpression()), !dbg !602
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !603, metadata !DIExpression()), !dbg !604
  br label %7, !dbg !605

7:                                                ; preds = %51, %2
  %8 = load ptr, ptr %4, align 8, !dbg !606
  %9 = load ptr, ptr %8, align 8, !dbg !607
  %10 = icmp ne ptr %9, null, !dbg !608
  br i1 %10, label %11, label %55, !dbg !605

11:                                               ; preds = %7
  %12 = load ptr, ptr %4, align 8, !dbg !609
  %13 = load ptr, ptr %12, align 8, !dbg !612
  %14 = load ptr, ptr %5, align 8, !dbg !613
  %15 = icmp eq ptr %13, %14, !dbg !614
  br i1 %15, label %16, label %51, !dbg !615

16:                                               ; preds = %11
  br label %17, !dbg !616

17:                                               ; preds = %16
  call void @llvm.dbg.declare(metadata ptr %6, metadata !618, metadata !DIExpression()), !dbg !620
  %18 = load ptr, ptr %5, align 8, !dbg !620
  %19 = getelementptr inbounds %struct.notifier_block, ptr %18, i32 0, i32 1, !dbg !620
  %20 = load ptr, ptr %19, align 8, !dbg !620
  %21 = ptrtoint ptr %20 to i64, !dbg !620
  store i64 %21, ptr %6, align 8, !dbg !620
  br i1 false, label %22, label %35, !dbg !621

22:                                               ; preds = %17
  %23 = load i64, ptr %6, align 8, !dbg !621
  %24 = icmp eq i64 %23, 0, !dbg !621
  br i1 %24, label %25, label %35, !dbg !620

25:                                               ; preds = %22
  br label %26, !dbg !621

26:                                               ; preds = %25
  br label %27, !dbg !623

27:                                               ; preds = %26
  br label %28, !dbg !625

28:                                               ; preds = %27
  br label %29, !dbg !623

29:                                               ; preds = %28
  %30 = load i64, ptr %6, align 8, !dbg !627
  %31 = inttoptr i64 %30 to ptr, !dbg !627
  %32 = load ptr, ptr %4, align 8, !dbg !627
  store volatile ptr %31, ptr %32, align 8, !dbg !627
  br label %33, !dbg !627

33:                                               ; preds = %29
  br label %34, !dbg !623

34:                                               ; preds = %33
  br label %49, !dbg !623

35:                                               ; preds = %22, %17
  br label %36, !dbg !621

36:                                               ; preds = %35
  br label %37, !dbg !629

37:                                               ; preds = %36
  br label %38, !dbg !631

38:                                               ; preds = %37
  call void asm sideeffect "", "~{memory},~{dirflag},~{fpsr},~{flags}"() #3, !dbg !629, !srcloc !633
  br label %39, !dbg !629

39:                                               ; preds = %38
  br label %40, !dbg !634

40:                                               ; preds = %39
  br label %41, !dbg !636

41:                                               ; preds = %40
  br label %42, !dbg !634

42:                                               ; preds = %41
  %43 = load i64, ptr %6, align 8, !dbg !638
  %44 = inttoptr i64 %43 to ptr, !dbg !638
  %45 = load ptr, ptr %4, align 8, !dbg !638
  store volatile ptr %44, ptr %45, align 8, !dbg !638
  br label %46, !dbg !638

46:                                               ; preds = %42
  br label %47, !dbg !634

47:                                               ; preds = %46
  br label %48, !dbg !629

48:                                               ; preds = %47
  br label %49

49:                                               ; preds = %48, %34
  br label %50, !dbg !620

50:                                               ; preds = %49
  store i32 0, ptr %3, align 4, !dbg !640
  br label %56, !dbg !640

51:                                               ; preds = %11
  %52 = load ptr, ptr %4, align 8, !dbg !641
  %53 = load ptr, ptr %52, align 8, !dbg !642
  %54 = getelementptr inbounds %struct.notifier_block, ptr %53, i32 0, i32 1, !dbg !643
  store ptr %54, ptr %4, align 8, !dbg !644
  br label %7, !dbg !605, !llvm.loop !645

55:                                               ; preds = %7
  store i32 -2, ptr %3, align 4, !dbg !647
  br label %56, !dbg !647

56:                                               ; preds = %55, %50
  %57 = load i32, ptr %3, align 4, !dbg !648
  ret i32 %57, !dbg !648
}

declare dso_local void @synchronize_rcu() #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @atomic_notifier_call_chain(ptr noundef %0, i64 noundef %1, ptr noundef %2) #0 !dbg !649 {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !652, metadata !DIExpression()), !dbg !653
  store i64 %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !654, metadata !DIExpression()), !dbg !655
  store ptr %2, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !656, metadata !DIExpression()), !dbg !657
  call void @llvm.dbg.declare(metadata ptr %7, metadata !658, metadata !DIExpression()), !dbg !659
  call void @__rcu_read_lock() #3, !dbg !660
  %8 = load ptr, ptr %4, align 8, !dbg !666
  %9 = getelementptr inbounds %struct.atomic_notifier_head, ptr %8, i32 0, i32 1, !dbg !667
  %10 = load i64, ptr %5, align 8, !dbg !668
  %11 = load ptr, ptr %6, align 8, !dbg !669
  %12 = call i32 @notifier_call_chain(ptr noundef %9, i64 noundef %10, ptr noundef %11, i32 noundef -1, ptr noundef null), !dbg !670
  store i32 %12, ptr %7, align 4, !dbg !671
  call void @rcu_read_unlock(), !dbg !672
  %13 = load i32, ptr %7, align 4, !dbg !673
  ret i32 %13, !dbg !674
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @rcu_read_unlock() #0 !dbg !675 {
  br label %1, !dbg !676

1:                                                ; preds = %0
  br label %2, !dbg !677

2:                                                ; preds = %1
  call void @__rcu_read_unlock(), !dbg !679
  br label %3, !dbg !680

3:                                                ; preds = %2
  br label %4, !dbg !681

4:                                                ; preds = %3
  ret void, !dbg !683
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @blocking_notifier_chain_register(ptr noundef %0, ptr noundef %1) #0 !dbg !684 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !688, metadata !DIExpression()), !dbg !689
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !690, metadata !DIExpression()), !dbg !691
  call void @llvm.dbg.declare(metadata ptr %6, metadata !692, metadata !DIExpression()), !dbg !693
  %7 = load i32, ptr @system_state, align 4, !dbg !694
  %8 = icmp eq i32 %7, 0, !dbg !694
  %9 = xor i1 %8, true, !dbg !694
  %10 = xor i1 %9, true, !dbg !694
  %11 = zext i1 %10 to i32, !dbg !694
  %12 = sext i32 %11 to i64, !dbg !694
  %13 = icmp ne i64 %12, 0, !dbg !694
  br i1 %13, label %14, label %19, !dbg !696

14:                                               ; preds = %2
  %15 = load ptr, ptr %4, align 8, !dbg !697
  %16 = getelementptr inbounds %struct.blocking_notifier_head, ptr %15, i32 0, i32 1, !dbg !698
  %17 = load ptr, ptr %5, align 8, !dbg !699
  %18 = call i32 @notifier_chain_register(ptr noundef %16, ptr noundef %17), !dbg !700
  store i32 %18, ptr %3, align 4, !dbg !701
  br label %29, !dbg !701

19:                                               ; preds = %2
  %20 = load ptr, ptr %4, align 8, !dbg !702
  %21 = getelementptr inbounds %struct.blocking_notifier_head, ptr %20, i32 0, i32 0, !dbg !703
  call void @down_write(ptr noundef %21), !dbg !704
  %22 = load ptr, ptr %4, align 8, !dbg !705
  %23 = getelementptr inbounds %struct.blocking_notifier_head, ptr %22, i32 0, i32 1, !dbg !706
  %24 = load ptr, ptr %5, align 8, !dbg !707
  %25 = call i32 @notifier_chain_register(ptr noundef %23, ptr noundef %24), !dbg !708
  store i32 %25, ptr %6, align 4, !dbg !709
  %26 = load ptr, ptr %4, align 8, !dbg !710
  %27 = getelementptr inbounds %struct.blocking_notifier_head, ptr %26, i32 0, i32 0, !dbg !711
  call void @up_write(ptr noundef %27), !dbg !712
  %28 = load i32, ptr %6, align 4, !dbg !713
  store i32 %28, ptr %3, align 4, !dbg !714
  br label %29, !dbg !714

29:                                               ; preds = %19, %14
  %30 = load i32, ptr %3, align 4, !dbg !715
  ret i32 %30, !dbg !715
}

declare dso_local void @down_write(ptr noundef) #2

declare dso_local void @up_write(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @blocking_notifier_chain_unregister(ptr noundef %0, ptr noundef %1) #0 !dbg !716 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !717, metadata !DIExpression()), !dbg !718
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !719, metadata !DIExpression()), !dbg !720
  call void @llvm.dbg.declare(metadata ptr %6, metadata !721, metadata !DIExpression()), !dbg !722
  %7 = load i32, ptr @system_state, align 4, !dbg !723
  %8 = icmp eq i32 %7, 0, !dbg !723
  %9 = xor i1 %8, true, !dbg !723
  %10 = xor i1 %9, true, !dbg !723
  %11 = zext i1 %10 to i32, !dbg !723
  %12 = sext i32 %11 to i64, !dbg !723
  %13 = icmp ne i64 %12, 0, !dbg !723
  br i1 %13, label %14, label %19, !dbg !725

14:                                               ; preds = %2
  %15 = load ptr, ptr %4, align 8, !dbg !726
  %16 = getelementptr inbounds %struct.blocking_notifier_head, ptr %15, i32 0, i32 1, !dbg !727
  %17 = load ptr, ptr %5, align 8, !dbg !728
  %18 = call i32 @notifier_chain_unregister(ptr noundef %16, ptr noundef %17), !dbg !729
  store i32 %18, ptr %3, align 4, !dbg !730
  br label %29, !dbg !730

19:                                               ; preds = %2
  %20 = load ptr, ptr %4, align 8, !dbg !731
  %21 = getelementptr inbounds %struct.blocking_notifier_head, ptr %20, i32 0, i32 0, !dbg !732
  call void @down_write(ptr noundef %21), !dbg !733
  %22 = load ptr, ptr %4, align 8, !dbg !734
  %23 = getelementptr inbounds %struct.blocking_notifier_head, ptr %22, i32 0, i32 1, !dbg !735
  %24 = load ptr, ptr %5, align 8, !dbg !736
  %25 = call i32 @notifier_chain_unregister(ptr noundef %23, ptr noundef %24), !dbg !737
  store i32 %25, ptr %6, align 4, !dbg !738
  %26 = load ptr, ptr %4, align 8, !dbg !739
  %27 = getelementptr inbounds %struct.blocking_notifier_head, ptr %26, i32 0, i32 0, !dbg !740
  call void @up_write(ptr noundef %27), !dbg !741
  %28 = load i32, ptr %6, align 4, !dbg !742
  store i32 %28, ptr %3, align 4, !dbg !743
  br label %29, !dbg !743

29:                                               ; preds = %19, %14
  %30 = load i32, ptr %3, align 4, !dbg !744
  ret i32 %30, !dbg !744
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @blocking_notifier_call_chain_robust(ptr noundef %0, i64 noundef %1, i64 noundef %2, ptr noundef %3) #0 !dbg !745 {
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i32, align 4
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  %12 = alloca ptr, align 8
  store ptr %0, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !748, metadata !DIExpression()), !dbg !749
  store i64 %1, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !750, metadata !DIExpression()), !dbg !751
  store i64 %2, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !752, metadata !DIExpression()), !dbg !753
  store ptr %3, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !754, metadata !DIExpression()), !dbg !755
  call void @llvm.dbg.declare(metadata ptr %9, metadata !756, metadata !DIExpression()), !dbg !757
  store i32 0, ptr %9, align 4, !dbg !757
  call void @llvm.dbg.declare(metadata ptr %10, metadata !758, metadata !DIExpression()), !dbg !761
  br label %13, !dbg !762

13:                                               ; preds = %4
  br label %14, !dbg !764

14:                                               ; preds = %13
  %15 = load ptr, ptr %5, align 8, !dbg !762
  %16 = getelementptr inbounds %struct.blocking_notifier_head, ptr %15, i32 0, i32 1, !dbg !762
  %17 = load volatile ptr, ptr %16, align 8, !dbg !762
  store ptr %17, ptr %11, align 8, !dbg !764
  %18 = load ptr, ptr %11, align 8, !dbg !762
  store ptr %18, ptr %10, align 8, !dbg !761
  %19 = load ptr, ptr %10, align 8, !dbg !761
  store ptr %19, ptr %12, align 8, !dbg !761
  %20 = load ptr, ptr %12, align 8, !dbg !761
  %21 = icmp ne ptr %20, null, !dbg !766
  br i1 %21, label %22, label %33, !dbg !767

22:                                               ; preds = %14
  %23 = load ptr, ptr %5, align 8, !dbg !768
  %24 = getelementptr inbounds %struct.blocking_notifier_head, ptr %23, i32 0, i32 0, !dbg !770
  call void @down_read(ptr noundef %24), !dbg !771
  %25 = load ptr, ptr %5, align 8, !dbg !772
  %26 = getelementptr inbounds %struct.blocking_notifier_head, ptr %25, i32 0, i32 1, !dbg !773
  %27 = load i64, ptr %6, align 8, !dbg !774
  %28 = load i64, ptr %7, align 8, !dbg !775
  %29 = load ptr, ptr %8, align 8, !dbg !776
  %30 = call i32 @notifier_call_chain_robust(ptr noundef %26, i64 noundef %27, i64 noundef %28, ptr noundef %29), !dbg !777
  store i32 %30, ptr %9, align 4, !dbg !778
  %31 = load ptr, ptr %5, align 8, !dbg !779
  %32 = getelementptr inbounds %struct.blocking_notifier_head, ptr %31, i32 0, i32 0, !dbg !780
  call void @up_read(ptr noundef %32), !dbg !781
  br label %33, !dbg !782

33:                                               ; preds = %22, %14
  %34 = load i32, ptr %9, align 4, !dbg !783
  ret i32 %34, !dbg !784
}

declare dso_local void @down_read(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @notifier_call_chain_robust(ptr noundef %0, i64 noundef %1, i64 noundef %2, ptr noundef %3) #0 !dbg !785 {
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  store ptr %0, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !788, metadata !DIExpression()), !dbg !789
  store i64 %1, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !790, metadata !DIExpression()), !dbg !791
  store i64 %2, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !792, metadata !DIExpression()), !dbg !793
  store ptr %3, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !794, metadata !DIExpression()), !dbg !795
  call void @llvm.dbg.declare(metadata ptr %9, metadata !796, metadata !DIExpression()), !dbg !797
  call void @llvm.dbg.declare(metadata ptr %10, metadata !798, metadata !DIExpression()), !dbg !799
  store i32 0, ptr %10, align 4, !dbg !799
  %11 = load ptr, ptr %5, align 8, !dbg !800
  %12 = load i64, ptr %6, align 8, !dbg !801
  %13 = load ptr, ptr %8, align 8, !dbg !802
  %14 = call i32 @notifier_call_chain(ptr noundef %11, i64 noundef %12, ptr noundef %13, i32 noundef -1, ptr noundef %10), !dbg !803
  store i32 %14, ptr %9, align 4, !dbg !804
  %15 = load i32, ptr %9, align 4, !dbg !805
  %16 = and i32 %15, 32768, !dbg !807
  %17 = icmp ne i32 %16, 0, !dbg !807
  br i1 %17, label %18, label %25, !dbg !808

18:                                               ; preds = %4
  %19 = load ptr, ptr %5, align 8, !dbg !809
  %20 = load i64, ptr %7, align 8, !dbg !810
  %21 = load ptr, ptr %8, align 8, !dbg !811
  %22 = load i32, ptr %10, align 4, !dbg !812
  %23 = sub nsw i32 %22, 1, !dbg !813
  %24 = call i32 @notifier_call_chain(ptr noundef %19, i64 noundef %20, ptr noundef %21, i32 noundef %23, ptr noundef null), !dbg !814
  br label %25, !dbg !814

25:                                               ; preds = %18, %4
  %26 = load i32, ptr %9, align 4, !dbg !815
  ret i32 %26, !dbg !816
}

declare dso_local void @up_read(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @blocking_notifier_call_chain(ptr noundef %0, i64 noundef %1, ptr noundef %2) #0 !dbg !817 {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i32, align 4
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !820, metadata !DIExpression()), !dbg !821
  store i64 %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !822, metadata !DIExpression()), !dbg !823
  store ptr %2, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !824, metadata !DIExpression()), !dbg !825
  call void @llvm.dbg.declare(metadata ptr %7, metadata !826, metadata !DIExpression()), !dbg !827
  store i32 0, ptr %7, align 4, !dbg !827
  call void @llvm.dbg.declare(metadata ptr %8, metadata !828, metadata !DIExpression()), !dbg !831
  br label %11, !dbg !832

11:                                               ; preds = %3
  br label %12, !dbg !834

12:                                               ; preds = %11
  %13 = load ptr, ptr %4, align 8, !dbg !832
  %14 = getelementptr inbounds %struct.blocking_notifier_head, ptr %13, i32 0, i32 1, !dbg !832
  %15 = load volatile ptr, ptr %14, align 8, !dbg !832
  store ptr %15, ptr %9, align 8, !dbg !834
  %16 = load ptr, ptr %9, align 8, !dbg !832
  store ptr %16, ptr %8, align 8, !dbg !831
  %17 = load ptr, ptr %8, align 8, !dbg !831
  store ptr %17, ptr %10, align 8, !dbg !831
  %18 = load ptr, ptr %10, align 8, !dbg !831
  %19 = icmp ne ptr %18, null, !dbg !836
  br i1 %19, label %20, label %30, !dbg !837

20:                                               ; preds = %12
  %21 = load ptr, ptr %4, align 8, !dbg !838
  %22 = getelementptr inbounds %struct.blocking_notifier_head, ptr %21, i32 0, i32 0, !dbg !840
  call void @down_read(ptr noundef %22), !dbg !841
  %23 = load ptr, ptr %4, align 8, !dbg !842
  %24 = getelementptr inbounds %struct.blocking_notifier_head, ptr %23, i32 0, i32 1, !dbg !843
  %25 = load i64, ptr %5, align 8, !dbg !844
  %26 = load ptr, ptr %6, align 8, !dbg !845
  %27 = call i32 @notifier_call_chain(ptr noundef %24, i64 noundef %25, ptr noundef %26, i32 noundef -1, ptr noundef null), !dbg !846
  store i32 %27, ptr %7, align 4, !dbg !847
  %28 = load ptr, ptr %4, align 8, !dbg !848
  %29 = getelementptr inbounds %struct.blocking_notifier_head, ptr %28, i32 0, i32 0, !dbg !849
  call void @up_read(ptr noundef %29), !dbg !850
  br label %30, !dbg !851

30:                                               ; preds = %20, %12
  %31 = load i32, ptr %7, align 4, !dbg !852
  ret i32 %31, !dbg !853
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @raw_notifier_chain_register(ptr noundef %0, ptr noundef %1) #0 !dbg !854 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !861, metadata !DIExpression()), !dbg !862
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !863, metadata !DIExpression()), !dbg !864
  %5 = load ptr, ptr %3, align 8, !dbg !865
  %6 = getelementptr inbounds %struct.raw_notifier_head, ptr %5, i32 0, i32 0, !dbg !866
  %7 = load ptr, ptr %4, align 8, !dbg !867
  %8 = call i32 @notifier_chain_register(ptr noundef %6, ptr noundef %7), !dbg !868
  ret i32 %8, !dbg !869
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @raw_notifier_chain_unregister(ptr noundef %0, ptr noundef %1) #0 !dbg !870 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !871, metadata !DIExpression()), !dbg !872
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !873, metadata !DIExpression()), !dbg !874
  %5 = load ptr, ptr %3, align 8, !dbg !875
  %6 = getelementptr inbounds %struct.raw_notifier_head, ptr %5, i32 0, i32 0, !dbg !876
  %7 = load ptr, ptr %4, align 8, !dbg !877
  %8 = call i32 @notifier_chain_unregister(ptr noundef %6, ptr noundef %7), !dbg !878
  ret i32 %8, !dbg !879
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @raw_notifier_call_chain_robust(ptr noundef %0, i64 noundef %1, i64 noundef %2, ptr noundef %3) #0 !dbg !880 {
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca ptr, align 8
  store ptr %0, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !883, metadata !DIExpression()), !dbg !884
  store i64 %1, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !885, metadata !DIExpression()), !dbg !886
  store i64 %2, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !887, metadata !DIExpression()), !dbg !888
  store ptr %3, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !889, metadata !DIExpression()), !dbg !890
  %9 = load ptr, ptr %5, align 8, !dbg !891
  %10 = getelementptr inbounds %struct.raw_notifier_head, ptr %9, i32 0, i32 0, !dbg !892
  %11 = load i64, ptr %6, align 8, !dbg !893
  %12 = load i64, ptr %7, align 8, !dbg !894
  %13 = load ptr, ptr %8, align 8, !dbg !895
  %14 = call i32 @notifier_call_chain_robust(ptr noundef %10, i64 noundef %11, i64 noundef %12, ptr noundef %13), !dbg !896
  ret i32 %14, !dbg !897
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @raw_notifier_call_chain(ptr noundef %0, i64 noundef %1, ptr noundef %2) #0 !dbg !898 {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !901, metadata !DIExpression()), !dbg !902
  store i64 %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !903, metadata !DIExpression()), !dbg !904
  store ptr %2, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !905, metadata !DIExpression()), !dbg !906
  %7 = load ptr, ptr %4, align 8, !dbg !907
  %8 = getelementptr inbounds %struct.raw_notifier_head, ptr %7, i32 0, i32 0, !dbg !908
  %9 = load i64, ptr %5, align 8, !dbg !909
  %10 = load ptr, ptr %6, align 8, !dbg !910
  %11 = call i32 @notifier_call_chain(ptr noundef %8, i64 noundef %9, ptr noundef %10, i32 noundef -1, ptr noundef null), !dbg !911
  ret i32 %11, !dbg !912
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @srcu_notifier_chain_register(ptr noundef %0, ptr noundef %1) #0 !dbg !913 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !916, metadata !DIExpression()), !dbg !917
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !918, metadata !DIExpression()), !dbg !919
  call void @llvm.dbg.declare(metadata ptr %6, metadata !920, metadata !DIExpression()), !dbg !921
  %7 = load i32, ptr @system_state, align 4, !dbg !922
  %8 = icmp eq i32 %7, 0, !dbg !922
  %9 = xor i1 %8, true, !dbg !922
  %10 = xor i1 %9, true, !dbg !922
  %11 = zext i1 %10 to i32, !dbg !922
  %12 = sext i32 %11 to i64, !dbg !922
  %13 = icmp ne i64 %12, 0, !dbg !922
  br i1 %13, label %14, label %19, !dbg !924

14:                                               ; preds = %2
  %15 = load ptr, ptr %4, align 8, !dbg !925
  %16 = getelementptr inbounds %struct.srcu_notifier_head, ptr %15, i32 0, i32 2, !dbg !926
  %17 = load ptr, ptr %5, align 8, !dbg !927
  %18 = call i32 @notifier_chain_register(ptr noundef %16, ptr noundef %17), !dbg !928
  store i32 %18, ptr %3, align 4, !dbg !929
  br label %29, !dbg !929

19:                                               ; preds = %2
  %20 = load ptr, ptr %4, align 8, !dbg !930
  %21 = getelementptr inbounds %struct.srcu_notifier_head, ptr %20, i32 0, i32 0, !dbg !931
  call void @mutex_lock(ptr noundef %21), !dbg !932
  %22 = load ptr, ptr %4, align 8, !dbg !933
  %23 = getelementptr inbounds %struct.srcu_notifier_head, ptr %22, i32 0, i32 2, !dbg !934
  %24 = load ptr, ptr %5, align 8, !dbg !935
  %25 = call i32 @notifier_chain_register(ptr noundef %23, ptr noundef %24), !dbg !936
  store i32 %25, ptr %6, align 4, !dbg !937
  %26 = load ptr, ptr %4, align 8, !dbg !938
  %27 = getelementptr inbounds %struct.srcu_notifier_head, ptr %26, i32 0, i32 0, !dbg !939
  call void @mutex_unlock(ptr noundef %27), !dbg !940
  %28 = load i32, ptr %6, align 4, !dbg !941
  store i32 %28, ptr %3, align 4, !dbg !942
  br label %29, !dbg !942

29:                                               ; preds = %19, %14
  %30 = load i32, ptr %3, align 4, !dbg !943
  ret i32 %30, !dbg !943
}

declare dso_local void @mutex_lock(ptr noundef) #2

declare dso_local void @mutex_unlock(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @srcu_notifier_chain_unregister(ptr noundef %0, ptr noundef %1) #0 !dbg !944 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !945, metadata !DIExpression()), !dbg !946
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !947, metadata !DIExpression()), !dbg !948
  call void @llvm.dbg.declare(metadata ptr %6, metadata !949, metadata !DIExpression()), !dbg !950
  %7 = load i32, ptr @system_state, align 4, !dbg !951
  %8 = icmp eq i32 %7, 0, !dbg !951
  %9 = xor i1 %8, true, !dbg !951
  %10 = xor i1 %9, true, !dbg !951
  %11 = zext i1 %10 to i32, !dbg !951
  %12 = sext i32 %11 to i64, !dbg !951
  %13 = icmp ne i64 %12, 0, !dbg !951
  br i1 %13, label %14, label %19, !dbg !953

14:                                               ; preds = %2
  %15 = load ptr, ptr %4, align 8, !dbg !954
  %16 = getelementptr inbounds %struct.srcu_notifier_head, ptr %15, i32 0, i32 2, !dbg !955
  %17 = load ptr, ptr %5, align 8, !dbg !956
  %18 = call i32 @notifier_chain_unregister(ptr noundef %16, ptr noundef %17), !dbg !957
  store i32 %18, ptr %3, align 4, !dbg !958
  br label %31, !dbg !958

19:                                               ; preds = %2
  %20 = load ptr, ptr %4, align 8, !dbg !959
  %21 = getelementptr inbounds %struct.srcu_notifier_head, ptr %20, i32 0, i32 0, !dbg !960
  call void @mutex_lock(ptr noundef %21), !dbg !961
  %22 = load ptr, ptr %4, align 8, !dbg !962
  %23 = getelementptr inbounds %struct.srcu_notifier_head, ptr %22, i32 0, i32 2, !dbg !963
  %24 = load ptr, ptr %5, align 8, !dbg !964
  %25 = call i32 @notifier_chain_unregister(ptr noundef %23, ptr noundef %24), !dbg !965
  store i32 %25, ptr %6, align 4, !dbg !966
  %26 = load ptr, ptr %4, align 8, !dbg !967
  %27 = getelementptr inbounds %struct.srcu_notifier_head, ptr %26, i32 0, i32 0, !dbg !968
  call void @mutex_unlock(ptr noundef %27), !dbg !969
  %28 = load ptr, ptr %4, align 8, !dbg !970
  %29 = getelementptr inbounds %struct.srcu_notifier_head, ptr %28, i32 0, i32 1, !dbg !971
  call void @synchronize_srcu(ptr noundef %29), !dbg !972
  %30 = load i32, ptr %6, align 4, !dbg !973
  store i32 %30, ptr %3, align 4, !dbg !974
  br label %31, !dbg !974

31:                                               ; preds = %19, %14
  %32 = load i32, ptr %3, align 4, !dbg !975
  ret i32 %32, !dbg !975
}

declare dso_local void @synchronize_srcu(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @srcu_notifier_call_chain(ptr noundef %0, i64 noundef %1, ptr noundef %2) #0 !dbg !976 {
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !979, metadata !DIExpression()), !dbg !980
  store i64 %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !981, metadata !DIExpression()), !dbg !982
  store ptr %2, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !983, metadata !DIExpression()), !dbg !984
  call void @llvm.dbg.declare(metadata ptr %7, metadata !985, metadata !DIExpression()), !dbg !986
  call void @llvm.dbg.declare(metadata ptr %8, metadata !987, metadata !DIExpression()), !dbg !988
  %9 = load ptr, ptr %4, align 8, !dbg !989
  %10 = getelementptr inbounds %struct.srcu_notifier_head, ptr %9, i32 0, i32 1, !dbg !990
  %11 = call i32 @srcu_read_lock(ptr noundef %10), !dbg !991
  store i32 %11, ptr %8, align 4, !dbg !992
  %12 = load ptr, ptr %4, align 8, !dbg !993
  %13 = getelementptr inbounds %struct.srcu_notifier_head, ptr %12, i32 0, i32 2, !dbg !994
  %14 = load i64, ptr %5, align 8, !dbg !995
  %15 = load ptr, ptr %6, align 8, !dbg !996
  %16 = call i32 @notifier_call_chain(ptr noundef %13, i64 noundef %14, ptr noundef %15, i32 noundef -1, ptr noundef null), !dbg !997
  store i32 %16, ptr %7, align 4, !dbg !998
  %17 = load ptr, ptr %4, align 8, !dbg !999
  %18 = getelementptr inbounds %struct.srcu_notifier_head, ptr %17, i32 0, i32 1, !dbg !1000
  %19 = load i32, ptr %8, align 4, !dbg !1001
  call void @srcu_read_unlock(ptr noundef %18, i32 noundef %19), !dbg !1002
  %20 = load i32, ptr %7, align 4, !dbg !1003
  ret i32 %20, !dbg !1004
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @srcu_read_lock(ptr noundef %0) #0 !dbg !1005 {
  %2 = alloca ptr, align 8
  %3 = alloca i32, align 4
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !1008, metadata !DIExpression()), !dbg !1009
  call void @llvm.dbg.declare(metadata ptr %3, metadata !1010, metadata !DIExpression()), !dbg !1011
  %4 = load ptr, ptr %2, align 8, !dbg !1012
  %5 = call i32 @__srcu_read_lock(ptr noundef %4), !dbg !1013
  store i32 %5, ptr %3, align 4, !dbg !1014
  br label %6, !dbg !1015

6:                                                ; preds = %1
  br label %7, !dbg !1016

7:                                                ; preds = %6
  %8 = load i32, ptr %3, align 4, !dbg !1018
  ret i32 %8, !dbg !1019
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @srcu_read_unlock(ptr noundef %0, i32 noundef %1) #0 !dbg !1020 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !1023, metadata !DIExpression()), !dbg !1024
  store i32 %1, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1025, metadata !DIExpression()), !dbg !1026
  call void @llvm.dbg.declare(metadata ptr %5, metadata !1027, metadata !DIExpression()), !dbg !1029
  %7 = load i32, ptr %4, align 4, !dbg !1029
  %8 = and i32 %7, -2, !dbg !1029
  %9 = icmp ne i32 %8, 0, !dbg !1029
  %10 = xor i1 %9, true, !dbg !1029
  %11 = xor i1 %10, true, !dbg !1029
  %12 = zext i1 %11 to i32, !dbg !1029
  store i32 %12, ptr %5, align 4, !dbg !1029
  %13 = load i32, ptr %5, align 4, !dbg !1030
  %14 = icmp ne i32 %13, 0, !dbg !1030
  %15 = xor i1 %14, true, !dbg !1030
  %16 = xor i1 %15, true, !dbg !1030
  %17 = zext i1 %16 to i32, !dbg !1030
  %18 = sext i32 %17 to i64, !dbg !1030
  %19 = icmp ne i64 %18, 0, !dbg !1030
  br i1 %19, label %20, label %29, !dbg !1029

20:                                               ; preds = %2
  br label %21, !dbg !1030

21:                                               ; preds = %20
  br label %22, !dbg !1032

22:                                               ; preds = %21
  br label %23, !dbg !1034

23:                                               ; preds = %22
  br label %24, !dbg !1032

24:                                               ; preds = %23
  call void asm sideeffect "1:\09.byte 0x0f, 0x0b\0A.pushsection __bug_table,\22aw\22\0A2:\09.long 1b - 2b\09# bug_entry::bug_addr\0A\09.long ${0:c} - 2b\09# bug_entry::file\0A\09.word ${1:c}\09# bug_entry::line\0A\09.word ${2:c}\09# bug_entry::flags\0A\09.org 2b+${3:c}\0A.popsection", "i,i,i,i,~{dirflag},~{fpsr},~{flags}"(ptr @.str.3, i32 188, i32 2307, i64 12) #3, !dbg !1036, !srcloc !1038
  br label %25, !dbg !1036

25:                                               ; preds = %24
  call void asm sideeffect "50:\0A\09.pushsection .discard.reachable\0A\09.long 50b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 50) #3, !dbg !1039, !srcloc !1041
  br label %26, !dbg !1032

26:                                               ; preds = %25
  br label %27, !dbg !1042

27:                                               ; preds = %26
  br label %28, !dbg !1032

28:                                               ; preds = %27
  br label %29, !dbg !1032

29:                                               ; preds = %28, %2
  %30 = load i32, ptr %5, align 4, !dbg !1029
  %31 = icmp ne i32 %30, 0, !dbg !1029
  %32 = xor i1 %31, true, !dbg !1029
  %33 = xor i1 %32, true, !dbg !1029
  %34 = zext i1 %33 to i32, !dbg !1029
  %35 = sext i32 %34 to i64, !dbg !1029
  store i64 %35, ptr %6, align 8, !dbg !1030
  %36 = load i64, ptr %6, align 8, !dbg !1029
  br label %37, !dbg !1044

37:                                               ; preds = %29
  br label %38, !dbg !1045

38:                                               ; preds = %37
  %39 = load ptr, ptr %3, align 8, !dbg !1047
  %40 = load i32, ptr %4, align 4, !dbg !1048
  call void @__srcu_read_unlock(ptr noundef %39, i32 noundef %40), !dbg !1049
  ret void, !dbg !1050
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @srcu_init_notifier_head(ptr noundef %0) #0 !dbg !89 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !1051, metadata !DIExpression()), !dbg !1052
  br label %3, !dbg !1053

3:                                                ; preds = %1
  %4 = load ptr, ptr %2, align 8, !dbg !1054
  %5 = getelementptr inbounds %struct.srcu_notifier_head, ptr %4, i32 0, i32 0, !dbg !1054
  call void @__mutex_init(ptr noundef %5, ptr noundef @.str, ptr noundef @srcu_init_notifier_head.__key), !dbg !1054
  br label %6, !dbg !1054

6:                                                ; preds = %3
  %7 = load ptr, ptr %2, align 8, !dbg !1056
  %8 = getelementptr inbounds %struct.srcu_notifier_head, ptr %7, i32 0, i32 1, !dbg !1058
  %9 = call i32 @init_srcu_struct(ptr noundef %8), !dbg !1059
  %10 = icmp slt i32 %9, 0, !dbg !1060
  br i1 %10, label %11, label %20, !dbg !1061

11:                                               ; preds = %6
  br label %12, !dbg !1062

12:                                               ; preds = %11
  br label %13, !dbg !1063

13:                                               ; preds = %12
  br label %14, !dbg !1065

14:                                               ; preds = %13
  br label %15, !dbg !1063

15:                                               ; preds = %14
  call void asm sideeffect "1:\09.byte 0x0f, 0x0b\0A.pushsection __bug_table,\22aw\22\0A2:\09.long 1b - 2b\09# bug_entry::bug_addr\0A\09.long ${0:c} - 2b\09# bug_entry::file\0A\09.word ${1:c}\09# bug_entry::line\0A\09.word ${2:c}\09# bug_entry::flags\0A\09.org 2b+${3:c}\0A.popsection", "i,i,i,i,~{dirflag},~{fpsr},~{flags}"(ptr @.str.1, i32 508, i32 0, i64 12) #3, !dbg !1067, !srcloc !1069
  br label %16, !dbg !1067

16:                                               ; preds = %15
  br label %17, !dbg !1063

17:                                               ; preds = %16
  call void asm sideeffect "268:\0A\09.pushsection .discard.unreachable\0A\09.long 268b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 268) #3, !dbg !1070, !srcloc !1073
  unreachable, !dbg !1074

18:                                               ; No predecessors!
  br label %19, !dbg !1063

19:                                               ; preds = %18
  br label %20, !dbg !1063

20:                                               ; preds = %19, %6
  %21 = load ptr, ptr %2, align 8, !dbg !1075
  %22 = getelementptr inbounds %struct.srcu_notifier_head, ptr %21, i32 0, i32 2, !dbg !1076
  store ptr null, ptr %22, align 8, !dbg !1077
  ret void, !dbg !1078
}

declare dso_local void @__mutex_init(ptr noundef, ptr noundef, ptr noundef) #2

declare dso_local i32 @init_srcu_struct(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @notify_die(i32 noundef %0, ptr noundef %1, ptr noundef %2, i64 noundef %3, i32 noundef %4, i32 noundef %5) #0 !dbg !1079 {
  %7 = alloca i32, align 4
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca i64, align 8
  %11 = alloca i32, align 4
  %12 = alloca i32, align 4
  %13 = alloca %struct.die_args, align 8
  store i32 %0, ptr %7, align 4
  call void @llvm.dbg.declare(metadata ptr %7, metadata !1109, metadata !DIExpression()), !dbg !1110
  store ptr %1, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !1111, metadata !DIExpression()), !dbg !1112
  store ptr %2, ptr %9, align 8
  call void @llvm.dbg.declare(metadata ptr %9, metadata !1113, metadata !DIExpression()), !dbg !1114
  store i64 %3, ptr %10, align 8
  call void @llvm.dbg.declare(metadata ptr %10, metadata !1115, metadata !DIExpression()), !dbg !1116
  store i32 %4, ptr %11, align 4
  call void @llvm.dbg.declare(metadata ptr %11, metadata !1117, metadata !DIExpression()), !dbg !1118
  store i32 %5, ptr %12, align 4
  call void @llvm.dbg.declare(metadata ptr %12, metadata !1119, metadata !DIExpression()), !dbg !1120
  call void @llvm.dbg.declare(metadata ptr %13, metadata !1121, metadata !DIExpression()), !dbg !1130
  %14 = getelementptr inbounds %struct.die_args, ptr %13, i32 0, i32 0, !dbg !1131
  %15 = load ptr, ptr %9, align 8, !dbg !1132
  store ptr %15, ptr %14, align 8, !dbg !1131
  %16 = getelementptr inbounds %struct.die_args, ptr %13, i32 0, i32 1, !dbg !1131
  %17 = load ptr, ptr %8, align 8, !dbg !1133
  store ptr %17, ptr %16, align 8, !dbg !1131
  %18 = getelementptr inbounds %struct.die_args, ptr %13, i32 0, i32 2, !dbg !1131
  %19 = load i64, ptr %10, align 8, !dbg !1134
  store i64 %19, ptr %18, align 8, !dbg !1131
  %20 = getelementptr inbounds %struct.die_args, ptr %13, i32 0, i32 3, !dbg !1131
  %21 = load i32, ptr %11, align 4, !dbg !1135
  store i32 %21, ptr %20, align 8, !dbg !1131
  %22 = getelementptr inbounds %struct.die_args, ptr %13, i32 0, i32 4, !dbg !1131
  %23 = load i32, ptr %12, align 4, !dbg !1136
  store i32 %23, ptr %22, align 4, !dbg !1131
  br label %24, !dbg !1137

24:                                               ; preds = %6
  br label %25, !dbg !1138

25:                                               ; preds = %24
  %26 = load i32, ptr %7, align 4, !dbg !1140
  %27 = zext i32 %26 to i64, !dbg !1140
  %28 = call i32 @atomic_notifier_call_chain(ptr noundef @die_chain, i64 noundef %27, ptr noundef %13), !dbg !1141
  ret i32 %28, !dbg !1142
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @register_die_notifier(ptr noundef %0) #0 !dbg !1143 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !1146, metadata !DIExpression()), !dbg !1147
  %3 = load ptr, ptr %2, align 8, !dbg !1148
  %4 = call i32 @atomic_notifier_chain_register(ptr noundef @die_chain, ptr noundef %3), !dbg !1149
  ret i32 %4, !dbg !1150
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @unregister_die_notifier(ptr noundef %0) #0 !dbg !1151 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !1152, metadata !DIExpression()), !dbg !1153
  %3 = load ptr, ptr %2, align 8, !dbg !1154
  %4 = call i32 @atomic_notifier_chain_unregister(ptr noundef @die_chain, ptr noundef %3), !dbg !1155
  ret i32 %4, !dbg !1156
}

declare dso_local void @__warn_printk(ptr noundef, ...) #2

declare dso_local void @_raw_spin_unlock_irqrestore(ptr noundef, i64 noundef) #2 section ".spinlock.text"

declare dso_local void @__rcu_read_lock() #2

declare dso_local void @__rcu_read_unlock() #2

declare dso_local i32 @__srcu_read_lock(ptr noundef) #2

declare dso_local void @__srcu_read_unlock(ptr noundef, i32 noundef) #2

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!349, !350, !351, !352, !353}
!llvm.ident = !{!354}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "reboot_notifier_list", scope: !2, file: !56, line: 15, type: !337, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "Debian clang version 15.0.6", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !32, globals: !53, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/kernel/notifier.c", directory: "/mlx_devbox/users/mayunlong.39/playground", checksumkind: CSK_MD5, checksum: "abea97fac9adcaf987d85762a41b50fb")
!4 = !{!5, !17}
!5 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "system_states", file: !6, line: 242, baseType: !7, size: 32, elements: !8)
!6 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/kernel.h", directory: "", checksumkind: CSK_MD5, checksum: "f9080ca305342b2a4060ec6855678039")
!7 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!8 = !{!9, !10, !11, !12, !13, !14, !15, !16}
!9 = !DIEnumerator(name: "SYSTEM_BOOTING", value: 0)
!10 = !DIEnumerator(name: "SYSTEM_SCHEDULING", value: 1)
!11 = !DIEnumerator(name: "SYSTEM_FREEING_INITMEM", value: 2)
!12 = !DIEnumerator(name: "SYSTEM_RUNNING", value: 3)
!13 = !DIEnumerator(name: "SYSTEM_HALT", value: 4)
!14 = !DIEnumerator(name: "SYSTEM_POWER_OFF", value: 5)
!15 = !DIEnumerator(name: "SYSTEM_RESTART", value: 6)
!16 = !DIEnumerator(name: "SYSTEM_SUSPEND", value: 7)
!17 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "die_val", file: !18, line: 10, baseType: !7, size: 32, elements: !19)
!18 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/arch/x86/include/asm/kdebug.h", directory: "", checksumkind: CSK_MD5, checksum: "a9b9e116c256f8ca47b10b852f08e11a")
!19 = !{!20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!20 = !DIEnumerator(name: "DIE_OOPS", value: 1)
!21 = !DIEnumerator(name: "DIE_INT3", value: 2)
!22 = !DIEnumerator(name: "DIE_DEBUG", value: 3)
!23 = !DIEnumerator(name: "DIE_PANIC", value: 4)
!24 = !DIEnumerator(name: "DIE_NMI", value: 5)
!25 = !DIEnumerator(name: "DIE_DIE", value: 6)
!26 = !DIEnumerator(name: "DIE_KERNELDEBUG", value: 7)
!27 = !DIEnumerator(name: "DIE_TRAP", value: 8)
!28 = !DIEnumerator(name: "DIE_GPF", value: 9)
!29 = !DIEnumerator(name: "DIE_CALL", value: 10)
!30 = !DIEnumerator(name: "DIE_PAGE_FAULT", value: 11)
!31 = !DIEnumerator(name: "DIE_NMIUNKNOWN", value: 12)
!32 = !{!33, !47, !44, !50, !52}
!33 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !34, size: 64)
!34 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "notifier_block", file: !35, line: 54, size: 192, elements: !36)
!35 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/notifier.h", directory: "", checksumkind: CSK_MD5, checksum: "2aa6b143b194d97d12a2f1e36fa6002b")
!36 = !{!37, !45, !46}
!37 = !DIDerivedType(tag: DW_TAG_member, name: "notifier_call", scope: !34, file: !35, line: 55, baseType: !38, size: 64)
!38 = !DIDerivedType(tag: DW_TAG_typedef, name: "notifier_fn_t", file: !35, line: 51, baseType: !39)
!39 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !40, size: 64)
!40 = !DISubroutineType(types: !41)
!41 = !{!42, !33, !43, !44}
!42 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!43 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!44 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!45 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !34, file: !35, line: 56, baseType: !33, size: 64, offset: 64)
!46 = !DIDerivedType(tag: DW_TAG_member, name: "priority", scope: !34, file: !35, line: 57, baseType: !42, size: 32, offset: 128)
!47 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !48, size: 64)
!48 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !49)
!49 = !DIDerivedType(tag: DW_TAG_volatile_type, baseType: !33)
!50 = !DIDerivedType(tag: DW_TAG_typedef, name: "uintptr_t", file: !51, line: 37, baseType: !43)
!51 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/types.h", directory: "", checksumkind: CSK_MD5, checksum: "79630acc61686427c74499289856428e")
!52 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !49, size: 64)
!53 = !{!0, !54, !57, !59, !61, !63, !65, !67, !69, !71, !73, !75, !77, !79, !81, !83, !85, !87, !301, !307, !312, !314, !316, !318, !320, !325, !331}
!54 = !DIGlobalVariableExpression(var: !55, expr: !DIExpression())
!55 = distinct !DIGlobalVariable(name: "_kbl_addr_notifier_call_chain", scope: !2, file: !56, line: 96, type: !43, isLocal: true, isDefinition: true)
!56 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/kernel/notifier.c", directory: "", checksumkind: CSK_MD5, checksum: "abea97fac9adcaf987d85762a41b50fb")
!57 = !DIGlobalVariableExpression(var: !58, expr: !DIExpression())
!58 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_atomic_notifier_chain_register252", scope: !2, file: !56, line: 151, type: !44, isLocal: true, isDefinition: true)
!59 = !DIGlobalVariableExpression(var: !60, expr: !DIExpression())
!60 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_atomic_notifier_chain_unregister253", scope: !2, file: !56, line: 174, type: !44, isLocal: true, isDefinition: true)
!61 = !DIGlobalVariableExpression(var: !62, expr: !DIExpression())
!62 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_atomic_notifier_call_chain254", scope: !2, file: !56, line: 204, type: !44, isLocal: true, isDefinition: true)
!63 = !DIGlobalVariableExpression(var: !64, expr: !DIExpression())
!64 = distinct !DIGlobalVariable(name: "_kbl_addr_atomic_notifier_call_chain", scope: !2, file: !56, line: 205, type: !43, isLocal: true, isDefinition: true)
!65 = !DIGlobalVariableExpression(var: !66, expr: !DIExpression())
!66 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_blocking_notifier_chain_register255", scope: !2, file: !56, line: 240, type: !44, isLocal: true, isDefinition: true)
!67 = !DIGlobalVariableExpression(var: !68, expr: !DIExpression())
!68 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_blocking_notifier_chain_unregister256", scope: !2, file: !56, line: 270, type: !44, isLocal: true, isDefinition: true)
!69 = !DIGlobalVariableExpression(var: !70, expr: !DIExpression())
!70 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_blocking_notifier_call_chain_robust258", scope: !2, file: !56, line: 289, type: !44, isLocal: true, isDefinition: true)
!71 = !DIGlobalVariableExpression(var: !72, expr: !DIExpression())
!72 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_blocking_notifier_call_chain260", scope: !2, file: !56, line: 324, type: !44, isLocal: true, isDefinition: true)
!73 = !DIGlobalVariableExpression(var: !74, expr: !DIExpression())
!74 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_raw_notifier_chain_register261", scope: !2, file: !56, line: 346, type: !44, isLocal: true, isDefinition: true)
!75 = !DIGlobalVariableExpression(var: !76, expr: !DIExpression())
!76 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_raw_notifier_chain_unregister262", scope: !2, file: !56, line: 363, type: !44, isLocal: true, isDefinition: true)
!77 = !DIGlobalVariableExpression(var: !78, expr: !DIExpression())
!78 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_raw_notifier_call_chain_robust263", scope: !2, file: !56, line: 370, type: !44, isLocal: true, isDefinition: true)
!79 = !DIGlobalVariableExpression(var: !80, expr: !DIExpression())
!80 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_raw_notifier_call_chain264", scope: !2, file: !56, line: 394, type: !44, isLocal: true, isDefinition: true)
!81 = !DIGlobalVariableExpression(var: !82, expr: !DIExpression())
!82 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_srcu_notifier_chain_register265", scope: !2, file: !56, line: 430, type: !44, isLocal: true, isDefinition: true)
!83 = !DIGlobalVariableExpression(var: !84, expr: !DIExpression())
!84 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_srcu_notifier_chain_unregister266", scope: !2, file: !56, line: 461, type: !44, isLocal: true, isDefinition: true)
!85 = !DIGlobalVariableExpression(var: !86, expr: !DIExpression())
!86 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_srcu_notifier_call_chain267", scope: !2, file: !56, line: 490, type: !44, isLocal: true, isDefinition: true)
!87 = !DIGlobalVariableExpression(var: !88, expr: !DIExpression())
!88 = distinct !DIGlobalVariable(name: "__key", scope: !89, file: !56, line: 506, type: !300, isLocal: true, isDefinition: true)
!89 = distinct !DISubprogram(name: "srcu_init_notifier_head", scope: !56, file: !56, line: 504, type: !90, scopeLine: 505, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!90 = !DISubroutineType(types: !91)
!91 = !{null, !92}
!92 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !93, size: 64)
!93 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "srcu_notifier_head", file: !35, line: 74, size: 6656, elements: !94)
!94 = !{!95, !156, !299}
!95 = !DIDerivedType(tag: DW_TAG_member, name: "mutex", scope: !93, file: !35, line: 75, baseType: !96, size: 256)
!96 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mutex", file: !97, line: 63, size: 256, elements: !98)
!97 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/mutex.h", directory: "", checksumkind: CSK_MD5, checksum: "d9f272c57aded2ac4d380cd0455049b3")
!98 = !{!99, !111, !145, !150}
!99 = !DIDerivedType(tag: DW_TAG_member, name: "owner", scope: !96, file: !97, line: 64, baseType: !100, size: 64)
!100 = !DIDerivedType(tag: DW_TAG_typedef, name: "atomic_long_t", file: !101, line: 13, baseType: !102)
!101 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/atomic/atomic-long.h", directory: "", checksumkind: CSK_MD5, checksum: "49905841291815a398cecb7e09bce429")
!102 = !DIDerivedType(tag: DW_TAG_typedef, name: "atomic64_t", file: !51, line: 175, baseType: !103)
!103 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !51, line: 173, size: 64, elements: !104)
!104 = !{!105}
!105 = !DIDerivedType(tag: DW_TAG_member, name: "counter", scope: !103, file: !51, line: 174, baseType: !106, size: 64)
!106 = !DIDerivedType(tag: DW_TAG_typedef, name: "s64", file: !107, line: 22, baseType: !108)
!107 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/asm-generic/int-ll64.h", directory: "", checksumkind: CSK_MD5, checksum: "12ca7bdb629352cc4c9a492f86b435a7")
!108 = !DIDerivedType(tag: DW_TAG_typedef, name: "__s64", file: !109, line: 30, baseType: !110)
!109 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/uapi/asm-generic/int-ll64.h", directory: "", checksumkind: CSK_MD5, checksum: "f4d0ec5bcdd84e825a78a7add39d54dd")
!110 = !DIBasicType(name: "long long", size: 64, encoding: DW_ATE_signed)
!111 = !DIDerivedType(tag: DW_TAG_member, name: "wait_lock", scope: !96, file: !97, line: 65, baseType: !112, size: 32, offset: 64)
!112 = !DIDerivedType(tag: DW_TAG_typedef, name: "raw_spinlock_t", file: !113, line: 23, baseType: !114)
!113 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/spinlock_types_raw.h", directory: "", checksumkind: CSK_MD5, checksum: "9bbdf30cd339c0a21e6a004bafba78e0")
!114 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "raw_spinlock", file: !113, line: 14, size: 32, elements: !115)
!115 = !{!116}
!116 = !DIDerivedType(tag: DW_TAG_member, name: "raw_lock", scope: !114, file: !113, line: 15, baseType: !117, size: 32)
!117 = !DIDerivedType(tag: DW_TAG_typedef, name: "arch_spinlock_t", file: !118, line: 44, baseType: !119)
!118 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/asm-generic/qspinlock_types.h", directory: "", checksumkind: CSK_MD5, checksum: "2a1236eda9a125c2ce03b9a345491b46")
!119 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "qspinlock", file: !118, line: 14, size: 32, elements: !120)
!120 = !{!121}
!121 = !DIDerivedType(tag: DW_TAG_member, scope: !119, file: !118, line: 15, baseType: !122, size: 32)
!122 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !119, file: !118, line: 15, size: 32, elements: !123)
!123 = !{!124, !129, !137}
!124 = !DIDerivedType(tag: DW_TAG_member, name: "val", scope: !122, file: !118, line: 16, baseType: !125, size: 32)
!125 = !DIDerivedType(tag: DW_TAG_typedef, name: "atomic_t", file: !51, line: 168, baseType: !126)
!126 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !51, line: 166, size: 32, elements: !127)
!127 = !{!128}
!128 = !DIDerivedType(tag: DW_TAG_member, name: "counter", scope: !126, file: !51, line: 167, baseType: !42, size: 32)
!129 = !DIDerivedType(tag: DW_TAG_member, scope: !122, file: !118, line: 24, baseType: !130, size: 16)
!130 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !122, file: !118, line: 24, size: 16, elements: !131)
!131 = !{!132, !136}
!132 = !DIDerivedType(tag: DW_TAG_member, name: "locked", scope: !130, file: !118, line: 25, baseType: !133, size: 8)
!133 = !DIDerivedType(tag: DW_TAG_typedef, name: "u8", file: !107, line: 17, baseType: !134)
!134 = !DIDerivedType(tag: DW_TAG_typedef, name: "__u8", file: !109, line: 21, baseType: !135)
!135 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!136 = !DIDerivedType(tag: DW_TAG_member, name: "pending", scope: !130, file: !118, line: 26, baseType: !133, size: 8, offset: 8)
!137 = !DIDerivedType(tag: DW_TAG_member, scope: !122, file: !118, line: 28, baseType: !138, size: 32)
!138 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !122, file: !118, line: 28, size: 32, elements: !139)
!139 = !{!140, !144}
!140 = !DIDerivedType(tag: DW_TAG_member, name: "locked_pending", scope: !138, file: !118, line: 29, baseType: !141, size: 16)
!141 = !DIDerivedType(tag: DW_TAG_typedef, name: "u16", file: !107, line: 19, baseType: !142)
!142 = !DIDerivedType(tag: DW_TAG_typedef, name: "__u16", file: !109, line: 24, baseType: !143)
!143 = !DIBasicType(name: "unsigned short", size: 16, encoding: DW_ATE_unsigned)
!144 = !DIDerivedType(tag: DW_TAG_member, name: "tail", scope: !138, file: !118, line: 30, baseType: !141, size: 16, offset: 16)
!145 = !DIDerivedType(tag: DW_TAG_member, name: "osq", scope: !96, file: !97, line: 67, baseType: !146, size: 32, offset: 96)
!146 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "optimistic_spin_queue", file: !147, line: 15, size: 32, elements: !148)
!147 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/osq_lock.h", directory: "", checksumkind: CSK_MD5, checksum: "96068aa09fa474bd706a2c16987ff9d4")
!148 = !{!149}
!149 = !DIDerivedType(tag: DW_TAG_member, name: "tail", scope: !146, file: !147, line: 20, baseType: !125, size: 32)
!150 = !DIDerivedType(tag: DW_TAG_member, name: "wait_list", scope: !96, file: !97, line: 69, baseType: !151, size: 128, offset: 128)
!151 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "list_head", file: !51, line: 178, size: 128, elements: !152)
!152 = !{!153, !155}
!153 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !151, file: !51, line: 179, baseType: !154, size: 64)
!154 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !151, size: 64)
!155 = !DIDerivedType(tag: DW_TAG_member, name: "prev", scope: !151, file: !51, line: 179, baseType: !154, size: 64, offset: 64)
!156 = !DIDerivedType(tag: DW_TAG_member, name: "srcu", scope: !93, file: !35, line: 76, baseType: !157, size: 6336, offset: 256)
!157 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "srcu_struct", file: !158, line: 64, size: 6336, elements: !159)
!158 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/srcutree.h", directory: "", checksumkind: CSK_MD5, checksum: "5795159b423614242e774c7673f35ed0")
!159 = !{!160, !185, !189, !190, !191, !192, !193, !194, !195, !196, !197, !272, !273, !274, !285, !286, !295}
!160 = !DIDerivedType(tag: DW_TAG_member, name: "node", scope: !157, file: !158, line: 65, baseType: !161, size: 3840)
!161 = !DICompositeType(tag: DW_TAG_array_type, baseType: !162, size: 3840, elements: !183)
!162 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "srcu_node", file: !158, line: 48, size: 768, elements: !163)
!163 = !{!164, !173, !177, !178, !179, !181, !182}
!164 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !162, file: !158, line: 49, baseType: !165, size: 32)
!165 = !DIDerivedType(tag: DW_TAG_typedef, name: "spinlock_t", file: !166, line: 29, baseType: !167)
!166 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/spinlock_types.h", directory: "", checksumkind: CSK_MD5, checksum: "fc7950471ffdc176b6c133b1f7370f88")
!167 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "spinlock", file: !166, line: 17, size: 32, elements: !168)
!168 = !{!169}
!169 = !DIDerivedType(tag: DW_TAG_member, scope: !167, file: !166, line: 18, baseType: !170, size: 32)
!170 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !167, file: !166, line: 18, size: 32, elements: !171)
!171 = !{!172}
!172 = !DIDerivedType(tag: DW_TAG_member, name: "rlock", scope: !170, file: !166, line: 19, baseType: !114, size: 32)
!173 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_have_cbs", scope: !162, file: !158, line: 50, baseType: !174, size: 256, offset: 64)
!174 = !DICompositeType(tag: DW_TAG_array_type, baseType: !43, size: 256, elements: !175)
!175 = !{!176}
!176 = !DISubrange(count: 4)
!177 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_data_have_cbs", scope: !162, file: !158, line: 53, baseType: !174, size: 256, offset: 320)
!178 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_seq_needed_exp", scope: !162, file: !158, line: 55, baseType: !43, size: 64, offset: 576)
!179 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_parent", scope: !162, file: !158, line: 56, baseType: !180, size: 64, offset: 640)
!180 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !162, size: 64)
!181 = !DIDerivedType(tag: DW_TAG_member, name: "grplo", scope: !162, file: !158, line: 57, baseType: !42, size: 32, offset: 704)
!182 = !DIDerivedType(tag: DW_TAG_member, name: "grphi", scope: !162, file: !158, line: 58, baseType: !42, size: 32, offset: 736)
!183 = !{!184}
!184 = !DISubrange(count: 5)
!185 = !DIDerivedType(tag: DW_TAG_member, name: "level", scope: !157, file: !158, line: 66, baseType: !186, size: 192, offset: 3840)
!186 = !DICompositeType(tag: DW_TAG_array_type, baseType: !180, size: 192, elements: !187)
!187 = !{!188}
!188 = !DISubrange(count: 3)
!189 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_cb_mutex", scope: !157, file: !158, line: 68, baseType: !96, size: 256, offset: 4032)
!190 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !157, file: !158, line: 69, baseType: !165, size: 32, offset: 4288)
!191 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_mutex", scope: !157, file: !158, line: 70, baseType: !96, size: 256, offset: 4352)
!192 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_idx", scope: !157, file: !158, line: 71, baseType: !7, size: 32, offset: 4608)
!193 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_seq", scope: !157, file: !158, line: 72, baseType: !43, size: 64, offset: 4672)
!194 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_seq_needed", scope: !157, file: !158, line: 73, baseType: !43, size: 64, offset: 4736)
!195 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_seq_needed_exp", scope: !157, file: !158, line: 74, baseType: !43, size: 64, offset: 4800)
!196 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_last_gp_end", scope: !157, file: !158, line: 75, baseType: !43, size: 64, offset: 4864)
!197 = !DIDerivedType(tag: DW_TAG_member, name: "sda", scope: !157, file: !158, line: 76, baseType: !198, size: 64, offset: 4928)
!198 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !199, size: 64)
!199 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "srcu_data", file: !158, line: 24, size: 3072, elements: !200)
!200 = !{!201, !205, !206, !207, !229, !230, !231, !234, !254, !266, !267, !268, !269, !270}
!201 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_lock_count", scope: !199, file: !158, line: 26, baseType: !202, size: 128)
!202 = !DICompositeType(tag: DW_TAG_array_type, baseType: !43, size: 128, elements: !203)
!203 = !{!204}
!204 = !DISubrange(count: 2)
!205 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_unlock_count", scope: !199, file: !158, line: 27, baseType: !202, size: 128, offset: 128)
!206 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !199, file: !158, line: 30, baseType: !165, size: 32, align: 512, offset: 512)
!207 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_cblist", scope: !199, file: !158, line: 31, baseType: !208, size: 960, offset: 576)
!208 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "rcu_segcblist", file: !209, line: 183, size: 960, elements: !210)
!209 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/rcu_segcblist.h", directory: "", checksumkind: CSK_MD5, checksum: "e310feaf37ebc885db21b7e2e0db32a8")
!210 = !{!211, !220, !223, !224, !226, !228}
!211 = !DIDerivedType(tag: DW_TAG_member, name: "head", scope: !208, file: !209, line: 184, baseType: !212, size: 64)
!212 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !213, size: 64)
!213 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "callback_head", file: !51, line: 220, size: 128, align: 64, elements: !214)
!214 = !{!215, !216}
!215 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !213, file: !51, line: 221, baseType: !212, size: 64)
!216 = !DIDerivedType(tag: DW_TAG_member, name: "func", scope: !213, file: !51, line: 222, baseType: !217, size: 64, offset: 64)
!217 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !218, size: 64)
!218 = !DISubroutineType(types: !219)
!219 = !{null, !212}
!220 = !DIDerivedType(tag: DW_TAG_member, name: "tails", scope: !208, file: !209, line: 185, baseType: !221, size: 256, offset: 64)
!221 = !DICompositeType(tag: DW_TAG_array_type, baseType: !222, size: 256, elements: !175)
!222 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !212, size: 64)
!223 = !DIDerivedType(tag: DW_TAG_member, name: "gp_seq", scope: !208, file: !209, line: 186, baseType: !174, size: 256, offset: 320)
!224 = !DIDerivedType(tag: DW_TAG_member, name: "len", scope: !208, file: !209, line: 190, baseType: !225, size: 64, offset: 576)
!225 = !DIBasicType(name: "long", size: 64, encoding: DW_ATE_signed)
!226 = !DIDerivedType(tag: DW_TAG_member, name: "seglen", scope: !208, file: !209, line: 192, baseType: !227, size: 256, offset: 640)
!227 = !DICompositeType(tag: DW_TAG_array_type, baseType: !225, size: 256, elements: !175)
!228 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !208, file: !209, line: 193, baseType: !133, size: 8, offset: 896)
!229 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_seq_needed", scope: !199, file: !158, line: 32, baseType: !43, size: 64, offset: 1536)
!230 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_gp_seq_needed_exp", scope: !199, file: !158, line: 33, baseType: !43, size: 64, offset: 1600)
!231 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_cblist_invoking", scope: !199, file: !158, line: 34, baseType: !232, size: 8, offset: 1664)
!232 = !DIDerivedType(tag: DW_TAG_typedef, name: "bool", file: !51, line: 30, baseType: !233)
!233 = !DIBasicType(name: "_Bool", size: 8, encoding: DW_ATE_boolean)
!234 = !DIDerivedType(tag: DW_TAG_member, name: "delay_work", scope: !199, file: !158, line: 35, baseType: !235, size: 320, offset: 1728)
!235 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "timer_list", file: !236, line: 11, size: 320, elements: !237)
!236 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/timer.h", directory: "", checksumkind: CSK_MD5, checksum: "cefedf967707c4a35c9449708be17b92")
!237 = !{!238, !245, !246, !251}
!238 = !DIDerivedType(tag: DW_TAG_member, name: "entry", scope: !235, file: !236, line: 16, baseType: !239, size: 128)
!239 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "hlist_node", file: !51, line: 186, size: 128, elements: !240)
!240 = !{!241, !243}
!241 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !239, file: !51, line: 187, baseType: !242, size: 64)
!242 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !239, size: 64)
!243 = !DIDerivedType(tag: DW_TAG_member, name: "pprev", scope: !239, file: !51, line: 187, baseType: !244, size: 64, offset: 64)
!244 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !242, size: 64)
!245 = !DIDerivedType(tag: DW_TAG_member, name: "expires", scope: !235, file: !236, line: 17, baseType: !43, size: 64, offset: 128)
!246 = !DIDerivedType(tag: DW_TAG_member, name: "function", scope: !235, file: !236, line: 18, baseType: !247, size: 64, offset: 192)
!247 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !248, size: 64)
!248 = !DISubroutineType(types: !249)
!249 = !{null, !250}
!250 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !235, size: 64)
!251 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !235, file: !236, line: 19, baseType: !252, size: 32, offset: 256)
!252 = !DIDerivedType(tag: DW_TAG_typedef, name: "u32", file: !107, line: 21, baseType: !253)
!253 = !DIDerivedType(tag: DW_TAG_typedef, name: "__u32", file: !109, line: 27, baseType: !7)
!254 = !DIDerivedType(tag: DW_TAG_member, name: "work", scope: !199, file: !158, line: 36, baseType: !255, size: 256, offset: 2048)
!255 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "work_struct", file: !256, line: 97, size: 256, elements: !257)
!256 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/workqueue.h", directory: "", checksumkind: CSK_MD5, checksum: "d3070c6f8f64827d6e9d8ac1ced56114")
!257 = !{!258, !259, !260}
!258 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !255, file: !256, line: 98, baseType: !100, size: 64)
!259 = !DIDerivedType(tag: DW_TAG_member, name: "entry", scope: !255, file: !256, line: 99, baseType: !151, size: 128, offset: 64)
!260 = !DIDerivedType(tag: DW_TAG_member, name: "func", scope: !255, file: !256, line: 100, baseType: !261, size: 64, offset: 192)
!261 = !DIDerivedType(tag: DW_TAG_typedef, name: "work_func_t", file: !256, line: 21, baseType: !262)
!262 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !263, size: 64)
!263 = !DISubroutineType(types: !264)
!264 = !{null, !265}
!265 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !255, size: 64)
!266 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_barrier_head", scope: !199, file: !158, line: 37, baseType: !213, size: 128, align: 64, offset: 2304)
!267 = !DIDerivedType(tag: DW_TAG_member, name: "mynode", scope: !199, file: !158, line: 38, baseType: !180, size: 64, offset: 2432)
!268 = !DIDerivedType(tag: DW_TAG_member, name: "grpmask", scope: !199, file: !158, line: 39, baseType: !43, size: 64, offset: 2496)
!269 = !DIDerivedType(tag: DW_TAG_member, name: "cpu", scope: !199, file: !158, line: 41, baseType: !42, size: 32, offset: 2560)
!270 = !DIDerivedType(tag: DW_TAG_member, name: "ssp", scope: !199, file: !158, line: 42, baseType: !271, size: 64, offset: 2624)
!271 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !157, size: 64)
!272 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_barrier_seq", scope: !157, file: !158, line: 77, baseType: !43, size: 64, offset: 4992)
!273 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_barrier_mutex", scope: !157, file: !158, line: 78, baseType: !96, size: 256, offset: 5056)
!274 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_barrier_completion", scope: !157, file: !158, line: 79, baseType: !275, size: 256, offset: 5312)
!275 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "completion", file: !276, line: 26, size: 256, elements: !277)
!276 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/completion.h", directory: "", checksumkind: CSK_MD5, checksum: "0877760728cd9fc1246ca00da2b808a1")
!277 = !{!278, !279}
!278 = !DIDerivedType(tag: DW_TAG_member, name: "done", scope: !275, file: !276, line: 27, baseType: !7, size: 32)
!279 = !DIDerivedType(tag: DW_TAG_member, name: "wait", scope: !275, file: !276, line: 28, baseType: !280, size: 192, offset: 64)
!280 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "swait_queue_head", file: !281, line: 43, size: 192, elements: !282)
!281 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/swait.h", directory: "", checksumkind: CSK_MD5, checksum: "426b09bb279807aa78d8e2ce484d86d2")
!282 = !{!283, !284}
!283 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !280, file: !281, line: 44, baseType: !112, size: 32)
!284 = !DIDerivedType(tag: DW_TAG_member, name: "task_list", scope: !280, file: !281, line: 45, baseType: !151, size: 128, offset: 64)
!285 = !DIDerivedType(tag: DW_TAG_member, name: "srcu_barrier_cpu_cnt", scope: !157, file: !158, line: 81, baseType: !125, size: 32, offset: 5568)
!286 = !DIDerivedType(tag: DW_TAG_member, name: "work", scope: !157, file: !158, line: 84, baseType: !287, size: 704, offset: 5632)
!287 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "delayed_work", file: !256, line: 110, size: 704, elements: !288)
!288 = !{!289, !290, !291, !294}
!289 = !DIDerivedType(tag: DW_TAG_member, name: "work", scope: !287, file: !256, line: 111, baseType: !255, size: 256)
!290 = !DIDerivedType(tag: DW_TAG_member, name: "timer", scope: !287, file: !256, line: 112, baseType: !235, size: 320, offset: 256)
!291 = !DIDerivedType(tag: DW_TAG_member, name: "wq", scope: !287, file: !256, line: 115, baseType: !292, size: 64, offset: 576)
!292 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !293, size: 64)
!293 = !DICompositeType(tag: DW_TAG_structure_type, name: "workqueue_struct", file: !256, line: 18, flags: DIFlagFwdDecl)
!294 = !DIDerivedType(tag: DW_TAG_member, name: "cpu", scope: !287, file: !256, line: 116, baseType: !42, size: 32, offset: 640)
!295 = !DIDerivedType(tag: DW_TAG_member, name: "dep_map", scope: !157, file: !158, line: 85, baseType: !296, offset: 6336)
!296 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "lockdep_map", file: !297, line: 202, elements: !298)
!297 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/lockdep_types.h", directory: "", checksumkind: CSK_MD5, checksum: "18c928ea1fb6a1d5566b79fa3e19ebc8")
!298 = !{}
!299 = !DIDerivedType(tag: DW_TAG_member, name: "head", scope: !93, file: !35, line: 77, baseType: !33, size: 64, offset: 6592)
!300 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "lock_class_key", file: !297, line: 197, elements: !298)
!301 = !DIGlobalVariableExpression(var: !302, expr: !DIExpression())
!302 = distinct !DIGlobalVariable(scope: null, file: !56, line: 506, type: !303, isLocal: true, isDefinition: true)
!303 = !DICompositeType(tag: DW_TAG_array_type, baseType: !304, size: 88, elements: !305)
!304 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!305 = !{!306}
!306 = !DISubrange(count: 11)
!307 = !DIGlobalVariableExpression(var: !308, expr: !DIExpression())
!308 = distinct !DIGlobalVariable(scope: null, file: !56, line: 508, type: !309, isLocal: true, isDefinition: true)
!309 = !DICompositeType(tag: DW_TAG_array_type, baseType: !304, size: 488, elements: !310)
!310 = !{!311}
!311 = !DISubrange(count: 61)
!312 = !DIGlobalVariableExpression(var: !313, expr: !DIExpression())
!313 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_srcu_init_notifier_head269", scope: !2, file: !56, line: 511, type: !44, isLocal: true, isDefinition: true)
!314 = !DIGlobalVariableExpression(var: !315, expr: !DIExpression())
!315 = distinct !DIGlobalVariable(name: "_kbl_addr_notify_die", scope: !2, file: !56, line: 532, type: !43, isLocal: true, isDefinition: true)
!316 = !DIGlobalVariableExpression(var: !317, expr: !DIExpression())
!317 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_register_die_notifier270", scope: !2, file: !56, line: 538, type: !44, isLocal: true, isDefinition: true)
!318 = !DIGlobalVariableExpression(var: !319, expr: !DIExpression())
!319 = distinct !DIGlobalVariable(name: "__UNIQUE_ID___addressable_unregister_die_notifier271", scope: !2, file: !56, line: 544, type: !44, isLocal: true, isDefinition: true)
!320 = !DIGlobalVariableExpression(var: !321, expr: !DIExpression())
!321 = distinct !DIGlobalVariable(scope: null, file: !56, line: 27, type: !322, isLocal: true, isDefinition: true)
!322 = !DICompositeType(tag: DW_TAG_array_type, baseType: !304, size: 328, elements: !323)
!323 = !{!324}
!324 = !DISubrange(count: 41)
!325 = !DIGlobalVariableExpression(var: !326, expr: !DIExpression())
!326 = distinct !DIGlobalVariable(scope: null, file: !327, line: 188, type: !328, isLocal: true, isDefinition: true)
!327 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/srcu.h", directory: "", checksumkind: CSK_MD5, checksum: "1f668945a69809da3a8ed022320a6dd9")
!328 = !DICompositeType(tag: DW_TAG_array_type, baseType: !304, size: 512, elements: !329)
!329 = !{!330}
!330 = !DISubrange(count: 64)
!331 = !DIGlobalVariableExpression(var: !332, expr: !DIExpression())
!332 = distinct !DIGlobalVariable(name: "die_chain", scope: !2, file: !56, line: 515, type: !333, isLocal: true, isDefinition: true)
!333 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "atomic_notifier_head", file: !35, line: 60, size: 128, elements: !334)
!334 = !{!335, !336}
!335 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !333, file: !35, line: 61, baseType: !165, size: 32)
!336 = !DIDerivedType(tag: DW_TAG_member, name: "head", scope: !333, file: !35, line: 62, baseType: !33, size: 64, offset: 64)
!337 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "blocking_notifier_head", file: !35, line: 65, size: 384, elements: !338)
!338 = !{!339, !348}
!339 = !DIDerivedType(tag: DW_TAG_member, name: "rwsem", scope: !337, file: !35, line: 66, baseType: !340, size: 320)
!340 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "rw_semaphore", file: !341, line: 47, size: 320, elements: !342)
!341 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/rwsem.h", directory: "", checksumkind: CSK_MD5, checksum: "eb14f5aa10047e46d0b066935bb4d043")
!342 = !{!343, !344, !345, !346, !347}
!343 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !340, file: !341, line: 48, baseType: !100, size: 64)
!344 = !DIDerivedType(tag: DW_TAG_member, name: "owner", scope: !340, file: !341, line: 54, baseType: !100, size: 64, offset: 64)
!345 = !DIDerivedType(tag: DW_TAG_member, name: "osq", scope: !340, file: !341, line: 56, baseType: !146, size: 32, offset: 128)
!346 = !DIDerivedType(tag: DW_TAG_member, name: "wait_lock", scope: !340, file: !341, line: 58, baseType: !112, size: 32, offset: 160)
!347 = !DIDerivedType(tag: DW_TAG_member, name: "wait_list", scope: !340, file: !341, line: 59, baseType: !151, size: 128, offset: 192)
!348 = !DIDerivedType(tag: DW_TAG_member, name: "head", scope: !337, file: !35, line: 67, baseType: !33, size: 64, offset: 320)
!349 = !{i32 7, !"Dwarf Version", i32 5}
!350 = !{i32 2, !"Debug Info Version", i32 3}
!351 = !{i32 1, !"wchar_size", i32 4}
!352 = !{i32 7, !"uwtable", i32 2}
!353 = !{i32 7, !"frame-pointer", i32 2}
!354 = !{!"Debian clang version 15.0.6"}
!355 = distinct !DISubprogram(name: "notifier_call_chain", scope: !56, file: !56, line: 65, type: !356, scopeLine: 68, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !298)
!356 = !DISubroutineType(types: !357)
!357 = !{!42, !358, !43, !44, !42, !359}
!358 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !33, size: 64)
!359 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !42, size: 64)
!360 = !DILocalVariable(name: "nl", arg: 1, scope: !355, file: !56, line: 65, type: !358)
!361 = !DILocation(line: 65, column: 56, scope: !355)
!362 = !DILocalVariable(name: "val", arg: 2, scope: !355, file: !56, line: 66, type: !43)
!363 = !DILocation(line: 66, column: 25, scope: !355)
!364 = !DILocalVariable(name: "v", arg: 3, scope: !355, file: !56, line: 66, type: !44)
!365 = !DILocation(line: 66, column: 36, scope: !355)
!366 = !DILocalVariable(name: "nr_to_call", arg: 4, scope: !355, file: !56, line: 67, type: !42)
!367 = !DILocation(line: 67, column: 15, scope: !355)
!368 = !DILocalVariable(name: "nr_calls", arg: 5, scope: !355, file: !56, line: 67, type: !359)
!369 = !DILocation(line: 67, column: 32, scope: !355)
!370 = !DILocalVariable(name: "ret", scope: !355, file: !56, line: 69, type: !42)
!371 = !DILocation(line: 69, column: 6, scope: !355)
!372 = !DILocalVariable(name: "nb", scope: !355, file: !56, line: 70, type: !33)
!373 = !DILocation(line: 70, column: 25, scope: !355)
!374 = !DILocalVariable(name: "next_nb", scope: !355, file: !56, line: 70, type: !33)
!375 = !DILocation(line: 70, column: 30, scope: !355)
!376 = !DILocalVariable(name: "________p1", scope: !377, file: !56, line: 72, type: !33)
!377 = distinct !DILexicalBlock(scope: !355, file: !56, line: 72, column: 7)
!378 = !DILocation(line: 72, column: 7, scope: !377)
!379 = !DILocation(line: 72, column: 7, scope: !380)
!380 = distinct !DILexicalBlock(scope: !377, file: !56, line: 72, column: 7)
!381 = !DILocation(line: 72, column: 7, scope: !382)
!382 = distinct !DILexicalBlock(scope: !380, file: !56, line: 72, column: 7)
!383 = !DILocation(line: 72, column: 5, scope: !355)
!384 = !DILocation(line: 74, column: 2, scope: !355)
!385 = !DILocation(line: 74, column: 9, scope: !355)
!386 = !DILocation(line: 74, column: 12, scope: !355)
!387 = !DILocation(line: 74, column: 15, scope: !355)
!388 = !DILocation(line: 0, scope: !355)
!389 = !DILocalVariable(name: "________p1", scope: !390, file: !56, line: 75, type: !33)
!390 = distinct !DILexicalBlock(scope: !391, file: !56, line: 75, column: 13)
!391 = distinct !DILexicalBlock(scope: !355, file: !56, line: 74, column: 27)
!392 = !DILocation(line: 75, column: 13, scope: !390)
!393 = !DILocation(line: 75, column: 13, scope: !394)
!394 = distinct !DILexicalBlock(scope: !390, file: !56, line: 75, column: 13)
!395 = !DILocation(line: 75, column: 13, scope: !396)
!396 = distinct !DILexicalBlock(scope: !394, file: !56, line: 75, column: 13)
!397 = !DILocation(line: 75, column: 11, scope: !391)
!398 = !DILocation(line: 84, column: 9, scope: !391)
!399 = !DILocation(line: 84, column: 13, scope: !391)
!400 = !DILocation(line: 84, column: 27, scope: !391)
!401 = !DILocation(line: 84, column: 31, scope: !391)
!402 = !DILocation(line: 84, column: 36, scope: !391)
!403 = !DILocation(line: 84, column: 7, scope: !391)
!404 = !DILocation(line: 86, column: 7, scope: !405)
!405 = distinct !DILexicalBlock(scope: !391, file: !56, line: 86, column: 7)
!406 = !DILocation(line: 86, column: 7, scope: !391)
!407 = !DILocation(line: 87, column: 6, scope: !405)
!408 = !DILocation(line: 87, column: 15, scope: !405)
!409 = !DILocation(line: 87, column: 4, scope: !405)
!410 = !DILocation(line: 89, column: 7, scope: !411)
!411 = distinct !DILexicalBlock(scope: !391, file: !56, line: 89, column: 7)
!412 = !DILocation(line: 89, column: 11, scope: !411)
!413 = !DILocation(line: 89, column: 7, scope: !391)
!414 = !DILocation(line: 90, column: 4, scope: !411)
!415 = !DILocation(line: 91, column: 8, scope: !391)
!416 = !DILocation(line: 91, column: 6, scope: !391)
!417 = !DILocation(line: 92, column: 13, scope: !391)
!418 = distinct !{!418, !384, !419, !420}
!419 = !DILocation(line: 93, column: 2, scope: !355)
!420 = !{!"llvm.loop.mustprogress"}
!421 = !DILocation(line: 94, column: 9, scope: !355)
!422 = !DILocation(line: 94, column: 2, scope: !355)
!423 = distinct !DISubprogram(name: "atomic_notifier_chain_register", scope: !56, file: !56, line: 140, type: !424, scopeLine: 142, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!424 = !DISubroutineType(types: !425)
!425 = !{!42, !426, !33}
!426 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !333, size: 64)
!427 = !DILocalVariable(name: "nh", arg: 1, scope: !423, file: !56, line: 140, type: !426)
!428 = !DILocation(line: 140, column: 65, scope: !423)
!429 = !DILocalVariable(name: "n", arg: 2, scope: !423, file: !56, line: 141, type: !33)
!430 = !DILocation(line: 141, column: 26, scope: !423)
!431 = !DILocalVariable(name: "flags", scope: !423, file: !56, line: 143, type: !43)
!432 = !DILocation(line: 143, column: 16, scope: !423)
!433 = !DILocalVariable(name: "ret", scope: !423, file: !56, line: 144, type: !42)
!434 = !DILocation(line: 144, column: 6, scope: !423)
!435 = !DILocation(line: 146, column: 2, scope: !423)
!436 = !DILocation(line: 146, column: 2, scope: !437)
!437 = distinct !DILexicalBlock(scope: !423, file: !56, line: 146, column: 2)
!438 = !DILocalVariable(name: "__dummy", scope: !439, file: !56, line: 146, type: !43)
!439 = distinct !DILexicalBlock(scope: !440, file: !56, line: 146, column: 2)
!440 = distinct !DILexicalBlock(scope: !437, file: !56, line: 146, column: 2)
!441 = !DILocation(line: 146, column: 2, scope: !439)
!442 = !DILocalVariable(name: "__dummy2", scope: !439, file: !56, line: 146, type: !43)
!443 = !DILocation(line: 146, column: 2, scope: !440)
!444 = !DILocalVariable(name: "lock", arg: 1, scope: !445, file: !446, line: 322, type: !450)
!445 = distinct !DISubprogram(name: "spinlock_check", scope: !446, file: !446, line: 322, type: !447, scopeLine: 323, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !298)
!446 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/spinlock.h", directory: "", checksumkind: CSK_MD5, checksum: "2047b391ef24ec0bc511590a513eb610")
!447 = !DISubroutineType(types: !448)
!448 = !{!449, !450}
!449 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !112, size: 64)
!450 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !165, size: 64)
!451 = !DILocation(line: 322, column: 67, scope: !445, inlinedAt: !452)
!452 = distinct !DILocation(line: 146, column: 2, scope: !440)
!453 = !DILocation(line: 324, column: 10, scope: !445, inlinedAt: !452)
!454 = !DILocation(line: 147, column: 33, scope: !423)
!455 = !DILocation(line: 147, column: 37, scope: !423)
!456 = !DILocation(line: 147, column: 43, scope: !423)
!457 = !DILocation(line: 147, column: 8, scope: !423)
!458 = !DILocation(line: 147, column: 6, scope: !423)
!459 = !DILocation(line: 148, column: 26, scope: !423)
!460 = !DILocation(line: 148, column: 30, scope: !423)
!461 = !DILocation(line: 148, column: 36, scope: !423)
!462 = !DILocalVariable(name: "lock", arg: 1, scope: !463, file: !446, line: 402, type: !450)
!463 = distinct !DISubprogram(name: "spin_unlock_irqrestore", scope: !446, file: !446, line: 402, type: !464, scopeLine: 403, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !298)
!464 = !DISubroutineType(types: !465)
!465 = !{null, !450, !43}
!466 = !DILocation(line: 402, column: 64, scope: !463, inlinedAt: !467)
!467 = distinct !DILocation(line: 148, column: 2, scope: !423)
!468 = !DILocalVariable(name: "flags", arg: 2, scope: !463, file: !446, line: 402, type: !43)
!469 = !DILocation(line: 402, column: 84, scope: !463, inlinedAt: !467)
!470 = !DILocalVariable(name: "__dummy", scope: !471, file: !446, line: 404, type: !43)
!471 = distinct !DILexicalBlock(scope: !472, file: !446, line: 404, column: 2)
!472 = distinct !DILexicalBlock(scope: !463, file: !446, line: 404, column: 2)
!473 = !DILocation(line: 404, column: 2, scope: !471, inlinedAt: !467)
!474 = !DILocalVariable(name: "__dummy2", scope: !471, file: !446, line: 404, type: !43)
!475 = !DILocation(line: 404, column: 2, scope: !472, inlinedAt: !467)
!476 = !DILocation(line: 149, column: 9, scope: !423)
!477 = !DILocation(line: 149, column: 2, scope: !423)
!478 = distinct !DISubprogram(name: "notifier_chain_register", scope: !56, file: !56, line: 22, type: !479, scopeLine: 24, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !298)
!479 = !DISubroutineType(types: !480)
!480 = !{!42, !358, !33}
!481 = !DILocalVariable(name: "nl", arg: 1, scope: !478, file: !56, line: 22, type: !358)
!482 = !DILocation(line: 22, column: 60, scope: !478)
!483 = !DILocalVariable(name: "n", arg: 2, scope: !478, file: !56, line: 23, type: !33)
!484 = !DILocation(line: 23, column: 31, scope: !478)
!485 = !DILocation(line: 25, column: 2, scope: !478)
!486 = !DILocation(line: 25, column: 11, scope: !478)
!487 = !DILocation(line: 25, column: 10, scope: !478)
!488 = !DILocation(line: 25, column: 15, scope: !478)
!489 = !DILocation(line: 26, column: 7, scope: !490)
!490 = distinct !DILexicalBlock(scope: !491, file: !56, line: 26, column: 7)
!491 = distinct !DILexicalBlock(scope: !478, file: !56, line: 25, column: 24)
!492 = !DILocation(line: 26, column: 7, scope: !491)
!493 = !DILocalVariable(name: "__ret_warn_on", scope: !494, file: !56, line: 27, type: !42)
!494 = distinct !DILexicalBlock(scope: !495, file: !56, line: 27, column: 4)
!495 = distinct !DILexicalBlock(scope: !490, file: !56, line: 26, column: 29)
!496 = !DILocation(line: 27, column: 4, scope: !494)
!497 = !DILocation(line: 27, column: 4, scope: !498)
!498 = distinct !DILexicalBlock(scope: !494, file: !56, line: 27, column: 4)
!499 = !DILocation(line: 27, column: 4, scope: !500)
!500 = distinct !DILexicalBlock(scope: !498, file: !56, line: 27, column: 4)
!501 = !DILocation(line: 27, column: 4, scope: !502)
!502 = distinct !DILexicalBlock(scope: !500, file: !56, line: 27, column: 4)
!503 = !DILocation(line: 27, column: 4, scope: !504)
!504 = distinct !DILexicalBlock(scope: !500, file: !56, line: 27, column: 4)
!505 = !DILocation(line: 27, column: 4, scope: !506)
!506 = distinct !DILexicalBlock(scope: !504, file: !56, line: 27, column: 4)
!507 = !DILocation(line: 27, column: 4, scope: !508)
!508 = distinct !DILexicalBlock(scope: !504, file: !56, line: 27, column: 4)
!509 = !{i64 2152919806, i64 2152919835, i64 2152919881, i64 2152919939, i64 2152919993, i64 2152920047, i64 2152920102, i64 2152920133}
!510 = !DILocation(line: 27, column: 4, scope: !511)
!511 = distinct !DILexicalBlock(scope: !504, file: !56, line: 27, column: 4)
!512 = !{i64 2152920825, i64 2152920652, i64 2152920701, i64 2152920753, i64 2152920781}
!513 = !DILocation(line: 27, column: 4, scope: !514)
!514 = distinct !DILexicalBlock(scope: !504, file: !56, line: 27, column: 4)
!515 = !DILocation(line: 27, column: 4, scope: !516)
!516 = distinct !DILexicalBlock(scope: !500, file: !56, line: 27, column: 4)
!517 = !DILocation(line: 29, column: 4, scope: !495)
!518 = !DILocation(line: 31, column: 7, scope: !519)
!519 = distinct !DILexicalBlock(scope: !491, file: !56, line: 31, column: 7)
!520 = !DILocation(line: 31, column: 10, scope: !519)
!521 = !DILocation(line: 31, column: 23, scope: !519)
!522 = !DILocation(line: 31, column: 22, scope: !519)
!523 = !DILocation(line: 31, column: 28, scope: !519)
!524 = !DILocation(line: 31, column: 19, scope: !519)
!525 = !DILocation(line: 31, column: 7, scope: !491)
!526 = !DILocation(line: 32, column: 4, scope: !519)
!527 = !DILocation(line: 33, column: 12, scope: !491)
!528 = !DILocation(line: 33, column: 11, scope: !491)
!529 = !DILocation(line: 33, column: 17, scope: !491)
!530 = !DILocation(line: 33, column: 6, scope: !491)
!531 = distinct !{!531, !485, !532, !420}
!532 = !DILocation(line: 34, column: 2, scope: !478)
!533 = !DILocation(line: 35, column: 13, scope: !478)
!534 = !DILocation(line: 35, column: 12, scope: !478)
!535 = !DILocation(line: 35, column: 2, scope: !478)
!536 = !DILocation(line: 35, column: 5, scope: !478)
!537 = !DILocation(line: 35, column: 10, scope: !478)
!538 = !DILocation(line: 36, column: 2, scope: !478)
!539 = !DILocalVariable(name: "_r_a_p__v", scope: !540, file: !56, line: 36, type: !50)
!540 = distinct !DILexicalBlock(scope: !478, file: !56, line: 36, column: 2)
!541 = !DILocation(line: 36, column: 2, scope: !540)
!542 = !DILocation(line: 36, column: 2, scope: !543)
!543 = distinct !DILexicalBlock(scope: !540, file: !56, line: 36, column: 2)
!544 = !DILocation(line: 36, column: 2, scope: !545)
!545 = distinct !DILexicalBlock(scope: !543, file: !56, line: 36, column: 2)
!546 = !DILocation(line: 36, column: 2, scope: !547)
!547 = distinct !DILexicalBlock(scope: !545, file: !56, line: 36, column: 2)
!548 = !DILocation(line: 36, column: 2, scope: !549)
!549 = distinct !DILexicalBlock(scope: !545, file: !56, line: 36, column: 2)
!550 = !DILocation(line: 36, column: 2, scope: !551)
!551 = distinct !DILexicalBlock(scope: !543, file: !56, line: 36, column: 2)
!552 = !DILocation(line: 36, column: 2, scope: !553)
!553 = distinct !DILexicalBlock(scope: !551, file: !56, line: 36, column: 2)
!554 = !{i64 2152923790}
!555 = !DILocation(line: 36, column: 2, scope: !556)
!556 = distinct !DILexicalBlock(scope: !551, file: !56, line: 36, column: 2)
!557 = !DILocation(line: 36, column: 2, scope: !558)
!558 = distinct !DILexicalBlock(scope: !556, file: !56, line: 36, column: 2)
!559 = !DILocation(line: 36, column: 2, scope: !560)
!560 = distinct !DILexicalBlock(scope: !556, file: !56, line: 36, column: 2)
!561 = !DILocation(line: 37, column: 2, scope: !478)
!562 = !DILocation(line: 38, column: 1, scope: !478)
!563 = distinct !DISubprogram(name: "atomic_notifier_chain_unregister", scope: !56, file: !56, line: 162, type: !424, scopeLine: 164, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!564 = !DILocalVariable(name: "nh", arg: 1, scope: !563, file: !56, line: 162, type: !426)
!565 = !DILocation(line: 162, column: 67, scope: !563)
!566 = !DILocalVariable(name: "n", arg: 2, scope: !563, file: !56, line: 163, type: !33)
!567 = !DILocation(line: 163, column: 26, scope: !563)
!568 = !DILocalVariable(name: "flags", scope: !563, file: !56, line: 165, type: !43)
!569 = !DILocation(line: 165, column: 16, scope: !563)
!570 = !DILocalVariable(name: "ret", scope: !563, file: !56, line: 166, type: !42)
!571 = !DILocation(line: 166, column: 6, scope: !563)
!572 = !DILocation(line: 168, column: 2, scope: !563)
!573 = !DILocation(line: 168, column: 2, scope: !574)
!574 = distinct !DILexicalBlock(scope: !563, file: !56, line: 168, column: 2)
!575 = !DILocalVariable(name: "__dummy", scope: !576, file: !56, line: 168, type: !43)
!576 = distinct !DILexicalBlock(scope: !577, file: !56, line: 168, column: 2)
!577 = distinct !DILexicalBlock(scope: !574, file: !56, line: 168, column: 2)
!578 = !DILocation(line: 168, column: 2, scope: !576)
!579 = !DILocalVariable(name: "__dummy2", scope: !576, file: !56, line: 168, type: !43)
!580 = !DILocation(line: 168, column: 2, scope: !577)
!581 = !DILocation(line: 322, column: 67, scope: !445, inlinedAt: !582)
!582 = distinct !DILocation(line: 168, column: 2, scope: !577)
!583 = !DILocation(line: 324, column: 10, scope: !445, inlinedAt: !582)
!584 = !DILocation(line: 169, column: 35, scope: !563)
!585 = !DILocation(line: 169, column: 39, scope: !563)
!586 = !DILocation(line: 169, column: 45, scope: !563)
!587 = !DILocation(line: 169, column: 8, scope: !563)
!588 = !DILocation(line: 169, column: 6, scope: !563)
!589 = !DILocation(line: 170, column: 26, scope: !563)
!590 = !DILocation(line: 170, column: 30, scope: !563)
!591 = !DILocation(line: 170, column: 36, scope: !563)
!592 = !DILocation(line: 402, column: 64, scope: !463, inlinedAt: !593)
!593 = distinct !DILocation(line: 170, column: 2, scope: !563)
!594 = !DILocation(line: 402, column: 84, scope: !463, inlinedAt: !593)
!595 = !DILocation(line: 404, column: 2, scope: !471, inlinedAt: !593)
!596 = !DILocation(line: 404, column: 2, scope: !472, inlinedAt: !593)
!597 = !DILocation(line: 171, column: 2, scope: !563)
!598 = !DILocation(line: 172, column: 9, scope: !563)
!599 = !DILocation(line: 172, column: 2, scope: !563)
!600 = distinct !DISubprogram(name: "notifier_chain_unregister", scope: !56, file: !56, line: 40, type: !479, scopeLine: 42, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !298)
!601 = !DILocalVariable(name: "nl", arg: 1, scope: !600, file: !56, line: 40, type: !358)
!602 = !DILocation(line: 40, column: 62, scope: !600)
!603 = !DILocalVariable(name: "n", arg: 2, scope: !600, file: !56, line: 41, type: !33)
!604 = !DILocation(line: 41, column: 26, scope: !600)
!605 = !DILocation(line: 43, column: 2, scope: !600)
!606 = !DILocation(line: 43, column: 11, scope: !600)
!607 = !DILocation(line: 43, column: 10, scope: !600)
!608 = !DILocation(line: 43, column: 15, scope: !600)
!609 = !DILocation(line: 44, column: 9, scope: !610)
!610 = distinct !DILexicalBlock(scope: !611, file: !56, line: 44, column: 7)
!611 = distinct !DILexicalBlock(scope: !600, file: !56, line: 43, column: 24)
!612 = !DILocation(line: 44, column: 8, scope: !610)
!613 = !DILocation(line: 44, column: 16, scope: !610)
!614 = !DILocation(line: 44, column: 13, scope: !610)
!615 = !DILocation(line: 44, column: 7, scope: !611)
!616 = !DILocation(line: 45, column: 4, scope: !617)
!617 = distinct !DILexicalBlock(scope: !610, file: !56, line: 44, column: 19)
!618 = !DILocalVariable(name: "_r_a_p__v", scope: !619, file: !56, line: 45, type: !50)
!619 = distinct !DILexicalBlock(scope: !617, file: !56, line: 45, column: 4)
!620 = !DILocation(line: 45, column: 4, scope: !619)
!621 = !DILocation(line: 45, column: 4, scope: !622)
!622 = distinct !DILexicalBlock(scope: !619, file: !56, line: 45, column: 4)
!623 = !DILocation(line: 45, column: 4, scope: !624)
!624 = distinct !DILexicalBlock(scope: !622, file: !56, line: 45, column: 4)
!625 = !DILocation(line: 45, column: 4, scope: !626)
!626 = distinct !DILexicalBlock(scope: !624, file: !56, line: 45, column: 4)
!627 = !DILocation(line: 45, column: 4, scope: !628)
!628 = distinct !DILexicalBlock(scope: !624, file: !56, line: 45, column: 4)
!629 = !DILocation(line: 45, column: 4, scope: !630)
!630 = distinct !DILexicalBlock(scope: !622, file: !56, line: 45, column: 4)
!631 = !DILocation(line: 45, column: 4, scope: !632)
!632 = distinct !DILexicalBlock(scope: !630, file: !56, line: 45, column: 4)
!633 = !{i64 2152928031}
!634 = !DILocation(line: 45, column: 4, scope: !635)
!635 = distinct !DILexicalBlock(scope: !630, file: !56, line: 45, column: 4)
!636 = !DILocation(line: 45, column: 4, scope: !637)
!637 = distinct !DILexicalBlock(scope: !635, file: !56, line: 45, column: 4)
!638 = !DILocation(line: 45, column: 4, scope: !639)
!639 = distinct !DILexicalBlock(scope: !635, file: !56, line: 45, column: 4)
!640 = !DILocation(line: 46, column: 4, scope: !617)
!641 = !DILocation(line: 48, column: 12, scope: !611)
!642 = !DILocation(line: 48, column: 11, scope: !611)
!643 = !DILocation(line: 48, column: 17, scope: !611)
!644 = !DILocation(line: 48, column: 6, scope: !611)
!645 = distinct !{!645, !605, !646, !420}
!646 = !DILocation(line: 49, column: 2, scope: !600)
!647 = !DILocation(line: 50, column: 2, scope: !600)
!648 = !DILocation(line: 51, column: 1, scope: !600)
!649 = distinct !DISubprogram(name: "atomic_notifier_call_chain", scope: !56, file: !56, line: 193, type: !650, scopeLine: 195, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!650 = !DISubroutineType(types: !651)
!651 = !{!42, !426, !43, !44}
!652 = !DILocalVariable(name: "nh", arg: 1, scope: !649, file: !56, line: 193, type: !426)
!653 = !DILocation(line: 193, column: 61, scope: !649)
!654 = !DILocalVariable(name: "val", arg: 2, scope: !649, file: !56, line: 194, type: !43)
!655 = !DILocation(line: 194, column: 25, scope: !649)
!656 = !DILocalVariable(name: "v", arg: 3, scope: !649, file: !56, line: 194, type: !44)
!657 = !DILocation(line: 194, column: 36, scope: !649)
!658 = !DILocalVariable(name: "ret", scope: !649, file: !56, line: 196, type: !42)
!659 = !DILocation(line: 196, column: 6, scope: !649)
!660 = !DILocation(line: 686, column: 2, scope: !661, inlinedAt: !665)
!661 = distinct !DISubprogram(name: "rcu_read_lock", scope: !662, file: !662, line: 684, type: !663, scopeLine: 685, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !298)
!662 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/rcupdate.h", directory: "", checksumkind: CSK_MD5, checksum: "080ee715fe97e9ab0d810612053be97d")
!663 = !DISubroutineType(types: !664)
!664 = !{null}
!665 = distinct !DILocation(line: 198, column: 2, scope: !649)
!666 = !DILocation(line: 199, column: 29, scope: !649)
!667 = !DILocation(line: 199, column: 33, scope: !649)
!668 = !DILocation(line: 199, column: 39, scope: !649)
!669 = !DILocation(line: 199, column: 44, scope: !649)
!670 = !DILocation(line: 199, column: 8, scope: !649)
!671 = !DILocation(line: 199, column: 6, scope: !649)
!672 = !DILocation(line: 200, column: 2, scope: !649)
!673 = !DILocation(line: 202, column: 9, scope: !649)
!674 = !DILocation(line: 202, column: 2, scope: !649)
!675 = distinct !DISubprogram(name: "rcu_read_unlock", scope: !662, file: !662, line: 715, type: !663, scopeLine: 716, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !298)
!676 = !DILocation(line: 717, column: 2, scope: !675)
!677 = !DILocation(line: 717, column: 2, scope: !678)
!678 = distinct !DILexicalBlock(scope: !675, file: !662, line: 717, column: 2)
!679 = !DILocation(line: 720, column: 2, scope: !675)
!680 = !DILocation(line: 721, column: 2, scope: !675)
!681 = !DILocation(line: 721, column: 2, scope: !682)
!682 = distinct !DILexicalBlock(scope: !675, file: !662, line: 721, column: 2)
!683 = !DILocation(line: 722, column: 1, scope: !675)
!684 = distinct !DISubprogram(name: "blocking_notifier_chain_register", scope: !56, file: !56, line: 222, type: !685, scopeLine: 224, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!685 = !DISubroutineType(types: !686)
!686 = !{!42, !687, !33}
!687 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !337, size: 64)
!688 = !DILocalVariable(name: "nh", arg: 1, scope: !684, file: !56, line: 222, type: !687)
!689 = !DILocation(line: 222, column: 69, scope: !684)
!690 = !DILocalVariable(name: "n", arg: 2, scope: !684, file: !56, line: 223, type: !33)
!691 = !DILocation(line: 223, column: 26, scope: !684)
!692 = !DILocalVariable(name: "ret", scope: !684, file: !56, line: 225, type: !42)
!693 = !DILocation(line: 225, column: 6, scope: !684)
!694 = !DILocation(line: 232, column: 6, scope: !695)
!695 = distinct !DILexicalBlock(scope: !684, file: !56, line: 232, column: 6)
!696 = !DILocation(line: 232, column: 6, scope: !684)
!697 = !DILocation(line: 233, column: 35, scope: !695)
!698 = !DILocation(line: 233, column: 39, scope: !695)
!699 = !DILocation(line: 233, column: 45, scope: !695)
!700 = !DILocation(line: 233, column: 10, scope: !695)
!701 = !DILocation(line: 233, column: 3, scope: !695)
!702 = !DILocation(line: 235, column: 14, scope: !684)
!703 = !DILocation(line: 235, column: 18, scope: !684)
!704 = !DILocation(line: 235, column: 2, scope: !684)
!705 = !DILocation(line: 236, column: 33, scope: !684)
!706 = !DILocation(line: 236, column: 37, scope: !684)
!707 = !DILocation(line: 236, column: 43, scope: !684)
!708 = !DILocation(line: 236, column: 8, scope: !684)
!709 = !DILocation(line: 236, column: 6, scope: !684)
!710 = !DILocation(line: 237, column: 12, scope: !684)
!711 = !DILocation(line: 237, column: 16, scope: !684)
!712 = !DILocation(line: 237, column: 2, scope: !684)
!713 = !DILocation(line: 238, column: 9, scope: !684)
!714 = !DILocation(line: 238, column: 2, scope: !684)
!715 = !DILocation(line: 239, column: 1, scope: !684)
!716 = distinct !DISubprogram(name: "blocking_notifier_chain_unregister", scope: !56, file: !56, line: 252, type: !685, scopeLine: 254, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!717 = !DILocalVariable(name: "nh", arg: 1, scope: !716, file: !56, line: 252, type: !687)
!718 = !DILocation(line: 252, column: 71, scope: !716)
!719 = !DILocalVariable(name: "n", arg: 2, scope: !716, file: !56, line: 253, type: !33)
!720 = !DILocation(line: 253, column: 26, scope: !716)
!721 = !DILocalVariable(name: "ret", scope: !716, file: !56, line: 255, type: !42)
!722 = !DILocation(line: 255, column: 6, scope: !716)
!723 = !DILocation(line: 262, column: 6, scope: !724)
!724 = distinct !DILexicalBlock(scope: !716, file: !56, line: 262, column: 6)
!725 = !DILocation(line: 262, column: 6, scope: !716)
!726 = !DILocation(line: 263, column: 37, scope: !724)
!727 = !DILocation(line: 263, column: 41, scope: !724)
!728 = !DILocation(line: 263, column: 47, scope: !724)
!729 = !DILocation(line: 263, column: 10, scope: !724)
!730 = !DILocation(line: 263, column: 3, scope: !724)
!731 = !DILocation(line: 265, column: 14, scope: !716)
!732 = !DILocation(line: 265, column: 18, scope: !716)
!733 = !DILocation(line: 265, column: 2, scope: !716)
!734 = !DILocation(line: 266, column: 35, scope: !716)
!735 = !DILocation(line: 266, column: 39, scope: !716)
!736 = !DILocation(line: 266, column: 45, scope: !716)
!737 = !DILocation(line: 266, column: 8, scope: !716)
!738 = !DILocation(line: 266, column: 6, scope: !716)
!739 = !DILocation(line: 267, column: 12, scope: !716)
!740 = !DILocation(line: 267, column: 16, scope: !716)
!741 = !DILocation(line: 267, column: 2, scope: !716)
!742 = !DILocation(line: 268, column: 9, scope: !716)
!743 = !DILocation(line: 268, column: 2, scope: !716)
!744 = !DILocation(line: 269, column: 1, scope: !716)
!745 = distinct !DISubprogram(name: "blocking_notifier_call_chain_robust", scope: !56, file: !56, line: 272, type: !746, scopeLine: 274, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!746 = !DISubroutineType(types: !747)
!747 = !{!42, !687, !43, !43, !44}
!748 = !DILocalVariable(name: "nh", arg: 1, scope: !745, file: !56, line: 272, type: !687)
!749 = !DILocation(line: 272, column: 72, scope: !745)
!750 = !DILocalVariable(name: "val_up", arg: 2, scope: !745, file: !56, line: 273, type: !43)
!751 = !DILocation(line: 273, column: 17, scope: !745)
!752 = !DILocalVariable(name: "val_down", arg: 3, scope: !745, file: !56, line: 273, type: !43)
!753 = !DILocation(line: 273, column: 39, scope: !745)
!754 = !DILocalVariable(name: "v", arg: 4, scope: !745, file: !56, line: 273, type: !44)
!755 = !DILocation(line: 273, column: 55, scope: !745)
!756 = !DILocalVariable(name: "ret", scope: !745, file: !56, line: 275, type: !42)
!757 = !DILocation(line: 275, column: 6, scope: !745)
!758 = !DILocalVariable(name: "_________p1", scope: !759, file: !56, line: 282, type: !33)
!759 = distinct !DILexicalBlock(scope: !760, file: !56, line: 282, column: 6)
!760 = distinct !DILexicalBlock(scope: !745, file: !56, line: 282, column: 6)
!761 = !DILocation(line: 282, column: 6, scope: !759)
!762 = !DILocation(line: 282, column: 6, scope: !763)
!763 = distinct !DILexicalBlock(scope: !759, file: !56, line: 282, column: 6)
!764 = !DILocation(line: 282, column: 6, scope: !765)
!765 = distinct !DILexicalBlock(scope: !763, file: !56, line: 282, column: 6)
!766 = !DILocation(line: 282, column: 6, scope: !760)
!767 = !DILocation(line: 282, column: 6, scope: !745)
!768 = !DILocation(line: 283, column: 14, scope: !769)
!769 = distinct !DILexicalBlock(scope: !760, file: !56, line: 282, column: 36)
!770 = !DILocation(line: 283, column: 18, scope: !769)
!771 = !DILocation(line: 283, column: 3, scope: !769)
!772 = !DILocation(line: 284, column: 37, scope: !769)
!773 = !DILocation(line: 284, column: 41, scope: !769)
!774 = !DILocation(line: 284, column: 47, scope: !769)
!775 = !DILocation(line: 284, column: 55, scope: !769)
!776 = !DILocation(line: 284, column: 65, scope: !769)
!777 = !DILocation(line: 284, column: 9, scope: !769)
!778 = !DILocation(line: 284, column: 7, scope: !769)
!779 = !DILocation(line: 285, column: 12, scope: !769)
!780 = !DILocation(line: 285, column: 16, scope: !769)
!781 = !DILocation(line: 285, column: 3, scope: !769)
!782 = !DILocation(line: 286, column: 2, scope: !769)
!783 = !DILocation(line: 287, column: 9, scope: !745)
!784 = !DILocation(line: 287, column: 2, scope: !745)
!785 = distinct !DISubprogram(name: "notifier_call_chain_robust", scope: !56, file: !56, line: 113, type: !786, scopeLine: 116, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !298)
!786 = !DISubroutineType(types: !787)
!787 = !{!42, !358, !43, !43, !44}
!788 = !DILocalVariable(name: "nl", arg: 1, scope: !785, file: !56, line: 113, type: !358)
!789 = !DILocation(line: 113, column: 63, scope: !785)
!790 = !DILocalVariable(name: "val_up", arg: 2, scope: !785, file: !56, line: 114, type: !43)
!791 = !DILocation(line: 114, column: 24, scope: !785)
!792 = !DILocalVariable(name: "val_down", arg: 3, scope: !785, file: !56, line: 114, type: !43)
!793 = !DILocation(line: 114, column: 46, scope: !785)
!794 = !DILocalVariable(name: "v", arg: 4, scope: !785, file: !56, line: 115, type: !44)
!795 = !DILocation(line: 115, column: 16, scope: !785)
!796 = !DILocalVariable(name: "ret", scope: !785, file: !56, line: 117, type: !42)
!797 = !DILocation(line: 117, column: 6, scope: !785)
!798 = !DILocalVariable(name: "nr", scope: !785, file: !56, line: 117, type: !42)
!799 = !DILocation(line: 117, column: 11, scope: !785)
!800 = !DILocation(line: 119, column: 28, scope: !785)
!801 = !DILocation(line: 119, column: 32, scope: !785)
!802 = !DILocation(line: 119, column: 40, scope: !785)
!803 = !DILocation(line: 119, column: 8, scope: !785)
!804 = !DILocation(line: 119, column: 6, scope: !785)
!805 = !DILocation(line: 120, column: 6, scope: !806)
!806 = distinct !DILexicalBlock(scope: !785, file: !56, line: 120, column: 6)
!807 = !DILocation(line: 120, column: 10, scope: !806)
!808 = !DILocation(line: 120, column: 6, scope: !785)
!809 = !DILocation(line: 121, column: 23, scope: !806)
!810 = !DILocation(line: 121, column: 27, scope: !806)
!811 = !DILocation(line: 121, column: 37, scope: !806)
!812 = !DILocation(line: 121, column: 40, scope: !806)
!813 = !DILocation(line: 121, column: 42, scope: !806)
!814 = !DILocation(line: 121, column: 3, scope: !806)
!815 = !DILocation(line: 123, column: 9, scope: !785)
!816 = !DILocation(line: 123, column: 2, scope: !785)
!817 = distinct !DISubprogram(name: "blocking_notifier_call_chain", scope: !56, file: !56, line: 307, type: !818, scopeLine: 309, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!818 = !DISubroutineType(types: !819)
!819 = !{!42, !687, !43, !44}
!820 = !DILocalVariable(name: "nh", arg: 1, scope: !817, file: !56, line: 307, type: !687)
!821 = !DILocation(line: 307, column: 65, scope: !817)
!822 = !DILocalVariable(name: "val", arg: 2, scope: !817, file: !56, line: 308, type: !43)
!823 = !DILocation(line: 308, column: 17, scope: !817)
!824 = !DILocalVariable(name: "v", arg: 3, scope: !817, file: !56, line: 308, type: !44)
!825 = !DILocation(line: 308, column: 28, scope: !817)
!826 = !DILocalVariable(name: "ret", scope: !817, file: !56, line: 310, type: !42)
!827 = !DILocation(line: 310, column: 6, scope: !817)
!828 = !DILocalVariable(name: "_________p1", scope: !829, file: !56, line: 317, type: !33)
!829 = distinct !DILexicalBlock(scope: !830, file: !56, line: 317, column: 6)
!830 = distinct !DILexicalBlock(scope: !817, file: !56, line: 317, column: 6)
!831 = !DILocation(line: 317, column: 6, scope: !829)
!832 = !DILocation(line: 317, column: 6, scope: !833)
!833 = distinct !DILexicalBlock(scope: !829, file: !56, line: 317, column: 6)
!834 = !DILocation(line: 317, column: 6, scope: !835)
!835 = distinct !DILexicalBlock(scope: !833, file: !56, line: 317, column: 6)
!836 = !DILocation(line: 317, column: 6, scope: !830)
!837 = !DILocation(line: 317, column: 6, scope: !817)
!838 = !DILocation(line: 318, column: 14, scope: !839)
!839 = distinct !DILexicalBlock(scope: !830, file: !56, line: 317, column: 36)
!840 = !DILocation(line: 318, column: 18, scope: !839)
!841 = !DILocation(line: 318, column: 3, scope: !839)
!842 = !DILocation(line: 319, column: 30, scope: !839)
!843 = !DILocation(line: 319, column: 34, scope: !839)
!844 = !DILocation(line: 319, column: 40, scope: !839)
!845 = !DILocation(line: 319, column: 45, scope: !839)
!846 = !DILocation(line: 319, column: 9, scope: !839)
!847 = !DILocation(line: 319, column: 7, scope: !839)
!848 = !DILocation(line: 320, column: 12, scope: !839)
!849 = !DILocation(line: 320, column: 16, scope: !839)
!850 = !DILocation(line: 320, column: 3, scope: !839)
!851 = !DILocation(line: 321, column: 2, scope: !839)
!852 = !DILocation(line: 322, column: 9, scope: !817)
!853 = !DILocation(line: 322, column: 2, scope: !817)
!854 = distinct !DISubprogram(name: "raw_notifier_chain_register", scope: !56, file: !56, line: 341, type: !855, scopeLine: 343, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!855 = !DISubroutineType(types: !856)
!856 = !{!42, !857, !33}
!857 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !858, size: 64)
!858 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "raw_notifier_head", file: !35, line: 70, size: 64, elements: !859)
!859 = !{!860}
!860 = !DIDerivedType(tag: DW_TAG_member, name: "head", scope: !858, file: !35, line: 71, baseType: !33, size: 64)
!861 = !DILocalVariable(name: "nh", arg: 1, scope: !854, file: !56, line: 341, type: !857)
!862 = !DILocation(line: 341, column: 59, scope: !854)
!863 = !DILocalVariable(name: "n", arg: 2, scope: !854, file: !56, line: 342, type: !33)
!864 = !DILocation(line: 342, column: 26, scope: !854)
!865 = !DILocation(line: 344, column: 34, scope: !854)
!866 = !DILocation(line: 344, column: 38, scope: !854)
!867 = !DILocation(line: 344, column: 44, scope: !854)
!868 = !DILocation(line: 344, column: 9, scope: !854)
!869 = !DILocation(line: 344, column: 2, scope: !854)
!870 = distinct !DISubprogram(name: "raw_notifier_chain_unregister", scope: !56, file: !56, line: 358, type: !855, scopeLine: 360, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!871 = !DILocalVariable(name: "nh", arg: 1, scope: !870, file: !56, line: 358, type: !857)
!872 = !DILocation(line: 358, column: 61, scope: !870)
!873 = !DILocalVariable(name: "n", arg: 2, scope: !870, file: !56, line: 359, type: !33)
!874 = !DILocation(line: 359, column: 26, scope: !870)
!875 = !DILocation(line: 361, column: 36, scope: !870)
!876 = !DILocation(line: 361, column: 40, scope: !870)
!877 = !DILocation(line: 361, column: 46, scope: !870)
!878 = !DILocation(line: 361, column: 9, scope: !870)
!879 = !DILocation(line: 361, column: 2, scope: !870)
!880 = distinct !DISubprogram(name: "raw_notifier_call_chain_robust", scope: !56, file: !56, line: 365, type: !881, scopeLine: 367, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!881 = !DISubroutineType(types: !882)
!882 = !{!42, !857, !43, !43, !44}
!883 = !DILocalVariable(name: "nh", arg: 1, scope: !880, file: !56, line: 365, type: !857)
!884 = !DILocation(line: 365, column: 62, scope: !880)
!885 = !DILocalVariable(name: "val_up", arg: 2, scope: !880, file: !56, line: 366, type: !43)
!886 = !DILocation(line: 366, column: 17, scope: !880)
!887 = !DILocalVariable(name: "val_down", arg: 3, scope: !880, file: !56, line: 366, type: !43)
!888 = !DILocation(line: 366, column: 39, scope: !880)
!889 = !DILocalVariable(name: "v", arg: 4, scope: !880, file: !56, line: 366, type: !44)
!890 = !DILocation(line: 366, column: 55, scope: !880)
!891 = !DILocation(line: 368, column: 37, scope: !880)
!892 = !DILocation(line: 368, column: 41, scope: !880)
!893 = !DILocation(line: 368, column: 47, scope: !880)
!894 = !DILocation(line: 368, column: 55, scope: !880)
!895 = !DILocation(line: 368, column: 65, scope: !880)
!896 = !DILocation(line: 368, column: 9, scope: !880)
!897 = !DILocation(line: 368, column: 2, scope: !880)
!898 = distinct !DISubprogram(name: "raw_notifier_call_chain", scope: !56, file: !56, line: 389, type: !899, scopeLine: 391, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!899 = !DISubroutineType(types: !900)
!900 = !{!42, !857, !43, !44}
!901 = !DILocalVariable(name: "nh", arg: 1, scope: !898, file: !56, line: 389, type: !857)
!902 = !DILocation(line: 389, column: 55, scope: !898)
!903 = !DILocalVariable(name: "val", arg: 2, scope: !898, file: !56, line: 390, type: !43)
!904 = !DILocation(line: 390, column: 17, scope: !898)
!905 = !DILocalVariable(name: "v", arg: 3, scope: !898, file: !56, line: 390, type: !44)
!906 = !DILocation(line: 390, column: 28, scope: !898)
!907 = !DILocation(line: 392, column: 30, scope: !898)
!908 = !DILocation(line: 392, column: 34, scope: !898)
!909 = !DILocation(line: 392, column: 40, scope: !898)
!910 = !DILocation(line: 392, column: 45, scope: !898)
!911 = !DILocation(line: 392, column: 9, scope: !898)
!912 = !DILocation(line: 392, column: 2, scope: !898)
!913 = distinct !DISubprogram(name: "srcu_notifier_chain_register", scope: !56, file: !56, line: 412, type: !914, scopeLine: 414, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!914 = !DISubroutineType(types: !915)
!915 = !{!42, !92, !33}
!916 = !DILocalVariable(name: "nh", arg: 1, scope: !913, file: !56, line: 412, type: !92)
!917 = !DILocation(line: 412, column: 61, scope: !913)
!918 = !DILocalVariable(name: "n", arg: 2, scope: !913, file: !56, line: 413, type: !33)
!919 = !DILocation(line: 413, column: 26, scope: !913)
!920 = !DILocalVariable(name: "ret", scope: !913, file: !56, line: 415, type: !42)
!921 = !DILocation(line: 415, column: 6, scope: !913)
!922 = !DILocation(line: 422, column: 6, scope: !923)
!923 = distinct !DILexicalBlock(scope: !913, file: !56, line: 422, column: 6)
!924 = !DILocation(line: 422, column: 6, scope: !913)
!925 = !DILocation(line: 423, column: 35, scope: !923)
!926 = !DILocation(line: 423, column: 39, scope: !923)
!927 = !DILocation(line: 423, column: 45, scope: !923)
!928 = !DILocation(line: 423, column: 10, scope: !923)
!929 = !DILocation(line: 423, column: 3, scope: !923)
!930 = !DILocation(line: 425, column: 14, scope: !913)
!931 = !DILocation(line: 425, column: 18, scope: !913)
!932 = !DILocation(line: 425, column: 2, scope: !913)
!933 = !DILocation(line: 426, column: 33, scope: !913)
!934 = !DILocation(line: 426, column: 37, scope: !913)
!935 = !DILocation(line: 426, column: 43, scope: !913)
!936 = !DILocation(line: 426, column: 8, scope: !913)
!937 = !DILocation(line: 426, column: 6, scope: !913)
!938 = !DILocation(line: 427, column: 16, scope: !913)
!939 = !DILocation(line: 427, column: 20, scope: !913)
!940 = !DILocation(line: 427, column: 2, scope: !913)
!941 = !DILocation(line: 428, column: 9, scope: !913)
!942 = !DILocation(line: 428, column: 2, scope: !913)
!943 = !DILocation(line: 429, column: 1, scope: !913)
!944 = distinct !DISubprogram(name: "srcu_notifier_chain_unregister", scope: !56, file: !56, line: 442, type: !914, scopeLine: 444, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!945 = !DILocalVariable(name: "nh", arg: 1, scope: !944, file: !56, line: 442, type: !92)
!946 = !DILocation(line: 442, column: 63, scope: !944)
!947 = !DILocalVariable(name: "n", arg: 2, scope: !944, file: !56, line: 443, type: !33)
!948 = !DILocation(line: 443, column: 26, scope: !944)
!949 = !DILocalVariable(name: "ret", scope: !944, file: !56, line: 445, type: !42)
!950 = !DILocation(line: 445, column: 6, scope: !944)
!951 = !DILocation(line: 452, column: 6, scope: !952)
!952 = distinct !DILexicalBlock(scope: !944, file: !56, line: 452, column: 6)
!953 = !DILocation(line: 452, column: 6, scope: !944)
!954 = !DILocation(line: 453, column: 37, scope: !952)
!955 = !DILocation(line: 453, column: 41, scope: !952)
!956 = !DILocation(line: 453, column: 47, scope: !952)
!957 = !DILocation(line: 453, column: 10, scope: !952)
!958 = !DILocation(line: 453, column: 3, scope: !952)
!959 = !DILocation(line: 455, column: 14, scope: !944)
!960 = !DILocation(line: 455, column: 18, scope: !944)
!961 = !DILocation(line: 455, column: 2, scope: !944)
!962 = !DILocation(line: 456, column: 35, scope: !944)
!963 = !DILocation(line: 456, column: 39, scope: !944)
!964 = !DILocation(line: 456, column: 45, scope: !944)
!965 = !DILocation(line: 456, column: 8, scope: !944)
!966 = !DILocation(line: 456, column: 6, scope: !944)
!967 = !DILocation(line: 457, column: 16, scope: !944)
!968 = !DILocation(line: 457, column: 20, scope: !944)
!969 = !DILocation(line: 457, column: 2, scope: !944)
!970 = !DILocation(line: 458, column: 20, scope: !944)
!971 = !DILocation(line: 458, column: 24, scope: !944)
!972 = !DILocation(line: 458, column: 2, scope: !944)
!973 = !DILocation(line: 459, column: 9, scope: !944)
!974 = !DILocation(line: 459, column: 2, scope: !944)
!975 = !DILocation(line: 460, column: 1, scope: !944)
!976 = distinct !DISubprogram(name: "srcu_notifier_call_chain", scope: !56, file: !56, line: 479, type: !977, scopeLine: 481, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!977 = !DISubroutineType(types: !978)
!978 = !{!42, !92, !43, !44}
!979 = !DILocalVariable(name: "nh", arg: 1, scope: !976, file: !56, line: 479, type: !92)
!980 = !DILocation(line: 479, column: 57, scope: !976)
!981 = !DILocalVariable(name: "val", arg: 2, scope: !976, file: !56, line: 480, type: !43)
!982 = !DILocation(line: 480, column: 17, scope: !976)
!983 = !DILocalVariable(name: "v", arg: 3, scope: !976, file: !56, line: 480, type: !44)
!984 = !DILocation(line: 480, column: 28, scope: !976)
!985 = !DILocalVariable(name: "ret", scope: !976, file: !56, line: 482, type: !42)
!986 = !DILocation(line: 482, column: 6, scope: !976)
!987 = !DILocalVariable(name: "idx", scope: !976, file: !56, line: 483, type: !42)
!988 = !DILocation(line: 483, column: 6, scope: !976)
!989 = !DILocation(line: 485, column: 24, scope: !976)
!990 = !DILocation(line: 485, column: 28, scope: !976)
!991 = !DILocation(line: 485, column: 8, scope: !976)
!992 = !DILocation(line: 485, column: 6, scope: !976)
!993 = !DILocation(line: 486, column: 29, scope: !976)
!994 = !DILocation(line: 486, column: 33, scope: !976)
!995 = !DILocation(line: 486, column: 39, scope: !976)
!996 = !DILocation(line: 486, column: 44, scope: !976)
!997 = !DILocation(line: 486, column: 8, scope: !976)
!998 = !DILocation(line: 486, column: 6, scope: !976)
!999 = !DILocation(line: 487, column: 20, scope: !976)
!1000 = !DILocation(line: 487, column: 24, scope: !976)
!1001 = !DILocation(line: 487, column: 30, scope: !976)
!1002 = !DILocation(line: 487, column: 2, scope: !976)
!1003 = !DILocation(line: 488, column: 9, scope: !976)
!1004 = !DILocation(line: 488, column: 2, scope: !976)
!1005 = distinct !DISubprogram(name: "srcu_read_lock", scope: !327, file: !327, line: 159, type: !1006, scopeLine: 160, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !298)
!1006 = !DISubroutineType(types: !1007)
!1007 = !{!42, !271}
!1008 = !DILocalVariable(name: "ssp", arg: 1, scope: !1005, file: !327, line: 159, type: !271)
!1009 = !DILocation(line: 159, column: 54, scope: !1005)
!1010 = !DILocalVariable(name: "retval", scope: !1005, file: !327, line: 161, type: !42)
!1011 = !DILocation(line: 161, column: 6, scope: !1005)
!1012 = !DILocation(line: 163, column: 28, scope: !1005)
!1013 = !DILocation(line: 163, column: 11, scope: !1005)
!1014 = !DILocation(line: 163, column: 9, scope: !1005)
!1015 = !DILocation(line: 164, column: 2, scope: !1005)
!1016 = !DILocation(line: 164, column: 2, scope: !1017)
!1017 = distinct !DILexicalBlock(scope: !1005, file: !327, line: 164, column: 2)
!1018 = !DILocation(line: 165, column: 9, scope: !1005)
!1019 = !DILocation(line: 165, column: 2, scope: !1005)
!1020 = distinct !DISubprogram(name: "srcu_read_unlock", scope: !327, file: !327, line: 185, type: !1021, scopeLine: 187, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, retainedNodes: !298)
!1021 = !DISubroutineType(types: !1022)
!1022 = !{null, !271, !42}
!1023 = !DILocalVariable(name: "ssp", arg: 1, scope: !1020, file: !327, line: 185, type: !271)
!1024 = !DILocation(line: 185, column: 57, scope: !1020)
!1025 = !DILocalVariable(name: "idx", arg: 2, scope: !1020, file: !327, line: 185, type: !42)
!1026 = !DILocation(line: 185, column: 66, scope: !1020)
!1027 = !DILocalVariable(name: "__ret_warn_on", scope: !1028, file: !327, line: 188, type: !42)
!1028 = distinct !DILexicalBlock(scope: !1020, file: !327, line: 188, column: 2)
!1029 = !DILocation(line: 188, column: 2, scope: !1028)
!1030 = !DILocation(line: 188, column: 2, scope: !1031)
!1031 = distinct !DILexicalBlock(scope: !1028, file: !327, line: 188, column: 2)
!1032 = !DILocation(line: 188, column: 2, scope: !1033)
!1033 = distinct !DILexicalBlock(scope: !1031, file: !327, line: 188, column: 2)
!1034 = !DILocation(line: 188, column: 2, scope: !1035)
!1035 = distinct !DILexicalBlock(scope: !1033, file: !327, line: 188, column: 2)
!1036 = !DILocation(line: 188, column: 2, scope: !1037)
!1037 = distinct !DILexicalBlock(scope: !1033, file: !327, line: 188, column: 2)
!1038 = !{i64 2149600990, i64 2149601019, i64 2149601065, i64 2149601123, i64 2149601177, i64 2149601231, i64 2149601286, i64 2149601317}
!1039 = !DILocation(line: 188, column: 2, scope: !1040)
!1040 = distinct !DILexicalBlock(scope: !1033, file: !327, line: 188, column: 2)
!1041 = !{i64 2149602009, i64 2149601840, i64 2149601889, i64 2149601941, i64 2149601969}
!1042 = !DILocation(line: 188, column: 2, scope: !1043)
!1043 = distinct !DILexicalBlock(scope: !1033, file: !327, line: 188, column: 2)
!1044 = !DILocation(line: 189, column: 2, scope: !1020)
!1045 = !DILocation(line: 189, column: 2, scope: !1046)
!1046 = distinct !DILexicalBlock(scope: !1020, file: !327, line: 189, column: 2)
!1047 = !DILocation(line: 190, column: 21, scope: !1020)
!1048 = !DILocation(line: 190, column: 26, scope: !1020)
!1049 = !DILocation(line: 190, column: 2, scope: !1020)
!1050 = !DILocation(line: 191, column: 1, scope: !1020)
!1051 = !DILocalVariable(name: "nh", arg: 1, scope: !89, file: !56, line: 504, type: !92)
!1052 = !DILocation(line: 504, column: 57, scope: !89)
!1053 = !DILocation(line: 506, column: 2, scope: !89)
!1054 = !DILocation(line: 506, column: 2, scope: !1055)
!1055 = distinct !DILexicalBlock(scope: !89, file: !56, line: 506, column: 2)
!1056 = !DILocation(line: 507, column: 24, scope: !1057)
!1057 = distinct !DILexicalBlock(scope: !89, file: !56, line: 507, column: 6)
!1058 = !DILocation(line: 507, column: 28, scope: !1057)
!1059 = !DILocation(line: 507, column: 6, scope: !1057)
!1060 = !DILocation(line: 507, column: 34, scope: !1057)
!1061 = !DILocation(line: 507, column: 6, scope: !89)
!1062 = !DILocation(line: 508, column: 3, scope: !1057)
!1063 = !DILocation(line: 508, column: 3, scope: !1064)
!1064 = distinct !DILexicalBlock(scope: !1057, file: !56, line: 508, column: 3)
!1065 = !DILocation(line: 508, column: 3, scope: !1066)
!1066 = distinct !DILexicalBlock(scope: !1064, file: !56, line: 508, column: 3)
!1067 = !DILocation(line: 508, column: 3, scope: !1068)
!1068 = distinct !DILexicalBlock(scope: !1064, file: !56, line: 508, column: 3)
!1069 = !{i64 2152981580, i64 2152981609, i64 2152981655, i64 2152981713, i64 2152981767, i64 2152981821, i64 2152981876, i64 2152981907}
!1070 = !DILocation(line: 508, column: 3, scope: !1071)
!1071 = distinct !DILexicalBlock(scope: !1072, file: !56, line: 508, column: 3)
!1072 = distinct !DILexicalBlock(scope: !1064, file: !56, line: 508, column: 3)
!1073 = !{i64 2152982635, i64 2152982460, i64 2152982511, i64 2152982563, i64 2152982591}
!1074 = !DILocation(line: 508, column: 3, scope: !1072)
!1075 = !DILocation(line: 509, column: 2, scope: !89)
!1076 = !DILocation(line: 509, column: 6, scope: !89)
!1077 = !DILocation(line: 509, column: 11, scope: !89)
!1078 = !DILocation(line: 510, column: 1, scope: !89)
!1079 = distinct !DISubprogram(name: "notify_die", scope: !56, file: !56, line: 517, type: !1080, scopeLine: 519, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!1080 = !DISubroutineType(types: !1081)
!1081 = !{!42, !17, !1082, !1084, !225, !42, !42}
!1082 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1083, size: 64)
!1083 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !304)
!1084 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1085, size: 64)
!1085 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "pt_regs", file: !1086, line: 59, size: 1344, elements: !1087)
!1086 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/arch/x86/include/asm/ptrace.h", directory: "", checksumkind: CSK_MD5, checksum: "09bbf2aa7e49d26370839dc846b3287f")
!1087 = !{!1088, !1089, !1090, !1091, !1092, !1093, !1094, !1095, !1096, !1097, !1098, !1099, !1100, !1101, !1102, !1103, !1104, !1105, !1106, !1107, !1108}
!1088 = !DIDerivedType(tag: DW_TAG_member, name: "r15", scope: !1085, file: !1086, line: 64, baseType: !43, size: 64)
!1089 = !DIDerivedType(tag: DW_TAG_member, name: "r14", scope: !1085, file: !1086, line: 65, baseType: !43, size: 64, offset: 64)
!1090 = !DIDerivedType(tag: DW_TAG_member, name: "r13", scope: !1085, file: !1086, line: 66, baseType: !43, size: 64, offset: 128)
!1091 = !DIDerivedType(tag: DW_TAG_member, name: "r12", scope: !1085, file: !1086, line: 67, baseType: !43, size: 64, offset: 192)
!1092 = !DIDerivedType(tag: DW_TAG_member, name: "bp", scope: !1085, file: !1086, line: 68, baseType: !43, size: 64, offset: 256)
!1093 = !DIDerivedType(tag: DW_TAG_member, name: "bx", scope: !1085, file: !1086, line: 69, baseType: !43, size: 64, offset: 320)
!1094 = !DIDerivedType(tag: DW_TAG_member, name: "r11", scope: !1085, file: !1086, line: 71, baseType: !43, size: 64, offset: 384)
!1095 = !DIDerivedType(tag: DW_TAG_member, name: "r10", scope: !1085, file: !1086, line: 72, baseType: !43, size: 64, offset: 448)
!1096 = !DIDerivedType(tag: DW_TAG_member, name: "r9", scope: !1085, file: !1086, line: 73, baseType: !43, size: 64, offset: 512)
!1097 = !DIDerivedType(tag: DW_TAG_member, name: "r8", scope: !1085, file: !1086, line: 74, baseType: !43, size: 64, offset: 576)
!1098 = !DIDerivedType(tag: DW_TAG_member, name: "ax", scope: !1085, file: !1086, line: 75, baseType: !43, size: 64, offset: 640)
!1099 = !DIDerivedType(tag: DW_TAG_member, name: "cx", scope: !1085, file: !1086, line: 76, baseType: !43, size: 64, offset: 704)
!1100 = !DIDerivedType(tag: DW_TAG_member, name: "dx", scope: !1085, file: !1086, line: 77, baseType: !43, size: 64, offset: 768)
!1101 = !DIDerivedType(tag: DW_TAG_member, name: "si", scope: !1085, file: !1086, line: 78, baseType: !43, size: 64, offset: 832)
!1102 = !DIDerivedType(tag: DW_TAG_member, name: "di", scope: !1085, file: !1086, line: 79, baseType: !43, size: 64, offset: 896)
!1103 = !DIDerivedType(tag: DW_TAG_member, name: "orig_ax", scope: !1085, file: !1086, line: 84, baseType: !43, size: 64, offset: 960)
!1104 = !DIDerivedType(tag: DW_TAG_member, name: "ip", scope: !1085, file: !1086, line: 86, baseType: !43, size: 64, offset: 1024)
!1105 = !DIDerivedType(tag: DW_TAG_member, name: "cs", scope: !1085, file: !1086, line: 87, baseType: !43, size: 64, offset: 1088)
!1106 = !DIDerivedType(tag: DW_TAG_member, name: "flags", scope: !1085, file: !1086, line: 88, baseType: !43, size: 64, offset: 1152)
!1107 = !DIDerivedType(tag: DW_TAG_member, name: "sp", scope: !1085, file: !1086, line: 89, baseType: !43, size: 64, offset: 1216)
!1108 = !DIDerivedType(tag: DW_TAG_member, name: "ss", scope: !1085, file: !1086, line: 90, baseType: !43, size: 64, offset: 1280)
!1109 = !DILocalVariable(name: "val", arg: 1, scope: !1079, file: !56, line: 517, type: !17)
!1110 = !DILocation(line: 517, column: 37, scope: !1079)
!1111 = !DILocalVariable(name: "str", arg: 2, scope: !1079, file: !56, line: 517, type: !1082)
!1112 = !DILocation(line: 517, column: 54, scope: !1079)
!1113 = !DILocalVariable(name: "regs", arg: 3, scope: !1079, file: !56, line: 518, type: !1084)
!1114 = !DILocation(line: 518, column: 25, scope: !1079)
!1115 = !DILocalVariable(name: "err", arg: 4, scope: !1079, file: !56, line: 518, type: !225)
!1116 = !DILocation(line: 518, column: 36, scope: !1079)
!1117 = !DILocalVariable(name: "trap", arg: 5, scope: !1079, file: !56, line: 518, type: !42)
!1118 = !DILocation(line: 518, column: 45, scope: !1079)
!1119 = !DILocalVariable(name: "sig", arg: 6, scope: !1079, file: !56, line: 518, type: !42)
!1120 = !DILocation(line: 518, column: 55, scope: !1079)
!1121 = !DILocalVariable(name: "args", scope: !1079, file: !56, line: 520, type: !1122)
!1122 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "die_args", file: !1123, line: 9, size: 256, elements: !1124)
!1123 = !DIFile(filename: "/tmp/syzbot_repair_ir_1283433/0a6e6b3c7db6/include/linux/kdebug.h", directory: "", checksumkind: CSK_MD5, checksum: "540f38f52f30c3c7c1cb3ec423fc360f")
!1124 = !{!1125, !1126, !1127, !1128, !1129}
!1125 = !DIDerivedType(tag: DW_TAG_member, name: "regs", scope: !1122, file: !1123, line: 10, baseType: !1084, size: 64)
!1126 = !DIDerivedType(tag: DW_TAG_member, name: "str", scope: !1122, file: !1123, line: 11, baseType: !1082, size: 64, offset: 64)
!1127 = !DIDerivedType(tag: DW_TAG_member, name: "err", scope: !1122, file: !1123, line: 12, baseType: !225, size: 64, offset: 128)
!1128 = !DIDerivedType(tag: DW_TAG_member, name: "trapnr", scope: !1122, file: !1123, line: 13, baseType: !42, size: 32, offset: 192)
!1129 = !DIDerivedType(tag: DW_TAG_member, name: "signr", scope: !1122, file: !1123, line: 14, baseType: !42, size: 32, offset: 224)
!1130 = !DILocation(line: 520, column: 18, scope: !1079)
!1131 = !DILocation(line: 520, column: 25, scope: !1079)
!1132 = !DILocation(line: 521, column: 11, scope: !1079)
!1133 = !DILocation(line: 522, column: 10, scope: !1079)
!1134 = !DILocation(line: 523, column: 10, scope: !1079)
!1135 = !DILocation(line: 524, column: 13, scope: !1079)
!1136 = !DILocation(line: 525, column: 12, scope: !1079)
!1137 = !DILocation(line: 528, column: 2, scope: !1079)
!1138 = !DILocation(line: 528, column: 2, scope: !1139)
!1139 = distinct !DILexicalBlock(scope: !1079, file: !56, line: 528, column: 2)
!1140 = !DILocation(line: 530, column: 48, scope: !1079)
!1141 = !DILocation(line: 530, column: 9, scope: !1079)
!1142 = !DILocation(line: 530, column: 2, scope: !1079)
!1143 = distinct !DISubprogram(name: "register_die_notifier", scope: !56, file: !56, line: 534, type: !1144, scopeLine: 535, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!1144 = !DISubroutineType(types: !1145)
!1145 = !{!42, !33}
!1146 = !DILocalVariable(name: "nb", arg: 1, scope: !1143, file: !56, line: 534, type: !33)
!1147 = !DILocation(line: 534, column: 50, scope: !1143)
!1148 = !DILocation(line: 536, column: 52, scope: !1143)
!1149 = !DILocation(line: 536, column: 9, scope: !1143)
!1150 = !DILocation(line: 536, column: 2, scope: !1143)
!1151 = distinct !DISubprogram(name: "unregister_die_notifier", scope: !56, file: !56, line: 540, type: !1144, scopeLine: 541, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !298)
!1152 = !DILocalVariable(name: "nb", arg: 1, scope: !1151, file: !56, line: 540, type: !33)
!1153 = !DILocation(line: 540, column: 52, scope: !1151)
!1154 = !DILocation(line: 542, column: 54, scope: !1151)
!1155 = !DILocation(line: 542, column: 9, scope: !1151)
!1156 = !DILocation(line: 542, column: 2, scope: !1151)
