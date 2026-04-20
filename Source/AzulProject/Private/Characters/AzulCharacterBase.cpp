#include "Characters/AzulCharacterBase.h"
//#include "Widgets/AzulWidgetBolsoBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Interfaces/AzulInteractuableInterface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"
#include "AzulComponentes/AzulBolsoComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "AzulSubsystem/AzulTutorialSubsystem.h"
#include "AzulSubsystem/AzulGameSubsystem.h"
#include "GameplayTagContainer.h"
#include "Actors/AzulInteractuableBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogAzulCharacter, Log, All);

// Sets default values
AAzulCharacterBase::AAzulCharacterBase()
{
    PrimaryActorTick.bCanEverTick = true;

    BolsoComponent = CreateDefaultSubobject<UAzulBolsoComponent>(TEXT("BolsoComponent"));

    CurrentInteractable = nullptr;

    UE_LOG(LogAzulCharacter, Warning, TEXT("[CTOR] Character creado: %s"), *GetNameSafe(this));
}

// Called when the game starts or when spawned
void AAzulCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogAzulCharacter, Warning, TEXT("[BeginPlay] Character=%s | HiloActor=%s | Controller=%s | HUDWidget=%s"),
        *GetNameSafe(this),
        *GetNameSafe(HiloActor),
        *GetNameSafe(GetController()),
        *GetNameSafe(HUDWidget));

    if (HiloActor)
    {
        HiloActor->OnHiloHidden.AddDynamic(
            this,
            &AAzulCharacterBase::NotifyHiloHidden
        );

        UE_LOG(LogAzulCharacter, Warning, TEXT("[BeginPlay] Bind a OnHiloHidden hecho correctamente con %s"),
            *GetNameSafe(HiloActor));
    }

    // --- AÑADIDO MANUAL DE INTERACTUABLES INICIALES ---
    TArray<AActor*> FoundActors;

    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        AAzulInteractuableBase::StaticClass(),
        FoundActors
    );

    UE_LOG(LogAzulCharacter, Warning, TEXT("[BeginPlay] Interactuables encontrados=%d"), FoundActors.Num());

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

            UE_LOG(LogAzulCharacter, Warning, TEXT("[BeginPlay] Interactable inicial añadido: %s"),
                *GetNameSafe(Interactable));
        }

    }

    SetCurrentGameplayTag();

    UE_LOG(LogAzulCharacter, Warning, TEXT("[BeginPlay] ActiveStoryTags inicializados"));
}

// Called every frame
void AAzulCharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CheckCrosshairTrace();

    //LOG
    static float TickAccumulator = 0.0f;
    TickAccumulator += DeltaTime;
    if (TickAccumulator >= 2.0f)
    {
        TickAccumulator = 0.0f;

        UE_LOG(LogAzulCharacter, Verbose, TEXT("[Tick] Character=%s | HiloActor=%s | bMovementLockedByHilo=%d | bCanInteract=%d"),
            *GetNameSafe(this),
            *GetNameSafe(HiloActor),
            bMovementLockedByHilo,
            bCanInteract);
    }
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

    UE_LOG(LogAzulCharacter, Warning, TEXT("[SetupPlayerInputComponent] CALLED | Character=%s | InputComponent=%s | IA_MostrarHilo=%s"),
        *GetNameSafe(this),
        *GetNameSafe(PlayerInputComponent),
        *GetNameSafe(IA_MostrarHilo));

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        UE_LOG(LogAzulCharacter, Warning, TEXT("[SetupPlayerInputComponent] EnhancedInputComponent FOUND"));

        if (IA_MostrarHilo)
        {
            EIC->BindAction(
                IA_MostrarHilo,
                ETriggerEvent::Started,
                this,
                &AAzulCharacterBase::OnSpacePressed
            );

            UE_LOG(LogAzulCharacter, Warning, TEXT("[SetupPlayerInputComponent] IA_MostrarHilo bind OK"));
        }
        else
        {
            UE_LOG(LogAzulCharacter, Error, TEXT("[SetupPlayerInputComponent] IA_MostrarHilo es NULL"));
        }
    }
    else
    {
        UE_LOG(LogAzulCharacter, Error, TEXT("[SetupPlayerInputComponent] EnhancedInputComponent NOT FOUND"));
    }
}


