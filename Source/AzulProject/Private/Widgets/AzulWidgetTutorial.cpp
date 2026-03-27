#include "Widgets/AzulWidgetTutorial.h"
#include "GameFramework/PlayerController.h"
#include "Characters/AzulCharacterBase.h"
#include "AzulSubsystem/AzulTutorialSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAzulWidgetTutorial::NativeConstruct()
{
    Super::NativeConstruct();

    ContinueButton->SetIsEnabled(false);

    CheckBox_1->SetVisibility(ESlateVisibility::HitTestInvisible);
    CheckBox_2->SetVisibility(ESlateVisibility::HitTestInvisible);
    CheckBox_3->SetVisibility(ESlateVisibility::HitTestInvisible);

    if (InteractHelp_FirstSet)
        InteractHelp_FirstSet->SetVisibility(ESlateVisibility::Hidden);

    if (InteractHelp_SecondSet)
        InteractHelp_SecondSet->SetVisibility(ESlateVisibility::Hidden);
    if (InteractHelp_ThirdSet)
        InteractHelp_ThirdSet->SetVisibility(ESlateVisibility::Hidden);


    if (GetGameInstance())
    {
        if (UAzulTutorialSubsystem* TutorialSubsystem =
            GetGameInstance()->GetSubsystem<UAzulTutorialSubsystem>())
        {
            TutorialSubsystem->OnTutorialStepUpdated.RemoveDynamic(
                this,
                &UAzulWidgetTutorial::FirstPartTutorial
            );

            TutorialSubsystem->OnTutorialStepUpdated.AddDynamic(
                this,
                &UAzulWidgetTutorial::FirstPartTutorial
            );

            TutorialSubsystem->OnTutorialCompleted.RemoveDynamic(
                this,
                &UAzulWidgetTutorial::HandleTutorialCompleted
            );

            TutorialSubsystem->OnTutorialCompleted.AddDynamic(
                this,
                &UAzulWidgetTutorial::HandleTutorialCompleted
            );
        }
    }

}

void UAzulWidgetTutorial::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (ContinueButton)
    {
        ContinueButton->OnClicked.AddUniqueDynamic(
            this,
            &UAzulWidgetTutorial::OnContinueButtonPressed
        );
    }

    if (SkipButton)
    {
        SkipButton->OnClicked.AddUniqueDynamic(
            this,
            &UAzulWidgetTutorial::OnSkipTutorialPressed
        );
    }
}

void UAzulWidgetTutorial::NativeDestruct()
{
    Super::NativeDestruct();

    if (GetGameInstance())
    {
        if (UAzulTutorialSubsystem* TutorialSubsystem =
            GetGameInstance()->GetSubsystem<UAzulTutorialSubsystem>())
        {
            TutorialSubsystem->OnTutorialCompleted.RemoveDynamic(
                this,
                &UAzulWidgetTutorial::HandleTutorialCompleted
            );
        }
    }
}



