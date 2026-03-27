#include "Dialogos/AzulDialogue.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h" // Para AddOnScreenDebugMessage
#include "Characters/AzulCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/OutputDeviceDebug.h"

void UAzulDialogue::StartDialogue(UDataTable* OverrideTable, bool bRestart)
{
    UE_LOG(LogTemp, Warning, TEXT("StartDialogue ejecutado"));

    if (OverrideTable)
    {
        DialogueTable = OverrideTable;
        CurrentTable = OverrideTable;
    }
    else
    {
        CurrentTable = DialogueTable;
    }

    if (!CurrentTable)
    {
        UE_LOG(LogTemp, Error, TEXT("StartDialogue: Tabla inválida"));
        OnDialogueFinished.Broadcast();
        return;
    }

    if (bRestart)
    {
        CurrentID = 1;
        PlayerScore = 0;
        bHasFinishedTable = false;
    }

    ChoiceButtons.Empty();

    LoadCurrentRow();
    OnDialogueUpdated.Broadcast();
}


void UAzulDialogue::LoadCurrentRow()
{
    if (!DialogueTable)
    {
        UE_LOG(LogTemp, Error, TEXT("LoadCurrentRow: DialogueTable es NULL"));
        return;
    }

    FString RowIDString = FString::FromInt(CurrentID);
    CurrentRow = DialogueTable->FindRow<FDialogueRow>(*RowIDString, TEXT(""));

    if (!CurrentRow)
    {
        UE_LOG(LogTemp, Error, TEXT("LoadCurrentRow: Fila %d NO encontrada"), CurrentID);
        OnDialogueFinished.Broadcast();
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("LoadCurrentRow ejecutado. CurrentID = %d"), CurrentID);

    OnDialogueUpdated.Broadcast();
}




FString UAzulDialogue::GetCurrentText() const
{
    if (!CurrentRow)
    {
        return FString("ERROR: No hay fila actual");
    }

    FString Text = CurrentRow->Text;

    return Text;
}


void UAzulDialogue::UpdateWidget(UHorizontalBox* ChoicesContainer)
{
    if (!ChoicesContainer || !CurrentRow)
        return;

    if (ContinueButton)
    {
        ContinueButton->SetVisibility(
            CurrentRow->IsDecision
            ? ESlateVisibility::Collapsed
            : ESlateVisibility::Visible
        );
    }

    // 1. Primera vez: rellenar el array automáticamente desde el HorizontalBox
// SIEMPRE reconstruir los botones
    ChoiceButtons.Empty();

    const int32 ChildCount = ChoicesContainer->GetChildrenCount();
    for (int32 i = 0; i < ChildCount; i++)
    {
        UWidget* Child = ChoicesContainer->GetChildAt(i);
        UButton* Btn = Cast<UButton>(Child);

        if (Btn)
        {
            ChoiceButtons.Add(Btn);
        }
    }


    // Si no hay botones → salir
    if (ChoiceButtons.Num() == 0)
        return;

    // -------------------------------------------------------
    // 2. Si NO es decisión → ocultar todos los botones
    // -------------------------------------------------------
    if (!CurrentRow->IsDecision)
    {
        for (UButton* Btn : ChoiceButtons)
        {
            if (Btn)
                Btn->SetVisibility(ESlateVisibility::Collapsed);
        }

        return;
    }

    // -------------------------------------------------------
    // 3. Mostrar solo los necesarios y poner texto
    // -------------------------------------------------------

    int32 NumChoices = CurrentRow->ChoicesText.Num();

    for (int32 i = 0; i < ChoiceButtons.Num(); i++)
    {
        UButton* Btn = ChoiceButtons[i];
        if (!Btn) continue;

        if (i < NumChoices)
        {
            Btn->SetVisibility(ESlateVisibility::Visible);

            if (UTextBlock* Label = Cast<UTextBlock>(Btn->GetChildAt(0)))
            {
                Label->SetText(FText::FromString(CurrentRow->ChoicesText[i]));
            }
        }
        else
        {
            Btn->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}


void UAzulDialogue::HandleContinueClicked()
{
    ContinueDialogue();
    OnDialogueUpdated.Broadcast();
}

FString UAzulDialogue::ProcessSonName(const FString& InText) const
{
    FString Out = InText;

    AAzulCharacterBase* Player = Cast<AAzulCharacterBase>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
    );

    if (Player)
    {
        Out = Out.Replace(TEXT("{SonName}"), *Player->SonName);
    }

    return Out;
}



void UAzulDialogue::OnChoiceClicked(int32 Index)
{
    if (!CurrentRow || !CurrentRow->IsDecision)
    {
        UE_LOG(LogTemp, Error, TEXT("OnChoiceClicked llamado en fila NO decisión"));
        return;
    }

    if (!CurrentRow->ChoicesNext.IsValidIndex(Index))
    {
        UE_LOG(LogTemp, Error, TEXT("ChoiceClicked: índice %d inválido"), Index);
        return;
    }

    // 1. Saltar a la fila decidida
    CurrentID = CurrentRow->ChoicesNext[Index];

    UE_LOG(LogTemp, Warning, TEXT("ChoiceClicked: Avanzando a fila %d"), CurrentID);

    // 2. Cargar la siguiente línea
    LoadCurrentRow();
}





void UAzulDialogue::ContinueDialogue()
{
    if (!CurrentRow)
    {
        UE_LOG(LogTemp, Error, TEXT("ContinueDialogue: CurrentRow es NULL"));
        return;
    }

    // 1. Si es decisión, NO se avanza automáticamente
    if (CurrentRow->IsDecision)
    {
        UE_LOG(LogTemp, Warning, TEXT("ContinueDialogue: Fila %d ES decisión → no avanzar"), CurrentID);
        return;
    }

    // 2. Si NextID = 0 → fin del diálogo
    if (CurrentRow->NextID == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ContinueDialogue: Fin del diálogo (NextID = 0)"));
        OnDialogueFinished.Broadcast();
        return;
    }

    // 3. Avance normal
    CurrentID = CurrentRow->NextID;

    UE_LOG(LogTemp, Warning, TEXT("ContinueDialogue: Avanzando a fila %d"), CurrentID);

    LoadCurrentRow();
}

void UAzulDialogue::SetDialogueText(UTextBlock* Text)
{
    if (!Text)
    {
        return;
    }

    FString CurrentText = GetCurrentText();

    // reemplazar {SonName}
    CurrentText = ProcessSonName(CurrentText);

    Text->SetText(FText::FromString(CurrentText));

    UE_LOG(LogTemp, Warning, TEXT("SETDIALOGUETEXT ejecutado: %s"), *CurrentText);

}

void UAzulDialogue::ForceDialogue(int NewID)
{
    CurrentID = NewID;
}

FString UAzulDialogue::GetProcessedCurrentText() const
{
    if (!CurrentRow)
    {
        return FString();
    }

    return ProcessSonName(CurrentRow->Text);
}

FText UAzulDialogue::GetDialogueText() const
{
    return DialogueText;
}

float UAzulDialogue::GetDialogueDuration() const
{
    return DialogueDuration;
}

void UAzulDialogue::SetDialogueDuration(float NewDuration)
{
    DialogueDuration = NewDuration;
}
