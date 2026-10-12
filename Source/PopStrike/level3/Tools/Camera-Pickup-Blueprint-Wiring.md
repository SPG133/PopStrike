# BP_PSEquipmentComponent：缓存摄像机并检测拾取

目标资产：`/Game/PopStrike/Blueprints/Component/BP_PSEquipmentComponent`。
已通过 MCP 编辑并保存该蓝图，完成 BeginPlay 缓存和现有 TryPickUp 的摄像机射线检测；蓝图编译通过，未启动 PIE 或联网运行测试。
用户已明确允许 MCP 编辑和蓝图自动骨架编译；没有关闭 UE、编译 C++、Live Coding 或打包。

## 组件变量（均不复制）

- `CachedPawn`：Pawn Object Reference。
- `CachedCamera`：Camera Component Object Reference。
检测距离暂为节点中的 250 cm 常量，可直接修改乘法节点连接的数值。

## Event BeginPlay

1. Get Owner → Cast To Pawn。
2. 转换成功后 Set CachedPawn。
3. CachedPawn → Get Component by Class，Component Class 选择 CameraComponent。
4. 将返回的 Camera Component 引用赋给 CachedCamera。Get Component by Class 的输出根据所选类确定类型，不需要额外 Cast。

相机引用来自所属角色，不使用 PlayerController、CameraManager 或接口。
该接法要求角色只有一个用于交互的 Camera Component；多相机时要按实际活动相机选择，不能默认第一个。

## 现有 TryPickUp

将原本每次调用的 Get Owner / Cast To Pawn 替换为 CachedPawn；保留本地控制判断。

1. Is Valid(CachedPawn) → Is Locally Controlled → Branch，False 不执行。
2. Is Valid(CachedCamera)，无有效相机则不执行。
3. CachedCamera → Get World Location，作为 Start。
4. CachedCamera → Get Forward Vector，乘 250，再加 Start，作为 End。
5. Line Trace By Channel：Visibility、Trace Complex=false、Actors to Ignore=[CachedPawn]、Draw Debug Type=None。
6. Return Value=true → Break Hit Result → Hit Actor → Cast To PSWeaponBase。
7. 武器转换成功 → 调用继承的 PickupWeapon，Target=Self，Weapon=命中的武器。

角色的拾取输入 Started 只调用组件 TryPickUp。不增加 Tick 射线，不缓存每帧变化的相机位置或方向。
武器自身的交互碰撞必须启用查询并阻挡 Visibility。
武器类型检查发生在命中后；相机与角色的转换只在 BeginPlay 做一次。

## 网络边界与验证

射线用于本地选中目标。OnPickupRequested 在服务器触发后，仍须在组件蓝图验证距离、遮挡和角色状态，再调用 EquipWeapon；此文件不代替这些已有联网接线。
验证摄像机旋转时射线方向正确、不会命中自己、空命中不发送请求、其他角色组件不执行本地检测。

## 修改与验证记录

只修改 BP_PSEquipmentComponent 的上述两个图和新增两个缓存变量，保留原有空 Tick 事件，不在 Tick 执行检测。
核对了实际引脚：乘法是 Vector × 浮点数，HitActor 接入武器转换，ActorsToIgnore 包含 CachedPawn，拾取函数内没有重复 Pawn 转换。
编辑前的资产及当时两个图的快照位于 Saved/Backups/EquipmentCamera-20261012-193635。
组件只实现本地检测并发送既有拾取请求。角色输入调用、地面武器碰撞和服务器拾取校验仍需用户接线及运行验证。
