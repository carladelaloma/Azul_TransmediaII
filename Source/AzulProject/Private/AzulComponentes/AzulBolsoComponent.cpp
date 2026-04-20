#include "AzulComponentes/AzulBolsoComponent.h"
#include "Characters/AzulCharacterBase.h"
#include "Widgets/AzulWidgetBolsoBase.h"
#include "Interfaces/AzulBolsoInterface.h"
#include "GameFramework/Actor.h"

UAzulBolsoComponent::UAzulBolsoComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAzulBolsoComponent::InitializeBolso(AAzulCharacterBase* InOwner)
{
    OwnerCharacter = InOwner;

    Bolso.SetNum(2);
    Bolso[0] = FItemData();
    Bolso[1] = FItemData();

    if (BolsoWidgetClass && OwnerCharacter)
    {
        BolsoWidgetInstance = CreateWidget<UAzulWidgetBolsoBase>(
            OwnerCharacter->GetWorld(),
            BolsoWidgetClass
        );

        if (BolsoWidgetInstance)
        {
            BolsoWidgetInstance->OwnerCharacter = OwnerCharacter;
            BolsoWidgetInstance->SlotItems.SetNum(2);
            BolsoWidgetInstance->UpdateUI();
        }
    }
}

bool UAzulBolsoComponent::TryAddItem(AActor* WorldActor)
{
    if (!WorldActor)
    {
        return false;
    }

    if (!WorldActor->GetClass()->ImplementsInterface(UAzulBolsoInterface::StaticClass()))

    const bool bCanGoToBolso = IAzulBolsoInterface::Execute_CanGoToBolso(WorldActor);

    const FItemData NewItem = IAzulBolsoInterface::Execute_GetItemData(WorldActor);
    if (!NewItem.IsValid())
    {
        return false;
    }

    for (int32 i = 0; i < Bolso.Num(); ++i)
    {
        if (!Bolso[i].IsValid())
        {
            Bolso[i] = NewItem;

            if (BolsoWidgetInstance)
            {
                if (BolsoWidgetInstance->SlotItems.Num() < Bolso.Num())
                {
                    BolsoWidgetInstance->SlotItems.SetNum(Bolso.Num());
                }

                BolsoWidgetInstance->SlotItems[i] = Bolso[i];
                BolsoWidgetInstance->UpdateUI();
            }

            WorldActor->Destroy();
            return true;
        }
    }

    PendingBolsoItem = NewItem;
    PendingBolsoItemWorldLocation = WorldActor->GetActorLocation();
    PendingPhysicalPick = WorldActor;

    WorldActor->SetActorEnableCollision(false);
    WorldActor->SetActorTickEnabled(false);
    WorldActor->SetActorHiddenInGame(true);

    if (BolsoWidgetInstance)
    {
        BolsoWidgetInstance->ShowFullBolsoDialog();
    }

    return false;
}

void UAzulBolsoComponent::HandleSwapConfirmed(int32 SlotIndex)
{
    if (!PendingBolsoItem.IsValid() || !Bolso.IsValidIndex(SlotIndex) || !BolsoWidgetInstance)
    {
        return;
    }

    const FItemData OldItem = Bolso[SlotIndex];
    const FVector DropLocation = PendingBolsoItemWorldLocation;

    Bolso[SlotIndex] = PendingBolsoItem;
    BolsoWidgetInstance->SlotItems[SlotIndex] = Bolso[SlotIndex];

    if (PendingPhysicalPick)
    {
        PendingPhysicalPick->Destroy();
        PendingPhysicalPick = nullptr;
    }

    SpawnDroppedItem(OldItem, DropLocation);

    PendingBolsoItem = FItemData();
    PendingBolsoItemWorldLocation = FVector::ZeroVector;

    BolsoWidgetInstance->ResetBolsoLayout();
    BolsoWidgetInstance->UpdateUI();
}

void UAzulBolsoComponent::SpawnDroppedItem(const FItemData& Item, const FVector& Location)
{
    if (!Item.IsValid() || !ItemActorClass || !OwnerCharacter)
    {
        return;
    }

    AActor* SpawnedActor = OwnerCharacter->GetWorld()->SpawnActor<AActor>(
        ItemActorClass,
        Location,
        FRotator::ZeroRotator
    );

    if (!SpawnedActor)
    {
        return;
    }

    if (SpawnedActor->GetClass()->ImplementsInterface(UAzulBolsoInterface::StaticClass()))
    {
        IAzulBolsoInterface::Execute_ApplyItemData(SpawnedActor, Item);
    }
}