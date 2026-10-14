# 2026-10-14 全量 E-only 结果基准

**此报告与逐例JSON是当前结果基准；此前七例pilot和旧全量结果作为历史保留。**

71例最终完整运行中，13明确对应 +17粗粒度对应 = **30/71（42.3%）**，41例漏报。全72例分母为30/72（41.7%）；CVE-2017-15265按原约定保留不一致输入、未运行。

用户约定以大致匹配真实缺陷判断召回，不要求根因、类型、全部操作与GT一致；粗粒度对应也计召回。仅同模块/字段却安全需求不同不计。报告、轨迹、源码、修复及Req/Gua/E逐例阅读对照；脚本只整理人工判断、核验引用和求和，不生成GT匹配/阶段标签。

召回与科学性分开：17例粗对应的操作集、同步域、绑定及E关系缺口在下文保留。847条选定最终E报告不是847个真实缺陷；工作流完成也不是Req抽取完备。特殊GT（有意竞态注释、普通NULL/API前提、死锁回滚等）保留原分母，不事后删除难例。

## 版本与配置

| 项目 | 冻结值 |
| --- | --- |
| 分支 | `codex/source-contract-artifacts` |
| 实际检测代码 | `18ab94cdac000190e30f935e7c347a533ccc6aaa` |
| 实验manifest HEAD | `4ab053aaece564ff4fb09a016bf9f35ca7dabf2b`（相对代码仅文档变化） |
| detector SHA256 | `5ef7883c13e02e0483b83b41fa72ca61ee22d788af98bf41e91df48616877c04` |
| 模型 / SDK | `gpt-5.5-2026-04-24`；openai `3.26.0`、openai-agents `0.23.1` |
| API / reasoning | Chat Completions / provider default |
| 模式 | E-only，Phase C关闭；两次源码抽取（requirement-sweep on） |
| 契约 / cap | artifacts文件接口 / max-turns=0（无硬性cap） |
| 并发 | 首轮7；4例补跑2 |
| 上下文 | 请求auto；网关本轮未提供可用Responses原生compaction，没有自制摘要丢弃历史 |

此次发布只整理文档、冻结输入计划和结果，没有继续改实现或启动新模型实验。原manifest仍保持原版本，不能将新文档提交冒充实验实现提交。代码版本发布前6 CTest、25 Python测试和localhost SDK验证已通过；工程验证不作为召回。

## 运行选择与统计

首轮北京时间2026-10-13 20:50:33—2026-10-14 00:17:30，71例结束：67完整、4网关429资源不足中断；中断例仍对已接受契约执行E，原exit2/partial保留。

4例在独立目录以相同代码、模型、输入和抽取参数补跑：10-14 14:53:52—15:38:28，全部exit0、两次抽取与E完成、implementation_unchanged。最终评估取67个首轮完整结果加4个补跑完整结果，不合并两次不同契约为一个假运行。

| 补跑Case | 首轮状态 / 报告数 | 补跑状态 / 报告数 | 最终召回 |
| --- | --- | --- | --- |
| CVE-2025-37882 | partial、exit2 / 1 | complete、exit0 / 10 | 漏报 |
| CVE-2025-37920 | partial、exit2 / 6 | complete、exit0 / 18 | 粗粒度对应 |
| CVE-2025-38078 | partial、exit2 / 4 | complete、exit0 / 7 | 粗粒度对应 |
| CVE-2025-38242 | partial、exit2 / 3 | complete、exit0 / 5 | 漏报 |

71份选定完整运行：4099模型请求、输入448,943,380 tokens、输出2,753,231 tokens；1070 Req、847最终E报告，单例平均17.29分钟。**这些usage不含4个失败首轮的完整usage，不能称实际全部调用/成本；输入累计含重复历史，不是独立代码量或已确认计费token。** 首轮墙钟约3小时27分，补跑约44分35秒；两次运行时间分开保留。

本轮是development cohort，不是独立测试集。旧三例/七例/全量采用不同实现、源码范围或流程；不可与旧严格匹配评分直接声称算法提升。

## 输入与复跑

冻结计划：[source_contract_full72_plan.json](../tests/source_contract_full72_plan.json)，SHA256 `79c5239a4af0ea53dc66539b577074b85c5d5e4798f67b412e308005ac8866d8`。

输入是完整C文件/头文件组成的受控功能单元，IR由相应选定翻译单元构建；不按入口/GT操作裁剪，不代表整个Linux/所有配置。71对源码/IR/配置hash已发布并与上一全量输入核验一致，逐例revision和hash见[结果JSON](results/source_contract_full72_20261014.json)。原始数据不进Git。

在既有实验环境中复跑（不要为了恢复上下文重复已完成实验）：

```sh
source /mlx_devbox/users/mayunlong.39/playground/LLM4Con/setup_env.sh >/dev/null 2>&1
export PATH="$PWD/external/joern-cli:$PWD/external/joern-cli/bin:$PATH"
python harness/run_batch.py \
  --plan tests/source_contract_full72_plan.json --split development \
  --dataset /path/to/lace-controlled-dataset72-20261012/dataset \
  --repository /path/to/pinned-linux \
  --output /path/to/new-run \
  --no-review --jobs 7 --max-turns 0 \
  --requirement-sweep on --contract-input artifacts
```

模型凭据/既有目的地由环境提供，不在文档/计划中保存。GT/修复不提供给检测Agent。Joern缓存/源码解析和编译配置需以各run/build manifest为准，不宣称每次重新解析所有源码或覆盖所有构建配置。

## 完整原始档案

首轮 `lace-source-contract-full72-20261013`，补跑 `lace-source-contract-full72-retry4-20261014`。远端入口在 `/mlx_devbox/users/mayunlong.39/playground/`，真实内容在独立NVMe `/tmp/`同名目录；不能只归档顶层symlink。本地完整副本在 `/Users/bytedance/Documents/并发缺陷检测/analysis_outputs/`下同名目录。

| 原始档案 | 文件 / 原始bytes | 压缩归档SHA256 |
| --- | --- | --- |
| 首轮 | 3800 / 4,124,847,400 | `fa8b36d37926fbc92e6e72c3fda5b7a478cc50fac8ace2dbd50c6ac2455a7edc` |
| 补跑 | 204 / 337,379,413 | `93a9fba995d08b6ab43876aa924d147c716bcb07b1d03acfcf8b546d48da0f77` |

两批完整原始文件均已下载、逐文件hash核验，0 mismatch。每例保留run.json、module_model、module_contracts、SDK/源码工具轨迹、workspace源码/契约及reports.json/bugs.txt。首轮目录的`manual_recall_review_20261014/final_recall_review.md/.json`还保留完整证据路径和4例first_round_review。Git内JSON提供portable run名、报告/Req ID、输入hash和人工结论，不替代这些原始档案。

## 逐例结论

报告/Req ID仅在该例选定artifact_run中解释。标为“补跑”的例使用独立补跑完整结果。详见[结构化逐例记录](results/source_contract_full72_20261014.json)。

