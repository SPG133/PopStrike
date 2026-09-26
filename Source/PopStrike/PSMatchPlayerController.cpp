#include "PSMatchPlayerController.h"

#include "PSCombatComponent.h"
#include "PSFPSCharacter.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputActionValue.h"

void APSMatchPlayerController::BeginPlayingState()
{
    Super::BeginPlayingState();
    if (!IsLocalController()) return;
    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly());
    if (auto* Input = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()); Input && MatchMapping)
        Input->AddMappingContext(MatchMapping, 0);
    if (CombatHUDClass && !CombatHUD)
    {
        CombatHUD = CreateWidget<UUserWidget>(this, CombatHUDClass);
        if (CombatHUD) CombatHUD->AddToViewport();
    }
}

void APSMatchPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IsLocalController())
    {
        if (auto* Input = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()); Input && MatchMapping)
            Input->RemoveMappingContext(MatchMapping);
    }
    if (CombatHUD) CombatHUD->RemoveFromParent();
    Super::EndPlay(EndPlayReason);
}

void APSMatchPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    auto* Input = Cast<UEnhancedInputComponent>(InputComponent);
    if (!Input) return;
    if (MoveAction) Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
    if (LookAction) Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
    if (JumpAction)
    {
        Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::StartJump);
        Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::EndJump);
        Input->BindAction(JumpAction, ETriggerEvent::Canceled, this, &ThisClass::EndJump);
    }
    if (CrouchAction)
    {
        Input->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::StartCrouch);
        Input->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ThisClass::EndCrouch);
        Input->BindAction(CrouchAction, ETriggerEvent::Canceled, this, &ThisClass::EndCrouch);
    }
    if (FireAction)
    {
        Input->BindAction(FireAction, ETriggerEvent::Started, this, &ThisClass::BeginFireInput);
        Input->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::EndFire);
        Input->BindAction(FireAction, ETriggerEvent::Canceled, this, &ThisClass::EndFire);
    }
    if (ReloadAction) Input->BindAction(ReloadAction, ETriggerEvent::Started, this, &ThisClass::Reload);
}

void APSMatchPlayerController::Move(const FInputActionValue& Value)
{
    auto* FPSPawn = Cast<APSFPSCharacter>(GetPawn());
    if (!FPSPawn || !FPSPawn->CanMove()) return;
    const FVector2D Axis = Value.Get<FVector2D>();
    const FRotator Yaw(0, GetControlRotation().Yaw, 0);
    FPSPawn->AddMovementInput(Yaw.Vector(), Axis.Y);
    FPSPawn->AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), Axis.X);
}

void APSMatchPlayerController::Look(const FInputActionValue& Value)
{
    const auto* FPSPawn = Cast<APSFPSCharacter>(GetPawn());
    if (!FPSPawn || !FPSPawn->IsAlive()) return;
    const FVector2D Axis = Value.Get<FVector2D>();
    AddYawInput(Axis.X);
    AddPitchInput(Axis.Y);
}

void APSMatchPlayerController::StartJump()
{
    if (auto* FPSPawn = Cast<APSFPSCharacter>(GetPawn()); FPSPawn && FPSPawn->CanMove()) FPSPawn->Jump();
}
void APSMatchPlayerController::EndJump()
{
    if (auto* FPSPawn = Cast<ACharacter>(GetPawn())) FPSPawn->StopJumping();
}
void APSMatchPlayerController::StartCrouch()
{
    if (auto* FPSPawn = Cast<APSFPSCharacter>(GetPawn()); FPSPawn && FPSPawn->CanMove()) FPSPawn->Crouch();
}
void APSMatchPlayerController::EndCrouch()
{
    if (auto* FPSPawn = Cast<ACharacter>(GetPawn())) FPSPawn->UnCrouch();
}
void APSMatchPlayerController::BeginFireInput()
{
    if (auto* FPSPawn = Cast<APSFPSCharacter>(GetPawn())) FPSPawn->Combat->StartFire();
}
void APSMatchPlayerController::EndFire()
{
    if (auto* FPSPawn = Cast<APSFPSCharacter>(GetPawn())) FPSPawn->Combat->StopFire();
}
void APSMatchPlayerController::Reload()
{
    if (auto* FPSPawn = Cast<APSFPSCharacter>(GetPawn())) FPSPawn->Combat->Reload();
}
void APSMatchPlayerController::FlushPressedKeys()
{
    EndFire();
    EndJump();
    EndCrouch();
    Super::FlushPressedKeys();
}
