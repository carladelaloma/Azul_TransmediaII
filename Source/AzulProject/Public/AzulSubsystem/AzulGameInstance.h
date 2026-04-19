#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Dialogos/AzulWidgetDialogueBase.h"
#include "AzulGameInstance.generated.h"

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Azul|UI")
	TSubclassOf<UAzulWidgetDialogueBase> DialogueWidgetClass;
};