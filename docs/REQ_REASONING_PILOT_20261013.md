# Req 推导与绑定优化：七例真实模型结果

2026-10-13 北京时间17:14:42–17:37:09。七例全部结束，没有模型队列待接管。
实际实现提交 `18ab94cdac000190e30f935e7c347a533ccc6aaa`；分支 `codex/source-contract-artifacts`，远端工作树 `LLM4Con-req-first-workflow`。
本次查询结果只做人工复核和记录，没有继续改实现或启动新实验。

## 结果

7/7 native exit0，两次 Phase A 与 E 完成，Phase C 关闭，implementation_unchanged=true。
全部 artifact_errors 为空，没有上下文超限或 SDK 异常。context management 请求 auto，但实际仍因 Chat Completions 而未启用 native compaction；不是主动关闭。
模型 gpt-5.5-2026-04-24，SDK openai 3.26.0 / openai-agents 0.23.1，reasoning provider default。
jobs=7、max-turns=0、requirement-sweep=on、contract-input=artifacts。
源码、IR、autoconf 与上一批 c4f4695 的七个 development 输入逐例 hash 核验一致；依赖查询固定相同 Linux revision。

| Case | 请求 | 分钟 | E 报告 | 人工 GT 对照 |
| --- | ---: | ---: | ---: | --- |
| KVM / CVE-2024-40953 | 65 | 19.9 | 17 | 未有目标关系；仍无 last_boosted_vcpu Req |
| MACSEC / SYZBOT-35301db28c64310d | 47 | 12.7 | 13 | 对应候选保住 |
| CIFS / CVE-2024-35999 | 68 | 22.5 | 9 | 粗相关；已加入 picker 锁内读，仍漏真正锁外返回读 |
| serdev / CVE-2024-35977 | 62 | 12.7 | 10 | 粗相关；明确写出 open 早于初始化，dispatcher/准入假设仍残留 |
| GPU / CVE-2025-37854 | 78 | 20.0 | 8 | 未有硬件/reset 完成→BO 释放关系 |
| ipset / CVE-2024-39503 | 27 | 8.3 | 15 | 有 GC 生命周期泛候选，仍未形成目标 owner/RCU callback 关系 |
| NF objects / CVE-2024-27019 | 47 | 12.9 | 9 | 回退：上一批对应的全局类型注册表 Req 未产生 |

按上一批同一宽松人工口径：**1例明确对应 + 2例粗相关/逻辑缺口，4例无对应目标**。
粗相关计入时是 3/7；上一批为 2明确+2粗相关，即4/7。没有整体召回改善。
不要求根因/类型/全部 GT 操作精确一致；粗相关继续保留，但不能据此认为 Req/Gua 操作域和 E 关系正确。
这只是七个 development 输入的一次对照，不是72例正式 benchmark，不是单变量消融；抽取也有随机性。

总请求407→394，input 41,909,577→38,528,782，output 291,283→274,912。
平均单例15.23→15.56分钟；批次22.45分钟。速度总体接近，不能声称本次明显提速。
input 含反复发送历史，不是独立代码量或已经核实的计费 token。
Req 111→106，Gua 94→82，报告75→81；数量不能当召回或抽取完整性的替代指标。

## 人工契约与轨迹核对

