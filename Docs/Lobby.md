# 大厅蓝图连接

C++ 只维护网络数据：玩家资料、头像字节、队伍、房主、地图索引和开局。`WBP_Lobby` 继承 `PSLobbyWidget`，`WBP_PlayerSlot` 继承 `PSPlayerSlotWidget`；具体显示和按钮行为仍由蓝图负责。

源码按职责分为 `PSLobbyTypes`、`PSLobbyPlayerState`、`PSLobbyGameState`、`PSLobbyPlayerController`、`PSLobbyGameMode`、`PSLobbyWidget`、`PSPlayerSlotWidget`，每个类独立文件。

## WBP_Lobby

### Construct

1. `Get Game State` → `Cast to PSLobbyGameState`，保存为变量 `LobbyState`。
2. `LobbyState.OnChanged` → `Bind Event`，绑定到自定义事件 `RefreshLobby`。
3. `Get Owning Player` → `Cast to PSLobbyPlayerController`，保存为变量 `LobbyController`。
4. `LobbyController.OnLobbyError` → `Bind Event`，把 `Message` 显示到你的提示 Text。
5. 调用一次 `RefreshLobby`。

### RefreshLobby

队伍：

- `LobbyState.GetTeamPlayers(A)` 得到 A 队数组；逐个调用 `Slot_A1..A3` 中自定义的 `SetPlayer`，没有玩家的位置传空。
- `LobbyState.GetTeamPlayers(B)` 同理更新 `Slot_B1..B3`。

地图：

- `Maps.Length > 0` 后，`LobbyState.Maps` → `Get(LobbyState.SelectedMap)`。
- 用返回结构的 `Name / Description / Preview` 更新你的文字和图片。
- `LobbyController.IsRoomHost` 控制上一张、下一张和开始按钮是否可用。
- `LobbyState.bStarting` 为 true 时禁用所有操作按钮。

按钮：

| 控件 | 蓝图调用 |
|---|---|
| `Btn_JoinTeamA` | `LobbyController.ServerJoinTeam(A)` |
| `Btn_JoinTeamB` | `LobbyController.ServerJoinTeam(B)` |
| `Btn_MapPrev` | `(SelectedMap - 1 + Maps.Length) % Maps.Length` → `ServerSelectMap` |
| `Btn_MapNext` | `(SelectedMap + 1) % Maps.Length` → `ServerSelectMap` |
| `Btn_StartGame` | `LobbyController.ServerStartGame` |
| `Btn_LeaveRoom` | `Open Level /Game/Maps/Level_0/L_MainMenu` |

创建大厅 Widget、`Add to Viewport`、鼠标显示和输入模式也放在 `L_Lobby` 关卡蓝图或你自己的 HUD/Controller 蓝图里。C++ 不再自动创建界面。

## WBP_PlayerSlot

创建函数 `SetPlayer`：

- 输入 `Player`，类型 `PSLobbyPlayerState Object Reference`。
- `Is Valid(Player)` 为 false：显示“空位”，清空头像和房主标记。
- 为 true：`Player.GetPlayerName` 更新名字，`Player.GetAvatar` 更新头像。
- `Get Game State` → `Cast PSLobbyGameState` → 比较 `Host == Player`，决定是否显示房主标记。

## 地图与 GameMode

`L_Lobby` 的 World Settings → GameMode Override 使用 `PSLobbyGameMode`。现在 C++ 默认提供果汁工厂一张地图。

要让地图列表可在编辑器配置，可建一个继承 `PSLobbyGameMode` 的蓝图 GameMode，在 `Available Maps` 中添加 `Name / Description / Level / Preview`，再把大厅关卡的 Override 改为该蓝图。

比赛地图必须使用自己的比赛 GameMode，不能继续使用 `PSLobbyGameMode`。

## 主菜单与头像

- 头像按钮 → `Select Avatar`；`On Avatar Changed` 中自行设置头像 Brush。
- 加入按钮 → `Connect To Server(Address, PlayerName)`；返回字符串非空就显示错误。
- `PSConnectionSubsystem.OnConnectionError` 可显示房满、超时或拒绝信息。

头像在客户端压为 128×128 JPEG（最多 16KB），连接大厅后自动上传；服务器校验后存入 `PSLobbyPlayerState` 并复制给所有客户端。客户端文件路径不会上传。

连接地址示例：`127.0.0.1:7777`。
