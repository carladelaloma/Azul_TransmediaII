#include "Dialogos/AzulDialogue.h"
#include "AzulSubsystem/AzulGameSubsystem.h"
#include "Engine/GameInstance.h"

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
        bHasFinishedTable = false;
    }

    LoadCurrentRow();
}

void UAzulDialogue::LoadCurrentRow()
{
    if (!CurrentTable)
    {
        UE_LOG(LogTemp, Error, TEXT("LoadCurrentRow: CurrentTable es NULL"));
        OnDialogueFinished.Broadcast();
        return;
    }

    const FString RowIDString = FString::FromInt(CurrentID);
    CurrentRow = CurrentTable->FindRow<FDialogueRow>(*RowIDString, TEXT("LoadCurrentRow"));

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
        return FString();
    }

    return CurrentRow->Text;
}

FString UAzulDialogue::GetProcessedCurrentText() const
{
    if (!CurrentRow)
    {
        return FString();
    }

    return ProcessSonName(CurrentRow->Text);
}

FString UAzulDialogue::ProcessSonName(const FString& InText) const
{
    FString Out = InText;

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UAzulGameSubsystem* GameSubsystem = GI->GetSubsystem<UAzulGameSubsystem>())
            {
                const FString& SonNameString = GameSubsystem->SonName;

                if (!SonNameString.IsEmpty())
                {
                    Out = Out.Replace(
                        TEXT("{SonName}"),
                        *SonNameString,
                        ESearchCase::IgnoreCase
                    );
                }
            }
        }
    }

    return Out;
}

void UAzulDialogue::ContinueDialogue()
{
    if (!CurrentRow)
    {
        UE_LOG(LogTemp, Error, TEXT("ContinueDialogue: CurrentRow es NULL"));
        return;
    }

    // Si es decisión, no se avanza con continuar; hay que elegir opción
    if (CurrentRow->IsDecision)
    {
        UE_LOG(LogTemp, Warning, TEXT("ContinueDialogue: Fila %d ES decisión -> no avanzar"), CurrentID);
        return;
    }

    // Si NextID = 0 -> fin del diálogo
    if (CurrentRow->NextID == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ContinueDialogue: Fin del diálogo (NextID = 0)"));
        OnDialogueFinished.Broadcast();
        return;
    }

    // Avance normal
    CurrentID = CurrentRow->NextID;

    UE_LOG(LogTemp, Warning, TEXT("ContinueDialogue: Avanzando a fila %d"), CurrentID);

    LoadCurrentRow();
}

void UAzulDialogue::OnChoiceClicked(int32 ChoiceIndex)
{
    if (!CurrentRow || !CurrentRow->IsDecision)
    {
        UE_LOG(LogTemp, Error, TEXT("OnChoiceClicked llamado en fila NO decisión"));
        return;
    }

    if (!CurrentRow->ChoicesNext.IsValidIndex(ChoiceIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("OnChoiceClicked: índice %d inválido"), ChoiceIndex);
        return;
    }

    // Saltar a la fila decidida
    CurrentID = CurrentRow->ChoicesNext[ChoiceIndex];

    UE_LOG(LogTemp, Warning, TEXT("OnChoiceClicked: Avanzando a fila %d"), CurrentID);

    LoadCurrentRow();
}

void UAzulDialogue::ForceDialogue(int NewID)
{
    CurrentID = NewID;
}

bool UAzulDialogue::IsCurrentRowDecision() const
{
    return CurrentRow && CurrentRow->IsDecision;
}

int32 UAzulDialogue::GetChoicesCount() const
{
    return CurrentRow ? CurrentRow->ChoicesText.Num() : 0;
}

FString UAzulDialogue::GetChoiceText(int32 Index) const
{
    if (!CurrentRow || !CurrentRow->ChoicesText.IsValidIndex(Index))
    {
        return FString();
    }

    return CurrentRow->ChoicesText[Index];
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