- **MACSEC**：req:15 / finding:11 是同设备普通 rx_dropped/tx_dropped/rx_errors 更新与读，11个唯一操作全部绑定，E评估28组合。真实同字段读写/写写冲突存在；三个字段仍合并，跨字段组合需要另行优化。
- **serdev**：req:5 / finding:4 明确指出 uart.c:266 open 先于272/273/294/302初始化，纠正了上一批“set_client_ops必为第一次admission”的断言。5个初始化操作全部绑定，priv对应6838；回调get_drvdata/priv读取也绑定，`&ec_uart->response`被当read仍未知，E检查10个组合。Req仍把准确准入列为外部framework假设；B在用户回调体，没有进入回调前的serdev dispatcher ops NULL解引用。按宽松口径计粗相关，不宣称目标逻辑已经完全正确。
- **CIFS**：req:8 / finding:8 加入transport.c:1031/1032的picker锁内读；12/14个操作绑定，35组合。完整函数1028–1062至少3次返回到上下文，1062真实锁外读出现在5个shell返回中，却没被选入Req。实际load123360在图。gua:8对列出的chan_lock区域成立，但两项bitmap宏操作未绑定，整条ineligible/未用于消解；此候选不能说明锁外GT读已被E检查。相比上批有阅读和参与者改善，操作选择仍偏离目标。
- **KVM**：23Req/17报告没有last_boosted_vcpu；源码返回仍包含2589读、2623写，图中39630/39819存在。新online_vcpus publication和其他生命周期需求不覆盖该字段。当前日志没有明确放弃理由，不推断隐藏推理，更不能说E把它消解了。
- **ipset**：19Req/15报告没有__list_set_del_rcu里e->set使用owner先于owner释放的关系。req:3表达成员读者→成员/extension释放，req:6是正在运行的timer→map/set释放，req:10是packet slot用户→set释放。图中owner读20554、ip_set_ext_destroy调用20558存在。timer取消不能替代已经排入RCU的callback完成。gua:2的RCU、gua:5的timer等待又因未核验API语义被声明unconditional=false，进一步增加候选；报告2→15不是新增目标召回。
- **GPU**：12Req/8报告仍无目标硬件/reset完成与BO释放需求。已读release worker/BO free和reset_work；新增5项pinned读取集中在event、ioctl、interrupt和eviction，未追目标reset完成链。不能把“这次用了依赖工具”当成依赖调查已经到位。
- **NF objects**：全局nf_tables_objects和6893/6908注册/注销返回到了上下文，但查找函数__nft_obj_type_get没有出现在shell源码结果中。没有对应类型注册表Req；图中70022/70055读、44886注销仍在。req:6的RCU dump/table child object lifetime是表内实例的另一关系，它被discharged不意味着目标类型表Req被误消解。这里是实际回退。

## 绑定与流程

| Case | 引用操作数 旧→新 | 未知操作数 旧→新 |
| --- | ---: | ---: |
| KVM | 204→202 | 39→36 |
| MACSEC | 207→176 | 38→27 |
| CIFS | 164→119 | 14→7 |
| serdev | 52→91 | 23→12 |
| GPU | 153→89 | 21→14 |
| ipset | 176→175 | 85→33 |
| NF objects | 171→106 | 32→15 |

Req/Gua实际引用的去重操作1127→958，unknown252→144，占比22.4%→15.0%。
两轮契约和操作集合不同，不能当成纯binder成功率消融；上一轮冻结账本33个自动绑定改善仍只属于独立离线验证。
这次所有write已采用字段locator，没有触发source_effect_selector归一化，也没有function_selector_mismatch。格式和调查质量有改善；copy/helper/多效果宏、伪read/缺IR等仍有未知。
新E状态73条requires_semantic_review、31条undischarged、2条discharged；unknown状态并非没有检查，绑定组合仍正常进入E。
本次没有把notes转成Req或进行自动GT/阶段分类；人工判断保存在本地manual_review.json。

下一步的关键不是继续增加提示关键词：CIFS已经读到目标访问却没选入，NF则没闭合注册表的查找侧。
需要围绕已发现资源追完整参与者/访问和同步边界，并防止正常API语义因未查询而整体退为unknown。此处仅记录方向，尚未实现下一轮修改。

## 原始记录

远端 `/mlx_devbox/users/mayunlong.39/playground/lace-req-reasoning-pilot-20261013`；本地 `../analysis_outputs/lace-req-reasoning-pilot-20261013`。
完整382文件、311,337,428bytes已下载并逐文件核验SHA256。archive SHA256为 `fc0bcee5920603eeffeff70d034b51449380657165a99dfd78ac8b5c1cb601fd`。
原生native batch/run manifest、模型、原Req/Gua、SDK轨迹、workspace文件和最终reports/bugs.txt均保留。
本地新增cost_and_binding_comparison.json只计算工程统计，manual_review.json只保存人工结论；不改原生结果或上一批目录。
GT/历史补丁仅在事后人工读取，未发送到Agent。原IR完整构建来源仍按既有manifest说明，不因source byte核验就声称已经证明IR provenance。
