#include "PSAnimInstance.h"
#include "PSFPSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPSAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    const auto* Character = Cast<APSFPSCharacter>(TryGetPawnOwner());
    if (!Character) return;
    Speed = Character->GetVelocity().Size2D();
    const FVector LocalVelocity = Character->GetActorRotation().UnrotateVector(Character->GetVelocity());
    ForwardSpeed = LocalVelocity.X;
    RightSpeed = LocalVelocity.Y;
    bMoving = Speed > 3.f;
    
    //bFalling = Character->GetCharacterMovement()->IsFalling();
    bIsIdle = !bMoving && !bFalling;
    bCrouching = Character->bIsCrouched;
    bDead = !Character->IsAlive();
}
