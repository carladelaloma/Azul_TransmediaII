//#include "Widgets/AzulWidgetBolsoBase.h"
//#include "Characters/AzulCharacterBase.h"
//#include "Kismet/KismetMathLibrary.h"
//#include "Components/CanvasPanelSlot.h"
//#include "Components/HorizontalBoxSlot.h"
//#include "Engine/World.h"
//#include "TimerManager.h"
//#include "Blueprint/WidgetLayoutLibrary.h"
//
//void UAzulWidgetBolsoBase::NativeConstruct()
//{
//    Super::NativeConstruct();
//
//    SetButtonsEnabled(false);
//
//    //Guardar el estilo de los botones presionados
//    if (SlotButton1) Slot1BaseStyle = SlotButton1->GetStyle();
//    if (SlotButton2) Slot2BaseStyle = SlotButton2->GetStyle();
//
//    // Ocultar elementos
//    if (FullText) FullText->SetVisibility(ESlateVisibility::Hidden);
//    if (InfoText) InfoText->SetVisibility(ESlateVisibility::Hidden);
//    if (ConfirmButton) ConfirmButton->SetVisibility(ESlateVisibility::Hidden);
//
//    SlotItems.SetNum(2);
//
//    if (SlotButton1 && !SlotButton1->OnClicked.IsAlreadyBound(this, &UAzulWidgetBolsoBase::OnSlot1Clicked))
//    {
//        SlotButton1->OnClicked.AddDynamic(this, &UAzulWidgetBolsoBase::OnSlot1Clicked);
//    }
//
//    if (SlotButton2 && !SlotButton2->OnClicked.IsAlreadyBound(this, &UAzulWidgetBolsoBase::OnSlot2Clicked))
//    {
//        SlotButton2->OnClicked.AddDynamic(this, &UAzulWidgetBolsoBase::OnSlot2Clicked);
//    }
//
//    if (ConfirmButton && !ConfirmButton->OnClicked.IsAlreadyBound(this, &UAzulWidgetBolsoBase::OnConfirmClicked))
//        ConfirmButton->OnClicked.AddDynamic(this, &UAzulWidgetBolsoBase::OnConfirmClicked);
//
//    // Configurar posición inicial del HorizontalBox
//    if (BolsoPanel)
//    {
//        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(BolsoPanel->Slot))
//        {
//            CanvasSlot->SetPosition(PanelStartPos);
//            CanvasSlot->SetSize(PanelStartSize);
//        }
//    }
//
//
//    // Tick manual con Timer
//    GetWorld()->GetTimerManager().SetTimer(
//        AnimTimer,
//        this,
//        &UAzulWidgetBolsoBase::TickAnimation,
//        0.016f,
//        true
//    );
//
//    UpdateUI();
//}
//
//
//void UAzulWidgetBolsoBase::UpdateUI()
//{
//    if (SlotIcon1)
//        SlotIcon1->SetBrushFromTexture(SlotItems[0].Icon);
//
//    if (SlotIcon2)
//        SlotIcon2->SetBrushFromTexture(SlotItems[1].Icon);
//}
//
//void UAzulWidgetBolsoBase::SetButtonsEnabled(bool bEnabled)
//{
//    if (SlotButton1) SlotButton1->SetIsEnabled(bEnabled);
//    if (SlotButton2) SlotButton2->SetIsEnabled(bEnabled);
//    if (ConfirmButton) ConfirmButton->SetIsEnabled(bEnabled);
//}
//
//void UAzulWidgetBolsoBase::ShowFullBolsoDialog()
//{
//    if (!FullText) return;
//
//    // MOSTRAR mensaje 2 segundos
//    FullText->SetVisibility(ESlateVisibility::Visible);
//    bPlayFullText = true;
//    FullTextTimer = 0.f;
//}
//
//void UAzulWidgetBolsoBase::ResetBolsoLayout()
//{
//    // Ocultar elementos
//    if (FullText) FullText->SetVisibility(ESlateVisibility::Hidden);
//    if (InfoText) InfoText->SetVisibility(ESlateVisibility::Hidden);
//    if (ConfirmButton) ConfirmButton->SetVisibility(ESlateVisibility::Hidden);
//
//    // Reset animación
//    bPlayFullText = false;
//    bPlayMoveAnim = false;
//    FullTextTimer = 0.f;
//    HB_AnimTime = 0.f;
//
//    // Reset estilos de botones
//    if (SlotButton1) SlotButton1->SetStyle(Slot1BaseStyle);
//    if (SlotButton2) SlotButton2->SetStyle(Slot2BaseStyle);
//
//    // Reset posición/tamaño del panel
//    if (BolsoPanel)
//    {
//        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(BolsoPanel->Slot))
//        {
//            CanvasSlot->SetPosition(PanelStartPos);
//            CanvasSlot->SetSize(PanelStartSize);
//        }
//    }
//}
//
//
//
//void UAzulWidgetBolsoBase::TickAnimation()
//{
//   // MOSTRAR FULLTEXT 2 segundos
//    if (bPlayFullText)
//    {
//        FullTextTimer += 0.016f;
//
//        if (FullTextTimer >= 2.f)
//        {
//            if (FullText)
//                FullText->SetVisibility(ESlateVisibility::Hidden);
//
//            bPlayFullText = false;
//            bPlayMoveAnim = true; // pasar a la animación
//        }
//        return;
//    }
//
//    // 2) ANIMACIÓN DEL PANEL PRINCIPAL (BolsoPanel)
//    if (bPlayMoveAnim && BolsoPanel)
//    {
//        HB_AnimTime += 0.016f;
//        float Alpha = FMath::Clamp(HB_AnimTime / HB_AnimDuration, 0.f, 1.f);
//        float Smooth = Ease(Alpha);
//
//        FVector2D NewPos = FMath::Lerp(PanelStartPos, PanelEndPos, Smooth);
//        FVector2D NewSize = FMath::Lerp(PanelStartSize, PanelEndSize, Smooth);
//
//        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(BolsoPanel->Slot))
//        {
//            CanvasSlot->SetPosition(NewPos);
//            CanvasSlot->SetSize(NewSize);
//        }
//
//        if (Alpha >= 1.f)
//        {
//            bPlayMoveAnim = false;
//
//            if (InfoText) InfoText->SetVisibility(ESlateVisibility::Visible);
//            if (ConfirmButton) ConfirmButton->SetVisibility(ESlateVisibility::Visible);
//        }
//    }
//}
//
//
//void UAzulWidgetBolsoBase::OnConfirmClicked()
//{
//    if (SelectedSlot < 0)
//        return;
//
//    OwnerCharacter->BolsoComponent->HandleSwapConfirmed(SelectedSlot);
//    SelectedSlot = -1;
//
//    SlotButton1->SetColorAndOpacity(FLinearColor::White);
//    SlotButton1->SetRenderTransform(FWidgetTransform());
//
//    SlotButton2->SetColorAndOpacity(FLinearColor::White);
//    SlotButton2->SetRenderTransform(FWidgetTransform());
//
//    ResetBolsoLayout();
//}
//
//void UAzulWidgetBolsoBase::OnSlot1Clicked()
//{
//    SelectedSlot = 0;
//
//    // Aplicar estilo PRESSED como estilo SELECTED
//    FButtonStyle PressedStyle = Slot1BaseStyle;
//    PressedStyle.Normal = Slot1BaseStyle.Pressed;
//    PressedStyle.Hovered = Slot1BaseStyle.Pressed;
//    PressedStyle.NormalPadding = Slot1BaseStyle.PressedPadding;
//    PressedStyle.PressedPadding = Slot1BaseStyle.PressedPadding;
//
//    SlotButton1->SetStyle(PressedStyle);
//
//    // Restaurar estilo normal al botón 2
//    SlotButton2->SetStyle(Slot2BaseStyle);
//}
//
//
//void UAzulWidgetBolsoBase::OnSlot2Clicked()
//{
//    SelectedSlot = 1;
//
//    FButtonStyle PressedStyle = Slot2BaseStyle;
//    PressedStyle.Normal = Slot2BaseStyle.Pressed;
//    PressedStyle.Hovered = Slot2BaseStyle.Pressed;
//    PressedStyle.NormalPadding = Slot2BaseStyle.PressedPadding;
//    PressedStyle.PressedPadding = Slot2BaseStyle.PressedPadding;
//
//    SlotButton2->SetStyle(PressedStyle);
//
//    SlotButton1->SetStyle(Slot1BaseStyle);
//}
//
//
//
//
//
