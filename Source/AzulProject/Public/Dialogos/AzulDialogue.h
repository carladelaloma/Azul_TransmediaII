#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "AzulDialogue.generated.h"

USTRUCT(BlueprintType)
struct FDialogueRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsDecision = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NextID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ChoicesText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> ChoicesNext;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueFinished);

UCLASS(Blueprintable, BlueprintType)
class AZULPROJECT_API UAzulDialogue : public UObject
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Dialogo")
    UDataTable* DialogueTable = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Azul|Dialogo")
    int32 CurrentTableIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Dialogo")
    bool bRepeatUntilFinished = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Dialogo")
    bool bHasFinishedTable = false;

    UPROPERTY(BlueprintReadOnly, Category = "Azul|Dialogo")
    int32 CurrentID = 1;

    // Puntero interno a la fila actual (no expuesto como UPROPERTY)
    FDialogueRow* CurrentRow = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Azul|Dialogo")
    void StartDialogue(UDataTable* OverrideTable = nullptr, bool bRestart = true);

    UFUNCTION()
    void LoadCurrentRow();

    UFUNCTION()
    FString GetCurrentText() const;

    UFUNCTION()
    FString GetProcessedCurrentText() const;

    UFUNCTION()
    void ContinueDialogue();

    UFUNCTION()
    void OnChoiceClicked(int32 ChoiceIndex);

    UFUNCTION()
    void ForceDialogue(int NewID);

    UFUNCTION()
    bool IsCurrentRowDecision() const;

    UFUNCTION()
    int32 GetChoicesCount() const;

    UFUNCTION()
    FString GetChoiceText(int32 Index) const;

    UPROPERTY(BlueprintAssignable)
    FOnDialogueFinished OnDialogueFinished;

    UPROPERTY(BlueprintAssignable)
    FOnDialogueUpdated OnDialogueUpdated;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    float DialogueDuration = 0.f;

public:
    FText GetDialogueText() const;
    float GetDialogueDuration() const;
    void SetDialogueDuration(float NewDuration);

private:
    FString ProcessSonName(const FString& InText) const;

    UPROPERTY()
    UDataTable* CurrentTable = nullptr;
};