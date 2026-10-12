# PopStrike：从当前工程到可玩的联网 FPS

这份文档是实施和学习入口。架构约束见 [AGENTS.md](../AGENTS.md)，已有大厅的蓝图接线见 [Lobby.md](Lobby.md)。每完成一个小步骤，就更新本文件的进度和验证记录。计划中的类名表示未来职责，不代表文件已经存在。

## 1. 我们先做出什么

第一版目标是：玩家进入现有大厅、加入两队、进入果汁工厂，以第一人称移动、持枪、射击和换弹；服务器决定伤害和胜负，其他玩家能看到正确动作；遮挡后的敌方敏感信息按规则停止下发，重新出现时动作连续。最终在六人、模拟弱网和独立服务器上验证。

这是一条逐步实现路线，不是一轮内堆出全部功能。每次交付一个能解释、能运行的小增量。具体胜负规则、复活规则和技能在相应阶段确定，不预先加入商城、匹配后端、账号服务或复杂技能体系。

“最小代码量”指尽可能复用引擎已验证的能力，保持一个清晰的数据来源；不是省略服务器校验、状态恢复或用大量蓝图节点隐藏复杂度。

## 2. 借鉴的完整项目，以及具体怎么看

| 参考项目 | 看什么 | 用在本项目哪里 | 边界 |
|---|---|---|---|
| [VALORANT 迷雾复盘](https://www.riotgames.com/en/news/demolishing-wallhacks-valorants-fog-war) | 先读 Network Relevancy、Server Visibility、Consistency & State Management | 按玩家过滤信息；敌人出现时恢复当前动作 | 已发行产品的一手复盘，无完整源码；不能当可直接安装的插件 |
| [VALORANT 网络复盘](https://www.riotgames.com/en/news/peeking-valorants-netcode) | 先看本地预测、服务器纠正和命中回溯示意 | 理解“立刻响应”与“服务器说了算”如何共存 | 不照搬其固定步长系统或 128 Tick 预算 |
| [Valve Source SDK 2013](https://github.com/ValveSoftware/source-sdk-2013) | TF2 / HL2: DM；找到 `src/game/server/player_lagcompensation.cpp`，跟踪开始回溯、查询、恢复 | 设计有界历史命中检查和异常时间处理 | 已发行游戏的公开游戏代码；许可不适合直接搬进此 UE 项目 |
| [Quake III Arena](https://github.com/id-Software/Quake-III-Arena/blob/master/code/server/sv_snapshot.c) | `SV_BuildClientSnapshot` → `SV_AddEntitiesVisibleFromPoint` | 理解“每个玩家收到的世界可以不同” | 完整发行项目源码，GPL；研究原理，不复制实现 |
| [Lyra](https://dev.epicgames.com/documentation/en-us/unreal-engine/lyra-sample-game-in-unreal-engine) | 装备定义、武器实例、角色动画层；按一个装备流程读 | 数据与运行时状态分离、动画组织、UE 实现风格 | 官方完整样例，不能冒充上线 FPS；本工程并不具备完整 Lyra 框架 |

阅读时每次只追一个问题，例如“服务器如何知道客户端请求开火”“敌人在换弹中重新出现怎么办”。先理解我们自己的小实现，再对照参考项目对应的一个函数或图，不要求先读完整个引擎。

## 3. 当前起点：有什么、缺什么

2026-09-20 检查结果：

- 引擎日志为 UE 5.8.2；本机编辑器在 `D:/UnrealEngine-release/Engine/Binaries/Win64/UnrealEditor.exe`。
- 已有 `PopStrikeServer.Target.cs`、`PopStrikeClient.Target.cs`；现有二进制目录发现 Editor 模块，尚不能宣称 Server/Client 独立目标已经构建和打包。
- 已有连接、大厅权限、队伍、地图选择与 `ServerTravel` 代码；没有比赛 Character/Combat 的 C++ 实现。大厅故意设置 `DefaultPawnClass = nullptr`，因此大厅里没有可走动角色不等于移动坏了。
- 当前编辑器 `net.Iris.UseIrisReplication = 0`；实际服务器仍须通过启动日志确认复制后端。
- 已导入 Manny/Quinn 和枪械。优先选择真实存在的 `/Game/Weapons/Rifle/Mesh/SK_Rifle` 作为第一把枪的素材。
- 资产注册表确认 `ABP_Mannequin_Base` 继承 `/Script/LyraGame.LyraAnimInstance`；`GA_Weapon_Fire` 继承 `/Script/LyraGame.LyraGameplayAbility_RangedWeapon`。当前模块不包含 LyraGame，不能直接把这些资产当可运行系统。
- `B_Weapon` 依赖 `/Script/LyraGame` 与 `/Game/Effects/...`。现有 Content 顶层未发现 Effects，接入前要补足选用素材的依赖或使用独立的小型武器表现。

前面仅依据动画数量判断“资源足够”，不能证明动画蓝图能直接挂到普通 Character 上。后续复用网格和适配的动画序列；第一版创建项目自己的简单 AnimBP，不强行给 Lyra 原始 AnimBP 换父类，也不删除原始资产。

## 4. 最小职责划分

| 谁 | 管什么 | 不应该管什么 |
|---|---|---|
| 现有 ConnectionSubsystem | 地址连接、连接失败提示、本地资料 | 比赛伤害、敌方位置 |
| 现有大厅类 | 组队、房主权限、地图选择 | FPS 移动和枪械 |
| 比赛 GameMode | 服务器出生、比赛开始结束、胜负 | 客户端 UI |
| GameState / PlayerState | 允许公开的比赛、身份和队伍状态 | 未授权敌人的精确动态信息 |
| Character + CMC | 身体、相机、移动、输入接口 | 一整套手写位置复制协议 |
| CombatComponent | 装备、射速、弹药、换弹和射击请求 | UI 控件布局 |
| WeaponDefinition / 武器外观 | 静态配置 / 模型和特效 | 独立于 Combat 的第二套弹药数据 |
| 服务器可见性策略 | 每个连接允许接收什么 | 客户端画面遮罩、服务器敌人是否模拟 |
| AnimBP / UI | 从状态产生姿势和界面 | 判定伤害、决定换弹是否成功 |

这些职责按需要逐个增加，初期保留一个 Runtime 模块。普通状态复制、移动平滑、计时器、数据资产优先用引擎现有机制；多个类不意味着必须多个模块。

## 5. 实施顺序与每一步的学习任务

### 第 0 步：可重复启动与资源体检

交付：资源依赖结论、独立服务器与两个客户端的连接检查、清晰日志。

先用现有 Editor 二进制的 `-server` / `-game` 模式验证网络基础；这是真正独立进程的开发运行方式，但不等于 Server/Client Target 编译或打包成功。随后再验证有画面的客户端大厅操作。

学习顺序：

1. 打开 `Config/DefaultEngine.ini`，看客户端默认地图与服务器默认地图为什么不同。
2. 看 `PSConnectionSubsystem.cpp` 的 `ClientTravel`：客户端选择去哪个服务器。
3. 看 `PSLobbyPlayerController.cpp` 的 `ServerJoinTeam`：请求从客户端进入服务器，服务器校验队伍容量。
4. 看 `PSLobbyPlayerState.cpp` 的 `GetLifetimeReplicatedProps`：服务器修改后如何发给客户端。
5. 按 `Docs/Lobby.md` 理解 UI 订阅状态变化，最后再看房主的 `ServerStartGame` 和 `ServerTravel`。

本次提供的 `Scripts/Test-NetworkBaseline.ps1` 会启动一个仅绑定本机的服务器和两个无渲染客户端，检查连接日志，结束后关闭它自己启动的进程。它不操作当前编辑器，也不测试画面、换队、开局、FPS 或弱网。

在项目根目录 PowerShell 运行：

```powershell
.\Scripts\Test-NetworkBaseline.ps1 -EditorPath 'D:\UnrealEngine-release\Engine\Binaries\Win64\UnrealEditor.exe'
```

预期：打印 PASS 和本轮日志目录。日志保存在 `Saved/NetworkBaseline/<本轮编号>/`；FAIL 会报告原因并保留日志。首次缓存未准备好时可增加 `-TimeoutSeconds 180`。默认使用端口 17777，不修改工程默认端口。

可视操作验收：在编辑器 Play 设置里选 Play As Client、2 个玩家、关闭 Run Under One Process，使用现有大厅地图；按 `Lobby.md` 操作加入队伍/房主开局。检查两个客户端队伍信息一致、非房主无法开局。地图切换后的角色创建和队伍保留属于下一步，当前不可提前打勾。

### 第 1 步：两个人能以第一人称移动

交付：最小比赛 GameMode、Character、Enhanced Input 配置、相机、现有角色网格和简单移动 AnimBP。优先复用 CMC 的网络移动；参考 [CMC 官方网络说明](https://dev.epicgames.com/documentation/unreal-engine/understanding-networked-movement-in-the-character-movement-component-for-unreal-engine)。

按“小角色出生 → 输入移动 → 第二客户端可见 → 动画”顺序实现。确认现有地图碰撞与真实 PlayerStart，缺少时在编辑器按实际场景设置，不能靠未知坐标生成假出生点。比赛地图只配置自己的 GameMode，不替换整个项目的菜单/大厅 GameMode。

第一人称镜头、本地持枪外观与远端世界外观分开处理，参考 [Epic FPS 角色说明](https://dev.epicgames.com/documentation/unreal-engine/implementing-your-character-in-unreal-engine)。先验证现有动画适合哪些视角；如果缺专用第一人称手臂动画，明确记录视觉限制，不假装第三人称动画已经满足最终手感。

同时检查大厅到比赛的生命周期：是否使用无缝切图、PlayerState 的队伍/身份如何保留、输入模式是否从 UI 恢复 Game。根据实际代码实施最小交接，不能把只在直连测试里正常视为完整流程通过。

你学什么：从“按 W”追到输入回调，再追到 `AddMovementInput` 和 CMC；理解 `HasAuthority` 与 `IsLocallyControlled` 的不同。验收：两端走跳和朝向正确，自己的镜头不控制别人，模拟延迟下仍立即响应。

### 第 2 步：一把步枪完成射击、伤害、换弹

交付：一个武器配置、一套 Combat 状态、武器外观和最小生命状态。先服务器即时射线，暂不同时实现霰弹枪/投射物/穿墙。

顺序：挂枪 → 请求开火 → 服务器检查射速与弹药 → 射线/伤害 → 状态复制 → 本地即时特效 → 换弹。镜头瞄准与枪口遮挡必须统一约定，防止贴墙时从墙后射击。

借鉴 Lyra 的配置与实例分离，以及 Valve/Riot 的服务器裁决；复用 UE RPC、RepNotify、Timer、DataAsset，不整套搬 GAS。只有当明确技能需求使 GAS 更省维护成本时才重新评估。

你学什么：画出一次开火的数据流，指出哪一步本地预测、哪一步服务器最终确认；再观察服务器拒绝射击时如何恢复显示。验收：空弹、连续请求、换弹打断、重复确认都不会多扣弹或重复伤害。

### 第 3 步：尽早接入战争迷雾

交付：服务器按连接筛选敌方信息、调试原因、出现/隐藏过渡、持续动作恢复。先在现有地图选一个遮挡明确的角落完成两人场景。

顺序：先观测候选集合和判断原因 → 接复制过滤 → 处理附件/声音 → 增加预揭示与隐藏滞后 → 在换弹中进出遮挡。借鉴 VALORANT 的问题与解决方向，不把单条射线当最终方案；PVS/区域门户在测量后决定是否引入。

你学什么：比较“客户端不画敌人”和“客户端没有收到敌人新状态”。记录一个敌人在遮挡后移动的例子，并检查客户端接收数据。验收：隐蔽数据没有其他复制通道泄漏，露头时不会晚出现，重新出现的换弹进度正确。

### 第 4 步：弱网下仍可理解的命中与动作

交付：有界服务器历史命中检查、时间校验、射击确认去重及拒绝处理；实现网络测试矩阵。

先看服务器当前判定与客户端瞄准为何有偏差，再引入回溯。记录命中体历史，不回滚整个游戏；限制回溯窗口，重生后不能用上一条生命的历史。对动态遮挡明确策略后再支持。

你学什么：跟随同一个 ShotId 比较请求、校验、回溯、伤害与客户端确认。验收：50/100/150–200 ms 实测 RTT 与不同丢包下没有永久状态分歧；命中差异有日志，不只凭手感判断。

### 第 5 步：六人比赛与内容扩展

交付：两队、出生/复活、最小胜负规则、结束与返回大厅，以及第二种武器。此阶段落实具体模式，角色技能另按真实需求加入。

你学什么：用新增同机制武器检验数据驱动是否成立；如果每加一把枪都要修改多个角色类，就先修正职责分配。验收：六人完整一局可结束，队伍正确，迷雾和服务器预算达标。出现瓶颈才选择 Replication Graph 或 Iris。

### 第 6 步：独立构建、打包和持续验证

交付：真正的 Server/Client Target 构建与 Cook、资源收集、跨机连接、关键回归用例。构建方式参考 [Epic 的 Lyra 专用服务器教程](https://dev.epicgames.com/documentation/unreal-engine/setting-up-dedicated-servers-in-unreal-engine)。

你学什么：区分“C++ 编译”“链接”“Cook”“打包”“运行”五件事；会从日志定位失败环节。验收：目标机器独立启动，不依赖编辑器；已实现的联网/迷雾规则在打包版复测通过。

## 6. 每次如何一起学

每次交付按这个顺序阅读：

1. 本次效果：玩家现在能做什么，尚不能做什么。
2. 改动文件：先看 `.h` 的职责和状态，再看 `.cpp` 中一条完整操作链。
3. 引擎替我们做的事：指出哪些复制、移动或生命周期机制没有必要重写。
4. 参考项目的对应片段：解释哪些原理保留，哪些规模相关实现暂未采用。
5. 亲手实验：给出一个正常操作和一个异常操作，预先写出应当看到的结果。
6. 验收记录：区分已经执行、失败和仍需手动验证的项目。

学习时始终回答三个问题：这个值谁能改？谁会收到它？丢了一条消息或晚加入还能恢复吗？这三个问题会贯穿移动、射击、UI 和战争迷雾。

## 7. 实施进度

| 项目 | 状态 |
|---|---|
| 开发路线与学习顺序 | 已写入本文 |
| 资产父类/核心依赖检查 | 已发现 LyraGame 依赖，原始资源保留；选用素材的完整加载验证继续进行 |
| 独立进程连接检查工具 | 已编写，实际运行结果见下方记录 |
| 可视大厅操作、开局切图 | 待验收 |
| FPS 角色、战斗、迷雾、回溯 | 未实现，按第 1–4 步推进 |
| 六人完整比赛、打包与跨机 | 待实现/验收 |

### 本轮运行记录

待执行第 0 步连接检查后填写；未通过的检查不标为完成。
