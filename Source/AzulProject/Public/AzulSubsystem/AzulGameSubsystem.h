/*
 * UAzulGameSubsystem
 * ------------------------------------------------------
 * Este subsystem centraliza sistemas globales del juego
 * que deben mantenerse accesibles entre niveles.
 *
 * Responsabilidades principales:
 * - Gestionar el diálogo activo y su widget.
 * - Crear y mantener widgets persistentes del juego.
 * - Servir de puente entre Blueprints y sistemas globales.
 * - Controlar cinemáticas, vídeos y bloqueo del jugador.
 * - Guardar variables globales compartidas entre niveles
 *   (por ejemplo SonName, estados y progreso).
 *
 * En el sistema de diálogo, este subsystem actúa como
 * gestor central: conserva la instancia activa del diálogo
 * y actualiza el widget cuando el contenido cambia.
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "Widgets/AzulWidgetHUDPlayer.h"
#include "Dialogos/AzulWidgetDialogueBase.h"
#include "AzulGameSubsystem.generated.h"

class UAzulDialogue;
class AAzulInteractuableBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnLevelSequenceFinished,
    ULevelSequence*, Sequence
);

UCLASS()
class AZULPROJECT_API UAzulGameSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    // Se llama cuando el subsystem se inicializa
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    //-------------------------------------NAME
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Azul|SonName")
    FString SonName;

    //---------------------------------------------------DIALOGOS

    /* Diálogo actualmente activo (si lo hay) */
    UPROPERTY(BlueprintReadOnly, Category = "Azul|Narrative")
    UAzulDialogue* ActiveDialogue = nullptr;

    /* Registrar un diálogo como activo */
    UFUNCTION()
    void RegisterDialogue(UAzulDialogue* Dialogue);

    /* Finalizar narrativa activa */
    UFUNCTION()
    void ClearDialogue();

    /* Petición genérica de avance (input, tutorial, etc.) */
    UFUNCTION()
    void RequestAdvanceDialogue();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azul|Dialogue")
    TSubclassOf<UAzulWidgetDialogueBase> WidgetDialogueClass;

    UPROPERTY(BlueprintReadOnly, Category = "Azul|Dialogue")
    UAzulWidgetDialogueBase* WidgetDialogue = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Azul|Dialogue")
    void RegisterDialogueWidget(UAzulWidgetDialogueBase* InWidget);

    UFUNCTION()
    void RefreshDialogueWidget();

    UFUNCTION()
    FString GetActiveDialogueText() const;

    UFUNCTION()
    FString GetDialogueTextFromWidget() const;

    UFUNCTION(BlueprintCallable, Category = "Azul|Dialogue")
    void OpenDialogue(UDataTable* InDialogueTable, bool bRestart = true, int32 StartID = 1);

    UFUNCTION()
    void CreateDialogueSystem();

    UFUNCTION()
    void SetInputForDialogue(bool bEnable);

    //-----------------------------------------------------------CINEMÁTICAS

    UFUNCTION(BlueprintCallable, Category = "Azul|Cinematics")
    void PlayLevelSequence(
        ULevelSequence* Sequence,
        bool bRestoreControlOnFinish = true,
        bool bHideCharacterMesh = true
    );

    UPROPERTY(BlueprintAssignable, Category = "Azul|Cinematics")
    FOnLevelSequenceFinished OnLevelSequenceFinished;

    UFUNCTION(BlueprintCallable, Category = "Azul|Cinematics")
    void PlayVideo(UMediaPlayer* MediaPlayer, UMediaSource* MediaSource);

    UFUNCTION()
    bool IsSequenceActive();

	//--------------------------WIDGET HUD PLAYER
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azul|HUD Player")
    TSubclassOf<UAzulWidgetHUDPlayer> WidgetHUDPlayerClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|HUD Player")
    UAzulWidgetHUDPlayer* WidgetHUDPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|HUD Player|Mirilla")
    bool bIsLookingItem = false;

    void OnLevelLoaded(UWorld* LoadedWorld);

    //-----------------------------STORY TEXTS PROPS
    UFUNCTION(BlueprintCallable, Category = "Azul|StoryProps")
    FName GetCurrentGameplayName();

    UFUNCTION(BlueprintCallable, Category = "Azul|StoryProps")
    bool IsGameGameplay();

    //---------------------------------HILO
    UFUNCTION(BlueprintCallable, Category = "Azul|Hilo")
    AAzulInteractuableBase* GetCurrentHijoActor();

    //----------------------------------CUADROS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Cuadros")
    bool bBlendFinishedCR= true;


    //-------------------------PLANETAS
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Azul|Planetas")
    bool  bPlanetsCompleted = false;
    UFUNCTION(BlueprintCallable, Category = "Azul|Planetas")
    void ChangePlanetsToCompleted();

    //-------------------------ATICO
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Atico")
    FString LastLevelBeforeAtico;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Atico")
    bool bOpenFromAtico;


private:

    UPROPERTY()
    ULevelSequencePlayer* SequencePlayer = nullptr;

    UPROPERTY()
    ULevelSequence* CurrentSequence = nullptr;

    UPROPERTY()
    UMediaPlayer* ActiveMediaPlayer = nullptr;

    UPROPERTY()
    bool bRestoreControlAfterSequence = true;

    UPROPERTY()
    bool bHideCharacterMeshDuringSequence = true;

    UPROPERTY()
    bool bEnableMoveLookAfterSequence = true;



    UFUNCTION()
    void OnSequenceFinished();

    UFUNCTION()
    void OnVideoFinished();

    UFUNCTION()
    void OnActiveDialogueUpdated();

    UFUNCTION()
    void OnActiveDialogueFinished();
};
