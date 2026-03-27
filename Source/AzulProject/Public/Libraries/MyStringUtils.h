// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyStringUtils.generated.h"

/**
 * 
 */
UCLASS()
class AZULPROJECT_API UMyStringUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "StringUtils")
    static bool IsStringAlphabeticNoSpaces(const FString& Input);
	
};
