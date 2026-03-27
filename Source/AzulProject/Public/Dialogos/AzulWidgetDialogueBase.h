#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AzulWidgetDialogueBase.generated.h"

class UAzulDialogue;
class UTextBlock;

UCLASS()
class AZULPROJECT_API UAzulWidgetDialogueBase : public UUserWidget
{
    GENERATED_BODY()

public:

    /* Referencia al objeto diálogo activo */
    UPROPERTY(BlueprintReadWrite, Category = "Azul|Dialogue")
    UAzulDialogue* Dialogue;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Azul|Dialogue")
    UTextBlock* DialogueTextBlock = nullptr;

    /* Llamado desde el botón continuar */
    UFUNCTION(BlueprintCallable, Category = "Azul|Dialogue")
    void PressContinue();

    UFUNCTION(BlueprintCallable, Category = "Azul|Dialogue")
    void PressChoice(int32 ChoiceIndex);

    /* Pinta el texto en el widget de diálogo */
    UFUNCTION(BlueprintCallable, Category = "Azul|Dialogue")
    void SetDialogueText(const FString& NewText);

    /* Devuelve el texto actualmente visible en el widget */
    UFUNCTION(BlueprintCallable, Category = "Azul|Dialogue")
    FString GetDialogueTextString() const;

protected:

    virtual FReply NativeOnMouseButtonDown(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent
    ) override;
};
