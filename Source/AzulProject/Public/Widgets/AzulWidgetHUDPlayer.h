// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "AzulWidgetHUDPlayer.generated.h"

UENUM(BlueprintType)
enum class EInteractUIState : uint8
{
    Default,
    Active
};

UCLASS()
class AZULPROJECT_API UAzulWidgetHUDPlayer : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UImage* InteractImage;

    // UNA sola textura
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Interact UI")
    UTexture2D* CrosshairTexture;

    // Colores
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Interact UI")
    FLinearColor DefaultColor = FLinearColor(0.1333f, 0.3176f, 1.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Interact UI")
    FLinearColor ActiveColor = FLinearColor(0.7765f, 1.0f, 1.0f, 0.5451f);

    UPROPERTY(meta = (BindWidget))
    UBorder* TextBorder;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* StoryText;


    UFUNCTION(BlueprintCallable)
    void SetUIState(EInteractUIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Azul|StoryText")
    void SetStoryText(const FString& NewText);
};
