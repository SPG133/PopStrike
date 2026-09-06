# 验证记录 — 2026-09-06

环境：Windows 10、Visual Studio 2022、UE 5.8.2 源码版；目标为 **PopStrikeEditor / Win64 / Development**。

| 检查 | 结果 | 证据 |
| --- | --- | --- |
| C++、UHT 与编辑器 Python 桥接模块编译 | 通过 | UnrealBuildTool `Result: Succeeded` |
| 读取当前主菜单、大厅、玩家槽控件树 | 通过 | `Saved/UIBlueprintReport.txt`，初始父类均为 UserWidget |
| 三个蓝图父类、BindWidget、事件图编译保存 | 通过 | `Saved/Logs/UIBuild.log`：0 errors；包含 8 个按钮事件 |
| 重复运行生成脚本 | 通过 | 核对现有事件连接，无重复节点；`Saved/UIBuildReport.txt` |
| 地址、异常头像、地图索引自动化测试 | 通过 | `Saved/Logs/NetworkAutomation.log`：`PopStrike.Network.InputBoundaries` Success |
| 本地专用服务器与两个客户端进入大厅 | 通过 | `Saved/NetworkSmokeResults.json`；各客户端记录 `Load map complete /Game/UI/Maps/L_Lobby` |
| 第二客户端 150 ms 延迟 / 5% 丢包 | 通过连接测试 | 日志确认 `PktLag set to 150`、`PktLoss set to 5`，成功加载大厅 |
| 服务器进程意外结束 | 通过 | 两客户端约 60 秒后均加载 L_MainMenu；无 Fatal error / Accessed None / UI 焦点错误 |

UI 蓝图初始备份：`Saved/UIBackups/20260906_231008/`。后续生成分别备份，均只保存在本机。

## 验证边界

- 只构建了编辑器目标，没有声称完成独立 Client/Server/Shipping 打包。
- 进程测试使用 NullRHI，验证执行链和连接恢复；未进行屏幕渲染截图验收。
- 未进行真实公网、NAT/防火墙、多地域、长时间断网、六人同时操作或恶意客户端压力测试。
- 20 秒连接看门狗、失败后按钮恢复、满员并发、房主变更、请求限频和切图失败恢复已实现；本次进程测试没有逐一自动点击验证这些交互。
- 项目没有 `/Game/Maps/L_MatchTest`、`L_Factory`、`L_Rooftop` 以及对应预览图。UI 会显示预览占位，开局会提示地图缺失；实际对战切图无法验收。
- 大厅/菜单没有 Pawn 和 PlayerStart，日志存在相关非致命警告；本次未创建游戏关卡或角色。

完整原始日志留在 `Saved/Logs/`，不上传包含本机环境信息的日志和缓存。
