#include "PSConnectionSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PSGameConfig.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Misc/Char.h"

void UPSConnectionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    if (GEngine)
    {
        GEngine->OnNetworkFailure().AddUObject(this, &ThisClass::OnNetworkFailure);
        GEngine->OnTravelFailure().AddUObject(this, &ThisClass::OnTravelFailure);
    }
}

void UPSConnectionSubsystem::Deinitialize()
{
    if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(ConnectionTimer);
    if (GEngine)
    {
        GEngine->OnNetworkFailure().RemoveAll(this);
        GEngine->OnTravelFailure().RemoveAll(this);
    }
    Super::Deinitialize();
}

FString UPSConnectionSubsystem::ConsumeError()
{
    FString Result = MoveTemp(LastError);
    LastError.Reset();
    return Result;
}

void UPSConnectionSubsystem::OnNetworkFailure(UWorld* World, UNetDriver*, ENetworkFailure::Type, const FString& Error)
{
    ReturnToMenu(World, Error.IsEmpty() ? TEXT("连接服务器失败。") : Error);
}

void UPSConnectionSubsystem::OnTravelFailure(UWorld* World, ETravelFailure::Type, const FString& Error)
{
    ReturnToMenu(World, Error.IsEmpty() ? TEXT("地图切换失败。") : Error);
}

void UPSConnectionSubsystem::ReturnToMenu(UWorld* World, FString Error)
{
    if (!World || World->GetGameInstance() != GetGameInstance() || World->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    LastError = MoveTemp(Error);
    bConnecting = false;
    World->GetTimerManager().ClearTimer(ConnectionTimer);
    if (GEngine) GEngine->CancelPending(World);
    OnConnectionChanged.Broadcast();
    if (bReturningToMenu || World->GetMapName().Contains(TEXT("L_MainMenu")))
    {
        return;
    }

    bReturningToMenu = true;
    World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]
    {
        bReturningToMenu = false;
        if (UWorld* World = GetWorld())
        {
            UGameplayStatics::OpenLevel(World, FName(PSConfig::MainMenuMap));
        }
    }));
}


bool UPSConnectionSubsystem::IsValidAddress(const FString& Address)
{
    // Direct host[:port] only. Reject travel options, local maps and URL injection.
    if (Address.IsEmpty() || Address.Len() > 253) return false;
    FString Host, Port;
    if (!Address.Split(TEXT(":"), &Host, &Port)) Host = Address;
    else
    {
        if (Port.IsEmpty() || Port.Len() > 5 || !Port.IsNumeric()) return false;
        for (TCHAR C : Port) if (C < '0' || C > '9') return false;
        const int32 Number = FCString::Atoi(*Port);
        if (Number < 1 || Number > 65535) return false;
    }
    if (Host.IsEmpty() || Host.StartsWith(TEXT(".")) || Host.EndsWith(TEXT("."))) return false;
    TArray<FString> Labels;
    Host.ParseIntoArray(Labels, TEXT("."), false);
    bool bNumeric = true;
    for (const FString& Label : Labels)
    {
        if (Label.IsEmpty() || Label.Len() > 63 || Label.StartsWith(TEXT("-")) || Label.EndsWith(TEXT("-"))) return false;
        for (TCHAR C : Label)
        {
            const bool bDigit = C >= '0' && C <= '9';
            bNumeric &= bDigit;
            if (!bDigit && !(C >= 'a' && C <= 'z') && !(C >= 'A' && C <= 'Z') && C != '-') return false;
        }
    }
    if (bNumeric)
    {
        if (Labels.Num() != 4) return false;
        for (const FString& Label : Labels) if (Label.Len() > 3 || FCString::Atoi(*Label) > 255) return false;
    }
    return true;
}

bool UPSConnectionSubsystem::BeginConnection(APlayerController* PC, const FString& Address)
{
    if (!PC || !PC->IsLocalController() || bConnecting || bReturningToMenu || !IsValidAddress(Address)) return false;
    LastError.Reset();
    ServerAddress = Address;
    bConnecting = true;
    OnConnectionChanged.Broadcast();
    GetWorld()->GetTimerManager().SetTimer(ConnectionTimer, FTimerDelegate::CreateWeakLambda(this, [this]
    {
        ReturnToMenu(GetWorld(), TEXT("Connection timed out. Please check the address and retry."));
    }), 20.0f, false);
    PC->ClientTravel(Address, TRAVEL_Absolute);
    return true;
}

void UPSConnectionSubsystem::MarkConnected()
{
    bConnecting = false;
    bReturningToMenu = false;
    LastError.Reset();
    if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(ConnectionTimer);
    OnConnectionChanged.Broadcast();
}
