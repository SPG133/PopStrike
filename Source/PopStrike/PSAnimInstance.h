#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PSAnimInstance.generated.h"

/** AnimBP owns poses and blends; this instance only samples character state. */
UCLASS()
class POPSTRIKE_API UPSAnimInstance : public UAnimInstance
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly, Category="Locomotion") float Speed = 0;
    UPROPERTY(BlueprintReadOnly, Category="Locomotion") float ForwardSpeed = 0;
    UPROPERTY(BlueprintReadOnly, Category="Locomotion") float RightSpeed = 0;
    UPROPERTY(BlueprintReadOnly, Category="Locomotion") bool bMoving = false;
    UPROPERTY(BlueprintReadOnly, Category="Locomotion") bool bFalling = false;
    UPROPERTY(BlueprintReadOnly, Category="Locomotion")  bool bIsIdle = false;
    UPROPERTY(BlueprintReadOnly, Category="Locomotion") bool bCrouching = false;
    UPROPERTY(BlueprintReadOnly, Category="Locomotion") bool bDead = false;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