| Case | 判定 | 最终报告 / Req | GT与对应判断 |
| --- | --- | --- |
| CVE-2013-1792 | 粗粒度对应 | finding:7 / req:15 | finding:7/req:15明确点出同一用户的uid_keyring/session_keyring部分发布、install与lookup并发；109/110两写及60/617/623相关读均在候选内。按资源、真实参与路径和部分发布安全需求宽松对应。其措辞混合单指针torn与跨字段partial publication，未独立表达session检查→install早退→session使用的依赖链，故计coarse。 |
| CVE-2015-7550 | 漏报 | — | 17条最终报告没有验证→获取读锁→payload使用的安全需求。closest finding:14只检查flags单次位读写完整性，既未依赖754成功观察，也未连接761/user_read。非最终req:6有payload read/revoke资源，但并未成为final candidate，且逻辑仍是payload访问互斥而非验证有效期。 |
| CVE-2016-7911 | 明确对应 | finding:3, finding:1 / req:3, req:1 | finding:3/req:3完整描述同task的check→dependent dereference与exit清slot/put干扰；finding:1/req:1另覆盖io_context对象寿命。资源、两个真实参与者和具体依赖窗口均对应GT，明确召回。 |
| CVE-2016-9806 | 粗粒度对应 | finding:2 / req:4 | finding:2/req:4描述同nlk dump generation、start安装与completion/continuation并发、saved skb/module状态和旧/部分callback状态，资源及真实参与函数可宽松对应。它的操作集合却没有2138/2139清理读/释放和2185/2187保存身份，实际诉求更偏callback gate，计coarse而非精确逻辑召回。 |
| CVE-2017-6346 | 漏报 | — | 17条final候选中最接近的finding:11/req:14是RX rollover使用→fanout_release的RCU reclamation，finding:15/req:18是RX间history/sock访问；都未涉及两个fanout_add的分配、覆盖和错误回滚身份。finding:1/req:2是fanout注册表/关联的访问需求，未覆盖rollover所有权事务。共享字段或模块相同不足计召回。 |
| CVE-2022-48830 | 粗粒度对应 | finding:9 / req:10 | finding:9/req:10明确同socket分段接收rx len/idx/sn/buf/state、FF与CF两个回调并发、不同PDU代际混合及复制部分buffer的安全需求。真实资源、参与者、接收事务一致性已对应，宽松召回；没有准确提出alloc(len)→skb_put(len)长度依赖窗口。 |
| CVE-2022-48931 | 粗粒度对应 | finding:4 / req:6 | finding:4/req:6将cg_children和ci_entry层级、注册/注销link/unlink纳入并发列表一致性安全需求，实际747/721操作就是root路径最终执行的mutation，因此宽松对应。其B仅lookup/path/dependency读者，未把两subsystem注册/注销writer-writer列成相对参与者，也未显式区分root，故coarse。 |
| CVE-2022-49215 | 漏报 | — | 只有3条final；closest finding:0/req:0针对bind初始化先于bound读者可见，A均为xsk_bind发布写，没有unbind state/dev清除或引用退役。它与GT共用xs字段和send/recv，但安全需求是初始化可见性，与已绑定代际被撤销后继续使用不同；不计宽松召回。 |
| CVE-2022-49607 | 漏报 | — | closest finding:1/req:1要求RCU输出/fault读者先于最终rb free；finding:8/req:11要求registry取得的event先于free。两者都是内存对象退役安全，未覆盖rb仍被引用但mmap_count=0时redirect/close的闭环进度需求。14个Req没有set_output、mmap_close的目标复合事务。 |
| CVE-2023-53046 | 明确对应 | finding:0 / req:1 | finding:0/req:1明确列producer queue、正常worker、unregister clear三方，并直接要求clear无锁遍历/删除/释放与添加互斥或入场有序。参与函数、共享list/entry及缺失锁安全需求均明确对应GT。 |
| CVE-2024-26974 | 明确对应 | finding:0 / req:0 | finding:0/req:0明确timeout=0仍free、worker随后读取/complete同reset_data，精确对应GT参与路径与具体生命周期需求。 |
| CVE-2024-26984 | 粗粒度对应 | finding:2 / req:5 | finding:2/req:5把同nv50 mapping的ptrs/maps与半发布安全需求明确纳入，并选250 ptrs写及253 maps发布；按资源与acquire发布路径宽松对应。另一参与者在Req中却是release/eviction/dtor，没有第二acquire快路225/226与ptrs使用；所以不能计完整逻辑召回。 |
| CVE-2024-27019 | 粗粒度对应 | finding:7 / req:8 | finding:7/req:8明确对象类型register/unregister与batch lookup、pin在遍历之后的窗口，安全需求和GT一致。原生E未绑定遍历宏，所以按描述与已检查调用边界宽松召回，不称完整绑定召回。此结论只使用本批新native报告，不使用此前manual probe。 |
| CVE-2024-27030 | 漏报 | — | finding:2/req:4和finding:3/req:5检查IRQ写workslot.num_msgs与workqueue读取/清零的丢计数/错误迭代窗口；不是PF/VF两个IRQ activation处理同一事件。finding:9/req:12为vfs初始化发布。虽然irq handler和字段接近，参与者及安全需求不对应GT，维持missed。 |
| CVE-2024-27404 | 漏报 | — | 6条final中没有remote_id访问需求；finding:1/req:3是icsk_ulp_data读者与整个subflow_context RCU free，finding:2/req:4是全局join_entries syncookie slot校验/提取，虽含连接身份信息仍为不同对象及事务。没有字段级安全需求或GT参与路径组合，missed。 |
| CVE-2024-35898 | 漏报 | — | 7条final均无nf_tables_flowtables/nf_flowtable_type需求。closest finding:6/req:7讲每个nft_flowtable的hook_list、netlink事务与NETDEV_UNREGISTER notifier；类型全局registry和每实例设备hook list为两个不同对象、不同actor与安全需求。其余table规则对象RCU生命周期亦不补全类型查找。 |
| CVE-2024-35977 | 明确对应 | finding:2 / req:2 | finding:2/req:2在首段完整追出open→ttyport_open→SERPORT_ACTIVE→dispatcher dereference，明确302 ops安装与其他初始化必须先于任何receive，并点266早open。报告描述、资源、参与路径和GT缺失ops场景对应，明确召回。 |
| CVE-2024-35986 | 粗粒度对应 | finding:8 / req:9 | 最终finding:8/req:9报告power_supply_get_property driver callback与同psy unregister/driver data的生命周期冲突，两个真实API路径及同外部provider资源对应，依用户宽松口径并经主agent复核计coarse。它实际描述的是check→callback内unregister、计数降到非正，未关联TUSB长期缓存及unregister后count仍正、稍后再次调用的GT真路径；逻辑不能称正确复现GT。 |
| CVE-2024-35999 | 粗粒度对应 | finding:8 / req:9 | finding:8/req:9明确同session channel table的add、pick、secondary-disable并发，要求防止选已移除channel/混乱引用；资源、picker路径和保护需求可按宽松口径对应。 |
| CVE-2024-36938 | 明确对应 | finding:10 / req:12 | finding:10/req:12明确saved_data_ready callback指针的start/stop写与ingress/data_ready读调用，描述call through NULL/wrong generation；对应GT对象、读路径和故障。 |
| CVE-2024-39503 | 粗粒度对应 | finding:0 / req:0 | finding:0/req:0覆盖同list_set/map与owning ip_set的GC异步使用对netns/destroy回收，要求callback不再使用/重排后才free。虽然落在timer callback而非其后续RCU cleanup callback，属于同资源、同GC/namespace retirement路径和UAF安全需求，按宽松口径计入。 |
| CVE-2024-39508 | 漏报 | — | 9条新最终候选均未涉及io_worker->flags、FREE/RUNNING bits；只有io-wq pending-work队列相邻。 |
| CVE-2024-40953 | 漏报 | — | 8条最终报告未出现last_boosted_vcpu或同spin函数访问竞争。 |
| CVE-2024-41081 | 明确对应 | finding:7 / req:8 | finding:7/req:8明确helper官方BH调用前提、同per-CPU slot重叠get/set、dst引用/cookie/source address损坏需求。与GT一致。 |
| CVE-2024-42234 | 粗粒度对应 | finding:1 / req:1 | finding:1/req:1明确同folio deferred-list membership、shrinker和undo/split路径的重叠列表修改会损坏链表，选中了GT undo/list_move操作，按宽松资源/参与路径/安全需求计入。 |
| CVE-2024-43830 | 明确对应 | finding:3 / req:3 | finding:3/req:3直接选pattern sysfs repeat/show/store读取trigger_data/锁data，与pattern_deactivate kfree，assumption明确device_remove_groups位于deactivate后；精准覆盖GT一种trigger实例。 |
| CVE-2024-43891 | 漏报 | — | 4个新最终候选没有format/f_start/FREED前提缺失。相邻file lifetime或trigger-start路径仍表达不同的失效关系。 |
| CVE-2024-45000 | 漏报 | — | 12条最终候选覆盖LRU链表、proc/hash、write completion、cache ops pointer；没有cookie n_accesses gate对LRU backend withdrawal。 |
| CVE-2024-46704 | 漏报 | — | 21条最终候选未选4187普通读或同flush barrier写，相关work->data候选是PENDING所有权竞争。 |
| CVE-2024-47715 | 漏报 | — | 15条最终候选没有NULL PHY或phy index valid性；cleanup相关候选均队列/列表并行维护。 |
| CVE-2024-50082 | 漏报 | — | finding16/req18是非常接近的同stack wait协议候选，但其明确安全需求只有got_token visibility/token泄漏或双计；没有回调尾访问必须早于waiter return/reclaim的生命周期关系，故初判missed。 |
| CVE-2024-53124 | 漏报 | — | 10条最终报告没有sk_forward_alloc、clone/charge或listener内存会计需求；只有相邻网络状态、队列、路由缓存。 |
| CVE-2024-53136 | 漏报 | — | 13条最终报告均数据访问/发布/回收或list竞争，无getattr inode-lock重入/等待环。 |
| CVE-2024-53160 | 漏报 | — | 10条最终候选没有monitor_work expiry或krcp锁域；timer->function、running_timer等相邻原语需求不同。 |
| CVE-2024-53186 | 粗粒度对应 | finding:4 / req:4 | finding:4/req:4明确同conn的request work全部使用先于handler terminate/disconnect/free，双方路径与UAF寿命需求相同，并选了279计数归零，宽松计召回。 |
| CVE-2024-56555 | 漏报 | — | 15条最终报告无proc->nodes迭代/union representation转换；freeze count、binder_ref树、allocator树均不同。 |
| CVE-2024-56788 | 明确对应 | finding:3 / req:4 | finding:3/req:4明确waiting_tx_skb生产/消费handoff及lost skb、duplicate ownership；1213 producer store对1007 consumer读和1008 clear，覆盖GT真实路径/资源/安全需求。无需要求Req类型与历史根因完全一致。finding:4/req:5是producer自并发empty-check，不能单靠它算GT召回。 |
| CVE-2024-58072 | 粗粒度对应 | finding:1 / req:1 | finding:1/req:1指出同global list的probe/add、disconnect/delete、buddy遍历不受共同保护，与GT中的真实list互斥/腐坏问题粗对应。未识别probe失败不摘链的主要寿命漏洞，且把未调用buddy hook当并发读者，故列coarse而非明确完整召回。 |
| CVE-2025-21732 | 漏报 | — | 最接近finding:9/req:11为硬件DMA停止/revoke必须先于umem pages释放；finding:5/req:5为ODP pagefault/prefetch使用MR先于kfree。两者都未包含invalidate_range对已撤销lkey再发UMR命令的安全需求，不能仅同MR/revoke/dereg判召回。 |
| CVE-2025-22050 | 粗粒度对应 | finding:9 / req:10 | finding:9/req:10同rxq SKB membership、state/URB ownership并明确completion defer_bh::__skb_unlink与producer queue。按宽松口径覆盖GT的同skb入链/完成消费安全问题；但没有建submit成功→必须入链或going_away准入的真正约束，操作集合还把producer/complete放同侧。 |
| CVE-2025-23142 | 漏报 | — | 最近finding:5/req:6是primary_path更新与getsockopt字段读，finding:1/req:2是transport timer lifetime；二者均不含sender保存指针、sndbuf等待放锁、send恢复使用。 |
| CVE-2025-23151 | 明确对应 | finding:20 / req:22 | finding:20/req:22直接把mhi_gen_tre1199 buf_ring->wp与1224 tre_ring->wp同mhi_deinit_chan_ctxt625 dma_free、627 vfree及629/631 clear联系，描述in-flight queue/event读取不得与channel free交叉/UAF，覆盖真实资源、producer/unprepare路径和寿命需求。 |
| CVE-2025-37772 | 漏报 | — | finding:2/req:2是work使用id_priv→kfree容器寿命；finding:17/req:19是id_table注册表mutate/traverse。两个都不是INIT_WORK覆写已排队work的安全问题。 |
| CVE-2025-37854 | 漏报 | — | finding:3/req:3仅CPU SRCU process对象使用→kfree(p)，finding:4/req:7为p->ef指针/eviction fence世代。不包含硬件用户队列→BO回收/reset完成的安全需求。 |
| CVE-2025-37882 | 漏报（补跑） | — | 完整补跑10报告均未提出旧xrun事件TRB地址在TD世代复用后误完成新URB的身份需求。最近finding:1/req:3覆盖同endpoint ring TD发布与完成/取消列表操作，但安全需求是列表完整性/未完整初始化的TD；finding:7/req:9覆盖TRB payload在cycle/doorbell之前发布。GT新TD已经完整发布，错误来自延迟旧xrun事件被按新TD解释；两者均不能按宽松口径算作该缺陷召回。 |
| CVE-2025-37920 | 粗粒度对应（补跑） | finding:11 / req:15 | 补跑finding:11/req:15指同共享xsk_buff_pool allocator free_list/free_heads列表/计数的allocate/free冲突。source __xsk_rcv242以xs->pool进行分配，xp_alloc546调用__xp_alloc，后者513取pool->fq、537推进FILLconsumer，同时529/550修改同pool缓存分配状态；两个共享UMEM socket可在各自rx_lock下进入这一同pool协议。按宽松口径，与GT generic RX共享pool分配安全需求粗对应。候选未直接提出cross-xs FILL queue consumer冲突，故不是精确召回。 |
| CVE-2025-38078 | 粗粒度对应（补跑） | finding:1 / req:2 | 完整补跑finding:1/req:2明确runtime->dma_area/dma_buffer_p在copy与snd_pcm_lib_free_pages之间必须保住同缓冲区寿命，并锚GT同一释放do_free_pages488。按宽松口径，与OSS silence1083访问同DMA缓冲区时可与释放相撞的安全需求粗对应。Req实际使用方是native default_write/default_read copy；未包含裸OSS snd_pcm_format_set_silence，因此并未精确定位绕过buffer_accessing的GT根因。finding:5/req:7另指runtime->oss.buffer转换缓冲区，和GT DMA缓冲区不同，不能因同OSS函数算作直接召回。 |
| CVE-2025-38104 | 漏报 | — | closest finding:0/req:1是PCIE indirect aperture命令互斥，finding:1为reset-domain硬件访问。RLCG事务Req:2虽抽取正确mailbox顺序，但没成为报告且其互斥需求不同于睡眠锁上下文错误。 |
| CVE-2025-38165 | 漏报 | — | finding:4/req:5是ingress_skb空检查→direct enqueue的FIFO排序事务；finding:2是psock backlog容器寿命。不是skb引用数与linearize可修改性前提。 |
| CVE-2025-38217 | 明确对应 | finding:1 / req:1 | finding:1/req:1精确说明427 check→430 BIT use与186/189 update插入，涉及同channel缓存与update_lock只覆盖写方，明确对应GT。 |
| CVE-2025-38242 | 漏报（补跑） | — | 完整补跑5报告均未提出move_pages_pte的swap entry/cache folio身份重验需求。最近finding:4/req:5包含目标1392 filemap_get_folio，但检查的是所有cache lookup/insertion必须先于swapoff exit_swap_address_space669/671回收整个数组；GT无需swapoff，活设备内同entry的cache folio身份/是否存在发生ABA，PTE值恢复不能保证lookup对象仍相同。finding:1/req:1是p->users保护整设备资源生命周期，也不同。req:2仅__swap_duplicate的swap_map计数check/update与put/free原子性，既无move操作也无最终报告；不能据共同swap_map/cache字眼算召回。 |
| CVE-2025-38250 | 漏报 | — | finding:4/req:6是suspend_work→kfree(data)，finding:13/req:20是debugfs private_data访问→free。两者同容器寿命但没有hdev lookup后的ioctl reset/flush参与路径；finding:1仅registry list结构锁，不能覆盖解锁后driver_data使用。 |
| CVE-2025-38337 | 漏报 | — | closest finding:14/req:17 active handle使用h_transaction→commit free transaction，但没有b_modified读写，也未指出NULL/aborted检查次序；不能以同dirty_metadata函数认召回。 |
| CVE-2025-38383 | 漏报 | — | finding:3/req:5是vm_struct pages/nr_pages初始化publication，虽含show_numa_info读者但不是counters数组RMW。其他registry/计数器候选也没有m->private counter。 |
| CVE-2025-38429 | 漏报 | — | 最接近finding:4/req:4和finding:6/req:6是ring cache/offset与reset的寿命冲突，未报告payload完成→host可见rp的发布顺序。 |
| SYZBOT-01affb1491750534 | 漏报 | — | req:21明确提出pwq->stats计数RMW一致性，并选到3244，但未进入最终19条候选。 |
| SYZBOT-1c486d0b62032c82 | 漏报 | — | finding:3/req:3要求check_unsafe_exec的users检查/mark对CLONE_FS为原子事务；这是另一个安全需求，不含失败清零与新exec的世代冲突。 |
| SYZBOT-2d373c9936c00d7e | 粗粒度对应 | finding:14 / req:15 | finding:14/req:15明确pool配置与poll/output availability读者一致性，包含真实poll所调用的snd_seq_output_ok读room与ioctl/init写room。 |
| SYZBOT-2e4de7fe846aba66 | 漏报 | — | finding:7/req:10是GRO offload callback注册表RCU寿命，其他候选为flow/XPS等；均不是同NAPI GRO执行并发。 |
| SYZBOT-35301db28c64310d | 明确对应 | finding:12 / req:15 | finding:12/req:15准确报告rx_dropped/tx_dropped/rx_errors非原子读写及丢增量。 |
| SYZBOT-3b6b32dc50537a49 | 漏报 | — | finding:3/req:6报告attach/detach/cleanup列表腐坏或漏epi；相同f_ep字段，但遗漏header45真实fast check且其安全需求不同。 |
| SYZBOT-3cc3a12efa69aa6f | 漏报 | — | 注册表RCU/链表候选及NAPI配置候选均不含reg_state同步域。 |
| SYZBOT-417aeb05fd190f3a | 漏报 | — | finding:6/req:6是保护下的pool列表/计数；finding:9/req:9是导出统计reader。均没有GT unmap5954决定恢复预约的读者。 |
| SYZBOT-44cf88a58d91b12b | 漏报 | — | finding:5/req:5是bind/connect更新对proc显示；未报告bind对connect的真实关系。未报告req:7是packet lookup对地址修改，也不是目标connect读。 |
| SYZBOT-4a03518df1e31b53 | 粗粒度对应 | finding:0 / req:1 | finding:0/req:1明确cache_es读取/替换一致性并含实际range-reader调用及GT改动的lookup934读，按宽松口径对应。 |
| SYZBOT-4a06d4373fd52f0b | 明确对应 | finding:15 / req:17 | finding:15/req:17直接指出remove写wd与event-path113读未被idr_lock覆盖以及失效世代事件。 |
| SYZBOT-52e3dbded1f71729 | 明确对应 | finding:7 / req:7 | finding:7/req:7明确无锁destructor读与RX reservation写rx_psock的冲突。 |
| SYZBOT-5676077ba016d741 | 漏报 | — | 最终14条Req/12报告是RCU表、neighbor、GRE/xfrm生命周期等，不含needed_headroom。 |
| SYZBOT-5a486fef3de40e0d | 漏报 | — | fixed-file查找/取得node引用的req:2及其他CQ/cache候选均不是trace表大小访问。 |
| SYZBOT-5cce5938c6c2c518 | 漏报 | — | 5个最终候选为CHTLS列表/round_robin、UDP hash等，未包含inet_id。 |
| SYZBOT-63cbe31877bb80ef | 漏报 | — | req:23针对check flags但所有操作未知/E=0；finding:24/req:27仅access-control owner/locked，并未构建两个锁域位域共享字节关系。 |
| CVE-2017-15265 | 未完成：输入不一致 | — | snd_core.h与pinned版本无canonical字节匹配，原输入保留，未运行。 |

