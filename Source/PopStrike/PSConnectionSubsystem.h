#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PSConnectionSubsystem.generated.h"

class UNetDriver;
class APlayerController;
DECLARE_MULTICAST_DELEGATE(FPSConnectionChanged);

UCLASS()
class POPSTRIKE_API UPSConnectionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void SetPlayerName(const FString& Value) { PlayerName = Value; }
    const FString& GetPlayerName() const { return PlayerName; }

    void SetAvatarBytes(TArray<uint8> Value) { AvatarBytes = MoveTemp(Value); }
    const TArray<uint8>& GetAvatarBytes() const { return AvatarBytes; }

    void SetServerAddress(const FString& Value) { ServerAddress = Value; }
    const FString& GetServerAddress() const { return ServerAddress; }

    FString ConsumeError();
    bool BeginConnection(APlayerController* PC, const FString& Address);
    void MarkConnected();
    bool IsConnecting() const { return bConnecting; }
    static bool IsValidAddress(const FString& Address);
    FPSConnectionChanged OnConnectionChanged;

private:
    void OnNetworkFailure(UWorld* World, UNetDriver*, ENetworkFailure::Type, const FString& Error);
    void OnTravelFailure(UWorld* World, ETravelFailure::Type, const FString& Error);
    void ReturnToMenu(UWorld* World, FString Error);

    FString PlayerName;
    TArray<uint8> AvatarBytes;
    FString ServerAddress = TEXT("127.0.0.1:7777");
    FString LastError;
    bool bReturningToMenu = false;
    bool bConnecting = false;
    FTimerHandle ConnectionTimer;
};
