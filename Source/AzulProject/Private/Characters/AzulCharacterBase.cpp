#include "Characters/AzulCharacterBase.h"
//#include "Actors/AzulStoryObjectBase.h"
//#include "Widgets/AzulWidgetBolsoBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Interfaces/AzulInteractuableInterface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "AzulSubsystem/AzulTutorialSubsystem.h"
#include "AzulSubsystem/AzulGameSubsystem.h"
#include "GameplayTagContainer.h"
#include "Actors/AzulInteractuableBase.h"


// Sets default values
AAzulCharacterBase::AAzulCharacterBase()
{
    PrimaryActorTick.bCanEverTick = true;

    //BolsoComponent = CreateDefaultSubobject<UAzulBolsoComponent>(TEXT("BolsoComponent"));

    //HiloComponent = CreateDefaultSubobject<UAzulHiloComponent>(TEXT("HiloComponent"));

    CurrentInteractable = nullptr;
}

// Called when the game starts or when spawned
void AAzulCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    //if (BolsoComponent)
    //{
    //    BolsoComponent->InitializeBolso(this);
    //}

    if (HiloActor)
    {
        HiloActor->OnHiloHidden.AddDynamic(
            this,
            &AAzulCharacterBase::NotifyHiloHidden
        );
    }

    // --- AÑADIDO MANUAL DE INTERACTUABLES INICIALES ---
    TArray<AActor*> FoundActors;

    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        AAzulInteractuableBase::StaticClass(),
        FoundActors
    );

    for (AActor* Actor : FoundActors)
    {
        AAzulInteractuableBase* Interactable =
            Cast<AAzulInteractuableBase>(Actor);

        if (!Interactable)
            continue;

        // AQUÍ decides cuáles sí y cuáles no
        const FString Name = Interactable->GetName();

        if (Name == TEXT("BP_InteractuableTexto_C_2") ||
            Name == TEXT("BP_InteractuableTexto_C_1"))
        {
            AddInteractable(
                TScriptInterface<IAzulInteractuableInterface>(Interactable)
            );
        }
    }

    SetCurrentGameplayTag();

}

// Called every frame
void AAzulCharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CheckCrosshairTrace();
}

void AAzulCharacterBase::AddStoryTag(const FGameplayTag& NewTag)
{
    ActiveStoryTags.AddTag(NewTag);
}

void AAzulCharacterBase::SetCurrentGameplayTag()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    FString LevelName = UGameplayStatics::GetCurrentLevelName(World, true);

    const FString Prefix = TEXT("LV_Gameplay_");
    if (!LevelName.StartsWith(Prefix))
        return;

    FString LevelNumber = LevelName.RightChop(Prefix.Len());
    FString TagString = FString::Printf(TEXT("Gameplay.%s"), *LevelNumber);

    FGameplayTag LevelTag = FGameplayTag::RequestGameplayTag(FName(*TagString), false);
    if (!LevelTag.IsValid())
        return;

    FGameplayTagContainer& ActiveTags = ActiveStoryTags;

    // Obtener el tag padre
    FGameplayTag GameplayParentTag =
        FGameplayTag::RequestGameplayTag(FName("Gameplay"));

    // 1️ Crear lista temporal
    TArray<FGameplayTag> TagsToRemove;

    for (const FGameplayTag& Tag : ActiveTags)
    {
        if (Tag.MatchesTag(GameplayParentTag))
        {
            TagsToRemove.Add(Tag);
        }
    }

    // 2️ Eliminar los encontrados
    for (const FGameplayTag& Tag : TagsToRemove)
    {
        ActiveTags.RemoveTag(Tag);
    }

    // 3️ Añadir el nuevo
    ActiveTags.AddTag(LevelTag);


}


//------------------------------INPUT------------------------------

void AAzulCharacterBase::SetControlMode(EAzulControlMode NewMode)
{
    if (CurrentControlMode == NewMode)
        return;

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer) return;

    UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
        LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

    if (!InputSubsystem) return;

    // Quitar IMC anterior
    if (MappingContexts.Contains(CurrentControlMode))
    {
        InputSubsystem->RemoveMappingContext(
            MappingContexts[CurrentControlMode]
        );
    }

    // Añadir IMC nuevo
    if (MappingContexts.Contains(NewMode))
    {
        InputSubsystem->AddMappingContext(
            MappingContexts[NewMode],
            0
        );
    }

    CurrentControlMode = NewMode;
}


