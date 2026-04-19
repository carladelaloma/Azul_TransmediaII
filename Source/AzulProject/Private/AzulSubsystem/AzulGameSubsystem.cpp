/*
 * UAzulGameSubsystem - Implementación
 * ------------------------------------------------------
 * Este archivo implementa la lógica global del subsystem.
 *
 * Aquí se realiza:
 * - La inicialización de widgets globales al comenzar.
 * - El registro y refresco del widget de diálogo.
 * - La comunicación con el diálogo activo.
 * - La gestión de cinemáticas y vídeos.
 * - El bloqueo y desbloqueo del control del jugador
 *   durante secuencias o eventos especiales.
 *
 * En el sistema de diálogo, este archivo coordina
 * la conexión entre:
 *   1) la lógica del diálogo (UAzulDialogue)
 *   2) el widget visual (UAzulWidgetDialogueBase)
 *   3) los actores o Blueprints que solicitan abrir diálogo
 */

#include "AzulSubsystem/AzulGameSubsystem.h"
#include "AzulSubsystem/AzulGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/AzulCharacterBase.h"
#include "GameplayTagContainer.h"
#include "Dialogos/AzulDialogue.h"
#include "Widgets/AzulWidgetHUDPlayer.h"
#include "Blueprint/UserWidget.h"
#include "Actors/AzulInteractuableBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogAzulCinematics, Log, All);

void UAzulGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("[Subsystem] Initialize START"));

    UWorld* World = GetWorld();
    UE_LOG(LogTemp, Warning, TEXT("[Subsystem] World = %s"), *GetNameSafe(World));
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    UE_LOG(LogTemp, Warning, TEXT("[Subsystem] PC = %s"), *GetNameSafe(PC));
    if (!PC) return;

    if (IsGameGameplay())
    {
        if (WidgetHUDPlayerClass)
        {
            WidgetHUDPlayer = CreateWidget<UAzulWidgetHUDPlayer>(PC, WidgetHUDPlayerClass);
            UE_LOG(LogTemp, Warning, TEXT("[Subsystem] WidgetHUDPlayer creado = %s"), *GetNameSafe(WidgetHUDPlayer));

            if (WidgetHUDPlayer)
            {
                WidgetHUDPlayer->AddToViewport();
                WidgetHUDPlayer->SetVisibility(ESlateVisibility::Visible);
            }
        }
    }

    CreateDialogueSystem();
}


void UAzulGameSubsystem::CreateDialogueSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("[Subsystem] CreateDialogueSystem START"));

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    UAzulGameInstance* GI = Cast<UAzulGameInstance>(GetGameInstance());
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("[Subsystem] GameInstance no es UAzulGameInstance"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Subsystem] DialogueWidgetClass desde GI = %s"), *GetNameSafe(GI->DialogueWidgetClass));

    if (!WidgetDialogue && GI->DialogueWidgetClass)
    {
        WidgetDialogue = CreateWidget<UAzulWidgetDialogueBase>(PC, GI->DialogueWidgetClass);

        UE_LOG(LogTemp, Warning, TEXT("[Subsystem] WidgetDialogue creado = %s"), *GetNameSafe(WidgetDialogue));

        if (WidgetDialogue)
        {
            WidgetDialogue->AddToViewport(100);
            WidgetDialogue->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    if (!ActiveDialogue)
    {
        ActiveDialogue = NewObject<UAzulDialogue>(this, UAzulDialogue::StaticClass());

        if (ActiveDialogue)
        {
            ActiveDialogue->OnDialogueUpdated.AddDynamic(this, &UAzulGameSubsystem::OnActiveDialogueUpdated);
            ActiveDialogue->OnDialogueFinished.AddDynamic(this, &UAzulGameSubsystem::OnActiveDialogueFinished);
        }
    }

    if (WidgetDialogue)
    {
        WidgetDialogue->Dialogue = ActiveDialogue;
    }
}

void UAzulGameSubsystem::OpenDialogue(UDataTable* InDialogueTable, bool bRestart, int32 StartID)
{
    UE_LOG(LogTemp, Warning, TEXT("[Subsystem] OpenDialogue START"));
    UE_LOG(LogTemp, Warning, TEXT("[Subsystem] ActiveDialogue = %s | WidgetDialogue = %s | Table = %s"),
        *GetNameSafe(ActiveDialogue),
        *GetNameSafe(WidgetDialogue),
        *GetNameSafe(InDialogueTable));

    if (!ActiveDialogue)
    {
        CreateDialogueSystem();
    }

    if (!ActiveDialogue || !InDialogueTable)
        return;

    ActiveDialogue->ForceDialogue(StartID);
    ActiveDialogue->StartDialogue(InDialogueTable, bRestart);

    if (WidgetDialogue)
    {
        WidgetDialogue->Dialogue = ActiveDialogue;
    }

    RefreshDialogueWidget();
    SetInputForDialogue(true);
}

void UAzulGameSubsystem::SetInputForDialogue(bool bEnable)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    if (bEnable)
    {
        FInputModeUIOnly InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

        if (WidgetDialogue)
        {
            InputMode.SetWidgetToFocus(WidgetDialogue->TakeWidget());
        }

        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(true);
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;
    }
    else
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(false);
        PC->bEnableClickEvents = false;
        PC->bEnableMouseOverEvents = false;
    }
}


