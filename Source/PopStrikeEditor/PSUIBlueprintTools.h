#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PSUIBlueprintTools.generated.h"
class UWidgetBlueprint;
UCLASS()
class POPSTRIKEEDITOR_API UPSUIBlueprintTools : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PopStrike|Editor")
    static FString Inspect(UWidgetBlueprint* Blueprint);
    UFUNCTION(BlueprintCallable, Category="PopStrike|Editor")
    static bool WireButton(UWidgetBlueprint* Blueprint, FName WidgetName, FName FunctionName, int32 Row);
    UFUNCTION(BlueprintCallable, Category="PopStrike|Editor")
    static bool Prepare(UWidgetBlueprint* Blueprint, UClass* ParentClass);
    UFUNCTION(BlueprintCallable, Category="PopStrike|Editor")
    static bool CompileChecked(UWidgetBlueprint* Blueprint);
};