void AAzulCharacterBase::BlockPlayerControl()
{
    if (bIsBlocked) return;
    bIsBlocked = true;

    // 1️⃣ Bloquear movimiento
    GetCharacterMovement()->DisableMovement();

    // 2️⃣ Bloquear input
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetIgnoreLookInput(true); // Bloquea rotación

    //    DisableInput(PC);
    }
}

void AAzulCharacterBase::UnblockPlayerControl()
{
    if (!bIsBlocked) return;
    bIsBlocked = false;

    // 1Restaurar movimiento
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);

    // 2Restaurar input
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetIgnoreLookInput(false);
    //    EnableInput(PC);

        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}


void AAzulCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent CALLED"));

     if (UEnhancedInputComponent* EIC =
        Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent FOUND"));

        EIC->BindAction(
            IA_MostrarHilo,
            ETriggerEvent::Started,
            this,
            &AAzulCharacterBase::OnSpacePressed
        );
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EnhancedInputComponent NOT FOUND"));
    }
}


void AAzulCharacterBase::OnSpacePressed()
{
    //--- TUTORIAL
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAzulTutorialSubsystem* TutorialSubsystem =
            GI->GetSubsystem<UAzulTutorialSubsystem>())
        {
            if (TutorialSubsystem->IsTutorialActive())
            {
                const FGameplayTag SpaceTag =
                    FGameplayTag::RequestGameplayTag(TEXT("Tutorial.First.Space"));

                if (!TutorialSubsystem->IsActionCompleted(SpaceTag)) {
                    TutorialSubsystem->NotifyActionCompleted(
                        FGameplayTag::RequestGameplayTag("Tutorial.First.Space")
                    );
                }
            }
        }
    }

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAzulGameSubsystem* GameSubsystem =
            GI->GetSubsystem<UAzulGameSubsystem>())
        {
            if (!GameSubsystem->IsSequenceActive())
            {
                // --- HILO ---
                if (HiloActor->IsHiloVisible())
                {
                    HiloActor->ForceHideHilo();
                }
                else
                {
                    HiloActor->ShowHilo();
                    NotifyHiloShown();
                }
            }
        }
    }
}

//----------------------------------------------HILO

void AAzulCharacterBase::NotifyHiloShown()
{
    bMovementLockedByHilo = true;
    GetCharacterMovement()->DisableMovement();
    BP_OnHiloShown();
}

void AAzulCharacterBase::NotifyHiloHidden()
{
    bMovementLockedByHilo = false;

    if (CanMoveAccordingToTutorial())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }

    BP_OnHiloHidden();
}

//------------------------------------TUTORIAL
bool AAzulCharacterBase::CanMoveAccordingToTutorial() const
{
    // Si el tutorial no está activo → movimiento normal
    if (!GetGameInstance())
        return true;

    const UAzulTutorialSubsystem* TutorialSubsystem =
        GetGameInstance()->GetSubsystem<UAzulTutorialSubsystem>();

    if (!TutorialSubsystem || !TutorialSubsystem->IsTutorialActive())
        return true;

    // No se permite movimiento todavía
    if (!bTutorialAllowMovement)
        return false;

    // Caso especial: hilo activo bloquea movimiento
    if (bTutorialForbidMovementWhileHilo && bMovementLockedByHilo)
        return false;

    return true;
}


//------------------------------------MIRILLA

void AAzulCharacterBase::OpenMirilla()
{
    BP_OpenMirilla();
}




