#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"

// Widgets
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CheckBox.h"

// Gameplay Tags
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"

#include "AzulWidgetTutorial.generated.h"

UENUM()
enum class EInteractHelpState : uint8
{
    None,
    FirstSet,
    SecondSet,
    ThirdSet
};


UCLASS()
class AZULPROJECT_API UAzulWidgetTutorial : public UUserWidget
{
    GENERATED_BODY()

public:

    /* Llamada desde el Event Dispatcher del Subsystem */
    UFUNCTION(BlueprintCallable, Category = "Azul|Tutorial")
    void FirstPartTutorial(FGameplayTag StepTag, bool bCompleted);

    virtual FReply NativeOnKeyDown(
        const FGeometry& InGeometry,
        const FKeyEvent& InKeyEvent
    ) override;


protected:

    virtual void NativeConstruct() override;
    virtual void NativeOnInitialized() override;
    virtual void NativeDestruct() override;

    /* -------- WIDGETS -------- */

    UPROPERTY(meta = (BindWidget))
    UBorder* TutorialBorder;

    UPROPERTY(meta = (BindWidget))
    UBorder* TextBorder;

    UPROPERTY(meta = (BindWidget))
    UVerticalBox* TutorialVerticalBox;

    UPROPERTY(meta = (BindWidget))
    UVerticalBox* CheckBoxVerticalBox;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* CheckBox_1;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* CheckBox_2;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* CheckBox_3;

    UPROPERTY(meta = (BindWidget))
    UButton* ContinueButton;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* TutorialText;
    
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* TutorialPageText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TareaText_1;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TareaText_2;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TareaText_3;

    UPROPERTY()
    FString MainText;

    UPROPERTY(meta = (BindWidget))
    UButton* SkipButton;

    UPROPERTY(meta = (BindWidget))
    UBorder* InteractHelp_FirstSet;

    UPROPERTY(meta = (BindWidget))
    UBorder* InteractHelp_SecondSet;

    UPROPERTY(meta = (BindWidget))
    UBorder* InteractHelp_ThirdSet;


    bool bSecondPartActivated = false;

    EInteractHelpState InteractHelpState = EInteractHelpState::None;

    void SetTaskCompleted(UCheckBox* CheckBox, UTextBlock* TaskText);


    UFUNCTION()
    void ClearTutorialText();

    UFUNCTION()
    void ShowSecondInteractHelpSet();
    
    UFUNCTION()
    void ShowThirdInteractHelpSet();

    UFUNCTION()
    void CloseAllInteractHelp();

    UFUNCTION()
    void OnSkipTutorialPressed();

    UFUNCTION()
    void OnContinueButtonPressed();

    UFUNCTION()
    void SetCheckBoxsForSecondPart();

    UFUNCTION(BlueprintCallable)
    void SetTutorialText(const FString& NewText);

    UFUNCTION(BlueprintCallable)
    void SetVisibilityTutorialBorder(bool bVisibility);

    void ApplyTutorialText();

    void EnableContinueButton();

    void OpenInteractHelp();

    void CompleteTutorial(const FGameplayTag& Tag);

    void ApplyMoveCompletedText();

    UFUNCTION()
    void HandleTutorialCompleted(FGameplayTag CompletedTag);

    FTimerHandle TextTimer;
    FTimerHandle ButtonTimer;

private:

    /* Acción actual que está esperando el tutorial */
    FGameplayTag CurrentStepTag;
};
