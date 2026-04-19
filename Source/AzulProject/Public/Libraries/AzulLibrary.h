// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AzulLibrary.generated.h"

UENUM(BlueprintType)
enum class EGameplayLevelBranch : uint8
{
    LV_Gameplay_01 UMETA(DisplayName = "LV_Gameplay_01"),
    LV_Gameplay_02 UMETA(DisplayName = "LV_Gameplay_02"),
    LV_Gameplay_03 UMETA(DisplayName = "LV_Gameplay_03"),
    LV_Gameplay_04 UMETA(DisplayName = "LV_Gameplay_04"),
    LV_Gameplay_05 UMETA(DisplayName = "LV_Gameplay_05"),
    LV_Gameplay_06 UMETA(DisplayName = "LV_Gameplay_06"),
    LV_Gameplay_07 UMETA(DisplayName = "LV_Gameplay_07"),
    LV_Gameplay_08 UMETA(DisplayName = "LV_Gameplay_08"),
    LV_Gameplay_09 UMETA(DisplayName = "LV_Gameplay_09"),
    LV_Gameplay_10 UMETA(DisplayName = "LV_Gameplay_10"),
    LV_Gameplay_11 UMETA(DisplayName = "LV_Gameplay_11"),
    LV_Gameplay_12 UMETA(DisplayName = "LV_Gameplay_12")
};
/**
 * 
 */
UCLASS()
class AZULPROJECT_API UAzulLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "AzulLibrary")
    static bool IsStringAlphabeticNoSpaces(const FString& Input);

    UFUNCTION(BlueprintCallable, Category = "Levels", meta = (ExpandEnumAsExecs = "Branch"))
    static void SwitchLevelByName(const FString& LevelName, EGameplayLevelBranch& Branch);
	
};