void UAzulGameSubsystem::RegisterDialogueWidget(UAzulWidgetDialogueBase* InWidget)
{
    WidgetDialogue = InWidget;

    if (WidgetDialogue && ActiveDialogue)
    {
        WidgetDialogue->Dialogue = ActiveDialogue;
        WidgetDialogue->SetVisibility(ESlateVisibility::Visible);
        RefreshDialogueWidget();
    }
}

void UAzulGameSubsystem::RefreshDialogueWidget()
{
    if (!WidgetDialogue)
        return;

    if (!ActiveDialogue)
    {
        WidgetDialogue->Dialogue = nullptr;
        WidgetDialogue->SetDialogueText(FString());

        if (WidgetDialogue->TextName)
        {
            WidgetDialogue->TextName->SetText(FText::GetEmpty());
        }

        WidgetDialogue->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    WidgetDialogue->Dialogue = ActiveDialogue;
    WidgetDialogue->SetDialogueText(GetActiveDialogueText());

    if (ActiveDialogue->CurrentRow && WidgetDialogue->TextName)
    {
        WidgetDialogue->TextName->SetText(
            FText::FromString(ActiveDialogue->CurrentRow->Name)
        );
    }

    WidgetDialogue->RefreshDecisionUI();
    WidgetDialogue->SetVisibility(ESlateVisibility::Visible);
}

FString UAzulGameSubsystem::GetActiveDialogueText() const
{
    if (!ActiveDialogue)
        return FString();

    return ActiveDialogue->GetProcessedCurrentText();
}

FString UAzulGameSubsystem::GetDialogueTextFromWidget() const
{
    if (!WidgetDialogue)
        return FString();

    return WidgetDialogue->GetDialogueTextString();
}

void UAzulGameSubsystem::OnActiveDialogueUpdated()
{
    RefreshDialogueWidget();
}

void UAzulGameSubsystem::OnActiveDialogueFinished()
{
    // Limpia el diálogo activo
    ActiveDialogue = nullptr;

    if (WidgetDialogue)
    {
        WidgetDialogue->Dialogue = nullptr;
        WidgetDialogue->SetDialogueText(FString());

        if (WidgetDialogue->TextName)
        {
            WidgetDialogue->TextName->SetText(FText::GetEmpty());
        }

        // Ocultar widget pero NO destruirlo
        WidgetDialogue->SetVisibility(ESlateVisibility::Collapsed);
    }

    // Restaurar input a juego
    SetInputForDialogue(false);
}

void UAzulGameSubsystem::PlayLevelSequence(
    ULevelSequence* Sequence,
    bool bRestoreControlOnFinish,
    bool bHideCharacterMesh
)
{
    if (!Sequence) return;

    UWorld* World = GetWorld();
    if (!World) return;

    CurrentSequence = Sequence;

    // 🔹 Guardamos los valores recibidos desde Blueprint
    bRestoreControlAfterSequence = bRestoreControlOnFinish;
    bHideCharacterMeshDuringSequence = bHideCharacterMesh;

    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        APawn* Pawn = PC->GetPawn();
        AAzulCharacterBase* Character = Pawn ? Cast<AAzulCharacterBase>(Pawn) : nullptr;

        if (!Character)
        {
            UE_LOG(
                LogAzulCinematics,
                Warning,
                TEXT("PlayLevelSequence: Pawn no es AAzulCharacterBase (%s)"),
                *GetNameSafe(Pawn)
            );
            return;
        }

        // Bloqueamos control
        Character->BlockPlayerControl();

        // Ocultamos mesh si procede
        if (bHideCharacterMeshDuringSequence)
        {
            if (USkeletalMeshComponent* Mesh = Character->GetMesh())
            {
                Mesh->SetVisibility(false, true);
            }
        }
    }

    ALevelSequenceActor* SequenceActor = nullptr;

    FMovieSceneSequencePlaybackSettings Settings;
    SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
        World,
        Sequence,
        Settings,
        SequenceActor
    );

    if (SequencePlayer)
    {
        SequencePlayer->OnFinished.AddDynamic(
            this,
            &UAzulGameSubsystem::OnSequenceFinished
        );

        SequencePlayer->Play();
    }
}


