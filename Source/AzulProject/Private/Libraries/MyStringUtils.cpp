// Fill out your copyright notice in the Description page of Project Settings.


#include "Libraries/MyStringUtils.h"


bool UMyStringUtils::IsStringAlphabeticNoSpaces(const FString& Input)
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
