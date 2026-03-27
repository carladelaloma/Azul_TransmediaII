#include "Dialogos/AzulWidgetDialogueBase.h"
#include "Dialogos/AzulDialogue.h"
#include "Components/TextBlock.h"
#include "AzulSubsystem/AzulGameSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

void UAzulWidgetDialogueBase::PressContinue()
{
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAzulGameSubsystem* GameSubsystem = GI->GetSubsystem<UAzulGameSubsystem>())
        {
            GameSubsystem->RequestAdvanceDialogue();
        }
    }
}

void UAzulWidgetDialogueBase::PressChoice(int32 ChoiceIndex)
{
    if (Dialogue)
    {
        Dialogue->OnChoiceClicked(ChoiceIndex);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("DialogueWidget: Dialogue es NULL"));
    }
}

void UAzulWidgetDialogueBase::SetDialogueText(const FString& NewText)
{
    if (!DialogueTextBlock)
    {
        return;
    }

    DialogueTextBlock->SetText(FText::FromString(NewText));
}

FString UAzulWidgetDialogueBase::GetDialogueTextString() const
{
    if (!DialogueTextBlock)
    {
        return FString();
    }

    return DialogueTextBlock->GetText().ToString();
}


FReply UAzulWidgetDialogueBase::NativeOnMouseButtonDown(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent
)
{
    // Solo botón izquierdo / touch
    if (!InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
    }

    // Si no hay diálogo, no hacer nada
    if (!Dialogue || !Dialogue->CurrentRow)
    {
        return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
    }

    // Si es decisión → NO avanzar con click general
    if (Dialogue->CurrentRow->IsDecision)
    {
        return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
    }

    // No es decisión → avanzar diálogo
    PressContinue();

    return FReply::Handled();
}