void UAzulGameSubsystem::PlayVideo(UMediaPlayer* MediaPlayer, UMediaSource* MediaSource)
{
    if (!MediaPlayer || !MediaSource)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        APawn* Pawn = PC->GetPawn();
        AAzulCharacterBase* Character = Pawn ? Cast<AAzulCharacterBase>(Pawn) : nullptr;

        if (!Character)
        {
            UE_LOG(
                LogAzulCinematics,
                Warning,
                TEXT("PlayVideo: Pawn no es AAzulCharacterBase (%s)"),
                *GetNameSafe(Pawn)
            );
            return;
        }

        Character->BlockPlayerControl();
    }

    if (ActiveMediaPlayer)
    {
        ActiveMediaPlayer->OnEndReached.RemoveDynamic(
            this,
            &UAzulGameSubsystem::OnVideoFinished
        );
    }

    ActiveMediaPlayer = MediaPlayer;

    ActiveMediaPlayer->OnEndReached.AddDynamic(
        this,
        &UAzulGameSubsystem::OnVideoFinished
    );

    const bool bOpened = ActiveMediaPlayer->OpenSource(MediaSource);

    if (!bOpened)
    {
        UE_LOG(
            LogAzulCinematics,
            Error,
            TEXT("PlayVideo: No se pudo abrir el MediaSource (%s)"),
            *GetNameSafe(MediaSource)
        );

        ActiveMediaPlayer->OnEndReached.RemoveDynamic(
            this,
            &UAzulGameSubsystem::OnVideoFinished
        );

        ActiveMediaPlayer = nullptr;
        return;
    }

    ActiveMediaPlayer->Play();
}

bool UAzulGameSubsystem::IsSequenceActive()
{
    return SequencePlayer != nullptr;
}

