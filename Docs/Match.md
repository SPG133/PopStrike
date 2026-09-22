# 比赛地图最小接入

`PSMatchGameMode` 只指定 `PSMatchPlayerState`，不提前实现尚未接入角色和交互的回合、计分、炸弹及击杀系统。

大厅通过无缝旅行把每名玩家的数据复制到对应的比赛 PlayerState：

- 名称：由 `APlayerState::CopyProperties` 复制。
- 队伍、头像：由 `APSLobbyPlayerState::CopyProperties` 复制。
- 同名玩家不参与身份匹配，因此不会互相覆盖。

比赛地图的 World Settings → GameMode Override 必须设为 `PSMatchGameMode` 或其蓝图子类。后续实现比赛玩法时，再按可验证阶段增加所需状态。