void UAzulWidgetTutorial::FirstPartTutorial(FGameplayTag StepTag, bool bCompleted)
{
    if (!TutorialVerticalBox || !ContinueButton)
        return;

    if (!bCompleted)
        return;

    CurrentStepTag = StepTag;

    // ---------- SPACE ----------
    if (StepTag == FGameplayTag::RequestGameplayTag("Tutorial.First.Space"))
    {
        SetTaskCompleted(CheckBox_1, TareaText_1);


        MainText = TEXT("This is the thread that will always take you to your son whenever you want it. It lasts 4 seconds, but you can remove it before that time by pressing the spacebar again. You cannot move while you are watching the thread.");
        GetWorld()->GetTimerManager().ClearTimer(TextTimer);
        
        //Pasa 2 segundos y cambiamos texto
        GetWorld()->GetTimerManager().SetTimer(
            TextTimer,
            this,
            &UAzulWidgetTutorial::ApplyTutorialText,
            1.0f,
            false
        );

        EnableContinueButton();

        return;
    }

    // ---------- MOVE (WASD) ----------
    if (StepTag.MatchesTag(FGameplayTag::RequestGameplayTag("Tutorial.First.Move")))
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            if (AAzulCharacterBase* Character = Cast<AAzulCharacterBase>(PC->GetPawn()))
            {
                if (Character->bIsReadyToMoveTutorial) {
                    SetTaskCompleted(CheckBox_2, TareaText_2);

                }
            }
        }

        // ⏱ Esperar 3 segundos antes de mostrar el texto
        GetWorld()->GetTimerManager().ClearTimer(TextTimer);
        GetWorld()->GetTimerManager().SetTimer(
            TextTimer,
            this,
            &UAzulWidgetTutorial::ApplyMoveCompletedText,
            3.0f,
            false
        );

        EnableContinueButton();
        return;
    }


    // ---------- LOOK ----------
    if (StepTag == FGameplayTag::RequestGameplayTag("Tutorial.First.Look"))
    {
        SetTaskCompleted(CheckBox_3, TareaText_3);

        ContinueButton->SetVisibility(ESlateVisibility::Hidden);

        ClearTutorialText();

        MainText = TEXT("Great, now you can see everything you want by turning your character's head.");

        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            if (AAzulCharacterBase* Character = Cast<AAzulCharacterBase>(PC->GetPawn()))
            {
                // Bloquear movimiento WASD
                if (Character->GetCharacterMovement())
                {
                    Character->GetCharacterMovement()->DisableMovement();
                }

                // Permitir LOOK
                PC->SetIgnoreLookInput(false);
            }
        }


        GetWorld()->GetTimerManager().SetTimer(
            TextTimer,
            this,
            &UAzulWidgetTutorial::ApplyTutorialText,
            1.0f,
            false
        );

        // Timer solo informativo, no decisivo
        GetWorld()->GetTimerManager().SetTimer(
            ButtonTimer,
            this,
            &UAzulWidgetTutorial::SetCheckBoxsForSecondPart,
            3.0f,
            false
        );

        return;
    }


    // ---------- INTERACTUAR ----------
    if (StepTag == FGameplayTag::RequestGameplayTag("Tutorial.Interact"))
    {
        SetTaskCompleted(CheckBox_1, TareaText_1);
        
        //MainText = TEXT("Very well,");
        GetWorld()->GetTimerManager().ClearTimer(ButtonTimer);
        GetWorld()->GetTimerManager().SetTimer(
            TextTimer,
            this,
            &UAzulWidgetTutorial::ApplyTutorialText,
            5.0f,
            false
        );

        return;
    }

    if (StepTag == FGameplayTag::RequestGameplayTag("Tutorial.TakeManual")) {
        SetTaskCompleted(CheckBox_2, TareaText_2);

        MainText = TEXT("You just picked up the manual, try opening it.");
        GetWorld()->GetTimerManager().ClearTimer(ButtonTimer);
        GetWorld()->GetTimerManager().SetTimer(
            TextTimer,
            this,
            &UAzulWidgetTutorial::ApplyTutorialText,
            5.0f,
            false
        );

        return;
    }

    if (StepTag == FGameplayTag::RequestGameplayTag("Tutorial.OpenManual")) {
        SetTaskCompleted(CheckBox_3, TareaText_3);

        MainText = TEXT("Yes, you've just completed the tutorial.");

        GetWorld()->GetTimerManager().ClearTimer(TextTimer);
        GetWorld()->GetTimerManager().SetTimer(
            TextTimer,
            this,
            &UAzulWidgetTutorial::ApplyTutorialText,
            0.1f,
            false
        );

        // ⏱ Limpiar texto tras 4 segundos
        GetWorld()->GetTimerManager().SetTimer(
            ButtonTimer,
            this,
            &UAzulWidgetTutorial::ClearTutorialText,
            4.0f,
            false
        );

        return;
    }

}

