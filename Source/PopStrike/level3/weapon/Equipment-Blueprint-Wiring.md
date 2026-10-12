# 装备组件蓝图接线

`PSFPSCharacter` 不再创建原生装备组件。创建继承 `PSEquipmentComponent` 的 `BP_EquipmentComponent`，然后在 `BP_Character` 的 Components 面板添加一个该组件。
确认组件的 Component Replicates 已开启。旧的原生 Equipment 成员已移除；已有节点如果引用它，改为引用手动添加的蓝图组件。
C++ 只维护服务器请求、持有者/Owner、当前武器复制及装备序号。
拾取检测、距离/遮挡/角色状态校验、附着、落点、碰撞和全部 IK 由蓝图实现。

## 组件 BP_EquipmentComponent

在组件自己的 BeginPlay 绑定自身的三个委托，并读取一次 GetCurrentWeapon 刷新当前表现。这里的 PickupWeapon、DropWeapon、EquipWeapon、UnequipWeapon 均以 Self 为目标。

- 创建组件的拾取处理函数：蓝图检测候选枪，调用自身 PickupWeapon(候选枪)。
- OnPickupRequested：只在服务器执行。重新检查距离、遮挡、角色是否允许拾取、请求频率；通过后调用 EquipWeapon。不要收到请求就无条件装备。
- 组件的丢弃处理函数：调用自身 DropWeapon。
- OnDropRequested：只在服务器执行。先计算并验证落点；成功后保存 Weapon 引用，调用 UnequipWeapon，成功返回后设置该 Weapon 的世界位置。无安全落点时不提交卸下。
- OnEquipmentChanged：在服务器和相关客户端通知当前状态。按需更新 UI/动画；Dedicated Server 跳过表现。

装备只通过组件的 EquipWeapon/UnequipWeapon 提交，不直接调用武器 Equip/Unequip，避免组件与武器状态分歧。
这两个提交接口有实际 HasAuthority 检查，客户端调用不会修改状态。
请求走玩家自己的复制组件，不向无 Owner 的地面武器发送 Server RPC。

## 角色 BP_Character

角色只把本地拾取/丢弃输入转发给手动添加的 BP_EquipmentComponent，装备规则写在组件自身。
AnimBP 或其他调用者可通过 Get Component by Class（PSEquipmentComponent）取得该蓝图子组件。每个角色只添加一个装备组件。

## 武器 BP_WeaponAK

- OnServerHolderChanged：NewHolder 有效时在服务器 Attach 到角色 Mesh 的真实 Socket；为空时 Detach，使用 Keep World Transform。
- 装备时关闭地面交互碰撞，丢弃时恢复；服务器和客户端都要从当前 WeaponState 恢复本地碰撞设置。
- OnWeaponStateChanged 是可重复调用的快照刷新入口。引用尚未就绪时等它就绪，再调用现有 RefreshPresentation；不要仅靠一次 Multicast 设置外观。
- 不销毁再生成武器，丢弃只修改同一 Actor 的持有状态与服务器位置。

武器已开启移动复制，持有时使用 Owner relevancy，卸下时清空 Owner 并恢复独立距离相关性。
普通地面武器并非全地图强制下发；如果需要全地图所有连接收到，可以在武器蓝图 Class Defaults 设置 Always Relevant，但持有时也会保持该设置。
角色销毁时组件只提交卸下，实际解除附着由 OnServerHolderChanged 蓝图处理；死亡时的落点和其他规则仍由角色蓝图决定。

## IK 和动画

这部分完全在现有 BP/AnimBP 实现；C++ 不计算 IK、不切动画层、不增加 IK 参数。
AnimBP 通过 Equipment.GetCurrentWeapon 获取当前枪。角色和武器快照可能先后到达，先检查引用及 Holder。
持枪切换 ABP_Rifle，空手切换现有空手层；左手目标、骨骼空间转换、Two Bone IK、肘部方向和 Alpha 插值自行在蓝图配置。
不要在 Blueprint Thread Safe Update Animation 中直接访问 Actor/Component；在普通 Update Animation 读取，再让动画图消费缓存值。

## 用户验证

未编译，由用户自行验证。尚未修改输入资产、武器蓝图或动画图。
完成接线后验证：双客户端争抢只有一人成功；不能隔墙/超距离拾取；丢弃后再次拾取；晚加入可恢复装备；延迟的旧丢弃请求不卸下新装备；角色/武器销毁后不残留错误引用。

职责划分参考 Epic 的 Lyra 装备组件，保留 PopStrike 已有武器 Actor，不引入 Lyra 库存或 GAS 装备实例：
https://dev.epicgames.com/documentation/en-us/unreal-engine/lyra-inventory-and-equipment-in-unreal-engine
https://dev.epicgames.com/documentation/unreal-engine/replicating-actor-components-in-unreal-engine
