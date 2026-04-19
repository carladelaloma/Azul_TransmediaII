// Fill out your copyright notice in the Description page of Project Settings.


#include "Libraries/AzulLibrary.h"


bool UAzulLibrary::IsStringAlphabeticNoSpaces(const FString& Input)
{
    // Si el string está vacío devolvemos false
    if (Input.IsEmpty())
        return false;

    for (TCHAR c : Input)
    {
        if (!FChar::IsAlpha(c))       // No es una letra
            return false;

        if (FChar::IsDigit(c))        // Es un número
            return false;

        if (FChar::IsWhitespace(c))   // Es un espacio o tabulación
            return false;
    }

    return true; // Todo perfecto, solo letras
}

void UAzulLibrary::SwitchLevelByName(const FString& LevelName, EGameplayLevelBranch& Branch)
{
    if (LevelName == TEXT("LV_Gameplay_01"))      Branch = EGameplayLevelBranch::LV_Gameplay_01;
    else if (LevelName == TEXT("LV_Gameplay_02")) Branch = EGameplayLevelBranch::LV_Gameplay_02;
    else if (LevelName == TEXT("LV_Gameplay_03")) Branch = EGameplayLevelBranch::LV_Gameplay_03;
    else if (LevelName == TEXT("LV_Gameplay_04")) Branch = EGameplayLevelBranch::LV_Gameplay_04;
    else if (LevelName == TEXT("LV_Gameplay_05")) Branch = EGameplayLevelBranch::LV_Gameplay_05;
    else if (LevelName == TEXT("LV_Gameplay_06")) Branch = EGameplayLevelBranch::LV_Gameplay_06;
    else if (LevelName == TEXT("LV_Gameplay_07")) Branch = EGameplayLevelBranch::LV_Gameplay_07;
    else if (LevelName == TEXT("LV_Gameplay_08")) Branch = EGameplayLevelBranch::LV_Gameplay_08;
    else if (LevelName == TEXT("LV_Gameplay_09")) Branch = EGameplayLevelBranch::LV_Gameplay_09;
    else if (LevelName == TEXT("LV_Gameplay_10")) Branch = EGameplayLevelBranch::LV_Gameplay_10;
    else if (LevelName == TEXT("LV_Gameplay_11")) Branch = EGameplayLevelBranch::LV_Gameplay_11;
    else                                          Branch = EGameplayLevelBranch::LV_Gameplay_12;
}
