#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PSMenuPlayerController.generated.h"

class UPSMainMenuWidget;

UCLASS()
class POPSTRIKE_API APSMenuPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    APSMenuPlayerController();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY() TSubclassOf<UPSMainMenuWidget> MenuClass;
    UPROPERTY() TObjectPtr<UPSMainMenuWidget> MenuWidget;
};
