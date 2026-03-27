// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "AzulDialogueMappingDataAsset.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FDialogueConditionSet
{
    GENERATED_BODY()

    // Tags que el Player debe tener para que esta entrada sea válida
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Dialogue")
    FGameplayTagContainer RequiredTags;
};

USTRUCT(BlueprintType)
struct FDialogueMappingEntry
{
    GENERATED_BODY()

    // Identidad base del NPC (ej: Dialogue.Mage)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Dialogue")
    FGameplayTag NPC_BaseTag;

    // Conjunto de condiciones (ej: {State.Scene03, Quest.Completed})
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Dialogue")
    FDialogueConditionSet Conditions;

    // Tabla final que se debe usar
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Dialogue")
    UDataTable* DialogueTable = nullptr;
};

UCLASS(BlueprintType)
class AZULPROJECT_API UAzulDialogueMappingDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:

    // Reglas de tu juego
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azul|Dialogue")
    TArray<FDialogueMappingEntry> Entries;


    /*
     * Función central:
     *  Selecciona la DataTable correcta usando el NPC_BaseTag + Tags del Player.
     *  Prioridad automática POR ESPECIFICIDAD:
     *    - Más tags requeridos = más prioridad
     *    - Orden en el DataAsset no importa
     */
    UFUNCTION(BlueprintCallable, Category = "Azul|Dialogue")
    UDataTable* ResolveDialogueTable(
        const FGameplayTag& NPC_BaseTag,
        const FGameplayTagContainer& PlayerTags
    ) const;
};