## 逐例科学性与漏报依据

### CVE-2013-1792：粗粒度对应

同一user_struct的默认uid/session keyring由install_user_keyrings先后写入。另一lookup_user_key看到session为空并调用install；若uid已发布，install的锁前单字段检查会过早返回，随后session仍为空，key->usage解引用失败。修复把uid与session均存在作为锁前返回条件。

Req是MustBeMediated，A=38113/38117，B=38033/39038/39052/39065/39079；全部绑定，E undischarged、实际检查10组，无facts或成功使用的Gua。源码创建mutex仅覆盖发布写而锁外读仍可见部分pair。E是逐对无法消解；没有证明两个字段同代际、更没有构造真实NULL见证。

原始索引：`first_batch/native/CVE-2013-1792/artifacts/CVE-2013-1792_2026-10-13_20-50-33/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2015-7550：漏报

keyctl_read_key在754先key_validate，再于760获取key->sem后调用type->read。key_revoke可在这段窗口取得写锁、设置REVOKED并令payload=NULL；读回调在锁内仍以旧验证结果进入user_read，193解引用NULL payload。补丁把验证与回调一并移入读锁。

req:6的4个A rcu_assign_keypointer调用全部失绑，A=[]、B=59314/59318/46353，E=0、e_candidate=false。gua:6确实仅保护760→762读回调与写回调，不能延伸至754验证。finding:14/req:16 E=4，A=27174/28371、B=39102/36675；没有代表GT依赖序列。修复payload宏绑定仍不会自动补出validation→use事务。 本批SDK14及52实际展示754验证→760加锁→761回调完整源码；目标窗口源码存在并曾展示，不能归因为没读到该代码。

漏报依据：有证据的缺口是目标validation/use窗口未进入任何Req；接近的payload Req又因A全失绑未进入最终报告。不能把这例归为Gua错误消解，因为原生req:6零组合、没有discharge。

原始索引：`first_batch/native/CVE-2015-7550/artifacts/CVE-2015-7550_2026-10-13_20-50-33/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2016-7911：明确对应

get_task_ioprio在153测试p->io_context后154再次读slot并解引用ioprio，未持task_lock。exit_io_context在同task锁内清slot并丢active ref，可使后续NULL/退役对象解引用。修复为读端增加task_lock/task_unlock。

req:3 A=5587/5594（153、154），I=3545/3555（206、210），全部绑定；MustBeAtomic实际检查2个干扰组合，undischarged，无gaps/facts/使用Gua。req:1另6组检查，但有外部入口前提缺口。原生候选已明确失效窗口；E本身仍不是可行性证明。

原始索引：`first_batch/native/CVE-2016-7911/artifacts/CVE-2016-7911_2026-10-13_20-50-33/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2016-9806：粗粒度对应

netlink_dump完成时先将cb_running=false、2137解锁，再从可被复用的nlk->cb读取module与skb并put/consume。另一__netlink_dump_start可在锁后清/重填同cb，使旧完成者错误释放新dump的module/skb。修复在解锁前把旧module、skb快照到局部。

A=25198/22522/25163；B=22319/22378/22442/22087；2179 memset选择器失绑。E requires_semantic_review但真实执行12个已绑定组合，无facts或Gua消解。gua:3的锁域只到2137/2191，正确覆盖安装与callback执行却不能保护2138/2139；该Gua因source operand不绑定未eligible。因此此final candidate不能作为GT错误共享skb释放已经被E检查的证据。

原始索引：`first_batch/native/CVE-2016-9806/artifacts/CVE-2016-9806_2026-10-13_20-50-33/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2017-6346：漏报

两个fanout_add针对同packet_sock可同时通过1647的po->fanout检查，在fanout_mutex之前1652各自写共享po->rollover，随后失败路径在1707解锁之后1709释放当前共享slot并置NULL，能破坏另一个加入者的rollover所有权。修复把检查/分配/错误回滚纳入fanout_mutex，并使用局部rollover待成功再发布。

