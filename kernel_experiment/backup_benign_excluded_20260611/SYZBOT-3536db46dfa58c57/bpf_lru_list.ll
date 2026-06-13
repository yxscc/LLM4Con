; ModuleID = '/mlx_devbox/users/mayunlong.39/playground/linux.git/kernel/bpf/bpf_lru_list.c'
source_filename = "/mlx_devbox/users/mayunlong.39/playground/linux.git/kernel/bpf/bpf_lru_list.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.cpumask = type { [128 x i64] }
%struct.lock_class_key = type { %union.anon.2 }
%union.anon.2 = type { %struct.hlist_node }
%struct.hlist_node = type { ptr, ptr }
%struct.bpf_lru = type { %union.anon, ptr, ptr, i32, i32, i8, [39 x i8] }
%union.anon = type { %struct.bpf_common_lru }
%struct.bpf_common_lru = type { %struct.bpf_lru_list, ptr, [56 x i8] }
%struct.bpf_lru_list = type { [3 x %struct.list_head], [2 x i32], ptr, %struct.raw_spinlock, [56 x i8] }
%struct.list_head = type { ptr, ptr }
%struct.raw_spinlock = type { %struct.qspinlock, i32, i32, ptr, %struct.lockdep_map }
%struct.qspinlock = type { %union.anon.0 }
%union.anon.0 = type { %struct.atomic_t }
%struct.atomic_t = type { i32 }
%struct.lockdep_map = type { ptr, [2 x ptr], ptr, i16, i16, i32, i64 }
%struct.bpf_lru_node = type { %struct.list_head, i16, i8, i8 }
%struct.bpf_lru_locallist = type { [2 x %struct.list_head], i16, %struct.raw_spinlock }

@__cpu_possible_mask = external dso_local global %struct.cpumask, align 8
@nr_cpu_ids = external dso_local global i32, align 4
@__per_cpu_offset = external dso_local global [8192 x i64], align 16
@cpu_number = external dso_local global i32, section ".data..percpu..read_mostly", align 4
@.str = private unnamed_addr constant [78 x i8] c"/mlx_devbox/users/mayunlong.39/playground/linux.git/kernel/bpf/bpf_lru_list.c\00", align 1, !dbg !0
@bpf_lru_list_init.__key = internal global %struct.lock_class_key zeroinitializer, align 8, !dbg !7
@.str.1 = private unnamed_addr constant [9 x i8] c"&l->lock\00", align 1, !dbg !196
@bpf_lru_locallist_init.__key = internal global %struct.lock_class_key zeroinitializer, align 8, !dbg !201
@.str.2 = private unnamed_addr constant [13 x i8] c"&loc_l->lock\00", align 1, !dbg !207

; Function Attrs: noinline nounwind optnone uwtable
define dso_local ptr @bpf_lru_pop_free(ptr noundef %0, i32 noundef %1) #0 !dbg !218 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !244, metadata !DIExpression()), !dbg !245
  store i32 %1, ptr %5, align 4
  call void @llvm.dbg.declare(metadata ptr %5, metadata !246, metadata !DIExpression()), !dbg !247
  %6 = load ptr, ptr %4, align 8, !dbg !248
  %7 = getelementptr inbounds %struct.bpf_lru, ptr %6, i32 0, i32 5, !dbg !250
  %8 = load i8, ptr %7, align 8, !dbg !250
  %9 = trunc i8 %8 to i1, !dbg !250
  br i1 %9, label %10, label %14, !dbg !251

10:                                               ; preds = %2
  %11 = load ptr, ptr %4, align 8, !dbg !252
  %12 = load i32, ptr %5, align 4, !dbg !253
  %13 = call ptr @bpf_percpu_lru_pop_free(ptr noundef %11, i32 noundef %12), !dbg !254
  store ptr %13, ptr %3, align 8, !dbg !255
  br label %18, !dbg !255

14:                                               ; preds = %2
  %15 = load ptr, ptr %4, align 8, !dbg !256
  %16 = load i32, ptr %5, align 4, !dbg !257
  %17 = call ptr @bpf_common_lru_pop_free(ptr noundef %15, i32 noundef %16), !dbg !258
  store ptr %17, ptr %3, align 8, !dbg !259
  br label %18, !dbg !259

18:                                               ; preds = %14, %10
  %19 = load ptr, ptr %3, align 8, !dbg !260
  ret ptr %19, !dbg !260
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @bpf_percpu_lru_pop_free(ptr noundef %0, i32 noundef %1) #0 !dbg !261 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i64, align 8
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  %11 = alloca ptr, align 8
  %12 = alloca i32, align 4
  %13 = alloca i32, align 4
  %14 = alloca i32, align 4
  %15 = alloca ptr, align 8
  %16 = alloca ptr, align 8
  %17 = alloca i64, align 8
  %18 = alloca ptr, align 8
  %19 = alloca i64, align 8
  %20 = alloca i64, align 8
  %21 = alloca i32, align 4
  %22 = alloca ptr, align 8
  %23 = alloca ptr, align 8
  %24 = alloca i64, align 8
  %25 = alloca i64, align 8
  %26 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !262, metadata !DIExpression()), !dbg !263
  store i32 %1, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !264, metadata !DIExpression()), !dbg !265
  call void @llvm.dbg.declare(metadata ptr %5, metadata !266, metadata !DIExpression()), !dbg !267
  call void @llvm.dbg.declare(metadata ptr %6, metadata !268, metadata !DIExpression()), !dbg !269
  store ptr null, ptr %6, align 8, !dbg !269
  call void @llvm.dbg.declare(metadata ptr %7, metadata !270, metadata !DIExpression()), !dbg !271
  call void @llvm.dbg.declare(metadata ptr %8, metadata !272, metadata !DIExpression()), !dbg !273
  call void @llvm.dbg.declare(metadata ptr %9, metadata !274, metadata !DIExpression()), !dbg !275
  call void @llvm.dbg.declare(metadata ptr %10, metadata !276, metadata !DIExpression()), !dbg !278
  br label %27, !dbg !278

27:                                               ; preds = %2
  call void @llvm.dbg.declare(metadata ptr %11, metadata !279, metadata !DIExpression()), !dbg !283
  store ptr null, ptr %11, align 8, !dbg !283
  %28 = load ptr, ptr %11, align 8, !dbg !283
  br label %29, !dbg !283

29:                                               ; preds = %27
  call void @llvm.dbg.declare(metadata ptr %12, metadata !284, metadata !DIExpression()), !dbg !286
  %30 = call i32 asm sideeffect "movl %gs:$1, $0", "=r,*m,~{dirflag},~{fpsr},~{flags}"(ptr elementtype(i32) @cpu_number) #4, !dbg !286, !srcloc !287
  store i32 %30, ptr %12, align 4, !dbg !286
  %31 = load i32, ptr %12, align 4, !dbg !286
  %32 = zext i32 %31 to i64, !dbg !286
  %33 = trunc i64 %32 to i32, !dbg !286
  store i32 %33, ptr %13, align 4, !dbg !286
  %34 = load i32, ptr %13, align 4, !dbg !286
  store i32 %34, ptr %10, align 4, !dbg !278
  %35 = load i32, ptr %10, align 4, !dbg !278
  store i32 %35, ptr %14, align 4, !dbg !278
  %36 = load i32, ptr %14, align 4, !dbg !278
  store i32 %36, ptr %9, align 4, !dbg !275
  br label %37, !dbg !288

37:                                               ; preds = %29
  call void @llvm.dbg.declare(metadata ptr %15, metadata !290, metadata !DIExpression()), !dbg !292
  store ptr null, ptr %15, align 8, !dbg !292
  %38 = load ptr, ptr %15, align 8, !dbg !292
  br label %39, !dbg !292

39:                                               ; preds = %37
  call void @llvm.dbg.declare(metadata ptr %17, metadata !293, metadata !DIExpression()), !dbg !295
  %40 = load ptr, ptr %3, align 8, !dbg !295
  %41 = getelementptr inbounds %struct.bpf_lru, ptr %40, i32 0, i32 0, !dbg !295
  %42 = load ptr, ptr %41, align 64, !dbg !295
  %43 = ptrtoint ptr %42 to i64, !dbg !295
  store i64 %43, ptr %17, align 8, !dbg !295
  %44 = load i64, ptr %17, align 8, !dbg !295
  %45 = load i32, ptr %9, align 4, !dbg !295
  %46 = sext i32 %45 to i64, !dbg !295
  %47 = getelementptr inbounds [8192 x i64], ptr @__per_cpu_offset, i64 0, i64 %46, !dbg !295
  %48 = load i64, ptr %47, align 8, !dbg !295
  %49 = add i64 %44, %48, !dbg !295
  %50 = inttoptr i64 %49 to ptr, !dbg !295
  store ptr %50, ptr %18, align 8, !dbg !295
  %51 = load ptr, ptr %18, align 8, !dbg !295
  store ptr %51, ptr %16, align 8, !dbg !292
  %52 = load ptr, ptr %16, align 8, !dbg !288
  store ptr %52, ptr %7, align 8, !dbg !296
  br label %53, !dbg !297

53:                                               ; preds = %39
  call void @llvm.dbg.declare(metadata ptr %19, metadata !298, metadata !DIExpression()), !dbg !301
  call void @llvm.dbg.declare(metadata ptr %20, metadata !302, metadata !DIExpression()), !dbg !301
  %54 = icmp eq ptr %19, %20, !dbg !301
  %55 = zext i1 %54 to i32, !dbg !301
  store i32 1, ptr %21, align 4, !dbg !301
  %56 = load i32, ptr %21, align 4, !dbg !301
  %57 = load ptr, ptr %7, align 8, !dbg !303
  %58 = getelementptr inbounds %struct.bpf_lru_list, ptr %57, i32 0, i32 3, !dbg !303
  %59 = call i64 @_raw_spin_lock_irqsave(ptr noundef %58), !dbg !303
  store i64 %59, ptr %8, align 8, !dbg !303
  br label %60, !dbg !303

60:                                               ; preds = %53
  %61 = load ptr, ptr %3, align 8, !dbg !304
  %62 = load ptr, ptr %7, align 8, !dbg !305
  call void @__bpf_lru_list_rotate(ptr noundef %61, ptr noundef %62), !dbg !306
  %63 = load ptr, ptr %7, align 8, !dbg !307
  %64 = getelementptr inbounds %struct.bpf_lru_list, ptr %63, i32 0, i32 0, !dbg !308
  %65 = getelementptr inbounds [3 x %struct.list_head], ptr %64, i64 0, i64 2, !dbg !307
  store ptr %65, ptr %5, align 8, !dbg !309
  %66 = load ptr, ptr %5, align 8, !dbg !310
  %67 = call i32 @list_empty(ptr noundef %66), !dbg !312
  %68 = icmp ne i32 %67, 0, !dbg !312
  br i1 %68, label %69, label %74, !dbg !313

69:                                               ; preds = %60
  %70 = load ptr, ptr %3, align 8, !dbg !314
  %71 = load ptr, ptr %7, align 8, !dbg !315
  %72 = load ptr, ptr %5, align 8, !dbg !316
  %73 = call i32 @__bpf_lru_list_shrink(ptr noundef %70, ptr noundef %71, i32 noundef 4, ptr noundef %72, i32 noundef 2), !dbg !317
  br label %74, !dbg !317

74:                                               ; preds = %69, %60
  %75 = load ptr, ptr %5, align 8, !dbg !318
  %76 = call i32 @list_empty(ptr noundef %75), !dbg !320
  %77 = icmp ne i32 %76, 0, !dbg !320
  br i1 %77, label %98, label %78, !dbg !321

78:                                               ; preds = %74
  call void @llvm.dbg.declare(metadata ptr %22, metadata !322, metadata !DIExpression()), !dbg !325
  %79 = load ptr, ptr %5, align 8, !dbg !325
  %80 = getelementptr inbounds %struct.list_head, ptr %79, i32 0, i32 0, !dbg !325
  %81 = load ptr, ptr %80, align 8, !dbg !325
  store ptr %81, ptr %22, align 8, !dbg !325
  br label %82, !dbg !325

82:                                               ; preds = %78
  br label %83, !dbg !326

83:                                               ; preds = %82
  %84 = load ptr, ptr %22, align 8, !dbg !325
  %85 = getelementptr i8, ptr %84, i64 0, !dbg !325
  store ptr %85, ptr %23, align 8, !dbg !326
  %86 = load ptr, ptr %23, align 8, !dbg !325
  store ptr %86, ptr %6, align 8, !dbg !328
  %87 = load i32, ptr %4, align 4, !dbg !329
  %88 = load ptr, ptr %6, align 8, !dbg !330
  %89 = load ptr, ptr %3, align 8, !dbg !331
  %90 = getelementptr inbounds %struct.bpf_lru, ptr %89, i32 0, i32 3, !dbg !332
  %91 = load i32, ptr %90, align 16, !dbg !332
  %92 = zext i32 %91 to i64, !dbg !333
  %93 = getelementptr i8, ptr %88, i64 %92, !dbg !333
  store i32 %87, ptr %93, align 4, !dbg !334
  %94 = load ptr, ptr %6, align 8, !dbg !335
  %95 = getelementptr inbounds %struct.bpf_lru_node, ptr %94, i32 0, i32 3, !dbg !336
  store i8 0, ptr %95, align 1, !dbg !337
  %96 = load ptr, ptr %7, align 8, !dbg !338
  %97 = load ptr, ptr %6, align 8, !dbg !339
  call void @__bpf_lru_node_move(ptr noundef %96, ptr noundef %97, i32 noundef 1), !dbg !340
  br label %98, !dbg !341

98:                                               ; preds = %83, %74
  br label %99, !dbg !342

99:                                               ; preds = %98
  call void @llvm.dbg.declare(metadata ptr %24, metadata !343, metadata !DIExpression()), !dbg !346
  call void @llvm.dbg.declare(metadata ptr %25, metadata !347, metadata !DIExpression()), !dbg !346
  %100 = icmp eq ptr %24, %25, !dbg !346
  %101 = zext i1 %100 to i32, !dbg !346
  store i32 1, ptr %26, align 4, !dbg !346
  %102 = load i32, ptr %26, align 4, !dbg !346
  %103 = load ptr, ptr %7, align 8, !dbg !348
  %104 = getelementptr inbounds %struct.bpf_lru_list, ptr %103, i32 0, i32 3, !dbg !348
  %105 = load i64, ptr %8, align 8, !dbg !348
  call void @_raw_spin_unlock_irqrestore(ptr noundef %104, i64 noundef %105), !dbg !348
  br label %106, !dbg !348

106:                                              ; preds = %99
  %107 = load ptr, ptr %6, align 8, !dbg !349
  ret ptr %107, !dbg !350
}

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @bpf_common_lru_pop_free(ptr noundef %0, i32 noundef %1) #0 !dbg !351 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca i32, align 4
  %11 = alloca i32, align 4
  %12 = alloca i64, align 8
  %13 = alloca i32, align 4
  %14 = alloca i32, align 4
  %15 = alloca ptr, align 8
  %16 = alloca i32, align 4
  %17 = alloca i32, align 4
  %18 = alloca i32, align 4
  %19 = alloca ptr, align 8
  %20 = alloca ptr, align 8
  %21 = alloca i64, align 8
  %22 = alloca ptr, align 8
  %23 = alloca i64, align 8
  %24 = alloca i64, align 8
  %25 = alloca i32, align 4
  %26 = alloca i64, align 8
  %27 = alloca i64, align 8
  %28 = alloca i32, align 4
  %29 = alloca ptr, align 8
  %30 = alloca ptr, align 8
  %31 = alloca i64, align 8
  %32 = alloca ptr, align 8
  %33 = alloca i64, align 8
  %34 = alloca i64, align 8
  %35 = alloca i32, align 4
  %36 = alloca i64, align 8
  %37 = alloca i64, align 8
  %38 = alloca i32, align 4
  %39 = alloca i64, align 8
  %40 = alloca i64, align 8
  %41 = alloca i32, align 4
  %42 = alloca i64, align 8
  %43 = alloca i64, align 8
  %44 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !352, metadata !DIExpression()), !dbg !353
  store i32 %1, ptr %5, align 4
  call void @llvm.dbg.declare(metadata ptr %5, metadata !354, metadata !DIExpression()), !dbg !355
  call void @llvm.dbg.declare(metadata ptr %6, metadata !356, metadata !DIExpression()), !dbg !357
  call void @llvm.dbg.declare(metadata ptr %7, metadata !358, metadata !DIExpression()), !dbg !359
  call void @llvm.dbg.declare(metadata ptr %8, metadata !360, metadata !DIExpression()), !dbg !362
  %45 = load ptr, ptr %4, align 8, !dbg !363
  %46 = getelementptr inbounds %struct.bpf_lru, ptr %45, i32 0, i32 0, !dbg !364
  store ptr %46, ptr %8, align 8, !dbg !362
  call void @llvm.dbg.declare(metadata ptr %9, metadata !365, metadata !DIExpression()), !dbg !366
  call void @llvm.dbg.declare(metadata ptr %10, metadata !367, metadata !DIExpression()), !dbg !368
  call void @llvm.dbg.declare(metadata ptr %11, metadata !369, metadata !DIExpression()), !dbg !370
  call void @llvm.dbg.declare(metadata ptr %12, metadata !371, metadata !DIExpression()), !dbg !372
  call void @llvm.dbg.declare(metadata ptr %13, metadata !373, metadata !DIExpression()), !dbg !374
  call void @llvm.dbg.declare(metadata ptr %14, metadata !375, metadata !DIExpression()), !dbg !377
  br label %47, !dbg !377

47:                                               ; preds = %2
  call void @llvm.dbg.declare(metadata ptr %15, metadata !378, metadata !DIExpression()), !dbg !380
  store ptr null, ptr %15, align 8, !dbg !380
  %48 = load ptr, ptr %15, align 8, !dbg !380
  br label %49, !dbg !380

49:                                               ; preds = %47
  call void @llvm.dbg.declare(metadata ptr %16, metadata !381, metadata !DIExpression()), !dbg !383
  %50 = call i32 asm sideeffect "movl %gs:$1, $0", "=r,*m,~{dirflag},~{fpsr},~{flags}"(ptr elementtype(i32) @cpu_number) #4, !dbg !383, !srcloc !384
  store i32 %50, ptr %16, align 4, !dbg !383
  %51 = load i32, ptr %16, align 4, !dbg !383
  %52 = zext i32 %51 to i64, !dbg !383
  %53 = trunc i64 %52 to i32, !dbg !383
  store i32 %53, ptr %17, align 4, !dbg !383
  %54 = load i32, ptr %17, align 4, !dbg !383
  store i32 %54, ptr %14, align 4, !dbg !377
  %55 = load i32, ptr %14, align 4, !dbg !377
  store i32 %55, ptr %18, align 4, !dbg !377
  %56 = load i32, ptr %18, align 4, !dbg !377
  store i32 %56, ptr %13, align 4, !dbg !374
  br label %57, !dbg !385

57:                                               ; preds = %49
  call void @llvm.dbg.declare(metadata ptr %19, metadata !387, metadata !DIExpression()), !dbg !389
  store ptr null, ptr %19, align 8, !dbg !389
  %58 = load ptr, ptr %19, align 8, !dbg !389
  br label %59, !dbg !389

59:                                               ; preds = %57
  call void @llvm.dbg.declare(metadata ptr %21, metadata !390, metadata !DIExpression()), !dbg !392
  %60 = load ptr, ptr %8, align 8, !dbg !392
  %61 = getelementptr inbounds %struct.bpf_common_lru, ptr %60, i32 0, i32 1, !dbg !392
  %62 = load ptr, ptr %61, align 64, !dbg !392
  %63 = ptrtoint ptr %62 to i64, !dbg !392
  store i64 %63, ptr %21, align 8, !dbg !392
  %64 = load i64, ptr %21, align 8, !dbg !392
  %65 = load i32, ptr %13, align 4, !dbg !392
  %66 = sext i32 %65 to i64, !dbg !392
  %67 = getelementptr inbounds [8192 x i64], ptr @__per_cpu_offset, i64 0, i64 %66, !dbg !392
  %68 = load i64, ptr %67, align 8, !dbg !392
  %69 = add i64 %64, %68, !dbg !392
  %70 = inttoptr i64 %69 to ptr, !dbg !392
  store ptr %70, ptr %22, align 8, !dbg !392
  %71 = load ptr, ptr %22, align 8, !dbg !392
  store ptr %71, ptr %20, align 8, !dbg !389
  %72 = load ptr, ptr %20, align 8, !dbg !385
  store ptr %72, ptr %6, align 8, !dbg !393
  br label %73, !dbg !394

73:                                               ; preds = %59
  call void @llvm.dbg.declare(metadata ptr %23, metadata !395, metadata !DIExpression()), !dbg !398
  call void @llvm.dbg.declare(metadata ptr %24, metadata !399, metadata !DIExpression()), !dbg !398
  %74 = icmp eq ptr %23, %24, !dbg !398
  %75 = zext i1 %74 to i32, !dbg !398
  store i32 1, ptr %25, align 4, !dbg !398
  %76 = load i32, ptr %25, align 4, !dbg !398
  %77 = load ptr, ptr %6, align 8, !dbg !400
  %78 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %77, i32 0, i32 2, !dbg !400
  %79 = call i64 @_raw_spin_lock_irqsave(ptr noundef %78), !dbg !400
  store i64 %79, ptr %12, align 8, !dbg !400
  br label %80, !dbg !400

80:                                               ; preds = %73
  %81 = load ptr, ptr %6, align 8, !dbg !401
  %82 = call ptr @__local_list_pop_free(ptr noundef %81), !dbg !402
  store ptr %82, ptr %9, align 8, !dbg !403
  %83 = load ptr, ptr %9, align 8, !dbg !404
  %84 = icmp ne ptr %83, null, !dbg !404
  br i1 %84, label %90, label %85, !dbg !406

85:                                               ; preds = %80
  %86 = load ptr, ptr %4, align 8, !dbg !407
  %87 = load ptr, ptr %6, align 8, !dbg !409
  call void @bpf_lru_list_pop_free_to_local(ptr noundef %86, ptr noundef %87), !dbg !410
  %88 = load ptr, ptr %6, align 8, !dbg !411
  %89 = call ptr @__local_list_pop_free(ptr noundef %88), !dbg !412
  store ptr %89, ptr %9, align 8, !dbg !413
  br label %90, !dbg !414

90:                                               ; preds = %85, %80
  %91 = load ptr, ptr %9, align 8, !dbg !415
  %92 = icmp ne ptr %91, null, !dbg !415
  br i1 %92, label %93, label %99, !dbg !417

93:                                               ; preds = %90
  %94 = load ptr, ptr %4, align 8, !dbg !418
  %95 = load ptr, ptr %6, align 8, !dbg !419
  %96 = load i32, ptr %13, align 4, !dbg !420
  %97 = load ptr, ptr %9, align 8, !dbg !421
  %98 = load i32, ptr %5, align 4, !dbg !422
  call void @__local_list_add_pending(ptr noundef %94, ptr noundef %95, i32 noundef %96, ptr noundef %97, i32 noundef %98), !dbg !423
  br label %99, !dbg !423

99:                                               ; preds = %93, %90
  br label %100, !dbg !424

100:                                              ; preds = %99
  call void @llvm.dbg.declare(metadata ptr %26, metadata !425, metadata !DIExpression()), !dbg !428
  call void @llvm.dbg.declare(metadata ptr %27, metadata !429, metadata !DIExpression()), !dbg !428
  %101 = icmp eq ptr %26, %27, !dbg !428
  %102 = zext i1 %101 to i32, !dbg !428
  store i32 1, ptr %28, align 4, !dbg !428
  %103 = load i32, ptr %28, align 4, !dbg !428
  %104 = load ptr, ptr %6, align 8, !dbg !430
  %105 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %104, i32 0, i32 2, !dbg !430
  %106 = load i64, ptr %12, align 8, !dbg !430
  call void @_raw_spin_unlock_irqrestore(ptr noundef %105, i64 noundef %106), !dbg !430
  br label %107, !dbg !430

107:                                              ; preds = %100
  %108 = load ptr, ptr %9, align 8, !dbg !431
  %109 = icmp ne ptr %108, null, !dbg !431
  br i1 %109, label %110, label %112, !dbg !433

110:                                              ; preds = %107
  %111 = load ptr, ptr %9, align 8, !dbg !434
  store ptr %111, ptr %3, align 8, !dbg !435
  br label %202, !dbg !435

112:                                              ; preds = %107
  %113 = load ptr, ptr %6, align 8, !dbg !436
  %114 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %113, i32 0, i32 1, !dbg !437
  %115 = load i16, ptr %114, align 8, !dbg !437
  %116 = zext i16 %115 to i32, !dbg !436
  store i32 %116, ptr %11, align 4, !dbg !438
  %117 = load i32, ptr %11, align 4, !dbg !439
  store i32 %117, ptr %10, align 4, !dbg !440
  br label %118, !dbg !441

118:                                              ; preds = %169, %112
  br label %119, !dbg !442

119:                                              ; preds = %118
  call void @llvm.dbg.declare(metadata ptr %29, metadata !445, metadata !DIExpression()), !dbg !447
  store ptr null, ptr %29, align 8, !dbg !447
  %120 = load ptr, ptr %29, align 8, !dbg !447
  br label %121, !dbg !447

121:                                              ; preds = %119
  call void @llvm.dbg.declare(metadata ptr %31, metadata !448, metadata !DIExpression()), !dbg !450
  %122 = load ptr, ptr %8, align 8, !dbg !450
  %123 = getelementptr inbounds %struct.bpf_common_lru, ptr %122, i32 0, i32 1, !dbg !450
  %124 = load ptr, ptr %123, align 64, !dbg !450
  %125 = ptrtoint ptr %124 to i64, !dbg !450
  store i64 %125, ptr %31, align 8, !dbg !450
  %126 = load i64, ptr %31, align 8, !dbg !450
  %127 = load i32, ptr %10, align 4, !dbg !450
  %128 = sext i32 %127 to i64, !dbg !450
  %129 = getelementptr inbounds [8192 x i64], ptr @__per_cpu_offset, i64 0, i64 %128, !dbg !450
  %130 = load i64, ptr %129, align 8, !dbg !450
  %131 = add i64 %126, %130, !dbg !450
  %132 = inttoptr i64 %131 to ptr, !dbg !450
  store ptr %132, ptr %32, align 8, !dbg !450
  %133 = load ptr, ptr %32, align 8, !dbg !450
  store ptr %133, ptr %30, align 8, !dbg !447
  %134 = load ptr, ptr %30, align 8, !dbg !442
  store ptr %134, ptr %7, align 8, !dbg !451
  br label %135, !dbg !452

135:                                              ; preds = %121
  call void @llvm.dbg.declare(metadata ptr %33, metadata !453, metadata !DIExpression()), !dbg !456
  call void @llvm.dbg.declare(metadata ptr %34, metadata !457, metadata !DIExpression()), !dbg !456
  %136 = icmp eq ptr %33, %34, !dbg !456
  %137 = zext i1 %136 to i32, !dbg !456
  store i32 1, ptr %35, align 4, !dbg !456
  %138 = load i32, ptr %35, align 4, !dbg !456
  %139 = load ptr, ptr %7, align 8, !dbg !458
  %140 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %139, i32 0, i32 2, !dbg !458
  %141 = call i64 @_raw_spin_lock_irqsave(ptr noundef %140), !dbg !458
  store i64 %141, ptr %12, align 8, !dbg !458
  br label %142, !dbg !458

142:                                              ; preds = %135
  %143 = load ptr, ptr %7, align 8, !dbg !459
  %144 = call ptr @__local_list_pop_free(ptr noundef %143), !dbg !460
  store ptr %144, ptr %9, align 8, !dbg !461
  %145 = load ptr, ptr %9, align 8, !dbg !462
  %146 = icmp ne ptr %145, null, !dbg !462
  br i1 %146, label %151, label %147, !dbg !464

147:                                              ; preds = %142
  %148 = load ptr, ptr %4, align 8, !dbg !465
  %149 = load ptr, ptr %7, align 8, !dbg !466
  %150 = call ptr @__local_list_pop_pending(ptr noundef %148, ptr noundef %149), !dbg !467
  store ptr %150, ptr %9, align 8, !dbg !468
  br label %151, !dbg !469

151:                                              ; preds = %147, %142
  br label %152, !dbg !470

152:                                              ; preds = %151
  call void @llvm.dbg.declare(metadata ptr %36, metadata !471, metadata !DIExpression()), !dbg !474
  call void @llvm.dbg.declare(metadata ptr %37, metadata !475, metadata !DIExpression()), !dbg !474
  %153 = icmp eq ptr %36, %37, !dbg !474
  %154 = zext i1 %153 to i32, !dbg !474
  store i32 1, ptr %38, align 4, !dbg !474
  %155 = load i32, ptr %38, align 4, !dbg !474
  %156 = load ptr, ptr %7, align 8, !dbg !476
  %157 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %156, i32 0, i32 2, !dbg !476
  %158 = load i64, ptr %12, align 8, !dbg !476
  call void @_raw_spin_unlock_irqrestore(ptr noundef %157, i64 noundef %158), !dbg !476
  br label %159, !dbg !476

159:                                              ; preds = %152
  %160 = load i32, ptr %10, align 4, !dbg !477
  %161 = call i32 @get_next_cpu(i32 noundef %160), !dbg !478
  store i32 %161, ptr %10, align 4, !dbg !479
  br label %162, !dbg !480

162:                                              ; preds = %159
  %163 = load ptr, ptr %9, align 8, !dbg !481
  %164 = icmp ne ptr %163, null, !dbg !481
  br i1 %164, label %169, label %165, !dbg !482

165:                                              ; preds = %162
  %166 = load i32, ptr %10, align 4, !dbg !483
  %167 = load i32, ptr %11, align 4, !dbg !484
  %168 = icmp ne i32 %166, %167, !dbg !485
  br label %169

169:                                              ; preds = %165, %162
  %170 = phi i1 [ false, %162 ], [ %168, %165 ], !dbg !486
  br i1 %170, label %118, label %171, !dbg !480, !llvm.loop !487

171:                                              ; preds = %169
  %172 = load i32, ptr %10, align 4, !dbg !490
  %173 = trunc i32 %172 to i16, !dbg !490
  %174 = load ptr, ptr %6, align 8, !dbg !491
  %175 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %174, i32 0, i32 1, !dbg !492
  store i16 %173, ptr %175, align 8, !dbg !493
  %176 = load ptr, ptr %9, align 8, !dbg !494
  %177 = icmp ne ptr %176, null, !dbg !494
  br i1 %177, label %178, label %200, !dbg !496

178:                                              ; preds = %171
  br label %179, !dbg !497

179:                                              ; preds = %178
  call void @llvm.dbg.declare(metadata ptr %39, metadata !499, metadata !DIExpression()), !dbg !502
  call void @llvm.dbg.declare(metadata ptr %40, metadata !503, metadata !DIExpression()), !dbg !502
  %180 = icmp eq ptr %39, %40, !dbg !502
  %181 = zext i1 %180 to i32, !dbg !502
  store i32 1, ptr %41, align 4, !dbg !502
  %182 = load i32, ptr %41, align 4, !dbg !502
  %183 = load ptr, ptr %6, align 8, !dbg !504
  %184 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %183, i32 0, i32 2, !dbg !504
  %185 = call i64 @_raw_spin_lock_irqsave(ptr noundef %184), !dbg !504
  store i64 %185, ptr %12, align 8, !dbg !504
  br label %186, !dbg !504

186:                                              ; preds = %179
  %187 = load ptr, ptr %4, align 8, !dbg !505
  %188 = load ptr, ptr %6, align 8, !dbg !506
  %189 = load i32, ptr %13, align 4, !dbg !507
  %190 = load ptr, ptr %9, align 8, !dbg !508
  %191 = load i32, ptr %5, align 4, !dbg !509
  call void @__local_list_add_pending(ptr noundef %187, ptr noundef %188, i32 noundef %189, ptr noundef %190, i32 noundef %191), !dbg !510
  br label %192, !dbg !511

192:                                              ; preds = %186
  call void @llvm.dbg.declare(metadata ptr %42, metadata !512, metadata !DIExpression()), !dbg !515
  call void @llvm.dbg.declare(metadata ptr %43, metadata !516, metadata !DIExpression()), !dbg !515
  %193 = icmp eq ptr %42, %43, !dbg !515
  %194 = zext i1 %193 to i32, !dbg !515
  store i32 1, ptr %44, align 4, !dbg !515
  %195 = load i32, ptr %44, align 4, !dbg !515
  %196 = load ptr, ptr %6, align 8, !dbg !517
  %197 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %196, i32 0, i32 2, !dbg !517
  %198 = load i64, ptr %12, align 8, !dbg !517
  call void @_raw_spin_unlock_irqrestore(ptr noundef %197, i64 noundef %198), !dbg !517
  br label %199, !dbg !517

199:                                              ; preds = %192
  br label %200, !dbg !518

200:                                              ; preds = %199, %171
  %201 = load ptr, ptr %9, align 8, !dbg !519
  store ptr %201, ptr %3, align 8, !dbg !520
  br label %202, !dbg !520

202:                                              ; preds = %200, %110
  %203 = load ptr, ptr %3, align 8, !dbg !521
  ret ptr %203, !dbg !521
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @bpf_lru_push_free(ptr noundef %0, ptr noundef %1) #0 !dbg !522 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !525, metadata !DIExpression()), !dbg !526
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !527, metadata !DIExpression()), !dbg !528
  %5 = load ptr, ptr %3, align 8, !dbg !529
  %6 = getelementptr inbounds %struct.bpf_lru, ptr %5, i32 0, i32 5, !dbg !531
  %7 = load i8, ptr %6, align 8, !dbg !531
  %8 = trunc i8 %7 to i1, !dbg !531
  br i1 %8, label %9, label %12, !dbg !532

9:                                                ; preds = %2
  %10 = load ptr, ptr %3, align 8, !dbg !533
  %11 = load ptr, ptr %4, align 8, !dbg !534
  call void @bpf_percpu_lru_push_free(ptr noundef %10, ptr noundef %11), !dbg !535
  br label %15, !dbg !535

12:                                               ; preds = %2
  %13 = load ptr, ptr %3, align 8, !dbg !536
  %14 = load ptr, ptr %4, align 8, !dbg !537
  call void @bpf_common_lru_push_free(ptr noundef %13, ptr noundef %14), !dbg !538
  br label %15

15:                                               ; preds = %12, %9
  ret void, !dbg !539
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bpf_percpu_lru_push_free(ptr noundef %0, ptr noundef %1) #0 !dbg !540 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i64, align 8
  %10 = alloca ptr, align 8
  %11 = alloca i64, align 8
  %12 = alloca i64, align 8
  %13 = alloca i32, align 4
  %14 = alloca i64, align 8
  %15 = alloca i64, align 8
  %16 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !541, metadata !DIExpression()), !dbg !542
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !543, metadata !DIExpression()), !dbg !544
  call void @llvm.dbg.declare(metadata ptr %5, metadata !545, metadata !DIExpression()), !dbg !546
  call void @llvm.dbg.declare(metadata ptr %6, metadata !547, metadata !DIExpression()), !dbg !548
  br label %17, !dbg !549

17:                                               ; preds = %2
  call void @llvm.dbg.declare(metadata ptr %7, metadata !551, metadata !DIExpression()), !dbg !553
  store ptr null, ptr %7, align 8, !dbg !553
  %18 = load ptr, ptr %7, align 8, !dbg !553
  br label %19, !dbg !553

19:                                               ; preds = %17
  call void @llvm.dbg.declare(metadata ptr %9, metadata !554, metadata !DIExpression()), !dbg !556
  %20 = load ptr, ptr %3, align 8, !dbg !556
  %21 = getelementptr inbounds %struct.bpf_lru, ptr %20, i32 0, i32 0, !dbg !556
  %22 = load ptr, ptr %21, align 64, !dbg !556
  %23 = ptrtoint ptr %22 to i64, !dbg !556
  store i64 %23, ptr %9, align 8, !dbg !556
  %24 = load i64, ptr %9, align 8, !dbg !556
  %25 = load ptr, ptr %4, align 8, !dbg !556
  %26 = getelementptr inbounds %struct.bpf_lru_node, ptr %25, i32 0, i32 1, !dbg !556
  %27 = load i16, ptr %26, align 8, !dbg !556
  %28 = zext i16 %27 to i64, !dbg !556
  %29 = getelementptr inbounds [8192 x i64], ptr @__per_cpu_offset, i64 0, i64 %28, !dbg !556
  %30 = load i64, ptr %29, align 8, !dbg !556
  %31 = add i64 %24, %30, !dbg !556
  %32 = inttoptr i64 %31 to ptr, !dbg !556
  store ptr %32, ptr %10, align 8, !dbg !556
  %33 = load ptr, ptr %10, align 8, !dbg !556
  store ptr %33, ptr %8, align 8, !dbg !553
  %34 = load ptr, ptr %8, align 8, !dbg !549
  store ptr %34, ptr %5, align 8, !dbg !557
  br label %35, !dbg !558

35:                                               ; preds = %19
  call void @llvm.dbg.declare(metadata ptr %11, metadata !559, metadata !DIExpression()), !dbg !562
  call void @llvm.dbg.declare(metadata ptr %12, metadata !563, metadata !DIExpression()), !dbg !562
  %36 = icmp eq ptr %11, %12, !dbg !562
  %37 = zext i1 %36 to i32, !dbg !562
  store i32 1, ptr %13, align 4, !dbg !562
  %38 = load i32, ptr %13, align 4, !dbg !562
  %39 = load ptr, ptr %5, align 8, !dbg !564
  %40 = getelementptr inbounds %struct.bpf_lru_list, ptr %39, i32 0, i32 3, !dbg !564
  %41 = call i64 @_raw_spin_lock_irqsave(ptr noundef %40), !dbg !564
  store i64 %41, ptr %6, align 8, !dbg !564
  br label %42, !dbg !564

42:                                               ; preds = %35
  %43 = load ptr, ptr %5, align 8, !dbg !565
  %44 = load ptr, ptr %4, align 8, !dbg !566
  call void @__bpf_lru_node_move(ptr noundef %43, ptr noundef %44, i32 noundef 2), !dbg !567
  br label %45, !dbg !568

45:                                               ; preds = %42
  call void @llvm.dbg.declare(metadata ptr %14, metadata !569, metadata !DIExpression()), !dbg !572
  call void @llvm.dbg.declare(metadata ptr %15, metadata !573, metadata !DIExpression()), !dbg !572
  %46 = icmp eq ptr %14, %15, !dbg !572
  %47 = zext i1 %46 to i32, !dbg !572
  store i32 1, ptr %16, align 4, !dbg !572
  %48 = load i32, ptr %16, align 4, !dbg !572
  %49 = load ptr, ptr %5, align 8, !dbg !574
  %50 = getelementptr inbounds %struct.bpf_lru_list, ptr %49, i32 0, i32 3, !dbg !574
  %51 = load i64, ptr %6, align 8, !dbg !574
  call void @_raw_spin_unlock_irqrestore(ptr noundef %50, i64 noundef %51), !dbg !574
  br label %52, !dbg !574

52:                                               ; preds = %45
  ret void, !dbg !575
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bpf_common_lru_push_free(ptr noundef %0, ptr noundef %1) #0 !dbg !576 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca i32, align 4
  %7 = alloca i64, align 8
  %8 = alloca i32, align 4
  %9 = alloca i64, align 8
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  %12 = alloca ptr, align 8
  %13 = alloca i64, align 8
  %14 = alloca ptr, align 8
  %15 = alloca i64, align 8
  %16 = alloca i64, align 8
  %17 = alloca i32, align 4
  %18 = alloca i64, align 8
  %19 = alloca i64, align 8
  %20 = alloca i32, align 4
  %21 = alloca i64, align 8
  %22 = alloca i64, align 8
  %23 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !577, metadata !DIExpression()), !dbg !578
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !579, metadata !DIExpression()), !dbg !580
  call void @llvm.dbg.declare(metadata ptr %5, metadata !581, metadata !DIExpression()), !dbg !582
  call void @llvm.dbg.declare(metadata ptr %6, metadata !583, metadata !DIExpression()), !dbg !586
  %24 = load ptr, ptr %4, align 8, !dbg !586
  %25 = getelementptr inbounds %struct.bpf_lru_node, ptr %24, i32 0, i32 2, !dbg !586
  %26 = load i8, ptr %25, align 2, !dbg !586
  %27 = zext i8 %26 to i32, !dbg !586
  %28 = icmp eq i32 %27, 2, !dbg !586
  %29 = xor i1 %28, true, !dbg !586
  %30 = xor i1 %29, true, !dbg !586
  %31 = zext i1 %30 to i32, !dbg !586
  store i32 %31, ptr %6, align 4, !dbg !586
  %32 = load i32, ptr %6, align 4, !dbg !587
  %33 = icmp ne i32 %32, 0, !dbg !587
  %34 = xor i1 %33, true, !dbg !587
  %35 = xor i1 %34, true, !dbg !587
  %36 = zext i1 %35 to i32, !dbg !587
  %37 = sext i32 %36 to i64, !dbg !587
  %38 = icmp ne i64 %37, 0, !dbg !587
  br i1 %38, label %39, label %44, !dbg !586

39:                                               ; preds = %2
  br label %40, !dbg !587

40:                                               ; preds = %39
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_begin\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 256) #4, !dbg !589, !srcloc !592
  br label %41, !dbg !593

41:                                               ; preds = %40
  call void asm sideeffect "1:\09.byte 0x0f, 0x0b\0A.pushsection __bug_table,\22aw\22\0A2:\09.long 1b - 2b\09# bug_entry::bug_addr\0A\09.long ${0:c} - 2b\09# bug_entry::file\0A\09.word ${1:c}\09# bug_entry::line\0A\09.word ${2:c}\09# bug_entry::flags\0A\09.org 2b+${3:c}\0A.popsection", "i,i,i,i,~{dirflag},~{fpsr},~{flags}"(ptr @.str, i32 507, i32 2307, i64 12) #4, !dbg !594, !srcloc !596
  br label %42, !dbg !594

42:                                               ; preds = %41
  call void asm sideeffect "${0:c}:\0A\09.pushsection .discard.reachable\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 257) #4, !dbg !597, !srcloc !599
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_end\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 258) #4, !dbg !600, !srcloc !602
  br label %43, !dbg !593

43:                                               ; preds = %42
  br label %44, !dbg !593

44:                                               ; preds = %43, %2
  %45 = load i32, ptr %6, align 4, !dbg !586
  %46 = icmp ne i32 %45, 0, !dbg !586
  %47 = xor i1 %46, true, !dbg !586
  %48 = xor i1 %47, true, !dbg !586
  %49 = zext i1 %48 to i32, !dbg !586
  %50 = sext i32 %49 to i64, !dbg !586
  store i64 %50, ptr %7, align 8, !dbg !587
  %51 = load i64, ptr %7, align 8, !dbg !586
  %52 = icmp ne i64 %51, 0, !dbg !603
  br i1 %52, label %83, label %53, !dbg !604

53:                                               ; preds = %44
  call void @llvm.dbg.declare(metadata ptr %8, metadata !605, metadata !DIExpression()), !dbg !607
  %54 = load ptr, ptr %4, align 8, !dbg !607
  %55 = getelementptr inbounds %struct.bpf_lru_node, ptr %54, i32 0, i32 2, !dbg !607
  %56 = load i8, ptr %55, align 2, !dbg !607
  %57 = zext i8 %56 to i32, !dbg !607
  %58 = icmp eq i32 %57, 3, !dbg !607
  %59 = xor i1 %58, true, !dbg !607
  %60 = xor i1 %59, true, !dbg !607
  %61 = zext i1 %60 to i32, !dbg !607
  store i32 %61, ptr %8, align 4, !dbg !607
  %62 = load i32, ptr %8, align 4, !dbg !608
  %63 = icmp ne i32 %62, 0, !dbg !608
  %64 = xor i1 %63, true, !dbg !608
  %65 = xor i1 %64, true, !dbg !608
  %66 = zext i1 %65 to i32, !dbg !608
  %67 = sext i32 %66 to i64, !dbg !608
  %68 = icmp ne i64 %67, 0, !dbg !608
  br i1 %68, label %69, label %74, !dbg !607

69:                                               ; preds = %53
  br label %70, !dbg !608

70:                                               ; preds = %69
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_begin\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 259) #4, !dbg !610, !srcloc !613
  br label %71, !dbg !614

71:                                               ; preds = %70
  call void asm sideeffect "1:\09.byte 0x0f, 0x0b\0A.pushsection __bug_table,\22aw\22\0A2:\09.long 1b - 2b\09# bug_entry::bug_addr\0A\09.long ${0:c} - 2b\09# bug_entry::file\0A\09.word ${1:c}\09# bug_entry::line\0A\09.word ${2:c}\09# bug_entry::flags\0A\09.org 2b+${3:c}\0A.popsection", "i,i,i,i,~{dirflag},~{fpsr},~{flags}"(ptr @.str, i32 508, i32 2307, i64 12) #4, !dbg !615, !srcloc !617
  br label %72, !dbg !615

72:                                               ; preds = %71
  call void asm sideeffect "${0:c}:\0A\09.pushsection .discard.reachable\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 260) #4, !dbg !618, !srcloc !620
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_end\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 261) #4, !dbg !621, !srcloc !623
  br label %73, !dbg !614

73:                                               ; preds = %72
  br label %74, !dbg !614

74:                                               ; preds = %73, %53
  %75 = load i32, ptr %8, align 4, !dbg !607
  %76 = icmp ne i32 %75, 0, !dbg !607
  %77 = xor i1 %76, true, !dbg !607
  %78 = xor i1 %77, true, !dbg !607
  %79 = zext i1 %78 to i32, !dbg !607
  %80 = sext i32 %79 to i64, !dbg !607
  store i64 %80, ptr %9, align 8, !dbg !608
  %81 = load i64, ptr %9, align 8, !dbg !607
  %82 = icmp ne i64 %81, 0, !dbg !624
  br i1 %82, label %83, label %84, !dbg !625

83:                                               ; preds = %74, %44
  br label %160, !dbg !626

84:                                               ; preds = %74
  %85 = load ptr, ptr %4, align 8, !dbg !627
  %86 = getelementptr inbounds %struct.bpf_lru_node, ptr %85, i32 0, i32 2, !dbg !629
  %87 = load i8, ptr %86, align 2, !dbg !629
  %88 = zext i8 %87 to i32, !dbg !627
  %89 = icmp eq i32 %88, 4, !dbg !630
  br i1 %89, label %90, label %154, !dbg !631

90:                                               ; preds = %84
  call void @llvm.dbg.declare(metadata ptr %10, metadata !632, metadata !DIExpression()), !dbg !634
  br label %91, !dbg !635

91:                                               ; preds = %90
  call void @llvm.dbg.declare(metadata ptr %11, metadata !637, metadata !DIExpression()), !dbg !639
  store ptr null, ptr %11, align 8, !dbg !639
  %92 = load ptr, ptr %11, align 8, !dbg !639
  br label %93, !dbg !639

93:                                               ; preds = %91
  call void @llvm.dbg.declare(metadata ptr %13, metadata !640, metadata !DIExpression()), !dbg !642
  %94 = load ptr, ptr %3, align 8, !dbg !642
  %95 = getelementptr inbounds %struct.bpf_lru, ptr %94, i32 0, i32 0, !dbg !642
  %96 = getelementptr inbounds %struct.bpf_common_lru, ptr %95, i32 0, i32 1, !dbg !642
  %97 = load ptr, ptr %96, align 64, !dbg !642
  %98 = ptrtoint ptr %97 to i64, !dbg !642
  store i64 %98, ptr %13, align 8, !dbg !642
  %99 = load i64, ptr %13, align 8, !dbg !642
  %100 = load ptr, ptr %4, align 8, !dbg !642
  %101 = getelementptr inbounds %struct.bpf_lru_node, ptr %100, i32 0, i32 1, !dbg !642
  %102 = load i16, ptr %101, align 8, !dbg !642
  %103 = zext i16 %102 to i64, !dbg !642
  %104 = getelementptr inbounds [8192 x i64], ptr @__per_cpu_offset, i64 0, i64 %103, !dbg !642
  %105 = load i64, ptr %104, align 8, !dbg !642
  %106 = add i64 %99, %105, !dbg !642
  %107 = inttoptr i64 %106 to ptr, !dbg !642
  store ptr %107, ptr %14, align 8, !dbg !642
  %108 = load ptr, ptr %14, align 8, !dbg !642
  store ptr %108, ptr %12, align 8, !dbg !639
  %109 = load ptr, ptr %12, align 8, !dbg !635
  store ptr %109, ptr %10, align 8, !dbg !643
  br label %110, !dbg !644

110:                                              ; preds = %93
  call void @llvm.dbg.declare(metadata ptr %15, metadata !645, metadata !DIExpression()), !dbg !648
  call void @llvm.dbg.declare(metadata ptr %16, metadata !649, metadata !DIExpression()), !dbg !648
  %111 = icmp eq ptr %15, %16, !dbg !648
  %112 = zext i1 %111 to i32, !dbg !648
  store i32 1, ptr %17, align 4, !dbg !648
  %113 = load i32, ptr %17, align 4, !dbg !648
  %114 = load ptr, ptr %10, align 8, !dbg !650
  %115 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %114, i32 0, i32 2, !dbg !650
  %116 = call i64 @_raw_spin_lock_irqsave(ptr noundef %115), !dbg !650
  store i64 %116, ptr %5, align 8, !dbg !650
  br label %117, !dbg !650

117:                                              ; preds = %110
  %118 = load ptr, ptr %4, align 8, !dbg !651
  %119 = getelementptr inbounds %struct.bpf_lru_node, ptr %118, i32 0, i32 2, !dbg !651
  %120 = load i8, ptr %119, align 2, !dbg !651
  %121 = zext i8 %120 to i32, !dbg !651
  %122 = icmp ne i32 %121, 4, !dbg !651
  %123 = xor i1 %122, true, !dbg !651
  %124 = xor i1 %123, true, !dbg !651
  %125 = zext i1 %124 to i32, !dbg !651
  %126 = sext i32 %125 to i64, !dbg !651
  %127 = icmp ne i64 %126, 0, !dbg !651
  br i1 %127, label %128, label %137, !dbg !653

128:                                              ; preds = %117
  br label %129, !dbg !654

129:                                              ; preds = %128
  call void @llvm.dbg.declare(metadata ptr %18, metadata !656, metadata !DIExpression()), !dbg !659
  call void @llvm.dbg.declare(metadata ptr %19, metadata !660, metadata !DIExpression()), !dbg !659
  %130 = icmp eq ptr %18, %19, !dbg !659
  %131 = zext i1 %130 to i32, !dbg !659
  store i32 1, ptr %20, align 4, !dbg !659
  %132 = load i32, ptr %20, align 4, !dbg !659
  %133 = load ptr, ptr %10, align 8, !dbg !661
  %134 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %133, i32 0, i32 2, !dbg !661
  %135 = load i64, ptr %5, align 8, !dbg !661
  call void @_raw_spin_unlock_irqrestore(ptr noundef %134, i64 noundef %135), !dbg !661
  br label %136, !dbg !661

136:                                              ; preds = %129
  br label %155, !dbg !662

137:                                              ; preds = %117
  %138 = load ptr, ptr %4, align 8, !dbg !663
  %139 = getelementptr inbounds %struct.bpf_lru_node, ptr %138, i32 0, i32 2, !dbg !664
  store i8 3, ptr %139, align 2, !dbg !665
  %140 = load ptr, ptr %4, align 8, !dbg !666
  %141 = getelementptr inbounds %struct.bpf_lru_node, ptr %140, i32 0, i32 3, !dbg !667
  store i8 0, ptr %141, align 1, !dbg !668
  %142 = load ptr, ptr %4, align 8, !dbg !669
  %143 = getelementptr inbounds %struct.bpf_lru_node, ptr %142, i32 0, i32 0, !dbg !670
  %144 = load ptr, ptr %10, align 8, !dbg !671
  %145 = call ptr @local_free_list(ptr noundef %144), !dbg !672
  call void @list_move(ptr noundef %143, ptr noundef %145), !dbg !673
  br label %146, !dbg !674

146:                                              ; preds = %137
  call void @llvm.dbg.declare(metadata ptr %21, metadata !675, metadata !DIExpression()), !dbg !678
  call void @llvm.dbg.declare(metadata ptr %22, metadata !679, metadata !DIExpression()), !dbg !678
  %147 = icmp eq ptr %21, %22, !dbg !678
  %148 = zext i1 %147 to i32, !dbg !678
  store i32 1, ptr %23, align 4, !dbg !678
  %149 = load i32, ptr %23, align 4, !dbg !678
  %150 = load ptr, ptr %10, align 8, !dbg !680
  %151 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %150, i32 0, i32 2, !dbg !680
  %152 = load i64, ptr %5, align 8, !dbg !680
  call void @_raw_spin_unlock_irqrestore(ptr noundef %151, i64 noundef %152), !dbg !680
  br label %153, !dbg !680

153:                                              ; preds = %146
  br label %160, !dbg !681

154:                                              ; preds = %84
  br label %155, !dbg !682

155:                                              ; preds = %154, %136
  call void @llvm.dbg.label(metadata !683), !dbg !684
  %156 = load ptr, ptr %3, align 8, !dbg !685
  %157 = getelementptr inbounds %struct.bpf_lru, ptr %156, i32 0, i32 0, !dbg !686
  %158 = getelementptr inbounds %struct.bpf_common_lru, ptr %157, i32 0, i32 0, !dbg !687
  %159 = load ptr, ptr %4, align 8, !dbg !688
  call void @bpf_lru_list_push_free(ptr noundef %158, ptr noundef %159), !dbg !689
  br label %160, !dbg !690

160:                                              ; preds = %155, %153, %83
  ret void, !dbg !690
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @bpf_lru_populate(ptr noundef %0, ptr noundef %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) #0 !dbg !691 {
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  store ptr %0, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !694, metadata !DIExpression()), !dbg !695
  store ptr %1, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !696, metadata !DIExpression()), !dbg !697
  store i32 %2, ptr %8, align 4
  call void @llvm.dbg.declare(metadata ptr %8, metadata !698, metadata !DIExpression()), !dbg !699
  store i32 %3, ptr %9, align 4
  call void @llvm.dbg.declare(metadata ptr %9, metadata !700, metadata !DIExpression()), !dbg !701
  store i32 %4, ptr %10, align 4
  call void @llvm.dbg.declare(metadata ptr %10, metadata !702, metadata !DIExpression()), !dbg !703
  %11 = load ptr, ptr %6, align 8, !dbg !704
  %12 = getelementptr inbounds %struct.bpf_lru, ptr %11, i32 0, i32 5, !dbg !706
  %13 = load i8, ptr %12, align 8, !dbg !706
  %14 = trunc i8 %13 to i1, !dbg !706
  br i1 %14, label %15, label %21, !dbg !707

15:                                               ; preds = %5
  %16 = load ptr, ptr %6, align 8, !dbg !708
  %17 = load ptr, ptr %7, align 8, !dbg !709
  %18 = load i32, ptr %8, align 4, !dbg !710
  %19 = load i32, ptr %9, align 4, !dbg !711
  %20 = load i32, ptr %10, align 4, !dbg !712
  call void @bpf_percpu_lru_populate(ptr noundef %16, ptr noundef %17, i32 noundef %18, i32 noundef %19, i32 noundef %20), !dbg !713
  br label %27, !dbg !713

21:                                               ; preds = %5
  %22 = load ptr, ptr %6, align 8, !dbg !714
  %23 = load ptr, ptr %7, align 8, !dbg !715
  %24 = load i32, ptr %8, align 4, !dbg !716
  %25 = load i32, ptr %9, align 4, !dbg !717
  %26 = load i32, ptr %10, align 4, !dbg !718
  call void @bpf_common_lru_populate(ptr noundef %22, ptr noundef %23, i32 noundef %24, i32 noundef %25, i32 noundef %26), !dbg !719
  br label %27

27:                                               ; preds = %21, %15
  ret void, !dbg !720
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bpf_percpu_lru_populate(ptr noundef %0, ptr noundef %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) #0 !dbg !721 {
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  %11 = alloca i32, align 4
  %12 = alloca i32, align 4
  %13 = alloca i32, align 4
  %14 = alloca ptr, align 8
  %15 = alloca ptr, align 8
  %16 = alloca ptr, align 8
  %17 = alloca ptr, align 8
  %18 = alloca i64, align 8
  %19 = alloca ptr, align 8
  store ptr %0, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !722, metadata !DIExpression()), !dbg !723
  store ptr %1, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !724, metadata !DIExpression()), !dbg !725
  store i32 %2, ptr %8, align 4
  call void @llvm.dbg.declare(metadata ptr %8, metadata !726, metadata !DIExpression()), !dbg !727
  store i32 %3, ptr %9, align 4
  call void @llvm.dbg.declare(metadata ptr %9, metadata !728, metadata !DIExpression()), !dbg !729
  store i32 %4, ptr %10, align 4
  call void @llvm.dbg.declare(metadata ptr %10, metadata !730, metadata !DIExpression()), !dbg !731
  call void @llvm.dbg.declare(metadata ptr %11, metadata !732, metadata !DIExpression()), !dbg !733
  call void @llvm.dbg.declare(metadata ptr %12, metadata !734, metadata !DIExpression()), !dbg !735
  call void @llvm.dbg.declare(metadata ptr %13, metadata !736, metadata !DIExpression()), !dbg !737
  call void @llvm.dbg.declare(metadata ptr %14, metadata !738, metadata !DIExpression()), !dbg !739
  %20 = load i32, ptr %10, align 4, !dbg !740
  %21 = call i32 @cpumask_weight(ptr noundef @__cpu_possible_mask), !dbg !741
  %22 = udiv i32 %20, %21, !dbg !742
  store i32 %22, ptr %12, align 4, !dbg !743
  store i32 0, ptr %11, align 4, !dbg !744
  store i32 -1, ptr %13, align 4, !dbg !745
  br label %23, !dbg !745

23:                                               ; preds = %80, %5
  %24 = load i32, ptr %13, align 4, !dbg !747
  %25 = call i32 @cpumask_next(i32 noundef %24, ptr noundef @__cpu_possible_mask), !dbg !747
  store i32 %25, ptr %13, align 4, !dbg !747
  %26 = load i32, ptr %13, align 4, !dbg !747
  %27 = load i32, ptr @nr_cpu_ids, align 4, !dbg !747
  %28 = icmp ult i32 %26, %27, !dbg !747
  br i1 %28, label %29, label %81, !dbg !745

29:                                               ; preds = %23
  call void @llvm.dbg.declare(metadata ptr %15, metadata !749, metadata !DIExpression()), !dbg !751
  br label %30, !dbg !752

30:                                               ; preds = %29
  call void @llvm.dbg.declare(metadata ptr %16, metadata !754, metadata !DIExpression()), !dbg !756
  store ptr null, ptr %16, align 8, !dbg !756
  %31 = load ptr, ptr %16, align 8, !dbg !756
  br label %32, !dbg !756

32:                                               ; preds = %30
  call void @llvm.dbg.declare(metadata ptr %18, metadata !757, metadata !DIExpression()), !dbg !759
  %33 = load ptr, ptr %6, align 8, !dbg !759
  %34 = getelementptr inbounds %struct.bpf_lru, ptr %33, i32 0, i32 0, !dbg !759
  %35 = load ptr, ptr %34, align 64, !dbg !759
  %36 = ptrtoint ptr %35 to i64, !dbg !759
  store i64 %36, ptr %18, align 8, !dbg !759
  %37 = load i64, ptr %18, align 8, !dbg !759
  %38 = load i32, ptr %13, align 4, !dbg !759
  %39 = sext i32 %38 to i64, !dbg !759
  %40 = getelementptr inbounds [8192 x i64], ptr @__per_cpu_offset, i64 0, i64 %39, !dbg !759
  %41 = load i64, ptr %40, align 8, !dbg !759
  %42 = add i64 %37, %41, !dbg !759
  %43 = inttoptr i64 %42 to ptr, !dbg !759
  store ptr %43, ptr %19, align 8, !dbg !759
  %44 = load ptr, ptr %19, align 8, !dbg !759
  store ptr %44, ptr %17, align 8, !dbg !756
  %45 = load ptr, ptr %17, align 8, !dbg !752
  store ptr %45, ptr %14, align 8, !dbg !760
  br label %46, !dbg !761

46:                                               ; preds = %79, %32
  call void @llvm.dbg.label(metadata !762), !dbg !763
  %47 = load ptr, ptr %7, align 8, !dbg !764
  %48 = load i32, ptr %8, align 4, !dbg !765
  %49 = zext i32 %48 to i64, !dbg !766
  %50 = getelementptr i8, ptr %47, i64 %49, !dbg !766
  store ptr %50, ptr %15, align 8, !dbg !767
  %51 = load i32, ptr %13, align 4, !dbg !768
  %52 = trunc i32 %51 to i16, !dbg !768
  %53 = load ptr, ptr %15, align 8, !dbg !769
  %54 = getelementptr inbounds %struct.bpf_lru_node, ptr %53, i32 0, i32 1, !dbg !770
  store i16 %52, ptr %54, align 8, !dbg !771
  %55 = load ptr, ptr %15, align 8, !dbg !772
  %56 = getelementptr inbounds %struct.bpf_lru_node, ptr %55, i32 0, i32 2, !dbg !773
  store i8 2, ptr %56, align 2, !dbg !774
  %57 = load ptr, ptr %15, align 8, !dbg !775
  %58 = getelementptr inbounds %struct.bpf_lru_node, ptr %57, i32 0, i32 3, !dbg !776
  store i8 0, ptr %58, align 1, !dbg !777
  %59 = load ptr, ptr %15, align 8, !dbg !778
  %60 = getelementptr inbounds %struct.bpf_lru_node, ptr %59, i32 0, i32 0, !dbg !779
  %61 = load ptr, ptr %14, align 8, !dbg !780
  %62 = getelementptr inbounds %struct.bpf_lru_list, ptr %61, i32 0, i32 0, !dbg !781
  %63 = getelementptr inbounds [3 x %struct.list_head], ptr %62, i64 0, i64 2, !dbg !780
  call void @list_add(ptr noundef %60, ptr noundef %63), !dbg !782
  %64 = load i32, ptr %11, align 4, !dbg !783
  %65 = add i32 %64, 1, !dbg !783
  store i32 %65, ptr %11, align 4, !dbg !783
  %66 = load i32, ptr %9, align 4, !dbg !784
  %67 = load ptr, ptr %7, align 8, !dbg !785
  %68 = zext i32 %66 to i64, !dbg !785
  %69 = getelementptr i8, ptr %67, i64 %68, !dbg !785
  store ptr %69, ptr %7, align 8, !dbg !785
  %70 = load i32, ptr %11, align 4, !dbg !786
  %71 = load i32, ptr %10, align 4, !dbg !788
  %72 = icmp eq i32 %70, %71, !dbg !789
  br i1 %72, label %73, label %74, !dbg !790

73:                                               ; preds = %46
  br label %81, !dbg !791

74:                                               ; preds = %46
  %75 = load i32, ptr %11, align 4, !dbg !792
  %76 = load i32, ptr %12, align 4, !dbg !794
  %77 = urem i32 %75, %76, !dbg !795
  %78 = icmp ne i32 %77, 0, !dbg !795
  br i1 %78, label %79, label %80, !dbg !796

79:                                               ; preds = %74
  br label %46, !dbg !797

80:                                               ; preds = %74
  br label %23, !dbg !747, !llvm.loop !798

81:                                               ; preds = %73, %23
  ret void, !dbg !800
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bpf_common_lru_populate(ptr noundef %0, ptr noundef %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) #0 !dbg !801 {
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  %11 = alloca ptr, align 8
  %12 = alloca i32, align 4
  %13 = alloca ptr, align 8
  store ptr %0, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !802, metadata !DIExpression()), !dbg !803
  store ptr %1, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !804, metadata !DIExpression()), !dbg !805
  store i32 %2, ptr %8, align 4
  call void @llvm.dbg.declare(metadata ptr %8, metadata !806, metadata !DIExpression()), !dbg !807
  store i32 %3, ptr %9, align 4
  call void @llvm.dbg.declare(metadata ptr %9, metadata !808, metadata !DIExpression()), !dbg !809
  store i32 %4, ptr %10, align 4
  call void @llvm.dbg.declare(metadata ptr %10, metadata !810, metadata !DIExpression()), !dbg !811
  call void @llvm.dbg.declare(metadata ptr %11, metadata !812, metadata !DIExpression()), !dbg !813
  %14 = load ptr, ptr %6, align 8, !dbg !814
  %15 = getelementptr inbounds %struct.bpf_lru, ptr %14, i32 0, i32 0, !dbg !815
  %16 = getelementptr inbounds %struct.bpf_common_lru, ptr %15, i32 0, i32 0, !dbg !816
  store ptr %16, ptr %11, align 8, !dbg !813
  call void @llvm.dbg.declare(metadata ptr %12, metadata !817, metadata !DIExpression()), !dbg !818
  store i32 0, ptr %12, align 4, !dbg !819
  br label %17, !dbg !821

17:                                               ; preds = %39, %5
  %18 = load i32, ptr %12, align 4, !dbg !822
  %19 = load i32, ptr %10, align 4, !dbg !824
  %20 = icmp ult i32 %18, %19, !dbg !825
  br i1 %20, label %21, label %42, !dbg !826

21:                                               ; preds = %17
  call void @llvm.dbg.declare(metadata ptr %13, metadata !827, metadata !DIExpression()), !dbg !829
  %22 = load ptr, ptr %7, align 8, !dbg !830
  %23 = load i32, ptr %8, align 4, !dbg !831
  %24 = zext i32 %23 to i64, !dbg !832
  %25 = getelementptr i8, ptr %22, i64 %24, !dbg !832
  store ptr %25, ptr %13, align 8, !dbg !833
  %26 = load ptr, ptr %13, align 8, !dbg !834
  %27 = getelementptr inbounds %struct.bpf_lru_node, ptr %26, i32 0, i32 2, !dbg !835
  store i8 2, ptr %27, align 2, !dbg !836
  %28 = load ptr, ptr %13, align 8, !dbg !837
  %29 = getelementptr inbounds %struct.bpf_lru_node, ptr %28, i32 0, i32 3, !dbg !838
  store i8 0, ptr %29, align 1, !dbg !839
  %30 = load ptr, ptr %13, align 8, !dbg !840
  %31 = getelementptr inbounds %struct.bpf_lru_node, ptr %30, i32 0, i32 0, !dbg !841
  %32 = load ptr, ptr %11, align 8, !dbg !842
  %33 = getelementptr inbounds %struct.bpf_lru_list, ptr %32, i32 0, i32 0, !dbg !843
  %34 = getelementptr inbounds [3 x %struct.list_head], ptr %33, i64 0, i64 2, !dbg !842
  call void @list_add(ptr noundef %31, ptr noundef %34), !dbg !844
  %35 = load i32, ptr %9, align 4, !dbg !845
  %36 = load ptr, ptr %7, align 8, !dbg !846
  %37 = zext i32 %35 to i64, !dbg !846
  %38 = getelementptr i8, ptr %36, i64 %37, !dbg !846
  store ptr %38, ptr %7, align 8, !dbg !846
  br label %39, !dbg !847

39:                                               ; preds = %21
  %40 = load i32, ptr %12, align 4, !dbg !848
  %41 = add i32 %40, 1, !dbg !848
  store i32 %41, ptr %12, align 4, !dbg !848
  br label %17, !dbg !849, !llvm.loop !850

42:                                               ; preds = %17
  ret void, !dbg !852
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @bpf_lru_init(ptr noundef %0, i1 noundef zeroext %1, i32 noundef %2, ptr noundef %3, ptr noundef %4) #0 !dbg !853 {
  %6 = alloca i32, align 4
  %7 = alloca ptr, align 8
  %8 = alloca i8, align 1
  %9 = alloca i32, align 4
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  %12 = alloca i32, align 4
  %13 = alloca ptr, align 8
  %14 = alloca ptr, align 8
  %15 = alloca ptr, align 8
  %16 = alloca i64, align 8
  %17 = alloca ptr, align 8
  %18 = alloca ptr, align 8
  %19 = alloca ptr, align 8
  %20 = alloca ptr, align 8
  %21 = alloca ptr, align 8
  %22 = alloca i64, align 8
  %23 = alloca ptr, align 8
  store ptr %0, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !856, metadata !DIExpression()), !dbg !857
  %24 = zext i1 %1 to i8
  store i8 %24, ptr %8, align 1
  call void @llvm.dbg.declare(metadata ptr %8, metadata !858, metadata !DIExpression()), !dbg !859
  store i32 %2, ptr %9, align 4
  call void @llvm.dbg.declare(metadata ptr %9, metadata !860, metadata !DIExpression()), !dbg !861
  store ptr %3, ptr %10, align 8
  call void @llvm.dbg.declare(metadata ptr %10, metadata !862, metadata !DIExpression()), !dbg !863
  store ptr %4, ptr %11, align 8
  call void @llvm.dbg.declare(metadata ptr %11, metadata !864, metadata !DIExpression()), !dbg !865
  call void @llvm.dbg.declare(metadata ptr %12, metadata !866, metadata !DIExpression()), !dbg !867
  %25 = load i8, ptr %8, align 1, !dbg !868
  %26 = trunc i8 %25 to i1, !dbg !868
  br i1 %26, label %27, label %64, !dbg !870

27:                                               ; preds = %5
  %28 = call ptr @__alloc_percpu(i64 noundef 192, i64 noundef 64), !dbg !871
  %29 = load ptr, ptr %7, align 8, !dbg !873
  %30 = getelementptr inbounds %struct.bpf_lru, ptr %29, i32 0, i32 0, !dbg !874
  store ptr %28, ptr %30, align 64, !dbg !875
  %31 = load ptr, ptr %7, align 8, !dbg !876
  %32 = getelementptr inbounds %struct.bpf_lru, ptr %31, i32 0, i32 0, !dbg !878
  %33 = load ptr, ptr %32, align 64, !dbg !878
  %34 = icmp ne ptr %33, null, !dbg !876
  br i1 %34, label %36, label %35, !dbg !879

35:                                               ; preds = %27
  store i32 -12, ptr %6, align 4, !dbg !880
  br label %121, !dbg !880

36:                                               ; preds = %27
  store i32 -1, ptr %12, align 4, !dbg !881
  br label %37, !dbg !881

37:                                               ; preds = %46, %36
  %38 = load i32, ptr %12, align 4, !dbg !883
  %39 = call i32 @cpumask_next(i32 noundef %38, ptr noundef @__cpu_possible_mask), !dbg !883
  store i32 %39, ptr %12, align 4, !dbg !883
  %40 = load i32, ptr %12, align 4, !dbg !883
  %41 = load i32, ptr @nr_cpu_ids, align 4, !dbg !883
  %42 = icmp ult i32 %40, %41, !dbg !883
  br i1 %42, label %43, label %61, !dbg !881

43:                                               ; preds = %37
  call void @llvm.dbg.declare(metadata ptr %13, metadata !885, metadata !DIExpression()), !dbg !887
  br label %44, !dbg !888

44:                                               ; preds = %43
  call void @llvm.dbg.declare(metadata ptr %14, metadata !890, metadata !DIExpression()), !dbg !892
  store ptr null, ptr %14, align 8, !dbg !892
  %45 = load ptr, ptr %14, align 8, !dbg !892
  br label %46, !dbg !892

46:                                               ; preds = %44
  call void @llvm.dbg.declare(metadata ptr %16, metadata !893, metadata !DIExpression()), !dbg !895
  %47 = load ptr, ptr %7, align 8, !dbg !895
  %48 = getelementptr inbounds %struct.bpf_lru, ptr %47, i32 0, i32 0, !dbg !895
  %49 = load ptr, ptr %48, align 64, !dbg !895
  %50 = ptrtoint ptr %49 to i64, !dbg !895
  store i64 %50, ptr %16, align 8, !dbg !895
  %51 = load i64, ptr %16, align 8, !dbg !895
  %52 = load i32, ptr %12, align 4, !dbg !895
  %53 = sext i32 %52 to i64, !dbg !895
  %54 = getelementptr inbounds [8192 x i64], ptr @__per_cpu_offset, i64 0, i64 %53, !dbg !895
  %55 = load i64, ptr %54, align 8, !dbg !895
  %56 = add i64 %51, %55, !dbg !895
  %57 = inttoptr i64 %56 to ptr, !dbg !895
  store ptr %57, ptr %17, align 8, !dbg !895
  %58 = load ptr, ptr %17, align 8, !dbg !895
  store ptr %58, ptr %15, align 8, !dbg !892
  %59 = load ptr, ptr %15, align 8, !dbg !888
  store ptr %59, ptr %13, align 8, !dbg !896
  %60 = load ptr, ptr %13, align 8, !dbg !897
  call void @bpf_lru_list_init(ptr noundef %60), !dbg !898
  br label %37, !dbg !883, !llvm.loop !899

61:                                               ; preds = %37
  %62 = load ptr, ptr %7, align 8, !dbg !901
  %63 = getelementptr inbounds %struct.bpf_lru, ptr %62, i32 0, i32 4, !dbg !902
  store i32 4, ptr %63, align 4, !dbg !903
  br label %106, !dbg !904

64:                                               ; preds = %5
  call void @llvm.dbg.declare(metadata ptr %18, metadata !905, metadata !DIExpression()), !dbg !907
  %65 = load ptr, ptr %7, align 8, !dbg !908
  %66 = getelementptr inbounds %struct.bpf_lru, ptr %65, i32 0, i32 0, !dbg !909
  store ptr %66, ptr %18, align 8, !dbg !907
  %67 = call ptr @__alloc_percpu(i64 noundef 112, i64 noundef 8), !dbg !910
  %68 = load ptr, ptr %18, align 8, !dbg !911
  %69 = getelementptr inbounds %struct.bpf_common_lru, ptr %68, i32 0, i32 1, !dbg !912
  store ptr %67, ptr %69, align 64, !dbg !913
  %70 = load ptr, ptr %18, align 8, !dbg !914
  %71 = getelementptr inbounds %struct.bpf_common_lru, ptr %70, i32 0, i32 1, !dbg !916
  %72 = load ptr, ptr %71, align 64, !dbg !916
  %73 = icmp ne ptr %72, null, !dbg !914
  br i1 %73, label %75, label %74, !dbg !917

74:                                               ; preds = %64
  store i32 -12, ptr %6, align 4, !dbg !918
  br label %121, !dbg !918

75:                                               ; preds = %64
  store i32 -1, ptr %12, align 4, !dbg !919
  br label %76, !dbg !919

76:                                               ; preds = %85, %75
  %77 = load i32, ptr %12, align 4, !dbg !921
  %78 = call i32 @cpumask_next(i32 noundef %77, ptr noundef @__cpu_possible_mask), !dbg !921
  store i32 %78, ptr %12, align 4, !dbg !921
  %79 = load i32, ptr %12, align 4, !dbg !921
  %80 = load i32, ptr @nr_cpu_ids, align 4, !dbg !921
  %81 = icmp ult i32 %79, %80, !dbg !921
  br i1 %81, label %82, label %101, !dbg !919

82:                                               ; preds = %76
  call void @llvm.dbg.declare(metadata ptr %19, metadata !923, metadata !DIExpression()), !dbg !925
  br label %83, !dbg !926

83:                                               ; preds = %82
  call void @llvm.dbg.declare(metadata ptr %20, metadata !928, metadata !DIExpression()), !dbg !930
  store ptr null, ptr %20, align 8, !dbg !930
  %84 = load ptr, ptr %20, align 8, !dbg !930
  br label %85, !dbg !930

85:                                               ; preds = %83
  call void @llvm.dbg.declare(metadata ptr %22, metadata !931, metadata !DIExpression()), !dbg !933
  %86 = load ptr, ptr %18, align 8, !dbg !933
  %87 = getelementptr inbounds %struct.bpf_common_lru, ptr %86, i32 0, i32 1, !dbg !933
  %88 = load ptr, ptr %87, align 64, !dbg !933
  %89 = ptrtoint ptr %88 to i64, !dbg !933
  store i64 %89, ptr %22, align 8, !dbg !933
  %90 = load i64, ptr %22, align 8, !dbg !933
  %91 = load i32, ptr %12, align 4, !dbg !933
  %92 = sext i32 %91 to i64, !dbg !933
  %93 = getelementptr inbounds [8192 x i64], ptr @__per_cpu_offset, i64 0, i64 %92, !dbg !933
  %94 = load i64, ptr %93, align 8, !dbg !933
  %95 = add i64 %90, %94, !dbg !933
  %96 = inttoptr i64 %95 to ptr, !dbg !933
  store ptr %96, ptr %23, align 8, !dbg !933
  %97 = load ptr, ptr %23, align 8, !dbg !933
  store ptr %97, ptr %21, align 8, !dbg !930
  %98 = load ptr, ptr %21, align 8, !dbg !926
  store ptr %98, ptr %19, align 8, !dbg !934
  %99 = load ptr, ptr %19, align 8, !dbg !935
  %100 = load i32, ptr %12, align 4, !dbg !936
  call void @bpf_lru_locallist_init(ptr noundef %99, i32 noundef %100), !dbg !937
  br label %76, !dbg !921, !llvm.loop !938

101:                                              ; preds = %76
  %102 = load ptr, ptr %18, align 8, !dbg !940
  %103 = getelementptr inbounds %struct.bpf_common_lru, ptr %102, i32 0, i32 0, !dbg !941
  call void @bpf_lru_list_init(ptr noundef %103), !dbg !942
  %104 = load ptr, ptr %7, align 8, !dbg !943
  %105 = getelementptr inbounds %struct.bpf_lru, ptr %104, i32 0, i32 4, !dbg !944
  store i32 128, ptr %105, align 4, !dbg !945
  br label %106

106:                                              ; preds = %101, %61
  %107 = load i8, ptr %8, align 1, !dbg !946
  %108 = trunc i8 %107 to i1, !dbg !946
  %109 = load ptr, ptr %7, align 8, !dbg !947
  %110 = getelementptr inbounds %struct.bpf_lru, ptr %109, i32 0, i32 5, !dbg !948
  %111 = zext i1 %108 to i8, !dbg !949
  store i8 %111, ptr %110, align 8, !dbg !949
  %112 = load ptr, ptr %10, align 8, !dbg !950
  %113 = load ptr, ptr %7, align 8, !dbg !951
  %114 = getelementptr inbounds %struct.bpf_lru, ptr %113, i32 0, i32 1, !dbg !952
  store ptr %112, ptr %114, align 64, !dbg !953
  %115 = load ptr, ptr %11, align 8, !dbg !954
  %116 = load ptr, ptr %7, align 8, !dbg !955
  %117 = getelementptr inbounds %struct.bpf_lru, ptr %116, i32 0, i32 2, !dbg !956
  store ptr %115, ptr %117, align 8, !dbg !957
  %118 = load i32, ptr %9, align 4, !dbg !958
  %119 = load ptr, ptr %7, align 8, !dbg !959
  %120 = getelementptr inbounds %struct.bpf_lru, ptr %119, i32 0, i32 3, !dbg !960
  store i32 %118, ptr %120, align 16, !dbg !961
  store i32 0, ptr %6, align 4, !dbg !962
  br label %121, !dbg !962

121:                                              ; preds = %106, %74, %35
  %122 = load i32, ptr %6, align 4, !dbg !963
  ret i32 %122, !dbg !963
}

declare dso_local ptr @__alloc_percpu(i64 noundef, i64 noundef) #2

declare dso_local i32 @cpumask_next(i32 noundef, ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bpf_lru_list_init(ptr noundef %0) #0 !dbg !9 {
  %2 = alloca ptr, align 8
  %3 = alloca i32, align 4
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !964, metadata !DIExpression()), !dbg !965
  call void @llvm.dbg.declare(metadata ptr %3, metadata !966, metadata !DIExpression()), !dbg !967
  store i32 0, ptr %3, align 4, !dbg !968
  br label %4, !dbg !970

4:                                                ; preds = %13, %1
  %5 = load i32, ptr %3, align 4, !dbg !971
  %6 = icmp slt i32 %5, 3, !dbg !973
  br i1 %6, label %7, label %16, !dbg !974

7:                                                ; preds = %4
  %8 = load ptr, ptr %2, align 8, !dbg !975
  %9 = getelementptr inbounds %struct.bpf_lru_list, ptr %8, i32 0, i32 0, !dbg !976
  %10 = load i32, ptr %3, align 4, !dbg !977
  %11 = sext i32 %10 to i64, !dbg !975
  %12 = getelementptr inbounds [3 x %struct.list_head], ptr %9, i64 0, i64 %11, !dbg !975
  call void @INIT_LIST_HEAD(ptr noundef %12), !dbg !978
  br label %13, !dbg !978

13:                                               ; preds = %7
  %14 = load i32, ptr %3, align 4, !dbg !979
  %15 = add nsw i32 %14, 1, !dbg !979
  store i32 %15, ptr %3, align 4, !dbg !979
  br label %4, !dbg !980, !llvm.loop !981

16:                                               ; preds = %4
  store i32 0, ptr %3, align 4, !dbg !983
  br label %17, !dbg !985

17:                                               ; preds = %26, %16
  %18 = load i32, ptr %3, align 4, !dbg !986
  %19 = icmp slt i32 %18, 2, !dbg !988
  br i1 %19, label %20, label %29, !dbg !989

20:                                               ; preds = %17
  %21 = load ptr, ptr %2, align 8, !dbg !990
  %22 = getelementptr inbounds %struct.bpf_lru_list, ptr %21, i32 0, i32 1, !dbg !991
  %23 = load i32, ptr %3, align 4, !dbg !992
  %24 = sext i32 %23 to i64, !dbg !990
  %25 = getelementptr inbounds [2 x i32], ptr %22, i64 0, i64 %24, !dbg !990
  store i32 0, ptr %25, align 4, !dbg !993
  br label %26, !dbg !990

26:                                               ; preds = %20
  %27 = load i32, ptr %3, align 4, !dbg !994
  %28 = add nsw i32 %27, 1, !dbg !994
  store i32 %28, ptr %3, align 4, !dbg !994
  br label %17, !dbg !995, !llvm.loop !996

29:                                               ; preds = %17
  %30 = load ptr, ptr %2, align 8, !dbg !998
  %31 = getelementptr inbounds %struct.bpf_lru_list, ptr %30, i32 0, i32 0, !dbg !999
  %32 = getelementptr inbounds [3 x %struct.list_head], ptr %31, i64 0, i64 1, !dbg !998
  %33 = load ptr, ptr %2, align 8, !dbg !1000
  %34 = getelementptr inbounds %struct.bpf_lru_list, ptr %33, i32 0, i32 2, !dbg !1001
  store ptr %32, ptr %34, align 8, !dbg !1002
  br label %35, !dbg !1003

35:                                               ; preds = %29
  %36 = load ptr, ptr %2, align 8, !dbg !1004
  %37 = getelementptr inbounds %struct.bpf_lru_list, ptr %36, i32 0, i32 3, !dbg !1004
  call void @__raw_spin_lock_init(ptr noundef %37, ptr noundef @.str.1, ptr noundef @bpf_lru_list_init.__key, i16 noundef signext 2), !dbg !1004
  br label %38, !dbg !1004

38:                                               ; preds = %35
  ret void, !dbg !1006
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bpf_lru_locallist_init(ptr noundef %0, i32 noundef %1) #0 !dbg !203 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !1007, metadata !DIExpression()), !dbg !1008
  store i32 %1, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1009, metadata !DIExpression()), !dbg !1010
  call void @llvm.dbg.declare(metadata ptr %5, metadata !1011, metadata !DIExpression()), !dbg !1012
  store i32 0, ptr %5, align 4, !dbg !1013
  br label %6, !dbg !1015

6:                                                ; preds = %15, %2
  %7 = load i32, ptr %5, align 4, !dbg !1016
  %8 = icmp slt i32 %7, 2, !dbg !1018
  br i1 %8, label %9, label %18, !dbg !1019

9:                                                ; preds = %6
  %10 = load ptr, ptr %3, align 8, !dbg !1020
  %11 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %10, i32 0, i32 0, !dbg !1021
  %12 = load i32, ptr %5, align 4, !dbg !1022
  %13 = sext i32 %12 to i64, !dbg !1020
  %14 = getelementptr inbounds [2 x %struct.list_head], ptr %11, i64 0, i64 %13, !dbg !1020
  call void @INIT_LIST_HEAD(ptr noundef %14), !dbg !1023
  br label %15, !dbg !1023

15:                                               ; preds = %9
  %16 = load i32, ptr %5, align 4, !dbg !1024
  %17 = add nsw i32 %16, 1, !dbg !1024
  store i32 %17, ptr %5, align 4, !dbg !1024
  br label %6, !dbg !1025, !llvm.loop !1026

18:                                               ; preds = %6
  %19 = load i32, ptr %4, align 4, !dbg !1028
  %20 = trunc i32 %19 to i16, !dbg !1028
  %21 = load ptr, ptr %3, align 8, !dbg !1029
  %22 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %21, i32 0, i32 1, !dbg !1030
  store i16 %20, ptr %22, align 8, !dbg !1031
  br label %23, !dbg !1032

23:                                               ; preds = %18
  %24 = load ptr, ptr %3, align 8, !dbg !1033
  %25 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %24, i32 0, i32 2, !dbg !1033
  call void @__raw_spin_lock_init(ptr noundef %25, ptr noundef @.str.2, ptr noundef @bpf_lru_locallist_init.__key, i16 noundef signext 2), !dbg !1033
  br label %26, !dbg !1033

26:                                               ; preds = %23
  ret void, !dbg !1035
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @bpf_lru_destroy(ptr noundef %0) #0 !dbg !1036 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !1039, metadata !DIExpression()), !dbg !1040
  %3 = load ptr, ptr %2, align 8, !dbg !1041
  %4 = getelementptr inbounds %struct.bpf_lru, ptr %3, i32 0, i32 5, !dbg !1043
  %5 = load i8, ptr %4, align 8, !dbg !1043
  %6 = trunc i8 %5 to i1, !dbg !1043
  br i1 %6, label %7, label %11, !dbg !1044

7:                                                ; preds = %1
  %8 = load ptr, ptr %2, align 8, !dbg !1045
  %9 = getelementptr inbounds %struct.bpf_lru, ptr %8, i32 0, i32 0, !dbg !1046
  %10 = load ptr, ptr %9, align 64, !dbg !1046
  call void @free_percpu(ptr noundef %10), !dbg !1047
  br label %16, !dbg !1047

11:                                               ; preds = %1
  %12 = load ptr, ptr %2, align 8, !dbg !1048
  %13 = getelementptr inbounds %struct.bpf_lru, ptr %12, i32 0, i32 0, !dbg !1049
  %14 = getelementptr inbounds %struct.bpf_common_lru, ptr %13, i32 0, i32 1, !dbg !1050
  %15 = load ptr, ptr %14, align 64, !dbg !1050
  call void @free_percpu(ptr noundef %15), !dbg !1051
  br label %16

16:                                               ; preds = %11, %7
  ret void, !dbg !1052
}

declare dso_local void @free_percpu(ptr noundef) #2

declare dso_local i64 @_raw_spin_lock_irqsave(ptr noundef) #2 section ".spinlock.text"

; Function Attrs: noinline nounwind optnone uwtable
define internal void @__bpf_lru_list_rotate(ptr noundef %0, ptr noundef %1) #0 !dbg !1053 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !1056, metadata !DIExpression()), !dbg !1057
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1058, metadata !DIExpression()), !dbg !1059
  %5 = load ptr, ptr %4, align 8, !dbg !1060
  %6 = call zeroext i1 @bpf_lru_list_inactive_low(ptr noundef %5), !dbg !1062
  br i1 %6, label %7, label %10, !dbg !1063

7:                                                ; preds = %2
  %8 = load ptr, ptr %3, align 8, !dbg !1064
  %9 = load ptr, ptr %4, align 8, !dbg !1065
  call void @__bpf_lru_list_rotate_active(ptr noundef %8, ptr noundef %9), !dbg !1066
  br label %10, !dbg !1066

10:                                               ; preds = %7, %2
  %11 = load ptr, ptr %3, align 8, !dbg !1067
  %12 = load ptr, ptr %4, align 8, !dbg !1068
  call void @__bpf_lru_list_rotate_inactive(ptr noundef %11, ptr noundef %12), !dbg !1069
  ret void, !dbg !1070
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @list_empty(ptr noundef %0) #0 !dbg !1071 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !1077, metadata !DIExpression()), !dbg !1078
  br label %4, !dbg !1079

4:                                                ; preds = %1
  br label %5, !dbg !1081

5:                                                ; preds = %4
  %6 = load ptr, ptr %2, align 8, !dbg !1079
  %7 = getelementptr inbounds %struct.list_head, ptr %6, i32 0, i32 0, !dbg !1079
  %8 = load volatile ptr, ptr %7, align 8, !dbg !1079
  store ptr %8, ptr %3, align 8, !dbg !1081
  %9 = load ptr, ptr %3, align 8, !dbg !1079
  %10 = load ptr, ptr %2, align 8, !dbg !1083
  %11 = icmp eq ptr %9, %10, !dbg !1084
  %12 = zext i1 %11 to i32, !dbg !1084
  ret i32 %12, !dbg !1085
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @__bpf_lru_list_shrink(ptr noundef %0, ptr noundef %1, i32 noundef %2, ptr noundef %3, i32 noundef %4) #0 !dbg !1086 {
  %6 = alloca i32, align 4
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i32, align 4
  %10 = alloca ptr, align 8
  %11 = alloca i32, align 4
  %12 = alloca ptr, align 8
  %13 = alloca ptr, align 8
  %14 = alloca ptr, align 8
  %15 = alloca i32, align 4
  %16 = alloca ptr, align 8
  %17 = alloca ptr, align 8
  %18 = alloca ptr, align 8
  %19 = alloca ptr, align 8
  %20 = alloca ptr, align 8
  %21 = alloca ptr, align 8
  store ptr %0, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !1089, metadata !DIExpression()), !dbg !1090
  store ptr %1, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !1091, metadata !DIExpression()), !dbg !1092
  store i32 %2, ptr %9, align 4
  call void @llvm.dbg.declare(metadata ptr %9, metadata !1093, metadata !DIExpression()), !dbg !1094
  store ptr %3, ptr %10, align 8
  call void @llvm.dbg.declare(metadata ptr %10, metadata !1095, metadata !DIExpression()), !dbg !1096
  store i32 %4, ptr %11, align 4
  call void @llvm.dbg.declare(metadata ptr %11, metadata !1097, metadata !DIExpression()), !dbg !1098
  call void @llvm.dbg.declare(metadata ptr %12, metadata !1099, metadata !DIExpression()), !dbg !1100
  call void @llvm.dbg.declare(metadata ptr %13, metadata !1101, metadata !DIExpression()), !dbg !1102
  call void @llvm.dbg.declare(metadata ptr %14, metadata !1103, metadata !DIExpression()), !dbg !1104
  call void @llvm.dbg.declare(metadata ptr %15, metadata !1105, metadata !DIExpression()), !dbg !1106
  %22 = load ptr, ptr %7, align 8, !dbg !1107
  %23 = load ptr, ptr %8, align 8, !dbg !1108
  %24 = load i32, ptr %9, align 4, !dbg !1109
  %25 = load ptr, ptr %10, align 8, !dbg !1110
  %26 = load i32, ptr %11, align 4, !dbg !1111
  %27 = call i32 @__bpf_lru_list_shrink_inactive(ptr noundef %22, ptr noundef %23, i32 noundef %24, ptr noundef %25, i32 noundef %26), !dbg !1112
  store i32 %27, ptr %15, align 4, !dbg !1113
  %28 = load i32, ptr %15, align 4, !dbg !1114
  %29 = icmp ne i32 %28, 0, !dbg !1114
  br i1 %29, label %30, label %32, !dbg !1116

30:                                               ; preds = %5
  %31 = load i32, ptr %15, align 4, !dbg !1117
  store i32 %31, ptr %6, align 4, !dbg !1118
  br label %97, !dbg !1118

32:                                               ; preds = %5
  %33 = load ptr, ptr %8, align 8, !dbg !1119
  %34 = getelementptr inbounds %struct.bpf_lru_list, ptr %33, i32 0, i32 0, !dbg !1121
  %35 = getelementptr inbounds [3 x %struct.list_head], ptr %34, i64 0, i64 1, !dbg !1119
  %36 = call i32 @list_empty(ptr noundef %35), !dbg !1122
  %37 = icmp ne i32 %36, 0, !dbg !1122
  br i1 %37, label %42, label %38, !dbg !1123

38:                                               ; preds = %32
  %39 = load ptr, ptr %8, align 8, !dbg !1124
  %40 = getelementptr inbounds %struct.bpf_lru_list, ptr %39, i32 0, i32 0, !dbg !1125
  %41 = getelementptr inbounds [3 x %struct.list_head], ptr %40, i64 0, i64 1, !dbg !1124
  store ptr %41, ptr %14, align 8, !dbg !1126
  br label %46, !dbg !1127

42:                                               ; preds = %32
  %43 = load ptr, ptr %8, align 8, !dbg !1128
  %44 = getelementptr inbounds %struct.bpf_lru_list, ptr %43, i32 0, i32 0, !dbg !1129
  %45 = getelementptr inbounds [3 x %struct.list_head], ptr %44, i64 0, i64 0, !dbg !1128
  store ptr %45, ptr %14, align 8, !dbg !1130
  br label %46

46:                                               ; preds = %42, %38
  call void @llvm.dbg.declare(metadata ptr %16, metadata !1131, metadata !DIExpression()), !dbg !1134
  %47 = load ptr, ptr %14, align 8, !dbg !1134
  %48 = getelementptr inbounds %struct.list_head, ptr %47, i32 0, i32 1, !dbg !1134
  %49 = load ptr, ptr %48, align 8, !dbg !1134
  store ptr %49, ptr %16, align 8, !dbg !1134
  br label %50, !dbg !1134

50:                                               ; preds = %46
  br label %51, !dbg !1135

51:                                               ; preds = %50
  %52 = load ptr, ptr %16, align 8, !dbg !1134
  %53 = getelementptr i8, ptr %52, i64 0, !dbg !1134
  store ptr %53, ptr %17, align 8, !dbg !1135
  %54 = load ptr, ptr %17, align 8, !dbg !1134
  store ptr %54, ptr %12, align 8, !dbg !1137
  call void @llvm.dbg.declare(metadata ptr %18, metadata !1138, metadata !DIExpression()), !dbg !1140
  %55 = load ptr, ptr %12, align 8, !dbg !1140
  %56 = getelementptr inbounds %struct.bpf_lru_node, ptr %55, i32 0, i32 0, !dbg !1140
  %57 = getelementptr inbounds %struct.list_head, ptr %56, i32 0, i32 1, !dbg !1140
  %58 = load ptr, ptr %57, align 8, !dbg !1140
  store ptr %58, ptr %18, align 8, !dbg !1140
  br label %59, !dbg !1140

59:                                               ; preds = %51
  br label %60, !dbg !1141

60:                                               ; preds = %59
  %61 = load ptr, ptr %18, align 8, !dbg !1140
  %62 = getelementptr i8, ptr %61, i64 0, !dbg !1140
  store ptr %62, ptr %19, align 8, !dbg !1141
  %63 = load ptr, ptr %19, align 8, !dbg !1140
  store ptr %63, ptr %13, align 8, !dbg !1137
  br label %64, !dbg !1137

64:                                               ; preds = %92, %60
  %65 = load ptr, ptr %12, align 8, !dbg !1143
  %66 = getelementptr inbounds %struct.bpf_lru_node, ptr %65, i32 0, i32 0, !dbg !1143
  %67 = load ptr, ptr %14, align 8, !dbg !1143
  %68 = icmp eq ptr %66, %67, !dbg !1143
  %69 = xor i1 %68, true, !dbg !1143
  br i1 %69, label %70, label %96, !dbg !1137

70:                                               ; preds = %64
  %71 = load ptr, ptr %7, align 8, !dbg !1145
  %72 = getelementptr inbounds %struct.bpf_lru, ptr %71, i32 0, i32 1, !dbg !1148
  %73 = load ptr, ptr %72, align 64, !dbg !1148
  %74 = load ptr, ptr %7, align 8, !dbg !1149
  %75 = getelementptr inbounds %struct.bpf_lru, ptr %74, i32 0, i32 2, !dbg !1150
  %76 = load ptr, ptr %75, align 8, !dbg !1150
  %77 = load ptr, ptr %12, align 8, !dbg !1151
  %78 = call zeroext i1 %73(ptr noundef %76, ptr noundef %77), !dbg !1145
  br i1 %78, label %79, label %84, !dbg !1152

79:                                               ; preds = %70
  %80 = load ptr, ptr %8, align 8, !dbg !1153
  %81 = load ptr, ptr %12, align 8, !dbg !1155
  %82 = load ptr, ptr %10, align 8, !dbg !1156
  %83 = load i32, ptr %11, align 4, !dbg !1157
  call void @__bpf_lru_node_move_to_free(ptr noundef %80, ptr noundef %81, ptr noundef %82, i32 noundef %83), !dbg !1158
  store i32 1, ptr %6, align 4, !dbg !1159
  br label %97, !dbg !1159

84:                                               ; preds = %70
  br label %85, !dbg !1160

85:                                               ; preds = %84
  %86 = load ptr, ptr %13, align 8, !dbg !1143
  store ptr %86, ptr %12, align 8, !dbg !1143
  call void @llvm.dbg.declare(metadata ptr %20, metadata !1161, metadata !DIExpression()), !dbg !1163
  %87 = load ptr, ptr %13, align 8, !dbg !1163
  %88 = getelementptr inbounds %struct.bpf_lru_node, ptr %87, i32 0, i32 0, !dbg !1163
  %89 = getelementptr inbounds %struct.list_head, ptr %88, i32 0, i32 1, !dbg !1163
  %90 = load ptr, ptr %89, align 8, !dbg !1163
  store ptr %90, ptr %20, align 8, !dbg !1163
  br label %91, !dbg !1163

91:                                               ; preds = %85
  br label %92, !dbg !1164

92:                                               ; preds = %91
  %93 = load ptr, ptr %20, align 8, !dbg !1163
  %94 = getelementptr i8, ptr %93, i64 0, !dbg !1163
  store ptr %94, ptr %21, align 8, !dbg !1164
  %95 = load ptr, ptr %21, align 8, !dbg !1163
  store ptr %95, ptr %13, align 8, !dbg !1143
  br label %64, !dbg !1143, !llvm.loop !1166

96:                                               ; preds = %64
  store i32 0, ptr %6, align 4, !dbg !1168
  br label %97, !dbg !1168

97:                                               ; preds = %96, %79, %30
  %98 = load i32, ptr %6, align 4, !dbg !1169
  ret i32 %98, !dbg !1169
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @__bpf_lru_node_move(ptr noundef %0, ptr noundef %1, i32 noundef %2) #0 !dbg !1170 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i64, align 8
  %9 = alloca i32, align 4
  %10 = alloca i64, align 8
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1173, metadata !DIExpression()), !dbg !1174
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !1175, metadata !DIExpression()), !dbg !1176
  store i32 %2, ptr %6, align 4
  call void @llvm.dbg.declare(metadata ptr %6, metadata !1177, metadata !DIExpression()), !dbg !1178
  call void @llvm.dbg.declare(metadata ptr %7, metadata !1179, metadata !DIExpression()), !dbg !1182
  %11 = load ptr, ptr %5, align 8, !dbg !1182
  %12 = getelementptr inbounds %struct.bpf_lru_node, ptr %11, i32 0, i32 2, !dbg !1182
  %13 = load i8, ptr %12, align 2, !dbg !1182
  %14 = zext i8 %13 to i32, !dbg !1182
  %15 = icmp sge i32 %14, 3, !dbg !1182
  %16 = xor i1 %15, true, !dbg !1182
  %17 = xor i1 %16, true, !dbg !1182
  %18 = zext i1 %17 to i32, !dbg !1182
  store i32 %18, ptr %7, align 4, !dbg !1182
  %19 = load i32, ptr %7, align 4, !dbg !1183
  %20 = icmp ne i32 %19, 0, !dbg !1183
  %21 = xor i1 %20, true, !dbg !1183
  %22 = xor i1 %21, true, !dbg !1183
  %23 = zext i1 %22 to i32, !dbg !1183
  %24 = sext i32 %23 to i64, !dbg !1183
  %25 = icmp ne i64 %24, 0, !dbg !1183
  br i1 %25, label %26, label %31, !dbg !1182

26:                                               ; preds = %3
  br label %27, !dbg !1183

27:                                               ; preds = %26
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_begin\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 225) #4, !dbg !1185, !srcloc !1188
  br label %28, !dbg !1189

28:                                               ; preds = %27
  call void asm sideeffect "1:\09.byte 0x0f, 0x0b\0A.pushsection __bug_table,\22aw\22\0A2:\09.long 1b - 2b\09# bug_entry::bug_addr\0A\09.long ${0:c} - 2b\09# bug_entry::file\0A\09.word ${1:c}\09# bug_entry::line\0A\09.word ${2:c}\09# bug_entry::flags\0A\09.org 2b+${3:c}\0A.popsection", "i,i,i,i,~{dirflag},~{fpsr},~{flags}"(ptr @.str, i32 104, i32 2307, i64 12) #4, !dbg !1190, !srcloc !1192
  br label %29, !dbg !1190

29:                                               ; preds = %28
  call void asm sideeffect "${0:c}:\0A\09.pushsection .discard.reachable\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 226) #4, !dbg !1193, !srcloc !1195
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_end\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 227) #4, !dbg !1196, !srcloc !1198
  br label %30, !dbg !1189

30:                                               ; preds = %29
  br label %31, !dbg !1189

31:                                               ; preds = %30, %3
  %32 = load i32, ptr %7, align 4, !dbg !1182
  %33 = icmp ne i32 %32, 0, !dbg !1182
  %34 = xor i1 %33, true, !dbg !1182
  %35 = xor i1 %34, true, !dbg !1182
  %36 = zext i1 %35 to i32, !dbg !1182
  %37 = sext i32 %36 to i64, !dbg !1182
  store i64 %37, ptr %8, align 8, !dbg !1183
  %38 = load i64, ptr %8, align 8, !dbg !1182
  %39 = icmp ne i64 %38, 0, !dbg !1199
  br i1 %39, label %67, label %40, !dbg !1200

40:                                               ; preds = %31
  call void @llvm.dbg.declare(metadata ptr %9, metadata !1201, metadata !DIExpression()), !dbg !1203
  %41 = load i32, ptr %6, align 4, !dbg !1203
  %42 = icmp uge i32 %41, 3, !dbg !1203
  %43 = xor i1 %42, true, !dbg !1203
  %44 = xor i1 %43, true, !dbg !1203
  %45 = zext i1 %44 to i32, !dbg !1203
  store i32 %45, ptr %9, align 4, !dbg !1203
  %46 = load i32, ptr %9, align 4, !dbg !1204
  %47 = icmp ne i32 %46, 0, !dbg !1204
  %48 = xor i1 %47, true, !dbg !1204
  %49 = xor i1 %48, true, !dbg !1204
  %50 = zext i1 %49 to i32, !dbg !1204
  %51 = sext i32 %50 to i64, !dbg !1204
  %52 = icmp ne i64 %51, 0, !dbg !1204
  br i1 %52, label %53, label %58, !dbg !1203

53:                                               ; preds = %40
  br label %54, !dbg !1204

54:                                               ; preds = %53
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_begin\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 228) #4, !dbg !1206, !srcloc !1209
  br label %55, !dbg !1210

55:                                               ; preds = %54
  call void asm sideeffect "1:\09.byte 0x0f, 0x0b\0A.pushsection __bug_table,\22aw\22\0A2:\09.long 1b - 2b\09# bug_entry::bug_addr\0A\09.long ${0:c} - 2b\09# bug_entry::file\0A\09.word ${1:c}\09# bug_entry::line\0A\09.word ${2:c}\09# bug_entry::flags\0A\09.org 2b+${3:c}\0A.popsection", "i,i,i,i,~{dirflag},~{fpsr},~{flags}"(ptr @.str, i32 105, i32 2307, i64 12) #4, !dbg !1211, !srcloc !1213
  br label %56, !dbg !1211

56:                                               ; preds = %55
  call void asm sideeffect "${0:c}:\0A\09.pushsection .discard.reachable\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 229) #4, !dbg !1214, !srcloc !1216
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_end\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 230) #4, !dbg !1217, !srcloc !1219
  br label %57, !dbg !1210

57:                                               ; preds = %56
  br label %58, !dbg !1210

58:                                               ; preds = %57, %40
  %59 = load i32, ptr %9, align 4, !dbg !1203
  %60 = icmp ne i32 %59, 0, !dbg !1203
  %61 = xor i1 %60, true, !dbg !1203
  %62 = xor i1 %61, true, !dbg !1203
  %63 = zext i1 %62 to i32, !dbg !1203
  %64 = sext i32 %63 to i64, !dbg !1203
  store i64 %64, ptr %10, align 8, !dbg !1204
  %65 = load i64, ptr %10, align 8, !dbg !1203
  %66 = icmp ne i64 %65, 0, !dbg !1220
  br i1 %66, label %67, label %68, !dbg !1221

67:                                               ; preds = %58, %31
  br label %112, !dbg !1222

68:                                               ; preds = %58
  %69 = load ptr, ptr %5, align 8, !dbg !1223
  %70 = getelementptr inbounds %struct.bpf_lru_node, ptr %69, i32 0, i32 2, !dbg !1225
  %71 = load i8, ptr %70, align 2, !dbg !1225
  %72 = zext i8 %71 to i32, !dbg !1223
  %73 = load i32, ptr %6, align 4, !dbg !1226
  %74 = icmp ne i32 %72, %73, !dbg !1227
  br i1 %74, label %75, label %87, !dbg !1228

75:                                               ; preds = %68
  %76 = load ptr, ptr %4, align 8, !dbg !1229
  %77 = load ptr, ptr %5, align 8, !dbg !1231
  %78 = getelementptr inbounds %struct.bpf_lru_node, ptr %77, i32 0, i32 2, !dbg !1232
  %79 = load i8, ptr %78, align 2, !dbg !1232
  %80 = zext i8 %79 to i32, !dbg !1231
  call void @bpf_lru_list_count_dec(ptr noundef %76, i32 noundef %80), !dbg !1233
  %81 = load ptr, ptr %4, align 8, !dbg !1234
  %82 = load i32, ptr %6, align 4, !dbg !1235
  call void @bpf_lru_list_count_inc(ptr noundef %81, i32 noundef %82), !dbg !1236
  %83 = load i32, ptr %6, align 4, !dbg !1237
  %84 = trunc i32 %83 to i8, !dbg !1237
  %85 = load ptr, ptr %5, align 8, !dbg !1238
  %86 = getelementptr inbounds %struct.bpf_lru_node, ptr %85, i32 0, i32 2, !dbg !1239
  store i8 %84, ptr %86, align 2, !dbg !1240
  br label %87, !dbg !1241

87:                                               ; preds = %75, %68
  %88 = load ptr, ptr %5, align 8, !dbg !1242
  %89 = getelementptr inbounds %struct.bpf_lru_node, ptr %88, i32 0, i32 3, !dbg !1243
  store i8 0, ptr %89, align 1, !dbg !1244
  %90 = load ptr, ptr %5, align 8, !dbg !1245
  %91 = getelementptr inbounds %struct.bpf_lru_node, ptr %90, i32 0, i32 0, !dbg !1247
  %92 = load ptr, ptr %4, align 8, !dbg !1248
  %93 = getelementptr inbounds %struct.bpf_lru_list, ptr %92, i32 0, i32 2, !dbg !1249
  %94 = load ptr, ptr %93, align 8, !dbg !1249
  %95 = icmp eq ptr %91, %94, !dbg !1250
  br i1 %95, label %96, label %104, !dbg !1251

96:                                               ; preds = %87
  %97 = load ptr, ptr %4, align 8, !dbg !1252
  %98 = getelementptr inbounds %struct.bpf_lru_list, ptr %97, i32 0, i32 2, !dbg !1253
  %99 = load ptr, ptr %98, align 8, !dbg !1253
  %100 = getelementptr inbounds %struct.list_head, ptr %99, i32 0, i32 1, !dbg !1254
  %101 = load ptr, ptr %100, align 8, !dbg !1254
  %102 = load ptr, ptr %4, align 8, !dbg !1255
  %103 = getelementptr inbounds %struct.bpf_lru_list, ptr %102, i32 0, i32 2, !dbg !1256
  store ptr %101, ptr %103, align 8, !dbg !1257
  br label %104, !dbg !1255

104:                                              ; preds = %96, %87
  %105 = load ptr, ptr %5, align 8, !dbg !1258
  %106 = getelementptr inbounds %struct.bpf_lru_node, ptr %105, i32 0, i32 0, !dbg !1259
  %107 = load ptr, ptr %4, align 8, !dbg !1260
  %108 = getelementptr inbounds %struct.bpf_lru_list, ptr %107, i32 0, i32 0, !dbg !1261
  %109 = load i32, ptr %6, align 4, !dbg !1262
  %110 = zext i32 %109 to i64, !dbg !1260
  %111 = getelementptr inbounds [3 x %struct.list_head], ptr %108, i64 0, i64 %110, !dbg !1260
  call void @list_move(ptr noundef %106, ptr noundef %111), !dbg !1263
  br label %112, !dbg !1264

112:                                              ; preds = %104, %67
  ret void, !dbg !1264
}

declare dso_local void @_raw_spin_unlock_irqrestore(ptr noundef, i64 noundef) #2 section ".spinlock.text"

; Function Attrs: noinline nounwind optnone uwtable
define internal zeroext i1 @bpf_lru_list_inactive_low(ptr noundef %0) #0 !dbg !1265 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !1270, metadata !DIExpression()), !dbg !1271
  %3 = load ptr, ptr %2, align 8, !dbg !1272
  %4 = getelementptr inbounds %struct.bpf_lru_list, ptr %3, i32 0, i32 1, !dbg !1273
  %5 = getelementptr inbounds [2 x i32], ptr %4, i64 0, i64 1, !dbg !1272
  %6 = load i32, ptr %5, align 4, !dbg !1272
  %7 = load ptr, ptr %2, align 8, !dbg !1274
  %8 = getelementptr inbounds %struct.bpf_lru_list, ptr %7, i32 0, i32 1, !dbg !1275
  %9 = getelementptr inbounds [2 x i32], ptr %8, i64 0, i64 0, !dbg !1274
  %10 = load i32, ptr %9, align 16, !dbg !1274
  %11 = icmp ult i32 %6, %10, !dbg !1276
  ret i1 %11, !dbg !1277
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @__bpf_lru_list_rotate_active(ptr noundef %0, ptr noundef %1) #0 !dbg !1278 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca i32, align 4
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  %12 = alloca ptr, align 8
  %13 = alloca ptr, align 8
  %14 = alloca ptr, align 8
  %15 = alloca ptr, align 8
  %16 = alloca ptr, align 8
  %17 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !1279, metadata !DIExpression()), !dbg !1280
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1281, metadata !DIExpression()), !dbg !1282
  call void @llvm.dbg.declare(metadata ptr %5, metadata !1283, metadata !DIExpression()), !dbg !1284
  %18 = load ptr, ptr %4, align 8, !dbg !1285
  %19 = getelementptr inbounds %struct.bpf_lru_list, ptr %18, i32 0, i32 0, !dbg !1286
  %20 = getelementptr inbounds [3 x %struct.list_head], ptr %19, i64 0, i64 0, !dbg !1285
  store ptr %20, ptr %5, align 8, !dbg !1284
  call void @llvm.dbg.declare(metadata ptr %6, metadata !1287, metadata !DIExpression()), !dbg !1288
  call void @llvm.dbg.declare(metadata ptr %7, metadata !1289, metadata !DIExpression()), !dbg !1290
  call void @llvm.dbg.declare(metadata ptr %8, metadata !1291, metadata !DIExpression()), !dbg !1292
  call void @llvm.dbg.declare(metadata ptr %9, metadata !1293, metadata !DIExpression()), !dbg !1294
  store i32 0, ptr %9, align 4, !dbg !1294
  call void @llvm.dbg.declare(metadata ptr %10, metadata !1295, metadata !DIExpression()), !dbg !1297
  %21 = load ptr, ptr %5, align 8, !dbg !1297
  %22 = getelementptr inbounds %struct.list_head, ptr %21, i32 0, i32 0, !dbg !1297
  %23 = load ptr, ptr %22, align 8, !dbg !1297
  store ptr %23, ptr %10, align 8, !dbg !1297
  br label %24, !dbg !1297

24:                                               ; preds = %2
  br label %25, !dbg !1298

25:                                               ; preds = %24
  %26 = load ptr, ptr %10, align 8, !dbg !1297
  %27 = getelementptr i8, ptr %26, i64 0, !dbg !1297
  store ptr %27, ptr %11, align 8, !dbg !1298
  %28 = load ptr, ptr %11, align 8, !dbg !1297
  store ptr %28, ptr %8, align 8, !dbg !1300
  call void @llvm.dbg.declare(metadata ptr %12, metadata !1301, metadata !DIExpression()), !dbg !1304
  %29 = load ptr, ptr %5, align 8, !dbg !1304
  %30 = getelementptr inbounds %struct.list_head, ptr %29, i32 0, i32 1, !dbg !1304
  %31 = load ptr, ptr %30, align 8, !dbg !1304
  store ptr %31, ptr %12, align 8, !dbg !1304
  br label %32, !dbg !1304

32:                                               ; preds = %25
  br label %33, !dbg !1305

33:                                               ; preds = %32
  %34 = load ptr, ptr %12, align 8, !dbg !1304
  %35 = getelementptr i8, ptr %34, i64 0, !dbg !1304
  store ptr %35, ptr %13, align 8, !dbg !1305
  %36 = load ptr, ptr %13, align 8, !dbg !1304
  store ptr %36, ptr %6, align 8, !dbg !1307
  call void @llvm.dbg.declare(metadata ptr %14, metadata !1308, metadata !DIExpression()), !dbg !1310
  %37 = load ptr, ptr %6, align 8, !dbg !1310
  %38 = getelementptr inbounds %struct.bpf_lru_node, ptr %37, i32 0, i32 0, !dbg !1310
  %39 = getelementptr inbounds %struct.list_head, ptr %38, i32 0, i32 1, !dbg !1310
  %40 = load ptr, ptr %39, align 8, !dbg !1310
  store ptr %40, ptr %14, align 8, !dbg !1310
  br label %41, !dbg !1310

41:                                               ; preds = %33
  br label %42, !dbg !1311

42:                                               ; preds = %41
  %43 = load ptr, ptr %14, align 8, !dbg !1310
  %44 = getelementptr i8, ptr %43, i64 0, !dbg !1310
  store ptr %44, ptr %15, align 8, !dbg !1311
  %45 = load ptr, ptr %15, align 8, !dbg !1310
  store ptr %45, ptr %7, align 8, !dbg !1307
  br label %46, !dbg !1307

46:                                               ; preds = %81, %42
  %47 = load ptr, ptr %6, align 8, !dbg !1313
  %48 = getelementptr inbounds %struct.bpf_lru_node, ptr %47, i32 0, i32 0, !dbg !1313
  %49 = load ptr, ptr %5, align 8, !dbg !1313
  %50 = icmp eq ptr %48, %49, !dbg !1313
  %51 = xor i1 %50, true, !dbg !1313
  br i1 %51, label %52, label %85, !dbg !1307

52:                                               ; preds = %46
  %53 = load ptr, ptr %6, align 8, !dbg !1315
  %54 = call zeroext i1 @bpf_lru_node_is_ref(ptr noundef %53), !dbg !1318
  br i1 %54, label %55, label %58, !dbg !1319

55:                                               ; preds = %52
  %56 = load ptr, ptr %4, align 8, !dbg !1320
  %57 = load ptr, ptr %6, align 8, !dbg !1321
  call void @__bpf_lru_node_move(ptr noundef %56, ptr noundef %57, i32 noundef 0), !dbg !1322
  br label %61, !dbg !1322

58:                                               ; preds = %52
  %59 = load ptr, ptr %4, align 8, !dbg !1323
  %60 = load ptr, ptr %6, align 8, !dbg !1324
  call void @__bpf_lru_node_move(ptr noundef %59, ptr noundef %60, i32 noundef 1), !dbg !1325
  br label %61

61:                                               ; preds = %58, %55
  %62 = load i32, ptr %9, align 4, !dbg !1326
  %63 = add i32 %62, 1, !dbg !1326
  store i32 %63, ptr %9, align 4, !dbg !1326
  %64 = load ptr, ptr %3, align 8, !dbg !1328
  %65 = getelementptr inbounds %struct.bpf_lru, ptr %64, i32 0, i32 4, !dbg !1329
  %66 = load i32, ptr %65, align 4, !dbg !1329
  %67 = icmp eq i32 %63, %66, !dbg !1330
  br i1 %67, label %72, label %68, !dbg !1331

68:                                               ; preds = %61
  %69 = load ptr, ptr %6, align 8, !dbg !1332
  %70 = load ptr, ptr %8, align 8, !dbg !1333
  %71 = icmp eq ptr %69, %70, !dbg !1334
  br i1 %71, label %72, label %73, !dbg !1335

72:                                               ; preds = %68, %61
  br label %85, !dbg !1336

73:                                               ; preds = %68
  br label %74, !dbg !1337

74:                                               ; preds = %73
  %75 = load ptr, ptr %7, align 8, !dbg !1313
  store ptr %75, ptr %6, align 8, !dbg !1313
  call void @llvm.dbg.declare(metadata ptr %16, metadata !1338, metadata !DIExpression()), !dbg !1340
  %76 = load ptr, ptr %7, align 8, !dbg !1340
  %77 = getelementptr inbounds %struct.bpf_lru_node, ptr %76, i32 0, i32 0, !dbg !1340
  %78 = getelementptr inbounds %struct.list_head, ptr %77, i32 0, i32 1, !dbg !1340
  %79 = load ptr, ptr %78, align 8, !dbg !1340
  store ptr %79, ptr %16, align 8, !dbg !1340
  br label %80, !dbg !1340

80:                                               ; preds = %74
  br label %81, !dbg !1341

81:                                               ; preds = %80
  %82 = load ptr, ptr %16, align 8, !dbg !1340
  %83 = getelementptr i8, ptr %82, i64 0, !dbg !1340
  store ptr %83, ptr %17, align 8, !dbg !1341
  %84 = load ptr, ptr %17, align 8, !dbg !1340
  store ptr %84, ptr %7, align 8, !dbg !1313
  br label %46, !dbg !1313, !llvm.loop !1343

85:                                               ; preds = %72, %46
  ret void, !dbg !1345
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @__bpf_lru_list_rotate_inactive(ptr noundef %0, ptr noundef %1) #0 !dbg !1346 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca i32, align 4
  %11 = alloca ptr, align 8
  %12 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !1347, metadata !DIExpression()), !dbg !1348
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1349, metadata !DIExpression()), !dbg !1350
  call void @llvm.dbg.declare(metadata ptr %5, metadata !1351, metadata !DIExpression()), !dbg !1352
  %13 = load ptr, ptr %4, align 8, !dbg !1353
  %14 = getelementptr inbounds %struct.bpf_lru_list, ptr %13, i32 0, i32 0, !dbg !1354
  %15 = getelementptr inbounds [3 x %struct.list_head], ptr %14, i64 0, i64 1, !dbg !1353
  store ptr %15, ptr %5, align 8, !dbg !1352
  call void @llvm.dbg.declare(metadata ptr %6, metadata !1355, metadata !DIExpression()), !dbg !1356
  call void @llvm.dbg.declare(metadata ptr %7, metadata !1357, metadata !DIExpression()), !dbg !1358
  call void @llvm.dbg.declare(metadata ptr %8, metadata !1359, metadata !DIExpression()), !dbg !1360
  %16 = load ptr, ptr %5, align 8, !dbg !1361
  store ptr %16, ptr %8, align 8, !dbg !1360
  call void @llvm.dbg.declare(metadata ptr %9, metadata !1362, metadata !DIExpression()), !dbg !1363
  call void @llvm.dbg.declare(metadata ptr %10, metadata !1364, metadata !DIExpression()), !dbg !1365
  store i32 0, ptr %10, align 4, !dbg !1365
  %17 = load ptr, ptr %5, align 8, !dbg !1366
  %18 = call i32 @list_empty(ptr noundef %17), !dbg !1368
  %19 = icmp ne i32 %18, 0, !dbg !1368
  br i1 %19, label %20, label %21, !dbg !1369

20:                                               ; preds = %2
  br label %80, !dbg !1370

21:                                               ; preds = %2
  %22 = load ptr, ptr %4, align 8, !dbg !1371
  %23 = getelementptr inbounds %struct.bpf_lru_list, ptr %22, i32 0, i32 2, !dbg !1372
  %24 = load ptr, ptr %23, align 8, !dbg !1372
  %25 = getelementptr inbounds %struct.list_head, ptr %24, i32 0, i32 0, !dbg !1373
  %26 = load ptr, ptr %25, align 8, !dbg !1373
  store ptr %26, ptr %7, align 8, !dbg !1374
  %27 = load ptr, ptr %7, align 8, !dbg !1375
  %28 = load ptr, ptr %5, align 8, !dbg !1377
  %29 = icmp eq ptr %27, %28, !dbg !1378
  br i1 %29, label %30, label %34, !dbg !1379

30:                                               ; preds = %21
  %31 = load ptr, ptr %7, align 8, !dbg !1380
  %32 = getelementptr inbounds %struct.list_head, ptr %31, i32 0, i32 0, !dbg !1381
  %33 = load ptr, ptr %32, align 8, !dbg !1381
  store ptr %33, ptr %7, align 8, !dbg !1382
  br label %34, !dbg !1383

34:                                               ; preds = %30, %21
  %35 = load ptr, ptr %4, align 8, !dbg !1384
  %36 = getelementptr inbounds %struct.bpf_lru_list, ptr %35, i32 0, i32 2, !dbg !1385
  %37 = load ptr, ptr %36, align 8, !dbg !1385
  store ptr %37, ptr %6, align 8, !dbg !1386
  br label %38, !dbg !1387

38:                                               ; preds = %72, %48, %34
  %39 = load i32, ptr %10, align 4, !dbg !1388
  %40 = load ptr, ptr %3, align 8, !dbg !1389
  %41 = getelementptr inbounds %struct.bpf_lru, ptr %40, i32 0, i32 4, !dbg !1390
  %42 = load i32, ptr %41, align 4, !dbg !1390
  %43 = icmp ult i32 %39, %42, !dbg !1391
  br i1 %43, label %44, label %76, !dbg !1387

44:                                               ; preds = %38
  %45 = load ptr, ptr %6, align 8, !dbg !1392
  %46 = load ptr, ptr %5, align 8, !dbg !1395
  %47 = icmp eq ptr %45, %46, !dbg !1396
  br i1 %47, label %48, label %52, !dbg !1397

48:                                               ; preds = %44
  %49 = load ptr, ptr %6, align 8, !dbg !1398
  %50 = getelementptr inbounds %struct.list_head, ptr %49, i32 0, i32 1, !dbg !1400
  %51 = load ptr, ptr %50, align 8, !dbg !1400
  store ptr %51, ptr %6, align 8, !dbg !1401
  br label %38, !dbg !1402, !llvm.loop !1403

52:                                               ; preds = %44
  call void @llvm.dbg.declare(metadata ptr %11, metadata !1405, metadata !DIExpression()), !dbg !1407
  %53 = load ptr, ptr %6, align 8, !dbg !1407
  store ptr %53, ptr %11, align 8, !dbg !1407
  br label %54, !dbg !1407

54:                                               ; preds = %52
  br label %55, !dbg !1408

55:                                               ; preds = %54
  %56 = load ptr, ptr %11, align 8, !dbg !1407
  %57 = getelementptr i8, ptr %56, i64 0, !dbg !1407
  store ptr %57, ptr %12, align 8, !dbg !1408
  %58 = load ptr, ptr %12, align 8, !dbg !1407
  store ptr %58, ptr %9, align 8, !dbg !1410
  %59 = load ptr, ptr %6, align 8, !dbg !1411
  %60 = getelementptr inbounds %struct.list_head, ptr %59, i32 0, i32 1, !dbg !1412
  %61 = load ptr, ptr %60, align 8, !dbg !1412
  store ptr %61, ptr %8, align 8, !dbg !1413
  %62 = load ptr, ptr %9, align 8, !dbg !1414
  %63 = call zeroext i1 @bpf_lru_node_is_ref(ptr noundef %62), !dbg !1416
  br i1 %63, label %64, label %67, !dbg !1417

64:                                               ; preds = %55
  %65 = load ptr, ptr %4, align 8, !dbg !1418
  %66 = load ptr, ptr %9, align 8, !dbg !1419
  call void @__bpf_lru_node_move(ptr noundef %65, ptr noundef %66, i32 noundef 0), !dbg !1420
  br label %67, !dbg !1420

67:                                               ; preds = %64, %55
  %68 = load ptr, ptr %6, align 8, !dbg !1421
  %69 = load ptr, ptr %7, align 8, !dbg !1423
  %70 = icmp eq ptr %68, %69, !dbg !1424
  br i1 %70, label %71, label %72, !dbg !1425

71:                                               ; preds = %67
  br label %76, !dbg !1426

72:                                               ; preds = %67
  %73 = load ptr, ptr %8, align 8, !dbg !1427
  store ptr %73, ptr %6, align 8, !dbg !1428
  %74 = load i32, ptr %10, align 4, !dbg !1429
  %75 = add i32 %74, 1, !dbg !1429
  store i32 %75, ptr %10, align 4, !dbg !1429
  br label %38, !dbg !1387, !llvm.loop !1403

76:                                               ; preds = %71, %38
  %77 = load ptr, ptr %8, align 8, !dbg !1430
  %78 = load ptr, ptr %4, align 8, !dbg !1431
  %79 = getelementptr inbounds %struct.bpf_lru_list, ptr %78, i32 0, i32 2, !dbg !1432
  store ptr %77, ptr %79, align 8, !dbg !1433
  br label %80, !dbg !1434

80:                                               ; preds = %76, %20
  ret void, !dbg !1434
}

; Function Attrs: noinline nounwind optnone uwtable
define internal zeroext i1 @bpf_lru_node_is_ref(ptr noundef %0) #0 !dbg !1435 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !1440, metadata !DIExpression()), !dbg !1441
  %3 = load ptr, ptr %2, align 8, !dbg !1442
  %4 = getelementptr inbounds %struct.bpf_lru_node, ptr %3, i32 0, i32 3, !dbg !1443
  %5 = load i8, ptr %4, align 1, !dbg !1443
  %6 = icmp ne i8 %5, 0, !dbg !1442
  ret i1 %6, !dbg !1444
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @__bpf_lru_list_shrink_inactive(ptr noundef %0, ptr noundef %1, i32 noundef %2, ptr noundef %3, i32 noundef %4) #0 !dbg !1445 {
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i32, align 4
  %9 = alloca ptr, align 8
  %10 = alloca i32, align 4
  %11 = alloca ptr, align 8
  %12 = alloca ptr, align 8
  %13 = alloca ptr, align 8
  %14 = alloca i32, align 4
  %15 = alloca i32, align 4
  %16 = alloca ptr, align 8
  %17 = alloca ptr, align 8
  %18 = alloca ptr, align 8
  %19 = alloca ptr, align 8
  %20 = alloca ptr, align 8
  %21 = alloca ptr, align 8
  store ptr %0, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !1446, metadata !DIExpression()), !dbg !1447
  store ptr %1, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !1448, metadata !DIExpression()), !dbg !1449
  store i32 %2, ptr %8, align 4
  call void @llvm.dbg.declare(metadata ptr %8, metadata !1450, metadata !DIExpression()), !dbg !1451
  store ptr %3, ptr %9, align 8
  call void @llvm.dbg.declare(metadata ptr %9, metadata !1452, metadata !DIExpression()), !dbg !1453
  store i32 %4, ptr %10, align 4
  call void @llvm.dbg.declare(metadata ptr %10, metadata !1454, metadata !DIExpression()), !dbg !1455
  call void @llvm.dbg.declare(metadata ptr %11, metadata !1456, metadata !DIExpression()), !dbg !1457
  %22 = load ptr, ptr %7, align 8, !dbg !1458
  %23 = getelementptr inbounds %struct.bpf_lru_list, ptr %22, i32 0, i32 0, !dbg !1459
  %24 = getelementptr inbounds [3 x %struct.list_head], ptr %23, i64 0, i64 1, !dbg !1458
  store ptr %24, ptr %11, align 8, !dbg !1457
  call void @llvm.dbg.declare(metadata ptr %12, metadata !1460, metadata !DIExpression()), !dbg !1461
  call void @llvm.dbg.declare(metadata ptr %13, metadata !1462, metadata !DIExpression()), !dbg !1463
  call void @llvm.dbg.declare(metadata ptr %14, metadata !1464, metadata !DIExpression()), !dbg !1465
  store i32 0, ptr %14, align 4, !dbg !1465
  call void @llvm.dbg.declare(metadata ptr %15, metadata !1466, metadata !DIExpression()), !dbg !1467
  store i32 0, ptr %15, align 4, !dbg !1467
  call void @llvm.dbg.declare(metadata ptr %16, metadata !1468, metadata !DIExpression()), !dbg !1471
  %25 = load ptr, ptr %11, align 8, !dbg !1471
  %26 = getelementptr inbounds %struct.list_head, ptr %25, i32 0, i32 1, !dbg !1471
  %27 = load ptr, ptr %26, align 8, !dbg !1471
  store ptr %27, ptr %16, align 8, !dbg !1471
  br label %28, !dbg !1471

28:                                               ; preds = %5
  br label %29, !dbg !1472

29:                                               ; preds = %28
  %30 = load ptr, ptr %16, align 8, !dbg !1471
  %31 = getelementptr i8, ptr %30, i64 0, !dbg !1471
  store ptr %31, ptr %17, align 8, !dbg !1472
  %32 = load ptr, ptr %17, align 8, !dbg !1471
  store ptr %32, ptr %12, align 8, !dbg !1474
  call void @llvm.dbg.declare(metadata ptr %18, metadata !1475, metadata !DIExpression()), !dbg !1477
  %33 = load ptr, ptr %12, align 8, !dbg !1477
  %34 = getelementptr inbounds %struct.bpf_lru_node, ptr %33, i32 0, i32 0, !dbg !1477
  %35 = getelementptr inbounds %struct.list_head, ptr %34, i32 0, i32 1, !dbg !1477
  %36 = load ptr, ptr %35, align 8, !dbg !1477
  store ptr %36, ptr %18, align 8, !dbg !1477
  br label %37, !dbg !1477

37:                                               ; preds = %29
  br label %38, !dbg !1478

38:                                               ; preds = %37
  %39 = load ptr, ptr %18, align 8, !dbg !1477
  %40 = getelementptr i8, ptr %39, i64 0, !dbg !1477
  store ptr %40, ptr %19, align 8, !dbg !1478
  %41 = load ptr, ptr %19, align 8, !dbg !1477
  store ptr %41, ptr %13, align 8, !dbg !1474
  br label %42, !dbg !1474

42:                                               ; preds = %91, %38
  %43 = load ptr, ptr %12, align 8, !dbg !1480
  %44 = getelementptr inbounds %struct.bpf_lru_node, ptr %43, i32 0, i32 0, !dbg !1480
  %45 = load ptr, ptr %11, align 8, !dbg !1480
  %46 = icmp eq ptr %44, %45, !dbg !1480
  %47 = xor i1 %46, true, !dbg !1480
  br i1 %47, label %48, label %95, !dbg !1474

48:                                               ; preds = %42
  %49 = load ptr, ptr %12, align 8, !dbg !1482
  %50 = call zeroext i1 @bpf_lru_node_is_ref(ptr noundef %49), !dbg !1485
  br i1 %50, label %51, label %54, !dbg !1486

51:                                               ; preds = %48
  %52 = load ptr, ptr %7, align 8, !dbg !1487
  %53 = load ptr, ptr %12, align 8, !dbg !1489
  call void @__bpf_lru_node_move(ptr noundef %52, ptr noundef %53, i32 noundef 0), !dbg !1490
  br label %75, !dbg !1491

54:                                               ; preds = %48
  %55 = load ptr, ptr %6, align 8, !dbg !1492
  %56 = getelementptr inbounds %struct.bpf_lru, ptr %55, i32 0, i32 1, !dbg !1494
  %57 = load ptr, ptr %56, align 64, !dbg !1494
  %58 = load ptr, ptr %6, align 8, !dbg !1495
  %59 = getelementptr inbounds %struct.bpf_lru, ptr %58, i32 0, i32 2, !dbg !1496
  %60 = load ptr, ptr %59, align 8, !dbg !1496
  %61 = load ptr, ptr %12, align 8, !dbg !1497
  %62 = call zeroext i1 %57(ptr noundef %60, ptr noundef %61), !dbg !1492
  br i1 %62, label %63, label %74, !dbg !1498

63:                                               ; preds = %54
  %64 = load ptr, ptr %7, align 8, !dbg !1499
  %65 = load ptr, ptr %12, align 8, !dbg !1501
  %66 = load ptr, ptr %9, align 8, !dbg !1502
  %67 = load i32, ptr %10, align 4, !dbg !1503
  call void @__bpf_lru_node_move_to_free(ptr noundef %64, ptr noundef %65, ptr noundef %66, i32 noundef %67), !dbg !1504
  %68 = load i32, ptr %14, align 4, !dbg !1505
  %69 = add i32 %68, 1, !dbg !1505
  store i32 %69, ptr %14, align 4, !dbg !1505
  %70 = load i32, ptr %8, align 4, !dbg !1507
  %71 = icmp eq i32 %69, %70, !dbg !1508
  br i1 %71, label %72, label %73, !dbg !1509

72:                                               ; preds = %63
  br label %95, !dbg !1510

73:                                               ; preds = %63
  br label %74, !dbg !1511

74:                                               ; preds = %73, %54
  br label %75

75:                                               ; preds = %74, %51
  %76 = load i32, ptr %15, align 4, !dbg !1512
  %77 = add i32 %76, 1, !dbg !1512
  store i32 %77, ptr %15, align 4, !dbg !1512
  %78 = load ptr, ptr %6, align 8, !dbg !1514
  %79 = getelementptr inbounds %struct.bpf_lru, ptr %78, i32 0, i32 4, !dbg !1515
  %80 = load i32, ptr %79, align 4, !dbg !1515
  %81 = icmp eq i32 %77, %80, !dbg !1516
  br i1 %81, label %82, label %83, !dbg !1517

82:                                               ; preds = %75
  br label %95, !dbg !1518

83:                                               ; preds = %75
  br label %84, !dbg !1519

84:                                               ; preds = %83
  %85 = load ptr, ptr %13, align 8, !dbg !1480
  store ptr %85, ptr %12, align 8, !dbg !1480
  call void @llvm.dbg.declare(metadata ptr %20, metadata !1520, metadata !DIExpression()), !dbg !1522
  %86 = load ptr, ptr %13, align 8, !dbg !1522
  %87 = getelementptr inbounds %struct.bpf_lru_node, ptr %86, i32 0, i32 0, !dbg !1522
  %88 = getelementptr inbounds %struct.list_head, ptr %87, i32 0, i32 1, !dbg !1522
  %89 = load ptr, ptr %88, align 8, !dbg !1522
  store ptr %89, ptr %20, align 8, !dbg !1522
  br label %90, !dbg !1522

90:                                               ; preds = %84
  br label %91, !dbg !1523

91:                                               ; preds = %90
  %92 = load ptr, ptr %20, align 8, !dbg !1522
  %93 = getelementptr i8, ptr %92, i64 0, !dbg !1522
  store ptr %93, ptr %21, align 8, !dbg !1523
  %94 = load ptr, ptr %21, align 8, !dbg !1522
  store ptr %94, ptr %13, align 8, !dbg !1480
  br label %42, !dbg !1480, !llvm.loop !1525

95:                                               ; preds = %82, %72, %42
  %96 = load i32, ptr %14, align 4, !dbg !1527
  ret i32 %96, !dbg !1528
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @__bpf_lru_node_move_to_free(ptr noundef %0, ptr noundef %1, ptr noundef %2, i32 noundef %3) #0 !dbg !1529 {
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i64, align 8
  store ptr %0, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !1532, metadata !DIExpression()), !dbg !1533
  store ptr %1, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !1534, metadata !DIExpression()), !dbg !1535
  store ptr %2, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !1536, metadata !DIExpression()), !dbg !1537
  store i32 %3, ptr %8, align 4
  call void @llvm.dbg.declare(metadata ptr %8, metadata !1538, metadata !DIExpression()), !dbg !1539
  call void @llvm.dbg.declare(metadata ptr %9, metadata !1540, metadata !DIExpression()), !dbg !1543
  %11 = load ptr, ptr %6, align 8, !dbg !1543
  %12 = getelementptr inbounds %struct.bpf_lru_node, ptr %11, i32 0, i32 2, !dbg !1543
  %13 = load i8, ptr %12, align 2, !dbg !1543
  %14 = zext i8 %13 to i32, !dbg !1543
  %15 = icmp sge i32 %14, 3, !dbg !1543
  %16 = xor i1 %15, true, !dbg !1543
  %17 = xor i1 %16, true, !dbg !1543
  %18 = zext i1 %17 to i32, !dbg !1543
  store i32 %18, ptr %9, align 4, !dbg !1543
  %19 = load i32, ptr %9, align 4, !dbg !1544
  %20 = icmp ne i32 %19, 0, !dbg !1544
  %21 = xor i1 %20, true, !dbg !1544
  %22 = xor i1 %21, true, !dbg !1544
  %23 = zext i1 %22 to i32, !dbg !1544
  %24 = sext i32 %23 to i64, !dbg !1544
  %25 = icmp ne i64 %24, 0, !dbg !1544
  br i1 %25, label %26, label %31, !dbg !1543

26:                                               ; preds = %4
  br label %27, !dbg !1544

27:                                               ; preds = %26
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_begin\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 216) #4, !dbg !1546, !srcloc !1549
  br label %28, !dbg !1550

28:                                               ; preds = %27
  call void asm sideeffect "1:\09.byte 0x0f, 0x0b\0A.pushsection __bug_table,\22aw\22\0A2:\09.long 1b - 2b\09# bug_entry::bug_addr\0A\09.long ${0:c} - 2b\09# bug_entry::file\0A\09.word ${1:c}\09# bug_entry::line\0A\09.word ${2:c}\09# bug_entry::flags\0A\09.org 2b+${3:c}\0A.popsection", "i,i,i,i,~{dirflag},~{fpsr},~{flags}"(ptr @.str, i32 66, i32 2307, i64 12) #4, !dbg !1551, !srcloc !1553
  br label %29, !dbg !1551

29:                                               ; preds = %28
  call void asm sideeffect "${0:c}:\0A\09.pushsection .discard.reachable\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 217) #4, !dbg !1554, !srcloc !1556
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_end\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 218) #4, !dbg !1557, !srcloc !1559
  br label %30, !dbg !1550

30:                                               ; preds = %29
  br label %31, !dbg !1550

31:                                               ; preds = %30, %4
  %32 = load i32, ptr %9, align 4, !dbg !1543
  %33 = icmp ne i32 %32, 0, !dbg !1543
  %34 = xor i1 %33, true, !dbg !1543
  %35 = xor i1 %34, true, !dbg !1543
  %36 = zext i1 %35 to i32, !dbg !1543
  %37 = sext i32 %36 to i64, !dbg !1543
  store i64 %37, ptr %10, align 8, !dbg !1544
  %38 = load i64, ptr %10, align 8, !dbg !1543
  %39 = icmp ne i64 %38, 0, !dbg !1560
  br i1 %39, label %40, label %41, !dbg !1561

40:                                               ; preds = %31
  br label %69, !dbg !1562

41:                                               ; preds = %31
  %42 = load ptr, ptr %6, align 8, !dbg !1563
  %43 = getelementptr inbounds %struct.bpf_lru_node, ptr %42, i32 0, i32 0, !dbg !1565
  %44 = load ptr, ptr %5, align 8, !dbg !1566
  %45 = getelementptr inbounds %struct.bpf_lru_list, ptr %44, i32 0, i32 2, !dbg !1567
  %46 = load ptr, ptr %45, align 8, !dbg !1567
  %47 = icmp eq ptr %43, %46, !dbg !1568
  br i1 %47, label %48, label %56, !dbg !1569

48:                                               ; preds = %41
  %49 = load ptr, ptr %5, align 8, !dbg !1570
  %50 = getelementptr inbounds %struct.bpf_lru_list, ptr %49, i32 0, i32 2, !dbg !1571
  %51 = load ptr, ptr %50, align 8, !dbg !1571
  %52 = getelementptr inbounds %struct.list_head, ptr %51, i32 0, i32 1, !dbg !1572
  %53 = load ptr, ptr %52, align 8, !dbg !1572
  %54 = load ptr, ptr %5, align 8, !dbg !1573
  %55 = getelementptr inbounds %struct.bpf_lru_list, ptr %54, i32 0, i32 2, !dbg !1574
  store ptr %53, ptr %55, align 8, !dbg !1575
  br label %56, !dbg !1573

56:                                               ; preds = %48, %41
  %57 = load ptr, ptr %5, align 8, !dbg !1576
  %58 = load ptr, ptr %6, align 8, !dbg !1577
  %59 = getelementptr inbounds %struct.bpf_lru_node, ptr %58, i32 0, i32 2, !dbg !1578
  %60 = load i8, ptr %59, align 2, !dbg !1578
  %61 = zext i8 %60 to i32, !dbg !1577
  call void @bpf_lru_list_count_dec(ptr noundef %57, i32 noundef %61), !dbg !1579
  %62 = load i32, ptr %8, align 4, !dbg !1580
  %63 = trunc i32 %62 to i8, !dbg !1580
  %64 = load ptr, ptr %6, align 8, !dbg !1581
  %65 = getelementptr inbounds %struct.bpf_lru_node, ptr %64, i32 0, i32 2, !dbg !1582
  store i8 %63, ptr %65, align 2, !dbg !1583
  %66 = load ptr, ptr %6, align 8, !dbg !1584
  %67 = getelementptr inbounds %struct.bpf_lru_node, ptr %66, i32 0, i32 0, !dbg !1585
  %68 = load ptr, ptr %7, align 8, !dbg !1586
  call void @list_move(ptr noundef %67, ptr noundef %68), !dbg !1587
  br label %69, !dbg !1588

69:                                               ; preds = %56, %40
  ret void, !dbg !1588
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bpf_lru_list_count_dec(ptr noundef %0, i32 noundef %1) #0 !dbg !1589 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !1592, metadata !DIExpression()), !dbg !1593
  store i32 %1, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1594, metadata !DIExpression()), !dbg !1595
  %5 = load i32, ptr %4, align 4, !dbg !1596
  %6 = icmp ult i32 %5, 2, !dbg !1598
  br i1 %6, label %7, label %15, !dbg !1599

7:                                                ; preds = %2
  %8 = load ptr, ptr %3, align 8, !dbg !1600
  %9 = getelementptr inbounds %struct.bpf_lru_list, ptr %8, i32 0, i32 1, !dbg !1601
  %10 = load i32, ptr %4, align 4, !dbg !1602
  %11 = zext i32 %10 to i64, !dbg !1600
  %12 = getelementptr inbounds [2 x i32], ptr %9, i64 0, i64 %11, !dbg !1600
  %13 = load i32, ptr %12, align 4, !dbg !1603
  %14 = add i32 %13, -1, !dbg !1603
  store i32 %14, ptr %12, align 4, !dbg !1603
  br label %15, !dbg !1600

15:                                               ; preds = %7, %2
  ret void, !dbg !1604
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @list_move(ptr noundef %0, ptr noundef %1) #0 !dbg !1605 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !1608, metadata !DIExpression()), !dbg !1609
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1610, metadata !DIExpression()), !dbg !1611
  %5 = load ptr, ptr %3, align 8, !dbg !1612
  call void @__list_del_entry(ptr noundef %5), !dbg !1613
  %6 = load ptr, ptr %3, align 8, !dbg !1614
  %7 = load ptr, ptr %4, align 8, !dbg !1615
  call void @list_add(ptr noundef %6, ptr noundef %7), !dbg !1616
  ret void, !dbg !1617
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @__list_del_entry(ptr noundef %0) #0 !dbg !1618 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !1621, metadata !DIExpression()), !dbg !1622
  %3 = load ptr, ptr %2, align 8, !dbg !1623
  %4 = call zeroext i1 @__list_del_entry_valid(ptr noundef %3), !dbg !1625
  br i1 %4, label %6, label %5, !dbg !1626

5:                                                ; preds = %1
  br label %13, !dbg !1627

6:                                                ; preds = %1
  %7 = load ptr, ptr %2, align 8, !dbg !1628
  %8 = getelementptr inbounds %struct.list_head, ptr %7, i32 0, i32 1, !dbg !1629
  %9 = load ptr, ptr %8, align 8, !dbg !1629
  %10 = load ptr, ptr %2, align 8, !dbg !1630
  %11 = getelementptr inbounds %struct.list_head, ptr %10, i32 0, i32 0, !dbg !1631
  %12 = load ptr, ptr %11, align 8, !dbg !1631
  call void @__list_del(ptr noundef %9, ptr noundef %12), !dbg !1632
  br label %13, !dbg !1633

13:                                               ; preds = %6, %5
  ret void, !dbg !1633
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @list_add(ptr noundef %0, ptr noundef %1) #0 !dbg !1634 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !1635, metadata !DIExpression()), !dbg !1636
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1637, metadata !DIExpression()), !dbg !1638
  %5 = load ptr, ptr %3, align 8, !dbg !1639
  %6 = load ptr, ptr %4, align 8, !dbg !1640
  %7 = load ptr, ptr %4, align 8, !dbg !1641
  %8 = getelementptr inbounds %struct.list_head, ptr %7, i32 0, i32 0, !dbg !1642
  %9 = load ptr, ptr %8, align 8, !dbg !1642
  call void @__list_add(ptr noundef %5, ptr noundef %6, ptr noundef %9), !dbg !1643
  ret void, !dbg !1644
}

declare dso_local zeroext i1 @__list_del_entry_valid(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal void @__list_del(ptr noundef %0, ptr noundef %1) #0 !dbg !1645 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !1646, metadata !DIExpression()), !dbg !1647
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1648, metadata !DIExpression()), !dbg !1649
  %5 = load ptr, ptr %3, align 8, !dbg !1650
  %6 = load ptr, ptr %4, align 8, !dbg !1651
  %7 = getelementptr inbounds %struct.list_head, ptr %6, i32 0, i32 1, !dbg !1652
  store ptr %5, ptr %7, align 8, !dbg !1653
  br label %8, !dbg !1654

8:                                                ; preds = %2
  br label %9, !dbg !1655

9:                                                ; preds = %8
  br label %10, !dbg !1657

10:                                               ; preds = %9
  br label %11, !dbg !1655

11:                                               ; preds = %10
  %12 = load ptr, ptr %4, align 8, !dbg !1659
  %13 = load ptr, ptr %3, align 8, !dbg !1659
  %14 = getelementptr inbounds %struct.list_head, ptr %13, i32 0, i32 0, !dbg !1659
  store volatile ptr %12, ptr %14, align 8, !dbg !1659
  br label %15, !dbg !1659

15:                                               ; preds = %11
  br label %16, !dbg !1655

16:                                               ; preds = %15
  ret void, !dbg !1661
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @__list_add(ptr noundef %0, ptr noundef %1, ptr noundef %2) #0 !dbg !1662 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1665, metadata !DIExpression()), !dbg !1666
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !1667, metadata !DIExpression()), !dbg !1668
  store ptr %2, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !1669, metadata !DIExpression()), !dbg !1670
  %7 = load ptr, ptr %4, align 8, !dbg !1671
  %8 = load ptr, ptr %5, align 8, !dbg !1673
  %9 = load ptr, ptr %6, align 8, !dbg !1674
  %10 = call zeroext i1 @__list_add_valid(ptr noundef %7, ptr noundef %8, ptr noundef %9), !dbg !1675
  br i1 %10, label %12, label %11, !dbg !1676

11:                                               ; preds = %3
  br label %30, !dbg !1677

12:                                               ; preds = %3
  %13 = load ptr, ptr %4, align 8, !dbg !1678
  %14 = load ptr, ptr %6, align 8, !dbg !1679
  %15 = getelementptr inbounds %struct.list_head, ptr %14, i32 0, i32 1, !dbg !1680
  store ptr %13, ptr %15, align 8, !dbg !1681
  %16 = load ptr, ptr %6, align 8, !dbg !1682
  %17 = load ptr, ptr %4, align 8, !dbg !1683
  %18 = getelementptr inbounds %struct.list_head, ptr %17, i32 0, i32 0, !dbg !1684
  store ptr %16, ptr %18, align 8, !dbg !1685
  %19 = load ptr, ptr %5, align 8, !dbg !1686
  %20 = load ptr, ptr %4, align 8, !dbg !1687
  %21 = getelementptr inbounds %struct.list_head, ptr %20, i32 0, i32 1, !dbg !1688
  store ptr %19, ptr %21, align 8, !dbg !1689
  br label %22, !dbg !1690

22:                                               ; preds = %12
  br label %23, !dbg !1691

23:                                               ; preds = %22
  br label %24, !dbg !1693

24:                                               ; preds = %23
  br label %25, !dbg !1691

25:                                               ; preds = %24
  %26 = load ptr, ptr %4, align 8, !dbg !1695
  %27 = load ptr, ptr %5, align 8, !dbg !1695
  %28 = getelementptr inbounds %struct.list_head, ptr %27, i32 0, i32 0, !dbg !1695
  store volatile ptr %26, ptr %28, align 8, !dbg !1695
  br label %29, !dbg !1695

29:                                               ; preds = %25
  br label %30, !dbg !1691

30:                                               ; preds = %11, %29
  ret void, !dbg !1697
}

declare dso_local zeroext i1 @__list_add_valid(ptr noundef, ptr noundef, ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bpf_lru_list_count_inc(ptr noundef %0, i32 noundef %1) #0 !dbg !1698 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !1699, metadata !DIExpression()), !dbg !1700
  store i32 %1, ptr %4, align 4
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1701, metadata !DIExpression()), !dbg !1702
  %5 = load i32, ptr %4, align 4, !dbg !1703
  %6 = icmp ult i32 %5, 2, !dbg !1705
  br i1 %6, label %7, label %15, !dbg !1706

7:                                                ; preds = %2
  %8 = load ptr, ptr %3, align 8, !dbg !1707
  %9 = getelementptr inbounds %struct.bpf_lru_list, ptr %8, i32 0, i32 1, !dbg !1708
  %10 = load i32, ptr %4, align 4, !dbg !1709
  %11 = zext i32 %10 to i64, !dbg !1707
  %12 = getelementptr inbounds [2 x i32], ptr %9, i64 0, i64 %11, !dbg !1707
  %13 = load i32, ptr %12, align 4, !dbg !1710
  %14 = add i32 %13, 1, !dbg !1710
  store i32 %14, ptr %12, align 4, !dbg !1710
  br label %15, !dbg !1707

15:                                               ; preds = %7, %2
  ret void, !dbg !1711
}

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @__local_list_pop_free(ptr noundef %0) #0 !dbg !1712 {
  %2 = alloca ptr, align 8
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !1715, metadata !DIExpression()), !dbg !1716
  call void @llvm.dbg.declare(metadata ptr %3, metadata !1717, metadata !DIExpression()), !dbg !1718
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1719, metadata !DIExpression()), !dbg !1721
  %10 = load ptr, ptr %2, align 8, !dbg !1721
  %11 = call ptr @local_free_list(ptr noundef %10), !dbg !1721
  store ptr %11, ptr %4, align 8, !dbg !1721
  call void @llvm.dbg.declare(metadata ptr %5, metadata !1722, metadata !DIExpression()), !dbg !1721
  br label %12, !dbg !1723

12:                                               ; preds = %1
  br label %13, !dbg !1725

13:                                               ; preds = %12
  %14 = load ptr, ptr %4, align 8, !dbg !1723
  %15 = getelementptr inbounds %struct.list_head, ptr %14, i32 0, i32 0, !dbg !1723
  %16 = load volatile ptr, ptr %15, align 8, !dbg !1723
  store ptr %16, ptr %6, align 8, !dbg !1725
  %17 = load ptr, ptr %6, align 8, !dbg !1723
  store ptr %17, ptr %5, align 8, !dbg !1721
  %18 = load ptr, ptr %5, align 8, !dbg !1721
  %19 = load ptr, ptr %4, align 8, !dbg !1721
  %20 = icmp ne ptr %18, %19, !dbg !1721
  br i1 %20, label %21, label %28, !dbg !1721

21:                                               ; preds = %13
  call void @llvm.dbg.declare(metadata ptr %8, metadata !1727, metadata !DIExpression()), !dbg !1729
  %22 = load ptr, ptr %5, align 8, !dbg !1729
  store ptr %22, ptr %8, align 8, !dbg !1729
  br label %23, !dbg !1729

23:                                               ; preds = %21
  br label %24, !dbg !1730

24:                                               ; preds = %23
  %25 = load ptr, ptr %8, align 8, !dbg !1729
  %26 = getelementptr i8, ptr %25, i64 0, !dbg !1729
  store ptr %26, ptr %9, align 8, !dbg !1730
  %27 = load ptr, ptr %9, align 8, !dbg !1729
  br label %29, !dbg !1721

28:                                               ; preds = %13
  br label %29, !dbg !1721

29:                                               ; preds = %28, %24
  %30 = phi ptr [ %27, %24 ], [ null, %28 ], !dbg !1721
  store ptr %30, ptr %7, align 8, !dbg !1721
  %31 = load ptr, ptr %7, align 8, !dbg !1721
  store ptr %31, ptr %3, align 8, !dbg !1732
  %32 = load ptr, ptr %3, align 8, !dbg !1733
  %33 = icmp ne ptr %32, null, !dbg !1733
  br i1 %33, label %34, label %37, !dbg !1735

34:                                               ; preds = %29
  %35 = load ptr, ptr %3, align 8, !dbg !1736
  %36 = getelementptr inbounds %struct.bpf_lru_node, ptr %35, i32 0, i32 0, !dbg !1737
  call void @list_del(ptr noundef %36), !dbg !1738
  br label %37, !dbg !1738

37:                                               ; preds = %34, %29
  %38 = load ptr, ptr %3, align 8, !dbg !1739
  ret ptr %38, !dbg !1740
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bpf_lru_list_pop_free_to_local(ptr noundef %0, ptr noundef %1) #0 !dbg !1741 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i32, align 4
  %9 = alloca ptr, align 8
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  %12 = alloca ptr, align 8
  %13 = alloca ptr, align 8
  %14 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !1744, metadata !DIExpression()), !dbg !1745
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1746, metadata !DIExpression()), !dbg !1747
  call void @llvm.dbg.declare(metadata ptr %5, metadata !1748, metadata !DIExpression()), !dbg !1749
  %15 = load ptr, ptr %3, align 8, !dbg !1750
  %16 = getelementptr inbounds %struct.bpf_lru, ptr %15, i32 0, i32 0, !dbg !1751
  %17 = getelementptr inbounds %struct.bpf_common_lru, ptr %16, i32 0, i32 0, !dbg !1752
  store ptr %17, ptr %5, align 8, !dbg !1749
  call void @llvm.dbg.declare(metadata ptr %6, metadata !1753, metadata !DIExpression()), !dbg !1754
  call void @llvm.dbg.declare(metadata ptr %7, metadata !1755, metadata !DIExpression()), !dbg !1756
  call void @llvm.dbg.declare(metadata ptr %8, metadata !1757, metadata !DIExpression()), !dbg !1758
  store i32 0, ptr %8, align 4, !dbg !1758
  %18 = load ptr, ptr %5, align 8, !dbg !1759
  %19 = getelementptr inbounds %struct.bpf_lru_list, ptr %18, i32 0, i32 3, !dbg !1759
  call void @_raw_spin_lock(ptr noundef %19), !dbg !1759
  %20 = load ptr, ptr %5, align 8, !dbg !1760
  %21 = load ptr, ptr %4, align 8, !dbg !1761
  call void @__local_list_flush(ptr noundef %20, ptr noundef %21), !dbg !1762
  %22 = load ptr, ptr %3, align 8, !dbg !1763
  %23 = load ptr, ptr %5, align 8, !dbg !1764
  call void @__bpf_lru_list_rotate(ptr noundef %22, ptr noundef %23), !dbg !1765
  call void @llvm.dbg.declare(metadata ptr %9, metadata !1766, metadata !DIExpression()), !dbg !1769
  %24 = load ptr, ptr %5, align 8, !dbg !1769
  %25 = getelementptr inbounds %struct.bpf_lru_list, ptr %24, i32 0, i32 0, !dbg !1769
  %26 = getelementptr inbounds [3 x %struct.list_head], ptr %25, i64 0, i64 2, !dbg !1769
  %27 = getelementptr inbounds %struct.list_head, ptr %26, i32 0, i32 0, !dbg !1769
  %28 = load ptr, ptr %27, align 32, !dbg !1769
  store ptr %28, ptr %9, align 8, !dbg !1769
  br label %29, !dbg !1769

29:                                               ; preds = %2
  br label %30, !dbg !1770

30:                                               ; preds = %29
  %31 = load ptr, ptr %9, align 8, !dbg !1769
  %32 = getelementptr i8, ptr %31, i64 0, !dbg !1769
  store ptr %32, ptr %10, align 8, !dbg !1770
  %33 = load ptr, ptr %10, align 8, !dbg !1769
  store ptr %33, ptr %6, align 8, !dbg !1772
  call void @llvm.dbg.declare(metadata ptr %11, metadata !1773, metadata !DIExpression()), !dbg !1775
  %34 = load ptr, ptr %6, align 8, !dbg !1775
  %35 = getelementptr inbounds %struct.bpf_lru_node, ptr %34, i32 0, i32 0, !dbg !1775
  %36 = getelementptr inbounds %struct.list_head, ptr %35, i32 0, i32 0, !dbg !1775
  %37 = load ptr, ptr %36, align 8, !dbg !1775
  store ptr %37, ptr %11, align 8, !dbg !1775
  br label %38, !dbg !1775

38:                                               ; preds = %30
  br label %39, !dbg !1776

39:                                               ; preds = %38
  %40 = load ptr, ptr %11, align 8, !dbg !1775
  %41 = getelementptr i8, ptr %40, i64 0, !dbg !1775
  store ptr %41, ptr %12, align 8, !dbg !1776
  %42 = load ptr, ptr %12, align 8, !dbg !1775
  store ptr %42, ptr %7, align 8, !dbg !1772
  br label %43, !dbg !1772

43:                                               ; preds = %68, %39
  %44 = load ptr, ptr %6, align 8, !dbg !1778
  %45 = getelementptr inbounds %struct.bpf_lru_node, ptr %44, i32 0, i32 0, !dbg !1778
  %46 = load ptr, ptr %5, align 8, !dbg !1778
  %47 = getelementptr inbounds %struct.bpf_lru_list, ptr %46, i32 0, i32 0, !dbg !1778
  %48 = getelementptr inbounds [3 x %struct.list_head], ptr %47, i64 0, i64 2, !dbg !1778
  %49 = icmp eq ptr %45, %48, !dbg !1778
  %50 = xor i1 %49, true, !dbg !1778
  br i1 %50, label %51, label %72, !dbg !1772

51:                                               ; preds = %43
  %52 = load ptr, ptr %5, align 8, !dbg !1780
  %53 = load ptr, ptr %6, align 8, !dbg !1782
  %54 = load ptr, ptr %4, align 8, !dbg !1783
  %55 = call ptr @local_free_list(ptr noundef %54), !dbg !1784
  call void @__bpf_lru_node_move_to_free(ptr noundef %52, ptr noundef %53, ptr noundef %55, i32 noundef 3), !dbg !1785
  %56 = load i32, ptr %8, align 4, !dbg !1786
  %57 = add i32 %56, 1, !dbg !1786
  store i32 %57, ptr %8, align 4, !dbg !1786
  %58 = icmp eq i32 %57, 128, !dbg !1788
  br i1 %58, label %59, label %60, !dbg !1789

59:                                               ; preds = %51
  br label %72, !dbg !1790

60:                                               ; preds = %51
  br label %61, !dbg !1791

61:                                               ; preds = %60
  %62 = load ptr, ptr %7, align 8, !dbg !1778
  store ptr %62, ptr %6, align 8, !dbg !1778
  call void @llvm.dbg.declare(metadata ptr %13, metadata !1792, metadata !DIExpression()), !dbg !1794
  %63 = load ptr, ptr %7, align 8, !dbg !1794
  %64 = getelementptr inbounds %struct.bpf_lru_node, ptr %63, i32 0, i32 0, !dbg !1794
  %65 = getelementptr inbounds %struct.list_head, ptr %64, i32 0, i32 0, !dbg !1794
  %66 = load ptr, ptr %65, align 8, !dbg !1794
  store ptr %66, ptr %13, align 8, !dbg !1794
  br label %67, !dbg !1794

67:                                               ; preds = %61
  br label %68, !dbg !1795

68:                                               ; preds = %67
  %69 = load ptr, ptr %13, align 8, !dbg !1794
  %70 = getelementptr i8, ptr %69, i64 0, !dbg !1794
  store ptr %70, ptr %14, align 8, !dbg !1795
  %71 = load ptr, ptr %14, align 8, !dbg !1794
  store ptr %71, ptr %7, align 8, !dbg !1778
  br label %43, !dbg !1778, !llvm.loop !1797

72:                                               ; preds = %59, %43
  %73 = load i32, ptr %8, align 4, !dbg !1799
  %74 = icmp ult i32 %73, 128, !dbg !1801
  br i1 %74, label %75, label %83, !dbg !1802

75:                                               ; preds = %72
  %76 = load ptr, ptr %3, align 8, !dbg !1803
  %77 = load ptr, ptr %5, align 8, !dbg !1804
  %78 = load i32, ptr %8, align 4, !dbg !1805
  %79 = sub i32 128, %78, !dbg !1806
  %80 = load ptr, ptr %4, align 8, !dbg !1807
  %81 = call ptr @local_free_list(ptr noundef %80), !dbg !1808
  %82 = call i32 @__bpf_lru_list_shrink(ptr noundef %76, ptr noundef %77, i32 noundef %79, ptr noundef %81, i32 noundef 3), !dbg !1809
  br label %83, !dbg !1809

83:                                               ; preds = %75, %72
  %84 = load ptr, ptr %5, align 8, !dbg !1810
  %85 = getelementptr inbounds %struct.bpf_lru_list, ptr %84, i32 0, i32 3, !dbg !1810
  call void @_raw_spin_unlock(ptr noundef %85), !dbg !1810
  ret void, !dbg !1811
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @__local_list_add_pending(ptr noundef %0, ptr noundef %1, i32 noundef %2, ptr noundef %3, i32 noundef %4) #0 !dbg !1812 {
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca i32, align 4
  %9 = alloca ptr, align 8
  %10 = alloca i32, align 4
  store ptr %0, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !1815, metadata !DIExpression()), !dbg !1816
  store ptr %1, ptr %7, align 8
  call void @llvm.dbg.declare(metadata ptr %7, metadata !1817, metadata !DIExpression()), !dbg !1818
  store i32 %2, ptr %8, align 4
  call void @llvm.dbg.declare(metadata ptr %8, metadata !1819, metadata !DIExpression()), !dbg !1820
  store ptr %3, ptr %9, align 8
  call void @llvm.dbg.declare(metadata ptr %9, metadata !1821, metadata !DIExpression()), !dbg !1822
  store i32 %4, ptr %10, align 4
  call void @llvm.dbg.declare(metadata ptr %10, metadata !1823, metadata !DIExpression()), !dbg !1824
  %11 = load i32, ptr %10, align 4, !dbg !1825
  %12 = load ptr, ptr %9, align 8, !dbg !1826
  %13 = load ptr, ptr %6, align 8, !dbg !1827
  %14 = getelementptr inbounds %struct.bpf_lru, ptr %13, i32 0, i32 3, !dbg !1828
  %15 = load i32, ptr %14, align 16, !dbg !1828
  %16 = zext i32 %15 to i64, !dbg !1829
  %17 = getelementptr i8, ptr %12, i64 %16, !dbg !1829
  store i32 %11, ptr %17, align 4, !dbg !1830
  %18 = load i32, ptr %8, align 4, !dbg !1831
  %19 = trunc i32 %18 to i16, !dbg !1831
  %20 = load ptr, ptr %9, align 8, !dbg !1832
  %21 = getelementptr inbounds %struct.bpf_lru_node, ptr %20, i32 0, i32 1, !dbg !1833
  store i16 %19, ptr %21, align 8, !dbg !1834
  %22 = load ptr, ptr %9, align 8, !dbg !1835
  %23 = getelementptr inbounds %struct.bpf_lru_node, ptr %22, i32 0, i32 2, !dbg !1836
  store i8 4, ptr %23, align 2, !dbg !1837
  %24 = load ptr, ptr %9, align 8, !dbg !1838
  %25 = getelementptr inbounds %struct.bpf_lru_node, ptr %24, i32 0, i32 3, !dbg !1839
  store i8 0, ptr %25, align 1, !dbg !1840
  %26 = load ptr, ptr %9, align 8, !dbg !1841
  %27 = getelementptr inbounds %struct.bpf_lru_node, ptr %26, i32 0, i32 0, !dbg !1842
  %28 = load ptr, ptr %7, align 8, !dbg !1843
  %29 = call ptr @local_pending_list(ptr noundef %28), !dbg !1844
  call void @list_add(ptr noundef %27, ptr noundef %29), !dbg !1845
  ret void, !dbg !1846
}

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @__local_list_pop_pending(ptr noundef %0, ptr noundef %1) #0 !dbg !1847 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i8, align 1
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1850, metadata !DIExpression()), !dbg !1851
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !1852, metadata !DIExpression()), !dbg !1853
  call void @llvm.dbg.declare(metadata ptr %6, metadata !1854, metadata !DIExpression()), !dbg !1855
  call void @llvm.dbg.declare(metadata ptr %7, metadata !1856, metadata !DIExpression()), !dbg !1857
  store i8 0, ptr %7, align 1, !dbg !1857
  br label %12, !dbg !1858

12:                                               ; preds = %62, %2
  call void @llvm.dbg.label(metadata !1859), !dbg !1860
  call void @llvm.dbg.declare(metadata ptr %8, metadata !1861, metadata !DIExpression()), !dbg !1864
  %13 = load ptr, ptr %5, align 8, !dbg !1864
  %14 = call ptr @local_pending_list(ptr noundef %13), !dbg !1864
  %15 = getelementptr inbounds %struct.list_head, ptr %14, i32 0, i32 1, !dbg !1864
  %16 = load ptr, ptr %15, align 8, !dbg !1864
  store ptr %16, ptr %8, align 8, !dbg !1864
  br label %17, !dbg !1864

17:                                               ; preds = %12
  br label %18, !dbg !1865

18:                                               ; preds = %17
  %19 = load ptr, ptr %8, align 8, !dbg !1864
  %20 = getelementptr i8, ptr %19, i64 0, !dbg !1864
  store ptr %20, ptr %9, align 8, !dbg !1865
  %21 = load ptr, ptr %9, align 8, !dbg !1864
  store ptr %21, ptr %6, align 8, !dbg !1867
  br label %22, !dbg !1867

22:                                               ; preds = %55, %18
  %23 = load ptr, ptr %6, align 8, !dbg !1868
  %24 = getelementptr inbounds %struct.bpf_lru_node, ptr %23, i32 0, i32 0, !dbg !1868
  %25 = load ptr, ptr %5, align 8, !dbg !1868
  %26 = call ptr @local_pending_list(ptr noundef %25), !dbg !1868
  %27 = icmp eq ptr %24, %26, !dbg !1868
  %28 = xor i1 %27, true, !dbg !1868
  br i1 %28, label %29, label %59, !dbg !1867

29:                                               ; preds = %22
  %30 = load ptr, ptr %6, align 8, !dbg !1870
  %31 = call zeroext i1 @bpf_lru_node_is_ref(ptr noundef %30), !dbg !1873
  br i1 %31, label %32, label %35, !dbg !1874

32:                                               ; preds = %29
  %33 = load i8, ptr %7, align 1, !dbg !1875
  %34 = trunc i8 %33 to i1, !dbg !1875
  br i1 %34, label %35, label %48, !dbg !1876

35:                                               ; preds = %32, %29
  %36 = load ptr, ptr %4, align 8, !dbg !1877
  %37 = getelementptr inbounds %struct.bpf_lru, ptr %36, i32 0, i32 1, !dbg !1878
  %38 = load ptr, ptr %37, align 64, !dbg !1878
  %39 = load ptr, ptr %4, align 8, !dbg !1879
  %40 = getelementptr inbounds %struct.bpf_lru, ptr %39, i32 0, i32 2, !dbg !1880
  %41 = load ptr, ptr %40, align 8, !dbg !1880
  %42 = load ptr, ptr %6, align 8, !dbg !1881
  %43 = call zeroext i1 %38(ptr noundef %41, ptr noundef %42), !dbg !1877
  br i1 %43, label %44, label %48, !dbg !1882

44:                                               ; preds = %35
  %45 = load ptr, ptr %6, align 8, !dbg !1883
  %46 = getelementptr inbounds %struct.bpf_lru_node, ptr %45, i32 0, i32 0, !dbg !1885
  call void @list_del(ptr noundef %46), !dbg !1886
  %47 = load ptr, ptr %6, align 8, !dbg !1887
  store ptr %47, ptr %3, align 8, !dbg !1888
  br label %64, !dbg !1888

48:                                               ; preds = %35, %32
  br label %49, !dbg !1889

49:                                               ; preds = %48
  call void @llvm.dbg.declare(metadata ptr %10, metadata !1890, metadata !DIExpression()), !dbg !1892
  %50 = load ptr, ptr %6, align 8, !dbg !1892
  %51 = getelementptr inbounds %struct.bpf_lru_node, ptr %50, i32 0, i32 0, !dbg !1892
  %52 = getelementptr inbounds %struct.list_head, ptr %51, i32 0, i32 1, !dbg !1892
  %53 = load ptr, ptr %52, align 8, !dbg !1892
  store ptr %53, ptr %10, align 8, !dbg !1892
  br label %54, !dbg !1892

54:                                               ; preds = %49
  br label %55, !dbg !1893

55:                                               ; preds = %54
  %56 = load ptr, ptr %10, align 8, !dbg !1892
  %57 = getelementptr i8, ptr %56, i64 0, !dbg !1892
  store ptr %57, ptr %11, align 8, !dbg !1893
  %58 = load ptr, ptr %11, align 8, !dbg !1892
  store ptr %58, ptr %6, align 8, !dbg !1868
  br label %22, !dbg !1868, !llvm.loop !1895

59:                                               ; preds = %22
  %60 = load i8, ptr %7, align 1, !dbg !1897
  %61 = trunc i8 %60 to i1, !dbg !1897
  br i1 %61, label %63, label %62, !dbg !1899

62:                                               ; preds = %59
  store i8 1, ptr %7, align 1, !dbg !1900
  br label %12, !dbg !1902

63:                                               ; preds = %59
  store ptr null, ptr %3, align 8, !dbg !1903
  br label %64, !dbg !1903

64:                                               ; preds = %63, %44
  %65 = load ptr, ptr %3, align 8, !dbg !1904
  ret ptr %65, !dbg !1904
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @get_next_cpu(i32 noundef %0) #0 !dbg !1905 {
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  call void @llvm.dbg.declare(metadata ptr %2, metadata !1908, metadata !DIExpression()), !dbg !1909
  %3 = load i32, ptr %2, align 4, !dbg !1910
  %4 = call i32 @cpumask_next(i32 noundef %3, ptr noundef @__cpu_possible_mask), !dbg !1911
  store i32 %4, ptr %2, align 4, !dbg !1912
  %5 = load i32, ptr %2, align 4, !dbg !1913
  %6 = load i32, ptr @nr_cpu_ids, align 4, !dbg !1915
  %7 = icmp uge i32 %5, %6, !dbg !1916
  br i1 %7, label %8, label %10, !dbg !1917

8:                                                ; preds = %1
  %9 = call i32 @cpumask_first(ptr noundef @__cpu_possible_mask), !dbg !1918
  store i32 %9, ptr %2, align 4, !dbg !1919
  br label %10, !dbg !1920

10:                                               ; preds = %8, %1
  %11 = load i32, ptr %2, align 4, !dbg !1921
  ret i32 %11, !dbg !1922
}

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @local_free_list(ptr noundef %0) #0 !dbg !1923 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !1926, metadata !DIExpression()), !dbg !1927
  %3 = load ptr, ptr %2, align 8, !dbg !1928
  %4 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %3, i32 0, i32 0, !dbg !1929
  %5 = getelementptr inbounds [2 x %struct.list_head], ptr %4, i64 0, i64 0, !dbg !1928
  ret ptr %5, !dbg !1930
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @list_del(ptr noundef %0) #0 !dbg !1931 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !1932, metadata !DIExpression()), !dbg !1933
  %3 = load ptr, ptr %2, align 8, !dbg !1934
  call void @__list_del_entry(ptr noundef %3), !dbg !1935
  %4 = load ptr, ptr %2, align 8, !dbg !1936
  %5 = getelementptr inbounds %struct.list_head, ptr %4, i32 0, i32 0, !dbg !1937
  store ptr getelementptr (i8, ptr inttoptr (i64 256 to ptr), i64 -2401263026318606336), ptr %5, align 8, !dbg !1938
  %6 = load ptr, ptr %2, align 8, !dbg !1939
  %7 = getelementptr inbounds %struct.list_head, ptr %6, i32 0, i32 1, !dbg !1940
  store ptr getelementptr (i8, ptr inttoptr (i64 290 to ptr), i64 -2401263026318606336), ptr %7, align 8, !dbg !1941
  ret void, !dbg !1942
}

declare dso_local void @_raw_spin_lock(ptr noundef) #2 section ".spinlock.text"

; Function Attrs: noinline nounwind optnone uwtable
define internal void @__local_list_flush(ptr noundef %0, ptr noundef %1) #0 !dbg !1943 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca ptr, align 8
  %11 = alloca ptr, align 8
  %12 = alloca ptr, align 8
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !1946, metadata !DIExpression()), !dbg !1947
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1948, metadata !DIExpression()), !dbg !1949
  call void @llvm.dbg.declare(metadata ptr %5, metadata !1950, metadata !DIExpression()), !dbg !1951
  call void @llvm.dbg.declare(metadata ptr %6, metadata !1952, metadata !DIExpression()), !dbg !1953
  call void @llvm.dbg.declare(metadata ptr %7, metadata !1954, metadata !DIExpression()), !dbg !1957
  %13 = load ptr, ptr %4, align 8, !dbg !1957
  %14 = call ptr @local_pending_list(ptr noundef %13), !dbg !1957
  %15 = getelementptr inbounds %struct.list_head, ptr %14, i32 0, i32 1, !dbg !1957
  %16 = load ptr, ptr %15, align 8, !dbg !1957
  store ptr %16, ptr %7, align 8, !dbg !1957
  br label %17, !dbg !1957

17:                                               ; preds = %2
  br label %18, !dbg !1958

18:                                               ; preds = %17
  %19 = load ptr, ptr %7, align 8, !dbg !1957
  %20 = getelementptr i8, ptr %19, i64 0, !dbg !1957
  store ptr %20, ptr %8, align 8, !dbg !1958
  %21 = load ptr, ptr %8, align 8, !dbg !1957
  store ptr %21, ptr %5, align 8, !dbg !1960
  call void @llvm.dbg.declare(metadata ptr %9, metadata !1961, metadata !DIExpression()), !dbg !1963
  %22 = load ptr, ptr %5, align 8, !dbg !1963
  %23 = getelementptr inbounds %struct.bpf_lru_node, ptr %22, i32 0, i32 0, !dbg !1963
  %24 = getelementptr inbounds %struct.list_head, ptr %23, i32 0, i32 1, !dbg !1963
  %25 = load ptr, ptr %24, align 8, !dbg !1963
  store ptr %25, ptr %9, align 8, !dbg !1963
  br label %26, !dbg !1963

26:                                               ; preds = %18
  br label %27, !dbg !1964

27:                                               ; preds = %26
  %28 = load ptr, ptr %9, align 8, !dbg !1963
  %29 = getelementptr i8, ptr %28, i64 0, !dbg !1963
  store ptr %29, ptr %10, align 8, !dbg !1964
  %30 = load ptr, ptr %10, align 8, !dbg !1963
  store ptr %30, ptr %6, align 8, !dbg !1960
  br label %31, !dbg !1960

31:                                               ; preds = %55, %27
  %32 = load ptr, ptr %5, align 8, !dbg !1966
  %33 = getelementptr inbounds %struct.bpf_lru_node, ptr %32, i32 0, i32 0, !dbg !1966
  %34 = load ptr, ptr %4, align 8, !dbg !1966
  %35 = call ptr @local_pending_list(ptr noundef %34), !dbg !1966
  %36 = icmp eq ptr %33, %35, !dbg !1966
  %37 = xor i1 %36, true, !dbg !1966
  br i1 %37, label %38, label %59, !dbg !1960

38:                                               ; preds = %31
  %39 = load ptr, ptr %5, align 8, !dbg !1968
  %40 = call zeroext i1 @bpf_lru_node_is_ref(ptr noundef %39), !dbg !1971
  br i1 %40, label %41, label %44, !dbg !1972

41:                                               ; preds = %38
  %42 = load ptr, ptr %3, align 8, !dbg !1973
  %43 = load ptr, ptr %5, align 8, !dbg !1974
  call void @__bpf_lru_node_move_in(ptr noundef %42, ptr noundef %43, i32 noundef 0), !dbg !1975
  br label %47, !dbg !1975

44:                                               ; preds = %38
  %45 = load ptr, ptr %3, align 8, !dbg !1976
  %46 = load ptr, ptr %5, align 8, !dbg !1977
  call void @__bpf_lru_node_move_in(ptr noundef %45, ptr noundef %46, i32 noundef 1), !dbg !1978
  br label %47

47:                                               ; preds = %44, %41
  br label %48, !dbg !1979

48:                                               ; preds = %47
  %49 = load ptr, ptr %6, align 8, !dbg !1966
  store ptr %49, ptr %5, align 8, !dbg !1966
  call void @llvm.dbg.declare(metadata ptr %11, metadata !1980, metadata !DIExpression()), !dbg !1982
  %50 = load ptr, ptr %6, align 8, !dbg !1982
  %51 = getelementptr inbounds %struct.bpf_lru_node, ptr %50, i32 0, i32 0, !dbg !1982
  %52 = getelementptr inbounds %struct.list_head, ptr %51, i32 0, i32 1, !dbg !1982
  %53 = load ptr, ptr %52, align 8, !dbg !1982
  store ptr %53, ptr %11, align 8, !dbg !1982
  br label %54, !dbg !1982

54:                                               ; preds = %48
  br label %55, !dbg !1983

55:                                               ; preds = %54
  %56 = load ptr, ptr %11, align 8, !dbg !1982
  %57 = getelementptr i8, ptr %56, i64 0, !dbg !1982
  store ptr %57, ptr %12, align 8, !dbg !1983
  %58 = load ptr, ptr %12, align 8, !dbg !1982
  store ptr %58, ptr %6, align 8, !dbg !1966
  br label %31, !dbg !1966, !llvm.loop !1985

59:                                               ; preds = %31
  ret void, !dbg !1987
}

declare dso_local void @_raw_spin_unlock(ptr noundef) #2 section ".spinlock.text"

; Function Attrs: noinline nounwind optnone uwtable
define internal ptr @local_pending_list(ptr noundef %0) #0 !dbg !1988 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !1989, metadata !DIExpression()), !dbg !1990
  %3 = load ptr, ptr %2, align 8, !dbg !1991
  %4 = getelementptr inbounds %struct.bpf_lru_locallist, ptr %3, i32 0, i32 0, !dbg !1992
  %5 = getelementptr inbounds [2 x %struct.list_head], ptr %4, i64 0, i64 1, !dbg !1991
  ret ptr %5, !dbg !1993
}

; Function Attrs: noinline nounwind optnone uwtable
define internal void @__bpf_lru_node_move_in(ptr noundef %0, ptr noundef %1, i32 noundef %2) #0 !dbg !1994 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i64, align 8
  %9 = alloca i32, align 4
  %10 = alloca i64, align 8
  store ptr %0, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !1995, metadata !DIExpression()), !dbg !1996
  store ptr %1, ptr %5, align 8
  call void @llvm.dbg.declare(metadata ptr %5, metadata !1997, metadata !DIExpression()), !dbg !1998
  store i32 %2, ptr %6, align 4
  call void @llvm.dbg.declare(metadata ptr %6, metadata !1999, metadata !DIExpression()), !dbg !2000
  call void @llvm.dbg.declare(metadata ptr %7, metadata !2001, metadata !DIExpression()), !dbg !2004
  %11 = load ptr, ptr %5, align 8, !dbg !2004
  %12 = getelementptr inbounds %struct.bpf_lru_node, ptr %11, i32 0, i32 2, !dbg !2004
  %13 = load i8, ptr %12, align 2, !dbg !2004
  %14 = zext i8 %13 to i32, !dbg !2004
  %15 = icmp sge i32 %14, 3, !dbg !2004
  %16 = xor i1 %15, true, !dbg !2004
  %17 = xor i1 %16, true, !dbg !2004
  %18 = xor i1 %17, true, !dbg !2004
  %19 = zext i1 %18 to i32, !dbg !2004
  store i32 %19, ptr %7, align 4, !dbg !2004
  %20 = load i32, ptr %7, align 4, !dbg !2005
  %21 = icmp ne i32 %20, 0, !dbg !2005
  %22 = xor i1 %21, true, !dbg !2005
  %23 = xor i1 %22, true, !dbg !2005
  %24 = zext i1 %23 to i32, !dbg !2005
  %25 = sext i32 %24 to i64, !dbg !2005
  %26 = icmp ne i64 %25, 0, !dbg !2005
  br i1 %26, label %27, label %32, !dbg !2004

27:                                               ; preds = %3
  br label %28, !dbg !2005

28:                                               ; preds = %27
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_begin\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 219) #4, !dbg !2007, !srcloc !2010
  br label %29, !dbg !2011

29:                                               ; preds = %28
  call void asm sideeffect "1:\09.byte 0x0f, 0x0b\0A.pushsection __bug_table,\22aw\22\0A2:\09.long 1b - 2b\09# bug_entry::bug_addr\0A\09.long ${0:c} - 2b\09# bug_entry::file\0A\09.word ${1:c}\09# bug_entry::line\0A\09.word ${2:c}\09# bug_entry::flags\0A\09.org 2b+${3:c}\0A.popsection", "i,i,i,i,~{dirflag},~{fpsr},~{flags}"(ptr @.str, i32 86, i32 2307, i64 12) #4, !dbg !2012, !srcloc !2014
  br label %30, !dbg !2012

30:                                               ; preds = %29
  call void asm sideeffect "${0:c}:\0A\09.pushsection .discard.reachable\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 220) #4, !dbg !2015, !srcloc !2017
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_end\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 221) #4, !dbg !2018, !srcloc !2020
  br label %31, !dbg !2011

31:                                               ; preds = %30
  br label %32, !dbg !2011

32:                                               ; preds = %31, %3
  %33 = load i32, ptr %7, align 4, !dbg !2004
  %34 = icmp ne i32 %33, 0, !dbg !2004
  %35 = xor i1 %34, true, !dbg !2004
  %36 = xor i1 %35, true, !dbg !2004
  %37 = zext i1 %36 to i32, !dbg !2004
  %38 = sext i32 %37 to i64, !dbg !2004
  store i64 %38, ptr %8, align 8, !dbg !2005
  %39 = load i64, ptr %8, align 8, !dbg !2004
  %40 = icmp ne i64 %39, 0, !dbg !2021
  br i1 %40, label %68, label %41, !dbg !2022

41:                                               ; preds = %32
  call void @llvm.dbg.declare(metadata ptr %9, metadata !2023, metadata !DIExpression()), !dbg !2025
  %42 = load i32, ptr %6, align 4, !dbg !2025
  %43 = icmp uge i32 %42, 3, !dbg !2025
  %44 = xor i1 %43, true, !dbg !2025
  %45 = xor i1 %44, true, !dbg !2025
  %46 = zext i1 %45 to i32, !dbg !2025
  store i32 %46, ptr %9, align 4, !dbg !2025
  %47 = load i32, ptr %9, align 4, !dbg !2026
  %48 = icmp ne i32 %47, 0, !dbg !2026
  %49 = xor i1 %48, true, !dbg !2026
  %50 = xor i1 %49, true, !dbg !2026
  %51 = zext i1 %50 to i32, !dbg !2026
  %52 = sext i32 %51 to i64, !dbg !2026
  %53 = icmp ne i64 %52, 0, !dbg !2026
  br i1 %53, label %54, label %59, !dbg !2025

54:                                               ; preds = %41
  br label %55, !dbg !2026

55:                                               ; preds = %54
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_begin\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 222) #4, !dbg !2028, !srcloc !2031
  br label %56, !dbg !2032

56:                                               ; preds = %55
  call void asm sideeffect "1:\09.byte 0x0f, 0x0b\0A.pushsection __bug_table,\22aw\22\0A2:\09.long 1b - 2b\09# bug_entry::bug_addr\0A\09.long ${0:c} - 2b\09# bug_entry::file\0A\09.word ${1:c}\09# bug_entry::line\0A\09.word ${2:c}\09# bug_entry::flags\0A\09.org 2b+${3:c}\0A.popsection", "i,i,i,i,~{dirflag},~{fpsr},~{flags}"(ptr @.str, i32 87, i32 2307, i64 12) #4, !dbg !2033, !srcloc !2035
  br label %57, !dbg !2033

57:                                               ; preds = %56
  call void asm sideeffect "${0:c}:\0A\09.pushsection .discard.reachable\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 223) #4, !dbg !2036, !srcloc !2038
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_end\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 224) #4, !dbg !2039, !srcloc !2041
  br label %58, !dbg !2032

58:                                               ; preds = %57
  br label %59, !dbg !2032

59:                                               ; preds = %58, %41
  %60 = load i32, ptr %9, align 4, !dbg !2025
  %61 = icmp ne i32 %60, 0, !dbg !2025
  %62 = xor i1 %61, true, !dbg !2025
  %63 = xor i1 %62, true, !dbg !2025
  %64 = zext i1 %63 to i32, !dbg !2025
  %65 = sext i32 %64 to i64, !dbg !2025
  store i64 %65, ptr %10, align 8, !dbg !2026
  %66 = load i64, ptr %10, align 8, !dbg !2025
  %67 = icmp ne i64 %66, 0, !dbg !2042
  br i1 %67, label %68, label %69, !dbg !2043

68:                                               ; preds = %59, %32
  br label %85, !dbg !2044

69:                                               ; preds = %59
  %70 = load ptr, ptr %4, align 8, !dbg !2045
  %71 = load i32, ptr %6, align 4, !dbg !2046
  call void @bpf_lru_list_count_inc(ptr noundef %70, i32 noundef %71), !dbg !2047
  %72 = load i32, ptr %6, align 4, !dbg !2048
  %73 = trunc i32 %72 to i8, !dbg !2048
  %74 = load ptr, ptr %5, align 8, !dbg !2049
  %75 = getelementptr inbounds %struct.bpf_lru_node, ptr %74, i32 0, i32 2, !dbg !2050
  store i8 %73, ptr %75, align 2, !dbg !2051
  %76 = load ptr, ptr %5, align 8, !dbg !2052
  %77 = getelementptr inbounds %struct.bpf_lru_node, ptr %76, i32 0, i32 3, !dbg !2053
  store i8 0, ptr %77, align 1, !dbg !2054
  %78 = load ptr, ptr %5, align 8, !dbg !2055
  %79 = getelementptr inbounds %struct.bpf_lru_node, ptr %78, i32 0, i32 0, !dbg !2056
  %80 = load ptr, ptr %4, align 8, !dbg !2057
  %81 = getelementptr inbounds %struct.bpf_lru_list, ptr %80, i32 0, i32 0, !dbg !2058
  %82 = load i32, ptr %6, align 4, !dbg !2059
  %83 = zext i32 %82 to i64, !dbg !2057
  %84 = getelementptr inbounds [3 x %struct.list_head], ptr %81, i64 0, i64 %83, !dbg !2057
  call void @list_move(ptr noundef %79, ptr noundef %84), !dbg !2060
  br label %85, !dbg !2061

85:                                               ; preds = %69, %68
  ret void, !dbg !2061
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.label(metadata) #1

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @cpumask_first(ptr noundef %0) #0 !dbg !2062 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !2065, metadata !DIExpression()), !dbg !2066
  %3 = load ptr, ptr %2, align 8, !dbg !2067
  %4 = getelementptr inbounds %struct.cpumask, ptr %3, i32 0, i32 0, !dbg !2067
  %5 = getelementptr inbounds [128 x i64], ptr %4, i64 0, i64 0, !dbg !2067
  %6 = load i32, ptr @nr_cpu_ids, align 4, !dbg !2068
  %7 = zext i32 %6 to i64, !dbg !2068
  %8 = call i64 @find_first_bit(ptr noundef %5, i64 noundef %7), !dbg !2069
  %9 = trunc i64 %8 to i32, !dbg !2069
  ret i32 %9, !dbg !2070
}

declare dso_local i64 @find_first_bit(ptr noundef, i64 noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bpf_lru_list_push_free(ptr noundef %0, ptr noundef %1) #0 !dbg !2071 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  %6 = alloca i32, align 4
  %7 = alloca i64, align 8
  %8 = alloca i64, align 8
  %9 = alloca i64, align 8
  %10 = alloca i32, align 4
  %11 = alloca i64, align 8
  %12 = alloca i64, align 8
  %13 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  call void @llvm.dbg.declare(metadata ptr %3, metadata !2074, metadata !DIExpression()), !dbg !2075
  store ptr %1, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !2076, metadata !DIExpression()), !dbg !2077
  call void @llvm.dbg.declare(metadata ptr %5, metadata !2078, metadata !DIExpression()), !dbg !2079
  call void @llvm.dbg.declare(metadata ptr %6, metadata !2080, metadata !DIExpression()), !dbg !2083
  %14 = load ptr, ptr %4, align 8, !dbg !2083
  %15 = getelementptr inbounds %struct.bpf_lru_node, ptr %14, i32 0, i32 2, !dbg !2083
  %16 = load i8, ptr %15, align 2, !dbg !2083
  %17 = zext i8 %16 to i32, !dbg !2083
  %18 = icmp sge i32 %17, 3, !dbg !2083
  %19 = xor i1 %18, true, !dbg !2083
  %20 = xor i1 %19, true, !dbg !2083
  %21 = zext i1 %20 to i32, !dbg !2083
  store i32 %21, ptr %6, align 4, !dbg !2083
  %22 = load i32, ptr %6, align 4, !dbg !2084
  %23 = icmp ne i32 %22, 0, !dbg !2084
  %24 = xor i1 %23, true, !dbg !2084
  %25 = xor i1 %24, true, !dbg !2084
  %26 = zext i1 %25 to i32, !dbg !2084
  %27 = sext i32 %26 to i64, !dbg !2084
  %28 = icmp ne i64 %27, 0, !dbg !2084
  br i1 %28, label %29, label %34, !dbg !2083

29:                                               ; preds = %2
  br label %30, !dbg !2084

30:                                               ; preds = %29
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_begin\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 245) #4, !dbg !2086, !srcloc !2089
  br label %31, !dbg !2090

31:                                               ; preds = %30
  call void asm sideeffect "1:\09.byte 0x0f, 0x0b\0A.pushsection __bug_table,\22aw\22\0A2:\09.long 1b - 2b\09# bug_entry::bug_addr\0A\09.long ${0:c} - 2b\09# bug_entry::file\0A\09.word ${1:c}\09# bug_entry::line\0A\09.word ${2:c}\09# bug_entry::flags\0A\09.org 2b+${3:c}\0A.popsection", "i,i,i,i,~{dirflag},~{fpsr},~{flags}"(ptr @.str, i32 310, i32 2307, i64 12) #4, !dbg !2091, !srcloc !2093
  br label %32, !dbg !2091

32:                                               ; preds = %31
  call void asm sideeffect "${0:c}:\0A\09.pushsection .discard.reachable\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 246) #4, !dbg !2094, !srcloc !2096
  call void asm sideeffect "${0:c}: nop\0A\09.pushsection .discard.instr_end\0A\09.long ${0:c}b - .\0A\09.popsection\0A\09", "i,~{dirflag},~{fpsr},~{flags}"(i32 247) #4, !dbg !2097, !srcloc !2099
  br label %33, !dbg !2090

33:                                               ; preds = %32
  br label %34, !dbg !2090

34:                                               ; preds = %33, %2
  %35 = load i32, ptr %6, align 4, !dbg !2083
  %36 = icmp ne i32 %35, 0, !dbg !2083
  %37 = xor i1 %36, true, !dbg !2083
  %38 = xor i1 %37, true, !dbg !2083
  %39 = zext i1 %38 to i32, !dbg !2083
  %40 = sext i32 %39 to i64, !dbg !2083
  store i64 %40, ptr %7, align 8, !dbg !2084
  %41 = load i64, ptr %7, align 8, !dbg !2083
  %42 = icmp ne i64 %41, 0, !dbg !2100
  br i1 %42, label %43, label %44, !dbg !2101

43:                                               ; preds = %34
  br label %62, !dbg !2102

44:                                               ; preds = %34
  br label %45, !dbg !2103

45:                                               ; preds = %44
  call void @llvm.dbg.declare(metadata ptr %8, metadata !2104, metadata !DIExpression()), !dbg !2107
  call void @llvm.dbg.declare(metadata ptr %9, metadata !2108, metadata !DIExpression()), !dbg !2107
  %46 = icmp eq ptr %8, %9, !dbg !2107
  %47 = zext i1 %46 to i32, !dbg !2107
  store i32 1, ptr %10, align 4, !dbg !2107
  %48 = load i32, ptr %10, align 4, !dbg !2107
  %49 = load ptr, ptr %3, align 8, !dbg !2109
  %50 = getelementptr inbounds %struct.bpf_lru_list, ptr %49, i32 0, i32 3, !dbg !2109
  %51 = call i64 @_raw_spin_lock_irqsave(ptr noundef %50), !dbg !2109
  store i64 %51, ptr %5, align 8, !dbg !2109
  br label %52, !dbg !2109

52:                                               ; preds = %45
  %53 = load ptr, ptr %3, align 8, !dbg !2110
  %54 = load ptr, ptr %4, align 8, !dbg !2111
  call void @__bpf_lru_node_move(ptr noundef %53, ptr noundef %54, i32 noundef 2), !dbg !2112
  br label %55, !dbg !2113

55:                                               ; preds = %52
  call void @llvm.dbg.declare(metadata ptr %11, metadata !2114, metadata !DIExpression()), !dbg !2117
  call void @llvm.dbg.declare(metadata ptr %12, metadata !2118, metadata !DIExpression()), !dbg !2117
  %56 = icmp eq ptr %11, %12, !dbg !2117
  %57 = zext i1 %56 to i32, !dbg !2117
  store i32 1, ptr %13, align 4, !dbg !2117
  %58 = load i32, ptr %13, align 4, !dbg !2117
  %59 = load ptr, ptr %3, align 8, !dbg !2119
  %60 = getelementptr inbounds %struct.bpf_lru_list, ptr %59, i32 0, i32 3, !dbg !2119
  %61 = load i64, ptr %5, align 8, !dbg !2119
  call void @_raw_spin_unlock_irqrestore(ptr noundef %60, i64 noundef %61), !dbg !2119
  br label %62, !dbg !2119

62:                                               ; preds = %43, %55
  ret void, !dbg !2120
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @cpumask_weight(ptr noundef %0) #0 !dbg !2121 {
  %2 = alloca i64, align 8
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 8
  %7 = alloca i32, align 4
  %8 = alloca ptr, align 8
  store ptr %0, ptr %8, align 8
  call void @llvm.dbg.declare(metadata ptr %8, metadata !2122, metadata !DIExpression()), !dbg !2123
  %9 = load ptr, ptr %8, align 8, !dbg !2124
  %10 = getelementptr inbounds %struct.cpumask, ptr %9, i32 0, i32 0, !dbg !2124
  %11 = getelementptr inbounds [128 x i64], ptr %10, i64 0, i64 0, !dbg !2124
  %12 = load i32, ptr @nr_cpu_ids, align 4, !dbg !2125
  store ptr %11, ptr %6, align 8
  call void @llvm.dbg.declare(metadata ptr %6, metadata !2126, metadata !DIExpression()), !dbg !2133
  store i32 %12, ptr %7, align 4
  call void @llvm.dbg.declare(metadata ptr %7, metadata !2135, metadata !DIExpression()), !dbg !2136
  %13 = load i32, ptr %7, align 4, !dbg !2137
  %14 = call i1 @llvm.is.constant.i32(i32 %13), !dbg !2137
  br i1 %14, label %15, label %521, !dbg !2137

15:                                               ; preds = %1
  %16 = load i32, ptr %7, align 4, !dbg !2137
  %17 = icmp ule i32 %16, 64, !dbg !2137
  br i1 %17, label %18, label %521, !dbg !2137

18:                                               ; preds = %15
  %19 = load i32, ptr %7, align 4, !dbg !2137
  %20 = icmp ugt i32 %19, 0, !dbg !2137
  br i1 %20, label %21, label %521, !dbg !2139

21:                                               ; preds = %18
  %22 = load ptr, ptr %6, align 8, !dbg !2140
  %23 = load i64, ptr %22, align 8, !dbg !2141
  %24 = load i32, ptr %7, align 4, !dbg !2142
  %25 = sub i32 0, %24, !dbg !2142
  %26 = and i32 %25, 63, !dbg !2142
  %27 = zext i32 %26 to i64, !dbg !2142
  %28 = lshr i64 -1, %27, !dbg !2142
  %29 = and i64 %23, %28, !dbg !2143
  store i64 %29, ptr %4, align 8
  call void @llvm.dbg.declare(metadata ptr %4, metadata !2144, metadata !DIExpression()), !dbg !2149
  %30 = load i64, ptr %4, align 8, !dbg !2151
  %31 = call i1 @llvm.is.constant.i64(i64 %30), !dbg !2151
  br i1 %31, label %32, label %513, !dbg !2151

32:                                               ; preds = %21
  %33 = load i64, ptr %4, align 8, !dbg !2151
  %34 = and i64 %33, 1, !dbg !2151
  %35 = icmp ne i64 %34, 0, !dbg !2151
  %36 = xor i1 %35, true, !dbg !2151
  %37 = zext i1 %35 to i32, !dbg !2151
  %38 = load i64, ptr %4, align 8, !dbg !2151
  %39 = and i64 %38, 2, !dbg !2151
  %40 = icmp ne i64 %39, 0, !dbg !2151
  %41 = xor i1 %40, true, !dbg !2151
  %42 = zext i1 %40 to i32, !dbg !2151
  %43 = add nsw i32 %37, %42, !dbg !2151
  %44 = load i64, ptr %4, align 8, !dbg !2151
  %45 = and i64 %44, 4, !dbg !2151
  %46 = icmp ne i64 %45, 0, !dbg !2151
  %47 = xor i1 %46, true, !dbg !2151
  %48 = zext i1 %46 to i32, !dbg !2151
  %49 = add nsw i32 %43, %48, !dbg !2151
  %50 = load i64, ptr %4, align 8, !dbg !2151
  %51 = and i64 %50, 8, !dbg !2151
  %52 = icmp ne i64 %51, 0, !dbg !2151
  %53 = xor i1 %52, true, !dbg !2151
  %54 = zext i1 %52 to i32, !dbg !2151
  %55 = add nsw i32 %49, %54, !dbg !2151
  %56 = load i64, ptr %4, align 8, !dbg !2151
  %57 = and i64 %56, 16, !dbg !2151
  %58 = icmp ne i64 %57, 0, !dbg !2151
  %59 = xor i1 %58, true, !dbg !2151
  %60 = zext i1 %58 to i32, !dbg !2151
  %61 = add nsw i32 %55, %60, !dbg !2151
  %62 = load i64, ptr %4, align 8, !dbg !2151
  %63 = and i64 %62, 32, !dbg !2151
  %64 = icmp ne i64 %63, 0, !dbg !2151
  %65 = xor i1 %64, true, !dbg !2151
  %66 = zext i1 %64 to i32, !dbg !2151
  %67 = add nsw i32 %61, %66, !dbg !2151
  %68 = load i64, ptr %4, align 8, !dbg !2151
  %69 = and i64 %68, 64, !dbg !2151
  %70 = icmp ne i64 %69, 0, !dbg !2151
  %71 = xor i1 %70, true, !dbg !2151
  %72 = zext i1 %70 to i32, !dbg !2151
  %73 = add nsw i32 %67, %72, !dbg !2151
  %74 = load i64, ptr %4, align 8, !dbg !2151
  %75 = and i64 %74, 128, !dbg !2151
  %76 = icmp ne i64 %75, 0, !dbg !2151
  %77 = xor i1 %76, true, !dbg !2151
  %78 = zext i1 %76 to i32, !dbg !2151
  %79 = add nsw i32 %73, %78, !dbg !2151
  %80 = load i64, ptr %4, align 8, !dbg !2151
  %81 = lshr i64 %80, 8, !dbg !2151
  %82 = and i64 %81, 1, !dbg !2151
  %83 = icmp ne i64 %82, 0, !dbg !2151
  %84 = xor i1 %83, true, !dbg !2151
  %85 = zext i1 %83 to i32, !dbg !2151
  %86 = load i64, ptr %4, align 8, !dbg !2151
  %87 = lshr i64 %86, 8, !dbg !2151
  %88 = and i64 %87, 2, !dbg !2151
  %89 = icmp ne i64 %88, 0, !dbg !2151
  %90 = xor i1 %89, true, !dbg !2151
  %91 = zext i1 %89 to i32, !dbg !2151
  %92 = add nsw i32 %85, %91, !dbg !2151
  %93 = load i64, ptr %4, align 8, !dbg !2151
  %94 = lshr i64 %93, 8, !dbg !2151
  %95 = and i64 %94, 4, !dbg !2151
  %96 = icmp ne i64 %95, 0, !dbg !2151
  %97 = xor i1 %96, true, !dbg !2151
  %98 = zext i1 %96 to i32, !dbg !2151
  %99 = add nsw i32 %92, %98, !dbg !2151
  %100 = load i64, ptr %4, align 8, !dbg !2151
  %101 = lshr i64 %100, 8, !dbg !2151
  %102 = and i64 %101, 8, !dbg !2151
  %103 = icmp ne i64 %102, 0, !dbg !2151
  %104 = xor i1 %103, true, !dbg !2151
  %105 = zext i1 %103 to i32, !dbg !2151
  %106 = add nsw i32 %99, %105, !dbg !2151
  %107 = load i64, ptr %4, align 8, !dbg !2151
  %108 = lshr i64 %107, 8, !dbg !2151
  %109 = and i64 %108, 16, !dbg !2151
  %110 = icmp ne i64 %109, 0, !dbg !2151
  %111 = xor i1 %110, true, !dbg !2151
  %112 = zext i1 %110 to i32, !dbg !2151
  %113 = add nsw i32 %106, %112, !dbg !2151
  %114 = load i64, ptr %4, align 8, !dbg !2151
  %115 = lshr i64 %114, 8, !dbg !2151
  %116 = and i64 %115, 32, !dbg !2151
  %117 = icmp ne i64 %116, 0, !dbg !2151
  %118 = xor i1 %117, true, !dbg !2151
  %119 = zext i1 %117 to i32, !dbg !2151
  %120 = add nsw i32 %113, %119, !dbg !2151
  %121 = load i64, ptr %4, align 8, !dbg !2151
  %122 = lshr i64 %121, 8, !dbg !2151
  %123 = and i64 %122, 64, !dbg !2151
  %124 = icmp ne i64 %123, 0, !dbg !2151
  %125 = xor i1 %124, true, !dbg !2151
  %126 = zext i1 %124 to i32, !dbg !2151
  %127 = add nsw i32 %120, %126, !dbg !2151
  %128 = load i64, ptr %4, align 8, !dbg !2151
  %129 = lshr i64 %128, 8, !dbg !2151
  %130 = and i64 %129, 128, !dbg !2151
  %131 = icmp ne i64 %130, 0, !dbg !2151
  %132 = xor i1 %131, true, !dbg !2151
  %133 = zext i1 %131 to i32, !dbg !2151
  %134 = add nsw i32 %127, %133, !dbg !2151
  %135 = add i32 %79, %134, !dbg !2151
  %136 = load i64, ptr %4, align 8, !dbg !2151
  %137 = lshr i64 %136, 16, !dbg !2151
  %138 = and i64 %137, 1, !dbg !2151
  %139 = icmp ne i64 %138, 0, !dbg !2151
  %140 = xor i1 %139, true, !dbg !2151
  %141 = zext i1 %139 to i32, !dbg !2151
  %142 = load i64, ptr %4, align 8, !dbg !2151
  %143 = lshr i64 %142, 16, !dbg !2151
  %144 = and i64 %143, 2, !dbg !2151
  %145 = icmp ne i64 %144, 0, !dbg !2151
  %146 = xor i1 %145, true, !dbg !2151
  %147 = zext i1 %145 to i32, !dbg !2151
  %148 = add nsw i32 %141, %147, !dbg !2151
  %149 = load i64, ptr %4, align 8, !dbg !2151
  %150 = lshr i64 %149, 16, !dbg !2151
  %151 = and i64 %150, 4, !dbg !2151
  %152 = icmp ne i64 %151, 0, !dbg !2151
  %153 = xor i1 %152, true, !dbg !2151
  %154 = zext i1 %152 to i32, !dbg !2151
  %155 = add nsw i32 %148, %154, !dbg !2151
  %156 = load i64, ptr %4, align 8, !dbg !2151
  %157 = lshr i64 %156, 16, !dbg !2151
  %158 = and i64 %157, 8, !dbg !2151
  %159 = icmp ne i64 %158, 0, !dbg !2151
  %160 = xor i1 %159, true, !dbg !2151
  %161 = zext i1 %159 to i32, !dbg !2151
  %162 = add nsw i32 %155, %161, !dbg !2151
  %163 = load i64, ptr %4, align 8, !dbg !2151
  %164 = lshr i64 %163, 16, !dbg !2151
  %165 = and i64 %164, 16, !dbg !2151
  %166 = icmp ne i64 %165, 0, !dbg !2151
  %167 = xor i1 %166, true, !dbg !2151
  %168 = zext i1 %166 to i32, !dbg !2151
  %169 = add nsw i32 %162, %168, !dbg !2151
  %170 = load i64, ptr %4, align 8, !dbg !2151
  %171 = lshr i64 %170, 16, !dbg !2151
  %172 = and i64 %171, 32, !dbg !2151
  %173 = icmp ne i64 %172, 0, !dbg !2151
  %174 = xor i1 %173, true, !dbg !2151
  %175 = zext i1 %173 to i32, !dbg !2151
  %176 = add nsw i32 %169, %175, !dbg !2151
  %177 = load i64, ptr %4, align 8, !dbg !2151
  %178 = lshr i64 %177, 16, !dbg !2151
  %179 = and i64 %178, 64, !dbg !2151
  %180 = icmp ne i64 %179, 0, !dbg !2151
  %181 = xor i1 %180, true, !dbg !2151
  %182 = zext i1 %180 to i32, !dbg !2151
  %183 = add nsw i32 %176, %182, !dbg !2151
  %184 = load i64, ptr %4, align 8, !dbg !2151
  %185 = lshr i64 %184, 16, !dbg !2151
  %186 = and i64 %185, 128, !dbg !2151
  %187 = icmp ne i64 %186, 0, !dbg !2151
  %188 = xor i1 %187, true, !dbg !2151
  %189 = zext i1 %187 to i32, !dbg !2151
  %190 = add nsw i32 %183, %189, !dbg !2151
  %191 = load i64, ptr %4, align 8, !dbg !2151
  %192 = lshr i64 %191, 16, !dbg !2151
  %193 = lshr i64 %192, 8, !dbg !2151
  %194 = and i64 %193, 1, !dbg !2151
  %195 = icmp ne i64 %194, 0, !dbg !2151
  %196 = xor i1 %195, true, !dbg !2151
  %197 = zext i1 %195 to i32, !dbg !2151
  %198 = load i64, ptr %4, align 8, !dbg !2151
  %199 = lshr i64 %198, 16, !dbg !2151
  %200 = lshr i64 %199, 8, !dbg !2151
  %201 = and i64 %200, 2, !dbg !2151
  %202 = icmp ne i64 %201, 0, !dbg !2151
  %203 = xor i1 %202, true, !dbg !2151
  %204 = zext i1 %202 to i32, !dbg !2151
  %205 = add nsw i32 %197, %204, !dbg !2151
  %206 = load i64, ptr %4, align 8, !dbg !2151
  %207 = lshr i64 %206, 16, !dbg !2151
  %208 = lshr i64 %207, 8, !dbg !2151
  %209 = and i64 %208, 4, !dbg !2151
  %210 = icmp ne i64 %209, 0, !dbg !2151
  %211 = xor i1 %210, true, !dbg !2151
  %212 = zext i1 %210 to i32, !dbg !2151
  %213 = add nsw i32 %205, %212, !dbg !2151
  %214 = load i64, ptr %4, align 8, !dbg !2151
  %215 = lshr i64 %214, 16, !dbg !2151
  %216 = lshr i64 %215, 8, !dbg !2151
  %217 = and i64 %216, 8, !dbg !2151
  %218 = icmp ne i64 %217, 0, !dbg !2151
  %219 = xor i1 %218, true, !dbg !2151
  %220 = zext i1 %218 to i32, !dbg !2151
  %221 = add nsw i32 %213, %220, !dbg !2151
  %222 = load i64, ptr %4, align 8, !dbg !2151
  %223 = lshr i64 %222, 16, !dbg !2151
  %224 = lshr i64 %223, 8, !dbg !2151
  %225 = and i64 %224, 16, !dbg !2151
  %226 = icmp ne i64 %225, 0, !dbg !2151
  %227 = xor i1 %226, true, !dbg !2151
  %228 = zext i1 %226 to i32, !dbg !2151
  %229 = add nsw i32 %221, %228, !dbg !2151
  %230 = load i64, ptr %4, align 8, !dbg !2151
  %231 = lshr i64 %230, 16, !dbg !2151
  %232 = lshr i64 %231, 8, !dbg !2151
  %233 = and i64 %232, 32, !dbg !2151
  %234 = icmp ne i64 %233, 0, !dbg !2151
  %235 = xor i1 %234, true, !dbg !2151
  %236 = zext i1 %234 to i32, !dbg !2151
  %237 = add nsw i32 %229, %236, !dbg !2151
  %238 = load i64, ptr %4, align 8, !dbg !2151
  %239 = lshr i64 %238, 16, !dbg !2151
  %240 = lshr i64 %239, 8, !dbg !2151
  %241 = and i64 %240, 64, !dbg !2151
  %242 = icmp ne i64 %241, 0, !dbg !2151
  %243 = xor i1 %242, true, !dbg !2151
  %244 = zext i1 %242 to i32, !dbg !2151
  %245 = add nsw i32 %237, %244, !dbg !2151
  %246 = load i64, ptr %4, align 8, !dbg !2151
  %247 = lshr i64 %246, 16, !dbg !2151
  %248 = lshr i64 %247, 8, !dbg !2151
  %249 = and i64 %248, 128, !dbg !2151
  %250 = icmp ne i64 %249, 0, !dbg !2151
  %251 = xor i1 %250, true, !dbg !2151
  %252 = zext i1 %250 to i32, !dbg !2151
  %253 = add nsw i32 %245, %252, !dbg !2151
  %254 = add i32 %190, %253, !dbg !2151
  %255 = add i32 %135, %254, !dbg !2151
  %256 = load i64, ptr %4, align 8, !dbg !2151
  %257 = lshr i64 %256, 32, !dbg !2151
  %258 = and i64 %257, 1, !dbg !2151
  %259 = icmp ne i64 %258, 0, !dbg !2151
  %260 = xor i1 %259, true, !dbg !2151
  %261 = zext i1 %259 to i32, !dbg !2151
  %262 = load i64, ptr %4, align 8, !dbg !2151
  %263 = lshr i64 %262, 32, !dbg !2151
  %264 = and i64 %263, 2, !dbg !2151
  %265 = icmp ne i64 %264, 0, !dbg !2151
  %266 = xor i1 %265, true, !dbg !2151
  %267 = zext i1 %265 to i32, !dbg !2151
  %268 = add nsw i32 %261, %267, !dbg !2151
  %269 = load i64, ptr %4, align 8, !dbg !2151
  %270 = lshr i64 %269, 32, !dbg !2151
  %271 = and i64 %270, 4, !dbg !2151
  %272 = icmp ne i64 %271, 0, !dbg !2151
  %273 = xor i1 %272, true, !dbg !2151
  %274 = zext i1 %272 to i32, !dbg !2151
  %275 = add nsw i32 %268, %274, !dbg !2151
  %276 = load i64, ptr %4, align 8, !dbg !2151
  %277 = lshr i64 %276, 32, !dbg !2151
  %278 = and i64 %277, 8, !dbg !2151
  %279 = icmp ne i64 %278, 0, !dbg !2151
  %280 = xor i1 %279, true, !dbg !2151
  %281 = zext i1 %279 to i32, !dbg !2151
  %282 = add nsw i32 %275, %281, !dbg !2151
  %283 = load i64, ptr %4, align 8, !dbg !2151
  %284 = lshr i64 %283, 32, !dbg !2151
  %285 = and i64 %284, 16, !dbg !2151
  %286 = icmp ne i64 %285, 0, !dbg !2151
  %287 = xor i1 %286, true, !dbg !2151
  %288 = zext i1 %286 to i32, !dbg !2151
  %289 = add nsw i32 %282, %288, !dbg !2151
  %290 = load i64, ptr %4, align 8, !dbg !2151
  %291 = lshr i64 %290, 32, !dbg !2151
  %292 = and i64 %291, 32, !dbg !2151
  %293 = icmp ne i64 %292, 0, !dbg !2151
  %294 = xor i1 %293, true, !dbg !2151
  %295 = zext i1 %293 to i32, !dbg !2151
  %296 = add nsw i32 %289, %295, !dbg !2151
  %297 = load i64, ptr %4, align 8, !dbg !2151
  %298 = lshr i64 %297, 32, !dbg !2151
  %299 = and i64 %298, 64, !dbg !2151
  %300 = icmp ne i64 %299, 0, !dbg !2151
  %301 = xor i1 %300, true, !dbg !2151
  %302 = zext i1 %300 to i32, !dbg !2151
  %303 = add nsw i32 %296, %302, !dbg !2151
  %304 = load i64, ptr %4, align 8, !dbg !2151
  %305 = lshr i64 %304, 32, !dbg !2151
  %306 = and i64 %305, 128, !dbg !2151
  %307 = icmp ne i64 %306, 0, !dbg !2151
  %308 = xor i1 %307, true, !dbg !2151
  %309 = zext i1 %307 to i32, !dbg !2151
  %310 = add nsw i32 %303, %309, !dbg !2151
  %311 = load i64, ptr %4, align 8, !dbg !2151
  %312 = lshr i64 %311, 32, !dbg !2151
  %313 = lshr i64 %312, 8, !dbg !2151
  %314 = and i64 %313, 1, !dbg !2151
  %315 = icmp ne i64 %314, 0, !dbg !2151
  %316 = xor i1 %315, true, !dbg !2151
  %317 = zext i1 %315 to i32, !dbg !2151
  %318 = load i64, ptr %4, align 8, !dbg !2151
  %319 = lshr i64 %318, 32, !dbg !2151
  %320 = lshr i64 %319, 8, !dbg !2151
  %321 = and i64 %320, 2, !dbg !2151
  %322 = icmp ne i64 %321, 0, !dbg !2151
  %323 = xor i1 %322, true, !dbg !2151
  %324 = zext i1 %322 to i32, !dbg !2151
  %325 = add nsw i32 %317, %324, !dbg !2151
  %326 = load i64, ptr %4, align 8, !dbg !2151
  %327 = lshr i64 %326, 32, !dbg !2151
  %328 = lshr i64 %327, 8, !dbg !2151
  %329 = and i64 %328, 4, !dbg !2151
  %330 = icmp ne i64 %329, 0, !dbg !2151
  %331 = xor i1 %330, true, !dbg !2151
  %332 = zext i1 %330 to i32, !dbg !2151
  %333 = add nsw i32 %325, %332, !dbg !2151
  %334 = load i64, ptr %4, align 8, !dbg !2151
  %335 = lshr i64 %334, 32, !dbg !2151
  %336 = lshr i64 %335, 8, !dbg !2151
  %337 = and i64 %336, 8, !dbg !2151
  %338 = icmp ne i64 %337, 0, !dbg !2151
  %339 = xor i1 %338, true, !dbg !2151
  %340 = zext i1 %338 to i32, !dbg !2151
  %341 = add nsw i32 %333, %340, !dbg !2151
  %342 = load i64, ptr %4, align 8, !dbg !2151
  %343 = lshr i64 %342, 32, !dbg !2151
  %344 = lshr i64 %343, 8, !dbg !2151
  %345 = and i64 %344, 16, !dbg !2151
  %346 = icmp ne i64 %345, 0, !dbg !2151
  %347 = xor i1 %346, true, !dbg !2151
  %348 = zext i1 %346 to i32, !dbg !2151
  %349 = add nsw i32 %341, %348, !dbg !2151
  %350 = load i64, ptr %4, align 8, !dbg !2151
  %351 = lshr i64 %350, 32, !dbg !2151
  %352 = lshr i64 %351, 8, !dbg !2151
  %353 = and i64 %352, 32, !dbg !2151
  %354 = icmp ne i64 %353, 0, !dbg !2151
  %355 = xor i1 %354, true, !dbg !2151
  %356 = zext i1 %354 to i32, !dbg !2151
  %357 = add nsw i32 %349, %356, !dbg !2151
  %358 = load i64, ptr %4, align 8, !dbg !2151
  %359 = lshr i64 %358, 32, !dbg !2151
  %360 = lshr i64 %359, 8, !dbg !2151
  %361 = and i64 %360, 64, !dbg !2151
  %362 = icmp ne i64 %361, 0, !dbg !2151
  %363 = xor i1 %362, true, !dbg !2151
  %364 = zext i1 %362 to i32, !dbg !2151
  %365 = add nsw i32 %357, %364, !dbg !2151
  %366 = load i64, ptr %4, align 8, !dbg !2151
  %367 = lshr i64 %366, 32, !dbg !2151
  %368 = lshr i64 %367, 8, !dbg !2151
  %369 = and i64 %368, 128, !dbg !2151
  %370 = icmp ne i64 %369, 0, !dbg !2151
  %371 = xor i1 %370, true, !dbg !2151
  %372 = zext i1 %370 to i32, !dbg !2151
  %373 = add nsw i32 %365, %372, !dbg !2151
  %374 = add i32 %310, %373, !dbg !2151
  %375 = load i64, ptr %4, align 8, !dbg !2151
  %376 = lshr i64 %375, 32, !dbg !2151
  %377 = lshr i64 %376, 16, !dbg !2151
  %378 = and i64 %377, 1, !dbg !2151
  %379 = icmp ne i64 %378, 0, !dbg !2151
  %380 = xor i1 %379, true, !dbg !2151
  %381 = zext i1 %379 to i32, !dbg !2151
  %382 = load i64, ptr %4, align 8, !dbg !2151
  %383 = lshr i64 %382, 32, !dbg !2151
  %384 = lshr i64 %383, 16, !dbg !2151
  %385 = and i64 %384, 2, !dbg !2151
  %386 = icmp ne i64 %385, 0, !dbg !2151
  %387 = xor i1 %386, true, !dbg !2151
  %388 = zext i1 %386 to i32, !dbg !2151
  %389 = add nsw i32 %381, %388, !dbg !2151
  %390 = load i64, ptr %4, align 8, !dbg !2151
  %391 = lshr i64 %390, 32, !dbg !2151
  %392 = lshr i64 %391, 16, !dbg !2151
  %393 = and i64 %392, 4, !dbg !2151
  %394 = icmp ne i64 %393, 0, !dbg !2151
  %395 = xor i1 %394, true, !dbg !2151
  %396 = zext i1 %394 to i32, !dbg !2151
  %397 = add nsw i32 %389, %396, !dbg !2151
  %398 = load i64, ptr %4, align 8, !dbg !2151
  %399 = lshr i64 %398, 32, !dbg !2151
  %400 = lshr i64 %399, 16, !dbg !2151
  %401 = and i64 %400, 8, !dbg !2151
  %402 = icmp ne i64 %401, 0, !dbg !2151
  %403 = xor i1 %402, true, !dbg !2151
  %404 = zext i1 %402 to i32, !dbg !2151
  %405 = add nsw i32 %397, %404, !dbg !2151
  %406 = load i64, ptr %4, align 8, !dbg !2151
  %407 = lshr i64 %406, 32, !dbg !2151
  %408 = lshr i64 %407, 16, !dbg !2151
  %409 = and i64 %408, 16, !dbg !2151
  %410 = icmp ne i64 %409, 0, !dbg !2151
  %411 = xor i1 %410, true, !dbg !2151
  %412 = zext i1 %410 to i32, !dbg !2151
  %413 = add nsw i32 %405, %412, !dbg !2151
  %414 = load i64, ptr %4, align 8, !dbg !2151
  %415 = lshr i64 %414, 32, !dbg !2151
  %416 = lshr i64 %415, 16, !dbg !2151
  %417 = and i64 %416, 32, !dbg !2151
  %418 = icmp ne i64 %417, 0, !dbg !2151
  %419 = xor i1 %418, true, !dbg !2151
  %420 = zext i1 %418 to i32, !dbg !2151
  %421 = add nsw i32 %413, %420, !dbg !2151
  %422 = load i64, ptr %4, align 8, !dbg !2151
  %423 = lshr i64 %422, 32, !dbg !2151
  %424 = lshr i64 %423, 16, !dbg !2151
  %425 = and i64 %424, 64, !dbg !2151
  %426 = icmp ne i64 %425, 0, !dbg !2151
  %427 = xor i1 %426, true, !dbg !2151
  %428 = zext i1 %426 to i32, !dbg !2151
  %429 = add nsw i32 %421, %428, !dbg !2151
  %430 = load i64, ptr %4, align 8, !dbg !2151
  %431 = lshr i64 %430, 32, !dbg !2151
  %432 = lshr i64 %431, 16, !dbg !2151
  %433 = and i64 %432, 128, !dbg !2151
  %434 = icmp ne i64 %433, 0, !dbg !2151
  %435 = xor i1 %434, true, !dbg !2151
  %436 = zext i1 %434 to i32, !dbg !2151
  %437 = add nsw i32 %429, %436, !dbg !2151
  %438 = load i64, ptr %4, align 8, !dbg !2151
  %439 = lshr i64 %438, 32, !dbg !2151
  %440 = lshr i64 %439, 16, !dbg !2151
  %441 = lshr i64 %440, 8, !dbg !2151
  %442 = and i64 %441, 1, !dbg !2151
  %443 = icmp ne i64 %442, 0, !dbg !2151
  %444 = xor i1 %443, true, !dbg !2151
  %445 = zext i1 %443 to i32, !dbg !2151
  %446 = load i64, ptr %4, align 8, !dbg !2151
  %447 = lshr i64 %446, 32, !dbg !2151
  %448 = lshr i64 %447, 16, !dbg !2151
  %449 = lshr i64 %448, 8, !dbg !2151
  %450 = and i64 %449, 2, !dbg !2151
  %451 = icmp ne i64 %450, 0, !dbg !2151
  %452 = xor i1 %451, true, !dbg !2151
  %453 = zext i1 %451 to i32, !dbg !2151
  %454 = add nsw i32 %445, %453, !dbg !2151
  %455 = load i64, ptr %4, align 8, !dbg !2151
  %456 = lshr i64 %455, 32, !dbg !2151
  %457 = lshr i64 %456, 16, !dbg !2151
  %458 = lshr i64 %457, 8, !dbg !2151
  %459 = and i64 %458, 4, !dbg !2151
  %460 = icmp ne i64 %459, 0, !dbg !2151
  %461 = xor i1 %460, true, !dbg !2151
  %462 = zext i1 %460 to i32, !dbg !2151
  %463 = add nsw i32 %454, %462, !dbg !2151
  %464 = load i64, ptr %4, align 8, !dbg !2151
  %465 = lshr i64 %464, 32, !dbg !2151
  %466 = lshr i64 %465, 16, !dbg !2151
  %467 = lshr i64 %466, 8, !dbg !2151
  %468 = and i64 %467, 8, !dbg !2151
  %469 = icmp ne i64 %468, 0, !dbg !2151
  %470 = xor i1 %469, true, !dbg !2151
  %471 = zext i1 %469 to i32, !dbg !2151
  %472 = add nsw i32 %463, %471, !dbg !2151
  %473 = load i64, ptr %4, align 8, !dbg !2151
  %474 = lshr i64 %473, 32, !dbg !2151
  %475 = lshr i64 %474, 16, !dbg !2151
  %476 = lshr i64 %475, 8, !dbg !2151
  %477 = and i64 %476, 16, !dbg !2151
  %478 = icmp ne i64 %477, 0, !dbg !2151
  %479 = xor i1 %478, true, !dbg !2151
  %480 = zext i1 %478 to i32, !dbg !2151
  %481 = add nsw i32 %472, %480, !dbg !2151
  %482 = load i64, ptr %4, align 8, !dbg !2151
  %483 = lshr i64 %482, 32, !dbg !2151
  %484 = lshr i64 %483, 16, !dbg !2151
  %485 = lshr i64 %484, 8, !dbg !2151
  %486 = and i64 %485, 32, !dbg !2151
  %487 = icmp ne i64 %486, 0, !dbg !2151
  %488 = xor i1 %487, true, !dbg !2151
  %489 = zext i1 %487 to i32, !dbg !2151
  %490 = add nsw i32 %481, %489, !dbg !2151
  %491 = load i64, ptr %4, align 8, !dbg !2151
  %492 = lshr i64 %491, 32, !dbg !2151
  %493 = lshr i64 %492, 16, !dbg !2151
  %494 = lshr i64 %493, 8, !dbg !2151
  %495 = and i64 %494, 64, !dbg !2151
  %496 = icmp ne i64 %495, 0, !dbg !2151
  %497 = xor i1 %496, true, !dbg !2151
  %498 = zext i1 %496 to i32, !dbg !2151
  %499 = add nsw i32 %490, %498, !dbg !2151
  %500 = load i64, ptr %4, align 8, !dbg !2151
  %501 = lshr i64 %500, 32, !dbg !2151
  %502 = lshr i64 %501, 16, !dbg !2151
  %503 = lshr i64 %502, 8, !dbg !2151
  %504 = and i64 %503, 128, !dbg !2151
  %505 = icmp ne i64 %504, 0, !dbg !2151
  %506 = xor i1 %505, true, !dbg !2151
  %507 = zext i1 %505 to i32, !dbg !2151
  %508 = add nsw i32 %499, %507, !dbg !2151
  %509 = add i32 %437, %508, !dbg !2151
  %510 = add i32 %374, %509, !dbg !2151
  %511 = add i32 %255, %510, !dbg !2151
  %512 = zext i32 %511 to i64, !dbg !2151
  br label %518, !dbg !2151

513:                                              ; preds = %21
  %514 = load i64, ptr %4, align 8, !dbg !2151
  store i64 %514, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !2152, metadata !DIExpression()), !dbg !2157
  call void @llvm.dbg.declare(metadata ptr %3, metadata !2159, metadata !DIExpression()), !dbg !2160
  %515 = load i64, ptr %2, align 8, !dbg !2161
  %516 = call i64 asm "# ALT: oldnstr\0A661:\0A\09call __sw_hweight64\0A662:\0A# ALT: padding\0A.skip -(((6651f-6641f)-(662b-661b)) > 0) * ((6651f-6641f)-(662b-661b)),0x90\0A663:\0A.pushsection .altinstructions,\22a\22\0A .long 661b - .\0A .long 6641f - .\0A .word ( 4*32+23)\0A .byte 663b-661b\0A .byte 6651f-6641f\0A .byte 663b-662b\0A.popsection\0A.pushsection .altinstr_replacement, \22ax\22\0A# ALT: replacement 1\0A6641:\0A\09popcntq $1, $0\0A6651:\0A.popsection\0A", "={ax},{di},~{dirflag},~{fpsr},~{flags}"(i64 %515) #5, !dbg !2162, !srcloc !2163
  store i64 %516, ptr %3, align 8, !dbg !2162
  %517 = load i64, ptr %3, align 8, !dbg !2164
  br label %518, !dbg !2151

518:                                              ; preds = %32, %513
  %519 = phi i64 [ %512, %32 ], [ %517, %513 ], !dbg !2151
  %520 = trunc i64 %519 to i32, !dbg !2165
  store i32 %520, ptr %5, align 4, !dbg !2166
  br label %525, !dbg !2166

521:                                              ; preds = %18, %15, %1
  %522 = load ptr, ptr %6, align 8, !dbg !2167
  %523 = load i32, ptr %7, align 4, !dbg !2168
  %524 = call i32 @__bitmap_weight(ptr noundef %522, i32 noundef %523) #4, !dbg !2169
  store i32 %524, ptr %5, align 4, !dbg !2170
  br label %525, !dbg !2170

525:                                              ; preds = %518, %521
  %526 = load i32, ptr %5, align 4, !dbg !2171
  ret i32 %526, !dbg !2172
}

; Function Attrs: convergent nocallback nofree nosync nounwind readnone willreturn
declare i1 @llvm.is.constant.i32(i32) #3

declare dso_local i32 @__bitmap_weight(ptr noundef, i32 noundef) #2

; Function Attrs: convergent nocallback nofree nosync nounwind readnone willreturn
declare i1 @llvm.is.constant.i64(i64) #3

; Function Attrs: noinline nounwind optnone uwtable
define internal void @INIT_LIST_HEAD(ptr noundef %0) #0 !dbg !2173 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.dbg.declare(metadata ptr %2, metadata !2174, metadata !DIExpression()), !dbg !2175
  br label %3, !dbg !2176

3:                                                ; preds = %1
  br label %4, !dbg !2177

4:                                                ; preds = %3
  br label %5, !dbg !2179

5:                                                ; preds = %4
  br label %6, !dbg !2177

6:                                                ; preds = %5
  %7 = load ptr, ptr %2, align 8, !dbg !2181
  %8 = load ptr, ptr %2, align 8, !dbg !2181
  %9 = getelementptr inbounds %struct.list_head, ptr %8, i32 0, i32 0, !dbg !2181
  store volatile ptr %7, ptr %9, align 8, !dbg !2181
  br label %10, !dbg !2181

10:                                               ; preds = %6
  br label %11, !dbg !2177

11:                                               ; preds = %10
  %12 = load ptr, ptr %2, align 8, !dbg !2183
  %13 = load ptr, ptr %2, align 8, !dbg !2184
  %14 = getelementptr inbounds %struct.list_head, ptr %13, i32 0, i32 1, !dbg !2185
  store ptr %12, ptr %14, align 8, !dbg !2186
  ret void, !dbg !2187
}

declare dso_local void @__raw_spin_lock_init(ptr noundef, ptr noundef, ptr noundef, i16 noundef signext) #2

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { convergent nocallback nofree nosync nounwind readnone willreturn }
attributes #4 = { nounwind }
attributes #5 = { nounwind readnone }

!llvm.dbg.cu = !{!138}
!llvm.module.flags = !{!212, !213, !214, !215, !216}
!llvm.ident = !{!217}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(scope: null, file: !2, line: 66, type: !3, isLocal: true, isDefinition: true)
!2 = !DIFile(filename: "kernel/bpf/bpf_lru_list.c", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "1ac063b7d8187dee8c4e176d54003545")
!3 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 624, elements: !5)
!4 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!5 = !{!6}
!6 = !DISubrange(count: 78)
!7 = !DIGlobalVariableExpression(var: !8, expr: !DIExpression())
!8 = distinct !DIGlobalVariable(name: "__key", scope: !9, file: !2, line: 642, type: !79, isLocal: true, isDefinition: true)
!9 = distinct !DISubprogram(name: "bpf_lru_list_init", scope: !2, file: !2, line: 630, type: !10, scopeLine: 631, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!10 = !DISubroutineType(types: !11)
!11 = !{null, !12}
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bpf_lru_list", file: !14, line: 30, size: 1536, elements: !15)
!14 = !DIFile(filename: "kernel/bpf/bpf_lru_list.h", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "bb5526030aeb6ee66085f674eff05ba3")
!15 = !{!16, !26, !31, !32}
!16 = !DIDerivedType(tag: DW_TAG_member, name: "lists", scope: !13, file: !14, line: 31, baseType: !17, size: 384)
!17 = !DICompositeType(tag: DW_TAG_array_type, baseType: !18, size: 384, elements: !24)
!18 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "list_head", file: !19, line: 178, size: 128, elements: !20)
!19 = !DIFile(filename: "include/linux/types.h", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "d48c365ef36f47a7a868e8af02ae7515")
!20 = !{!21, !23}
!21 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !18, file: !19, line: 179, baseType: !22, size: 64)
!22 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !18, size: 64)
!23 = !DIDerivedType(tag: DW_TAG_member, name: "prev", scope: !18, file: !19, line: 179, baseType: !22, size: 64, offset: 64)
!24 = !{!25}
!25 = !DISubrange(count: 3)
!26 = !DIDerivedType(tag: DW_TAG_member, name: "counts", scope: !13, file: !14, line: 32, baseType: !27, size: 64, offset: 384)
!27 = !DICompositeType(tag: DW_TAG_array_type, baseType: !28, size: 64, elements: !29)
!28 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!29 = !{!30}
!30 = !DISubrange(count: 2)
!31 = !DIDerivedType(tag: DW_TAG_member, name: "next_inactive_rotation", scope: !13, file: !14, line: 34, baseType: !22, size: 64, offset: 448)
!32 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !13, file: !14, line: 36, baseType: !33, size: 576, align: 512, offset: 512)
!33 = !DIDerivedType(tag: DW_TAG_typedef, name: "raw_spinlock_t", file: !34, line: 29, baseType: !35)
!34 = !DIFile(filename: "include/linux/spinlock_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "d7c4af7be2220310cae8fcee28d17930")
!35 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "raw_spinlock", file: !34, line: 20, size: 576, elements: !36)
!36 = !{!37, !69, !70, !71, !73}
!37 = !DIDerivedType(tag: DW_TAG_member, name: "raw_lock", scope: !35, file: !34, line: 21, baseType: !38, size: 32)
!38 = !DIDerivedType(tag: DW_TAG_typedef, name: "arch_spinlock_t", file: !39, line: 44, baseType: !40)
!39 = !DIFile(filename: "include/asm-generic/qspinlock_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "2a1236eda9a125c2ce03b9a345491b46")
!40 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "qspinlock", file: !39, line: 14, size: 32, elements: !41)
!41 = !{!42}
!42 = !DIDerivedType(tag: DW_TAG_member, scope: !40, file: !39, line: 15, baseType: !43, size: 32)
!43 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !40, file: !39, line: 15, size: 32, elements: !44)
!44 = !{!45, !51, !61}
!45 = !DIDerivedType(tag: DW_TAG_member, name: "val", scope: !43, file: !39, line: 16, baseType: !46, size: 32)
!46 = !DIDerivedType(tag: DW_TAG_typedef, name: "atomic_t", file: !19, line: 168, baseType: !47)
!47 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !19, line: 166, size: 32, elements: !48)
!48 = !{!49}
!49 = !DIDerivedType(tag: DW_TAG_member, name: "counter", scope: !47, file: !19, line: 167, baseType: !50, size: 32)
!50 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!51 = !DIDerivedType(tag: DW_TAG_member, scope: !43, file: !39, line: 24, baseType: !52, size: 16)
!52 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !43, file: !39, line: 24, size: 16, elements: !53)
!53 = !{!54, !60}
!54 = !DIDerivedType(tag: DW_TAG_member, name: "locked", scope: !52, file: !39, line: 25, baseType: !55, size: 8)
!55 = !DIDerivedType(tag: DW_TAG_typedef, name: "u8", file: !56, line: 17, baseType: !57)
!56 = !DIFile(filename: "include/asm-generic/int-ll64.h", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "12ca7bdb629352cc4c9a492f86b435a7")
!57 = !DIDerivedType(tag: DW_TAG_typedef, name: "__u8", file: !58, line: 21, baseType: !59)
!58 = !DIFile(filename: "include/uapi/asm-generic/int-ll64.h", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "f4d0ec5bcdd84e825a78a7add39d54dd")
!59 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!60 = !DIDerivedType(tag: DW_TAG_member, name: "pending", scope: !52, file: !39, line: 26, baseType: !55, size: 8, offset: 8)
!61 = !DIDerivedType(tag: DW_TAG_member, scope: !43, file: !39, line: 28, baseType: !62, size: 32)
!62 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !43, file: !39, line: 28, size: 32, elements: !63)
!63 = !{!64, !68}
!64 = !DIDerivedType(tag: DW_TAG_member, name: "locked_pending", scope: !62, file: !39, line: 29, baseType: !65, size: 16)
!65 = !DIDerivedType(tag: DW_TAG_typedef, name: "u16", file: !56, line: 19, baseType: !66)
!66 = !DIDerivedType(tag: DW_TAG_typedef, name: "__u16", file: !58, line: 24, baseType: !67)
!67 = !DIBasicType(name: "unsigned short", size: 16, encoding: DW_ATE_unsigned)
!68 = !DIDerivedType(tag: DW_TAG_member, name: "tail", scope: !62, file: !39, line: 30, baseType: !65, size: 16, offset: 16)
!69 = !DIDerivedType(tag: DW_TAG_member, name: "magic", scope: !35, file: !34, line: 23, baseType: !28, size: 32, offset: 32)
!70 = !DIDerivedType(tag: DW_TAG_member, name: "owner_cpu", scope: !35, file: !34, line: 23, baseType: !28, size: 32, offset: 64)
!71 = !DIDerivedType(tag: DW_TAG_member, name: "owner", scope: !35, file: !34, line: 24, baseType: !72, size: 64, offset: 128)
!72 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!73 = !DIDerivedType(tag: DW_TAG_member, name: "dep_map", scope: !35, file: !34, line: 27, baseType: !74, size: 384, offset: 192)
!74 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "lockdep_map", file: !75, line: 168, size: 384, elements: !76)
!75 = !DIFile(filename: "include/linux/lockdep_types.h", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "9b426fb7920b0220b893addc4ff0c7bb")
!76 = !{!77, !98, !133, !134, !135, !136, !137}
!77 = !DIDerivedType(tag: DW_TAG_member, name: "key", scope: !74, file: !75, line: 169, baseType: !78, size: 64)
!78 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !79, size: 64)
!79 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "lock_class_key", file: !75, line: 68, size: 128, elements: !80)
!80 = !{!81}
!81 = !DIDerivedType(tag: DW_TAG_member, scope: !79, file: !75, line: 69, baseType: !82, size: 128)
!82 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !79, file: !75, line: 69, size: 128, elements: !83)
!83 = !{!84, !91}
!84 = !DIDerivedType(tag: DW_TAG_member, name: "hash_entry", scope: !82, file: !75, line: 70, baseType: !85, size: 128)
!85 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "hlist_node", file: !19, line: 186, size: 128, elements: !86)
!86 = !{!87, !89}
!87 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !85, file: !19, line: 187, baseType: !88, size: 64)
!88 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !85, size: 64)
!89 = !DIDerivedType(tag: DW_TAG_member, name: "pprev", scope: !85, file: !19, line: 187, baseType: !90, size: 64, offset: 64)
!90 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !88, size: 64)
!91 = !DIDerivedType(tag: DW_TAG_member, name: "subkeys", scope: !82, file: !75, line: 71, baseType: !92, size: 64)
!92 = !DICompositeType(tag: DW_TAG_array_type, baseType: !93, size: 64, elements: !96)
!93 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "lockdep_subclass_key", file: !75, line: 63, size: 8, elements: !94)
!94 = !{!95}
!95 = !DIDerivedType(tag: DW_TAG_member, name: "__one_byte", scope: !93, file: !75, line: 64, baseType: !4, size: 8)
!96 = !{!97}
!97 = !DISubrange(count: 8)
!98 = !DIDerivedType(tag: DW_TAG_member, name: "class_cache", scope: !74, file: !75, line: 170, baseType: !99, size: 128, offset: 64)
!99 = !DICompositeType(tag: DW_TAG_array_type, baseType: !100, size: 128, elements: !29)
!100 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !101, size: 64)
!101 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "lock_class", file: !75, line: 85, size: 2048, elements: !102)
!102 = !{!103, !104, !105, !106, !107, !110, !111, !112, !114, !121, !122, !125, !127, !128, !132}
!103 = !DIDerivedType(tag: DW_TAG_member, name: "hash_entry", scope: !101, file: !75, line: 89, baseType: !85, size: 128)
!104 = !DIDerivedType(tag: DW_TAG_member, name: "lock_entry", scope: !101, file: !75, line: 96, baseType: !18, size: 128, offset: 128)
!105 = !DIDerivedType(tag: DW_TAG_member, name: "locks_after", scope: !101, file: !75, line: 103, baseType: !18, size: 128, offset: 256)
!106 = !DIDerivedType(tag: DW_TAG_member, name: "locks_before", scope: !101, file: !75, line: 103, baseType: !18, size: 128, offset: 384)
!107 = !DIDerivedType(tag: DW_TAG_member, name: "key", scope: !101, file: !75, line: 105, baseType: !108, size: 64, offset: 512)
!108 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !109, size: 64)
!109 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !93)
!110 = !DIDerivedType(tag: DW_TAG_member, name: "subclass", scope: !101, file: !75, line: 106, baseType: !28, size: 32, offset: 576)
!111 = !DIDerivedType(tag: DW_TAG_member, name: "dep_gen_id", scope: !101, file: !75, line: 107, baseType: !28, size: 32, offset: 608)
!112 = !DIDerivedType(tag: DW_TAG_member, name: "usage_mask", scope: !101, file: !75, line: 112, baseType: !113, size: 64, offset: 640)
!113 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!114 = !DIDerivedType(tag: DW_TAG_member, name: "usage_traces", scope: !101, file: !75, line: 113, baseType: !115, size: 640, offset: 704)
!115 = !DICompositeType(tag: DW_TAG_array_type, baseType: !116, size: 640, elements: !119)
!116 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !117, size: 64)
!117 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !118)
!118 = !DICompositeType(tag: DW_TAG_structure_type, name: "lock_trace", file: !75, line: 77, flags: DIFlagFwdDecl)
!119 = !{!120}
!120 = !DISubrange(count: 10)
!121 = !DIDerivedType(tag: DW_TAG_member, name: "name_version", scope: !101, file: !75, line: 119, baseType: !50, size: 32, offset: 1344)
!122 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !101, file: !75, line: 120, baseType: !123, size: 64, offset: 1408)
!123 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !124, size: 64)
!124 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !4)
!125 = !DIDerivedType(tag: DW_TAG_member, name: "wait_type_inner", scope: !101, file: !75, line: 122, baseType: !126, size: 16, offset: 1472)
!126 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!127 = !DIDerivedType(tag: DW_TAG_member, name: "wait_type_outer", scope: !101, file: !75, line: 123, baseType: !126, size: 16, offset: 1488)
!128 = !DIDerivedType(tag: DW_TAG_member, name: "contention_point", scope: !101, file: !75, line: 126, baseType: !129, size: 256, offset: 1536)
!129 = !DICompositeType(tag: DW_TAG_array_type, baseType: !113, size: 256, elements: !130)
!130 = !{!131}
!131 = !DISubrange(count: 4)
!132 = !DIDerivedType(tag: DW_TAG_member, name: "contending_point", scope: !101, file: !75, line: 127, baseType: !129, size: 256, offset: 1792)
!133 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !74, file: !75, line: 171, baseType: !123, size: 64, offset: 192)
!134 = !DIDerivedType(tag: DW_TAG_member, name: "wait_type_outer", scope: !74, file: !75, line: 172, baseType: !126, size: 16, offset: 256)
!135 = !DIDerivedType(tag: DW_TAG_member, name: "wait_type_inner", scope: !74, file: !75, line: 173, baseType: !126, size: 16, offset: 272)
!136 = !DIDerivedType(tag: DW_TAG_member, name: "cpu", scope: !74, file: !75, line: 175, baseType: !50, size: 32, offset: 288)
!137 = !DIDerivedType(tag: DW_TAG_member, name: "ip", scope: !74, file: !75, line: 176, baseType: !113, size: 64, offset: 320)
!138 = distinct !DICompileUnit(language: DW_LANG_C99, file: !139, producer: "Debian clang version 15.0.6", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !140, retainedTypes: !161, globals: !195, splitDebugInlining: false, nameTableKind: None)
!139 = !DIFile(filename: "/mlx_devbox/users/mayunlong.39/playground/linux.git/kernel/bpf/bpf_lru_list.c", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "1ac063b7d8187dee8c4e176d54003545")
!140 = !{!141, !148, !153}
!141 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "bpf_lru_list_type", file: !14, line: 15, baseType: !28, size: 32, elements: !142)
!142 = !{!143, !144, !145, !146, !147}
!143 = !DIEnumerator(name: "BPF_LRU_LIST_T_ACTIVE", value: 0)
!144 = !DIEnumerator(name: "BPF_LRU_LIST_T_INACTIVE", value: 1)
!145 = !DIEnumerator(name: "BPF_LRU_LIST_T_FREE", value: 2)
!146 = !DIEnumerator(name: "BPF_LRU_LOCAL_LIST_T_FREE", value: 3)
!147 = !DIEnumerator(name: "BPF_LRU_LOCAL_LIST_T_PENDING", value: 4)
!148 = !DICompositeType(tag: DW_TAG_enumeration_type, file: !149, line: 10, baseType: !28, size: 32, elements: !150)
!149 = !DIFile(filename: "include/linux/stddef.h", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "675e289a7c8d2e9d577b6f883c583760")
!150 = !{!151, !152}
!151 = !DIEnumerator(name: "false", value: 0)
!152 = !DIEnumerator(name: "true", value: 1)
!153 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "lockdep_wait_type", file: !75, line: 17, baseType: !28, size: 32, elements: !154)
!154 = !{!155, !156, !157, !158, !159, !160}
!155 = !DIEnumerator(name: "LD_WAIT_INV", value: 0)
!156 = !DIEnumerator(name: "LD_WAIT_FREE", value: 1)
!157 = !DIEnumerator(name: "LD_WAIT_SPIN", value: 2)
!158 = !DIEnumerator(name: "LD_WAIT_CONFIG", value: 3)
!159 = !DIEnumerator(name: "LD_WAIT_SLEEP", value: 4)
!160 = !DIEnumerator(name: "LD_WAIT_MAX", value: 5)
!161 = !{!12, !162, !72, !113, !171, !178, !50, !179, !186, !189, !192, !193, !28}
!162 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !163, size: 64)
!163 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !164)
!164 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "cpumask", file: !165, line: 17, size: 8192, elements: !166)
!165 = !DIFile(filename: "include/linux/cpumask.h", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "05b7885ea96cbc570196a200b9d16d25")
!166 = !{!167}
!167 = !DIDerivedType(tag: DW_TAG_member, name: "bits", scope: !164, file: !165, line: 17, baseType: !168, size: 8192)
!168 = !DICompositeType(tag: DW_TAG_array_type, baseType: !113, size: 8192, elements: !169)
!169 = !{!170}
!170 = !DISubrange(count: 128)
!171 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !172, size: 64)
!172 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bpf_lru_locallist", file: !14, line: 39, size: 896, elements: !173)
!173 = !{!174, !176, !177}
!174 = !DIDerivedType(tag: DW_TAG_member, name: "lists", scope: !172, file: !14, line: 40, baseType: !175, size: 256)
!175 = !DICompositeType(tag: DW_TAG_array_type, baseType: !18, size: 256, elements: !29)
!176 = !DIDerivedType(tag: DW_TAG_member, name: "next_steal", scope: !172, file: !14, line: 41, baseType: !65, size: 16, offset: 256)
!177 = !DIDerivedType(tag: DW_TAG_member, name: "lock", scope: !172, file: !14, line: 42, baseType: !33, size: 576, offset: 320)
!178 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !50, size: 64)
!179 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !180, size: 64)
!180 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bpf_lru_node", file: !14, line: 23, size: 192, elements: !181)
!181 = !{!182, !183, !184, !185}
!182 = !DIDerivedType(tag: DW_TAG_member, name: "list", scope: !180, file: !14, line: 24, baseType: !18, size: 128)
!183 = !DIDerivedType(tag: DW_TAG_member, name: "cpu", scope: !180, file: !14, line: 25, baseType: !65, size: 16, offset: 128)
!184 = !DIDerivedType(tag: DW_TAG_member, name: "type", scope: !180, file: !14, line: 26, baseType: !55, size: 8, offset: 144)
!185 = !DIDerivedType(tag: DW_TAG_member, name: "ref", scope: !180, file: !14, line: 27, baseType: !55, size: 8, offset: 152)
!186 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !187, size: 64)
!187 = !DIDerivedType(tag: DW_TAG_typedef, name: "u32", file: !56, line: 21, baseType: !188)
!188 = !DIDerivedType(tag: DW_TAG_typedef, name: "__u32", file: !58, line: 27, baseType: !28)
!189 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !190, size: 64)
!190 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !191)
!191 = !DIDerivedType(tag: DW_TAG_volatile_type, baseType: !22)
!192 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !191, size: 64)
!193 = !DIDerivedType(tag: DW_TAG_typedef, name: "__u64", file: !58, line: 31, baseType: !194)
!194 = !DIBasicType(name: "unsigned long long", size: 64, encoding: DW_ATE_unsigned)
!195 = !{!0, !7, !196, !201, !207}
!196 = !DIGlobalVariableExpression(var: !197, expr: !DIExpression())
!197 = distinct !DIGlobalVariable(scope: null, file: !2, line: 642, type: !198, isLocal: true, isDefinition: true)
!198 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 72, elements: !199)
!199 = !{!200}
!200 = !DISubrange(count: 9)
!201 = !DIGlobalVariableExpression(var: !202, expr: !DIExpression())
!202 = distinct !DIGlobalVariable(name: "__key", scope: !203, file: !2, line: 627, type: !79, isLocal: true, isDefinition: true)
!203 = distinct !DISubprogram(name: "bpf_lru_locallist_init", scope: !2, file: !2, line: 618, type: !204, scopeLine: 619, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!204 = !DISubroutineType(types: !205)
!205 = !{null, !171, !50}
!206 = !{}
!207 = !DIGlobalVariableExpression(var: !208, expr: !DIExpression())
!208 = distinct !DIGlobalVariable(scope: null, file: !2, line: 627, type: !209, isLocal: true, isDefinition: true)
!209 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 104, elements: !210)
!210 = !{!211}
!211 = !DISubrange(count: 13)
!212 = !{i32 7, !"Dwarf Version", i32 5}
!213 = !{i32 2, !"Debug Info Version", i32 3}
!214 = !{i32 1, !"wchar_size", i32 4}
!215 = !{i32 7, !"uwtable", i32 2}
!216 = !{i32 7, !"frame-pointer", i32 2}
!217 = !{!"Debian clang version 15.0.6"}
!218 = distinct !DISubprogram(name: "bpf_lru_pop_free", scope: !2, file: !2, line: 494, type: !219, scopeLine: 495, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !138, retainedNodes: !206)
!219 = !DISubroutineType(types: !220)
!220 = !{!179, !221, !187}
!221 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !222, size: 64)
!222 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bpf_lru", file: !14, line: 52, size: 2560, elements: !223)
!223 = !{!224, !233, !240, !241, !242, !243}
!224 = !DIDerivedType(tag: DW_TAG_member, scope: !222, file: !14, line: 53, baseType: !225, size: 2048)
!225 = distinct !DICompositeType(tag: DW_TAG_union_type, scope: !222, file: !14, line: 53, size: 2048, elements: !226)
!226 = !{!227, !232}
!227 = !DIDerivedType(tag: DW_TAG_member, name: "common_lru", scope: !225, file: !14, line: 54, baseType: !228, size: 2048)
!228 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bpf_common_lru", file: !14, line: 45, size: 2048, elements: !229)
!229 = !{!230, !231}
!230 = !DIDerivedType(tag: DW_TAG_member, name: "lru_list", scope: !228, file: !14, line: 46, baseType: !13, size: 1536)
!231 = !DIDerivedType(tag: DW_TAG_member, name: "local_list", scope: !228, file: !14, line: 47, baseType: !171, size: 64, offset: 1536)
!232 = !DIDerivedType(tag: DW_TAG_member, name: "percpu_lru", scope: !225, file: !14, line: 55, baseType: !12, size: 64)
!233 = !DIDerivedType(tag: DW_TAG_member, name: "del_from_htab", scope: !222, file: !14, line: 57, baseType: !234, size: 64, offset: 2048)
!234 = !DIDerivedType(tag: DW_TAG_typedef, name: "del_from_htab_func", file: !14, line: 50, baseType: !235)
!235 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !236, size: 64)
!236 = !DISubroutineType(types: !237)
!237 = !{!238, !72, !179}
!238 = !DIDerivedType(tag: DW_TAG_typedef, name: "bool", file: !19, line: 30, baseType: !239)
!239 = !DIBasicType(name: "_Bool", size: 8, encoding: DW_ATE_boolean)
!240 = !DIDerivedType(tag: DW_TAG_member, name: "del_arg", scope: !222, file: !14, line: 58, baseType: !72, size: 64, offset: 2112)
!241 = !DIDerivedType(tag: DW_TAG_member, name: "hash_offset", scope: !222, file: !14, line: 59, baseType: !28, size: 32, offset: 2176)
!242 = !DIDerivedType(tag: DW_TAG_member, name: "nr_scans", scope: !222, file: !14, line: 60, baseType: !28, size: 32, offset: 2208)
!243 = !DIDerivedType(tag: DW_TAG_member, name: "percpu", scope: !222, file: !14, line: 61, baseType: !238, size: 8, offset: 2240)
!244 = !DILocalVariable(name: "lru", arg: 1, scope: !218, file: !2, line: 494, type: !221)
!245 = !DILocation(line: 494, column: 55, scope: !218)
!246 = !DILocalVariable(name: "hash", arg: 2, scope: !218, file: !2, line: 494, type: !187)
!247 = !DILocation(line: 494, column: 64, scope: !218)
!248 = !DILocation(line: 496, column: 6, scope: !249)
!249 = distinct !DILexicalBlock(scope: !218, file: !2, line: 496, column: 6)
!250 = !DILocation(line: 496, column: 11, scope: !249)
!251 = !DILocation(line: 496, column: 6, scope: !218)
!252 = !DILocation(line: 497, column: 34, scope: !249)
!253 = !DILocation(line: 497, column: 39, scope: !249)
!254 = !DILocation(line: 497, column: 10, scope: !249)
!255 = !DILocation(line: 497, column: 3, scope: !249)
!256 = !DILocation(line: 499, column: 34, scope: !249)
!257 = !DILocation(line: 499, column: 39, scope: !249)
!258 = !DILocation(line: 499, column: 10, scope: !249)
!259 = !DILocation(line: 499, column: 3, scope: !249)
!260 = !DILocation(line: 500, column: 1, scope: !218)
!261 = distinct !DISubprogram(name: "bpf_percpu_lru_pop_free", scope: !2, file: !2, line: 399, type: !219, scopeLine: 401, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!262 = !DILocalVariable(name: "lru", arg: 1, scope: !261, file: !2, line: 399, type: !221)
!263 = !DILocation(line: 399, column: 69, scope: !261)
!264 = !DILocalVariable(name: "hash", arg: 2, scope: !261, file: !2, line: 400, type: !187)
!265 = !DILocation(line: 400, column: 15, scope: !261)
!266 = !DILocalVariable(name: "free_list", scope: !261, file: !2, line: 402, type: !22)
!267 = !DILocation(line: 402, column: 20, scope: !261)
!268 = !DILocalVariable(name: "node", scope: !261, file: !2, line: 403, type: !179)
!269 = !DILocation(line: 403, column: 23, scope: !261)
!270 = !DILocalVariable(name: "l", scope: !261, file: !2, line: 404, type: !12)
!271 = !DILocation(line: 404, column: 23, scope: !261)
!272 = !DILocalVariable(name: "flags", scope: !261, file: !2, line: 405, type: !113)
!273 = !DILocation(line: 405, column: 16, scope: !261)
!274 = !DILocalVariable(name: "cpu", scope: !261, file: !2, line: 406, type: !50)
!275 = !DILocation(line: 406, column: 6, scope: !261)
!276 = !DILocalVariable(name: "pscr_ret__", scope: !277, file: !2, line: 406, type: !50)
!277 = distinct !DILexicalBlock(scope: !261, file: !2, line: 406, column: 12)
!278 = !DILocation(line: 406, column: 12, scope: !277)
!279 = !DILocalVariable(name: "__vpp_verify", scope: !280, file: !2, line: 406, type: !281)
!280 = distinct !DILexicalBlock(scope: !277, file: !2, line: 406, column: 12)
!281 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !282, size: 64)
!282 = !DIDerivedType(tag: DW_TAG_const_type, baseType: null)
!283 = !DILocation(line: 406, column: 12, scope: !280)
!284 = !DILocalVariable(name: "pfo_val__", scope: !285, file: !2, line: 406, type: !187)
!285 = distinct !DILexicalBlock(scope: !277, file: !2, line: 406, column: 12)
!286 = !DILocation(line: 406, column: 12, scope: !285)
!287 = !{i64 2150409856}
!288 = !DILocation(line: 408, column: 6, scope: !289)
!289 = distinct !DILexicalBlock(scope: !261, file: !2, line: 408, column: 6)
!290 = !DILocalVariable(name: "__vpp_verify", scope: !291, file: !2, line: 408, type: !281)
!291 = distinct !DILexicalBlock(scope: !289, file: !2, line: 408, column: 6)
!292 = !DILocation(line: 408, column: 6, scope: !291)
!293 = !DILocalVariable(name: "__ptr", scope: !294, file: !2, line: 408, type: !113)
!294 = distinct !DILexicalBlock(scope: !289, file: !2, line: 408, column: 6)
!295 = !DILocation(line: 408, column: 6, scope: !294)
!296 = !DILocation(line: 408, column: 4, scope: !261)
!297 = !DILocation(line: 410, column: 2, scope: !261)
!298 = !DILocalVariable(name: "__dummy", scope: !299, file: !2, line: 410, type: !113)
!299 = distinct !DILexicalBlock(scope: !300, file: !2, line: 410, column: 2)
!300 = distinct !DILexicalBlock(scope: !261, file: !2, line: 410, column: 2)
!301 = !DILocation(line: 410, column: 2, scope: !299)
!302 = !DILocalVariable(name: "__dummy2", scope: !299, file: !2, line: 410, type: !113)
!303 = !DILocation(line: 410, column: 2, scope: !300)
!304 = !DILocation(line: 412, column: 24, scope: !261)
!305 = !DILocation(line: 412, column: 29, scope: !261)
!306 = !DILocation(line: 412, column: 2, scope: !261)
!307 = !DILocation(line: 414, column: 15, scope: !261)
!308 = !DILocation(line: 414, column: 18, scope: !261)
!309 = !DILocation(line: 414, column: 12, scope: !261)
!310 = !DILocation(line: 415, column: 17, scope: !311)
!311 = distinct !DILexicalBlock(scope: !261, file: !2, line: 415, column: 6)
!312 = !DILocation(line: 415, column: 6, scope: !311)
!313 = !DILocation(line: 415, column: 6, scope: !261)
!314 = !DILocation(line: 416, column: 25, scope: !311)
!315 = !DILocation(line: 416, column: 30, scope: !311)
!316 = !DILocation(line: 416, column: 53, scope: !311)
!317 = !DILocation(line: 416, column: 3, scope: !311)
!318 = !DILocation(line: 419, column: 18, scope: !319)
!319 = distinct !DILexicalBlock(scope: !261, file: !2, line: 419, column: 6)
!320 = !DILocation(line: 419, column: 7, scope: !319)
!321 = !DILocation(line: 419, column: 6, scope: !261)
!322 = !DILocalVariable(name: "__mptr", scope: !323, file: !2, line: 420, type: !72)
!323 = distinct !DILexicalBlock(scope: !324, file: !2, line: 420, column: 10)
!324 = distinct !DILexicalBlock(scope: !319, file: !2, line: 419, column: 30)
!325 = !DILocation(line: 420, column: 10, scope: !323)
!326 = !DILocation(line: 420, column: 10, scope: !327)
!327 = distinct !DILexicalBlock(scope: !323, file: !2, line: 420, column: 10)
!328 = !DILocation(line: 420, column: 8, scope: !324)
!329 = !DILocation(line: 421, column: 47, scope: !324)
!330 = !DILocation(line: 421, column: 20, scope: !324)
!331 = !DILocation(line: 421, column: 27, scope: !324)
!332 = !DILocation(line: 421, column: 32, scope: !324)
!333 = !DILocation(line: 421, column: 25, scope: !324)
!334 = !DILocation(line: 421, column: 45, scope: !324)
!335 = !DILocation(line: 422, column: 3, scope: !324)
!336 = !DILocation(line: 422, column: 9, scope: !324)
!337 = !DILocation(line: 422, column: 13, scope: !324)
!338 = !DILocation(line: 423, column: 23, scope: !324)
!339 = !DILocation(line: 423, column: 26, scope: !324)
!340 = !DILocation(line: 423, column: 3, scope: !324)
!341 = !DILocation(line: 424, column: 2, scope: !324)
!342 = !DILocation(line: 426, column: 2, scope: !261)
!343 = !DILocalVariable(name: "__dummy", scope: !344, file: !2, line: 426, type: !113)
!344 = distinct !DILexicalBlock(scope: !345, file: !2, line: 426, column: 2)
!345 = distinct !DILexicalBlock(scope: !261, file: !2, line: 426, column: 2)
!346 = !DILocation(line: 426, column: 2, scope: !344)
!347 = !DILocalVariable(name: "__dummy2", scope: !344, file: !2, line: 426, type: !113)
!348 = !DILocation(line: 426, column: 2, scope: !345)
!349 = !DILocation(line: 428, column: 9, scope: !261)
!350 = !DILocation(line: 428, column: 2, scope: !261)
!351 = distinct !DISubprogram(name: "bpf_common_lru_pop_free", scope: !2, file: !2, line: 431, type: !219, scopeLine: 433, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!352 = !DILocalVariable(name: "lru", arg: 1, scope: !351, file: !2, line: 431, type: !221)
!353 = !DILocation(line: 431, column: 69, scope: !351)
!354 = !DILocalVariable(name: "hash", arg: 2, scope: !351, file: !2, line: 432, type: !187)
!355 = !DILocation(line: 432, column: 15, scope: !351)
!356 = !DILocalVariable(name: "loc_l", scope: !351, file: !2, line: 434, type: !171)
!357 = !DILocation(line: 434, column: 28, scope: !351)
!358 = !DILocalVariable(name: "steal_loc_l", scope: !351, file: !2, line: 434, type: !171)
!359 = !DILocation(line: 434, column: 36, scope: !351)
!360 = !DILocalVariable(name: "clru", scope: !351, file: !2, line: 435, type: !361)
!361 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !228, size: 64)
!362 = !DILocation(line: 435, column: 25, scope: !351)
!363 = !DILocation(line: 435, column: 33, scope: !351)
!364 = !DILocation(line: 435, column: 38, scope: !351)
!365 = !DILocalVariable(name: "node", scope: !351, file: !2, line: 436, type: !179)
!366 = !DILocation(line: 436, column: 23, scope: !351)
!367 = !DILocalVariable(name: "steal", scope: !351, file: !2, line: 437, type: !50)
!368 = !DILocation(line: 437, column: 6, scope: !351)
!369 = !DILocalVariable(name: "first_steal", scope: !351, file: !2, line: 437, type: !50)
!370 = !DILocation(line: 437, column: 13, scope: !351)
!371 = !DILocalVariable(name: "flags", scope: !351, file: !2, line: 438, type: !113)
!372 = !DILocation(line: 438, column: 16, scope: !351)
!373 = !DILocalVariable(name: "cpu", scope: !351, file: !2, line: 439, type: !50)
!374 = !DILocation(line: 439, column: 6, scope: !351)
!375 = !DILocalVariable(name: "pscr_ret__", scope: !376, file: !2, line: 439, type: !50)
!376 = distinct !DILexicalBlock(scope: !351, file: !2, line: 439, column: 12)
!377 = !DILocation(line: 439, column: 12, scope: !376)
!378 = !DILocalVariable(name: "__vpp_verify", scope: !379, file: !2, line: 439, type: !281)
!379 = distinct !DILexicalBlock(scope: !376, file: !2, line: 439, column: 12)
!380 = !DILocation(line: 439, column: 12, scope: !379)
!381 = !DILocalVariable(name: "pfo_val__", scope: !382, file: !2, line: 439, type: !187)
!382 = distinct !DILexicalBlock(scope: !376, file: !2, line: 439, column: 12)
!383 = !DILocation(line: 439, column: 12, scope: !382)
!384 = !{i64 2150416005}
!385 = !DILocation(line: 441, column: 10, scope: !386)
!386 = distinct !DILexicalBlock(scope: !351, file: !2, line: 441, column: 10)
!387 = !DILocalVariable(name: "__vpp_verify", scope: !388, file: !2, line: 441, type: !281)
!388 = distinct !DILexicalBlock(scope: !386, file: !2, line: 441, column: 10)
!389 = !DILocation(line: 441, column: 10, scope: !388)
!390 = !DILocalVariable(name: "__ptr", scope: !391, file: !2, line: 441, type: !113)
!391 = distinct !DILexicalBlock(scope: !386, file: !2, line: 441, column: 10)
!392 = !DILocation(line: 441, column: 10, scope: !391)
!393 = !DILocation(line: 441, column: 8, scope: !351)
!394 = !DILocation(line: 443, column: 2, scope: !351)
!395 = !DILocalVariable(name: "__dummy", scope: !396, file: !2, line: 443, type: !113)
!396 = distinct !DILexicalBlock(scope: !397, file: !2, line: 443, column: 2)
!397 = distinct !DILexicalBlock(scope: !351, file: !2, line: 443, column: 2)
!398 = !DILocation(line: 443, column: 2, scope: !396)
!399 = !DILocalVariable(name: "__dummy2", scope: !396, file: !2, line: 443, type: !113)
!400 = !DILocation(line: 443, column: 2, scope: !397)
!401 = !DILocation(line: 445, column: 31, scope: !351)
!402 = !DILocation(line: 445, column: 9, scope: !351)
!403 = !DILocation(line: 445, column: 7, scope: !351)
!404 = !DILocation(line: 446, column: 7, scope: !405)
!405 = distinct !DILexicalBlock(scope: !351, file: !2, line: 446, column: 6)
!406 = !DILocation(line: 446, column: 6, scope: !351)
!407 = !DILocation(line: 447, column: 34, scope: !408)
!408 = distinct !DILexicalBlock(scope: !405, file: !2, line: 446, column: 13)
!409 = !DILocation(line: 447, column: 39, scope: !408)
!410 = !DILocation(line: 447, column: 3, scope: !408)
!411 = !DILocation(line: 448, column: 32, scope: !408)
!412 = !DILocation(line: 448, column: 10, scope: !408)
!413 = !DILocation(line: 448, column: 8, scope: !408)
!414 = !DILocation(line: 449, column: 2, scope: !408)
!415 = !DILocation(line: 451, column: 6, scope: !416)
!416 = distinct !DILexicalBlock(scope: !351, file: !2, line: 451, column: 6)
!417 = !DILocation(line: 451, column: 6, scope: !351)
!418 = !DILocation(line: 452, column: 28, scope: !416)
!419 = !DILocation(line: 452, column: 33, scope: !416)
!420 = !DILocation(line: 452, column: 40, scope: !416)
!421 = !DILocation(line: 452, column: 45, scope: !416)
!422 = !DILocation(line: 452, column: 51, scope: !416)
!423 = !DILocation(line: 452, column: 3, scope: !416)
!424 = !DILocation(line: 454, column: 2, scope: !351)
!425 = !DILocalVariable(name: "__dummy", scope: !426, file: !2, line: 454, type: !113)
!426 = distinct !DILexicalBlock(scope: !427, file: !2, line: 454, column: 2)
!427 = distinct !DILexicalBlock(scope: !351, file: !2, line: 454, column: 2)
!428 = !DILocation(line: 454, column: 2, scope: !426)
!429 = !DILocalVariable(name: "__dummy2", scope: !426, file: !2, line: 454, type: !113)
!430 = !DILocation(line: 454, column: 2, scope: !427)
!431 = !DILocation(line: 456, column: 6, scope: !432)
!432 = distinct !DILexicalBlock(scope: !351, file: !2, line: 456, column: 6)
!433 = !DILocation(line: 456, column: 6, scope: !351)
!434 = !DILocation(line: 457, column: 10, scope: !432)
!435 = !DILocation(line: 457, column: 3, scope: !432)
!436 = !DILocation(line: 467, column: 16, scope: !351)
!437 = !DILocation(line: 467, column: 23, scope: !351)
!438 = !DILocation(line: 467, column: 14, scope: !351)
!439 = !DILocation(line: 468, column: 10, scope: !351)
!440 = !DILocation(line: 468, column: 8, scope: !351)
!441 = !DILocation(line: 469, column: 2, scope: !351)
!442 = !DILocation(line: 470, column: 17, scope: !443)
!443 = distinct !DILexicalBlock(scope: !444, file: !2, line: 470, column: 17)
!444 = distinct !DILexicalBlock(scope: !351, file: !2, line: 469, column: 5)
!445 = !DILocalVariable(name: "__vpp_verify", scope: !446, file: !2, line: 470, type: !281)
!446 = distinct !DILexicalBlock(scope: !443, file: !2, line: 470, column: 17)
!447 = !DILocation(line: 470, column: 17, scope: !446)
!448 = !DILocalVariable(name: "__ptr", scope: !449, file: !2, line: 470, type: !113)
!449 = distinct !DILexicalBlock(scope: !443, file: !2, line: 470, column: 17)
!450 = !DILocation(line: 470, column: 17, scope: !449)
!451 = !DILocation(line: 470, column: 15, scope: !444)
!452 = !DILocation(line: 472, column: 3, scope: !444)
!453 = !DILocalVariable(name: "__dummy", scope: !454, file: !2, line: 472, type: !113)
!454 = distinct !DILexicalBlock(scope: !455, file: !2, line: 472, column: 3)
!455 = distinct !DILexicalBlock(scope: !444, file: !2, line: 472, column: 3)
!456 = !DILocation(line: 472, column: 3, scope: !454)
!457 = !DILocalVariable(name: "__dummy2", scope: !454, file: !2, line: 472, type: !113)
!458 = !DILocation(line: 472, column: 3, scope: !455)
!459 = !DILocation(line: 474, column: 32, scope: !444)
!460 = !DILocation(line: 474, column: 10, scope: !444)
!461 = !DILocation(line: 474, column: 8, scope: !444)
!462 = !DILocation(line: 475, column: 8, scope: !463)
!463 = distinct !DILexicalBlock(scope: !444, file: !2, line: 475, column: 7)
!464 = !DILocation(line: 475, column: 7, scope: !444)
!465 = !DILocation(line: 476, column: 36, scope: !463)
!466 = !DILocation(line: 476, column: 41, scope: !463)
!467 = !DILocation(line: 476, column: 11, scope: !463)
!468 = !DILocation(line: 476, column: 9, scope: !463)
!469 = !DILocation(line: 476, column: 4, scope: !463)
!470 = !DILocation(line: 478, column: 3, scope: !444)
!471 = !DILocalVariable(name: "__dummy", scope: !472, file: !2, line: 478, type: !113)
!472 = distinct !DILexicalBlock(scope: !473, file: !2, line: 478, column: 3)
!473 = distinct !DILexicalBlock(scope: !444, file: !2, line: 478, column: 3)
!474 = !DILocation(line: 478, column: 3, scope: !472)
!475 = !DILocalVariable(name: "__dummy2", scope: !472, file: !2, line: 478, type: !113)
!476 = !DILocation(line: 478, column: 3, scope: !473)
!477 = !DILocation(line: 480, column: 24, scope: !444)
!478 = !DILocation(line: 480, column: 11, scope: !444)
!479 = !DILocation(line: 480, column: 9, scope: !444)
!480 = !DILocation(line: 481, column: 2, scope: !444)
!481 = !DILocation(line: 481, column: 12, scope: !351)
!482 = !DILocation(line: 481, column: 17, scope: !351)
!483 = !DILocation(line: 481, column: 20, scope: !351)
!484 = !DILocation(line: 481, column: 29, scope: !351)
!485 = !DILocation(line: 481, column: 26, scope: !351)
!486 = !DILocation(line: 0, scope: !351)
!487 = distinct !{!487, !441, !488, !489}
!488 = !DILocation(line: 481, column: 40, scope: !351)
!489 = !{!"llvm.loop.mustprogress"}
!490 = !DILocation(line: 483, column: 22, scope: !351)
!491 = !DILocation(line: 483, column: 2, scope: !351)
!492 = !DILocation(line: 483, column: 9, scope: !351)
!493 = !DILocation(line: 483, column: 20, scope: !351)
!494 = !DILocation(line: 485, column: 6, scope: !495)
!495 = distinct !DILexicalBlock(scope: !351, file: !2, line: 485, column: 6)
!496 = !DILocation(line: 485, column: 6, scope: !351)
!497 = !DILocation(line: 486, column: 3, scope: !498)
!498 = distinct !DILexicalBlock(scope: !495, file: !2, line: 485, column: 12)
!499 = !DILocalVariable(name: "__dummy", scope: !500, file: !2, line: 486, type: !113)
!500 = distinct !DILexicalBlock(scope: !501, file: !2, line: 486, column: 3)
!501 = distinct !DILexicalBlock(scope: !498, file: !2, line: 486, column: 3)
!502 = !DILocation(line: 486, column: 3, scope: !500)
!503 = !DILocalVariable(name: "__dummy2", scope: !500, file: !2, line: 486, type: !113)
!504 = !DILocation(line: 486, column: 3, scope: !501)
!505 = !DILocation(line: 487, column: 28, scope: !498)
!506 = !DILocation(line: 487, column: 33, scope: !498)
!507 = !DILocation(line: 487, column: 40, scope: !498)
!508 = !DILocation(line: 487, column: 45, scope: !498)
!509 = !DILocation(line: 487, column: 51, scope: !498)
!510 = !DILocation(line: 487, column: 3, scope: !498)
!511 = !DILocation(line: 488, column: 3, scope: !498)
!512 = !DILocalVariable(name: "__dummy", scope: !513, file: !2, line: 488, type: !113)
!513 = distinct !DILexicalBlock(scope: !514, file: !2, line: 488, column: 3)
!514 = distinct !DILexicalBlock(scope: !498, file: !2, line: 488, column: 3)
!515 = !DILocation(line: 488, column: 3, scope: !513)
!516 = !DILocalVariable(name: "__dummy2", scope: !513, file: !2, line: 488, type: !113)
!517 = !DILocation(line: 488, column: 3, scope: !514)
!518 = !DILocation(line: 489, column: 2, scope: !498)
!519 = !DILocation(line: 491, column: 9, scope: !351)
!520 = !DILocation(line: 491, column: 2, scope: !351)
!521 = !DILocation(line: 492, column: 1, scope: !351)
!522 = distinct !DISubprogram(name: "bpf_lru_push_free", scope: !2, file: !2, line: 550, type: !523, scopeLine: 551, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !138, retainedNodes: !206)
!523 = !DISubroutineType(types: !524)
!524 = !{null, !221, !179}
!525 = !DILocalVariable(name: "lru", arg: 1, scope: !522, file: !2, line: 550, type: !221)
!526 = !DILocation(line: 550, column: 40, scope: !522)
!527 = !DILocalVariable(name: "node", arg: 2, scope: !522, file: !2, line: 550, type: !179)
!528 = !DILocation(line: 550, column: 66, scope: !522)
!529 = !DILocation(line: 552, column: 6, scope: !530)
!530 = distinct !DILexicalBlock(scope: !522, file: !2, line: 552, column: 6)
!531 = !DILocation(line: 552, column: 11, scope: !530)
!532 = !DILocation(line: 552, column: 6, scope: !522)
!533 = !DILocation(line: 553, column: 28, scope: !530)
!534 = !DILocation(line: 553, column: 33, scope: !530)
!535 = !DILocation(line: 553, column: 3, scope: !530)
!536 = !DILocation(line: 555, column: 28, scope: !530)
!537 = !DILocation(line: 555, column: 33, scope: !530)
!538 = !DILocation(line: 555, column: 3, scope: !530)
!539 = !DILocation(line: 556, column: 1, scope: !522)
!540 = distinct !DISubprogram(name: "bpf_percpu_lru_push_free", scope: !2, file: !2, line: 535, type: !523, scopeLine: 537, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!541 = !DILocalVariable(name: "lru", arg: 1, scope: !540, file: !2, line: 535, type: !221)
!542 = !DILocation(line: 535, column: 54, scope: !540)
!543 = !DILocalVariable(name: "node", arg: 2, scope: !540, file: !2, line: 536, type: !179)
!544 = !DILocation(line: 536, column: 31, scope: !540)
!545 = !DILocalVariable(name: "l", scope: !540, file: !2, line: 538, type: !12)
!546 = !DILocation(line: 538, column: 23, scope: !540)
!547 = !DILocalVariable(name: "flags", scope: !540, file: !2, line: 539, type: !113)
!548 = !DILocation(line: 539, column: 16, scope: !540)
!549 = !DILocation(line: 541, column: 6, scope: !550)
!550 = distinct !DILexicalBlock(scope: !540, file: !2, line: 541, column: 6)
!551 = !DILocalVariable(name: "__vpp_verify", scope: !552, file: !2, line: 541, type: !281)
!552 = distinct !DILexicalBlock(scope: !550, file: !2, line: 541, column: 6)
!553 = !DILocation(line: 541, column: 6, scope: !552)
!554 = !DILocalVariable(name: "__ptr", scope: !555, file: !2, line: 541, type: !113)
!555 = distinct !DILexicalBlock(scope: !550, file: !2, line: 541, column: 6)
!556 = !DILocation(line: 541, column: 6, scope: !555)
!557 = !DILocation(line: 541, column: 4, scope: !540)
!558 = !DILocation(line: 543, column: 2, scope: !540)
!559 = !DILocalVariable(name: "__dummy", scope: !560, file: !2, line: 543, type: !113)
!560 = distinct !DILexicalBlock(scope: !561, file: !2, line: 543, column: 2)
!561 = distinct !DILexicalBlock(scope: !540, file: !2, line: 543, column: 2)
!562 = !DILocation(line: 543, column: 2, scope: !560)
!563 = !DILocalVariable(name: "__dummy2", scope: !560, file: !2, line: 543, type: !113)
!564 = !DILocation(line: 543, column: 2, scope: !561)
!565 = !DILocation(line: 545, column: 22, scope: !540)
!566 = !DILocation(line: 545, column: 25, scope: !540)
!567 = !DILocation(line: 545, column: 2, scope: !540)
!568 = !DILocation(line: 547, column: 2, scope: !540)
!569 = !DILocalVariable(name: "__dummy", scope: !570, file: !2, line: 547, type: !113)
!570 = distinct !DILexicalBlock(scope: !571, file: !2, line: 547, column: 2)
!571 = distinct !DILexicalBlock(scope: !540, file: !2, line: 547, column: 2)
!572 = !DILocation(line: 547, column: 2, scope: !570)
!573 = !DILocalVariable(name: "__dummy2", scope: !570, file: !2, line: 547, type: !113)
!574 = !DILocation(line: 547, column: 2, scope: !571)
!575 = !DILocation(line: 548, column: 1, scope: !540)
!576 = distinct !DISubprogram(name: "bpf_common_lru_push_free", scope: !2, file: !2, line: 502, type: !523, scopeLine: 504, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!577 = !DILocalVariable(name: "lru", arg: 1, scope: !576, file: !2, line: 502, type: !221)
!578 = !DILocation(line: 502, column: 54, scope: !576)
!579 = !DILocalVariable(name: "node", arg: 2, scope: !576, file: !2, line: 503, type: !179)
!580 = !DILocation(line: 503, column: 31, scope: !576)
!581 = !DILocalVariable(name: "flags", scope: !576, file: !2, line: 505, type: !113)
!582 = !DILocation(line: 505, column: 16, scope: !576)
!583 = !DILocalVariable(name: "__ret_warn_on", scope: !584, file: !2, line: 507, type: !50)
!584 = distinct !DILexicalBlock(scope: !585, file: !2, line: 507, column: 6)
!585 = distinct !DILexicalBlock(scope: !576, file: !2, line: 507, column: 6)
!586 = !DILocation(line: 507, column: 6, scope: !584)
!587 = !DILocation(line: 507, column: 6, scope: !588)
!588 = distinct !DILexicalBlock(scope: !584, file: !2, line: 507, column: 6)
!589 = !DILocation(line: 507, column: 6, scope: !590)
!590 = distinct !DILexicalBlock(scope: !591, file: !2, line: 507, column: 6)
!591 = distinct !DILexicalBlock(scope: !588, file: !2, line: 507, column: 6)
!592 = !{i64 2150420246, i64 2150420257, i64 2150420311, i64 2150420342, i64 2150420372}
!593 = !DILocation(line: 507, column: 6, scope: !591)
!594 = !DILocation(line: 507, column: 6, scope: !595)
!595 = distinct !DILexicalBlock(scope: !591, file: !2, line: 507, column: 6)
!596 = !{i64 2150420448, i64 2150420477, i64 2150420523, i64 2150420581, i64 2150420635, i64 2150420689, i64 2150420744, i64 2150420775}
!597 = !DILocation(line: 507, column: 6, scope: !598)
!598 = distinct !DILexicalBlock(scope: !591, file: !2, line: 507, column: 6)
!599 = !{i64 2150421254, i64 2150421261, i64 2150421313, i64 2150421344, i64 2150421374}
!600 = !DILocation(line: 507, column: 6, scope: !601)
!601 = distinct !DILexicalBlock(scope: !591, file: !2, line: 507, column: 6)
!602 = !{i64 2150421435, i64 2150421446, i64 2150421497, i64 2150421528, i64 2150421558}
!603 = !DILocation(line: 507, column: 6, scope: !585)
!604 = !DILocation(line: 507, column: 54, scope: !585)
!605 = !DILocalVariable(name: "__ret_warn_on", scope: !606, file: !2, line: 508, type: !50)
!606 = distinct !DILexicalBlock(scope: !585, file: !2, line: 508, column: 6)
!607 = !DILocation(line: 508, column: 6, scope: !606)
!608 = !DILocation(line: 508, column: 6, scope: !609)
!609 = distinct !DILexicalBlock(scope: !606, file: !2, line: 508, column: 6)
!610 = !DILocation(line: 508, column: 6, scope: !611)
!611 = distinct !DILexicalBlock(scope: !612, file: !2, line: 508, column: 6)
!612 = distinct !DILexicalBlock(scope: !609, file: !2, line: 508, column: 6)
!613 = !{i64 2150422150, i64 2150422161, i64 2150422215, i64 2150422246, i64 2150422276}
!614 = !DILocation(line: 508, column: 6, scope: !612)
!615 = !DILocation(line: 508, column: 6, scope: !616)
!616 = distinct !DILexicalBlock(scope: !612, file: !2, line: 508, column: 6)
!617 = !{i64 2150422352, i64 2150422381, i64 2150422427, i64 2150422485, i64 2150422539, i64 2150422593, i64 2150422648, i64 2150422679}
!618 = !DILocation(line: 508, column: 6, scope: !619)
!619 = distinct !DILexicalBlock(scope: !612, file: !2, line: 508, column: 6)
!620 = !{i64 2150423158, i64 2150423165, i64 2150423217, i64 2150423248, i64 2150423278}
!621 = !DILocation(line: 508, column: 6, scope: !622)
!622 = distinct !DILexicalBlock(scope: !612, file: !2, line: 508, column: 6)
!623 = !{i64 2150423339, i64 2150423350, i64 2150423401, i64 2150423432, i64 2150423462}
!624 = !DILocation(line: 508, column: 6, scope: !585)
!625 = !DILocation(line: 507, column: 6, scope: !576)
!626 = !DILocation(line: 509, column: 3, scope: !585)
!627 = !DILocation(line: 511, column: 6, scope: !628)
!628 = distinct !DILexicalBlock(scope: !576, file: !2, line: 511, column: 6)
!629 = !DILocation(line: 511, column: 12, scope: !628)
!630 = !DILocation(line: 511, column: 17, scope: !628)
!631 = !DILocation(line: 511, column: 6, scope: !576)
!632 = !DILocalVariable(name: "loc_l", scope: !633, file: !2, line: 512, type: !171)
!633 = distinct !DILexicalBlock(scope: !628, file: !2, line: 511, column: 50)
!634 = !DILocation(line: 512, column: 29, scope: !633)
!635 = !DILocation(line: 514, column: 11, scope: !636)
!636 = distinct !DILexicalBlock(scope: !633, file: !2, line: 514, column: 11)
!637 = !DILocalVariable(name: "__vpp_verify", scope: !638, file: !2, line: 514, type: !281)
!638 = distinct !DILexicalBlock(scope: !636, file: !2, line: 514, column: 11)
!639 = !DILocation(line: 514, column: 11, scope: !638)
!640 = !DILocalVariable(name: "__ptr", scope: !641, file: !2, line: 514, type: !113)
!641 = distinct !DILexicalBlock(scope: !636, file: !2, line: 514, column: 11)
!642 = !DILocation(line: 514, column: 11, scope: !641)
!643 = !DILocation(line: 514, column: 9, scope: !633)
!644 = !DILocation(line: 516, column: 3, scope: !633)
!645 = !DILocalVariable(name: "__dummy", scope: !646, file: !2, line: 516, type: !113)
!646 = distinct !DILexicalBlock(scope: !647, file: !2, line: 516, column: 3)
!647 = distinct !DILexicalBlock(scope: !633, file: !2, line: 516, column: 3)
!648 = !DILocation(line: 516, column: 3, scope: !646)
!649 = !DILocalVariable(name: "__dummy2", scope: !646, file: !2, line: 516, type: !113)
!650 = !DILocation(line: 516, column: 3, scope: !647)
!651 = !DILocation(line: 518, column: 7, scope: !652)
!652 = distinct !DILexicalBlock(scope: !633, file: !2, line: 518, column: 7)
!653 = !DILocation(line: 518, column: 7, scope: !633)
!654 = !DILocation(line: 519, column: 4, scope: !655)
!655 = distinct !DILexicalBlock(scope: !652, file: !2, line: 518, column: 61)
!656 = !DILocalVariable(name: "__dummy", scope: !657, file: !2, line: 519, type: !113)
!657 = distinct !DILexicalBlock(scope: !658, file: !2, line: 519, column: 4)
!658 = distinct !DILexicalBlock(scope: !655, file: !2, line: 519, column: 4)
!659 = !DILocation(line: 519, column: 4, scope: !657)
!660 = !DILocalVariable(name: "__dummy2", scope: !657, file: !2, line: 519, type: !113)
!661 = !DILocation(line: 519, column: 4, scope: !658)
!662 = !DILocation(line: 520, column: 4, scope: !655)
!663 = !DILocation(line: 523, column: 3, scope: !633)
!664 = !DILocation(line: 523, column: 9, scope: !633)
!665 = !DILocation(line: 523, column: 14, scope: !633)
!666 = !DILocation(line: 524, column: 3, scope: !633)
!667 = !DILocation(line: 524, column: 9, scope: !633)
!668 = !DILocation(line: 524, column: 13, scope: !633)
!669 = !DILocation(line: 525, column: 14, scope: !633)
!670 = !DILocation(line: 525, column: 20, scope: !633)
!671 = !DILocation(line: 525, column: 42, scope: !633)
!672 = !DILocation(line: 525, column: 26, scope: !633)
!673 = !DILocation(line: 525, column: 3, scope: !633)
!674 = !DILocation(line: 527, column: 3, scope: !633)
!675 = !DILocalVariable(name: "__dummy", scope: !676, file: !2, line: 527, type: !113)
!676 = distinct !DILexicalBlock(scope: !677, file: !2, line: 527, column: 3)
!677 = distinct !DILexicalBlock(scope: !633, file: !2, line: 527, column: 3)
!678 = !DILocation(line: 527, column: 3, scope: !676)
!679 = !DILocalVariable(name: "__dummy2", scope: !676, file: !2, line: 527, type: !113)
!680 = !DILocation(line: 527, column: 3, scope: !677)
!681 = !DILocation(line: 528, column: 3, scope: !633)
!682 = !DILocation(line: 511, column: 20, scope: !628)
!683 = !DILabel(scope: !576, name: "check_lru_list", file: !2, line: 531)
!684 = !DILocation(line: 531, column: 1, scope: !576)
!685 = !DILocation(line: 532, column: 26, scope: !576)
!686 = !DILocation(line: 532, column: 31, scope: !576)
!687 = !DILocation(line: 532, column: 42, scope: !576)
!688 = !DILocation(line: 532, column: 52, scope: !576)
!689 = !DILocation(line: 532, column: 2, scope: !576)
!690 = !DILocation(line: 533, column: 1, scope: !576)
!691 = distinct !DISubprogram(name: "bpf_lru_populate", scope: !2, file: !2, line: 607, type: !692, scopeLine: 609, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !138, retainedNodes: !206)
!692 = !DISubroutineType(types: !693)
!693 = !{null, !221, !72, !187, !187, !187}
!694 = !DILocalVariable(name: "lru", arg: 1, scope: !691, file: !2, line: 607, type: !221)
!695 = !DILocation(line: 607, column: 39, scope: !691)
!696 = !DILocalVariable(name: "buf", arg: 2, scope: !691, file: !2, line: 607, type: !72)
!697 = !DILocation(line: 607, column: 50, scope: !691)
!698 = !DILocalVariable(name: "node_offset", arg: 3, scope: !691, file: !2, line: 607, type: !187)
!699 = !DILocation(line: 607, column: 59, scope: !691)
!700 = !DILocalVariable(name: "elem_size", arg: 4, scope: !691, file: !2, line: 608, type: !187)
!701 = !DILocation(line: 608, column: 13, scope: !691)
!702 = !DILocalVariable(name: "nr_elems", arg: 5, scope: !691, file: !2, line: 608, type: !187)
!703 = !DILocation(line: 608, column: 28, scope: !691)
!704 = !DILocation(line: 610, column: 6, scope: !705)
!705 = distinct !DILexicalBlock(scope: !691, file: !2, line: 610, column: 6)
!706 = !DILocation(line: 610, column: 11, scope: !705)
!707 = !DILocation(line: 610, column: 6, scope: !691)
!708 = !DILocation(line: 611, column: 27, scope: !705)
!709 = !DILocation(line: 611, column: 32, scope: !705)
!710 = !DILocation(line: 611, column: 37, scope: !705)
!711 = !DILocation(line: 611, column: 50, scope: !705)
!712 = !DILocation(line: 612, column: 6, scope: !705)
!713 = !DILocation(line: 611, column: 3, scope: !705)
!714 = !DILocation(line: 614, column: 27, scope: !705)
!715 = !DILocation(line: 614, column: 32, scope: !705)
!716 = !DILocation(line: 614, column: 37, scope: !705)
!717 = !DILocation(line: 614, column: 50, scope: !705)
!718 = !DILocation(line: 615, column: 6, scope: !705)
!719 = !DILocation(line: 614, column: 3, scope: !705)
!720 = !DILocation(line: 616, column: 1, scope: !691)
!721 = distinct !DISubprogram(name: "bpf_percpu_lru_populate", scope: !2, file: !2, line: 576, type: !692, scopeLine: 579, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!722 = !DILocalVariable(name: "lru", arg: 1, scope: !721, file: !2, line: 576, type: !221)
!723 = !DILocation(line: 576, column: 53, scope: !721)
!724 = !DILocalVariable(name: "buf", arg: 2, scope: !721, file: !2, line: 576, type: !72)
!725 = !DILocation(line: 576, column: 64, scope: !721)
!726 = !DILocalVariable(name: "node_offset", arg: 3, scope: !721, file: !2, line: 577, type: !187)
!727 = !DILocation(line: 577, column: 13, scope: !721)
!728 = !DILocalVariable(name: "elem_size", arg: 4, scope: !721, file: !2, line: 577, type: !187)
!729 = !DILocation(line: 577, column: 30, scope: !721)
!730 = !DILocalVariable(name: "nr_elems", arg: 5, scope: !721, file: !2, line: 578, type: !187)
!731 = !DILocation(line: 578, column: 13, scope: !721)
!732 = !DILocalVariable(name: "i", scope: !721, file: !2, line: 580, type: !187)
!733 = !DILocation(line: 580, column: 6, scope: !721)
!734 = !DILocalVariable(name: "pcpu_entries", scope: !721, file: !2, line: 580, type: !187)
!735 = !DILocation(line: 580, column: 9, scope: !721)
!736 = !DILocalVariable(name: "cpu", scope: !721, file: !2, line: 581, type: !50)
!737 = !DILocation(line: 581, column: 6, scope: !721)
!738 = !DILocalVariable(name: "l", scope: !721, file: !2, line: 582, type: !12)
!739 = !DILocation(line: 582, column: 23, scope: !721)
!740 = !DILocation(line: 584, column: 17, scope: !721)
!741 = !DILocation(line: 584, column: 28, scope: !721)
!742 = !DILocation(line: 584, column: 26, scope: !721)
!743 = !DILocation(line: 584, column: 15, scope: !721)
!744 = !DILocation(line: 586, column: 4, scope: !721)
!745 = !DILocation(line: 588, column: 2, scope: !746)
!746 = distinct !DILexicalBlock(scope: !721, file: !2, line: 588, column: 2)
!747 = !DILocation(line: 588, column: 2, scope: !748)
!748 = distinct !DILexicalBlock(scope: !746, file: !2, line: 588, column: 2)
!749 = !DILocalVariable(name: "node", scope: !750, file: !2, line: 589, type: !179)
!750 = distinct !DILexicalBlock(scope: !748, file: !2, line: 588, column: 29)
!751 = !DILocation(line: 589, column: 24, scope: !750)
!752 = !DILocation(line: 591, column: 7, scope: !753)
!753 = distinct !DILexicalBlock(scope: !750, file: !2, line: 591, column: 7)
!754 = !DILocalVariable(name: "__vpp_verify", scope: !755, file: !2, line: 591, type: !281)
!755 = distinct !DILexicalBlock(scope: !753, file: !2, line: 591, column: 7)
!756 = !DILocation(line: 591, column: 7, scope: !755)
!757 = !DILocalVariable(name: "__ptr", scope: !758, file: !2, line: 591, type: !113)
!758 = distinct !DILexicalBlock(scope: !753, file: !2, line: 591, column: 7)
!759 = !DILocation(line: 591, column: 7, scope: !758)
!760 = !DILocation(line: 591, column: 5, scope: !750)
!761 = !DILocation(line: 591, column: 3, scope: !750)
!762 = !DILabel(scope: !750, name: "again", file: !2, line: 592)
!763 = !DILocation(line: 592, column: 1, scope: !750)
!764 = !DILocation(line: 593, column: 34, scope: !750)
!765 = !DILocation(line: 593, column: 40, scope: !750)
!766 = !DILocation(line: 593, column: 38, scope: !750)
!767 = !DILocation(line: 593, column: 8, scope: !750)
!768 = !DILocation(line: 594, column: 15, scope: !750)
!769 = !DILocation(line: 594, column: 3, scope: !750)
!770 = !DILocation(line: 594, column: 9, scope: !750)
!771 = !DILocation(line: 594, column: 13, scope: !750)
!772 = !DILocation(line: 595, column: 3, scope: !750)
!773 = !DILocation(line: 595, column: 9, scope: !750)
!774 = !DILocation(line: 595, column: 14, scope: !750)
!775 = !DILocation(line: 596, column: 3, scope: !750)
!776 = !DILocation(line: 596, column: 9, scope: !750)
!777 = !DILocation(line: 596, column: 13, scope: !750)
!778 = !DILocation(line: 597, column: 13, scope: !750)
!779 = !DILocation(line: 597, column: 19, scope: !750)
!780 = !DILocation(line: 597, column: 26, scope: !750)
!781 = !DILocation(line: 597, column: 29, scope: !750)
!782 = !DILocation(line: 597, column: 3, scope: !750)
!783 = !DILocation(line: 598, column: 4, scope: !750)
!784 = !DILocation(line: 599, column: 10, scope: !750)
!785 = !DILocation(line: 599, column: 7, scope: !750)
!786 = !DILocation(line: 600, column: 7, scope: !787)
!787 = distinct !DILexicalBlock(scope: !750, file: !2, line: 600, column: 7)
!788 = !DILocation(line: 600, column: 12, scope: !787)
!789 = !DILocation(line: 600, column: 9, scope: !787)
!790 = !DILocation(line: 600, column: 7, scope: !750)
!791 = !DILocation(line: 601, column: 4, scope: !787)
!792 = !DILocation(line: 602, column: 7, scope: !793)
!793 = distinct !DILexicalBlock(scope: !750, file: !2, line: 602, column: 7)
!794 = !DILocation(line: 602, column: 11, scope: !793)
!795 = !DILocation(line: 602, column: 9, scope: !793)
!796 = !DILocation(line: 602, column: 7, scope: !750)
!797 = !DILocation(line: 603, column: 4, scope: !793)
!798 = distinct !{!798, !745, !799, !489}
!799 = !DILocation(line: 604, column: 2, scope: !746)
!800 = !DILocation(line: 605, column: 1, scope: !721)
!801 = distinct !DISubprogram(name: "bpf_common_lru_populate", scope: !2, file: !2, line: 558, type: !692, scopeLine: 561, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!802 = !DILocalVariable(name: "lru", arg: 1, scope: !801, file: !2, line: 558, type: !221)
!803 = !DILocation(line: 558, column: 53, scope: !801)
!804 = !DILocalVariable(name: "buf", arg: 2, scope: !801, file: !2, line: 558, type: !72)
!805 = !DILocation(line: 558, column: 64, scope: !801)
!806 = !DILocalVariable(name: "node_offset", arg: 3, scope: !801, file: !2, line: 559, type: !187)
!807 = !DILocation(line: 559, column: 13, scope: !801)
!808 = !DILocalVariable(name: "elem_size", arg: 4, scope: !801, file: !2, line: 559, type: !187)
!809 = !DILocation(line: 559, column: 30, scope: !801)
!810 = !DILocalVariable(name: "nr_elems", arg: 5, scope: !801, file: !2, line: 560, type: !187)
!811 = !DILocation(line: 560, column: 13, scope: !801)
!812 = !DILocalVariable(name: "l", scope: !801, file: !2, line: 562, type: !12)
!813 = !DILocation(line: 562, column: 23, scope: !801)
!814 = !DILocation(line: 562, column: 28, scope: !801)
!815 = !DILocation(line: 562, column: 33, scope: !801)
!816 = !DILocation(line: 562, column: 44, scope: !801)
!817 = !DILocalVariable(name: "i", scope: !801, file: !2, line: 563, type: !187)
!818 = !DILocation(line: 563, column: 6, scope: !801)
!819 = !DILocation(line: 565, column: 9, scope: !820)
!820 = distinct !DILexicalBlock(scope: !801, file: !2, line: 565, column: 2)
!821 = !DILocation(line: 565, column: 7, scope: !820)
!822 = !DILocation(line: 565, column: 14, scope: !823)
!823 = distinct !DILexicalBlock(scope: !820, file: !2, line: 565, column: 2)
!824 = !DILocation(line: 565, column: 18, scope: !823)
!825 = !DILocation(line: 565, column: 16, scope: !823)
!826 = !DILocation(line: 565, column: 2, scope: !820)
!827 = !DILocalVariable(name: "node", scope: !828, file: !2, line: 566, type: !179)
!828 = distinct !DILexicalBlock(scope: !823, file: !2, line: 565, column: 33)
!829 = !DILocation(line: 566, column: 24, scope: !828)
!830 = !DILocation(line: 568, column: 34, scope: !828)
!831 = !DILocation(line: 568, column: 40, scope: !828)
!832 = !DILocation(line: 568, column: 38, scope: !828)
!833 = !DILocation(line: 568, column: 8, scope: !828)
!834 = !DILocation(line: 569, column: 3, scope: !828)
!835 = !DILocation(line: 569, column: 9, scope: !828)
!836 = !DILocation(line: 569, column: 14, scope: !828)
!837 = !DILocation(line: 570, column: 3, scope: !828)
!838 = !DILocation(line: 570, column: 9, scope: !828)
!839 = !DILocation(line: 570, column: 13, scope: !828)
!840 = !DILocation(line: 571, column: 13, scope: !828)
!841 = !DILocation(line: 571, column: 19, scope: !828)
!842 = !DILocation(line: 571, column: 26, scope: !828)
!843 = !DILocation(line: 571, column: 29, scope: !828)
!844 = !DILocation(line: 571, column: 3, scope: !828)
!845 = !DILocation(line: 572, column: 10, scope: !828)
!846 = !DILocation(line: 572, column: 7, scope: !828)
!847 = !DILocation(line: 573, column: 2, scope: !828)
!848 = !DILocation(line: 565, column: 29, scope: !823)
!849 = !DILocation(line: 565, column: 2, scope: !823)
!850 = distinct !{!850, !826, !851, !489}
!851 = !DILocation(line: 573, column: 2, scope: !820)
!852 = !DILocation(line: 574, column: 1, scope: !801)
!853 = distinct !DISubprogram(name: "bpf_lru_init", scope: !2, file: !2, line: 645, type: !854, scopeLine: 647, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !138, retainedNodes: !206)
!854 = !DISubroutineType(types: !855)
!855 = !{!50, !221, !238, !187, !234, !72}
!856 = !DILocalVariable(name: "lru", arg: 1, scope: !853, file: !2, line: 645, type: !221)
!857 = !DILocation(line: 645, column: 34, scope: !853)
!858 = !DILocalVariable(name: "percpu", arg: 2, scope: !853, file: !2, line: 645, type: !238)
!859 = !DILocation(line: 645, column: 44, scope: !853)
!860 = !DILocalVariable(name: "hash_offset", arg: 3, scope: !853, file: !2, line: 645, type: !187)
!861 = !DILocation(line: 645, column: 56, scope: !853)
!862 = !DILocalVariable(name: "del_from_htab", arg: 4, scope: !853, file: !2, line: 646, type: !234)
!863 = !DILocation(line: 646, column: 23, scope: !853)
!864 = !DILocalVariable(name: "del_arg", arg: 5, scope: !853, file: !2, line: 646, type: !72)
!865 = !DILocation(line: 646, column: 44, scope: !853)
!866 = !DILocalVariable(name: "cpu", scope: !853, file: !2, line: 648, type: !50)
!867 = !DILocation(line: 648, column: 6, scope: !853)
!868 = !DILocation(line: 650, column: 6, scope: !869)
!869 = distinct !DILexicalBlock(scope: !853, file: !2, line: 650, column: 6)
!870 = !DILocation(line: 650, column: 6, scope: !853)
!871 = !DILocation(line: 651, column: 21, scope: !872)
!872 = distinct !DILexicalBlock(scope: !869, file: !2, line: 650, column: 14)
!873 = !DILocation(line: 651, column: 3, scope: !872)
!874 = !DILocation(line: 651, column: 8, scope: !872)
!875 = !DILocation(line: 651, column: 19, scope: !872)
!876 = !DILocation(line: 652, column: 8, scope: !877)
!877 = distinct !DILexicalBlock(scope: !872, file: !2, line: 652, column: 7)
!878 = !DILocation(line: 652, column: 13, scope: !877)
!879 = !DILocation(line: 652, column: 7, scope: !872)
!880 = !DILocation(line: 653, column: 4, scope: !877)
!881 = !DILocation(line: 655, column: 3, scope: !882)
!882 = distinct !DILexicalBlock(scope: !872, file: !2, line: 655, column: 3)
!883 = !DILocation(line: 655, column: 3, scope: !884)
!884 = distinct !DILexicalBlock(scope: !882, file: !2, line: 655, column: 3)
!885 = !DILocalVariable(name: "l", scope: !886, file: !2, line: 656, type: !12)
!886 = distinct !DILexicalBlock(scope: !884, file: !2, line: 655, column: 30)
!887 = !DILocation(line: 656, column: 25, scope: !886)
!888 = !DILocation(line: 658, column: 8, scope: !889)
!889 = distinct !DILexicalBlock(scope: !886, file: !2, line: 658, column: 8)
!890 = !DILocalVariable(name: "__vpp_verify", scope: !891, file: !2, line: 658, type: !281)
!891 = distinct !DILexicalBlock(scope: !889, file: !2, line: 658, column: 8)
!892 = !DILocation(line: 658, column: 8, scope: !891)
!893 = !DILocalVariable(name: "__ptr", scope: !894, file: !2, line: 658, type: !113)
!894 = distinct !DILexicalBlock(scope: !889, file: !2, line: 658, column: 8)
!895 = !DILocation(line: 658, column: 8, scope: !894)
!896 = !DILocation(line: 658, column: 6, scope: !886)
!897 = !DILocation(line: 659, column: 22, scope: !886)
!898 = !DILocation(line: 659, column: 4, scope: !886)
!899 = distinct !{!899, !881, !900, !489}
!900 = !DILocation(line: 660, column: 3, scope: !882)
!901 = !DILocation(line: 661, column: 3, scope: !872)
!902 = !DILocation(line: 661, column: 8, scope: !872)
!903 = !DILocation(line: 661, column: 17, scope: !872)
!904 = !DILocation(line: 662, column: 2, scope: !872)
!905 = !DILocalVariable(name: "clru", scope: !906, file: !2, line: 663, type: !361)
!906 = distinct !DILexicalBlock(scope: !869, file: !2, line: 662, column: 9)
!907 = !DILocation(line: 663, column: 26, scope: !906)
!908 = !DILocation(line: 663, column: 34, scope: !906)
!909 = !DILocation(line: 663, column: 39, scope: !906)
!910 = !DILocation(line: 665, column: 22, scope: !906)
!911 = !DILocation(line: 665, column: 3, scope: !906)
!912 = !DILocation(line: 665, column: 9, scope: !906)
!913 = !DILocation(line: 665, column: 20, scope: !906)
!914 = !DILocation(line: 666, column: 8, scope: !915)
!915 = distinct !DILexicalBlock(scope: !906, file: !2, line: 666, column: 7)
!916 = !DILocation(line: 666, column: 14, scope: !915)
!917 = !DILocation(line: 666, column: 7, scope: !906)
!918 = !DILocation(line: 667, column: 4, scope: !915)
!919 = !DILocation(line: 669, column: 3, scope: !920)
!920 = distinct !DILexicalBlock(scope: !906, file: !2, line: 669, column: 3)
!921 = !DILocation(line: 669, column: 3, scope: !922)
!922 = distinct !DILexicalBlock(scope: !920, file: !2, line: 669, column: 3)
!923 = !DILocalVariable(name: "loc_l", scope: !924, file: !2, line: 670, type: !171)
!924 = distinct !DILexicalBlock(scope: !922, file: !2, line: 669, column: 30)
!925 = !DILocation(line: 670, column: 30, scope: !924)
!926 = !DILocation(line: 672, column: 12, scope: !927)
!927 = distinct !DILexicalBlock(scope: !924, file: !2, line: 672, column: 12)
!928 = !DILocalVariable(name: "__vpp_verify", scope: !929, file: !2, line: 672, type: !281)
!929 = distinct !DILexicalBlock(scope: !927, file: !2, line: 672, column: 12)
!930 = !DILocation(line: 672, column: 12, scope: !929)
!931 = !DILocalVariable(name: "__ptr", scope: !932, file: !2, line: 672, type: !113)
!932 = distinct !DILexicalBlock(scope: !927, file: !2, line: 672, column: 12)
!933 = !DILocation(line: 672, column: 12, scope: !932)
!934 = !DILocation(line: 672, column: 10, scope: !924)
!935 = !DILocation(line: 673, column: 27, scope: !924)
!936 = !DILocation(line: 673, column: 34, scope: !924)
!937 = !DILocation(line: 673, column: 4, scope: !924)
!938 = distinct !{!938, !919, !939, !489}
!939 = !DILocation(line: 674, column: 3, scope: !920)
!940 = !DILocation(line: 676, column: 22, scope: !906)
!941 = !DILocation(line: 676, column: 28, scope: !906)
!942 = !DILocation(line: 676, column: 3, scope: !906)
!943 = !DILocation(line: 677, column: 3, scope: !906)
!944 = !DILocation(line: 677, column: 8, scope: !906)
!945 = !DILocation(line: 677, column: 17, scope: !906)
!946 = !DILocation(line: 680, column: 16, scope: !853)
!947 = !DILocation(line: 680, column: 2, scope: !853)
!948 = !DILocation(line: 680, column: 7, scope: !853)
!949 = !DILocation(line: 680, column: 14, scope: !853)
!950 = !DILocation(line: 681, column: 23, scope: !853)
!951 = !DILocation(line: 681, column: 2, scope: !853)
!952 = !DILocation(line: 681, column: 7, scope: !853)
!953 = !DILocation(line: 681, column: 21, scope: !853)
!954 = !DILocation(line: 682, column: 17, scope: !853)
!955 = !DILocation(line: 682, column: 2, scope: !853)
!956 = !DILocation(line: 682, column: 7, scope: !853)
!957 = !DILocation(line: 682, column: 15, scope: !853)
!958 = !DILocation(line: 683, column: 21, scope: !853)
!959 = !DILocation(line: 683, column: 2, scope: !853)
!960 = !DILocation(line: 683, column: 7, scope: !853)
!961 = !DILocation(line: 683, column: 19, scope: !853)
!962 = !DILocation(line: 685, column: 2, scope: !853)
!963 = !DILocation(line: 686, column: 1, scope: !853)
!964 = !DILocalVariable(name: "l", arg: 1, scope: !9, file: !2, line: 630, type: !12)
!965 = !DILocation(line: 630, column: 52, scope: !9)
!966 = !DILocalVariable(name: "i", scope: !9, file: !2, line: 632, type: !50)
!967 = !DILocation(line: 632, column: 6, scope: !9)
!968 = !DILocation(line: 634, column: 9, scope: !969)
!969 = distinct !DILexicalBlock(scope: !9, file: !2, line: 634, column: 2)
!970 = !DILocation(line: 634, column: 7, scope: !969)
!971 = !DILocation(line: 634, column: 14, scope: !972)
!972 = distinct !DILexicalBlock(scope: !969, file: !2, line: 634, column: 2)
!973 = !DILocation(line: 634, column: 16, scope: !972)
!974 = !DILocation(line: 634, column: 2, scope: !969)
!975 = !DILocation(line: 635, column: 19, scope: !972)
!976 = !DILocation(line: 635, column: 22, scope: !972)
!977 = !DILocation(line: 635, column: 28, scope: !972)
!978 = !DILocation(line: 635, column: 3, scope: !972)
!979 = !DILocation(line: 634, column: 38, scope: !972)
!980 = !DILocation(line: 634, column: 2, scope: !972)
!981 = distinct !{!981, !974, !982, !489}
!982 = !DILocation(line: 635, column: 30, scope: !969)
!983 = !DILocation(line: 637, column: 9, scope: !984)
!984 = distinct !DILexicalBlock(scope: !9, file: !2, line: 637, column: 2)
!985 = !DILocation(line: 637, column: 7, scope: !984)
!986 = !DILocation(line: 637, column: 14, scope: !987)
!987 = distinct !DILexicalBlock(scope: !984, file: !2, line: 637, column: 2)
!988 = !DILocation(line: 637, column: 16, scope: !987)
!989 = !DILocation(line: 637, column: 2, scope: !984)
!990 = !DILocation(line: 638, column: 3, scope: !987)
!991 = !DILocation(line: 638, column: 6, scope: !987)
!992 = !DILocation(line: 638, column: 13, scope: !987)
!993 = !DILocation(line: 638, column: 16, scope: !987)
!994 = !DILocation(line: 637, column: 42, scope: !987)
!995 = !DILocation(line: 637, column: 2, scope: !987)
!996 = distinct !{!996, !989, !997, !489}
!997 = !DILocation(line: 638, column: 18, scope: !984)
!998 = !DILocation(line: 640, column: 31, scope: !9)
!999 = !DILocation(line: 640, column: 34, scope: !9)
!1000 = !DILocation(line: 640, column: 2, scope: !9)
!1001 = !DILocation(line: 640, column: 5, scope: !9)
!1002 = !DILocation(line: 640, column: 28, scope: !9)
!1003 = !DILocation(line: 642, column: 2, scope: !9)
!1004 = !DILocation(line: 642, column: 2, scope: !1005)
!1005 = distinct !DILexicalBlock(scope: !9, file: !2, line: 642, column: 2)
!1006 = !DILocation(line: 643, column: 1, scope: !9)
!1007 = !DILocalVariable(name: "loc_l", arg: 1, scope: !203, file: !2, line: 618, type: !171)
!1008 = !DILocation(line: 618, column: 62, scope: !203)
!1009 = !DILocalVariable(name: "cpu", arg: 2, scope: !203, file: !2, line: 618, type: !50)
!1010 = !DILocation(line: 618, column: 73, scope: !203)
!1011 = !DILocalVariable(name: "i", scope: !203, file: !2, line: 620, type: !50)
!1012 = !DILocation(line: 620, column: 6, scope: !203)
!1013 = !DILocation(line: 622, column: 9, scope: !1014)
!1014 = distinct !DILexicalBlock(scope: !203, file: !2, line: 622, column: 2)
!1015 = !DILocation(line: 622, column: 7, scope: !1014)
!1016 = !DILocation(line: 622, column: 14, scope: !1017)
!1017 = distinct !DILexicalBlock(scope: !1014, file: !2, line: 622, column: 2)
!1018 = !DILocation(line: 622, column: 16, scope: !1017)
!1019 = !DILocation(line: 622, column: 2, scope: !1014)
!1020 = !DILocation(line: 623, column: 19, scope: !1017)
!1021 = !DILocation(line: 623, column: 26, scope: !1017)
!1022 = !DILocation(line: 623, column: 32, scope: !1017)
!1023 = !DILocation(line: 623, column: 3, scope: !1017)
!1024 = !DILocation(line: 622, column: 44, scope: !1017)
!1025 = !DILocation(line: 622, column: 2, scope: !1017)
!1026 = distinct !{!1026, !1019, !1027, !489}
!1027 = !DILocation(line: 623, column: 34, scope: !1014)
!1028 = !DILocation(line: 625, column: 22, scope: !203)
!1029 = !DILocation(line: 625, column: 2, scope: !203)
!1030 = !DILocation(line: 625, column: 9, scope: !203)
!1031 = !DILocation(line: 625, column: 20, scope: !203)
!1032 = !DILocation(line: 627, column: 2, scope: !203)
!1033 = !DILocation(line: 627, column: 2, scope: !1034)
!1034 = distinct !DILexicalBlock(scope: !203, file: !2, line: 627, column: 2)
!1035 = !DILocation(line: 628, column: 1, scope: !203)
!1036 = distinct !DISubprogram(name: "bpf_lru_destroy", scope: !2, file: !2, line: 688, type: !1037, scopeLine: 689, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1037 = !DISubroutineType(types: !1038)
!1038 = !{null, !221}
!1039 = !DILocalVariable(name: "lru", arg: 1, scope: !1036, file: !2, line: 688, type: !221)
!1040 = !DILocation(line: 688, column: 38, scope: !1036)
!1041 = !DILocation(line: 690, column: 6, scope: !1042)
!1042 = distinct !DILexicalBlock(scope: !1036, file: !2, line: 690, column: 6)
!1043 = !DILocation(line: 690, column: 11, scope: !1042)
!1044 = !DILocation(line: 690, column: 6, scope: !1036)
!1045 = !DILocation(line: 691, column: 15, scope: !1042)
!1046 = !DILocation(line: 691, column: 20, scope: !1042)
!1047 = !DILocation(line: 691, column: 3, scope: !1042)
!1048 = !DILocation(line: 693, column: 15, scope: !1042)
!1049 = !DILocation(line: 693, column: 20, scope: !1042)
!1050 = !DILocation(line: 693, column: 31, scope: !1042)
!1051 = !DILocation(line: 693, column: 3, scope: !1042)
!1052 = !DILocation(line: 694, column: 1, scope: !1036)
!1053 = distinct !DISubprogram(name: "__bpf_lru_list_rotate", scope: !2, file: !2, line: 237, type: !1054, scopeLine: 238, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1054 = !DISubroutineType(types: !1055)
!1055 = !{null, !221, !12}
!1056 = !DILocalVariable(name: "lru", arg: 1, scope: !1053, file: !2, line: 237, type: !221)
!1057 = !DILocation(line: 237, column: 51, scope: !1053)
!1058 = !DILocalVariable(name: "l", arg: 2, scope: !1053, file: !2, line: 237, type: !12)
!1059 = !DILocation(line: 237, column: 77, scope: !1053)
!1060 = !DILocation(line: 239, column: 32, scope: !1061)
!1061 = distinct !DILexicalBlock(scope: !1053, file: !2, line: 239, column: 6)
!1062 = !DILocation(line: 239, column: 6, scope: !1061)
!1063 = !DILocation(line: 239, column: 6, scope: !1053)
!1064 = !DILocation(line: 240, column: 32, scope: !1061)
!1065 = !DILocation(line: 240, column: 37, scope: !1061)
!1066 = !DILocation(line: 240, column: 3, scope: !1061)
!1067 = !DILocation(line: 242, column: 33, scope: !1053)
!1068 = !DILocation(line: 242, column: 38, scope: !1053)
!1069 = !DILocation(line: 242, column: 2, scope: !1053)
!1070 = !DILocation(line: 243, column: 1, scope: !1053)
!1071 = distinct !DISubprogram(name: "list_empty", scope: !1072, file: !1072, line: 280, type: !1073, scopeLine: 281, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1072 = !DIFile(filename: "include/linux/list.h", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "c6c4ff6a9d6f513d90de8d5ba2bb0226")
!1073 = !DISubroutineType(types: !1074)
!1074 = !{!50, !1075}
!1075 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1076, size: 64)
!1076 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !18)
!1077 = !DILocalVariable(name: "head", arg: 1, scope: !1071, file: !1072, line: 280, type: !1075)
!1078 = !DILocation(line: 280, column: 54, scope: !1071)
!1079 = !DILocation(line: 282, column: 9, scope: !1080)
!1080 = distinct !DILexicalBlock(scope: !1071, file: !1072, line: 282, column: 9)
!1081 = !DILocation(line: 282, column: 9, scope: !1082)
!1082 = distinct !DILexicalBlock(scope: !1080, file: !1072, line: 282, column: 9)
!1083 = !DILocation(line: 282, column: 34, scope: !1071)
!1084 = !DILocation(line: 282, column: 31, scope: !1071)
!1085 = !DILocation(line: 282, column: 2, scope: !1071)
!1086 = distinct !DISubprogram(name: "__bpf_lru_list_shrink", scope: !2, file: !2, line: 255, type: !1087, scopeLine: 261, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1087 = !DISubroutineType(types: !1088)
!1088 = !{!28, !221, !12, !28, !22, !141}
!1089 = !DILocalVariable(name: "lru", arg: 1, scope: !1086, file: !2, line: 255, type: !221)
!1090 = !DILocation(line: 255, column: 59, scope: !1086)
!1091 = !DILocalVariable(name: "l", arg: 2, scope: !1086, file: !2, line: 256, type: !12)
!1092 = !DILocation(line: 256, column: 29, scope: !1086)
!1093 = !DILocalVariable(name: "tgt_nshrink", arg: 3, scope: !1086, file: !2, line: 257, type: !28)
!1094 = !DILocation(line: 257, column: 21, scope: !1086)
!1095 = !DILocalVariable(name: "free_list", arg: 4, scope: !1086, file: !2, line: 258, type: !22)
!1096 = !DILocation(line: 258, column: 26, scope: !1086)
!1097 = !DILocalVariable(name: "tgt_free_type", arg: 5, scope: !1086, file: !2, line: 259, type: !141)
!1098 = !DILocation(line: 259, column: 31, scope: !1086)
!1099 = !DILocalVariable(name: "node", scope: !1086, file: !2, line: 262, type: !179)
!1100 = !DILocation(line: 262, column: 23, scope: !1086)
!1101 = !DILocalVariable(name: "tmp_node", scope: !1086, file: !2, line: 262, type: !179)
!1102 = !DILocation(line: 262, column: 30, scope: !1086)
!1103 = !DILocalVariable(name: "force_shrink_list", scope: !1086, file: !2, line: 263, type: !22)
!1104 = !DILocation(line: 263, column: 20, scope: !1086)
!1105 = !DILocalVariable(name: "nshrinked", scope: !1086, file: !2, line: 264, type: !28)
!1106 = !DILocation(line: 264, column: 15, scope: !1086)
!1107 = !DILocation(line: 266, column: 45, scope: !1086)
!1108 = !DILocation(line: 266, column: 50, scope: !1086)
!1109 = !DILocation(line: 266, column: 53, scope: !1086)
!1110 = !DILocation(line: 267, column: 10, scope: !1086)
!1111 = !DILocation(line: 267, column: 21, scope: !1086)
!1112 = !DILocation(line: 266, column: 14, scope: !1086)
!1113 = !DILocation(line: 266, column: 12, scope: !1086)
!1114 = !DILocation(line: 268, column: 6, scope: !1115)
!1115 = distinct !DILexicalBlock(scope: !1086, file: !2, line: 268, column: 6)
!1116 = !DILocation(line: 268, column: 6, scope: !1086)
!1117 = !DILocation(line: 269, column: 10, scope: !1115)
!1118 = !DILocation(line: 269, column: 3, scope: !1115)
!1119 = !DILocation(line: 272, column: 19, scope: !1120)
!1120 = distinct !DILexicalBlock(scope: !1086, file: !2, line: 272, column: 6)
!1121 = !DILocation(line: 272, column: 22, scope: !1120)
!1122 = !DILocation(line: 272, column: 7, scope: !1120)
!1123 = !DILocation(line: 272, column: 6, scope: !1086)
!1124 = !DILocation(line: 273, column: 24, scope: !1120)
!1125 = !DILocation(line: 273, column: 27, scope: !1120)
!1126 = !DILocation(line: 273, column: 21, scope: !1120)
!1127 = !DILocation(line: 273, column: 3, scope: !1120)
!1128 = !DILocation(line: 275, column: 24, scope: !1120)
!1129 = !DILocation(line: 275, column: 27, scope: !1120)
!1130 = !DILocation(line: 275, column: 21, scope: !1120)
!1131 = !DILocalVariable(name: "__mptr", scope: !1132, file: !2, line: 277, type: !72)
!1132 = distinct !DILexicalBlock(scope: !1133, file: !2, line: 277, column: 2)
!1133 = distinct !DILexicalBlock(scope: !1086, file: !2, line: 277, column: 2)
!1134 = !DILocation(line: 277, column: 2, scope: !1132)
!1135 = !DILocation(line: 277, column: 2, scope: !1136)
!1136 = distinct !DILexicalBlock(scope: !1132, file: !2, line: 277, column: 2)
!1137 = !DILocation(line: 277, column: 2, scope: !1133)
!1138 = !DILocalVariable(name: "__mptr", scope: !1139, file: !2, line: 277, type: !72)
!1139 = distinct !DILexicalBlock(scope: !1133, file: !2, line: 277, column: 2)
!1140 = !DILocation(line: 277, column: 2, scope: !1139)
!1141 = !DILocation(line: 277, column: 2, scope: !1142)
!1142 = distinct !DILexicalBlock(scope: !1139, file: !2, line: 277, column: 2)
!1143 = !DILocation(line: 277, column: 2, scope: !1144)
!1144 = distinct !DILexicalBlock(scope: !1133, file: !2, line: 277, column: 2)
!1145 = !DILocation(line: 279, column: 7, scope: !1146)
!1146 = distinct !DILexicalBlock(scope: !1147, file: !2, line: 279, column: 7)
!1147 = distinct !DILexicalBlock(scope: !1144, file: !2, line: 278, column: 13)
!1148 = !DILocation(line: 279, column: 12, scope: !1146)
!1149 = !DILocation(line: 279, column: 26, scope: !1146)
!1150 = !DILocation(line: 279, column: 31, scope: !1146)
!1151 = !DILocation(line: 279, column: 40, scope: !1146)
!1152 = !DILocation(line: 279, column: 7, scope: !1147)
!1153 = !DILocation(line: 280, column: 32, scope: !1154)
!1154 = distinct !DILexicalBlock(scope: !1146, file: !2, line: 279, column: 47)
!1155 = !DILocation(line: 280, column: 35, scope: !1154)
!1156 = !DILocation(line: 280, column: 41, scope: !1154)
!1157 = !DILocation(line: 281, column: 11, scope: !1154)
!1158 = !DILocation(line: 280, column: 4, scope: !1154)
!1159 = !DILocation(line: 282, column: 4, scope: !1154)
!1160 = !DILocation(line: 284, column: 2, scope: !1147)
!1161 = !DILocalVariable(name: "__mptr", scope: !1162, file: !2, line: 277, type: !72)
!1162 = distinct !DILexicalBlock(scope: !1144, file: !2, line: 277, column: 2)
!1163 = !DILocation(line: 277, column: 2, scope: !1162)
!1164 = !DILocation(line: 277, column: 2, scope: !1165)
!1165 = distinct !DILexicalBlock(scope: !1162, file: !2, line: 277, column: 2)
!1166 = distinct !{!1166, !1137, !1167, !489}
!1167 = !DILocation(line: 284, column: 2, scope: !1133)
!1168 = !DILocation(line: 286, column: 2, scope: !1086)
!1169 = !DILocation(line: 287, column: 1, scope: !1086)
!1170 = distinct !DISubprogram(name: "__bpf_lru_node_move", scope: !2, file: !2, line: 100, type: !1171, scopeLine: 103, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1171 = !DISubroutineType(types: !1172)
!1172 = !{null, !12, !179, !141}
!1173 = !DILocalVariable(name: "l", arg: 1, scope: !1170, file: !2, line: 100, type: !12)
!1174 = !DILocation(line: 100, column: 54, scope: !1170)
!1175 = !DILocalVariable(name: "node", arg: 2, scope: !1170, file: !2, line: 101, type: !179)
!1176 = !DILocation(line: 101, column: 26, scope: !1170)
!1177 = !DILocalVariable(name: "tgt_type", arg: 3, scope: !1170, file: !2, line: 102, type: !141)
!1178 = !DILocation(line: 102, column: 28, scope: !1170)
!1179 = !DILocalVariable(name: "__ret_warn_on", scope: !1180, file: !2, line: 104, type: !50)
!1180 = distinct !DILexicalBlock(scope: !1181, file: !2, line: 104, column: 6)
!1181 = distinct !DILexicalBlock(scope: !1170, file: !2, line: 104, column: 6)
!1182 = !DILocation(line: 104, column: 6, scope: !1180)
!1183 = !DILocation(line: 104, column: 6, scope: !1184)
!1184 = distinct !DILexicalBlock(scope: !1180, file: !2, line: 104, column: 6)
!1185 = !DILocation(line: 104, column: 6, scope: !1186)
!1186 = distinct !DILexicalBlock(scope: !1187, file: !2, line: 104, column: 6)
!1187 = distinct !DILexicalBlock(scope: !1184, file: !2, line: 104, column: 6)
!1188 = !{i64 2150357066, i64 2150357077, i64 2150357131, i64 2150357162, i64 2150357192}
!1189 = !DILocation(line: 104, column: 6, scope: !1187)
!1190 = !DILocation(line: 104, column: 6, scope: !1191)
!1191 = distinct !DILexicalBlock(scope: !1187, file: !2, line: 104, column: 6)
!1192 = !{i64 2150357268, i64 2150357297, i64 2150357343, i64 2150357401, i64 2150357455, i64 2150357509, i64 2150357564, i64 2150357595}
!1193 = !DILocation(line: 104, column: 6, scope: !1194)
!1194 = distinct !DILexicalBlock(scope: !1187, file: !2, line: 104, column: 6)
!1195 = !{i64 2150358074, i64 2150358081, i64 2150358133, i64 2150358164, i64 2150358194}
!1196 = !DILocation(line: 104, column: 6, scope: !1197)
!1197 = distinct !DILexicalBlock(scope: !1187, file: !2, line: 104, column: 6)
!1198 = !{i64 2150358255, i64 2150358266, i64 2150358317, i64 2150358348, i64 2150358378}
!1199 = !DILocation(line: 104, column: 6, scope: !1181)
!1200 = !DILocation(line: 104, column: 51, scope: !1181)
!1201 = !DILocalVariable(name: "__ret_warn_on", scope: !1202, file: !2, line: 105, type: !50)
!1202 = distinct !DILexicalBlock(scope: !1181, file: !2, line: 105, column: 6)
!1203 = !DILocation(line: 105, column: 6, scope: !1202)
!1204 = !DILocation(line: 105, column: 6, scope: !1205)
!1205 = distinct !DILexicalBlock(scope: !1202, file: !2, line: 105, column: 6)
!1206 = !DILocation(line: 105, column: 6, scope: !1207)
!1207 = distinct !DILexicalBlock(scope: !1208, file: !2, line: 105, column: 6)
!1208 = distinct !DILexicalBlock(scope: !1205, file: !2, line: 105, column: 6)
!1209 = !{i64 2150359017, i64 2150359028, i64 2150359082, i64 2150359113, i64 2150359143}
!1210 = !DILocation(line: 105, column: 6, scope: !1208)
!1211 = !DILocation(line: 105, column: 6, scope: !1212)
!1212 = distinct !DILexicalBlock(scope: !1208, file: !2, line: 105, column: 6)
!1213 = !{i64 2150359219, i64 2150359248, i64 2150359294, i64 2150359352, i64 2150359406, i64 2150359460, i64 2150359515, i64 2150359546}
!1214 = !DILocation(line: 105, column: 6, scope: !1215)
!1215 = distinct !DILexicalBlock(scope: !1208, file: !2, line: 105, column: 6)
!1216 = !{i64 2150360025, i64 2150360032, i64 2150360084, i64 2150360115, i64 2150360145}
!1217 = !DILocation(line: 105, column: 6, scope: !1218)
!1218 = distinct !DILexicalBlock(scope: !1208, file: !2, line: 105, column: 6)
!1219 = !{i64 2150360206, i64 2150360217, i64 2150360268, i64 2150360299, i64 2150360329}
!1220 = !DILocation(line: 105, column: 6, scope: !1181)
!1221 = !DILocation(line: 104, column: 6, scope: !1170)
!1222 = !DILocation(line: 106, column: 3, scope: !1181)
!1223 = !DILocation(line: 108, column: 6, scope: !1224)
!1224 = distinct !DILexicalBlock(scope: !1170, file: !2, line: 108, column: 6)
!1225 = !DILocation(line: 108, column: 12, scope: !1224)
!1226 = !DILocation(line: 108, column: 20, scope: !1224)
!1227 = !DILocation(line: 108, column: 17, scope: !1224)
!1228 = !DILocation(line: 108, column: 6, scope: !1170)
!1229 = !DILocation(line: 109, column: 26, scope: !1230)
!1230 = distinct !DILexicalBlock(scope: !1224, file: !2, line: 108, column: 30)
!1231 = !DILocation(line: 109, column: 29, scope: !1230)
!1232 = !DILocation(line: 109, column: 35, scope: !1230)
!1233 = !DILocation(line: 109, column: 3, scope: !1230)
!1234 = !DILocation(line: 110, column: 26, scope: !1230)
!1235 = !DILocation(line: 110, column: 29, scope: !1230)
!1236 = !DILocation(line: 110, column: 3, scope: !1230)
!1237 = !DILocation(line: 111, column: 16, scope: !1230)
!1238 = !DILocation(line: 111, column: 3, scope: !1230)
!1239 = !DILocation(line: 111, column: 9, scope: !1230)
!1240 = !DILocation(line: 111, column: 14, scope: !1230)
!1241 = !DILocation(line: 112, column: 2, scope: !1230)
!1242 = !DILocation(line: 113, column: 2, scope: !1170)
!1243 = !DILocation(line: 113, column: 8, scope: !1170)
!1244 = !DILocation(line: 113, column: 12, scope: !1170)
!1245 = !DILocation(line: 118, column: 7, scope: !1246)
!1246 = distinct !DILexicalBlock(scope: !1170, file: !2, line: 118, column: 6)
!1247 = !DILocation(line: 118, column: 13, scope: !1246)
!1248 = !DILocation(line: 118, column: 21, scope: !1246)
!1249 = !DILocation(line: 118, column: 24, scope: !1246)
!1250 = !DILocation(line: 118, column: 18, scope: !1246)
!1251 = !DILocation(line: 118, column: 6, scope: !1170)
!1252 = !DILocation(line: 119, column: 31, scope: !1246)
!1253 = !DILocation(line: 119, column: 34, scope: !1246)
!1254 = !DILocation(line: 119, column: 58, scope: !1246)
!1255 = !DILocation(line: 119, column: 3, scope: !1246)
!1256 = !DILocation(line: 119, column: 6, scope: !1246)
!1257 = !DILocation(line: 119, column: 29, scope: !1246)
!1258 = !DILocation(line: 121, column: 13, scope: !1170)
!1259 = !DILocation(line: 121, column: 19, scope: !1170)
!1260 = !DILocation(line: 121, column: 26, scope: !1170)
!1261 = !DILocation(line: 121, column: 29, scope: !1170)
!1262 = !DILocation(line: 121, column: 35, scope: !1170)
!1263 = !DILocation(line: 121, column: 2, scope: !1170)
!1264 = !DILocation(line: 122, column: 1, scope: !1170)
!1265 = distinct !DISubprogram(name: "bpf_lru_list_inactive_low", scope: !2, file: !2, line: 124, type: !1266, scopeLine: 125, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1266 = !DISubroutineType(types: !1267)
!1267 = !{!238, !1268}
!1268 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1269, size: 64)
!1269 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !13)
!1270 = !DILocalVariable(name: "l", arg: 1, scope: !1265, file: !2, line: 124, type: !1268)
!1271 = !DILocation(line: 124, column: 66, scope: !1265)
!1272 = !DILocation(line: 126, column: 9, scope: !1265)
!1273 = !DILocation(line: 126, column: 12, scope: !1265)
!1274 = !DILocation(line: 127, column: 3, scope: !1265)
!1275 = !DILocation(line: 127, column: 6, scope: !1265)
!1276 = !DILocation(line: 126, column: 44, scope: !1265)
!1277 = !DILocation(line: 126, column: 2, scope: !1265)
!1278 = distinct !DISubprogram(name: "__bpf_lru_list_rotate_active", scope: !2, file: !2, line: 139, type: !1054, scopeLine: 141, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1279 = !DILocalVariable(name: "lru", arg: 1, scope: !1278, file: !2, line: 139, type: !221)
!1280 = !DILocation(line: 139, column: 58, scope: !1278)
!1281 = !DILocalVariable(name: "l", arg: 2, scope: !1278, file: !2, line: 140, type: !12)
!1282 = !DILocation(line: 140, column: 28, scope: !1278)
!1283 = !DILocalVariable(name: "active", scope: !1278, file: !2, line: 142, type: !22)
!1284 = !DILocation(line: 142, column: 20, scope: !1278)
!1285 = !DILocation(line: 142, column: 30, scope: !1278)
!1286 = !DILocation(line: 142, column: 33, scope: !1278)
!1287 = !DILocalVariable(name: "node", scope: !1278, file: !2, line: 143, type: !179)
!1288 = !DILocation(line: 143, column: 23, scope: !1278)
!1289 = !DILocalVariable(name: "tmp_node", scope: !1278, file: !2, line: 143, type: !179)
!1290 = !DILocation(line: 143, column: 30, scope: !1278)
!1291 = !DILocalVariable(name: "first_node", scope: !1278, file: !2, line: 143, type: !179)
!1292 = !DILocation(line: 143, column: 41, scope: !1278)
!1293 = !DILocalVariable(name: "i", scope: !1278, file: !2, line: 144, type: !28)
!1294 = !DILocation(line: 144, column: 15, scope: !1278)
!1295 = !DILocalVariable(name: "__mptr", scope: !1296, file: !2, line: 146, type: !72)
!1296 = distinct !DILexicalBlock(scope: !1278, file: !2, line: 146, column: 15)
!1297 = !DILocation(line: 146, column: 15, scope: !1296)
!1298 = !DILocation(line: 146, column: 15, scope: !1299)
!1299 = distinct !DILexicalBlock(scope: !1296, file: !2, line: 146, column: 15)
!1300 = !DILocation(line: 146, column: 13, scope: !1278)
!1301 = !DILocalVariable(name: "__mptr", scope: !1302, file: !2, line: 147, type: !72)
!1302 = distinct !DILexicalBlock(scope: !1303, file: !2, line: 147, column: 2)
!1303 = distinct !DILexicalBlock(scope: !1278, file: !2, line: 147, column: 2)
!1304 = !DILocation(line: 147, column: 2, scope: !1302)
!1305 = !DILocation(line: 147, column: 2, scope: !1306)
!1306 = distinct !DILexicalBlock(scope: !1302, file: !2, line: 147, column: 2)
!1307 = !DILocation(line: 147, column: 2, scope: !1303)
!1308 = !DILocalVariable(name: "__mptr", scope: !1309, file: !2, line: 147, type: !72)
!1309 = distinct !DILexicalBlock(scope: !1303, file: !2, line: 147, column: 2)
!1310 = !DILocation(line: 147, column: 2, scope: !1309)
!1311 = !DILocation(line: 147, column: 2, scope: !1312)
!1312 = distinct !DILexicalBlock(scope: !1309, file: !2, line: 147, column: 2)
!1313 = !DILocation(line: 147, column: 2, scope: !1314)
!1314 = distinct !DILexicalBlock(scope: !1303, file: !2, line: 147, column: 2)
!1315 = !DILocation(line: 148, column: 27, scope: !1316)
!1316 = distinct !DILexicalBlock(scope: !1317, file: !2, line: 148, column: 7)
!1317 = distinct !DILexicalBlock(scope: !1314, file: !2, line: 147, column: 65)
!1318 = !DILocation(line: 148, column: 7, scope: !1316)
!1319 = !DILocation(line: 148, column: 7, scope: !1317)
!1320 = !DILocation(line: 149, column: 24, scope: !1316)
!1321 = !DILocation(line: 149, column: 27, scope: !1316)
!1322 = !DILocation(line: 149, column: 4, scope: !1316)
!1323 = !DILocation(line: 151, column: 24, scope: !1316)
!1324 = !DILocation(line: 151, column: 27, scope: !1316)
!1325 = !DILocation(line: 151, column: 4, scope: !1316)
!1326 = !DILocation(line: 153, column: 7, scope: !1327)
!1327 = distinct !DILexicalBlock(scope: !1317, file: !2, line: 153, column: 7)
!1328 = !DILocation(line: 153, column: 14, scope: !1327)
!1329 = !DILocation(line: 153, column: 19, scope: !1327)
!1330 = !DILocation(line: 153, column: 11, scope: !1327)
!1331 = !DILocation(line: 153, column: 28, scope: !1327)
!1332 = !DILocation(line: 153, column: 31, scope: !1327)
!1333 = !DILocation(line: 153, column: 39, scope: !1327)
!1334 = !DILocation(line: 153, column: 36, scope: !1327)
!1335 = !DILocation(line: 153, column: 7, scope: !1317)
!1336 = !DILocation(line: 154, column: 4, scope: !1327)
!1337 = !DILocation(line: 155, column: 2, scope: !1317)
!1338 = !DILocalVariable(name: "__mptr", scope: !1339, file: !2, line: 147, type: !72)
!1339 = distinct !DILexicalBlock(scope: !1314, file: !2, line: 147, column: 2)
!1340 = !DILocation(line: 147, column: 2, scope: !1339)
!1341 = !DILocation(line: 147, column: 2, scope: !1342)
!1342 = distinct !DILexicalBlock(scope: !1339, file: !2, line: 147, column: 2)
!1343 = distinct !{!1343, !1307, !1344, !489}
!1344 = !DILocation(line: 155, column: 2, scope: !1303)
!1345 = !DILocation(line: 156, column: 1, scope: !1278)
!1346 = distinct !DISubprogram(name: "__bpf_lru_list_rotate_inactive", scope: !2, file: !2, line: 166, type: !1054, scopeLine: 168, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1347 = !DILocalVariable(name: "lru", arg: 1, scope: !1346, file: !2, line: 166, type: !221)
!1348 = !DILocation(line: 166, column: 60, scope: !1346)
!1349 = !DILocalVariable(name: "l", arg: 2, scope: !1346, file: !2, line: 167, type: !12)
!1350 = !DILocation(line: 167, column: 30, scope: !1346)
!1351 = !DILocalVariable(name: "inactive", scope: !1346, file: !2, line: 169, type: !22)
!1352 = !DILocation(line: 169, column: 20, scope: !1346)
!1353 = !DILocation(line: 169, column: 32, scope: !1346)
!1354 = !DILocation(line: 169, column: 35, scope: !1346)
!1355 = !DILocalVariable(name: "cur", scope: !1346, file: !2, line: 170, type: !22)
!1356 = !DILocation(line: 170, column: 20, scope: !1346)
!1357 = !DILocalVariable(name: "last", scope: !1346, file: !2, line: 170, type: !22)
!1358 = !DILocation(line: 170, column: 26, scope: !1346)
!1359 = !DILocalVariable(name: "next", scope: !1346, file: !2, line: 170, type: !22)
!1360 = !DILocation(line: 170, column: 33, scope: !1346)
!1361 = !DILocation(line: 170, column: 40, scope: !1346)
!1362 = !DILocalVariable(name: "node", scope: !1346, file: !2, line: 171, type: !179)
!1363 = !DILocation(line: 171, column: 23, scope: !1346)
!1364 = !DILocalVariable(name: "i", scope: !1346, file: !2, line: 172, type: !28)
!1365 = !DILocation(line: 172, column: 15, scope: !1346)
!1366 = !DILocation(line: 174, column: 17, scope: !1367)
!1367 = distinct !DILexicalBlock(scope: !1346, file: !2, line: 174, column: 6)
!1368 = !DILocation(line: 174, column: 6, scope: !1367)
!1369 = !DILocation(line: 174, column: 6, scope: !1346)
!1370 = !DILocation(line: 175, column: 3, scope: !1367)
!1371 = !DILocation(line: 177, column: 9, scope: !1346)
!1372 = !DILocation(line: 177, column: 12, scope: !1346)
!1373 = !DILocation(line: 177, column: 36, scope: !1346)
!1374 = !DILocation(line: 177, column: 7, scope: !1346)
!1375 = !DILocation(line: 178, column: 6, scope: !1376)
!1376 = distinct !DILexicalBlock(scope: !1346, file: !2, line: 178, column: 6)
!1377 = !DILocation(line: 178, column: 14, scope: !1376)
!1378 = !DILocation(line: 178, column: 11, scope: !1376)
!1379 = !DILocation(line: 178, column: 6, scope: !1346)
!1380 = !DILocation(line: 179, column: 10, scope: !1376)
!1381 = !DILocation(line: 179, column: 16, scope: !1376)
!1382 = !DILocation(line: 179, column: 8, scope: !1376)
!1383 = !DILocation(line: 179, column: 3, scope: !1376)
!1384 = !DILocation(line: 181, column: 8, scope: !1346)
!1385 = !DILocation(line: 181, column: 11, scope: !1346)
!1386 = !DILocation(line: 181, column: 6, scope: !1346)
!1387 = !DILocation(line: 182, column: 2, scope: !1346)
!1388 = !DILocation(line: 182, column: 9, scope: !1346)
!1389 = !DILocation(line: 182, column: 13, scope: !1346)
!1390 = !DILocation(line: 182, column: 18, scope: !1346)
!1391 = !DILocation(line: 182, column: 11, scope: !1346)
!1392 = !DILocation(line: 183, column: 7, scope: !1393)
!1393 = distinct !DILexicalBlock(scope: !1394, file: !2, line: 183, column: 7)
!1394 = distinct !DILexicalBlock(scope: !1346, file: !2, line: 182, column: 28)
!1395 = !DILocation(line: 183, column: 14, scope: !1393)
!1396 = !DILocation(line: 183, column: 11, scope: !1393)
!1397 = !DILocation(line: 183, column: 7, scope: !1394)
!1398 = !DILocation(line: 184, column: 10, scope: !1399)
!1399 = distinct !DILexicalBlock(scope: !1393, file: !2, line: 183, column: 24)
!1400 = !DILocation(line: 184, column: 15, scope: !1399)
!1401 = !DILocation(line: 184, column: 8, scope: !1399)
!1402 = !DILocation(line: 185, column: 4, scope: !1399)
!1403 = distinct !{!1403, !1387, !1404, !489}
!1404 = !DILocation(line: 196, column: 2, scope: !1346)
!1405 = !DILocalVariable(name: "__mptr", scope: !1406, file: !2, line: 188, type: !72)
!1406 = distinct !DILexicalBlock(scope: !1394, file: !2, line: 188, column: 10)
!1407 = !DILocation(line: 188, column: 10, scope: !1406)
!1408 = !DILocation(line: 188, column: 10, scope: !1409)
!1409 = distinct !DILexicalBlock(scope: !1406, file: !2, line: 188, column: 10)
!1410 = !DILocation(line: 188, column: 8, scope: !1394)
!1411 = !DILocation(line: 189, column: 10, scope: !1394)
!1412 = !DILocation(line: 189, column: 15, scope: !1394)
!1413 = !DILocation(line: 189, column: 8, scope: !1394)
!1414 = !DILocation(line: 190, column: 27, scope: !1415)
!1415 = distinct !DILexicalBlock(scope: !1394, file: !2, line: 190, column: 7)
!1416 = !DILocation(line: 190, column: 7, scope: !1415)
!1417 = !DILocation(line: 190, column: 7, scope: !1394)
!1418 = !DILocation(line: 191, column: 24, scope: !1415)
!1419 = !DILocation(line: 191, column: 27, scope: !1415)
!1420 = !DILocation(line: 191, column: 4, scope: !1415)
!1421 = !DILocation(line: 192, column: 7, scope: !1422)
!1422 = distinct !DILexicalBlock(scope: !1394, file: !2, line: 192, column: 7)
!1423 = !DILocation(line: 192, column: 14, scope: !1422)
!1424 = !DILocation(line: 192, column: 11, scope: !1422)
!1425 = !DILocation(line: 192, column: 7, scope: !1394)
!1426 = !DILocation(line: 193, column: 4, scope: !1422)
!1427 = !DILocation(line: 194, column: 9, scope: !1394)
!1428 = !DILocation(line: 194, column: 7, scope: !1394)
!1429 = !DILocation(line: 195, column: 4, scope: !1394)
!1430 = !DILocation(line: 198, column: 30, scope: !1346)
!1431 = !DILocation(line: 198, column: 2, scope: !1346)
!1432 = !DILocation(line: 198, column: 5, scope: !1346)
!1433 = !DILocation(line: 198, column: 28, scope: !1346)
!1434 = !DILocation(line: 199, column: 1, scope: !1346)
!1435 = distinct !DISubprogram(name: "bpf_lru_node_is_ref", scope: !2, file: !2, line: 42, type: !1436, scopeLine: 43, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1436 = !DISubroutineType(types: !1437)
!1437 = !{!238, !1438}
!1438 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !1439, size: 64)
!1439 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !180)
!1440 = !DILocalVariable(name: "node", arg: 1, scope: !1435, file: !2, line: 42, type: !1438)
!1441 = !DILocation(line: 42, column: 60, scope: !1435)
!1442 = !DILocation(line: 44, column: 9, scope: !1435)
!1443 = !DILocation(line: 44, column: 15, scope: !1435)
!1444 = !DILocation(line: 44, column: 2, scope: !1435)
!1445 = distinct !DISubprogram(name: "__bpf_lru_list_shrink_inactive", scope: !2, file: !2, line: 206, type: !1087, scopeLine: 211, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1446 = !DILocalVariable(name: "lru", arg: 1, scope: !1445, file: !2, line: 206, type: !221)
!1447 = !DILocation(line: 206, column: 48, scope: !1445)
!1448 = !DILocalVariable(name: "l", arg: 2, scope: !1445, file: !2, line: 207, type: !12)
!1449 = !DILocation(line: 207, column: 32, scope: !1445)
!1450 = !DILocalVariable(name: "tgt_nshrink", arg: 3, scope: !1445, file: !2, line: 208, type: !28)
!1451 = !DILocation(line: 208, column: 24, scope: !1445)
!1452 = !DILocalVariable(name: "free_list", arg: 4, scope: !1445, file: !2, line: 209, type: !22)
!1453 = !DILocation(line: 209, column: 29, scope: !1445)
!1454 = !DILocalVariable(name: "tgt_free_type", arg: 5, scope: !1445, file: !2, line: 210, type: !141)
!1455 = !DILocation(line: 210, column: 34, scope: !1445)
!1456 = !DILocalVariable(name: "inactive", scope: !1445, file: !2, line: 212, type: !22)
!1457 = !DILocation(line: 212, column: 20, scope: !1445)
!1458 = !DILocation(line: 212, column: 32, scope: !1445)
!1459 = !DILocation(line: 212, column: 35, scope: !1445)
!1460 = !DILocalVariable(name: "node", scope: !1445, file: !2, line: 213, type: !179)
!1461 = !DILocation(line: 213, column: 23, scope: !1445)
!1462 = !DILocalVariable(name: "tmp_node", scope: !1445, file: !2, line: 213, type: !179)
!1463 = !DILocation(line: 213, column: 30, scope: !1445)
!1464 = !DILocalVariable(name: "nshrinked", scope: !1445, file: !2, line: 214, type: !28)
!1465 = !DILocation(line: 214, column: 15, scope: !1445)
!1466 = !DILocalVariable(name: "i", scope: !1445, file: !2, line: 215, type: !28)
!1467 = !DILocation(line: 215, column: 15, scope: !1445)
!1468 = !DILocalVariable(name: "__mptr", scope: !1469, file: !2, line: 217, type: !72)
!1469 = distinct !DILexicalBlock(scope: !1470, file: !2, line: 217, column: 2)
!1470 = distinct !DILexicalBlock(scope: !1445, file: !2, line: 217, column: 2)
!1471 = !DILocation(line: 217, column: 2, scope: !1469)
!1472 = !DILocation(line: 217, column: 2, scope: !1473)
!1473 = distinct !DILexicalBlock(scope: !1469, file: !2, line: 217, column: 2)
!1474 = !DILocation(line: 217, column: 2, scope: !1470)
!1475 = !DILocalVariable(name: "__mptr", scope: !1476, file: !2, line: 217, type: !72)
!1476 = distinct !DILexicalBlock(scope: !1470, file: !2, line: 217, column: 2)
!1477 = !DILocation(line: 217, column: 2, scope: !1476)
!1478 = !DILocation(line: 217, column: 2, scope: !1479)
!1479 = distinct !DILexicalBlock(scope: !1476, file: !2, line: 217, column: 2)
!1480 = !DILocation(line: 217, column: 2, scope: !1481)
!1481 = distinct !DILexicalBlock(scope: !1470, file: !2, line: 217, column: 2)
!1482 = !DILocation(line: 218, column: 27, scope: !1483)
!1483 = distinct !DILexicalBlock(scope: !1484, file: !2, line: 218, column: 7)
!1484 = distinct !DILexicalBlock(scope: !1481, file: !2, line: 217, column: 67)
!1485 = !DILocation(line: 218, column: 7, scope: !1483)
!1486 = !DILocation(line: 218, column: 7, scope: !1484)
!1487 = !DILocation(line: 219, column: 24, scope: !1488)
!1488 = distinct !DILexicalBlock(scope: !1483, file: !2, line: 218, column: 34)
!1489 = !DILocation(line: 219, column: 27, scope: !1488)
!1490 = !DILocation(line: 219, column: 4, scope: !1488)
!1491 = !DILocation(line: 220, column: 3, scope: !1488)
!1492 = !DILocation(line: 220, column: 14, scope: !1493)
!1493 = distinct !DILexicalBlock(scope: !1483, file: !2, line: 220, column: 14)
!1494 = !DILocation(line: 220, column: 19, scope: !1493)
!1495 = !DILocation(line: 220, column: 33, scope: !1493)
!1496 = !DILocation(line: 220, column: 38, scope: !1493)
!1497 = !DILocation(line: 220, column: 47, scope: !1493)
!1498 = !DILocation(line: 220, column: 14, scope: !1483)
!1499 = !DILocation(line: 221, column: 32, scope: !1500)
!1500 = distinct !DILexicalBlock(scope: !1493, file: !2, line: 220, column: 54)
!1501 = !DILocation(line: 221, column: 35, scope: !1500)
!1502 = !DILocation(line: 221, column: 41, scope: !1500)
!1503 = !DILocation(line: 222, column: 11, scope: !1500)
!1504 = !DILocation(line: 221, column: 4, scope: !1500)
!1505 = !DILocation(line: 223, column: 8, scope: !1506)
!1506 = distinct !DILexicalBlock(scope: !1500, file: !2, line: 223, column: 8)
!1507 = !DILocation(line: 223, column: 23, scope: !1506)
!1508 = !DILocation(line: 223, column: 20, scope: !1506)
!1509 = !DILocation(line: 223, column: 8, scope: !1500)
!1510 = !DILocation(line: 224, column: 5, scope: !1506)
!1511 = !DILocation(line: 225, column: 3, scope: !1500)
!1512 = !DILocation(line: 227, column: 7, scope: !1513)
!1513 = distinct !DILexicalBlock(scope: !1484, file: !2, line: 227, column: 7)
!1514 = !DILocation(line: 227, column: 14, scope: !1513)
!1515 = !DILocation(line: 227, column: 19, scope: !1513)
!1516 = !DILocation(line: 227, column: 11, scope: !1513)
!1517 = !DILocation(line: 227, column: 7, scope: !1484)
!1518 = !DILocation(line: 228, column: 4, scope: !1513)
!1519 = !DILocation(line: 229, column: 2, scope: !1484)
!1520 = !DILocalVariable(name: "__mptr", scope: !1521, file: !2, line: 217, type: !72)
!1521 = distinct !DILexicalBlock(scope: !1481, file: !2, line: 217, column: 2)
!1522 = !DILocation(line: 217, column: 2, scope: !1521)
!1523 = !DILocation(line: 217, column: 2, scope: !1524)
!1524 = distinct !DILexicalBlock(scope: !1521, file: !2, line: 217, column: 2)
!1525 = distinct !{!1525, !1474, !1526, !489}
!1526 = !DILocation(line: 229, column: 2, scope: !1470)
!1527 = !DILocation(line: 231, column: 9, scope: !1445)
!1528 = !DILocation(line: 231, column: 2, scope: !1445)
!1529 = distinct !DISubprogram(name: "__bpf_lru_node_move_to_free", scope: !2, file: !2, line: 61, type: !1530, scopeLine: 65, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1530 = !DISubroutineType(types: !1531)
!1531 = !{null, !12, !179, !22, !141}
!1532 = !DILocalVariable(name: "l", arg: 1, scope: !1529, file: !2, line: 61, type: !12)
!1533 = !DILocation(line: 61, column: 62, scope: !1529)
!1534 = !DILocalVariable(name: "node", arg: 2, scope: !1529, file: !2, line: 62, type: !179)
!1535 = !DILocation(line: 62, column: 27, scope: !1529)
!1536 = !DILocalVariable(name: "free_list", arg: 3, scope: !1529, file: !2, line: 63, type: !22)
!1537 = !DILocation(line: 63, column: 24, scope: !1529)
!1538 = !DILocalVariable(name: "tgt_free_type", arg: 4, scope: !1529, file: !2, line: 64, type: !141)
!1539 = !DILocation(line: 64, column: 29, scope: !1529)
!1540 = !DILocalVariable(name: "__ret_warn_on", scope: !1541, file: !2, line: 66, type: !50)
!1541 = distinct !DILexicalBlock(scope: !1542, file: !2, line: 66, column: 6)
!1542 = distinct !DILexicalBlock(scope: !1529, file: !2, line: 66, column: 6)
!1543 = !DILocation(line: 66, column: 6, scope: !1541)
!1544 = !DILocation(line: 66, column: 6, scope: !1545)
!1545 = distinct !DILexicalBlock(scope: !1541, file: !2, line: 66, column: 6)
!1546 = !DILocation(line: 66, column: 6, scope: !1547)
!1547 = distinct !DILexicalBlock(scope: !1548, file: !2, line: 66, column: 6)
!1548 = distinct !DILexicalBlock(scope: !1545, file: !2, line: 66, column: 6)
!1549 = !{i64 2150351206, i64 2150351217, i64 2150351271, i64 2150351302, i64 2150351332}
!1550 = !DILocation(line: 66, column: 6, scope: !1548)
!1551 = !DILocation(line: 66, column: 6, scope: !1552)
!1552 = distinct !DILexicalBlock(scope: !1548, file: !2, line: 66, column: 6)
!1553 = !{i64 2150351408, i64 2150351437, i64 2150351483, i64 2150351541, i64 2150351595, i64 2150351649, i64 2150351704, i64 2150351735}
!1554 = !DILocation(line: 66, column: 6, scope: !1555)
!1555 = distinct !DILexicalBlock(scope: !1548, file: !2, line: 66, column: 6)
!1556 = !{i64 2150352213, i64 2150352220, i64 2150352272, i64 2150352303, i64 2150352333}
!1557 = !DILocation(line: 66, column: 6, scope: !1558)
!1558 = distinct !DILexicalBlock(scope: !1548, file: !2, line: 66, column: 6)
!1559 = !{i64 2150352394, i64 2150352405, i64 2150352456, i64 2150352487, i64 2150352517}
!1560 = !DILocation(line: 66, column: 6, scope: !1542)
!1561 = !DILocation(line: 66, column: 6, scope: !1529)
!1562 = !DILocation(line: 67, column: 3, scope: !1542)
!1563 = !DILocation(line: 72, column: 7, scope: !1564)
!1564 = distinct !DILexicalBlock(scope: !1529, file: !2, line: 72, column: 6)
!1565 = !DILocation(line: 72, column: 13, scope: !1564)
!1566 = !DILocation(line: 72, column: 21, scope: !1564)
!1567 = !DILocation(line: 72, column: 24, scope: !1564)
!1568 = !DILocation(line: 72, column: 18, scope: !1564)
!1569 = !DILocation(line: 72, column: 6, scope: !1529)
!1570 = !DILocation(line: 73, column: 31, scope: !1564)
!1571 = !DILocation(line: 73, column: 34, scope: !1564)
!1572 = !DILocation(line: 73, column: 58, scope: !1564)
!1573 = !DILocation(line: 73, column: 3, scope: !1564)
!1574 = !DILocation(line: 73, column: 6, scope: !1564)
!1575 = !DILocation(line: 73, column: 29, scope: !1564)
!1576 = !DILocation(line: 75, column: 25, scope: !1529)
!1577 = !DILocation(line: 75, column: 28, scope: !1529)
!1578 = !DILocation(line: 75, column: 34, scope: !1529)
!1579 = !DILocation(line: 75, column: 2, scope: !1529)
!1580 = !DILocation(line: 77, column: 15, scope: !1529)
!1581 = !DILocation(line: 77, column: 2, scope: !1529)
!1582 = !DILocation(line: 77, column: 8, scope: !1529)
!1583 = !DILocation(line: 77, column: 13, scope: !1529)
!1584 = !DILocation(line: 78, column: 13, scope: !1529)
!1585 = !DILocation(line: 78, column: 19, scope: !1529)
!1586 = !DILocation(line: 78, column: 25, scope: !1529)
!1587 = !DILocation(line: 78, column: 2, scope: !1529)
!1588 = !DILocation(line: 79, column: 1, scope: !1529)
!1589 = distinct !DISubprogram(name: "bpf_lru_list_count_dec", scope: !2, file: !2, line: 54, type: !1590, scopeLine: 56, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1590 = !DISubroutineType(types: !1591)
!1591 = !{null, !12, !141}
!1592 = !DILocalVariable(name: "l", arg: 1, scope: !1589, file: !2, line: 54, type: !12)
!1593 = !DILocation(line: 54, column: 57, scope: !1589)
!1594 = !DILocalVariable(name: "type", arg: 2, scope: !1589, file: !2, line: 55, type: !141)
!1595 = !DILocation(line: 55, column: 31, scope: !1589)
!1596 = !DILocation(line: 57, column: 6, scope: !1597)
!1597 = distinct !DILexicalBlock(scope: !1589, file: !2, line: 57, column: 6)
!1598 = !DILocation(line: 57, column: 11, scope: !1597)
!1599 = !DILocation(line: 57, column: 6, scope: !1589)
!1600 = !DILocation(line: 58, column: 3, scope: !1597)
!1601 = !DILocation(line: 58, column: 6, scope: !1597)
!1602 = !DILocation(line: 58, column: 13, scope: !1597)
!1603 = !DILocation(line: 58, column: 18, scope: !1597)
!1604 = !DILocation(line: 59, column: 1, scope: !1589)
!1605 = distinct !DISubprogram(name: "list_move", scope: !1072, file: !1072, line: 213, type: !1606, scopeLine: 214, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1606 = !DISubroutineType(types: !1607)
!1607 = !{null, !22, !22}
!1608 = !DILocalVariable(name: "list", arg: 1, scope: !1605, file: !1072, line: 213, type: !22)
!1609 = !DILocation(line: 213, column: 48, scope: !1605)
!1610 = !DILocalVariable(name: "head", arg: 2, scope: !1605, file: !1072, line: 213, type: !22)
!1611 = !DILocation(line: 213, column: 72, scope: !1605)
!1612 = !DILocation(line: 215, column: 19, scope: !1605)
!1613 = !DILocation(line: 215, column: 2, scope: !1605)
!1614 = !DILocation(line: 216, column: 11, scope: !1605)
!1615 = !DILocation(line: 216, column: 17, scope: !1605)
!1616 = !DILocation(line: 216, column: 2, scope: !1605)
!1617 = !DILocation(line: 217, column: 1, scope: !1605)
!1618 = distinct !DISubprogram(name: "__list_del_entry", scope: !1072, file: !1072, line: 130, type: !1619, scopeLine: 131, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1619 = !DISubroutineType(types: !1620)
!1620 = !{null, !22}
!1621 = !DILocalVariable(name: "entry", arg: 1, scope: !1618, file: !1072, line: 130, type: !22)
!1622 = !DILocation(line: 130, column: 55, scope: !1618)
!1623 = !DILocation(line: 132, column: 30, scope: !1624)
!1624 = distinct !DILexicalBlock(scope: !1618, file: !1072, line: 132, column: 6)
!1625 = !DILocation(line: 132, column: 7, scope: !1624)
!1626 = !DILocation(line: 132, column: 6, scope: !1618)
!1627 = !DILocation(line: 133, column: 3, scope: !1624)
!1628 = !DILocation(line: 135, column: 13, scope: !1618)
!1629 = !DILocation(line: 135, column: 20, scope: !1618)
!1630 = !DILocation(line: 135, column: 26, scope: !1618)
!1631 = !DILocation(line: 135, column: 33, scope: !1618)
!1632 = !DILocation(line: 135, column: 2, scope: !1618)
!1633 = !DILocation(line: 136, column: 1, scope: !1618)
!1634 = distinct !DISubprogram(name: "list_add", scope: !1072, file: !1072, line: 84, type: !1606, scopeLine: 85, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1635 = !DILocalVariable(name: "new", arg: 1, scope: !1634, file: !1072, line: 84, type: !22)
!1636 = !DILocation(line: 84, column: 47, scope: !1634)
!1637 = !DILocalVariable(name: "head", arg: 2, scope: !1634, file: !1072, line: 84, type: !22)
!1638 = !DILocation(line: 84, column: 70, scope: !1634)
!1639 = !DILocation(line: 86, column: 13, scope: !1634)
!1640 = !DILocation(line: 86, column: 18, scope: !1634)
!1641 = !DILocation(line: 86, column: 24, scope: !1634)
!1642 = !DILocation(line: 86, column: 30, scope: !1634)
!1643 = !DILocation(line: 86, column: 2, scope: !1634)
!1644 = !DILocation(line: 87, column: 1, scope: !1634)
!1645 = distinct !DISubprogram(name: "__list_del", scope: !1072, file: !1072, line: 110, type: !1606, scopeLine: 111, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1646 = !DILocalVariable(name: "prev", arg: 1, scope: !1645, file: !1072, line: 110, type: !22)
!1647 = !DILocation(line: 110, column: 50, scope: !1645)
!1648 = !DILocalVariable(name: "next", arg: 2, scope: !1645, file: !1072, line: 110, type: !22)
!1649 = !DILocation(line: 110, column: 75, scope: !1645)
!1650 = !DILocation(line: 112, column: 15, scope: !1645)
!1651 = !DILocation(line: 112, column: 2, scope: !1645)
!1652 = !DILocation(line: 112, column: 8, scope: !1645)
!1653 = !DILocation(line: 112, column: 13, scope: !1645)
!1654 = !DILocation(line: 113, column: 2, scope: !1645)
!1655 = !DILocation(line: 113, column: 2, scope: !1656)
!1656 = distinct !DILexicalBlock(scope: !1645, file: !1072, line: 113, column: 2)
!1657 = !DILocation(line: 113, column: 2, scope: !1658)
!1658 = distinct !DILexicalBlock(scope: !1656, file: !1072, line: 113, column: 2)
!1659 = !DILocation(line: 113, column: 2, scope: !1660)
!1660 = distinct !DILexicalBlock(scope: !1656, file: !1072, line: 113, column: 2)
!1661 = !DILocation(line: 114, column: 1, scope: !1645)
!1662 = distinct !DISubprogram(name: "__list_add", scope: !1072, file: !1072, line: 63, type: !1663, scopeLine: 66, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1663 = !DISubroutineType(types: !1664)
!1664 = !{null, !22, !22, !22}
!1665 = !DILocalVariable(name: "new", arg: 1, scope: !1662, file: !1072, line: 63, type: !22)
!1666 = !DILocation(line: 63, column: 49, scope: !1662)
!1667 = !DILocalVariable(name: "prev", arg: 2, scope: !1662, file: !1072, line: 64, type: !22)
!1668 = !DILocation(line: 64, column: 28, scope: !1662)
!1669 = !DILocalVariable(name: "next", arg: 3, scope: !1662, file: !1072, line: 65, type: !22)
!1670 = !DILocation(line: 65, column: 28, scope: !1662)
!1671 = !DILocation(line: 67, column: 24, scope: !1672)
!1672 = distinct !DILexicalBlock(scope: !1662, file: !1072, line: 67, column: 6)
!1673 = !DILocation(line: 67, column: 29, scope: !1672)
!1674 = !DILocation(line: 67, column: 35, scope: !1672)
!1675 = !DILocation(line: 67, column: 7, scope: !1672)
!1676 = !DILocation(line: 67, column: 6, scope: !1662)
!1677 = !DILocation(line: 68, column: 3, scope: !1672)
!1678 = !DILocation(line: 70, column: 15, scope: !1662)
!1679 = !DILocation(line: 70, column: 2, scope: !1662)
!1680 = !DILocation(line: 70, column: 8, scope: !1662)
!1681 = !DILocation(line: 70, column: 13, scope: !1662)
!1682 = !DILocation(line: 71, column: 14, scope: !1662)
!1683 = !DILocation(line: 71, column: 2, scope: !1662)
!1684 = !DILocation(line: 71, column: 7, scope: !1662)
!1685 = !DILocation(line: 71, column: 12, scope: !1662)
!1686 = !DILocation(line: 72, column: 14, scope: !1662)
!1687 = !DILocation(line: 72, column: 2, scope: !1662)
!1688 = !DILocation(line: 72, column: 7, scope: !1662)
!1689 = !DILocation(line: 72, column: 12, scope: !1662)
!1690 = !DILocation(line: 73, column: 2, scope: !1662)
!1691 = !DILocation(line: 73, column: 2, scope: !1692)
!1692 = distinct !DILexicalBlock(scope: !1662, file: !1072, line: 73, column: 2)
!1693 = !DILocation(line: 73, column: 2, scope: !1694)
!1694 = distinct !DILexicalBlock(scope: !1692, file: !1072, line: 73, column: 2)
!1695 = !DILocation(line: 73, column: 2, scope: !1696)
!1696 = distinct !DILexicalBlock(scope: !1692, file: !1072, line: 73, column: 2)
!1697 = !DILocation(line: 74, column: 1, scope: !1662)
!1698 = distinct !DISubprogram(name: "bpf_lru_list_count_inc", scope: !2, file: !2, line: 47, type: !1590, scopeLine: 49, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1699 = !DILocalVariable(name: "l", arg: 1, scope: !1698, file: !2, line: 47, type: !12)
!1700 = !DILocation(line: 47, column: 57, scope: !1698)
!1701 = !DILocalVariable(name: "type", arg: 2, scope: !1698, file: !2, line: 48, type: !141)
!1702 = !DILocation(line: 48, column: 31, scope: !1698)
!1703 = !DILocation(line: 50, column: 6, scope: !1704)
!1704 = distinct !DILexicalBlock(scope: !1698, file: !2, line: 50, column: 6)
!1705 = !DILocation(line: 50, column: 11, scope: !1704)
!1706 = !DILocation(line: 50, column: 6, scope: !1698)
!1707 = !DILocation(line: 51, column: 3, scope: !1704)
!1708 = !DILocation(line: 51, column: 6, scope: !1704)
!1709 = !DILocation(line: 51, column: 13, scope: !1704)
!1710 = !DILocation(line: 51, column: 18, scope: !1704)
!1711 = !DILocation(line: 52, column: 1, scope: !1698)
!1712 = distinct !DISubprogram(name: "__local_list_pop_free", scope: !2, file: !2, line: 361, type: !1713, scopeLine: 362, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1713 = !DISubroutineType(types: !1714)
!1714 = !{!179, !171}
!1715 = !DILocalVariable(name: "loc_l", arg: 1, scope: !1712, file: !2, line: 361, type: !171)
!1716 = !DILocation(line: 361, column: 49, scope: !1712)
!1717 = !DILocalVariable(name: "node", scope: !1712, file: !2, line: 363, type: !179)
!1718 = !DILocation(line: 363, column: 23, scope: !1712)
!1719 = !DILocalVariable(name: "head__", scope: !1720, file: !2, line: 365, type: !22)
!1720 = distinct !DILexicalBlock(scope: !1712, file: !2, line: 365, column: 9)
!1721 = !DILocation(line: 365, column: 9, scope: !1720)
!1722 = !DILocalVariable(name: "pos__", scope: !1720, file: !2, line: 365, type: !22)
!1723 = !DILocation(line: 365, column: 9, scope: !1724)
!1724 = distinct !DILexicalBlock(scope: !1720, file: !2, line: 365, column: 9)
!1725 = !DILocation(line: 365, column: 9, scope: !1726)
!1726 = distinct !DILexicalBlock(scope: !1724, file: !2, line: 365, column: 9)
!1727 = !DILocalVariable(name: "__mptr", scope: !1728, file: !2, line: 365, type: !72)
!1728 = distinct !DILexicalBlock(scope: !1720, file: !2, line: 365, column: 9)
!1729 = !DILocation(line: 365, column: 9, scope: !1728)
!1730 = !DILocation(line: 365, column: 9, scope: !1731)
!1731 = distinct !DILexicalBlock(scope: !1728, file: !2, line: 365, column: 9)
!1732 = !DILocation(line: 365, column: 7, scope: !1712)
!1733 = !DILocation(line: 368, column: 6, scope: !1734)
!1734 = distinct !DILexicalBlock(scope: !1712, file: !2, line: 368, column: 6)
!1735 = !DILocation(line: 368, column: 6, scope: !1712)
!1736 = !DILocation(line: 369, column: 13, scope: !1734)
!1737 = !DILocation(line: 369, column: 19, scope: !1734)
!1738 = !DILocation(line: 369, column: 3, scope: !1734)
!1739 = !DILocation(line: 371, column: 9, scope: !1712)
!1740 = !DILocation(line: 371, column: 2, scope: !1712)
!1741 = distinct !DISubprogram(name: "bpf_lru_list_pop_free_to_local", scope: !2, file: !2, line: 318, type: !1742, scopeLine: 320, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1742 = !DISubroutineType(types: !1743)
!1743 = !{null, !221, !171}
!1744 = !DILocalVariable(name: "lru", arg: 1, scope: !1741, file: !2, line: 318, type: !221)
!1745 = !DILocation(line: 318, column: 60, scope: !1741)
!1746 = !DILocalVariable(name: "loc_l", arg: 2, scope: !1741, file: !2, line: 319, type: !171)
!1747 = !DILocation(line: 319, column: 35, scope: !1741)
!1748 = !DILocalVariable(name: "l", scope: !1741, file: !2, line: 321, type: !12)
!1749 = !DILocation(line: 321, column: 23, scope: !1741)
!1750 = !DILocation(line: 321, column: 28, scope: !1741)
!1751 = !DILocation(line: 321, column: 33, scope: !1741)
!1752 = !DILocation(line: 321, column: 44, scope: !1741)
!1753 = !DILocalVariable(name: "node", scope: !1741, file: !2, line: 322, type: !179)
!1754 = !DILocation(line: 322, column: 23, scope: !1741)
!1755 = !DILocalVariable(name: "tmp_node", scope: !1741, file: !2, line: 322, type: !179)
!1756 = !DILocation(line: 322, column: 30, scope: !1741)
!1757 = !DILocalVariable(name: "nfree", scope: !1741, file: !2, line: 323, type: !28)
!1758 = !DILocation(line: 323, column: 15, scope: !1741)
!1759 = !DILocation(line: 325, column: 2, scope: !1741)
!1760 = !DILocation(line: 327, column: 21, scope: !1741)
!1761 = !DILocation(line: 327, column: 24, scope: !1741)
!1762 = !DILocation(line: 327, column: 2, scope: !1741)
!1763 = !DILocation(line: 329, column: 24, scope: !1741)
!1764 = !DILocation(line: 329, column: 29, scope: !1741)
!1765 = !DILocation(line: 329, column: 2, scope: !1741)
!1766 = !DILocalVariable(name: "__mptr", scope: !1767, file: !2, line: 331, type: !72)
!1767 = distinct !DILexicalBlock(scope: !1768, file: !2, line: 331, column: 2)
!1768 = distinct !DILexicalBlock(scope: !1741, file: !2, line: 331, column: 2)
!1769 = !DILocation(line: 331, column: 2, scope: !1767)
!1770 = !DILocation(line: 331, column: 2, scope: !1771)
!1771 = distinct !DILexicalBlock(scope: !1767, file: !2, line: 331, column: 2)
!1772 = !DILocation(line: 331, column: 2, scope: !1768)
!1773 = !DILocalVariable(name: "__mptr", scope: !1774, file: !2, line: 331, type: !72)
!1774 = distinct !DILexicalBlock(scope: !1768, file: !2, line: 331, column: 2)
!1775 = !DILocation(line: 331, column: 2, scope: !1774)
!1776 = !DILocation(line: 331, column: 2, scope: !1777)
!1777 = distinct !DILexicalBlock(scope: !1774, file: !2, line: 331, column: 2)
!1778 = !DILocation(line: 331, column: 2, scope: !1779)
!1779 = distinct !DILexicalBlock(scope: !1768, file: !2, line: 331, column: 2)
!1780 = !DILocation(line: 333, column: 31, scope: !1781)
!1781 = distinct !DILexicalBlock(scope: !1779, file: !2, line: 332, column: 12)
!1782 = !DILocation(line: 333, column: 34, scope: !1781)
!1783 = !DILocation(line: 333, column: 56, scope: !1781)
!1784 = !DILocation(line: 333, column: 40, scope: !1781)
!1785 = !DILocation(line: 333, column: 3, scope: !1781)
!1786 = !DILocation(line: 335, column: 7, scope: !1787)
!1787 = distinct !DILexicalBlock(scope: !1781, file: !2, line: 335, column: 7)
!1788 = !DILocation(line: 335, column: 15, scope: !1787)
!1789 = !DILocation(line: 335, column: 7, scope: !1781)
!1790 = !DILocation(line: 336, column: 4, scope: !1787)
!1791 = !DILocation(line: 337, column: 2, scope: !1781)
!1792 = !DILocalVariable(name: "__mptr", scope: !1793, file: !2, line: 331, type: !72)
!1793 = distinct !DILexicalBlock(scope: !1779, file: !2, line: 331, column: 2)
!1794 = !DILocation(line: 331, column: 2, scope: !1793)
!1795 = !DILocation(line: 331, column: 2, scope: !1796)
!1796 = distinct !DILexicalBlock(scope: !1793, file: !2, line: 331, column: 2)
!1797 = distinct !{!1797, !1772, !1798, !489}
!1798 = !DILocation(line: 337, column: 2, scope: !1768)
!1799 = !DILocation(line: 339, column: 6, scope: !1800)
!1800 = distinct !DILexicalBlock(scope: !1741, file: !2, line: 339, column: 6)
!1801 = !DILocation(line: 339, column: 12, scope: !1800)
!1802 = !DILocation(line: 339, column: 6, scope: !1741)
!1803 = !DILocation(line: 340, column: 25, scope: !1800)
!1804 = !DILocation(line: 340, column: 30, scope: !1800)
!1805 = !DILocation(line: 340, column: 53, scope: !1800)
!1806 = !DILocation(line: 340, column: 51, scope: !1800)
!1807 = !DILocation(line: 341, column: 27, scope: !1800)
!1808 = !DILocation(line: 341, column: 11, scope: !1800)
!1809 = !DILocation(line: 340, column: 3, scope: !1800)
!1810 = !DILocation(line: 344, column: 2, scope: !1741)
!1811 = !DILocation(line: 345, column: 1, scope: !1741)
!1812 = distinct !DISubprogram(name: "__local_list_add_pending", scope: !2, file: !2, line: 347, type: !1813, scopeLine: 352, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1813 = !DISubroutineType(types: !1814)
!1814 = !{null, !221, !171, !50, !179, !187}
!1815 = !DILocalVariable(name: "lru", arg: 1, scope: !1812, file: !2, line: 347, type: !221)
!1816 = !DILocation(line: 347, column: 54, scope: !1812)
!1817 = !DILocalVariable(name: "loc_l", arg: 2, scope: !1812, file: !2, line: 348, type: !171)
!1818 = !DILocation(line: 348, column: 36, scope: !1812)
!1819 = !DILocalVariable(name: "cpu", arg: 3, scope: !1812, file: !2, line: 349, type: !50)
!1820 = !DILocation(line: 349, column: 14, scope: !1812)
!1821 = !DILocalVariable(name: "node", arg: 4, scope: !1812, file: !2, line: 350, type: !179)
!1822 = !DILocation(line: 350, column: 31, scope: !1812)
!1823 = !DILocalVariable(name: "hash", arg: 5, scope: !1812, file: !2, line: 351, type: !187)
!1824 = !DILocation(line: 351, column: 14, scope: !1812)
!1825 = !DILocation(line: 353, column: 46, scope: !1812)
!1826 = !DILocation(line: 353, column: 19, scope: !1812)
!1827 = !DILocation(line: 353, column: 26, scope: !1812)
!1828 = !DILocation(line: 353, column: 31, scope: !1812)
!1829 = !DILocation(line: 353, column: 24, scope: !1812)
!1830 = !DILocation(line: 353, column: 44, scope: !1812)
!1831 = !DILocation(line: 354, column: 14, scope: !1812)
!1832 = !DILocation(line: 354, column: 2, scope: !1812)
!1833 = !DILocation(line: 354, column: 8, scope: !1812)
!1834 = !DILocation(line: 354, column: 12, scope: !1812)
!1835 = !DILocation(line: 355, column: 2, scope: !1812)
!1836 = !DILocation(line: 355, column: 8, scope: !1812)
!1837 = !DILocation(line: 355, column: 13, scope: !1812)
!1838 = !DILocation(line: 356, column: 2, scope: !1812)
!1839 = !DILocation(line: 356, column: 8, scope: !1812)
!1840 = !DILocation(line: 356, column: 12, scope: !1812)
!1841 = !DILocation(line: 357, column: 12, scope: !1812)
!1842 = !DILocation(line: 357, column: 18, scope: !1812)
!1843 = !DILocation(line: 357, column: 43, scope: !1812)
!1844 = !DILocation(line: 357, column: 24, scope: !1812)
!1845 = !DILocation(line: 357, column: 2, scope: !1812)
!1846 = !DILocation(line: 358, column: 1, scope: !1812)
!1847 = distinct !DISubprogram(name: "__local_list_pop_pending", scope: !2, file: !2, line: 375, type: !1848, scopeLine: 376, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1848 = !DISubroutineType(types: !1849)
!1849 = !{!179, !221, !171}
!1850 = !DILocalVariable(name: "lru", arg: 1, scope: !1847, file: !2, line: 375, type: !221)
!1851 = !DILocation(line: 375, column: 42, scope: !1847)
!1852 = !DILocalVariable(name: "loc_l", arg: 2, scope: !1847, file: !2, line: 375, type: !171)
!1853 = !DILocation(line: 375, column: 73, scope: !1847)
!1854 = !DILocalVariable(name: "node", scope: !1847, file: !2, line: 377, type: !179)
!1855 = !DILocation(line: 377, column: 23, scope: !1847)
!1856 = !DILocalVariable(name: "force", scope: !1847, file: !2, line: 378, type: !238)
!1857 = !DILocation(line: 378, column: 7, scope: !1847)
!1858 = !DILocation(line: 378, column: 2, scope: !1847)
!1859 = !DILabel(scope: !1847, name: "ignore_ref", file: !2, line: 380)
!1860 = !DILocation(line: 380, column: 1, scope: !1847)
!1861 = !DILocalVariable(name: "__mptr", scope: !1862, file: !2, line: 382, type: !72)
!1862 = distinct !DILexicalBlock(scope: !1863, file: !2, line: 382, column: 2)
!1863 = distinct !DILexicalBlock(scope: !1847, file: !2, line: 382, column: 2)
!1864 = !DILocation(line: 382, column: 2, scope: !1862)
!1865 = !DILocation(line: 382, column: 2, scope: !1866)
!1866 = distinct !DILexicalBlock(scope: !1862, file: !2, line: 382, column: 2)
!1867 = !DILocation(line: 382, column: 2, scope: !1863)
!1868 = !DILocation(line: 382, column: 2, scope: !1869)
!1869 = distinct !DILexicalBlock(scope: !1863, file: !2, line: 382, column: 2)
!1870 = !DILocation(line: 384, column: 29, scope: !1871)
!1871 = distinct !DILexicalBlock(scope: !1872, file: !2, line: 384, column: 7)
!1872 = distinct !DILexicalBlock(scope: !1869, file: !2, line: 383, column: 15)
!1873 = !DILocation(line: 384, column: 9, scope: !1871)
!1874 = !DILocation(line: 384, column: 35, scope: !1871)
!1875 = !DILocation(line: 384, column: 38, scope: !1871)
!1876 = !DILocation(line: 384, column: 45, scope: !1871)
!1877 = !DILocation(line: 385, column: 7, scope: !1871)
!1878 = !DILocation(line: 385, column: 12, scope: !1871)
!1879 = !DILocation(line: 385, column: 26, scope: !1871)
!1880 = !DILocation(line: 385, column: 31, scope: !1871)
!1881 = !DILocation(line: 385, column: 40, scope: !1871)
!1882 = !DILocation(line: 384, column: 7, scope: !1872)
!1883 = !DILocation(line: 386, column: 14, scope: !1884)
!1884 = distinct !DILexicalBlock(scope: !1871, file: !2, line: 385, column: 47)
!1885 = !DILocation(line: 386, column: 20, scope: !1884)
!1886 = !DILocation(line: 386, column: 4, scope: !1884)
!1887 = !DILocation(line: 387, column: 11, scope: !1884)
!1888 = !DILocation(line: 387, column: 4, scope: !1884)
!1889 = !DILocation(line: 389, column: 2, scope: !1872)
!1890 = !DILocalVariable(name: "__mptr", scope: !1891, file: !2, line: 382, type: !72)
!1891 = distinct !DILexicalBlock(scope: !1869, file: !2, line: 382, column: 2)
!1892 = !DILocation(line: 382, column: 2, scope: !1891)
!1893 = !DILocation(line: 382, column: 2, scope: !1894)
!1894 = distinct !DILexicalBlock(scope: !1891, file: !2, line: 382, column: 2)
!1895 = distinct !{!1895, !1867, !1896, !489}
!1896 = !DILocation(line: 389, column: 2, scope: !1863)
!1897 = !DILocation(line: 391, column: 7, scope: !1898)
!1898 = distinct !DILexicalBlock(scope: !1847, file: !2, line: 391, column: 6)
!1899 = !DILocation(line: 391, column: 6, scope: !1847)
!1900 = !DILocation(line: 392, column: 9, scope: !1901)
!1901 = distinct !DILexicalBlock(scope: !1898, file: !2, line: 391, column: 14)
!1902 = !DILocation(line: 393, column: 3, scope: !1901)
!1903 = !DILocation(line: 396, column: 2, scope: !1847)
!1904 = !DILocation(line: 397, column: 1, scope: !1847)
!1905 = distinct !DISubprogram(name: "get_next_cpu", scope: !2, file: !2, line: 22, type: !1906, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1906 = !DISubroutineType(types: !1907)
!1907 = !{!50, !50}
!1908 = !DILocalVariable(name: "cpu", arg: 1, scope: !1905, file: !2, line: 22, type: !50)
!1909 = !DILocation(line: 22, column: 29, scope: !1905)
!1910 = !DILocation(line: 24, column: 21, scope: !1905)
!1911 = !DILocation(line: 24, column: 8, scope: !1905)
!1912 = !DILocation(line: 24, column: 6, scope: !1905)
!1913 = !DILocation(line: 25, column: 6, scope: !1914)
!1914 = distinct !DILexicalBlock(scope: !1905, file: !2, line: 25, column: 6)
!1915 = !DILocation(line: 25, column: 13, scope: !1914)
!1916 = !DILocation(line: 25, column: 10, scope: !1914)
!1917 = !DILocation(line: 25, column: 6, scope: !1905)
!1918 = !DILocation(line: 26, column: 9, scope: !1914)
!1919 = !DILocation(line: 26, column: 7, scope: !1914)
!1920 = !DILocation(line: 26, column: 3, scope: !1914)
!1921 = !DILocation(line: 27, column: 9, scope: !1905)
!1922 = !DILocation(line: 27, column: 2, scope: !1905)
!1923 = distinct !DISubprogram(name: "local_free_list", scope: !2, file: !2, line: 31, type: !1924, scopeLine: 32, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1924 = !DISubroutineType(types: !1925)
!1925 = !{!22, !171}
!1926 = !DILocalVariable(name: "loc_l", arg: 1, scope: !1923, file: !2, line: 31, type: !171)
!1927 = !DILocation(line: 31, column: 68, scope: !1923)
!1928 = !DILocation(line: 33, column: 10, scope: !1923)
!1929 = !DILocation(line: 33, column: 17, scope: !1923)
!1930 = !DILocation(line: 33, column: 2, scope: !1923)
!1931 = distinct !DISubprogram(name: "list_del", scope: !1072, file: !1072, line: 144, type: !1619, scopeLine: 145, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1932 = !DILocalVariable(name: "entry", arg: 1, scope: !1931, file: !1072, line: 144, type: !22)
!1933 = !DILocation(line: 144, column: 47, scope: !1931)
!1934 = !DILocation(line: 146, column: 19, scope: !1931)
!1935 = !DILocation(line: 146, column: 2, scope: !1931)
!1936 = !DILocation(line: 147, column: 2, scope: !1931)
!1937 = !DILocation(line: 147, column: 9, scope: !1931)
!1938 = !DILocation(line: 147, column: 14, scope: !1931)
!1939 = !DILocation(line: 148, column: 2, scope: !1931)
!1940 = !DILocation(line: 148, column: 9, scope: !1931)
!1941 = !DILocation(line: 148, column: 14, scope: !1931)
!1942 = !DILocation(line: 149, column: 1, scope: !1931)
!1943 = distinct !DISubprogram(name: "__local_list_flush", scope: !2, file: !2, line: 290, type: !1944, scopeLine: 292, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1944 = !DISubroutineType(types: !1945)
!1945 = !{null, !12, !171}
!1946 = !DILocalVariable(name: "l", arg: 1, scope: !1943, file: !2, line: 290, type: !12)
!1947 = !DILocation(line: 290, column: 53, scope: !1943)
!1948 = !DILocalVariable(name: "loc_l", arg: 2, scope: !1943, file: !2, line: 291, type: !171)
!1949 = !DILocation(line: 291, column: 37, scope: !1943)
!1950 = !DILocalVariable(name: "node", scope: !1943, file: !2, line: 293, type: !179)
!1951 = !DILocation(line: 293, column: 23, scope: !1943)
!1952 = !DILocalVariable(name: "tmp_node", scope: !1943, file: !2, line: 293, type: !179)
!1953 = !DILocation(line: 293, column: 30, scope: !1943)
!1954 = !DILocalVariable(name: "__mptr", scope: !1955, file: !2, line: 295, type: !72)
!1955 = distinct !DILexicalBlock(scope: !1956, file: !2, line: 295, column: 2)
!1956 = distinct !DILexicalBlock(scope: !1943, file: !2, line: 295, column: 2)
!1957 = !DILocation(line: 295, column: 2, scope: !1955)
!1958 = !DILocation(line: 295, column: 2, scope: !1959)
!1959 = distinct !DILexicalBlock(scope: !1955, file: !2, line: 295, column: 2)
!1960 = !DILocation(line: 295, column: 2, scope: !1956)
!1961 = !DILocalVariable(name: "__mptr", scope: !1962, file: !2, line: 295, type: !72)
!1962 = distinct !DILexicalBlock(scope: !1956, file: !2, line: 295, column: 2)
!1963 = !DILocation(line: 295, column: 2, scope: !1962)
!1964 = !DILocation(line: 295, column: 2, scope: !1965)
!1965 = distinct !DILexicalBlock(scope: !1962, file: !2, line: 295, column: 2)
!1966 = !DILocation(line: 295, column: 2, scope: !1967)
!1967 = distinct !DILexicalBlock(scope: !1956, file: !2, line: 295, column: 2)
!1968 = !DILocation(line: 297, column: 27, scope: !1969)
!1969 = distinct !DILexicalBlock(scope: !1970, file: !2, line: 297, column: 7)
!1970 = distinct !DILexicalBlock(scope: !1967, file: !2, line: 296, column: 40)
!1971 = !DILocation(line: 297, column: 7, scope: !1969)
!1972 = !DILocation(line: 297, column: 7, scope: !1970)
!1973 = !DILocation(line: 298, column: 27, scope: !1969)
!1974 = !DILocation(line: 298, column: 30, scope: !1969)
!1975 = !DILocation(line: 298, column: 4, scope: !1969)
!1976 = !DILocation(line: 300, column: 27, scope: !1969)
!1977 = !DILocation(line: 300, column: 30, scope: !1969)
!1978 = !DILocation(line: 300, column: 4, scope: !1969)
!1979 = !DILocation(line: 302, column: 2, scope: !1970)
!1980 = !DILocalVariable(name: "__mptr", scope: !1981, file: !2, line: 295, type: !72)
!1981 = distinct !DILexicalBlock(scope: !1967, file: !2, line: 295, column: 2)
!1982 = !DILocation(line: 295, column: 2, scope: !1981)
!1983 = !DILocation(line: 295, column: 2, scope: !1984)
!1984 = distinct !DILexicalBlock(scope: !1981, file: !2, line: 295, column: 2)
!1985 = distinct !{!1985, !1960, !1986, !489}
!1986 = !DILocation(line: 302, column: 2, scope: !1956)
!1987 = !DILocation(line: 303, column: 1, scope: !1943)
!1988 = distinct !DISubprogram(name: "local_pending_list", scope: !2, file: !2, line: 36, type: !1924, scopeLine: 37, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1989 = !DILocalVariable(name: "loc_l", arg: 1, scope: !1988, file: !2, line: 36, type: !171)
!1990 = !DILocation(line: 36, column: 71, scope: !1988)
!1991 = !DILocation(line: 38, column: 10, scope: !1988)
!1992 = !DILocation(line: 38, column: 17, scope: !1988)
!1993 = !DILocation(line: 38, column: 2, scope: !1988)
!1994 = distinct !DISubprogram(name: "__bpf_lru_node_move_in", scope: !2, file: !2, line: 82, type: !1171, scopeLine: 85, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!1995 = !DILocalVariable(name: "l", arg: 1, scope: !1994, file: !2, line: 82, type: !12)
!1996 = !DILocation(line: 82, column: 57, scope: !1994)
!1997 = !DILocalVariable(name: "node", arg: 2, scope: !1994, file: !2, line: 83, type: !179)
!1998 = !DILocation(line: 83, column: 29, scope: !1994)
!1999 = !DILocalVariable(name: "tgt_type", arg: 3, scope: !1994, file: !2, line: 84, type: !141)
!2000 = !DILocation(line: 84, column: 31, scope: !1994)
!2001 = !DILocalVariable(name: "__ret_warn_on", scope: !2002, file: !2, line: 86, type: !50)
!2002 = distinct !DILexicalBlock(scope: !2003, file: !2, line: 86, column: 6)
!2003 = distinct !DILexicalBlock(scope: !1994, file: !2, line: 86, column: 6)
!2004 = !DILocation(line: 86, column: 6, scope: !2002)
!2005 = !DILocation(line: 86, column: 6, scope: !2006)
!2006 = distinct !DILexicalBlock(scope: !2002, file: !2, line: 86, column: 6)
!2007 = !DILocation(line: 86, column: 6, scope: !2008)
!2008 = distinct !DILexicalBlock(scope: !2009, file: !2, line: 86, column: 6)
!2009 = distinct !DILexicalBlock(scope: !2006, file: !2, line: 86, column: 6)
!2010 = !{i64 2150353162, i64 2150353173, i64 2150353227, i64 2150353258, i64 2150353288}
!2011 = !DILocation(line: 86, column: 6, scope: !2009)
!2012 = !DILocation(line: 86, column: 6, scope: !2013)
!2013 = distinct !DILexicalBlock(scope: !2009, file: !2, line: 86, column: 6)
!2014 = !{i64 2150353364, i64 2150353393, i64 2150353439, i64 2150353497, i64 2150353551, i64 2150353605, i64 2150353660, i64 2150353691}
!2015 = !DILocation(line: 86, column: 6, scope: !2016)
!2016 = distinct !DILexicalBlock(scope: !2009, file: !2, line: 86, column: 6)
!2017 = !{i64 2150354169, i64 2150354176, i64 2150354228, i64 2150354259, i64 2150354289}
!2018 = !DILocation(line: 86, column: 6, scope: !2019)
!2019 = distinct !DILexicalBlock(scope: !2009, file: !2, line: 86, column: 6)
!2020 = !{i64 2150354350, i64 2150354361, i64 2150354412, i64 2150354443, i64 2150354473}
!2021 = !DILocation(line: 86, column: 6, scope: !2003)
!2022 = !DILocation(line: 86, column: 52, scope: !2003)
!2023 = !DILocalVariable(name: "__ret_warn_on", scope: !2024, file: !2, line: 87, type: !50)
!2024 = distinct !DILexicalBlock(scope: !2003, file: !2, line: 87, column: 6)
!2025 = !DILocation(line: 87, column: 6, scope: !2024)
!2026 = !DILocation(line: 87, column: 6, scope: !2027)
!2027 = distinct !DILexicalBlock(scope: !2024, file: !2, line: 87, column: 6)
!2028 = !DILocation(line: 87, column: 6, scope: !2029)
!2029 = distinct !DILexicalBlock(scope: !2030, file: !2, line: 87, column: 6)
!2030 = distinct !DILexicalBlock(scope: !2027, file: !2, line: 87, column: 6)
!2031 = !{i64 2150355112, i64 2150355123, i64 2150355177, i64 2150355208, i64 2150355238}
!2032 = !DILocation(line: 87, column: 6, scope: !2030)
!2033 = !DILocation(line: 87, column: 6, scope: !2034)
!2034 = distinct !DILexicalBlock(scope: !2030, file: !2, line: 87, column: 6)
!2035 = !{i64 2150355314, i64 2150355343, i64 2150355389, i64 2150355447, i64 2150355501, i64 2150355555, i64 2150355610, i64 2150355641}
!2036 = !DILocation(line: 87, column: 6, scope: !2037)
!2037 = distinct !DILexicalBlock(scope: !2030, file: !2, line: 87, column: 6)
!2038 = !{i64 2150356119, i64 2150356126, i64 2150356178, i64 2150356209, i64 2150356239}
!2039 = !DILocation(line: 87, column: 6, scope: !2040)
!2040 = distinct !DILexicalBlock(scope: !2030, file: !2, line: 87, column: 6)
!2041 = !{i64 2150356300, i64 2150356311, i64 2150356362, i64 2150356393, i64 2150356423}
!2042 = !DILocation(line: 87, column: 6, scope: !2003)
!2043 = !DILocation(line: 86, column: 6, scope: !1994)
!2044 = !DILocation(line: 88, column: 3, scope: !2003)
!2045 = !DILocation(line: 90, column: 25, scope: !1994)
!2046 = !DILocation(line: 90, column: 28, scope: !1994)
!2047 = !DILocation(line: 90, column: 2, scope: !1994)
!2048 = !DILocation(line: 91, column: 15, scope: !1994)
!2049 = !DILocation(line: 91, column: 2, scope: !1994)
!2050 = !DILocation(line: 91, column: 8, scope: !1994)
!2051 = !DILocation(line: 91, column: 13, scope: !1994)
!2052 = !DILocation(line: 92, column: 2, scope: !1994)
!2053 = !DILocation(line: 92, column: 8, scope: !1994)
!2054 = !DILocation(line: 92, column: 12, scope: !1994)
!2055 = !DILocation(line: 93, column: 13, scope: !1994)
!2056 = !DILocation(line: 93, column: 19, scope: !1994)
!2057 = !DILocation(line: 93, column: 26, scope: !1994)
!2058 = !DILocation(line: 93, column: 29, scope: !1994)
!2059 = !DILocation(line: 93, column: 35, scope: !1994)
!2060 = !DILocation(line: 93, column: 2, scope: !1994)
!2061 = !DILocation(line: 94, column: 1, scope: !1994)
!2062 = distinct !DISubprogram(name: "cpumask_first", scope: !165, file: !165, line: 222, type: !2063, scopeLine: 223, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!2063 = !DISubroutineType(types: !2064)
!2064 = !{!28, !162}
!2065 = !DILocalVariable(name: "srcp", arg: 1, scope: !2062, file: !165, line: 222, type: !162)
!2066 = !DILocation(line: 222, column: 64, scope: !2062)
!2067 = !DILocation(line: 224, column: 24, scope: !2062)
!2068 = !DILocation(line: 224, column: 44, scope: !2062)
!2069 = !DILocation(line: 224, column: 9, scope: !2062)
!2070 = !DILocation(line: 224, column: 2, scope: !2062)
!2071 = distinct !DISubprogram(name: "bpf_lru_list_push_free", scope: !2, file: !2, line: 305, type: !2072, scopeLine: 307, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!2072 = !DISubroutineType(types: !2073)
!2073 = !{null, !12, !179}
!2074 = !DILocalVariable(name: "l", arg: 1, scope: !2071, file: !2, line: 305, type: !12)
!2075 = !DILocation(line: 305, column: 57, scope: !2071)
!2076 = !DILocalVariable(name: "node", arg: 2, scope: !2071, file: !2, line: 306, type: !179)
!2077 = !DILocation(line: 306, column: 29, scope: !2071)
!2078 = !DILocalVariable(name: "flags", scope: !2071, file: !2, line: 308, type: !113)
!2079 = !DILocation(line: 308, column: 16, scope: !2071)
!2080 = !DILocalVariable(name: "__ret_warn_on", scope: !2081, file: !2, line: 310, type: !50)
!2081 = distinct !DILexicalBlock(scope: !2082, file: !2, line: 310, column: 6)
!2082 = distinct !DILexicalBlock(scope: !2071, file: !2, line: 310, column: 6)
!2083 = !DILocation(line: 310, column: 6, scope: !2081)
!2084 = !DILocation(line: 310, column: 6, scope: !2085)
!2085 = distinct !DILexicalBlock(scope: !2081, file: !2, line: 310, column: 6)
!2086 = !DILocation(line: 310, column: 6, scope: !2087)
!2087 = distinct !DILexicalBlock(scope: !2088, file: !2, line: 310, column: 6)
!2088 = distinct !DILexicalBlock(scope: !2085, file: !2, line: 310, column: 6)
!2089 = !{i64 2150391625, i64 2150391636, i64 2150391690, i64 2150391721, i64 2150391751}
!2090 = !DILocation(line: 310, column: 6, scope: !2088)
!2091 = !DILocation(line: 310, column: 6, scope: !2092)
!2092 = distinct !DILexicalBlock(scope: !2088, file: !2, line: 310, column: 6)
!2093 = !{i64 2150391827, i64 2150391856, i64 2150391902, i64 2150391960, i64 2150392014, i64 2150392068, i64 2150392123, i64 2150392154}
!2094 = !DILocation(line: 310, column: 6, scope: !2095)
!2095 = distinct !DILexicalBlock(scope: !2088, file: !2, line: 310, column: 6)
!2096 = !{i64 2150392633, i64 2150392640, i64 2150392692, i64 2150392723, i64 2150392753}
!2097 = !DILocation(line: 310, column: 6, scope: !2098)
!2098 = distinct !DILexicalBlock(scope: !2088, file: !2, line: 310, column: 6)
!2099 = !{i64 2150392814, i64 2150392825, i64 2150392876, i64 2150392907, i64 2150392937}
!2100 = !DILocation(line: 310, column: 6, scope: !2082)
!2101 = !DILocation(line: 310, column: 6, scope: !2071)
!2102 = !DILocation(line: 311, column: 3, scope: !2082)
!2103 = !DILocation(line: 313, column: 2, scope: !2071)
!2104 = !DILocalVariable(name: "__dummy", scope: !2105, file: !2, line: 313, type: !113)
!2105 = distinct !DILexicalBlock(scope: !2106, file: !2, line: 313, column: 2)
!2106 = distinct !DILexicalBlock(scope: !2071, file: !2, line: 313, column: 2)
!2107 = !DILocation(line: 313, column: 2, scope: !2105)
!2108 = !DILocalVariable(name: "__dummy2", scope: !2105, file: !2, line: 313, type: !113)
!2109 = !DILocation(line: 313, column: 2, scope: !2106)
!2110 = !DILocation(line: 314, column: 22, scope: !2071)
!2111 = !DILocation(line: 314, column: 25, scope: !2071)
!2112 = !DILocation(line: 314, column: 2, scope: !2071)
!2113 = !DILocation(line: 315, column: 2, scope: !2071)
!2114 = !DILocalVariable(name: "__dummy", scope: !2115, file: !2, line: 315, type: !113)
!2115 = distinct !DILexicalBlock(scope: !2116, file: !2, line: 315, column: 2)
!2116 = distinct !DILexicalBlock(scope: !2071, file: !2, line: 315, column: 2)
!2117 = !DILocation(line: 315, column: 2, scope: !2115)
!2118 = !DILocalVariable(name: "__dummy2", scope: !2115, file: !2, line: 315, type: !113)
!2119 = !DILocation(line: 315, column: 2, scope: !2116)
!2120 = !DILocation(line: 316, column: 1, scope: !2071)
!2121 = distinct !DISubprogram(name: "cpumask_weight", scope: !165, file: !165, line: 567, type: !2063, scopeLine: 568, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!2122 = !DILocalVariable(name: "srcp", arg: 1, scope: !2121, file: !165, line: 567, type: !162)
!2123 = !DILocation(line: 567, column: 65, scope: !2121)
!2124 = !DILocation(line: 569, column: 23, scope: !2121)
!2125 = !DILocation(line: 569, column: 43, scope: !2121)
!2126 = !DILocalVariable(name: "src", arg: 1, scope: !2127, file: !2128, line: 396, type: !2131)
!2127 = distinct !DISubprogram(name: "bitmap_weight", scope: !2128, file: !2128, line: 396, type: !2129, scopeLine: 397, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!2128 = !DIFile(filename: "include/linux/bitmap.h", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "b86b2082e3368cd8808d9fc1b1acdc9e")
!2129 = !DISubroutineType(types: !2130)
!2130 = !{!50, !2131, !28}
!2131 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !2132, size: 64)
!2132 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !113)
!2133 = !DILocation(line: 396, column: 63, scope: !2127, inlinedAt: !2134)
!2134 = distinct !DILocation(line: 569, column: 9, scope: !2121)
!2135 = !DILocalVariable(name: "nbits", arg: 2, scope: !2127, file: !2128, line: 396, type: !28)
!2136 = !DILocation(line: 396, column: 81, scope: !2127, inlinedAt: !2134)
!2137 = !DILocation(line: 398, column: 6, scope: !2138, inlinedAt: !2134)
!2138 = distinct !DILexicalBlock(scope: !2127, file: !2128, line: 398, column: 6)
!2139 = !DILocation(line: 398, column: 6, scope: !2127, inlinedAt: !2134)
!2140 = !DILocation(line: 399, column: 24, scope: !2138, inlinedAt: !2134)
!2141 = !DILocation(line: 399, column: 23, scope: !2138, inlinedAt: !2134)
!2142 = !DILocation(line: 399, column: 30, scope: !2138, inlinedAt: !2134)
!2143 = !DILocation(line: 399, column: 28, scope: !2138, inlinedAt: !2134)
!2144 = !DILocalVariable(name: "w", arg: 1, scope: !2145, file: !2146, line: 76, type: !113)
!2145 = distinct !DISubprogram(name: "hweight_long", scope: !2146, file: !2146, line: 76, type: !2147, scopeLine: 77, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!2146 = !DIFile(filename: "include/linux/bitops.h", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "dcbee127ec1b93d754b3a8f77b47ed42")
!2147 = !DISubroutineType(types: !2148)
!2148 = !{!113, !113}
!2149 = !DILocation(line: 76, column: 65, scope: !2145, inlinedAt: !2150)
!2150 = distinct !DILocation(line: 399, column: 10, scope: !2138, inlinedAt: !2134)
!2151 = !DILocation(line: 78, column: 41, scope: !2145, inlinedAt: !2150)
!2152 = !DILocalVariable(name: "w", arg: 1, scope: !2153, file: !2154, line: 43, type: !193)
!2153 = distinct !DISubprogram(name: "__arch_hweight64", scope: !2154, file: !2154, line: 43, type: !2155, scopeLine: 44, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!2154 = !DIFile(filename: "arch/x86/include/asm/arch_hweight.h", directory: "/mlx_devbox/users/mayunlong.39/playground/linux.git", checksumkind: CSK_MD5, checksum: "49102469a535fe59caa96e0b4cb43e29")
!2155 = !DISubroutineType(types: !2156)
!2156 = !{!113, !193}
!2157 = !DILocation(line: 43, column: 61, scope: !2153, inlinedAt: !2158)
!2158 = distinct !DILocation(line: 78, column: 41, scope: !2145, inlinedAt: !2150)
!2159 = !DILocalVariable(name: "res", scope: !2153, file: !2154, line: 45, type: !113)
!2160 = !DILocation(line: 45, column: 16, scope: !2153, inlinedAt: !2158)
!2161 = !DILocation(line: 49, column: 15, scope: !2153, inlinedAt: !2158)
!2162 = !DILocation(line: 47, column: 2, scope: !2153, inlinedAt: !2158)
!2163 = !{i64 2148585502, i64 2148585530, i64 2148585536, i64 2148585552, i64 2148585568, i64 2148585595, i64 2148585928, i64 2148585210, i64 2148585934, i64 2148585982, i64 2148586046, i64 2148586108, i64 2148586165, i64 2148586221, i64 2148585290, i64 2148585315, i64 2148586455, i64 2148586585, i64 2148586516, i64 2148586599, i64 2148585415}
!2164 = !DILocation(line: 51, column: 9, scope: !2153, inlinedAt: !2158)
!2165 = !DILocation(line: 399, column: 10, scope: !2138, inlinedAt: !2134)
!2166 = !DILocation(line: 399, column: 3, scope: !2138, inlinedAt: !2134)
!2167 = !DILocation(line: 400, column: 25, scope: !2127, inlinedAt: !2134)
!2168 = !DILocation(line: 400, column: 30, scope: !2127, inlinedAt: !2134)
!2169 = !DILocation(line: 400, column: 9, scope: !2127, inlinedAt: !2134)
!2170 = !DILocation(line: 400, column: 2, scope: !2127, inlinedAt: !2134)
!2171 = !DILocation(line: 401, column: 1, scope: !2127, inlinedAt: !2134)
!2172 = !DILocation(line: 569, column: 2, scope: !2121)
!2173 = distinct !DISubprogram(name: "INIT_LIST_HEAD", scope: !1072, file: !1072, line: 33, type: !1619, scopeLine: 34, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !138, retainedNodes: !206)
!2174 = !DILocalVariable(name: "list", arg: 1, scope: !2173, file: !1072, line: 33, type: !22)
!2175 = !DILocation(line: 33, column: 53, scope: !2173)
!2176 = !DILocation(line: 35, column: 2, scope: !2173)
!2177 = !DILocation(line: 35, column: 2, scope: !2178)
!2178 = distinct !DILexicalBlock(scope: !2173, file: !1072, line: 35, column: 2)
!2179 = !DILocation(line: 35, column: 2, scope: !2180)
!2180 = distinct !DILexicalBlock(scope: !2178, file: !1072, line: 35, column: 2)
!2181 = !DILocation(line: 35, column: 2, scope: !2182)
!2182 = distinct !DILexicalBlock(scope: !2178, file: !1072, line: 35, column: 2)
!2183 = !DILocation(line: 36, column: 15, scope: !2173)
!2184 = !DILocation(line: 36, column: 2, scope: !2173)
!2185 = !DILocation(line: 36, column: 8, scope: !2173)
!2186 = !DILocation(line: 36, column: 13, scope: !2173)
!2187 = !DILocation(line: 37, column: 1, scope: !2173)
