//#include "AzulComponentes/AzulHiloComponent.h"
//#include "Actors/AzulHiloBase.h"
//#include "Characters/AzulCharacterBase.h"
//#include "GameFramework/Actor.h"
//#include "TimerManager.h"
//#include "Engine/World.h"
//
//UAzulHiloComponent::UAzulHiloComponent()
//{
//    PrimaryComponentTick.bCanEverTick = true;
//}
//
//void UAzulHiloComponent::BeginPlay()
//{
//    Super::BeginPlay();
//
//    if (AActor* Owner = GetOwner())
//    {
//        LastOwnerLocation = Owner->GetActorLocation();
//    }
//}
//
//void UAzulHiloComponent::TickComponent(
//    float DeltaTime,
//    ELevelTick TickType,
//    FActorComponentTickFunction* ThisTickFunction
//)
//{
//    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//    if (!bIsVisible)
//        return;
//
//    // Cancelación OBLIGATORIA por movimiento
//    if (HasOwnerMoved())
//    {
//        HideHilo();
//    }
//}
//
//void UAzulHiloComponent::ToggleHilo()
//{
//    if (bIsVisible)
//    {
//        HideHilo();
//    }
//    else
//    {
//        ShowHilo();
//    }
//}
//
//void UAzulHiloComponent::ShowHilo()
//{
//    if (!HiloActor || !TargetActor)
//        return;
//
//    AActor* Owner = GetOwner();
//    if (!Owner)
//        return;
//
//    FVector Start = Owner->GetActorLocation();
//    FVector End = TargetActor->GetActorLocation();
//
//    const bool bSameFloor = AreOnSameFloor();
//
//    HiloActor->ShowSpline(Start, End, bSameFloor);
//
//    bIsVisible = true;
//    LastOwnerLocation = Start;
//
//    if (AAzulCharacterBase* Char = Cast<AAzulCharacterBase>(Owner))
//    {
//        Char->NotifyHiloShown();
//    }
//
//    GetWorld()->GetTimerManager().ClearTimer(HideTimer);
//    GetWorld()->GetTimerManager().SetTimer(
//        HideTimer,
//        this,
//        &UAzulHiloComponent::HideHilo,
//        VisibleTime,
//        false
//    );
//}
//
//
//void UAzulHiloComponent::HideHilo()
//{
//    if (!bIsVisible)
//        return;
//
//    bIsVisible = false;
//
//    if (HiloActor)
//    {
//        HiloActor->Hide();
//    }
//
//    GetWorld()->GetTimerManager().ClearTimer(HideTimer);
//
//    if (AAzulCharacterBase* Char = Cast<AAzulCharacterBase>(GetOwner()))
//    {
//        Char->NotifyHiloHidden();
//    }
//}
//
//
//bool UAzulHiloComponent::HasOwnerMoved() const
//{
//    if (!GetOwner())
//        return false;
//
//    return FVector::DistSquared(
//        GetOwner()->GetActorLocation(),
//        LastOwnerLocation
//    ) > 4.f;
//}
//
//bool UAzulHiloComponent::AreOnSameFloor() const
//{
//    if (!GetOwner() || !TargetActor)
//        return false;
//
//    const float Z1 = GetOwner()->GetActorLocation().Z;
//    const float Z2 = TargetActor->GetActorLocation().Z;
//
//    return FMath::Abs(Z1 - Z2) <= FloorZTolerance;
//}