FName UAzulGameSubsystem::GetCurrentGameplayName()
{
    UWorld* World = GetWorld();
    if (!World)
        return NAME_None;

    FString LevelName = UGameplayStatics::GetCurrentLevelName(World, true);

    const FString Prefix = TEXT("LV_Gameplay_");
    if (!LevelName.StartsWith(Prefix))
        return NAME_None;

    FString LevelNumber = LevelName.RightChop(Prefix.Len());
    FString TagString = FString::Printf(TEXT("Gameplay.%s"), *LevelNumber);

    FGameplayTag LevelTag = FGameplayTag::RequestGameplayTag(FName(*TagString), false);
    if (!LevelTag.IsValid())
        return NAME_None;

    AAzulCharacterBase* MomCharacter = Cast<AAzulCharacterBase>(
        UGameplayStatics::GetPlayerCharacter(World, 0)
    );

    if (!MomCharacter)
        return NAME_None;

    if (MomCharacter->ActiveStoryTags.HasTag(LevelTag))
    {
        return LevelTag.GetTagName();
    }

    return NAME_None;
}

bool UAzulGameSubsystem::IsGameGameplay()
{
    UWorld* World = GetWorld();
    if (!World)
        return false;

    FString LevelName = UGameplayStatics::GetCurrentLevelName(World, true);

    const FString Prefix = TEXT("LV_Gameplay_");
    if (!LevelName.StartsWith(Prefix))
        return false;

    // Extraemos el número del nivel
    FString LevelNumberString = LevelName.RightChop(Prefix.Len());

    // Convertimos el string a número entero
    int32 LevelNumber = FCString::Atoi(*LevelNumberString);

    // Si el número es 1 → false
    if (LevelNumber == 1)
        return false;

    // Si está entre 2 y 13 → true
    if (LevelNumber >= 2 && LevelNumber <= 13)
        return true;

    return false;
}

AAzulInteractuableBase* UAzulGameSubsystem::GetCurrentHijoActor()
{
    UWorld* World = GetWorld();
    if (!World)
        return nullptr;

    AAzulCharacterBase* Character = Cast<AAzulCharacterBase>(
        UGameplayStatics::GetPlayerCharacter(World, 0)
    );

    if (!Character)
        return nullptr;

    for (const FGameplayTag& Tag : Character->ActiveStoryTags)
    {
        FString TagString = Tag.GetTagName().ToString();

        if (!TagString.StartsWith(TEXT("Gameplay.")))
            continue;

        FString NumberString = TagString.RightChop(9); // Quita "Gameplay."
        int32 LevelNumber = FCString::Atoi(*NumberString);

        // Gameplay.01
        if (LevelNumber == 1)
        {
            UClass* Clase = LoadClass<AAzulInteractuableBase>(
                nullptr,
                TEXT("/All/Game/Code/BP_Bebe.BP_Bebe_C")
            );

            return Cast<AAzulInteractuableBase>(
                UGameplayStatics::GetActorOfClass(World, Clase)
            );
        }

        // Gameplay.02 - 04
        if (LevelNumber >= 2 && LevelNumber <= 4)
        {
            UClass* Clase = LoadClass<AAzulInteractuableBase>(
                nullptr,
                TEXT("/All/Game/Code/BP_Niño.BP_Niño_C")
            );

            return Cast<AAzulInteractuableBase>(
                UGameplayStatics::GetActorOfClass(World, Clase)
            );
        }

        // Gameplay.05 - 09
        if (LevelNumber >= 5 && LevelNumber <= 9)
        {
            UClass* Clase = LoadClass<AAzulInteractuableBase>(
                nullptr,
                TEXT("/All/Game/Code/BP_Adolescente.BP_Adolescente_C")
            );

            return Cast<AAzulInteractuableBase>(
                UGameplayStatics::GetActorOfClass(World, Clase)
            );
        }

        // Gameplay.10 - 12
        if (LevelNumber >= 10 && LevelNumber <= 12)
        {
            UClass* Clase = LoadClass<AAzulInteractuableBase>(
                nullptr,
                TEXT("/All/Game/Code/BP_HijoAdulto.BP_HijoAdulto_C")
            );

            return Cast<AAzulInteractuableBase>(
                UGameplayStatics::GetActorOfClass(World, Clase)
            );
        }
    }

    return nullptr;
}