void AAzulCharacterBase::OnSpacePressed()
{
    UE_LOG(LogAzulCharacter, Warning, TEXT("[OnSpacePressed] START | Character=%s | HiloActor=%s"),
        *GetNameSafe(this),
        *GetNameSafe(HiloActor));

    if (!HiloActor)
    {
        UE_LOG(LogAzulCharacter, Error, TEXT("[OnSpacePressed] ABORT | HiloActor es NULL"));
        return;
    }

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAzulTutorialSubsystem* TutorialSubsystem = GI->GetSubsystem<UAzulTutorialSubsystem>())
        {
            const bool bTutorialActive = TutorialSubsystem->IsTutorialActive();

            UE_LOG(LogAzulCharacter, Warning, TEXT("[OnSpacePressed] TutorialSubsystem OK | IsTutorialActive=%d"), bTutorialActive);

            if (bTutorialActive)
            {
                const FGameplayTag SpaceTag =
                    FGameplayTag::RequestGameplayTag(TEXT("Tutorial.First.Space"));

                const bool bCompleted = TutorialSubsystem->IsActionCompleted(SpaceTag);

                UE_LOG(LogAzulCharacter, Warning, TEXT("[OnSpacePressed] Tutorial tag Tutorial.First.Space completed=%d"), bCompleted);

                if (!bCompleted)
                {
                    TutorialSubsystem->NotifyActionCompleted(
                        FGameplayTag::RequestGameplayTag(TEXT("Tutorial.First.Space"))
                    );

                    UE_LOG(LogAzulCharacter, Warning, TEXT("[OnSpacePressed] Tutorial.First.Space marcado como completado"));
                }
            }
        }
        else
        {
            UE_LOG(LogAzulCharacter, Error, TEXT("[OnSpacePressed] TutorialSubsystem es NULL"));
        }
    }
    else
    {
        UE_LOG(LogAzulCharacter, Error, TEXT("[OnSpacePressed] GameInstance es NULL"));
    }

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAzulGameSubsystem* GameSubsystem = GI->GetSubsystem<UAzulGameSubsystem>())
        {
            const bool bSequenceActive = GameSubsystem->IsSequenceActive();

            UE_LOG(LogAzulCharacter, Warning, TEXT("[OnSpacePressed] GameSubsystem OK | IsSequenceActive=%d"), bSequenceActive);

            if (!bSequenceActive)
            {
                const bool bVisible = HiloActor->IsHiloVisible();

                UE_LOG(LogAzulCharacter, Warning, TEXT("[OnSpacePressed] Hilo visible antes de toggle=%d"), bVisible);

                if (bVisible)
                {
                    UE_LOG(LogAzulCharacter, Warning, TEXT("[OnSpacePressed] Ejecutando ForceHideHilo"));
                    HiloActor->ForceHideHilo();
                }
                else
                {
                    UE_LOG(LogAzulCharacter, Warning, TEXT("[OnSpacePressed] Ejecutando ShowHilo"));
                    HiloActor->ShowHilo();

                    // SOLO si realmente ha quedado visible marcamos el estado y BP
                    if (HiloActor->IsHiloVisible())
                    {
                        NotifyHiloShown();
                    }
                    else
                    {
                        UE_LOG(LogAzulCharacter, Error, TEXT("[OnSpacePressed] ShowHilo fue llamado pero el hilo NO quedó visible"));
                    }
                }
            }
            else
            {
                UE_LOG(LogAzulCharacter, Warning, TEXT("[OnSpacePressed] Bloqueado porque hay secuencia activa"));
            }
        }
        else
        {
            UE_LOG(LogAzulCharacter, Error, TEXT("[OnSpacePressed] GameSubsystem es NULL"));
        }
    }
}

//----------------------------------------------HILO

void AAzulCharacterBase::NotifyHiloShown()
{
    bMovementLockedByHilo = true;
    GetCharacterMovement()->DisableMovement();

    UE_LOG(LogAzulCharacter, Warning, TEXT("[NotifyHiloShown] Character=%s | bMovementLockedByHilo=%d | MovementMode=%d"),
        *GetNameSafe(this),
        bMovementLockedByHilo,
        GetCharacterMovement() ? (int32)GetCharacterMovement()->MovementMode : -1);

    BP_OnHiloShown();
}

void AAzulCharacterBase::NotifyHiloHidden()
{
    bMovementLockedByHilo = false;

    const bool bCanMove = CanMoveAccordingToTutorial();

    UE_LOG(LogAzulCharacter, Warning, TEXT("[NotifyHiloHidden] Character=%s | bMovementLockedByHilo=%d | CanMoveAccordingToTutorial=%d"),
        *GetNameSafe(this),
        bMovementLockedByHilo,
        bCanMove);

    if (bCanMove)
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);

        UE_LOG(LogAzulCharacter, Warning, TEXT("[NotifyHiloHidden] MovementMode restaurado a WALKING"));
    }
    else
    {
        UE_LOG(LogAzulCharacter, Warning, TEXT("[NotifyHiloHidden] Movimiento sigue bloqueado por tutorial"));
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