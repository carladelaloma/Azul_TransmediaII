//#pragma once
//
//#include "CoreMinimal.h"
//#include "Components/ActorComponent.h"
//#include "AzulHiloComponent.generated.h"
//
//class AAzulHiloBase;
//
//UCLASS(Blueprintable, ClassGroup = (Azul), meta = (BlueprintSpawnableComponent))
//class AZULPROJECT_API UAzulHiloComponent : public UActorComponent
//{
//    GENERATED_BODY()
//
//public:
//    UAzulHiloComponent();
//
//    // Llamada desde el Character (Input ESPACIO)
//    void ToggleHilo();
//
//protected:
//    virtual void BeginPlay() override;
//
//    virtual void TickComponent(
//        float DeltaTime,
//        ELevelTick TickType,
//        FActorComponentTickFunction* ThisTickFunction
//    ) override;
//
//private:
//    void ShowHilo();
//    void HideHilo();
//
//    bool HasOwnerMoved() const;
//    bool AreOnSameFloor() const;
//
//private:
//    // Actor visual del hilo
//    UPROPERTY(EditAnywhere, Category = "Azul|Hilo")
//    AAzulHiloBase* HiloActor;
//
//    // Destino (hijo / madre)
//    UPROPERTY(EditAnywhere, Category = "Azul|Hilo")
//    AActor* TargetActor;
//
//    // Tiempo visible
//    UPROPERTY(EditDefaultsOnly, Category = "Azul|Hilo")
//    float VisibleTime = 6.f;
//
//    // Margen de Z para planta
//    UPROPERTY(EditDefaultsOnly, Category = "Azul|Hilo")
//    float FloorZTolerance = 20.f;
//
//    bool bIsVisible = false;
//
//    FVector LastOwnerLocation;
//
//    FTimerHandle HideTimer;
//
//
//};