FReply UAzulWidgetTutorial::NativeOnKeyDown(
    const FGeometry& InGeometry,
    const FKeyEvent& InKeyEvent
)
{
    if (ContinueButton && ContinueButton->GetIsEnabled())
    {
        const FKey PressedKey = InKeyEvent.GetKey();

        // ✅ SOLO ENTER
        if (PressedKey == EKeys::Enter)
        {
            OnContinueButtonPressed();
            return FReply::Handled();
        }

        // ❌ Space se consume explícitamente
        if (PressedKey == EKeys::SpaceBar)
        {
            return FReply::Handled();
        }
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}



void UAzulWidgetTutorial::OnContinueButtonPressed()
{
    if (!TutorialText || !ContinueButton)
        return;

    // FLUJO DE INTERACT HELP
    if (InteractHelpState == EInteractHelpState::FirstSet)
    {
        ShowSecondInteractHelpSet();
        return;
    }
    if (InteractHelpState == EInteractHelpState::SecondSet)
    {
        ShowThirdInteractHelpSet();
        return;
    }

    if (InteractHelpState == EInteractHelpState::ThirdSet)
    {
        CloseAllInteractHelp();
        return;
    }

    ContinueButton->SetIsEnabled(false);

    // --- FLUJO DE TEXTO ---
    if (CurrentStepTag == FGameplayTag::RequestGameplayTag("Tutorial.First.Space"))
    {
        //RESTAURAR INPUT A JUEGO
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            //PC->bShowMouseCursor = true;

            //FInputModeGameOnly InputMode;
            //PC->SetInputMode(InputMode);

            FInputModeGameAndUI InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = true;

        }

        SetTutorialText(
            TEXT("Press W to move forward\nPress A and D to move sideways\nPress S to move backward")
        );

        CurrentStepTag = FGameplayTag::RequestGameplayTag("Tutorial.First.Move");

        if (AAzulCharacterBase* Character = Cast<AAzulCharacterBase>(
            GetWorld()->GetFirstPlayerController()->GetPawn()))
        {
            Character->bIsReadyToMoveTutorial = true;
            Character->bTutorialAllowMovement = true;
            Character->bTutorialForbidMovementWhileHilo = true;

            if (!Character->bMovementLockedByHilo)
            {
                Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            }
        }

        return;
    }


    if (CurrentStepTag == FGameplayTag::RequestGameplayTag("Tutorial.First.Move"))
    {
        SetTutorialText(
            TEXT("Move your mouse to look around")
        );

        
        CurrentStepTag =
            FGameplayTag::RequestGameplayTag("Tutorial.First.Look");

        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            if (AAzulCharacterBase* Character = Cast<AAzulCharacterBase>(PC->GetPawn()))
            {
                PC->SetIgnoreLookInput(false);
                PC->bShowMouseCursor = false;

                FInputModeGameOnly InputMode;
                PC->SetInputMode(InputMode);

                Character->bTutorialForbidMovementWhileHilo = false;
            }
        }

        if (!GetGameInstance())
            return;

        UAzulTutorialSubsystem* TutorialSubsystem =
            GetGameInstance()->GetSubsystem<UAzulTutorialSubsystem>();

        if (!TutorialSubsystem)
            return;

        TutorialSubsystem->bCanLookTutorial = true;

        return;
    }

    if (CurrentStepTag == FGameplayTag::RequestGameplayTag("Tutorial.First.Look")) {

        if (InteractHelp_FirstSet)
            InteractHelp_FirstSet->SetVisibility(ESlateVisibility::Hidden);

        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            if (AAzulCharacterBase* Character = Cast<AAzulCharacterBase>(PC->GetPawn()))
            {
                // --- Desbloquear el look (cámara) ---
                PC->SetIgnoreLookInput(false);

                // --- Ocultar cursor ---
                PC->bShowMouseCursor = false;

                // --- Modo de input: solo juego ---
                FInputModeGameOnly InputMode;
                PC->SetInputMode(InputMode);

                //Character->UnblockPlayerControl();
                Character->OpenMirilla();
                FString SonNameString;

                // Si SonName es FString
                SonNameString = Character->SonName;

                // Si SonName es FName (usa esta línea en vez de la anterior)
                // SonNameString = Character->SonName.ToString();

                FString TutorialString = FString::Printf(
                    TEXT("%s is crying, what could be wrong with him?"),
                    *SonNameString
                );

                SetTutorialText(TutorialString);


            }
        }
    }
}

