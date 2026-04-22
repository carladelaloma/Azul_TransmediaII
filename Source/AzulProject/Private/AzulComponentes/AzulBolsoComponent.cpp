#include "AzulComponentes/AzulBolsoComponent.h"
#include "Characters/AzulCharacterBase.h"
#include "Widgets/AzulWidgetBolsoBase.h"
#include "Interfaces/AzulBolsoInterface.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogAzulBolso, Log, All);

UAzulBolsoComponent::UAzulBolsoComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    UE_LOG(LogAzulBolso, Warning, TEXT("CTOR BolsoComponent creado"));
}

void UAzulBolsoComponent::InitializeBolso(AAzulCharacterBase* InOwner)
{
    OwnerCharacter = InOwner;

    UE_LOG(LogAzulBolso, Warning, TEXT("InitializeBolso START | Owner=%s"), *GetNameSafe(OwnerCharacter));

    Bolso.SetNum(2);
    Bolso[0] = FItemData();
    Bolso[1] = FItemData();

    const bool bGameplay03 = IsGameplay03();
    UE_LOG(LogAzulBolso, Warning, TEXT("InitializeBolso | IsGameplay03=%s"), bGameplay03 ? TEXT("true") : TEXT("false"));

    if (!bGameplay03)
    {
        UE_LOG(LogAzulBolso, Warning, TEXT("InitializeBolso | Abortado porque no estamos en Gameplay03"));
        return;
    }

    if (!BolsoWidgetClass)
    {
        UE_LOG(LogAzulBolso, Error, TEXT("InitializeBolso | BolsoWidgetClass es NULL"));
        return;
    }

    if (!OwnerCharacter)
    {
        UE_LOG(LogAzulBolso, Error, TEXT("InitializeBolso | OwnerCharacter es NULL"));
        return;
    }

    BolsoWidgetInstance = CreateWidget<UAzulWidgetBolsoBase>(
        OwnerCharacter->GetWorld(),
        BolsoWidgetClass
    );

    UE_LOG(LogAzulBolso, Warning, TEXT("InitializeBolso | Widget creado=%s"), *GetNameSafe(BolsoWidgetInstance));

    if (BolsoWidgetInstance)
    {
        BolsoWidgetInstance->OwnerCharacter = OwnerCharacter;
        BolsoWidgetInstance->SlotItems.SetNum(2);
        BolsoWidgetInstance->UpdateUI();
        BolsoWidgetInstance->AddToViewport();

        UE_LOG(LogAzulBolso, Warning, TEXT("InitializeBolso | Widget añadido al viewport"));
    }
}