closest req:14 A=36266/35991/36044、B=15284，E=3，缺1332读绑定；gua:14谈实际RCU grace-period，不处理add错误分支直接kfree。最终report的3组关系没有任何1652/1709操作。SDK第8条tool_result确实给模型展示过完整1647→1710窗口；缺陷不能简单解释为代码不可用或没读到。

漏报依据：已展示的fanout_add构造/错误回滚没有转为同slot所有权的Req；当前Req集中于收包和正常release。自动E只能检查这些已有操作，不能推导遗漏的两次加入事务。

原始索引：`first_batch/native/CVE-2017-6346/artifacts/CVE-2017-6346_2026-10-13_20-50-33/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2022-48830：粗粒度对应

isotp_rcv_cf在566以so->rx.len分配skb，570/571再次以共享len扩容/复制；同socket并发isotp_rcv_ff在457等更新rx.len，可使第二长度大于原分配，导致skb越界。修复对整段isotp_rcv加rx_lock序列化FF/CF。

req:10 A=13450/13612/13679，B=14763/14778/13493/13588；548 buffer复合write失绑。E实际12组、requires_semantic_review，无facts/使用Gua。所选操作是state/sn/buf；GT len写、alloc→skb_put重复读取并未纳入E，所以不能声称该E证明长度不一致或越界见证。

原始索引：`first_batch/native/CVE-2022-48830/artifacts/CVE-2022-48830_2026-10-13_20-50-33/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2022-48931：粗粒度对应

configfs_register_subsystem及注销/注册失败路径向全局根group的cg_children执行link_group/unlink_group。不同subsystem的su_mutex不能序列化同一root list；1862、1887、1934调用时没有共同root保护。补丁增加全局configfs_subsystem_mutex包围这些root mutation。

A=10846/9775/9778/10833，B=17580/13592/19683；180 traversal宏读失绑；E真实12组未消解。gua:6 eligible但未使用，只列1317、1515、1730、1789的普通su_mutex regions，既不包含1862/1887/1934，也不建立不同subsystem锁同实例。E没有比较root两writer的实际组合；读写候选不能当完整GT时序。

原始索引：`first_batch/native/CVE-2022-48931/artifacts/CVE-2022-48931_2026-10-13_20-50-33/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2022-49215：漏报

xsk_sendmsg/recvmsg/poll在xsk_is_bound成功后继续使用xs->dev/pool；xsk_unbind_dev在727改状态后731将dev=NULL，732 synchronize_net又没有包住整个上述syscall的读区。并发unregister可使此前通过bound检查的路径NULL解引用/生命周期失效。补丁删除NULL写，对整个send/recv/poll加RCU并在generic_xmit锁内重查bound。

closest req:0 A=13458/13468/13473/13498，B=15213/15225/14618/14740，全bound E=16/undischarged；gua:0源码确有bind wmb/reader rmb，但其source evidence失绑，不能据candidate断言bind本身有错。9个Req没有复合bound→use受unbind干扰的事务/retirement Req。SDK8及15有完整unbind窗口，包含731 NULL和732 synchronize_net，说明源码曾展示。

漏报依据：原生Req在同对象上提取了首次bind-publication协议，却没有提取unbind撤销协议；E检查16个初始发布组合不能补出已通过bound的在途读者。更深层为何遗漏，现有材料不能确定。

原始索引：`first_batch/native/CVE-2022-49215/artifacts/CVE-2022-49215_2026-10-13_21-00-06/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2022-49607：漏报

perf_mmap_close(e2)令rb->mmap_count从1到0并扫描rb->event_list拆全部redirect；另一perf_event_set_output(e1,e2)只持e1锁，能在扫描未看见e1后11886把e1挂到已unmapped rb。e1后续perf_mmap在6254非零增长失败后无限again。补丁同时持e1/e2锁、检查rb mmap_count非零，并在retry主动detach。

closest req:1仅A=65116（mmap_fault）、B=89710/89732（page/rb free），E=2；3个__perf_output_begin源操作因函数selector与lowered函数不匹配失绑。gua:1是RCU final free的真实retirement边界，与mmap_count逻辑脱离无关。SDK12展示6254检测，SDK14展示close扫描；不据此推断模型理解了两锁不同实例。

漏报依据：缺失的是mmap_count非零观察/redirect attach与close detach-scan之间的逻辑事务及不同event锁实例关系；已有生命周期Req无法覆盖GT。

原始索引：`first_batch/native/CVE-2022-49607/artifacts/CVE-2022-49607_2026-10-13_21-00-18/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2023-53046：明确对应

hci_cmd_sync_queue可在HCI_UNREGISTER检查之后与clear重叠；clear虽cancel_work_sync但没有cmd_sync_work_lock，646/650/651对同list遍历删除释放与701并发添加/再次queue形成list corruption/UAF。补丁在clear遍历/释放区域增加该mutex。

A list_add=86068；B worker list_del=93491、clear list_del=96159、clear kfree=96161；E实际3组、requires_semantic_review。288/646 traversal helpers失绑，但target clear delete/free已经检查。gua:1不包含clear region（条件也明确排除），又因source operand未bound不eligible。cancel不会等价于禁止之后的新queue。该候选有正常worker可能受锁保护的过宽组合，但其中clear两组是GT核心。

原始索引：`first_batch/native/CVE-2023-53046/artifacts/CVE-2023-53046_2026-10-13_21-00-58/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-26974：明确对应

同步AER reset调用者在wait_for_completion_timeout返回0时仍141释放reset_data；异步reset worker可能尚未到达104 mode读取/105 embedded complete，导致UAF。补丁区分timeout所有权，让worker在超时后的同步reset完成时释放，成功wait路径由caller释放。

A=5946/6056/6061、B=5614，全部bound；E undischarged，实际3个use-before-free组合，无gaps/facts。gua:0仅成功wait非零条件，conditional unresolved，不用于discharge；源条件明确零timeout不完成，因此候选没有把有时限wait误当全路径完成。

原始索引：`first_batch/native/CVE-2024-26974/artifacts/CVE-2024-26974_2026-10-13_21-04-41/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-26984：粗粒度对应

两个nv50_instobj_acquire并发：A写memory.ptrs后253 refcount_set(maps,1)不提供发布次序；B在225 refcount_inc_not_zero成功走无锁快路径，可能尚未看到ptrs写入，随后memory accessor NULL解引用。补丁在refcount_set前wmb，在快路成功后rmb。

A=17585/17605/17036；253 refcount_set未bound（只有debug坐标候选17044等，没有匹配操作）；B=17516/17523/17530/17146/17152/16676。E真实18组、requires_semantic_review，无facts/使用Gua。gua:4覆盖锁内kmap/LRU/slow acquire，但既没有保护225快路，也因source operand失绑未eligible。这18组是发布/退役混合检查，未检查GT两个acquire可见性配对。

原始索引：`first_batch/native/CVE-2024-26984/artifacts/CVE-2024-26984_2026-10-13_21-04-43/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-27019：粗粒度对应

nft_unregister_obj在6908 list_del_rcu移除全局类型节点；__nft_obj_type_get在7048普通list迭代、7053字段读取，nft_obj_type_get直到7065才try_module_get且无整个查询的RCU保护。并发module exit可破坏遍历/类型寿命。补丁使用list_for_each_entry_rcu及覆盖查询、pin的rcu_read_lock/unlock。

A=44904/44886（两个实际list call），B=70194 try_module_get、64912 __nft_obj_type_get调用；7048 list_for_each_entry call selector失绑。E requires_semantic_review仍真实4组无facts；gua:9 eligible但仅覆盖写侧mutations，不覆盖lookup，未用于消解。GT字段/链表读不在这4组内，调用关系未证明同entry/真实feasible schedule。

原始索引：`first_batch/native/CVE-2024-27019/artifacts/CVE-2024-27019_2026-10-13_21-06-41/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-27030：漏报

PF→AF向量与VF→AF向量在2884、2952、2967注册同rvu_mbox_intr_handler；两个CPU处理各向量时都读取/清理PF和VF mailbox事件，可能同时服务同一event并破坏消息。修复拆PF专用handler，保留VF handler负责VF寄存器。

req:4 A=62465/B=46768（IRQ store↔worker reset）；req:5 A=62515/B=46523；各E=1、requires_semantic_review，worker读2144/2234失绑。检查没有同IRQ actor的A↔A或PF/VF向量admission关系，不能拿两个E1算重复event需求。

漏报依据：已提取需求集中在IRQ/worker数据交接，没有同handler的多向量并发event claim/处理事务。没有证据认定外部中断绑定不存在；源码明确三处注册，缺口在候选所表达的协议。

原始索引：`first_batch/native/CVE-2024-27030/artifacts/CVE-2024-27030_2026-10-13_21-06-58/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-27404：漏报

subflow->remote_id由subflow_finish_connect、subflow_connect以及request处理路径普通写，PM远端地址选择446及remove805等无配对访问完整性读。GT为remote_id跨协议路径数据竞争，补丁以WRITE_ONCE/READ_ONCE覆盖这些共享字段访问。

req:3 E=10，undischarged；req:4 E=7，undischarged。这些非零检查并非GT字段读写；所有9个Req的资源与操作均无subflow->remote_id目标。SDK14展示PM446读（SDK6也出现request struct的remote_id声明，不能把同名字段当相同对象）。不声称因没有源码而漏报。

漏报依据：最终和非最终Req均没有目标subflow_context.remote_id跨路径访问完整性需求；不能把context整体生命期或不同syncookie slot算作字段竞争召回。

原始索引：`first_batch/native/CVE-2024-27404/artifacts/CVE-2024-27404_2026-10-13_21-11-47/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-35898：漏报

nft_unregister_flowtable_type在8156从nf_tables_flowtables删除类型；__nft_flowtable_type_get在8303普通迭代同全局registry，8316在lookup之后try_module_get，整个查询未有RCU read区。module exit可与newflowtable查类型并发。补丁RCU迭代并以read lock覆盖type查询/pin。

req:7 A=73070/71667/50297、B=48760，E实际3组，9039 hook_list遍历失绑；gua:7是commit_mutex包围netlink及NETDEV_UNREGISTER的hook-list事务，与类型registry无关。8个Req都没8303/8156/8316；SDK79完整展示了GT8303及8316查询，说明源码可用且实际展示过，精确内部遗漏原因仍未知。

漏报依据：模型提交的Req没有全局flowtable type registry查询/退出协议；当前E检查的是不同flowtable实例hook-list。没有证据把漏报归为原生目标E的绑定或Gua消解，因为目标Req根本未形成。

原始索引：`first_batch/native/CVE-2024-35898/artifacts/CVE-2024-35898_2026-10-13_21-13-04/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-35977：明确对应

