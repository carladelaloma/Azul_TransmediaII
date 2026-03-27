// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "AzulWidgetEleccionBase.generated.h"

/**
 * 
 */
UCLASS()
class AZULPROJECT_API UAzulWidgetEleccionBase : public UUserWidget
{
	GENERATED_BODY()

public:
   UPROPERTY(meta = (BindWidget))
    UButton* SlotButton1;

    UPROPERTY(meta = (BindWidget))
    UButton* SlotButton2;

    UPROPERTY(meta = (BindWidget))
    UButton* SlotButton3;

    UPROPERTY()
    FButtonStyle Slot1BaseStyle;

    UPROPERTY()
    FButtonStyle Slot2BaseStyle;

    UPROPERTY()
    FButtonStyle Slot3BaseStyle;

    UFUNCTION()
    void OnSlot1Clicked();

    UFUNCTION()
    void OnSlot2Clicked();

    UFUNCTION()
    void OnSlot3Clicked();

protected:

    virtual void NativeConstruct() override;
	
};