void UAzulWidgetTutorial::SetCheckBoxsForSecondPart()
{
    if (bSecondPartActivated)
        return;

    bSecondPartActivated = true;
    InteractHelpState = EInteractHelpState::FirstSet;

    ClearTutorialText();

    CheckBox_1->SetIsChecked(false);
    CheckBox_2->SetIsChecked(false);
    CheckBox_3->SetIsChecked(false);

    TutorialPageText->SetText(FText::FromString(TEXT("Tutorial 2/2")));
    TareaText_1->SetText(FText::FromString(TEXT("Interact with any interactive object")));
    TareaText_2->SetText(FText::FromString(TEXT("Take the manual.")));
    TareaText_3->SetText(FText::FromString(TEXT("Open the manual with the M key")));

    TareaText_1->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    TareaText_2->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    TareaText_3->SetColorAndOpacity(FSlateColor(FLinearColor::White));

    if (InteractHelp_FirstSet)
        InteractHelp_FirstSet->SetVisibility(ESlateVisibility::Visible);

    if (InteractHelp_SecondSet)
        InteractHelp_SecondSet->SetVisibility(ESlateVisibility::Hidden);

    OpenInteractHelp();
}

void UAzulWidgetTutorial::ShowSecondInteractHelpSet()
{
    InteractHelpState = EInteractHelpState::SecondSet;

    if (InteractHelp_FirstSet)
        InteractHelp_FirstSet->SetVisibility(ESlateVisibility::Hidden);

    if (InteractHelp_SecondSet)
        InteractHelp_SecondSet->SetVisibility(ESlateVisibility::Visible);
}
void UAzulWidgetTutorial::ShowThirdInteractHelpSet()
{
    InteractHelpState = EInteractHelpState::ThirdSet;

    if (InteractHelp_ThirdSet)
        InteractHelp_ThirdSet->SetVisibility(ESlateVisibility::Visible);

    if (InteractHelp_SecondSet)
        InteractHelp_SecondSet->SetVisibility(ESlateVisibility::Hidden);
}

void UAzulWidgetTutorial::CloseAllInteractHelp()
{
    InteractHelpState = EInteractHelpState::None;

    if (InteractHelp_FirstSet)
        InteractHelp_FirstSet->SetVisibility(ESlateVisibility::Hidden);

    if (InteractHelp_SecondSet)
        InteractHelp_SecondSet->SetVisibility(ESlateVisibility::Hidden);

    if (InteractHelp_ThirdSet)
        InteractHelp_ThirdSet->SetVisibility(ESlateVisibility::Hidden);

    // Restaurar input
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->SetIgnoreLookInput(false);
        PC->bShowMouseCursor = false;

        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);

        if (AAzulCharacterBase* Character = Cast<AAzulCharacterBase>(PC->GetPawn()))
        {
            Character->OpenMirilla();
            if (Character->GetCharacterMovement())
            {
                Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            }
        }
    }

    // --- VOLVER A MOSTRAR TUTORIAL BORDER ---
    if (TutorialBorder)
    {
        TutorialBorder->SetVisibility(ESlateVisibility::Visible);
    }


    if (ContinueButton)
    {
        ContinueButton->SetVisibility(ESlateVisibility::Hidden);
        ContinueButton->SetIsEnabled(false);
    }

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (AAzulCharacterBase* Character = Cast<AAzulCharacterBase>(PC->GetPawn()))
        {
            FString SonNameString = Character->SonName;

            FString TutorialString = FString::Printf(
                TEXT("%s is crying, what could be wrong with him?"),
                *SonNameString
            );

            SetTutorialText(TutorialString);
        }
    }

}