cros_ec_uart_probe在266 devm_serdev_device_open激活TTY接收，却直到302才安装serdev->ops。serdev_controller_receive_buf在active后先解引用ops->receive_buf，可在ops=NULL窗口崩溃。此前driver data/wait queue/priv也晚于open。补丁把全部初始化和set_client_ops移到open之前。

A=6778/6783/6838/6858（drvdata、waitqueue、priv、ops），B=7433/7439/7475（callback上下文用），E实际12组，90 response选择器失绑。external dispatcher的ops读没有在此模块E中，不能把12组叫NULL-ops见证；但Req正文有已核验repository链，并且本模块源码与GT都直接支持open/ops实际次序。因此召回明确，E科学性仍有限。

原始索引：`first_batch/native/CVE-2024-35977/artifacts/CVE-2024-35977_2026-10-13_21-15-12/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-35986：粗粒度对应

TUSB把power_supply_get_by_name返回的外部charger长期缓存于tusb->charger，只在自身remove put。外部充电器driver仍可power_supply_unregister，use_cnt持有额外ref时WARN后仍继续device_unregister/driver teardown；设备壳引用不保证外部driver backing data寿命。之后tusb1210_get_online再get_property可调用失效driver data。补丁改为每次get_online临时取得charger、调用后立即put，历史修复说明保留理论短窗口。

req:9 A=12472 atomic_read/12492 get_property，I=11061 dec_return/11126 device_unregister，全部bound；E requires_semantic_review、真实2干扰组合，无facts。gua:5 AtomicOp eligible但不能覆盖整个check/use，未用于discharge。非final req:4（tusb缓存NULL→get/store→use和自身remove put）虽全部bound，却因10523→10536、10536→10544序列顺序未建立，E=0/e_candidate=false；这既非GT provider退出干扰，也不影响已有req:9宽松计入。

原始索引：`first_batch/native/CVE-2024-35986/artifacts/CVE-2024-35986_2026-10-13_21-15-16/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-35999：粗粒度对应

同一session的channel picker在释放ses->chan_lock后从ses->chans[index].server取返回值，期间channel增删/disable可能改变表或slot。历史修复将这一取值移到锁内，目标transport.c:1060 unlock后1062 read仍在。

科学性缺口：req:9的pick B只选1031/1032锁内读，没选GT1062锁外读；A包含channel add，与B disable/pick作A×B，缺部分真实read/remove组合。已绑定A=[103199,103229] B=[123118,123127,104779,104820]，E=8，requires_semantic_review，facts=[]。gua:8明确只保护列举锁区且不证明unlock后server lifetime，但其证据/selector问题使未eligible；不能以最终候选证明锁内读本身有GT故障。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-35999/artifacts/CVE-2024-35999_2026-10-13_21-20-25/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-36938：明确对应

sk_psock_skb_ingress_enqueue调用sk_psock_data_ready无sk_callback_lock，与drop/stop_verdict把saved_data_ready置NULL竞争，helper先检查再第二次load/call可能NULL。修复将read_lock_bh统一移进helper，目标header459/460和writer1248仍旧。

Req:12的7个操作均bound，stop_verdict写8213、helper读11066/11071；E=12，undischarged，facts=[]，no_synchronization_chain。gua:9只partial verdict_data_ready read锁，conditions明确ingress_enqueue不覆盖、writer锁来自caller；未采用。召回不代表可行时序已经形式化证明，source helper/caller差异支持其科学依据。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-36938/artifacts/CVE-2024-36938_2026-10-13_21-26-45/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-39503：粗粒度对应

list:set GC移除元素可排队__list_set_del_rcu，回调通过e->set访问set扩展；namespace/destroy在cancel GC后直接free set，缺cancel/flush以后rcu_barrier等待。单set还把barrier放cancel前。补丁先取消/flush所有GC，等回调，再free sets，并修userspace test RCU。

Req0全部5 source operations bound，E=6，undischarged，facts=[]，无使用Gua。gua:0把del_timer_sync自重排no-rearm写作未确定前提；这并不直接识别GT cancel后剩余RCU cleanup stage。Req没有__list_set_del_rcu的e->set使用与set free的对应operand，因此coarse不能称准确根因发现。 输入cancel_gc仅del_timer_sync、未含GT所注明依赖c1193d9的list-flush动作；不过GC set_cleanup_entries→list_set_del→call_rcu本身可先排队e cleanup，且namespace随后free set，目标生命周期缺口仍存在。不能把修复后cancel/flush步骤反写成当前源码行为。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-39503/artifacts/CVE-2024-39503_2026-10-13_21-26-50/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-39508：漏报

io_worker->flags plain bitmask由worker busy/idle/running/sleeping改，free-worker activation等读取。历史补丁unsigned改unsigned long并把&/|改为test_bit/set_bit/clear_bit；目标__io_worker_busy434仍plain &=。

finding:2的E=6，requires_semantic_review，部分list selectors unresolved，但没有任何worker->flags Req可供E检查。不能把队列保障绑定失败当GT未召回原因。

漏报依据：原始module_contracts的9条Req没有worker flags；SDK结果6/14显示busy434及多条flags读写，46/52仍读flags。可确认读到但未形成Req；无记录证明其为什么排除，内部理由未知。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-39508/artifacts/CVE-2024-39508_2026-10-13_21-27-14/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-40953：漏报

同VM多个kvm_vcpu_on_spin实例plain读写last_boosted_vcpu；compiler tearing可能产生越界index。补丁给两处访问加READ/WRITE_ONCE，目标2589/2623仍plain。

相邻req:10确实E=4并出候选，但所检查是slot/count publication与reader；目标读39630/写39819不属于本次任何Req。无目标Gua消解证据。

漏报依据：实际Req中无该字段。可见SDK只读2598-2610 spin循环片段（call79/result80）及2630以后区域（call18/result19），未在SDK结果出现2589 load和2623 store；有限源码访问是可证缺口，不能断言模型明确误判它安全。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-40953/artifacts/CVE-2024-40953_2026-10-13_21-27-37/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-41081：明确对应

process context ila_output可能被同CPU softirq重入，dst_cache_get/set访问同per-CPU slot需local BH disabled；RCU不提供此排他。补丁在get61与set90包BH禁止，目标均无屏蔽。

4624 get、4697 set均bound，Req8 E=1，undischarged，facts=[]，两向no_synchronization_chain；无目标Gua。报告与同CPU BH语义的相同slot绑定仍由source/repository语义承担，E本身未证明per-CPU实例与可行重入。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-41081/artifacts/CVE-2024-41081_2026-10-13_21-28-17/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-42234：粗粒度对应

迁移mem_cgroup_migrate在refcount未冻结时undo同folio->_deferred_list；shrinker已将folio搬到私有list在split_queue_lock外处理，锁无法保护私有list节点。修复把undo放folio_migrate_mapping引用冻结0窗口。

全部9操作绑定，E=14（2 producer A × 7 consumer B），undischarged。gua:1 eligible=true未used，continuous_exclusion fail。真实GT是consumer undo/migration与consumer private scan的B×B；当前Req不包含3306-3314 private-list使用，也没把mem_cgroup_migrate作为actor。最终邻近候选存在可粗对应，却不是该GT时序已被E构造；不能把后端对普通锁区的保守失败当精确发现。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-42234/artifacts/CVE-2024-42234_2026-10-13_21-37-55/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-43830：明确对应

led_trigger_set先deactivate释放trigger_data，后device_remove_groups撤销sysfs show/store准入，窗口内sysfs使用已释放data。修复对调顺序；目标183调用deactivate，184才remove_groups。

5 reader/lock operands与free7943全部bound，E=5，requires_semantic_review因external sysfs排空条件未证，facts=[]，no_synchronization_chain；但最终已成finding3。source与assumption写清真实窗口，未复现仍计明确召回。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-43830/artifacts/CVE-2024-43830_2026-10-13_21-39-19/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-43891：漏报

format file i_private直接持trace_event_call而不经过trace_event_file的FREED校验；动态event销毁后旧fd继续访问已free call/fields。patch将format通过file->event_call取得call，并在持event_mutex时统一event_file_file校验；其他trigger/hist/inject入口也补检查。

req5 E=3，对三处writer至bound trigger_start读，hist selector未resolved；req6 E=2，对tracepoint file reads至file slab free。两者均requires_semantic_review并成报告，但无format call或存活校验对应组合。未有Gua错误消解目标。

漏报依据：12条raw Req无format、FREED首次验证需求；SDK source-main切片主要跳过1539-1645，getter搜索能找到名字却未建立format持有的实际call对象/存活flag关系。缺少正确目标Req可证；其内部误判理由未见。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-43891/artifacts/CVE-2024-43891_2026-10-13_21-39-40/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-45000：漏报

ACTIVE DO_LRU_DISCARD分支直接转LRU_DISCARDING并withdraw cookie->cache_priv，绕过后续n_accesses!=0检查；仍在cache I/O的用户访问NULL backend对象。patch在743 LRU分支补计数检查。

req0 E=2只commit_link mutation组合；req10 E=3只cache->ops use与cache relinquish写。都无743/744→776与cookie访问结束关系，不能把“NULL backend”泛化为主匹配。

漏报依据：13条Req没有cookie n_accesses或DO_LRU_DISCARD gate。SDK第6和92实际读到了743/755这两不同分支；已读却未建相关需求，进一步理由未知。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-45000/artifacts/CVE-2024-45000_2026-10-13_21-40-51/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-46704：漏报

from_cancel=false时__flush_work仍plain读work->data，但短路以后完全不用值；与另一flush/barrier写产生spurious KCSAN报告。patch仅把读移进from_cancel true，无功能变化。

req7绑定queue/PENDING等5操作，E=6，undischarged；普通21389 load不在该Req。没有目标Gua消解或目标E检查。

漏报依据：Req无from_cancel/unused read；未形成该sanitizer竞争的需求。GT自称spurious且无functional change，所以也不能凭真实安全需求候选相同work字段强算召回。

范围备注：官方修复明示spurious KCSAN、unused value、no functional changes；保留原分母，单列benign/annotation类型。

