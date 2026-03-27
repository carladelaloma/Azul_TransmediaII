// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AzulGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class AZULPROJECT_API UAzulGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable)
    void LoadGameplayByIndex(int32 Index);

    UFUNCTION(BlueprintCallable)
    void LoadNextGameplay();

    UPROPERTY(BlueprintReadWrite)
    int32 CurrentGameplayIndex = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxGameplays = 12;
	
};
