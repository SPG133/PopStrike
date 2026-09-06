#include "PSUIBlueprintTools.h"
#include "Modules/ModuleManager.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "K2Node_ComponentBoundEvent.h"
#include "K2Node_CallFunction.h"
#include "EdGraphSchema_K2.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "KismetCompiler.h"
IMPLEMENT_MODULE(FDefaultModuleImpl, PopStrikeEditor)

bool UPSUIBlueprintTools::CompileChecked(UWidgetBlueprint* Blueprint)
{
    if (!Blueprint) return false;
    FCompilerResultsLog Results;
    FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::None, &Results);
    return Results.NumErrors == 0 && Blueprint->Status != BS_Error;
}

bool UPSUIBlueprintTools::Prepare(UWidgetBlueprint* Blueprint, UClass* ParentClass)
{
    if (!Blueprint || !ParentClass || !ParentClass->IsChildOf(UUserWidget::StaticClass())) return false;
    Blueprint->Modify();
    Blueprint->ParentClass = ParentClass;
    if (Blueprint->GetName() == TEXT("WBP_Lobby"))
    {
        for (const auto& Pair : {TPair<FName,FName>(TEXT("Txt_RoomStatusHint"), TEXT("BottomBar")), TPair<FName,FName>(TEXT("Txt_MapHostOnly"), TEXT("MapStack"))})
        {
            if (!Blueprint->WidgetTree->FindWidget(Pair.Key))
            {
                auto* Panel = Cast<UPanelWidget>(Blueprint->WidgetTree->FindWidget(Pair.Value));
                if (!Panel) return false;
                auto* Text = Blueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Pair.Key);
                Text->SetText(FText::FromString(TEXT("Waiting for server...")));
                auto Font = Text->GetFont(); Font.Size = 14; Text->SetFont(Font);
                Panel->AddChild(Text);
            }
        }
    }
    Blueprint->WidgetTree->ForEachWidget([Blueprint](UWidget* Widget)
    {
        Widget->bIsVariable = true;
        if (!Blueprint->WidgetVariableNameToGuidMap.Contains(Widget->GetFName())) Blueprint->OnVariableAdded(Widget->GetFName());
    });
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
    return CompileChecked(Blueprint);
}

FString UPSUIBlueprintTools::Inspect(UWidgetBlueprint* Blueprint)
{
    if (!Blueprint || !Blueprint->WidgetTree) return TEXT("INVALID");
    FString Result = FString::Printf(TEXT("Parent=%s\n"), *GetPathNameSafe(Blueprint->ParentClass));
    Blueprint->WidgetTree->ForEachWidget([&](UWidget* Widget)
    {
        Result += Widget->GetName() + TEXT(" : ") + Widget->GetClass()->GetPathName() + TEXT("\n");
    });
    for (UEdGraph* Graph : Blueprint->UbergraphPages)
        for (UEdGraphNode* Node : Graph->Nodes)
            Result += TEXT("NODE ") + Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString() + TEXT("\n");
    return Result;
}

bool UPSUIBlueprintTools::WireButton(UWidgetBlueprint* Blueprint, FName WidgetName, FName FunctionName, int32 Row)
{
    if (!Blueprint || !Blueprint->SkeletonGeneratedClass || Blueprint->UbergraphPages.IsEmpty()) return false;
    FObjectProperty* Property = FindFProperty<FObjectProperty>(Blueprint->SkeletonGeneratedClass, WidgetName);
    auto* Delegate = FindFProperty<FMulticastDelegateProperty>(UButton::StaticClass(), TEXT("OnClicked"));
    UFunction* Function = Blueprint->ParentClass->FindFunctionByName(FunctionName);
    if (!Property || !Delegate || !Function) return false;
    // Refuse to replace user-authored event logic. Generated events are idempotent.
    for (UEdGraph* ExistingGraph : Blueprint->UbergraphPages)
        for (UEdGraphNode* Node : ExistingGraph->Nodes)
            if (auto* Event = Cast<UK2Node_ComponentBoundEvent>(Node))
                if (Event->ComponentPropertyName == WidgetName && Event->DelegatePropertyName == TEXT("OnClicked"))
                {
                    auto* Then = Event->FindPin(UEdGraphSchema_K2::PN_Then);
                    if (Event->NodeComment != TEXT("PS_GENERATED_UI") || !Then || Then->LinkedTo.Num() != 1) return false;
                    auto* ExistingCall = Cast<UK2Node_CallFunction>(Then->LinkedTo[0]->GetOwningNode());
                    return ExistingCall && ExistingCall->FunctionReference.GetMemberName() == FunctionName;
                }
    Blueprint->Modify();
    UEdGraph* Graph = Blueprint->UbergraphPages[0];
    auto* Event = NewObject<UK2Node_ComponentBoundEvent>(Graph);
    Graph->AddNode(Event, false, false);
    Event->CreateNewGuid();
    Event->InitializeComponentBoundEventParams(Property, Delegate);
    Event->AllocateDefaultPins();
    Event->NodePosX = 0; Event->NodePosY = Row * 220;
    Event->NodeComment = TEXT("PS_GENERATED_UI");
    auto* Call = NewObject<UK2Node_CallFunction>(Graph);
    Graph->AddNode(Call, false, false);
    Call->CreateNewGuid(); Call->SetFromFunction(Function); Call->AllocateDefaultPins();
    Call->NodePosX = 440; Call->NodePosY = Row * 220;
    if (!Graph->GetSchema()->TryCreateConnection(Event->FindPinChecked(UEdGraphSchema_K2::PN_Then), Call->GetExecPin())) return false;
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
    return true;
}
