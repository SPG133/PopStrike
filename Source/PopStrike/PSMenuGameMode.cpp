#include "PSMenuGameMode.h"
#include "PSMenuPlayerController.h"

APSMenuGameMode::APSMenuGameMode()
{
    PlayerControllerClass = APSMenuPlayerController::StaticClass();
    DefaultPawnClass = nullptr;
    HUDClass = nullptr;
}