void UAzulWidgetTutorial::SetTutorialText(const FString& NewText)
{
    GetWorld()->GetTimerManager().ClearTimer(TextTimer);

    MainText = NewText;

    if (NewText.IsEmpty())
    {
        TutorialText->SetText(FText::GetEmpty());
        TextBorder->SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    TutorialText->SetText(FText::FromString(NewText));
    TextBorder->SetVisibility(ESlateVisibility::Visible);
}

void UAzulWidgetTutorial::SetVisibilityTutorialBorder(bool bVisibility)
{
    if (!TutorialBorder)
        return;

    TutorialBorder->SetVisibility(
        bVisibility ? ESlateVisibility::Visible : ESlateVisibility::Hidden
    );
}



void UAzulWidgetTutorial::ApplyTutorialText()
{
    SetTutorialText(MainText);
}

void UAzulWidgetTutorial::EnableContinueButton()
{
    ContinueButton->SetIsEnabled(true);
}

void UAzulWidgetTutorial::OpenInteractHelp()
{

    // --- OCULTAR TUTORIAL BORDER ---
    if (TutorialBorder)
    {
        TutorialBorder->SetVisibility(ESlateVisibility::Hidden);
    }

    if (InteractHelp_FirstSet)
        InteractHelp_FirstSet->SetVisibility(ESlateVisibility::Visible);

    // --- Obtener PlayerController ---
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
        return;

    // --- Bloquear el look (cámara) ---
    PC->SetIgnoreLookInput(true);

    // --- Mostrar cursor ---
    PC->bShowMouseCursor = true;

    // --- Modo de input: solo UI (o UI + Game si prefieres) ---
    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PC->SetInputMode(InputMode);

    // --- Mostrar botón Continue ---
    if (ContinueButton)
    {
        ContinueButton->SetVisibility(ESlateVisibility::Visible);
        ContinueButton->SetIsEnabled(true);
    }
}

void UAzulWidgetTutorial::CompleteTutorial(const FGameplayTag& Tag)
{
    UAzulTutorialSubsystem* TutorialSubsystem =
        GetGameInstance()->GetSubsystem<UAzulTutorialSubsystem>();

    TutorialSubsystem->OnTutorialCompleted.Broadcast(Tag);
}

void UAzulWidgetTutorial::HandleTutorialCompleted(FGameplayTag CompletedTag)
{
    // Si quieres filtrar:
    if (CompletedTag != FGameplayTag::RequestGameplayTag("Tutorial.Completed"))
        return;

    if (TutorialBorder)
    {
        TutorialBorder->SetVisibility(ESlateVisibility::Hidden);
    }
}


void UAzulWidgetTutorial::OnSkipTutorialPressed()
{
    // 1. Obtener el Subsystem
    UAzulTutorialSubsystem* TutorialSubsystem =
        GetGameInstance()->GetSubsystem<UAzulTutorialSubsystem>();

    if (!TutorialSubsystem)
        return;

    // 2. Marcar TODOS los pasos como completados
    TutorialSubsystem->NotifyActionCompleted(
        FGameplayTag::RequestGameplayTag("Tutorial.First.Space")
    );

    TutorialSubsystem->NotifyActionCompleted(
        FGameplayTag::RequestGameplayTag("Tutorial.First.Move")
    );

    TutorialSubsystem->NotifyActionCompleted(
        FGameplayTag::RequestGameplayTag("Tutorial.First.Look")
    );

    TutorialSubsystem->NotifyActionCompleted(
        FGameplayTag::RequestGameplayTag("Tutorial.Interact")
    );

    // 3. Restaurar INPUT COMPLETAMENTE
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        // Desbloquear cámara
        PC->SetIgnoreLookInput(false);

        // Ocultar cursor
        PC->bShowMouseCursor = false;

        // Modo de input: SOLO JUEGO
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);

        if (AAzulCharacterBase* Character = Cast<AAzulCharacterBase>(PC->GetPawn()))
        {
            Character->UnblockPlayerControl();
            Character->OpenMirilla();

            if (Character->GetCharacterMovement())
            {
                Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            }
        }
    }

    // 4. Ocultar el widget
    RemoveFromParent();
}

void UAzulWidgetTutorial::SetTaskCompleted(UCheckBox* CheckBox, UTextBlock* TaskText)
{
    if (CheckBox)
    {
        CheckBox->SetIsChecked(true);
    }

    if (TaskText)
    {
        TaskText->SetColorAndOpacity(
            FSlateColor(FLinearColor(0.443f, 0.443f, 0.443f, 1.0f))
        );
    }
}



void UAzulWidgetTutorial::ClearTutorialText()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(TextTimer);
    }

    MainText = TEXT("");

    if (TutorialText)
    {
        TutorialText->SetText(FText::GetEmpty());
    }

    if (TextBorder)
    {
        TextBorder->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UAzulWidgetTutorial::ApplyMoveCompletedText()
{
    SetTutorialText(
        TEXT("There you are, now you can move in all directions and explore every location.")
    );
}

