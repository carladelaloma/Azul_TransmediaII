/*
 * UAzulWidgetDialogueBase - Implementación
 * ------------------------------------------------------
 * Este archivo implementa el comportamiento del widget de diálogo.
 *
 * Aquí se gestiona:
 * - El botón de continuar.
 * - Los botones de elección.
 * - La actualización del texto visible en pantalla.
 * - El click general sobre el widget para avanzar diálogo
 *   cuando la fila actual no es una decisión.
 *
 * Este archivo actúa como puente entre la UI del widget
 * y el sistema central de diálogo.
 */

#include "Dialogos/AzulWidgetDialogueBase.h"
#include "Dialogos/AzulDialogue.h"
#include "AzulSubsystem/AzulGameSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"

void UAzulWidgetDialogueBase::NativeConstruct()
{
    Super::NativeConstruct();

    if (ButtonContinue)
    {
        ButtonContinue->OnClicked.AddDynamic(this, &UAzulWidgetDialogueBase::PressContinue);
    }

    if (ChoiceButton1)
    {
        ChoiceButton1->OnClicked.AddDynamic(this, &UAzulWidgetDialogueBase::HandleChoice1);
    }

    if (ChoiceButton2)
    {
        ChoiceButton2->OnClicked.AddDynamic(this, &UAzulWidgetDialogueBase::HandleChoice2);
    }

    if (ChoiceButton3)
    {
        ChoiceButton3->OnClicked.AddDynamic(this, &UAzulWidgetDialogueBase::HandleChoice3);
    }

    if (ChoiceButton4)
    {
        ChoiceButton4->OnClicked.AddDynamic(this, &UAzulWidgetDialogueBase::HandleChoice4);
    }
}

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
    if (!Dialogue)
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueWidget: Dialogue es NULL"));
        return;
    }

    Dialogue->OnChoiceClicked(ChoiceIndex);

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAzulGameSubsystem* GameSubsystem = GI->GetSubsystem<UAzulGameSubsystem>())
        {
            GameSubsystem->RefreshDialogueWidget();
        }
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
    if (!InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
    }

    if (!Dialogue || !Dialogue->CurrentRow)
    {
        return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
    }

    if (Dialogue->CurrentRow->IsDecision)
    {
        return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
    }

    PressContinue();
    return FReply::Handled();
}

void UAzulWidgetDialogueBase::RefreshDecisionUI()
{
    if (!Dialogue || !Dialogue->CurrentRow)
    {
        if (ButtonContinue)
        {
            ButtonContinue->SetVisibility(ESlateVisibility::Collapsed);
        }

        if (ChoiceButton1) ChoiceButton1->SetVisibility(ESlateVisibility::Collapsed);
        if (ChoiceButton2) ChoiceButton2->SetVisibility(ESlateVisibility::Collapsed);
        if (ChoiceButton3) ChoiceButton3->SetVisibility(ESlateVisibility::Collapsed);
        if (ChoiceButton4) ChoiceButton4->SetVisibility(ESlateVisibility::Collapsed);

        return;
    }

    const bool bIsDecision = Dialogue->CurrentRow->IsDecision;
    const int32 NumChoices = Dialogue->CurrentRow->ChoicesText.Num();

    if (ButtonContinue)
    {
        ButtonContinue->SetVisibility(ESlateVisibility::Visible);
        ButtonContinue->SetIsEnabled(!bIsDecision);
    }

    auto SetupChoiceButton = [](UButton* Button, UTextBlock* Label, bool bVisible, const FString& Text)
        {
            if (!Button)
            {
                return;
            }

            Button->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

            if (bVisible && Label)
            {
                Label->SetText(FText::FromString(Text));
            }
        };

    SetupChoiceButton(
        ChoiceButton1,
        ChoiceText1,
        bIsDecision && NumChoices > 0,
        NumChoices > 0 ? Dialogue->CurrentRow->ChoicesText[0] : FString()
    );

    SetupChoiceButton(
        ChoiceButton2,
        ChoiceText2,
        bIsDecision && NumChoices > 1,
        NumChoices > 1 ? Dialogue->CurrentRow->ChoicesText[1] : FString()
    );

    SetupChoiceButton(
        ChoiceButton3,
        ChoiceText3,
        bIsDecision && NumChoices > 2,
        NumChoices > 2 ? Dialogue->CurrentRow->ChoicesText[2] : FString()
    );

    SetupChoiceButton(
        ChoiceButton4,
        ChoiceText4,
        bIsDecision && NumChoices > 3,
        NumChoices > 3 ? Dialogue->CurrentRow->ChoicesText[3] : FString()
    );
}

void UAzulWidgetDialogueBase::HandleChoice1()
{
    PressChoice(0);
}

void UAzulWidgetDialogueBase::HandleChoice2()
{
    PressChoice(1);
}

void UAzulWidgetDialogueBase::HandleChoice3()
{
    PressChoice(2);
}

void UAzulWidgetDialogueBase::HandleChoice4()
{
    PressChoice(3);
}