#include "AzulSubsystem/AzulGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UAzulGameInstance::LoadGameplayByIndex(int32 Index)
{
    if (Index <= 0 || Index > MaxGameplays)
        return;

    CurrentGameplayIndex = Index;

    FString LevelName = FString::Printf(TEXT("LV_Gameplay_%02d"), Index);

    UGameplayStatics::OpenLevel(this, FName(*LevelName));
}

void UAzulGameInstance::LoadNextGameplay()
{
    int32 NextIndex = CurrentGameplayIndex + 1;

    if (NextIndex <= MaxGameplays)
    {
        LoadGameplayByIndex(NextIndex);
    }
}
