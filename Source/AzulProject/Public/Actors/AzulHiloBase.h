#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "AzulHiloBase.generated.h"

class AAzulInteractuableBase;
class AAzulCharacterBase;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSplineRouteChanged,
	const TArray<FVector>&, Previous,
	const TArray<FVector>&, Target);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHiloHidden);

UCLASS()
class AZULPROJECT_API AAzulHiloBase : public AActor
{
	GENERATED_BODY()

public:
	AAzulHiloBase();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:

	// Componentes
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USplineComponent* SplineComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UNiagaraComponent* NiagaraComp;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* NiagaraTemplate;



	// Evento para BP (Timeline)
	UPROPERTY(BlueprintAssignable, Category = "Azul|Hilo")
	FOnSplineRouteChanged OnSplineRouteChanged;

	UPROPERTY(BlueprintAssignable, Category = "Azul|Hilo")
	FOnHiloHidden OnHiloHidden;


	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Azul|Hilo")
	AAzulInteractuableBase* HijoActor = nullptr;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Azul|Hilo")
	ACharacter* CachedPlayer = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Azul|Hilo")
	void RecalculateHiloFromInput();

	// Llamado desde BP para aplicar puntos interpolados
	UFUNCTION(BlueprintCallable, Category = "Azul|Hilo")
	void ApplyInterpolatedSplinePoints(const TArray<FVector>& Points);

	UFUNCTION(BlueprintCallable)
	void ShowHilo();

	UFUNCTION(BlueprintCallable)
	void ForceHideHilo();

	bool IsHiloVisible() const { return bHiloVisible; }

	TArray<FVector> GenerateSmoothCurvedRoute(
		const FVector& StartPos,
		const FVector& StartForward,
		const FVector& EndPos,
		const FVector& EndForward
	);


private:
	void SetNiagaraLifeTime(float Value);

	// Estado
	bool bHiloVisible = false;

	// Timer
	FTimerHandle Timer_HideHilo;

	// Tiempo visible
	UPROPERTY(EditAnywhere, Category = "Azul|Hilo")
	float HiloVisibleTime = 4.0f;

	// Función interna de apagado
	void HideHilo();


	TArray<FVector> PreviousPoints;
	TArray<FVector> TargetPoints;

	FVector CachedStartPos;


	TArray<FVector> GenerateCurvedRoute(const FVector& StartPos, const FVector& StartTangentDir, const FVector& EndPos);
};
