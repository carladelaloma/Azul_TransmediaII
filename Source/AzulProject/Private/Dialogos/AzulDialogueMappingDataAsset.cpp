// Fill out your copyright notice in the Description page of Project Settings.


#include "Dialogos/AzulDialogueMappingDataAsset.h"

UDataTable* UAzulDialogueMappingDataAsset::ResolveDialogueTable(
    const FGameplayTag& NPC_BaseTag,
    const FGameplayTagContainer& PlayerTags
) const
{
    UE_LOG(LogTemp, Warning, TEXT("ResolveDialogueTable CALLED: NPC=%s"), *NPC_BaseTag.ToString());

    UDataTable* BestTable = nullptr;
    int32 BestSpecificity = -1;

    for (const FDialogueMappingEntry& Entry : Entries)
    {
        // 1. NPC BaseTag debe coincidir
        if (Entry.NPC_BaseTag != NPC_BaseTag)
            continue;

        // 2. Este set de condiciones debe estar contenido en PlayerTags
        const FGameplayTagContainer& Required = Entry.Conditions.RequiredTags;

        if (!PlayerTags.HasAll(Required))
            continue;

        // 3. Especificidad = número de tags requeridos
        const int32 Specificity = Required.Num();

        // Queremos la entrada con MÁS tags requeridos
        if (Specificity > BestSpecificity)
        {
            BestSpecificity = Specificity;
            BestTable = Entry.DialogueTable;
        }
    }

    // BestTable puede ser nullptr si no hay reglas que coincidan
    return BestTable;
}