原始索引：`first_batch/native/CVE-2024-46704/artifacts/CVE-2024-46704_2026-10-13_21-43-39/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-47715：漏报

MT7986 non-DBDC单ADIE主PHY band index可为1，dev->phys[1]为NULL，cleanup直接索引后phy->tx_lock崩溃。patch改用mt76_dev_phy fallback；不是需要并发的故障，即使由上一竞态修复引入。

req2 E=2与req17 E=3的list/helper节点被检查，但真实1527数组load28633不在对应Req。锁token为phy不自动证明phy有效；未错误消解GT。

漏报依据：未形成PHY指针有效性候选；普通配置回归属于scope特殊。供源码包含common错误cleanup，但不包含MT7915/MT7986 band配置完整触发实现，不能称目标硬件路径已全面建模。

范围备注：non-concurrency配置NULL解引用回归；保留分母。

原始索引：`first_batch/native/CVE-2024-47715/artifacts/CVE-2024-47715_2026-10-13_21-45-00/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-50082：漏报

rq_qos_wake_function先list_del_init后仍读data->task/wake，waiter见got_token并finish_wait看到empty可提前return，栈data被回收，尾wake使用垃圾task。patch先wake再list_del_init_careful，与finish_wait的list_empty_careful配对。

Req18全部6 operations bound，A47433/47441/47445 B47287/47313/47319，E=9，undischarged，facts=[]、18个no_synchronization_chain。即使E保留全部9组合，它检查的是mediation between token flag/callback and token reads，不是最后访存到栈退出的Order。Req与完整描述已保存供主agent裁决。

漏报依据：原始Req18/最终首段明确token计费故障。未发现另一个wait-stack lifetime Req；不是因binding丢失而漏GT。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-50082/artifacts/CVE-2024-50082_2026-10-13_21-50-45/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-53124：漏报

TCP/DCCPv6监听socket可无锁并发收包，在LISTEN判断前clone_and_charge_r修改sk_forward_alloc charge/reclaim，丢失计费更新。patch禁止listener clone并删除TCP多余free；目标1622/622仍无state gate。

邻近req4 E=6/req9 E=6有实际绑定，但都没有clone81307/49246节点或其内存会计副作用。无目标E组合，不能按listener概念吻合算召回。

漏报依据：raw10 Req无clone/forward_alloc。SDK第53/57读两target clone，但没有相应外部helper内存会计需求；可证callee副作用未进入契约，误判原因未知。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-53124/artifacts/CVE-2024-53124_2026-10-13_21-53-51/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-53136：漏报

回滚shmem_getattr包generic_fillattr的inode_lock_shared，修复tmpfs经NFS访问死锁；目标1017 lock与1019 unlock仍在。

req10 E=64与req12 E=1不检查1017 acquire及caller持锁依赖。真实shared-lock call22378存在但无该死锁Req；外部NFS/VFS caller cycle未重建，不猜为具体算法错误。

漏报依据：没有非重入/锁依赖候选；GT是死锁回滚并非新data race修复。外部NFS context不在本module，范围问题单列。

范围备注：revert/deadlock，保留分母；官方称被回滚修复原本只是sanitizer silencing。

原始索引：`first_batch/native/CVE-2024-53136/artifacts/CVE-2024-53136_2026-10-13_21-55-08/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-53160：漏报

schedule_delayed_monitor_work普通读krcp->monitor_work.timer.expires，与monitor排队经__mod_timer1173写同字段竞争。pending不是字段锁。patch给未持锁schedule caller加krcp锁，已持锁kvfree改调内部函数。

Req3 E=8/Req4 E=3只查其他timer字段。真实83979 expiry load/128889 store未被target Req组合；当前报告不能泛称timer竞争即匹配。

漏报依据：raw13 Req无monitor/expiry需求；SDK所有结果未命中schedule_delayed_monitor_work或monitor_work.timer.expires，tree直接源码读取跳过3513-3850（主要2460-3135与4450以后）。可证相关caller段未读的覆盖缺口，非仅没bind现成目标Req。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-53160/artifacts/CVE-2024-53160_2026-10-13_22-01-39/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-53186：粗粒度对应

worker将conn->r_count减0后还waitqueue_active/wake_up，connection handler见0可disconnect/free conn，尾访问UAF；patchworker增refcnt跨归零/唤醒尾部再减ref最终free。GT文字称worker原先直接conn_free不准确，target实际由handler disconnect释放。

A269/271/279绑定19970/19972/20037；B415间接disconnect已bound18905，仅413 terminate_fn read未resolved。E=3，requires_semantic_review，facts=[]，no_synchronization_chain；gua4因wake/wait源绑定不齐eligible=false/used=false。科学性缺口：Req没有279的waitqueue_active20047及280 wake20052，而Gua仍误把count归零表述为work已完成。候选粗对应UAF协议，但E未真正检查归零后的last-use窗口。 atomic_dec_return绑定20037的实际IR是含lock;xaddl的内联汇编call（debug位于inlined header85，source chain回279），而非LLVM atomicrmw；原始source-to-IR恢复不证明Wait endpoint已正确。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-53186/artifacts/CVE-2024-53186_2026-10-13_22-01-54/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-56555：漏报

freezer迭代proc->nodes时为锁序释放inner lock，deferred_release可erase并将union rb_node改作dead hlist；重获inner锁仍rb_next旧cursor导致OOB。tmp_ref仅保活分配。patch在5595重获锁后is_dead检查break。

Req8 E=12、Req3 E=16均没有rb_next38485与release erase29086相关关系。不存在目标Req/Gua被错误消解；大量普通锁域候选不会自动覆盖union成员身份稳定性。

漏报依据：SDK第42/60读binder_add_freeze_work段，第18读release erase，第16读dead_node union转换；但16条Req未提出该iterator validity需求。可证读取后遗漏，未见可信记录说明它主动以tmp_ref判安全，故该内部原因未知。

范围备注：普通并发安全缺陷；保留分母。

原始索引：`first_batch/native/CVE-2024-56555/artifacts/CVE-2024-56555_2026-10-13_22-02-12/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-56788：明确对应

网络TX producer写waiting_tx_skb与SPI consumer读waiting→赋ongoing→清waiting交叉，清零可抹掉新skb，造成丢包/泄漏；修复为这两侧增加tx_skb_lock。

req:4全部4操作绑定7143/4350/4356/4308，E undischarged，检查3对组合。gua:3未通过全部绑定，未消解。此报告识别了真实shared slot冲突，但没有完整写出consumer读→清零的MustBeAtomic，因此召回不等于事务表达完备。

原始索引：`first_batch/native/CVE-2024-56788/artifacts/CVE-2024-56788_2026-10-13_22-03-44/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2024-58072：粗粒度对应

未受glb_list_lock保护的global glb_priv_list在probe添加、disconnect删除；probe失败还残留已释放节点，后续probe可能访问/损坏链表。GT移除global list及未使用buddy hook，修复说明同时明确never-used lock与stale failure节点。

E requires_semantic_review，306 list_empty/307遍历、2014 add、2319 delete绑定，309 iter->priv未绑定；实际4组合检查。未有目标Gua。GT的buddy hook未使用必须保留可达性缺口，不能把该候选的reader并发当成已证明事实；writer列表无锁是历史修复明确提及的真问题。

原始索引：`first_batch/native/CVE-2024-58072/artifacts/CVE-2024-58072_2026-10-13_22-09-09/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-21732：漏报

dereg/revoke先让硬件lkey失效，另一个mlx5_ib_invalidate_range持umem_mutex仍取umem_odp->private并调用UMR update失效lkey，CQE/UMR QP error。GT revoke也持同umem_mutex并成功后清private，invalidate侧检查NULL。

req:11全绑定，undischarged，8组合；gua:10条件不确立，used=false。req:5全绑定，undischarged，4组合；gua:4端点有绑定缺口。E真实检查了两个不同生命周期需求，没有GT invalidate→revoke同锁/准入关系。

漏报依据：最终10条候选无invalidate_range或umem_odp->private与失效lkey命令；缺口在最终安全需求/对手选择。本轮源码明确可读该函数，不凭此猜模型为何未提。

原始索引：`first_batch/native/CVE-2025-21732/artifacts/CVE-2025-21732_2026-10-13_22-16-06/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-22050：粗粒度对应

rx_submit先usb_submit_urb成功，之后看到going_away而跳过rxq入链；completion→defer_bh仍__skb_unlink未入链skb，NPE。GT把going_away检查移到submit前，并让成功submit无条件queue。

E requires_semantic_review，8个已绑操作形成16组合，walk未绑；gua:8也有绑定缺口。已选queue/cleanup操作多已被queue lock保护，Req中B为unlink_urbs/resume而非GT admissionflag updater，候选本身的互斥诊断不足以解释实际NPE。coarse只记录同路径拓扑/ownership近似，不掩盖错误逻辑。

原始索引：`first_batch/native/CVE-2025-22050/artifacts/CVE-2025-22050_2026-10-13_22-16-17/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-23142：漏报

sendmsg保存transport到待发chunk后，sndbuf等待暂时release_sock；并发bindx remove frees transport，恢复send后outq用旧transport UAF。GT等待期间hold transport并检查dead，解除等待后put。

req:6全绑定12组合但scope/保护条件未证明；req:2 timer uses→destroy_rcu全绑定6组合。两者有真实E检查，不能以transport相同认作sendmsg lifetime召回。

漏报依据：最终7个候选没有sctp_wait_for_sndbuf或sendmsg/outq保存transport使用关系，缺失具体参与路径。源码9217-9256只hold asoc，9244 release_sock，source支持目标仍存在。

原始索引：`first_batch/native/CVE-2025-23142/artifacts/CVE-2025-23142_2026-10-13_22-19-30/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-23151：明确对应

mhi_unprepare_channel deinit frees channel rings后，mhi_gen_tre仍可取得channel锁但未验证ch_state，读已deinitialized ring wp并解引用。GT在锁后拒绝非enabled channel。

9操作全绑定，requires_semantic_review，20组合；gua:18 eligible但未消解。未精确指出锁后ch_state检查缺失，也未证明完整停止新准入；这影响修复可操作性/证明质量，但不要求所有GT操作才能召回。 进一步证据：Req:22 actors明确queue/event/debugfs activation与unprepare/remove，scope为same mhi_chan transfer context deinit generation；gua:18 conditions明确只串行prepare/unprepare/remove，does not exclude queue/event/debugfs readers。A1199真实load44012与B625/627 frees33057/33061，rule_failures逐一no_synchronization_chain。源1193注释说明write_lock_bh仅保护WP读增；1422置DISABLED、1423解锁后1427deinit，gen1194持锁后无state检查而1200即解引用buf_info。这支撑明确资源/实际路径/寿命需求召回，而不是仅同模块粗近似；根因诊断仍不完整。

原始索引：`first_batch/native/CVE-2025-23151/artifacts/CVE-2025-23151_2026-10-13_22-19-41/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-37772：漏报

