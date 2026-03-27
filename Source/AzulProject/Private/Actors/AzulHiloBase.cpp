#include "Actors/AzulHiloBase.h"
#include "Actors/AzulInteractuableBase.h"
#include "Characters/AzulCharacterBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"


AAzulHiloBase::AAzulHiloBase()
{
    PrimaryActorTick.bCanEverTick = true;

    bHiloVisible = false;

    SplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComp"));
    RootComponent = SplineComp;

    NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
    NiagaraComp->SetupAttachment(SplineComp);

    NiagaraComp->SetVisibility(false, true);
    NiagaraComp->SetAutoActivate(false);
}


void AAzulHiloBase::BeginPlay()
{
    Super::BeginPlay();

    CachedPlayer = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    if (NiagaraTemplate)
    {
        NiagaraComp->SetAsset(NiagaraTemplate);
    }
}


void AAzulHiloBase::SetNiagaraLifeTime(float Value)
{
    static const FName LifeTimeName(TEXT("LifeTime"));
    NiagaraComp->SetVariableFloat(LifeTimeName, Value);
}



TArray<FVector> AAzulHiloBase::GenerateCurvedRoute(
    const FVector& StartPos,
    const FVector& StartTangentDir,
    const FVector& EndPos
)
{
    TArray<FVector> Result;

    const int32 NumPoints = 20;

    // Control point alineado con el tramo recto
    FVector ControlPoint = StartPos + StartTangentDir * 60.0f;

    for (int32 i = 1; i <= NumPoints; ++i)
    {
        const float Alpha = static_cast<float>(i) / NumPoints;

        FVector Point =
            FMath::Pow(1.0f - Alpha, 2) * StartPos +
            2.0f * (1.0f - Alpha) * Alpha * ControlPoint +
            FMath::Pow(Alpha, 2) * EndPos;

        Result.Add(Point);
    }

    return Result;
}



void AAzulHiloBase::ApplyInterpolatedSplinePoints(const TArray<FVector>& Points)
{
    //UKismetSystemLibrary::PrintString(
    //    this,
    //    TEXT(">>> ClearSplinePoints CALLED <<<"),
    //    true
    //);

    SplineComp->ClearSplinePoints(true);

    for (int32 i = 0; i < Points.Num(); i++)
    {
        SplineComp->AddSplinePoint(Points[i], ESplineCoordinateSpace::World);

    }

    // Para evitar overshoot DEFINITIVAMENTE
    for (int32 i = 0; i < Points.Num(); i++)
    {
       SplineComp->SetSplinePointType(i, ESplinePointType::CurveClamped, false);
    }

    // Actualizar spline
    SplineComp->SetClosedLoop(false);
    SplineComp->UpdateSpline();

    //UKismetSystemLibrary::PrintString(
    //    this,
    //    FString::Printf(
    //        TEXT("SplinePoints: %d | Length: %.2f"),
    //        SplineComp->GetNumberOfSplinePoints(),
    //        SplineComp->GetSplineLength()
    //    ),
    //    true
    //);


    // VFX
    static const FName SpawnRateName(TEXT("SpawnRate"));
    NiagaraComp->SetFloatParameter(SpawnRateName, SplineComp->GetSplineLength());

    NiagaraComp->SetVisibility(true, true);
    NiagaraComp->ReinitializeSystem();
    NiagaraComp->ResetSystem();
    NiagaraComp->Activate(true);
}

