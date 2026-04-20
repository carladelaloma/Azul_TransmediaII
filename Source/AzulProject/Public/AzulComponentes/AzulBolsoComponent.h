#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/AzulBolsoInterface.h"
#include "Data/ItemData.h"
#include "AzulBolsoComponent.generated.h"

class UAzulWidgetBolsoBase;
class AAzulCharacterBase;

UCLASS(ClassGroup = (Azul), meta = (BlueprintSpawnableComponent))
class AZULPROJECT_API UAzulBolsoComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAzulBolsoComponent();

    void InitializeBolso(AAzulCharacterBase* OwnerCharacter);

    UFUNCTION(BlueprintCallable, Category = "Azul|Bolso")
    bool TryAddItem(AActor* WorldActor);

    UFUNCTION()
    void HandleSwapConfirmed(int32 SlotIndex);

private:
    void SpawnDroppedItem(const FItemData& Item, const FVector& Location);

    UPROPERTY()
    TArray<FItemData> Bolso;

    UPROPERTY()
    TObjectPtr<UAzulWidgetBolsoBase> BolsoWidgetInstance = nullptr;

    UPROPERTY(EditAnywhere, Category = "Azul|Bolso")
    TSubclassOf<UAzulWidgetBolsoBase> BolsoWidgetClass;

    UPROPERTY(EditAnywhere, Category = "Azul|Bolso")
    TSubclassOf<AActor> ItemActorClass;

    UPROPERTY()
    FItemData PendingBolsoItem;

    UPROPERTY()
    FVector PendingBolsoItemWorldLocation = FVector::ZeroVector;

    UPROPERTY()
    TObjectPtr<AActor> PendingPhysicalPick = nullptr;

    UPROPERTY()
    TObjectPtr<AAzulCharacterBase> OwnerCharacter = nullptr;
};