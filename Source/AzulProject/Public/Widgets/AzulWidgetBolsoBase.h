//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "Blueprint/UserWidget.h"
//#include "Engine/DataTable.h"
//#include "Components/Button.h"
//#include "Components/Image.h"
//#include "Components/HorizontalBox.h"
//#include "Components/Border.h"
//#include "Components/TextBlock.h"
//#include "Blueprint/WidgetLayoutLibrary.h"
////#include "Actors/AzulStoryObjectBase.h"
//#include "AzulWidgetBolsoBase.generated.h"
//
//class AAzulCharacterBase;
//
//USTRUCT(BlueprintType)
//struct FItemData
//{
//    GENERATED_BODY()
//
//public:
//
//    // Si no hay item → Mesh == nullptr
//    UPROPERTY()
//    UStaticMesh* Mesh = nullptr;
//
//    UPROPERTY()
//    UTexture2D* Icon = nullptr;
//
//    // En caso de querer identificar objetos en el futuro
//    UPROPERTY()
//    FName ItemID = NAME_None;
//
//    // Helper para saber si está vacío
//    bool IsValid() const { return Mesh != nullptr; }
//};
//
///**
// * 
// */
//UCLASS()
//class AZULPROJECT_API UAzulWidgetBolsoBase : public UUserWidget
//{
//	GENERATED_BODY()
//
//public:
//
//    //---------------VARIABLES DEL WIDGET
//    UPROPERTY(meta = (BindWidget))
//    UHorizontalBox* HorizontalBox;
//
//    UPROPERTY(meta = (BindWidget))
//    UBorder* BolsoPanel;
//
//    UPROPERTY(meta = (BindWidget))
//    UButton* SlotButton1;
//
//    UPROPERTY(meta = (BindWidget))
//    UButton* SlotButton2;
//
//    UPROPERTY(meta = (BindWidget))
//    UImage* SlotIcon1;
//
//    UPROPERTY(meta = (BindWidget))
//    UImage* SlotIcon2;
//
//    UPROPERTY(meta = (BindWidgetOptional))
//    UButton* ConfirmButton;
//
//    UPROPERTY(meta = (BindWidgetOptional))
//    UTextBlock* FullText;
//
//    UPROPERTY(meta = (BindWidgetOptional))
//    UTextBlock* InfoText;
//
//    //---------------------VARIABLES
//    UPROPERTY(BlueprintReadOnly)
//    TArray<FItemData> SlotItems;
//
//    UPROPERTY()
//    FButtonStyle Slot1BaseStyle;
//
//    UPROPERTY()
//    FButtonStyle Slot2BaseStyle;
//
//    UPROPERTY()
//    AAzulCharacterBase* OwnerCharacter;
//
//    int32 SelectedSlot = -1;
//
//    //-------------------------FUNCIONES
//    UFUNCTION()
//    void OnSlot1Clicked();
//
//    UFUNCTION()
//    void OnSlot2Clicked();
//
//    UFUNCTION()
//    void OnConfirmClicked();
//
//
//    void ShowFullBolsoDialog();
//
//    void ResetBolsoLayout();
//
//    void UpdateUI();
//
//    void SetButtonsEnabled(bool bEnabled);
//
//    UFUNCTION()
//    void TickAnimation();
//
//protected:
//
//    virtual void NativeConstruct() override;
//
//
//    // ANIMACIÓN DEL BOLSO LLENO
//    bool bPlayFullText = false;
//    float FullTextTimer = 0.f;
//
//    bool bPlayMoveAnim = false;
//
//    // Animation control
//// Movimiento del panel completo (no solo el HorizontalBox)
//    FVector2D PanelStartPos = FVector2D(-610.f, 300.f);
//    FVector2D PanelEndPos = FVector2D(0.f, 0.f);
//
//    FVector2D PanelStartSize = FVector2D(485.f, 255.f);
//    FVector2D PanelEndSize = FVector2D(485.f * 1.5f, 255.f * 1.5f); // x1.5
//
//
//    float HB_AnimTime = 0.f;
//    float HB_AnimDuration = 0.3f;
//
//    FTimerHandle AnimTimer;
//
//    float Ease(float t) const
//    {
//        return t < 0.5f ? 2 * t * t : 1 - FMath::Pow(-2 * t + 2, 2) / 2;
//    }
//};
