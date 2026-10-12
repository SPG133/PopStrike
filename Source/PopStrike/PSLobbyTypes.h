#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PSLobbyTypes.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FPSPlayerAppearance
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Appearance")
    FColor PrimaryColor = FColor(48, 112, 224);

    UPROPERTY(BlueprintReadOnly, Category="Appearance")
    FColor SecondaryColor = FColor(240, 96, 64);
};

UENUM(BlueprintType)
enum class EPSLobbyTeam : uint8
{
    None,
    A,
    B
};

USTRUCT(BlueprintType)
struct FPSLobbyMap
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Name;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Description;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UWorld> Level;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UTexture2D> Preview;
};

// 大厅地图的数据资产版本：内容浏览器右键 → Miscellaneous → Data Asset 创建，
// 让"加地图"变成纯资产操作，不用改 GameMode 代码
UCLASS(BlueprintType)
class POPSTRIKE_API UDA_LobbyMap : public UDataAsset
{
    GENERATED_BODY()

public:
    // 硬引用：随资产加载，蓝图拿到直接 Set Brush 显示，不用手动 LoadSynchronous
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<TObjectPtr<UTexture2D>> Preview;
};