同rdma_cm_id.net_work在每次notifier callback重INIT_WORK，覆盖已queued/running work data/entry；GT初始化移到create_id。

req:2全绑定4组合，gua:2 eligible但used=false；req:19 8组合且list读有绑定缺口。均不能检查5244 INIT_WORK与5246 queue/worker work state关系。

漏报依据：最终20个候选没有重初始化/已排队work状态需求。源码5244目标语句仍存在；不因候选数量或同net_work字样推断覆盖。

原始索引：`first_batch/native/CVE-2025-37772/artifacts/CVE-2025-37772_2026-10-13_22-24-13/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-37854：漏报

mode1 reset未结束时GPU用户队列仍DMA访问系统内存/VRAM，CPU process cleanup却free outstanding BO；GT在回收前flush各reset_domain wq。

req:3 2组合、一个work调度未绑，gua:3 eligible未用；req:7 24组合，fence作用域条件未建立。E确实查CPU process与fence，但没有BO free1140对GPU reset/DMA的关系。

漏报依据：最终5候选未出现GPU reset completion before BO free；不能将相同cleanup函数的p lifetime/fence混合状态认为召回。

原始索引：`first_batch/native/CVE-2025-37854/artifacts/CVE-2025-37854_2026-10-13_22-25-57/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-37882：漏报

xHCI旧xrun事件TRB地址在延后IRQ时对应新TD，skip旧TD后可错给back新URB而硬件仍用buffer。GT区分xrun并禁止按此事件完成当前TD。

req:3 A list_add3351/ring->enqueue3205/inc_enq3215/firstcycle3426绑定98279/85877/85911/98722；B cleanup843/dequeue1767/invalidate992绑定83564/102219/91936，三个TD列表读取2831/2845/842未绑定。E requires_semantic_review，12组合未消解；gua:2控制器xhci->lock Exclude因一处source binding失败而ineligible、unused。CPU端发布/取消锁不能为旧硬件事件建立TD世代身份。req:9五操作全绑定，E undischarged，6组合；gua:8 wmb Order因两个fence绑定失败/缺同步证据而未用，目标仍是payload-before-ownership。SDK源码轨迹result:36读到handle_tx_event skip/匹配分支，result:48读到2771/2774/2776 xrun逻辑；据最终契约可确认目标需求未提取，不能称E错误消解。

漏报依据：首要阻断是目标Req缺失。完整补跑12条Req和10最终报告没有旧event与当前TD身份/世代复用及提前giveback安全需求；最近两条分别检查CPU列表发布互斥和硬件消费前初始化顺序，与GT身份错误不同。

原始索引：`retry4/native/CVE-2025-37882/artifacts/CVE-2025-37882_2026-10-14_14-53-53/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-37920：粗粒度对应

共享UMEM的不同xdp_sock共用同xsk_buff_pool FILL queue，per-socket rx_lock不能串行不同socket RX消耗共享FILL。GT将rx_lock移到pool。

req:15 A25995/25856/25292/25306与B24843/24850/24167共12已绑定组合，requires_semantic_review；690 list_empty未绑，assumption为external driver是否单NAPI串行未知，无对应Gua。它有正确同pool分配对象，但操作是free_heads/free_list，未包含FILL cached_cons、generic xsk_generic_rcv341 per-xs锁或两个不同xs绑定同pool的实例分析。科学评价必须单列：真实GT漏锁域在共享pool而报告部分对手写为driver allocator/free，E并未证明该cross-socket调用情景。源码链和共享分配状态支持宽松资源/参与路径对应，不能宣称已验证FQ根因或完整形式关系。 特别地，Req actors泛化为driver alloc/free，没有刻画两个不同xs各自进入generic RX的两次激活；选定资源为free_heads/free_list而非GT FILL/cached_cons，因此契约/E非完整GT逻辑。

原始索引：`retry4/native/CVE-2025-37920/artifacts/CVE-2025-37920_2026-10-14_14-53-53/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-38078：粗粒度对应

OSS change_params在PCM ops/buffer访问锁外silence写runtime->dma_area，可与buffer free交叉UAF；GT转core helper并持buffer访问锁。

req:2两copy绑定63691/63831；三free/clear操作488/489/491绑定71254/71258/71261，全部五操作绑定，E undischarged，6组合，facts空。gua:2提取了正确native buffer_accessing原子准入协议：2260 atomic_inc_unless_negative/2268 atomic_dec 与693 atomic_dec_unless_positive/703 atomic_inc；四原子API操作都未成功绑定且synchronization_evidence_missing，因此ineligible、unused。原始源码2260-2268证明native copy已有准入，OSS1083 bare silence才绕过此协议。不能把恢复Linux atomic API绑定等同已满足LLVM atomic=true的AtomicOp pair。SDK源码轨迹result:8读到2260计数协议，result:74读到OSS1083 silence，Req仍未提取此关键使用方。req:7独立20组合undischarged，gua:6 params_lock eligible unused，保护的是OSS conversion buffer，不是GT DMA area。coarse召回保留科学性缺口：当前native copy候选不能证实为真实缺陷，E也未构造OSS安全失败。

原始索引：`retry4/native/CVE-2025-38078/artifacts/CVE-2025-38078_2026-10-14_15-05-57/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-38104：漏报

持gmc invalidate_lock自旋锁时经SRIOV RLCG取rlcg_reg_lock mutex，invalid wait context；GT改mutex为spin_lock_irqsave。

RLCG req:2四S/两I绑定，E0，gaps69072→69200 sequence_order_not_established；gua:2 mutex Exclude eligible=true/used=false，不能称discharged。final PCIE req:1 3组合即使同GPU MMIO也非GT锁上下文。

漏报依据：最终6报告无mutex-under-spinlock/invalid wait context。Req/Gua形式目前抽取互斥没有表达锁上下文合法性；范围问题保留而不删分母。

原始索引：`first_batch/native/CVE-2025-38104/artifacts/CVE-2025-38104_2026-10-13_22-30-08/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-38165：漏报

backlog先skb_get增加users再走skb_linearize，触发skb_shared断言；GT将take_ref移到linearize之后、queue_msg之前并区分verdict来源。单激活也可失败，属并发修复引入的API前提问题。

ordering req:5完整2S/3I绑定并检查3组合；E undischarged。模型有源543 linearize/628 get，但没有该操作前提Req/候选。

漏报依据：最终8报告不涉及skb_shared/linearize/引用增加位置；同ingress/backlog路径不替代不同安全需求。范围差异单列。

原始索引：`first_batch/native/CVE-2025-38165/artifacts/CVE-2025-38165_2026-10-13_22-37-29/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-38217：明确对应

fts_read先读fan_source检查非INVALID，再第二次读作为BIT shift；并发fts_update_device能改INVALID=255，造成undefined shift。GT单次局部快照及BITS_PER_LONG界限检查。

2S/2I全部绑定6464/6476及6833/6846，E undischarged检查2组合，无目标Gua。相同channel实例别名未由E独立证明，但源码及scope清楚，逻辑可复核。

原始索引：`first_batch/native/CVE-2025-38217/artifacts/CVE-2025-38217_2026-10-13_22-37-32/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-38242：漏报

UFFD move lockless查swapcache后entry复用/ABA，锁后仅复验PTE值不能证明folio仍属entry；NULL lookup后还可新增cache。GT补folio身份及NULL的SWAP_HAS_CACHE检查。

req:5 source A有四条cache lookup/add：__read_swap_cache_async382/445绑定99927/100074，try_to_unuse2348及GT move_pages_pte1392均operation_not_resolved；B数组669kvfree/671clear绑定101045/101053。E requires_semantic_review，只检查两已绑定A×两B=4组合，facts空，gua:5 Wait eligible但unused（source_completion跨函数顺序缺口），不能称discharged，也不能说GT lookup已被E检查。其Gua以get_swap_device或RCU保护same-type整设备活跃世代，无法建立entry/folio身份。req:2仅写3614和put1393绑定121612/105368，read3567与free1577未绑，MustBeAtomic E0、atomic_sequence_or_interference_domain_missing，gua:2 cluster锁因三个绑定缺口ineligible，未获报告；此需求并非GT。SDK工具轨迹33/34完整读取userfaultfd993-1460含move_swap_pte/PTE验证与1392lookup→1402trylock→1413move，55/56复读1392，57写出的却是数组生命周期契约；可证实目标已读到而目标需求未入最终契约。

漏报依据：首要阻断是目标Req缺失：GT需要拿folio锁后验证仍在swapcache且entry相符，并对NULL lookup在PTE锁后重验SWAP_HAS_CACHE；新9Req/5报告没有此lookup结果/entry/PTE跨阶段身份及存在性一致性需求。最近Req的1392绑定失败是数组回收候选的次要缺口，不能代替或被归因成GT目标遭E消解。

原始索引：`retry4/native/CVE-2025-38242/artifacts/CVE-2025-38242_2026-10-14_15-17-17/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-38250：漏报

ioctl hci_dev_reset取得hdev后调用flush→vhci_flush使用driver_data；close unregister摘链后free vhci_data，无等待已取出的ioctl。GT每hdev SRCU覆盖reset并在unlink后等待。

suspend_work 3组合、debugfs 5组合全部已绑，均undischarged，Gua等待/外部debugfs寿命条件未确立；检查的是不同callback，并不是目标被E消解。

漏报依据：最终15报告没有hci_dev_reset/vhci_flush after unlink关系，不能仅kfree(data)相同计召回。

原始索引：`first_batch/native/CVE-2025-38250/artifacts/CVE-2025-38250_2026-10-13_22-40-33/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-38337：漏报

GT为jh->b_modified锁外允许racy读取补data_race annotation，以及transaction可NULL时先is_handle_aborted后取t_journal。两部分均不等价于transaction被并发free的寿命问题。

E部分绑1492读70718、forget71630→free28036，共2组合，gua:11 eligible未用且source_completion缺跨函数must-order；与GT允许racyhint或NULL deref不同。

漏报依据：最终17报告无b_modified，NULL前提也没提；目标范围(annotation/普通NULL防护)单列，保留分母。

原始索引：`first_batch/native/CVE-2025-38337/artifacts/CVE-2025-38337_2026-10-13_22-42-19/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-38383：漏报

proc_create_single_data一次分配的counter数组经m->private被多seq_file共享，show_numa_info并发memset/累加/打印；GT每show分配私有counter。

E 12真实组合，pages读有未绑、gua:5ineligible；已选的是vm metadata。same函数不能当shared counter需求召回。

漏报依据：最终15报告无共享counter/private数据别名需求，source4945与5055共享实例可复核；不猜为什么模型漏提。

原始索引：`first_batch/native/CVE-2025-38383/artifacts/CVE-2025-38383_2026-10-13_22-47-15/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### CVE-2025-38429：漏报

