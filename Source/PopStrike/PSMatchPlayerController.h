#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PSMatchPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UUserWidget;
struct FInputActionValue;

UCLASS()
class POPSTRIKE_API APSMatchPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input") TObjectPtr<UInputMappingContext> MatchMapping;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input") TObjectPtr<UInputAction> MoveAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input") TObjectPtr<UInputAction> LookAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input") TObjectPtr<UInputAction> JumpAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input") TObjectPtr<UInputAction> CrouchAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input") TObjectPtr<UInputAction> FireAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input") TObjectPtr<UInputAction> ReloadAction;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI") TSubclassOf<UUserWidget> CombatHUDClass;
    virtual void FlushPressedKeys() override;
protected:
    virtual void BeginPlayingState() override;
    virtual void SetupInputComponent() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartJump();
    void EndJump();
    void StartCrouch();
    void EndCrouch();
    void BeginFireInput();
    void EndFire();
    void Reload();
    UPROPERTY(Transient) TObjectPtr<UUserWidget> CombatHUD;
};
