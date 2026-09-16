#include "PSConnectionSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void UPSConnectionSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
    Super::Initialize(Collection);

    if (GEngine)
    {
        GEngine->OnNetworkFailure().AddUObject(this, &ThisClass::HandleNetworkFailure);
    }
}

void UPSConnectionSubsystem::Deinitialize()
{
    if (GEngine)
    {
        GEngine->OnNetworkFailure().RemoveAll(this);
    }

    Super::Deinitialize();
}

void UPSConnectionSubsystem::Connect(
    APlayerController *PlayerController,
    const FString &ServerAddress)
{
    if (!PlayerController)
    {
        return;
    }

    PlayerController->ClientTravel(ServerAddress, ETravelType::TRAVEL_Absolute);
}

void UPSConnectionSubsystem::HandleNetworkFailure(
    UWorld *World,
    UNetDriver *,
    ENetworkFailure::Type,
    const FString &Error)
{
    if (World && World->GetGameInstance() != GetGameInstance())
    {
        return;
    }

    const FString Message = Error.IsEmpty()
                                ? TEXT("无法连接到服务器。")
                                : FString::Printf(TEXT("无法连接到服务器：%s"), *Error);
    OnConnectionError.Broadcast(FText::FromString(Message));
    UE_LOG(LogTemp, Warning, TEXT("HandleNetworkFailure: %s"), *Error);
}
