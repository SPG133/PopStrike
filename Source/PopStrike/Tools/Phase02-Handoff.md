# 阶段 0–2 交接（2026-09-22）

本轮未运行 C++/蓝图编译、PIE、自动化或网络测试。以下为源码与 MCP 资产检查结果，不是运行验收。

## 已确认及修改

- 本机 Build.version 为 UE 5.8.2。
- 用户日志中的两个错误已有源码修正：PopStrike.Build.cs 添加模块本地头文件路径；PSMatchGameMode.cpp 使用 TObjectPtr 遍历 PlayerArray。本轮保留这些已有修改。
- MCP 已能发现 PSFPSCharacter、PSMatchPlayerController、PSWeapon 等原生类；不能据此认定当前 DLL 包含最新源码。
- 当前编辑器 net.Iris.UseIrisReplication=0；实际独立服务器后端及性能仍待运行记录。
- 原始 ABP_Mannequin_Base 的注册表父类为 /Script/LyraGame.LyraAnimInstance，不能直接用作本项目角色动画蓝图。
- SK_Rifle 包含 Muzzle、ShellEject、ShellEject_0 插槽。
- MCP 创建并保存 /Game/PopStrike/Data/DA_Rifle，Mesh 指向真实的 /Game/Weapons/Rifle/Mesh/SK_Rifle。沿用原生默认参数：30 发弹匣、90 发备弹、0.1 秒射击间隔。握持与第一人称偏移尚未调校，武器表现蓝图尚未接入。
- PSFPSCharacter.cpp 统一 BaseEyeHeight 与初始 Camera 高度为 64，避免初始客户端视点与服务器 GetPawnViewLocation 高度不一致。

## 用户执行：先构建

保存工作并自行关闭编辑器，释放模块 DLL；在 PowerShell 执行：

```powershell
& 'D:\UnrealEngine-release\Engine\Build\BatchFiles\Build.bat' `
  PopStrikeEditor Win64 Development `
  '-Project=D:\Unreal Projects\PopStrike\PopStrike.uproject' `
  -WaitMutex -NoHotReloadFromIDE -NoLiveCoding
```

需看到最终 Result: Succeeded。单个 Compile action 成功不代表链接和整体构建成功。

## 用户执行：创建需要隐式编译的蓝图壳

构建成功后重新打开工程，在 Output Log 将命令模式切换为 Python，执行：

```python
exec(compile(open(r'D:\Unreal Projects\PopStrike\Source\PopStrike\Tools\Create-Phase02Blueprints.py', encoding='utf-8').read(), 'Create-Phase02Blueprints.py', 'exec'))
```

原因：本机 Kismet2.cpp 的 CreateBlueprint 内部直接调用 FBlueprintCompilationManager::CompileSynchronously；因此创建蓝图也交由用户操作。脚本只创建缺失蓝图并保存，保留同路径已有资产，不启动游戏或测试。脚本尚未执行，不能保证执行成功；报错时保留完整错误，勿删除已创建资产。

创建的四个普通蓝图为 BP_FPSCharacter、BP_MatchPlayerController、BP_MatchGameMode、BP_Rifle，动画蓝图为 ABP_FPSCharacter。它们只是供后续 MCP 编辑的壳，空 AnimGraph 不代表动画完成。

## 接下来仍须完成

1. MCP 配置角色网格、输入资产和映射、项目 AnimGraph、武器射击/换弹表现，以及 GameMode 默认类。
2. 配置现有果汁工厂地图与大厅旅行入口，核对出生点及身份/队伍交接。
3. 用户编译修改后的蓝图，再执行自动化与独立服务器双客户端验收。
4. 基线连接、服务器射线/伤害、换弹恢复、重复请求、弱网移动与连续射击均未在本轮测试；阶段 0–2 尚未验收完成。

后续基线连接测试可用现有脚本（只检查连接，不证明 FPS 闭环）：

```powershell
& 'D:\Unreal Projects\PopStrike\Scripts\Test-NetworkBaseline.ps1' `
  -EditorPath 'D:\UnrealEngine-release\Engine\Binaries\Win64\UnrealEditor.exe'
```

现有自动化用例名：PopStrike.Combat.AuthorityAndRecovery。待资产接线完成后由用户在 Session Frontend 的 Automation 面板运行；这不是网络多进程测试。

## 参考

- [UE5 迁移指南：TObjectPtr](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-migration-guide)
- [UE 5.8 属性复制](https://dev.epicgames.com/documentation/unreal-engine/replicate-actor-properties-in-unreal-engine)
- 本机 Engine/Source/Runtime/Engine/Private/Pawn.cpp：GetPawnViewLocation。
- 本机 Engine/Source/Editor/UnrealEd/Private/Kismet2/Kismet2.cpp：CreateBlueprint。
