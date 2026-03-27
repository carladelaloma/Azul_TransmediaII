#include "AzulSubsystem/AzulGameSubsystem.h"
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

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    if (IsGameGameplay())
    {
        if (WidgetHUDPlayerClass) {
            WidgetHUDPlayer = CreateWidget<UAzulWidgetHUDPlayer>(
                PC,
                WidgetHUDPlayerClass
            );

            if (WidgetHUDPlayer)
            {
                WidgetHUDPlayer->AddToViewport();
                WidgetHUDPlayer->SetVisibility(ESlateVisibility::Visible);
            }
        }
       
        if (WidgetDialogueClass)
        {
            WidgetDialogue = CreateWidget<UAzulWidgetDialogueBase>(
                PC,
                WidgetDialogueClass
            );

            if (WidgetDialogue)
            {
                WidgetDialogue->AddToViewport();
                WidgetDialogue->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
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
        WidgetDialogue->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    WidgetDialogue->Dialogue = ActiveDialogue;
    WidgetDialogue->SetDialogueText(GetActiveDialogueText());
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
    ClearDialogue();
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
    ActiveDialogue = Dialogue;
}

void UAzulGameSubsystem::ClearDialogue()
{
    ActiveDialogue = nullptr;
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