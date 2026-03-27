//#include "AzulComponentes/AzulBolsoComponent.h"
//#include "Characters/AzulCharacterBase.h"
//#include "Actors/AzulStoryObjectBase.h"
//#include "Widgets/AzulWidgetBolsoBase.h"
//#include "Kismet/GameplayStatics.h"
//
//UAzulBolsoComponent::UAzulBolsoComponent()
//{
//    PrimaryComponentTick.bCanEverTick = false;
//}
//
//void UAzulBolsoComponent::InitializeBolso(AAzulCharacterBase* InOwner)
//{
//    OwnerCharacter = InOwner;
//
//    Bolso.SetNum(2);
//    Bolso[0] = FItemData();
//    Bolso[1] = FItemData();
//
//    if (BolsoWidgetClass && OwnerCharacter)
//    {
//        BolsoWidgetInstance = CreateWidget<UAzulWidgetBolsoBase>(
//            OwnerCharacter->GetWorld(),
//            BolsoWidgetClass
//        );
//
//        BolsoWidgetInstance->OwnerCharacter = OwnerCharacter;
//        BolsoWidgetInstance->SlotItems.SetNum(2);
//        BolsoWidgetInstance->UpdateUI();
//    }
//}
//
//bool UAzulBolsoComponent::TryAddItem(AAzulStoryObjectBase* WorldItem)
//{
//    if (!WorldItem || !BolsoWidgetInstance) return false;
//
//    FItemData NewItem;
//    NewItem.Mesh = WorldItem->MeshComp->GetStaticMesh();
//    NewItem.Icon = WorldItem->Icon;
//
//    for (int i = 0; i < Bolso.Num(); i++)
//    {
//        if (!Bolso[i].IsValid())
//        {
//            Bolso[i] = NewItem;
//            PendingPhysicalPick = WorldItem;
//            PendingPhysicalPick->Destroy();
//
//            BolsoWidgetInstance->SlotItems[i] = Bolso[i];
//            BolsoWidgetInstance->UpdateUI();
//
//            return true;
//        }
//    }
//
//    PendingBolsoItem = NewItem;
//    PendingBolsoItemWorldLocation = WorldItem->GetActorLocation();
//    PendingPhysicalPick = WorldItem;
//
//    WorldItem->SetActorEnableCollision(false);
//    WorldItem->SetActorTickEnabled(false);
//
//    BolsoWidgetInstance->ShowFullBolsoDialog();
//    return false;
//}
//
//void UAzulBolsoComponent::HandleSwapConfirmed(int32 SlotIndex)
//{
//    if (!PendingBolsoItem.IsValid() || SlotIndex < 0) return;
//
//    FItemData OldItem = Bolso[SlotIndex];
//    FVector DropLocation = PendingBolsoItemWorldLocation;
//
//    Bolso[SlotIndex] = PendingBolsoItem;
//    BolsoWidgetInstance->SlotItems[SlotIndex] = Bolso[SlotIndex];
//
//    if (PendingPhysicalPick)
//    {
//        PendingPhysicalPick->Destroy();
//        PendingPhysicalPick = nullptr;
//    }
//
//    SpawnDroppedItem(OldItem, DropLocation);
//
//    PendingBolsoItem = FItemData();
//    BolsoWidgetInstance->ResetBolsoLayout();
//    BolsoWidgetInstance->UpdateUI();
//}
//
//void UAzulBolsoComponent::SpawnDroppedItem(const FItemData& Item, const FVector& Location)
//{
//    if (!Item.IsValid() || !ItemActorClass || !OwnerCharacter) return;
//
//    AAzulStoryObjectBase* Spawned =
//        OwnerCharacter->GetWorld()->SpawnActor<AAzulStoryObjectBase>(
//            ItemActorClass, Location, FRotator::ZeroRotator
//        );
//
//    if (Spawned)
//    {
//        Spawned->MeshComp->SetStaticMesh(Item.Mesh);
//        Spawned->Icon = Item.Icon;
//    }
//}
