/*
 * UAzulWidgetDialogueBase
 * ------------------------------------------------------
 * Este widget representa la interfaz visual del diálogo.
 *
 * Responsabilidades principales:
 * - Mostrar en pantalla el texto del diálogo actual.
 * - Mostrar el nombre del personaje que está hablando.
 * - Recibir input desde botones como Continuar o Elección.
 * - Reenviar esas acciones al objeto UAzulDialogue o al GameSubsystem.
 *
 * Este widget NO contiene la lógica narrativa del diálogo;
 * solo presenta la información y recoge interacción del jugador.
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AzulWidgetDialogueBase.generated.h"

class UAzulDialogue;
class UTextBlock;
class UButton;

UCLASS()
class AZULPROJECT_API UAzulWidgetDialogueBase : public UUserWidget
{
    GENERATED_BODY()

public:

    /** Diálogo activo asociado a este widget */
    UPROPERTY(BlueprintReadWrite, Category = "Azul|Dialogue")
    UAzulDialogue* Dialogue = nullptr;

    /** Texto principal del diálogo */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Azul|Dialogue")
    UTextBlock* DialogueTextBlock = nullptr;

    /** Nombre del personaje que habla */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Azul|Dialogue")
    UTextBlock* TextName = nullptr;

    /** Botón de continuar */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Azul|Dialogue")
    UButton* ButtonContinue = nullptr;

    /** Botones de elección */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Azul|Dialogue")
    UButton* ChoiceButton1 = nullptr;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Azul|Dialogue")
    UButton* ChoiceButton2 = nullptr;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Azul|Dialogue")
    UButton* ChoiceButton3 = nullptr;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Azul|Dialogue")
    UButton* ChoiceButton4 = nullptr;

    /** Textos internos de cada botón de elección */
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Azul|Dialogue")
    UTextBlock* ChoiceText1 = nullptr;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Azul|Dialogue")
    UTextBlock* ChoiceText2 = nullptr;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Azul|Dialogue")
    UTextBlock* ChoiceText3 = nullptr;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Azul|Dialogue")
    UTextBlock* ChoiceText4 = nullptr;

    /** Llamado desde el botón continuar */
    UFUNCTION(BlueprintCallable, Category = "Azul|Dialogue")
    void PressContinue();

    /** Llamado desde los botones de elección (con índice 0..3) */
    UFUNCTION(BlueprintCallable, Category = "Azul|Dialogue")
    void PressChoice(int32 ChoiceIndex);

    /** Pinta el texto del diálogo en el widget */
    UFUNCTION(BlueprintCallable, Category = "Azul|Dialogue")
    void SetDialogueText(const FString& NewText);

    /** Devuelve el texto actualmente visible en el widget */
    UFUNCTION(BlueprintCallable, Category = "Azul|Dialogue")
    FString GetDialogueTextString() const;

    /** Refresca visibilidad y texto de botones de decisión según CurrentRow */
    UFUNCTION(BlueprintCallable, Category = "Azul|Dialogue")
    void RefreshDecisionUI();

protected:

    virtual void NativeConstruct() override;

    virtual FReply NativeOnMouseButtonDown(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent
    ) override;

    /** Handlers internos para OnClicked de cada botón de choice */
    UFUNCTION()
    void HandleChoice1();

    UFUNCTION()
    void HandleChoice2();

    UFUNCTION()
    void HandleChoice3();

    UFUNCTION()
    void HandleChoice4();
};