bool UAzulBolsoComponent::TryAddItem(AActor* WorldActor)
{
    UE_LOG(LogAzulBolso, Warning, TEXT("TryAddItem START | Actor=%s"), *GetNameSafe(WorldActor));

    if (!WorldActor)
    {
        UE_LOG(LogAzulBolso, Error, TEXT("TryAddItem | WorldActor es NULL"));
        return false;
    }

    if (!WorldActor->GetClass()->ImplementsInterface(UAzulBolsoInterface::StaticClass()))
    {
        UE_LOG(LogAzulBolso, Error, TEXT("TryAddItem | %s NO implementa UAzulBolsoInterface"), *GetNameSafe(WorldActor));
        return false;
    }

    const bool bCanGoToBolso = IAzulBolsoInterface::Execute_CanGoToBolso(WorldActor);
    UE_LOG(LogAzulBolso, Warning, TEXT("TryAddItem | CanGoToBolso=%s"), bCanGoToBolso ? TEXT("true") : TEXT("false"));

    if (!bCanGoToBolso)
    {
        UE_LOG(LogAzulBolso, Warning, TEXT("TryAddItem | El actor no puede ir al bolso"));
        return false;
    }

    const FItemData NewItem = IAzulBolsoInterface::Execute_GetItemData(WorldActor);

    UE_LOG(LogAzulBolso, Warning, TEXT("TryAddItem | NewItem valid=%s | ItemID=%s | Mesh=%s | Icon=%s"),
        NewItem.IsValid() ? TEXT("true") : TEXT("false"),
        *NewItem.ItemID.ToString(),
        *GetNameSafe(NewItem.Mesh),
        *GetNameSafe(NewItem.Icon));

    if (!NewItem.IsValid())
    {
        UE_LOG(LogAzulBolso, Error, TEXT("TryAddItem | NewItem no es válido"));
        return false;
    }

    for (int32 i = 0; i < Bolso.Num(); ++i)
    {
        UE_LOG(LogAzulBolso, Warning, TEXT("TryAddItem | Revisando slot %d | ocupado=%s"),
            i,
            Bolso[i].IsValid() ? TEXT("true") : TEXT("false"));

        if (!Bolso[i].IsValid())
        {
            Bolso[i] = NewItem;

            UE_LOG(LogAzulBolso, Warning, TEXT("TryAddItem | Item guardado en slot %d"), i);

            if (BolsoWidgetInstance)
            {
                if (BolsoWidgetInstance->SlotItems.Num() < Bolso.Num())
                {
                    BolsoWidgetInstance->SlotItems.SetNum(Bolso.Num());
                }

                BolsoWidgetInstance->SlotItems[i] = Bolso[i];
                BolsoWidgetInstance->UpdateUI();

                UE_LOG(LogAzulBolso, Warning, TEXT("TryAddItem | UI actualizada para slot %d"), i);
            }
            else
            {
                UE_LOG(LogAzulBolso, Warning, TEXT("TryAddItem | BolsoWidgetInstance es NULL, no se actualiza UI"));
            }

            WorldActor->Destroy();
            UE_LOG(LogAzulBolso, Warning, TEXT("TryAddItem | Actor destruido tras guardar item"));
            return true;
        }
    }

    PendingBolsoItem = NewItem;
    PendingBolsoItemWorldLocation = WorldActor->GetActorLocation();
    PendingPhysicalPick = WorldActor;

    UE_LOG(LogAzulBolso, Warning, TEXT("TryAddItem | Bolso lleno, guardando item pendiente | ItemID=%s | Location=%s"),
        *PendingBolsoItem.ItemID.ToString(),
        *PendingBolsoItemWorldLocation.ToString());

    WorldActor->SetActorEnableCollision(false);
    WorldActor->SetActorTickEnabled(false);
    WorldActor->SetActorHiddenInGame(true);

    if (BolsoWidgetInstance)
    {
        UE_LOG(LogAzulBolso, Warning, TEXT("TryAddItem | Mostrando dialogo de bolso lleno"));
        BolsoWidgetInstance->ShowFullBolsoDialog();
    }
    else
    {
        UE_LOG(LogAzulBolso, Error, TEXT("TryAddItem | BolsoWidgetInstance es NULL al intentar mostrar dialogo"));
    }

    return false;
}

