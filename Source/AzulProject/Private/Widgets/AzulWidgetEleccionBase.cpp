#include "Widgets/AzulWidgetEleccionBase.h"
#include "Components/CanvasPanelSlot.h"

void UAzulWidgetEleccionBase::NativeConstruct()
{
    //Guardar el estilo de los botones presionados
    if (SlotButton1) Slot1BaseStyle = SlotButton1->GetStyle();
    if (SlotButton2) Slot2BaseStyle = SlotButton2->GetStyle();
    if (SlotButton3) Slot3BaseStyle = SlotButton3->GetStyle();

    if (SlotButton1 && !SlotButton1->OnClicked.IsAlreadyBound(this, &UAzulWidgetEleccionBase::OnSlot1Clicked))
    {
        SlotButton1->OnClicked.AddDynamic(this, &UAzulWidgetEleccionBase::OnSlot1Clicked);
    }

    if (SlotButton2 && !SlotButton2->OnClicked.IsAlreadyBound(this, &UAzulWidgetEleccionBase::OnSlot2Clicked))
    {
        SlotButton2->OnClicked.AddDynamic(this, &UAzulWidgetEleccionBase::OnSlot2Clicked);
    }

    if (SlotButton3 && !SlotButton3->OnClicked.IsAlreadyBound(this, &UAzulWidgetEleccionBase::OnSlot3Clicked))
    {
        SlotButton3->OnClicked.AddDynamic(this, &UAzulWidgetEleccionBase::OnSlot3Clicked);
    }
}

void UAzulWidgetEleccionBase::OnSlot1Clicked()
{
    // Aplicar estilo PRESSED como estilo SELECTED
    FButtonStyle PressedStyle = Slot1BaseStyle;
    PressedStyle.Normal = Slot1BaseStyle.Pressed;
    PressedStyle.Hovered = Slot1BaseStyle.Pressed;
    PressedStyle.NormalPadding = Slot1BaseStyle.PressedPadding;
    PressedStyle.PressedPadding = Slot1BaseStyle.PressedPadding;

    SlotButton1->SetStyle(PressedStyle);

    // Restaurar estilo normal al botón 2
    SlotButton2->SetStyle(Slot2BaseStyle);
    SlotButton3->SetStyle(Slot3BaseStyle);
}

void UAzulWidgetEleccionBase::OnSlot2Clicked()
{

    FButtonStyle PressedStyle = Slot2BaseStyle;
    PressedStyle.Normal = Slot2BaseStyle.Pressed;
    PressedStyle.Hovered = Slot2BaseStyle.Pressed;
    PressedStyle.NormalPadding = Slot2BaseStyle.PressedPadding;
    PressedStyle.PressedPadding = Slot2BaseStyle.PressedPadding;

    SlotButton2->SetStyle(PressedStyle);

    // Restaurar estilo normal al botón 1 y 3
	SlotButton3->SetStyle(Slot3BaseStyle);
    SlotButton1->SetStyle(Slot1BaseStyle);
}

void UAzulWidgetEleccionBase::OnSlot3Clicked()
{
    FButtonStyle PressedStyle = Slot3BaseStyle;
    PressedStyle.Normal = Slot3BaseStyle.Pressed;
    PressedStyle.Hovered = Slot3BaseStyle.Pressed;
    PressedStyle.NormalPadding = Slot3BaseStyle.PressedPadding;
    PressedStyle.PressedPadding = Slot3BaseStyle.PressedPadding;

    SlotButton3->SetStyle(PressedStyle);

	// Restaurar estilo normal al botón 1 y 2
	SlotButton2->SetStyle(Slot2BaseStyle);
    SlotButton1->SetStyle(Slot1BaseStyle);
}