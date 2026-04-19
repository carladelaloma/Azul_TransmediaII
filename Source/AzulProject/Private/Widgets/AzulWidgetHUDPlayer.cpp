#include "Widgets/AzulWidgetHUDPlayer.h"
#include "AzulSubsystem/AzulGameSubsystem.h"
#include "Engine/GameInstance.h"

void UAzulWidgetHUDPlayer::SetUIState(EInteractUIState NewState)
{
    if (!InteractImage || !CrosshairTexture)
        return;

    InteractImage->SetBrushFromTexture(CrosshairTexture, true);

    switch (NewState)
    {
    case EInteractUIState::Default:
        InteractImage->SetColorAndOpacity(DefaultColor);
        break;

    case EInteractUIState::Active:
        InteractImage->SetColorAndOpacity(ActiveColor);
        break;
    }
}

void UAzulWidgetHUDPlayer::SetStoryText(const FString& NewText, float Delay)
{
    if (!GetWorld())
    {
        return;
    }

    FString FinalText = NewText;

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UAzulGameSubsystem* GameSubsystem = GI->GetSubsystem<UAzulGameSubsystem>())
        {
            if (!GameSubsystem->SonName.IsEmpty())
            {
                FinalText = FinalText.Replace(
                    TEXT("{SonName}"),
                    *GameSubsystem->SonName,
                    ESearchCase::IgnoreCase
                );
            }
        }
    }

    GetWorld()->GetTimerManager().ClearTimer(StoryTextTimer);

    if (FinalText.IsEmpty())
    {
        if (StoryText)
        {
            StoryText->SetText(FText::GetEmpty());
        }

        if (TextBorder)
        {
            TextBorder->SetVisibility(ESlateVisibility::Hidden);
        }

        return;
    }

    if (StoryText)
    {
        StoryText->SetText(FText::FromString(FinalText));
    }

    if (TextBorder)
    {
        TextBorder->SetVisibility(ESlateVisibility::Visible);
    }

    if (Delay > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            StoryTextTimer,
            [this]()
            {
                if (StoryText)
                {
                    StoryText->SetText(FText::GetEmpty());
                }

                if (TextBorder)
                {
                    TextBorder->SetVisibility(ESlateVisibility::Hidden);
                }
            },
            Delay,
            false
        );
    }
}