void UAzulGameSubsystem::OnSequenceFinished()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        APawn* Pawn = PC->GetPawn();
        AAzulCharacterBase* Character = Pawn ? Cast<AAzulCharacterBase>(Pawn) : nullptr;

        if (!Character)
        {
            UE_LOG(
                LogAzulCinematics,
                Warning,
                TEXT("OnSequenceFinished: Pawn no es AAzulCharacterBase (%s)"),
                *GetNameSafe(Pawn)
            );
            return;
        }

        // Devolvemos control solo si procede
        if (bRestoreControlAfterSequence && bEnableMoveLookAfterSequence)
        {
            Character->UnblockPlayerControl();
        }

        // Volvemos a mostrar el mesh si se ocultó
        if (bHideCharacterMeshDuringSequence)
        {
            if (USkeletalMeshComponent* Mesh = Character->GetMesh())
            {
                Mesh->SetVisibility(true, true);
            }
        }
    }

    // Avisar a Blueprint de QUÉ secuencia ha terminado
    OnLevelSequenceFinished.Broadcast(CurrentSequence);

    CurrentSequence = nullptr;

    SequencePlayer = nullptr;
}


void UAzulGameSubsystem::OnVideoFinished()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        APawn* Pawn = PC->GetPawn();
        AAzulCharacterBase* Character = Pawn ? Cast<AAzulCharacterBase>(Pawn) : nullptr;

        if (!Character)
        {
            UE_LOG(
                LogAzulCinematics,
                Warning,
                TEXT("OnVideoFinished: Pawn no es AAzulCharacterBase (%s)"),
                *GetNameSafe(Pawn)
            );
            return;
        }

        Character->UnblockPlayerControl();
    }

    if (ActiveMediaPlayer)
    {
        ActiveMediaPlayer->OnEndReached.RemoveDynamic(
            this,
            &UAzulGameSubsystem::OnVideoFinished
        );
    }

    ActiveMediaPlayer = nullptr;
}

void UAzulGameSubsystem::RegisterDialogue(UAzulDialogue* Dialogue)
{
    if (ActiveDialogue == Dialogue)
    {
        return;
    }

    if (ActiveDialogue)
    {
        ActiveDialogue->OnDialogueUpdated.RemoveDynamic(this, &UAzulGameSubsystem::OnActiveDialogueUpdated);
        ActiveDialogue->OnDialogueFinished.RemoveDynamic(this, &UAzulGameSubsystem::OnActiveDialogueFinished);
    }

    ActiveDialogue = Dialogue;

    if (ActiveDialogue)
    {
        ActiveDialogue->OnDialogueUpdated.AddDynamic(this, &UAzulGameSubsystem::OnActiveDialogueUpdated);
        ActiveDialogue->OnDialogueFinished.AddDynamic(this, &UAzulGameSubsystem::OnActiveDialogueFinished);
    }

    RefreshDialogueWidget();
}

void UAzulGameSubsystem::ClearDialogue()
{
    if (WidgetDialogue)
    {
        WidgetDialogue->Dialogue = ActiveDialogue;
        WidgetDialogue->SetDialogueText(FString());

        if (WidgetDialogue->TextName)
        {
            WidgetDialogue->TextName->SetText(FText::GetEmpty());
        }

        WidgetDialogue->SetVisibility(ESlateVisibility::Collapsed);
        SetInputForDialogue(false);
    }
}

void UAzulGameSubsystem::RequestAdvanceDialogue()
{
    if (!ActiveDialogue)
        return;

    ActiveDialogue->ContinueDialogue();
}

void UAzulGameSubsystem::ChangePlanetsToCompleted() {
    bPlanetsCompleted = true;
    UE_LOG(LogTemp, Warning, TEXT("PlanetsCompleted"));
}