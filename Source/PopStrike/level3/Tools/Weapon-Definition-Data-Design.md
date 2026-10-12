# 武器定义：GAS 与 ALS 数据边界

适用：PopStrike，本机 UE 5.8.2。此改动只提供数据配置，不安装 ALS，也不接入 GAS 运行流程。

## 配置所有权

`UPSWeaponDefinition` 保留原有 17 个字段，新增 4 个，共 21 个顶层字段：

| 字段 | 意义 | 未来消费者 |
| --- | --- | --- |
| WeaponTags | 武器的静态类别与特征 | 能力条件、装备规则 |
| GrantedAbilities | 能力类、输入标签、能力等级的授予记录 | 装备/能力输入系统 |
| DamageEffect | 伤害效果类；Damage 是现有基础数值 | 服务器伤害能力 |
| OverlayMode | 持枪姿态语义，独立于武器类别 | 角色动画表现适配器 |

`WeaponTags` 不保存正在开火、换弹或死亡等动态状态。`GrantedAbilities` 的输入标签
不会由原生 GAS 自动绑定；后续消费者负责输入路由、服务器授予、记录句柄和卸装撤销。
能力的 SourceObject 应指向具体武器实例，不能只指向共享 Definition。

## ALS 的数据分工

ALS V4 和 ALS Refactored 的具体实现不同。原版 OverlayState 使用枚举，Refactored 使用
Gameplay Tags 和可切换的 Linked Anim Layers。PopStrike 借鉴其数据边界，不依赖 ALS 类型。

- AnimationType：保留现有 EPSGun 和蓝图兼容语义，不扩充枚举来表达所有持枪姿态。
- OverlayMode：例如单手/双手手枪、步枪姿态；同类武器可以使用不同姿态，多把武器可以共享姿态。
- CharacterAnimationLayer：可选的具体角色动画层实现。标签表达语义，类引用表达实现；
  当前小规模直接由 Definition 配置类即可，不另建全局映射表来重复维护相同关系。
- CharacterFireMontage / CharacterReloadMontage：角色动作动画，保留现有配置入口。
- HandSocket / GripTransform：武器与角色的挂接，不承担角色骨骼分层或手部 IK 策略。

身体各部位的 Overlay、Additive、Slot 混合权重和局部/网格空间混合，以及手脚 IK、
骨骼遮罩和曲线名称，属于动画资产与角色动画系统。不要把这些数值复制进每个武器定义。
移动速度、Gait、Stance、RotationMode、当前动作和动作时间属于运行状态，不能存在共享定义上。

## 配置约束与兼容

新增字段默认空，已有资产无需重填；现有字段名称、类型、基类和资产路径保持不变。
本次没有新增动画资产、Gameplay Tags 字典条目、Ability、Effect 或武器名称分支。

OverlayMode 的标签选择器允许 Animation.Overlay 和 Als.OverlayMode 两个命名空间。
使用前必须在项目字典或实际 ALS 插件中注册真实标签；这只是筛选元数据，不自动注册标签。
空 OverlayMode 不改变当前 AnimationType 驱动的表现。未来接入消费者后，需要规定有效
标签的选择、缺失层时的基础姿势回退以及切枪/死亡/晚加入的恢复方式。

角色 Layer 和两个角色 Montage 必须兼容实际角色 Skeleton；武器 Mesh 使用另一套 Skeleton
是正常的，不应拿武器骨架验证角色动画。导入的 Lyra AnimLayers 也不能直接当作 ALS 层使用：
须核对父类、动画层接口、Skeleton、曲线和插件依赖。

DamageEffect 和 GrantedAbilities 当前是未消费的配置入口。现有 ApplyPointDamage、弹药与
换弹流程继续运行；未来 GAS 接入必须为伤害、弹药和动作各指定一个权威写入者，避免重复结算。

## 来源与验证边界

- 原版 ALS V4：https://www.fab.com/listings/ef9651a4-fb55-4866-a2d9-1b38b028f9c7
- ALS Refactored 作者说明：https://github.com/Sixze/ALS-Refactored/blob/main/README.md
- Overlay 标签定义：https://github.com/Sixze/ALS-Refactored/blob/main/Source/ALS/Public/Utility/AlsGameplayTags.h
- Layering 运行状态：https://github.com/Sixze/ALS-Refactored/blob/main/Source/ALS/Public/State/AlsLayeringState.h
- Epic Lyra 能力组织：https://dev.epicgames.com/documentation/en-us/unreal-engine/abilities-in-lyra-in-unreal-engine

第三方 ALS 资料用于数据设计参考，不表示该插件已通过 PopStrike UE 5.8.2 的兼容性验收。
本次只验证 C++/UHT 配置声明；动画、GAS 运行、网络与打包验收须在消费者接入后完成。

???????Dedicated Server + ????????????????/???????????????? IsCrouched????????????????????????????????????????????????
