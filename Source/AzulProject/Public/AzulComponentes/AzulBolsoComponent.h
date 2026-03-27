//#pragma once
//
//#include "CoreMinimal.h"
//#include "Components/ActorComponent.h"
//#include "Widgets/AzulWidgetBolsoBase.h"
//#include "AzulBolsoComponent.generated.h"
//
//class AAzulStoryObjectBase;
//class AAzulCharacterBase;
//
//UCLASS(ClassGroup = (Azul), meta = (BlueprintSpawnableComponent))
//class AZULPROJECT_API UAzulBolsoComponent : public UActorComponent
//{
//    GENERATED_BODY()
//
//public:
//    UAzulBolsoComponent();
//
//    void InitializeBolso(AAzulCharacterBase* OwnerCharacter);
//
//    //UFUNCTION(BlueprintCallable, Category = "Azul|Bolso")
//    //bool TryAddItem(AAzulStoryObjectBase* WorldItem);
//
//    UFUNCTION()
//    void HandleSwapConfirmed(int32 SlotIndex);
//
//private:
//    void SpawnDroppedItem(const FItemData& Item, const FVector& Location);
//
//private:
//    // Datos del bolso
//    UPROPERTY()
//    TArray<FItemData> Bolso;
//
//    // UI
//    UPROPERTY()
//    UAzulWidgetBolsoBase* BolsoWidgetInstance;
//
//    UPROPERTY(EditAnywhere, Category = "Azul|Bolso")
//    TSubclassOf<UAzulWidgetBolsoBase> BolsoWidgetClass;
//
//    // Swap
//    UPROPERTY()
//    FItemData PendingBolsoItem;
//
//    UPROPERTY()
//    FVector PendingBolsoItemWorldLocation;
//
//    //UPROPERTY(EditAnywhere, Category = "Azul|Bolso")
//    //TSubclassOf<AAzulStoryObjectBase> ItemActorClass;
//
// //   UPROPERTY()
////    AAzulStoryObjectBase* PendingPhysicalPick;
//
//    // Owner
//    UPROPERTY()
//    AAzulCharacterBase* OwnerCharacter;
//};