MHI EP先发布ring_ctx.rp，后write_sync写入host buffer，host可看到尚未写完的元素。

全部12条Req均未表达此host发布要求；不能归为E消解错误。SDK源码工具第4行实际返回了发布和write_sync的顺序。

漏报依据：目标Req未提出：读到源码但未构建host buffer写完成与rp发布之间的需求。

原始索引：`first_batch/native/CVE-2025-38429/artifacts/CVE-2025-38429_2026-10-13_22-47-59/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-01affb1491750534：漏报

process_one_work在pool->lock外递增同pwq的PWQ_STAT_COMPLETED；修复把递增移到重新持锁之后。

req21全部7个操作operation_not_resolved，E=0；真实IR store29753存在，路径pwq.stats[1]，源码selector使用枚举PWQ_STAT_COMPLETED。另A/B把不同counter index分列，未完整表达completed增量与另一completed增量。

漏报依据：绑定阻断，并伴随RMW对侧操作/同索引关系缺口；不是没有相关Req，也不是被Gua消解。

原始索引：`first_batch/native/SYZBOT-01affb1491750534/artifacts/SYZBOT-01affb1491750534_2026-10-13_22-51-00/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-1c486d0b62032c82：漏报

失败exec放开cred_guard_mutex后迟到清零fs->in_exec，覆盖另一exec新设置的1；修复在持锁时清零。

req3四操作全bound，E=2，但I是copy_fs的读/增users，非旧exec清零。SDK第18行返回过两处current->fs->in_exec=0。

漏报依据：目标Req未提出；仅同in_exec字段的clone并发候选不能替代旧exec清零覆盖新世代。

原始索引：`first_batch/native/SYZBOT-1c486d0b62032c82/artifacts/SYZBOT-1c486d0b62032c82_2026-10-13_22-51-25/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-2d373c9936c00d7e：粗粒度对应

ALSA poll通过pool状态/room/total_elements判断可写，ioctl可并发修改；修复收拢poll检查并在pool_poll_wait加pool->lock。

实际reader33084与writers22227/34402已bound，E=20。漏caller的snd_seq_write_pool_allocated/total_elements读；另两fifo_pool_size操作未知，组合混入不同配置字段。

范围备注：官方修复称原poll数据竞态harmless，保留原分母并单列，不把粗召回视为Req全部关系科学正确。

原始索引：`first_batch/native/SYZBOT-2d373c9936c00d7e/artifacts/SYZBOT-2d373c9936c00d7e_2026-10-13_22-56-48/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-2e4de7fe846aba66：漏报

TUN用普通NAPI注册使busy-poll与napi_gro_frags并发访问同GRO状态；修复改netif_tx_napi_add以禁止busy-poll。

没有目标NAPI/GRO Req。SDK第14行读到netif_napi_add，源码工具返回中没有napi_busy_loop，不能宣称其完整追过busy-poll对手。

漏报依据：目标Req及busy-poll对手未提出/追清。

原始索引：`first_batch/native/SYZBOT-2e4de7fe846aba66/artifacts/SYZBOT-2e4de7fe846aba66_2026-10-13_22-56-49/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-35301db28c64310d：明确对应

MACSEC多CPU普通net_device计数递增/读取竞争，修复使用DEV_STATS_INC和atomic_long_read。

10操作全部bound，E=25；包含真实同字段写写/写读组合，但也混入不同counter字段组合，且未覆盖全部补丁递增点。明确召回不表示每个组合有效。

原始索引：`first_batch/native/SYZBOT-35301db28c64310d/artifacts/SYZBOT-35301db28c64310d_2026-10-13_22-57-21/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-3b6b32dc50537a49：漏报

__fput中的eventpoll_release允许无锁读取file->f_ep作fast check，需READ_ONCE/WRITE_ONCE注释访问；不是列表完整性缺锁。

req6 E=9，release_file1096读歧义；实际参与E的是attach读/写与__ep_remove写/链表操作。不能将正常f_lock保护的列表关系当无锁fast-check访问完整性。已做独立第二意见。

漏报依据：目标fast-check access-integrity Req未提出；最近候选为另一安全需求。

范围备注：修复为有意允许的racy fast path添加访问注释，保留原分母。

原始索引：`first_batch/native/SYZBOT-3b6b32dc50537a49/artifacts/SYZBOT-3b6b32dc50537a49_2026-10-13_23-00-36/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-3cc3a12efa69aa6f：漏报

dev_isalive可在RTNL或dev_base_lock下读reg_state，写端只持RTNL导致与sysfs读者冲突；修复写端同时持dev_base_lock。

22条Req没有reg_state。SDK第21和27行分别返回写端和dev_isalive读端，遗漏不能归因为未供应源码或E错误消解。

漏报依据：读到两侧但未提出reg_state的实际锁域一致性Req。

原始索引：`first_batch/native/SYZBOT-3cc3a12efa69aa6f/artifacts/SYZBOT-3cc3a12efa69aa6f_2026-10-13_23-05-04/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-417aeb05fd190f3a：漏报

__unmap_hugepage_range在page_table_lock而非hugetlb_lock下读h->surplus_huge_pages；另adjust_reservation没有每轮清零。

相关pool计数Req很多且E有报告，但不含unmap读者，不能将sysfs统计读替代实际预约决定。SDK第8行返回了5954检索结果；未证明Agent读完整unmap窗口。

漏报依据：目标读者/同步域Req遗漏；循环变量缺口另列，不据候选数量计召回。

原始索引：`first_batch/native/SYZBOT-417aeb05fd190f3a/artifacts/SYZBOT-417aeb05fd190f3a_2026-10-13_23-05-51/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-44cf88a58d91b12b：漏报

raw_bind没有lock_sock，与__ip4_datagram_connect并发读写同socket地址字段；修复为整个raw_bind加socket锁。

raw739链式赋值operation_not_resolved，req7读侧均未知/E=0；req5 E=2只剩connect写daddr→proc读。未构建目标关系，也有绑定缺口。

漏报依据：目标Req/对侧操作遗漏，并伴链式赋值和内联读绑定缺口。

原始索引：`first_batch/native/SYZBOT-44cf88a58d91b12b/artifacts/SYZBOT-44cf88a58d91b12b_2026-10-13_23-07-33/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-4a03518df1e31b53：粗粒度对应

extent-status lookup在读锁下并发读/替换cache_es，编译器refetch可使in_range观察错误；修复READ_ONCE/WRITE_ONCE及一次快照。

选定cache writer802/reader934未绑定，且漏读锁writer296；E=4实际是rb_insert/rb_erase→root读/range调用，未检查GT reader-reader cache conflict。独立第二意见支持报告层面粗对应，逻辑构造缺口仍明确。

原始索引：`first_batch/native/SYZBOT-4a03518df1e31b53/artifacts/SYZBOT-4a03518df1e31b53_2026-10-13_23-09-28/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-4a06d4373fd52f0b：明确对应

inotify mark删除把wd置-1，并发event构造可能报告无效wd；修复READ_ONCE快照并验证wd!=-1。

5操作全部bound，E=6含11602(remove写)→10622(event读)。Req为scalar mediation，未单独表达-1 validity检查；不影响实际事件descriptor问题匹配。

原始索引：`first_batch/native/SYZBOT-4a06d4373fd52f0b/artifacts/SYZBOT-4a06d4373fd52f0b_2026-10-13_23-10-00/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-52e3dbded1f71729：明确对应

kcm_rfree在rx_lock外读rx_psock，reserve/unreserve在锁内写；修复READ_ONCE/WRITE_ONCE注释。

6操作全部bound，E=8含98005读→106199(unreserve写)、107125(reserve写)，同时也检查rx_wait等其他组合。

原始索引：`first_batch/native/SYZBOT-52e3dbded1f71729/artifacts/SYZBOT-52e3dbded1f71729_2026-10-13_23-11-33/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-5676077ba016d741：漏报

多个xmit及LL_RESERVED_SPACE读写同dev->needed_headroom；修复单次访问READ_ONCE/WRITE_ONCE。

GT内嵌patch截断无diff，另人工读取pinned Linux仓库完整fix 4b397c06cb987935b1b097336532aa6b4210e091，确认三条xmit与LL_RESERVED_SPACE访问修复。SDK工具23/86/92/97行返回needed_headroom相关源码。

漏报依据：目标共享标量Req未提出，不是E消解了对应Req。

原始索引：`first_batch/native/SYZBOT-5676077ba016d741/artifacts/SYZBOT-5676077ba016d741_2026-10-13_23-14-57/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-5a486fef3de40e0d：漏报

io_uring_register放开uring_lock后trace读取file_table.data.nr/buf_table.nr，可与另一次注册并发；修复把trace移到锁内。

SDK第29行实际返回unlock之后trace操作。9条Req没有表大小trace reader。

漏报依据：诊断/trace shared scalar reader未产生Req。

范围备注：官方修复称只用于trace输出、不太在意该race；保留原分母。

原始索引：`first_batch/native/SYZBOT-5a486fef3de40e0d/artifacts/SYZBOT-5a486fef3de40e0d_2026-10-13_23-17-01/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-5cce5938c6c2c518：漏报

UDP无锁发送从多个CPU读/增同inet_id，修复atomic_t和非TCP atomic_add_return。

完整该header在模块输入中；SDK输出虽出现inet_id其他使用，但未返回关键inet_sk(sk)->inet_id += segs行。不能把完整供应等同Agent完整读过。

漏报依据：目标inline header的并发RMW未追到/未提出Req。

原始索引：`first_batch/native/SYZBOT-5cce5938c6c2c518/artifacts/SYZBOT-5cce5938c6c2c518_2026-10-13_23-20-22/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。

### SYZBOT-63cbe31877bb80ef：漏报

queue locked/klocked与check_again/check_blocked位域共用存储，由不同spinlock保护，编译器对共享字节RMW会相互破坏；修复为独立bool。

req23全6操作operation_not_resolved。req27 klocked操作未知，E=6只剩locked/owner写与owner/locked读；IR41872是store i8、DWARF仅q.locked，不能据逻辑字段不同排除实际共享存储。

漏报依据：共享物理存储的跨字段/跨锁域Req遗漏，且位域对应绑定失败。

原始索引：`first_batch/native/SYZBOT-63cbe31877bb80ef/artifacts/SYZBOT-63cbe31877bb80ef_2026-10-13_23-21-23/`；最终报告`stateful_bugs/reports.json`，契约/E为`module_contracts.json`。
