#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/ItemData.h"
#include "AzulBolsoInterface.generated.h"

UINTERFACE(BlueprintType)
class AZULPROJECT_API UAzulBolsoInterface : public UInterface
{
    GENERATED_BODY()
};

class AZULPROJECT_API IAzulBolsoInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Azul|Bolso")
    bool CanGoToBolso() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Azul|Bolso")
    FItemData GetItemData() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Azul|Bolso")
    void ApplyItemData(const FItemData& ItemData);
};