void AAzulCharacterBase::CheckCrosshairTrace()
{
    if (!HUDWidget)
        return;

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
        return;

    FVector WorldLocation;
    FVector WorldDirection;

    const FVector2D ViewportSize =
        FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());

    const FVector2D ScreenCenter(
        ViewportSize.X * 0.5f,
        ViewportSize.Y * 0.5f
    );

    if (!PC->DeprojectScreenPositionToWorld(
        ScreenCenter.X,
        ScreenCenter.Y,
        WorldLocation,
        WorldDirection))
    {
        bCanInteract = false;
        HUDWidget->SetUIState(EInteractUIState::Default);
        return;
    }

    const float TraceDistance = 350.f;

    FVector Start = WorldLocation;
    FVector End = Start + WorldDirection * TraceDistance;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        Start,
        End,
        ECC_Visibility,
        Params
    );

    bool bValidHit = false;

    CurrentInteractable = nullptr;
    CurrentExceptionActor = nullptr;

    if (bHit && Hit.GetActor() && Hit.GetComponent())
    {
        //if (GEngine)
        //{
        //    FString DebugMessage = FString::Printf(
        //        TEXT("Actor: %s | Comp: %s"),
        //        *Hit.GetActor()->GetName(),
        //        *Hit.GetComponent()->GetName()
        //    );

        //    GEngine->AddOnScreenDebugMessage(
        //        -1,             // Key (-1 = nuevo mensaje cada vez)
        //        2.0f,           // Duración en segundos
        //        FColor::Yellow, // Color
        //        DebugMessage
        //    );
        //}

        // ---- INTERACTUABLES NORMALES ----
        for (const TScriptInterface<IAzulInteractuableInterface>& Interactable : OverlappingInteractables)
        {
            AAzulInteractuableBase* InteractableActor =
                Cast<AAzulInteractuableBase>(Interactable.GetObject());

            if (!InteractableActor)
                continue;

            if (Hit.GetActor() != InteractableActor)
                continue;

            if (InteractableActor->IsValidInteractionComponent(
                Cast<UPrimitiveComponent>(Hit.GetComponent())))
            {
                bValidHit = true;
                CurrentInteractable = Interactable;
                break;
            }
        }

        // ---- EXCEPCIONES ----
        if (!bValidHit && IsExceptionInteractable(Hit.GetActor()))
        {
            bool bCanSetInteractable = true;

            const FString ActorName = Hit.GetActor()->GetName();

            // 🔒 ARMARIOS → solo Pomo / Pomo1
            if (ActorName.Contains(TEXT("BP_Armarios")))
            {
                bCanSetInteractable = false;

                if (UStaticMeshComponent* HitMesh =
                    Cast<UStaticMeshComponent>(Hit.GetComponent()))
                {
                    const FName MeshName = HitMesh->GetFName();

                    if (MeshName == TEXT("Pomo") || MeshName == TEXT("Pomo1"))
                    {
                        bCanSetInteractable = true;
                    }
                }
            }

            // 🔒 BAULONGO → solo ParaAbrir
            else if (ActorName.Contains(TEXT("BP_Baulongo")))
            {
                bCanSetInteractable = false;

                if (UStaticMeshComponent* HitMesh =
                    Cast<UStaticMeshComponent>(Hit.GetComponent()))
                {
                    const FName MeshName = HitMesh->GetFName();

                    if (MeshName == TEXT("ParaAbrir"))
                    {
                        bCanSetInteractable = true;
                    }
                }
            }

            // ✅ Solo si se puede interactuar
            if (bCanSetInteractable &&
                Hit.GetActor()->GetClass()->ImplementsInterface(
                    UAzulInteractuableInterface::StaticClass()))
            {
                CurrentInteractable =
                    TScriptInterface<IAzulInteractuableInterface>(Hit.GetActor());
            }

            // 🔑 AQUÍ ESTÁ LA CLAVE
            bValidHit = bCanSetInteractable;
            CurrentExceptionActor = bCanSetInteractable ? Hit.GetActor() : nullptr;
        }
    }

    bCanInteract = bValidHit;

    HUDWidget->SetUIState(
        bCanInteract
        ? EInteractUIState::Active
        : EInteractUIState::Default
    );
}



void AAzulCharacterBase::AddInteractable(
    TScriptInterface<IAzulInteractuableInterface> Interactable)
{
    if (!Interactable.GetObject())
        return;

    OverlappingInteractables.AddUnique(Interactable);
}


void AAzulCharacterBase::RemoveInteractable(
    TScriptInterface<IAzulInteractuableInterface> Interactable)
{
    if (!Interactable.GetObject())
        return;

    OverlappingInteractables.Remove(Interactable);

    if (OverlappingInteractables.Num() == 0 &&
        OverlappingExceptionActors.Num() == 0)
    {
        CurrentInteractable = nullptr;
    }
}


bool AAzulCharacterBase::IsExceptionInteractable(AActor* Actor) const
{
    if (!Actor)
        return false;

    for (const TSubclassOf<AActor>& ExceptionClass : InteractableClassExceptions)
    {
        if (!ExceptionClass)
            continue;

        if (Actor->IsA(ExceptionClass))
        {
            return true;
        }
    }

    return false;
}


void AAzulCharacterBase::AddInteractableException(AActor* Actor)
{
    if (!Actor)
        return;

    if (!IsExceptionInteractable(Actor))
        return;

    OverlappingExceptionActors.AddUnique(Actor);
}


void AAzulCharacterBase::RemoveInteractableException(AActor* Actor)
{
    OverlappingExceptionActors.Remove(Actor);

    if (OverlappingInteractables.Num() == 0 &&
        OverlappingExceptionActors.Num() == 0)
    {
        CurrentExceptionActor = nullptr;
    }
}