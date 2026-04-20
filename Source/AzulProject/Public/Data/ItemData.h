#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "ItemData.generated.h"

USTRUCT(BlueprintType)
struct FItemData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
    UStaticMesh* Mesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
    UTexture2D* Icon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
    FName ItemID = NAME_None;

    bool IsValid() const
    {
        return Mesh != nullptr;
    }
};