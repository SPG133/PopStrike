#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PSConnectionSubsystem.generated.h"

class APlayerController;
class UNetDriver;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSConnectionError, FText, ErrorMessage);

UCLASS(BlueprintType)
class POPSTRIKE_API UPSConnectionSubsystem
    : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPROPERTY(Transient)
    TArray<uint8> AvatarBytes;

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void Connect(
        APlayerController* PlayerController,
        const FString& ServerAddress);

    UPROPERTY(BlueprintAssignable, Category="PopStrike|Connection")
    FPSConnectionError OnConnectionError;


    void SetPlayerName(
        const FString& InPlayerName){
            Playername = InPlayerName;
        }

    const FString& GetPlayerName() const{
        return  Playername;
    }

private:

    void HandleNetworkFailure(
        UWorld* World, UNetDriver*, ENetworkFailure::Type, const FString& Error);

    FString Playername;
};
