# PopStrike

Unreal Engine **5.8.2** 源码版项目，包含主菜单、6 人大厅、玩家头像及网络 UI。

## 编译与运行

使用 Visual Studio 2022 C++ 工具链和 Windows SDK。其他机器先为 `PopStrike.uproject` 选择本机的 UE 5.8 引擎；项目的 EngineAssociation 是原开发机源码引擎 GUID。

```powershell
$UEPath = 'D:/UnrealEngine-release'
$ProjectPath = Join-Path (Get-Location) 'PopStrike.uproject'
& "$UEPath/Engine/Build/BatchFiles/Build.bat" PopStrikeEditor Win64 Development "-Project=$ProjectPath" -WaitMutex -NoHotReloadFromIDE
& "$UEPath/Engine/Binaries/Win64/UnrealEditor.exe" $ProjectPath
```

默认客户端地图为 `/Game/UI/Maps/L_MainMenu`。用编辑器构建运行本地专用服务器：

```powershell
& "$UEPath/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" $ProjectPath /Game/UI/Maps/L_Lobby -server -port=7777 -log
```

在主菜单输入玩家名和 `127.0.0.1:7777` 连接。服务器逻辑保留在权威端，UI 通过所属 PlayerController 的可靠 RPC 请求加入阵营、切图和开局。

## Python 事件图生成

已提交的三个 UI 蓝图已生成并编译，可直接使用。修改布局后需要重新生成时，关闭正在运行的编辑器，执行：

```powershell
$ScriptPath = Join-Path (Get-Location) 'Scripts/build_ui_events.py'
& "$UEPath/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" $ProjectPath -run=pythonscript "-script=$ScriptPath" -unattended -nullrhi -nosound
```

脚本通过仅编辑器模块 `PopStrikeEditor` 创建实际 K2 事件节点；游戏运行不依赖 Python。生成前备份到 `Saved/UIBackups/<时间>/`。已有生成事件会核对连接，不重复创建；发现用户自定义按钮事件时停止，不覆盖。

| 蓝图 | 父类 | 事件 |
| --- | --- | --- |
| WBP_MainMenu | PSMainMenuWidget | 开始连接、上传头像 |
| WBP_Lobby | PSLobbyWidget | 加入 A/B 队、上一张/下一张地图、开始、离开 |
| WBP_PlayerSlot | PSPlayerSlotWidget | 由大厅同步玩家名、头像、阵营和房主标记 |

保留现有设计树；主菜单使用最新版 `Txt_AvatarHint`。大厅补充 `Txt_RoomStatusHint`、`Txt_MapHostOnly`。两张 UI 地图分别使用 PSMenuGameMode、PSLobbyGameMode。

`Scripts/inspect_ui.py` 可导出控件与事件图清单。`Scripts/Archive/build_lobby_ui.py` 是原始布局生成脚本，仅归档，不要用它覆盖当前最新版 UI。

## 网络处理

- 主菜单只接受 IPv4 或 DNS 主机名及可选的 1–65535 端口，拒绝地图路径、URL travel 参数和非法地址。暂不支持 IPv6。
- 连接中禁用重复提交，20 秒未完成时取消待连接并恢复输入；失败提示、玩家名及头像保留，支持手动重试。
- 网络失败按 GameInstance 隔离；大厅掉线后返回本地主菜单。
- 大厅在 GameState 晚到或更换后重新绑定，并每 0.5 秒兜底刷新；销毁时清理计时器和委托。
- 服务端检查房主权限、每队 3 人、总人数 6 人；PreLogin 与 PostLogin 双重检查容量。开局期间拒绝新玩家及重复操作。
- 阵营/切图/开局请求在服务端限频；玩家资料每个连接只接收一次。头像限制 12 KiB、128×128 JPEG，并验证可解码后再复制给其他玩家。
- 开局前检查地图存在，处理 ServerTravel 立即失败及异步失败并解除锁定。

这不是完整的断线重连/会话迁移系统：重新连接是新玩家；专用服务器保持运行时，大厅房主由当前最早加入者确定。服务器进程消失后不会迁移服务器。

## 验证

自动化测试：

```powershell
& "$UEPath/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" $ProjectPath -unattended -nullrhi -nosound '-ExecCmds=Automation RunTests PopStrike.Network; Quit' '-TestExit=Automation Test Queue Empty'
python Scripts/network_smoke.py --engine $UEPath
```

`PopStrike.Network.InputBoundaries` 检查有效/无效地址、异常/超限头像和地图索引边界。进程测试启动一个本地专用服务器和两个客户端，第二客户端使用 150 ms 延迟、5% 丢包参数；验证进入大厅，并结束测试服务器验证客户端回到主菜单。只结束脚本自身启动的进程。

本次验证结果及未覆盖项见 `Docs/Verification.md`。

## 仓库

仅跟踪源码、配置、资产和脚本。编译结果、VS 数据库、DerivedDataCache、Intermediate、Saved、本机代理配置和资产备份不上传。Android File Server 当前禁用，仓库不保存其访问令牌。

首次云端整理前的本地历史保留在 `local-history-before-cloud-20260906` 分支，该分支包含旧缓存，不应推送。
