# 武器基类接线

本目录对应 C++ Classes / PopStrike / level3 / weapon，不是 Content 中的资产目录。
原生父类为 `PSWeaponBase`，配置资产类型为 `PSWeaponDefinition`，配置结构为 `PSWeaponData`。
只增加这些新类型，不改动当前角色、地图、输入或现有蓝图；旧 Tools 文档中提到的 `PSWeapon` 不是本次父类。

## 数据

武器蓝图 Class Defaults 的 WeaponSlot 设置 Primary（主武器）或 Secondary（副武器），默认 Primary。
蓝图可读取 WeaponSlot 并使用 Switch on EPSWeaponSlot。它是武器类的静态配置，不在运行时修改，也不单独复制；本字段不代表已经实现双槽装备管理。

在内容浏览器创建 Data Asset，选择 PSWeaponDefinition。再创建继承 PSWeaponBase 的蓝图，
在 Class Defaults 的 Definition 引用该资产。相同机制的新武器主要通过数据资产配置。
Definition 在实例生命周期内保持不变；它随武器蓝图的资源引用加载/打包，不把整套资源通过网络传输。
所有资源默认空，必须选用真实资产；未选择配置时不能开火或换弹。

代码维护入口：Equip/Unequip 共用 SetHolder；Reload/CancelReload 共用 SetReloading，
内部只使用一个 Reliable 换弹 RPC。换弹完成也通过 SetReloading(false) 结束。
修改持有者或换弹状态时集中修改这两个入口；蓝图公开函数、Event 和数据字段保持原有名称与类型。

- MagazineCapacity：单个弹匣容量。
- SpareMagazineCount：初始备用弹匣数量，不含装在枪里的一个。实际备弹为容量 × 数量。
- Reserve 为共享备弹池，战术换弹保留剩余子弹；不是逐个弹匣库存。若以后需要实体弹匣玩法再独立扩展。
- FireRateRPM：每分钟射击次数，服务器间隔为 60 / RPM。
- Damage、Range：供服务器蓝图射线/伤害逻辑读取，基类自身不执行命中判定。
- ReloadDuration：服务器换弹时间；动画 Notify 不得扣弹、补弹或决定完成时间。
- WeaponMesh / WeaponAnimClass：世界武器模型和 AnimBP。
- WeaponAnimations、CharacterAnimations、FirstPersonAnimations：各自的开火、换弹、装备、收枪 Montage。
- CharacterAnimationLayer：可选角色 Linked Anim Layer；仅提供资源入口，蓝图自行链接。
- HandSocket / MuzzleSocket / GripTransform：真实插槽及握持偏移，默认不猜测名称。
- Icon、FireSound、ReloadSound、MuzzleEffect：UI、声音和 Cascade 粒子资源；其他表现资源可在子蓝图添加。

角色、第一人称手臂和武器 Montage 必须分别兼容对应 Skeleton，并在 AnimGraph 配置正确 Slot。
没有通用结构能够预先覆盖所有未来武器机制；本结构只集中当前通用配置，不预建 GAS、投射物、回溯或库存系统。

## 蓝图事件

1. **服务器装备逻辑**：在服务器 Spawn 武器蓝图，再调用 Equip(角色 Pawn)。客户端不能通过此入口自行拾取。
   在 OnServerHolderChanged 中：NewHolder 有效时，Attach Actor To Component 到角色世界 Mesh，
   使用 HandSocket 与 GripTransform；为空时 Detach。此前 Holder 非空时也要清理旧持有者的表现/引用。
   该事件在 Dedicated Server 上也执行，使附件状态由服务器复制。死亡/切枪/丢弃时由装备逻辑调用 Unequip。
   基类并不裁定拾取距离、槽位、死亡或两把枪是否可同时使用。
2. **输入**：本地所属角色获取自己的武器引用，按下时调用 Fire 或 Reload，必要时 CancelReload。
   Fire 一次请求一次射击；不要在 Tick 连续调用 Reliable 的换弹/取消入口。
   所有客户端请求必须经已被服务器设置 Owner 的武器发送，等待 Holder/Owner 同步后才可调用。
3. **CanUseWeapon Override**：返回服务器确认的“角色存活且此武器为当前槽位”等条件。
   保持纯查询，不写状态、不播放表现。默认返回 true，仅表示基类未添加额外比赛规则。
4. **OnServerFire**：服务器验证并扣弹后执行。在这里做服务器射线/生成投射物/伤害。
   起点、方向和持有状态来自服务器验证的数据，不接受客户端直接提交命中结果或伤害值。
   这个 Event 不是完整命中系统，也没有历史回溯。