void UAzulBolsoComponent::HandleSwapConfirmed(int32 SlotIndex)
{
    UE_LOG(LogAzulBolso, Warning, TEXT("HandleSwapConfirmed START | SlotIndex=%d | PendingValid=%s"),
        SlotIndex,
        PendingBolsoItem.IsValid() ? TEXT("true") : TEXT("false"));

    if (!PendingBolsoItem.IsValid() || !Bolso.IsValidIndex(SlotIndex) || !BolsoWidgetInstance)
    {
        UE_LOG(LogAzulBolso, Error, TEXT("HandleSwapConfirmed | Datos invalidos | PendingValid=%s | SlotValid=%s | Widget=%s"),
            PendingBolsoItem.IsValid() ? TEXT("true") : TEXT("false"),
            Bolso.IsValidIndex(SlotIndex) ? TEXT("true") : TEXT("false"),
            *GetNameSafe(BolsoWidgetInstance));
        return;
    }

    const FItemData OldItem = Bolso[SlotIndex];
    const FVector DropLocation = PendingBolsoItemWorldLocation;

    UE_LOG(LogAzulBolso, Warning, TEXT("HandleSwapConfirmed | Reemplazando slot %d | OldItem=%s | NewItem=%s"),
        SlotIndex,
        *OldItem.ItemID.ToString(),
        *PendingBolsoItem.ItemID.ToString());

    Bolso[SlotIndex] = PendingBolsoItem;
    BolsoWidgetInstance->SlotItems[SlotIndex] = Bolso[SlotIndex];

    if (PendingPhysicalPick)
    {
        UE_LOG(LogAzulBolso, Warning, TEXT("HandleSwapConfirmed | Destruyendo pick pendiente %s"), *GetNameSafe(PendingPhysicalPick));
        PendingPhysicalPick->Destroy();
        PendingPhysicalPick = nullptr;
    }

    SpawnDroppedItem(OldItem, DropLocation);

    PendingBolsoItem = FItemData();
    PendingBolsoItemWorldLocation = FVector::ZeroVector;

    BolsoWidgetInstance->ResetBolsoLayout();
    BolsoWidgetInstance->UpdateUI();

    UE_LOG(LogAzulBolso, Warning, TEXT("HandleSwapConfirmed | Swap completado y UI actualizada"));
}

void UAzulBolsoComponent::SpawnDroppedItem(const FItemData& Item, const FVector& Location)
{
    UE_LOG(LogAzulBolso, Warning, TEXT("SpawnDroppedItem START | ItemID=%s | Location=%s | ItemActorClass=%s"),
        *Item.ItemID.ToString(),
        *Location.ToString(),
        *GetNameSafe(ItemActorClass));

    if (!Item.IsValid() || !ItemActorClass || !OwnerCharacter)
    {
        UE_LOG(LogAzulBolso, Error, TEXT("SpawnDroppedItem | Abortado | ItemValid=%s | ItemActorClass=%s | Owner=%s"),
            Item.IsValid() ? TEXT("true") : TEXT("false"),
            *GetNameSafe(ItemActorClass),
            *GetNameSafe(OwnerCharacter));
        return;
    }

    AActor* SpawnedActor = OwnerCharacter->GetWorld()->SpawnActor<AActor>(
        ItemActorClass,
        Location,
        FRotator::ZeroRotator
    );

    UE_LOG(LogAzulBolso, Warning, TEXT("SpawnDroppedItem | SpawnedActor=%s"), *GetNameSafe(SpawnedActor));

    if (!SpawnedActor)
    {
        return;
    }

    if (SpawnedActor->GetClass()->ImplementsInterface(UAzulBolsoInterface::StaticClass()))
    {
        IAzulBolsoInterface::Execute_ApplyItemData(SpawnedActor, Item);
        UE_LOG(LogAzulBolso, Warning, TEXT("SpawnDroppedItem | ApplyItemData ejecutado en %s"), *GetNameSafe(SpawnedActor));
    }
    else
    {
        UE_LOG(LogAzulBolso, Error, TEXT("SpawnDroppedItem | El actor spawneado no implementa la interfaz del bolso"));
    }
}

bool UAzulBolsoComponent::IsGameplay03() const
{
    if (!OwnerCharacter)
    {
        UE_LOG(LogAzulBolso, Error, TEXT("IsGameplay03 | OwnerCharacter es NULL"));
        return false;
    }

    UWorld* World = OwnerCharacter->GetWorld();
    if (!World)
    {
        UE_LOG(LogAzulBolso, Error, TEXT("IsGameplay03 | World es NULL"));
        return false;
    }

    const FString LevelName = UGameplayStatics::GetCurrentLevelName(World, true);
    const bool bIsGameplay03 = LevelName.Equals(TEXT("LV_Gameplay_03"));

    UE_LOG(LogAzulBolso, Warning, TEXT("IsGameplay03 | LevelName=%s | Resultado=%s"),
        *LevelName,
        bIsGameplay03 ? TEXT("true") : TEXT("false"));

    return bIsGameplay03;
}