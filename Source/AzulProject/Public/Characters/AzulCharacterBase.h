#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameplayTagContainer.h"
//#include "Actors/AzulStoryObjectBase.h"
#include "Interfaces/AzulInteractuableInterface.h"
#include "InputMappingContext.h"            
#include "EnhancedInputSubsystems.h" 
//#include "AzulComponentes/AzulBolsoComponent.h"
#include "Actors/AzulHiloBase.h"
#include "AzulComponentes/AzulHiloComponent.h"
#include "InputAction.h"
#include "Widgets/AzulWidgetHUDPlayer.h"
#include "AzulCharacterBase.generated.h"


UENUM(BlueprintType)
enum class EAzulControlMode : uint8
{
	Default,
	Menu,
	Look,
	Disabled
};


UCLASS()
class AZULPROJECT_API AAzulCharacterBase : public ACharacter, public IAzulInteractuableInterface
{
	GENERATED_BODY()

public:
	AAzulCharacterBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	//-------------------------INTERACTUAR----------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Interactuable")
	TScriptInterface<IAzulInteractuableInterface> CurrentInteractable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Interactuable")
	TArray<TScriptInterface<IAzulInteractuableInterface>> OverlappingInteractables;

	void AddInteractable(TScriptInterface<IAzulInteractuableInterface> Interactable);
	void RemoveInteractable(TScriptInterface<IAzulInteractuableInterface> Interactable);

	// --- EXCEPCIONES DE MIRILLA ---
	UPROPERTY()
	TArray<AActor*> OverlappingExceptionActors;

	UPROPERTY()
	AActor* CurrentExceptionActor = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azul|InteractionExceptions")
	TArray<TSubclassOf<AActor>> InteractableClassExceptions;


	bool IsExceptionInteractable(AActor* Actor) const;

	UFUNCTION(BlueprintCallable, Category = "Azul|InteractionExceptions")
	void AddInteractableException(AActor* Actor);
	
	UFUNCTION(BlueprintCallable, Category = "Azul|InteractionExceptions")
	void RemoveInteractableException(AActor* Actor);

	UPROPERTY(BlueprintReadOnly, Category = "Azul|Interactuable")
	bool bCanInteract = false;


	//---------------------------BIBERÓN-----------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Biberon")
	bool bHasBiberon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Biberon")
	bool bDoneBiberon = false;

	//------------------------JUGUETE-------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Juguete")
	bool bHasJuguete = false;

	//---------------------------------STORY--------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Story")
	FGameplayTagContainer ActiveStoryTags;

	UFUNCTION(BlueprintCallable, Category = "Azul|Story")
	void AddStoryTag(const FGameplayTag& NewTag);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Azul|Story")
	FString SonName;

	UFUNCTION(BlueprintCallable, Category = "Azul|Story")
	void SetCurrentGameplayTag();

	//---------------------------BOLSO---------------------------------------------
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Azul|Bolso")
	//UAzulBolsoComponent* BolsoComponent;

	//---------------------------HILO--------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Hilo")
	AAzulHiloBase* HiloActor = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Azul|Input")
	UInputAction* IA_MostrarHilo;

	UPROPERTY(BlueprintReadOnly)
	bool bMovementLockedByHilo = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Azul|Hilo")
	void BP_OnHiloShown();

	UFUNCTION(BlueprintImplementableEvent, Category = "Azul|Hilo")
	void BP_OnHiloHidden();

	// Llamadas desde componentes
	void NotifyHiloShown();

	UFUNCTION()
	void NotifyHiloHidden();

	FTimerHandle HiloTimer;

	UFUNCTION()
	void OnSpacePressed();

	//-----------------------------MIRILLA--------------------------------------
	// Instancia real del widget
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Azul|Mirilla")
	UAzulWidgetHUDPlayer* HUDWidget;
	
	UFUNCTION()
	void OpenMirilla();

	UFUNCTION(BlueprintImplementableEvent, Category = "Azul|Hilo")
	void BP_OpenMirilla();

	UFUNCTION()
	void CheckCrosshairTrace();

	//----------------------------TUTORIAL----------------------------------
	bool bIsReadyToMoveTutorial = false;

	UPROPERTY(BlueprintReadOnly)
	bool bTutorialForbidMovementWhileHilo = false;

	UPROPERTY(BlueprintReadWrite, Category = "Azul|Tutorial")
	bool bTutorialAllowMovement = false;

	UFUNCTION(BlueprintCallable)
	bool CanMoveAccordingToTutorial() const;


	//----------------------------INPUT-------------------------------------------

	// Input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Control modes
	void SetControlMode(EAzulControlMode NewMode);

	// Estados
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Azul|Input")
	EAzulControlMode CurrentControlMode = EAzulControlMode::Default;

	// IMC preparados
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azul|Input")
	TMap<EAzulControlMode, UInputMappingContext*> MappingContexts;

	//CONTROLES PARA CINEMÁTICAS
	void BlockPlayerControl();
	void UnblockPlayerControl();

private:
	bool bIsBlocked = false;

};