5. **OnFireEffects**：每个相关客户端及 Listen Server 本地各执行一次，Dedicated Server 跳过。
   用 Definition.Data 的 WeaponAnimations.Fire、CharacterAnimations.Fire 播放对应 Montage；
   仅本地所属角色播放 FirstPersonAnimations.Fire、相机后坐力等；在此播放枪口特效和一次性声音。
   不要再 Multicast 此 Event，不修改 Ammo，也不让服务器伤害依赖它。
6. **OnWeaponStateChanged**：按快照刷新装备外观/动画层；这不是“一次性动作通知”。
   Holder 有效、bReloading 为 true 且 ElapsedSeconds < Duration 时播放或定位换弹动画：
   Montage 的播放速率设为 MontageLength / Duration，开始位置设为 ElapsedSeconds × 播放速率。
   bReloading 为 false 时停止/清理旧换弹动画。先检查动画资源有效性。
   同一个 ActionSequence 可能因 Owner/引用解析/显式 Refresh 被调用多次，应重复刷新但避免重复声音、重启 Montage。
   初次收到正在换弹的快照只恢复进度，不重播开始音效；Elapsed 达到 Duration 时不要重播过期动画。
   Equip/Unequip Montage 由蓝图基于 Holder 转换播放；初次重建外观无需重播收枪/拔枪。
7. **OnAmmoChanged**：更新拥有者 UI。其它客户端不收到 Loaded/Reserve 新数据；不要在全局 UI/GameState 转播。

角色、网格或 AnimInstance 尚未就绪时先等待，准备好后调用 RefreshPresentation。
它同时从 BeginPlay、WeaponState RepNotify 和 Owner RepNotify 触发；Holder 的初始空值不是丢弃通知。
初始服务器时间尚未同步时也应在准备好后重新刷新。第一人称 Mesh、拥有者显示规则由角色蓝图配置。
装备事件的附着只是世界武器，第一人称表现可使用角色已有的本地网格；不要从客户端创建权威武器。

## 网络边界

| 数据/行为 | 权威写入者 | 接收者与恢复 |
| --- | --- | --- |
| Definition | 蓝图默认配置，运行时不改 | 同一个武器类的本地资产引用 |
| Ammo | 服务器初始化、开火扣弹、换弹补弹 | OwnerOnly 当前快照；丢包后后续复制恢复 |
| Holder / 换弹状态 / 序号 / 起始时间 / 持续时间 | 服务器 Equip/Unequip/Reload/Cancel/计时器 | 相关客户端快照；晚加入按服务器时间定位进度 |
| 单次开火意图 | 所属客户端请求，服务器检查 | Unreliable RPC；丢包可能少打一发，不能据此做自动连射 |
| 开火表现 | 服务器确认后发送 | Unreliable Multicast；丢包允许漏一次表现，不影响弹药/伤害；过期 0.5 秒不播放 |
| 换弹/取消意图 | 所属客户端请求，服务器检查 | Reliable RPC；重复请求不重置正在进行的换弹 |

EquipSequence 拒绝先前装备周期的延迟请求；开火请求序号拒绝重复和乱序，服务器冷却阻止超射速。
武器随 Owner relevancy，未实现迷雾；必须和角色/附件的后续信息策略一起验收。
本地开火动画目前等待服务器确认，有 RTT 延迟；未提供预测、命中确认、自动连射、生命状态或完整装备组件。
单独的 CanFire 在非拥有者客户端因没有私有弹药不能作为远端表现依据。

## 验证步骤（需接线后执行）

Dedicated Server + 两个独立客户端，配置真实资源并按上述方式接线：

- 两端看到世界装备，仅拥有者看到私有弹药和第一人称表现。
- 超过 RPM 频率调用 Fire：服务器不会超频扣弹；空弹匣/换弹中不执行 OnServerFire。
- 换弹完成只转移所需备弹；取消、切枪、死亡期间取消后不补弹；死亡必须接线到 Unequip/CanUseWeapon。
- 换弹中晚加入/重新相关：恢复剩余进度，不从头开始、不重复补弹或播放开始音效。
- 切换持有者再开火：旧装备周期请求不能作用到新持有者。
- 丢包和 100ms RTT：弹药最终由快照恢复；允许单发请求或开火表现丢失，记录当前确认延迟。

C++ 构建不代表上述联网、动画兼容和打包验收通过。

## 方案来源

- [Epic UE 5.8 属性复制](https://dev.epicgames.com/documentation/en-us/unreal-engine/replicate-actor-properties-in-unreal-engine)
- [Epic UE 5.8 RPC 与可靠性](https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-procedure-calls-in-unreal-engine)
- [Epic UE 5.8 Actor Owner 与所属连接](https://dev.epicgames.com/documentation/en-us/unreal-engine/actor-owner-and-owning-connection-in-unreal-engine)
- 本机 UE 5.8.2：Actor.h 的 OnRep_Owner、GameStateBase.h 的 GetServerWorldTimeSeconds。
