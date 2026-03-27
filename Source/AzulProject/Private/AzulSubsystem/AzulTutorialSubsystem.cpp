#include "AzulSubsystem/AzulTutorialSubsystem.h"
#include "GameplayTagContainer.h"
#include "Engine/Engine.h"

void UAzulTutorialSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Estado inicial: por seguridad lo dejamos desactivado
    bTutorialEnabled = false;

    // Si ya existe un mundo válido, comprobamos el mapa actual
    if (UWorld* World = GetWorld())
    {
        UpdateTutorialStateFromWorld(World);
    }

    // Nos suscribimos para actualizar el estado cada vez que cargue un mapa
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UAzulTutorialSubsystem::OnPostLoadMap);
   
}

void UAzulTutorialSubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

    Super::Deinitialize();
}

void UAzulTutorialSubsystem::OnPostLoadMap(UWorld* LoadedWorld)
{
    UpdateTutorialStateFromWorld(LoadedWorld);
}

void UAzulTutorialSubsystem::UpdateTutorialStateFromWorld(UWorld* World)
{
    const bool bWasTutorialEnabled = bTutorialEnabled;
    bTutorialEnabled = false;

    if (!World)
    {
        return;
    }

    FString MapName = World->GetMapName();

    // En PIE puede venir con prefijos como UEDPIE_0_
    MapName.RemoveFromStart(World->StreamingLevelsPrefix);

    if (MapName == TEXT("LV_Gameplay_01"))
    {
        bTutorialEnabled = true;
    }

   /* if (GEngine)
    {
        const FString DebugMessage = FString::Printf(
            TEXT("[TutorialSubsystem] Nivel: %s | Tutorial activo: %s"),
            *MapName,
            bTutorialEnabled ? TEXT("TRUE") : TEXT("FALSE")
        );

        GEngine->AddOnScreenDebugMessage(
            -1,
            5.0f,
            bTutorialEnabled ? FColor::Green : FColor::Red,
            DebugMessage
        );
    }*/

    // Si acabamos de entrar al nivel del tutorial
    if (!bWasTutorialEnabled && bTutorialEnabled)
    {
        InitializeTasksTutorial();
    }

    // Si acabamos de salir del nivel del tutorial
    if (bWasTutorialEnabled && !bTutorialEnabled)
    {
        // Aquí limpia o resetea lo necesario del tutorial
        // ResetTutorial();
    }
}

void UAzulTutorialSubsystem::NotifyActionCompleted(FGameplayTag ActionTag)
{
    UE_LOG(LogTemp, Log, TEXT("NotifyActionCompleted called"));

    if (!ActionTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("ActionTag is NOT valid"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ActionTag received: %s"), *ActionTag.ToString());

    if (CompletedActions.HasTag(ActionTag))
    {
        UE_LOG(LogTemp, Warning, TEXT("ActionTag %s already completed"), *ActionTag.ToString());
        return;
    }

    CompletedActions.AddTag(ActionTag);

    UE_LOG(LogTemp, Log, TEXT("ActionTag %s added to CompletedActions"), *ActionTag.ToString());

    OnTutorialStepUpdated.Broadcast(ActionTag, true);

    CheckTutorialCompletion();
}

bool UAzulTutorialSubsystem::IsActionCompleted(FGameplayTag ActionTag) const
{
    return CompletedActions.HasTag(ActionTag);
}

bool UAzulTutorialSubsystem::IsTutorialCompleted() const
{
    return bTutorialCompleted;
}

void UAzulTutorialSubsystem::CheckTutorialCompletion()
{
    if (bTutorialCompleted)
        return;

    if (CompletedActions.HasAll(TutorialCompletionRequirements))
    {
        bTutorialCompleted = true;

        FGameplayTag CompletedTag =
            FGameplayTag::RequestGameplayTag("Tutorial.Completed");

        UE_LOG(LogTemp, Warning, TEXT("=== TUTORIAL COMPLETED ==="));

        OnTutorialCompleted.Broadcast(CompletedTag);
    }
}

void UAzulTutorialSubsystem::StartTutorial()
{
    if (bTutorialActive)
        return;

    bTutorialActive = true;

    UE_LOG(LogTemp, Warning, TEXT("Tutorial STARTED"));
}

void UAzulTutorialSubsystem::ResetTutorial()
{
    CompletedActions.Reset();
    bTutorialCompleted = false;
    bTutorialActive = false;
    bCanLookTutorial = false;

    UE_LOG(LogTemp, Warning, TEXT("Tutorial RESET"));
}

bool UAzulTutorialSubsystem::IsTutorialActive() const
{
    return bTutorialActive;
}

void UAzulTutorialSubsystem::InitializeTasksTutorial()
{
    /* Requisitos para terminar el tutorial */
    TutorialCompletionRequirements.AddTag(
        FGameplayTag::RequestGameplayTag("Tutorial.First.Space")
    );

    TutorialCompletionRequirements.AddTag(
        FGameplayTag::RequestGameplayTag("Tutorial.First.Move")
    );

    TutorialCompletionRequirements.AddTag(
        FGameplayTag::RequestGameplayTag("Tutorial.First.Look")
    );

    TutorialCompletionRequirements.AddTag(
        FGameplayTag::RequestGameplayTag("Tutorial.Interact")
    );

    TutorialCompletionRequirements.AddTag(
        FGameplayTag::RequestGameplayTag("Tutorial.TakeManual")
    );

    TutorialCompletionRequirements.AddTag(
        FGameplayTag::RequestGameplayTag("Tutorial.OpenManual")
    );

    UE_LOG(LogTemp, Warning, TEXT("Tutorial Subsystem initialized with %d completion requirements"),
        TutorialCompletionRequirements.Num());
}

void UAzulTutorialSubsystem::ApplyPauseMenuInputMode()
{
    // Determinar si estamos antes del LOOK
    const FGameplayTag LookTag =
        FGameplayTag::RequestGameplayTag("Tutorial.First.Look");

    const bool bBeforeLook = !CompletedActions.HasTag(LookTag);

    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (bBeforeLook)
            {
                // 🔹 Tutorial temprano → Juego + UI
                FInputModeGameAndUI InputMode;
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                InputMode.SetHideCursorDuringCapture(false);

                PC->SetInputMode(InputMode);
                PC->bShowMouseCursor = true;
            }
            else
            {
                // 🔹 Tutorial avanzado → Solo juego
                FInputModeGameOnly InputMode;
                PC->SetInputMode(InputMode);

                PC->bShowMouseCursor = false;
            }
        }
    }
}