void AAzulHiloBase::RecalculateHiloFromInput()
{
    if (!CachedPlayer || !HijoActor || !SplineComp)
    {
        return;
    }

    // -------------------------------------------------
    // BLOQUEAR MOVIMIENTO DEL JUGADOR
    // -------------------------------------------------
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (AAzulCharacterBase* Character = Cast<AAzulCharacterBase>(PC->GetPawn()))
        {
            if (Character->GetCharacterMovement())
            {
                Character->GetCharacterMovement()->DisableMovement();
                Character->bMovementLockedByHilo = true;
            }
        }
    }

    SplineComp->SetVisibility(true, true);

    // -------------------------------------------------
    // START POINT (jugador)
    // -------------------------------------------------
    const float StartStraightDistance = 50.0f;
    const float StartZOffset = 5.0f;

    FVector StartPos = CachedPlayer->GetActorLocation();
    StartPos.Z += StartZOffset;

    const FVector StartForward = CachedPlayer->GetActorForwardVector();

    FVector SecondPoint = StartPos + StartForward * StartStraightDistance;
    SecondPoint.Z = StartPos.Z;

    // -------------------------------------------------
    // END POINT (USANDO ARROW)
    // -------------------------------------------------
    FVector EndPos;
    FVector EndForward;

    if (HijoActor->HiloEndPoint)
    {
        EndPos = HijoActor->HiloEndPoint->GetComponentLocation();
        EndForward = HijoActor->HiloEndPoint->GetForwardVector(); // 🔥 CLAVE
    }
    else
    {
        EndPos = HijoActor->GetActorLocation();
        EndForward = HijoActor->GetActorForwardVector();
    }

    // Tramo recto final definido por el ARROW
    const float EndStraightDistance = 5.0f;
    const FVector PreEndPoint = EndPos - EndForward * EndStraightDistance;

    // -------------------------------------------------
    // SPLINE POINTS
    // -------------------------------------------------
    PreviousPoints = TargetPoints;
    TargetPoints.Empty();

    TargetPoints.Add(StartPos);        // Start
    TargetPoints.Add(SecondPoint);     // Recto inicial

    // Curva hasta el inicio del tramo final
    //const TArray<FVector> CurvedPoints =
    //    GenerateCurvedRoute(
    //        SecondPoint,
    //        StartForward,
    //        PreEndPoint
    //    );

    const TArray<FVector> CurvedPoints =
        GenerateSmoothCurvedRoute(
            SecondPoint,
            StartForward,
            PreEndPoint,
            EndForward
        );


    TargetPoints.Append(CurvedPoints);

    // Recto final (controlado por Arrow)
    TargetPoints.Add(PreEndPoint);
    TargetPoints.Add(EndPos);

    // -------------------------------------------------
    // APLICAR SPLINE
    // -------------------------------------------------
    OnSplineRouteChanged.Broadcast(PreviousPoints, TargetPoints);
    ApplyInterpolatedSplinePoints(TargetPoints);

    // -------------------------------------------------
    // ESTADO / VISIBILIDAD
    // -------------------------------------------------
    bHiloVisible = true;

    SetNiagaraLifeTime(100.0f);

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(Timer_HideHilo);
        GetWorld()->GetTimerManager().SetTimer(
            Timer_HideHilo,
            this,
            &AAzulHiloBase::HideHilo,
            HiloVisibleTime,
            false
        );
    }
}

TArray<FVector> AAzulHiloBase::GenerateSmoothCurvedRoute(
    const FVector& StartPos,
    const FVector& StartForward,
    const FVector& EndPos,
    const FVector& EndForward
)
{
    TArray<FVector> Result;

    const int32 NumPoints = 50;

    const float StartTangentLength = 80.0f;
    const float EndTangentLength = 20.0f;

    const FVector ControlPointA =
        StartPos + StartForward * StartTangentLength;

    const FVector ControlPointB =
        EndPos - EndForward * EndTangentLength;

    for (int32 i = 1; i <= NumPoints; ++i)
    {
        const float T = static_cast<float>(i) / NumPoints;

        const FVector Point =
            FMath::Pow(1 - T, 3) * StartPos +
            3 * FMath::Pow(1 - T, 2) * T * ControlPointA +
            3 * (1 - T) * FMath::Pow(T, 2) * ControlPointB +
            FMath::Pow(T, 3) * EndPos;

        Result.Add(Point);
    }

    return Result;
}





void AAzulHiloBase::HideHilo()
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (AAzulCharacterBase* Character = Cast<AAzulCharacterBase>(PC->GetPawn()))
        {
            if (Character->GetCharacterMovement())
            {
                Character->bMovementLockedByHilo = false;

                // Solo permitir mover si el tutorial no lo prohíbe
                if (!Character->bTutorialForbidMovementWhileHilo)
                {
                    Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
                }

            }
        }
    }

    if (!bHiloVisible)
        return;

    GetWorld()->GetTimerManager().ClearTimer(Timer_HideHilo);

    // Ocultar spline y VFX
    SplineComp->SetVisibility(false, true);

    NiagaraComp->Deactivate();
    NiagaraComp->SetVisibility(false, true);

    bHiloVisible = false;

    //NOTIFICAR SIEMPRE (timer, input, lo que sea)
    OnHiloHidden.Broadcast();
}



void AAzulHiloBase::ShowHilo()
{
    if (bHiloVisible)
        return;

    RecalculateHiloFromInput();
}

void AAzulHiloBase::ForceHideHilo()
{
    if (!bHiloVisible)
        return;

    GetWorld()->GetTimerManager().ClearTimer(Timer_HideHilo);
    HideHilo();
}


void AAzulHiloBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bHiloVisible || !CachedPlayer)
        return;

    FVector NewStart = CachedPlayer->GetActorLocation();
    NewStart.Z -= 20.f;

    // Interpolación suave (clave)
    FVector CurrentStart = SplineComp->GetLocationAtSplinePoint(
        0,
        ESplineCoordinateSpace::World
    );

    FVector Smoothed =
        FMath::VInterpTo(CurrentStart, NewStart, DeltaTime, 8.0f);

    SplineComp->SetLocationAtSplinePoint(
        0,
        Smoothed,
        ESplineCoordinateSpace::World,
        true
    );
}
