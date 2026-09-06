#include "PSMenuPlayerController.h"

#include "PSGameConfig.h"
#include "PSMainMenuWidget.h"
#include "UObject/ConstructorHelpers.h"

APSMenuPlayerController::APSMenuPlayerController()
{
    static ConstructorHelpers::FClassFinder<UUserWidget> MenuBP(PSConfig::MainMenuWidget);
    if (MenuBP.Class && MenuBP.Class->IsChildOf(UPSMainMenuWidget::StaticClass())) MenuClass = MenuBP.Class.Get();
}

void APSMenuPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalController() || GetNetMode() == NM_DedicatedServer || !MenuClass)
    {
        return;
    }

    MenuWidget = CreateWidget<UPSMainMenuWidget>(this, MenuClass);
    if (!MenuWidget)
    {
        return;
    }

    MenuWidget->AddToViewport(100);
    bShowMouseCursor = true;

    FInputModeUIOnly Mode;
    Mode.SetWidgetToFocus(MenuWidget->TakeWidget());
    SetInputMode(Mode);
}
