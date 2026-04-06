#include "Actors/AzulHiloBase.h"
#include "Actors/AzulInteractuableBase.h"
#include "Characters/AzulCharacterBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogAzulHilo, Log, All);

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

    UE_LOG(LogAzulHilo, Warning, TEXT("[CTOR] Hilo creado: %s | SplineComp=%s | NiagaraComp=%s"),
        *GetNameSafe(this),
        *GetNameSafe(SplineComp),
        *GetNameSafe(NiagaraComp));
}


void AAzulHiloBase::BeginPlay()
{
    Super::BeginPlay();

    CachedPlayer = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    ResolveNiagaraComponent();

    UE_LOG(LogAzulHilo, Warning, TEXT("[BeginPlay] Hilo=%s | World=%s | CachedPlayer=%s | HijoActor=%s | NiagaraTemplate=%s | VisibleTime=%.2f"),
        *GetNameSafe(this),
        *GetNameSafe(GetWorld()),
        *GetNameSafe(CachedPlayer),
        *GetNameSafe(HijoActor),
        *GetNameSafe(NiagaraTemplate),
        HiloVisibleTime);

    if (NiagaraTemplate)
    {
        NiagaraComp->SetAsset(NiagaraTemplate);

        UE_LOG(LogAzulHilo, Warning, TEXT("[BeginPlay] NiagaraTemplate asignado correctamente en %s"), *GetNameSafe(this));
    }

    else
    {
        UE_LOG(LogAzulHilo, Error, TEXT("[BeginPlay] NiagaraTemplate es NULL en %s"), *GetNameSafe(this));
    }
}


void AAzulHiloBase::SetNiagaraLifeTime(float Value)
{
    ResolveNiagaraComponent();

    if (!NiagaraComp)
    {
        UE_LOG(LogAzulHilo, Error, TEXT("[SetNiagaraLifeTime] ABORT: NiagaraComp es NULL en %s"), *GetNameSafe(this));
        return;
    }

    static const FName LifeTimeName(TEXT("LifeTime"));
    NiagaraComp->SetVariableFloat(LifeTimeName, Value);

    UE_LOG(LogAzulHilo, Verbose, TEXT("[SetNiagaraLifeTime] Hilo=%s | Value=%.2f"),
        *GetNameSafe(this), Value);
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

    UE_LOG(LogAzulHilo, Verbose, TEXT("[GenerateCurvedRoute] NumPoints=%d | Start=%s | End=%s"),
        Result.Num(), *StartPos.ToString(), *EndPos.ToString());

    return Result;
}

void AAzulHiloBase::ResolveNiagaraComponent()
{
    if (NiagaraComp)
    {
        return;
    }

    UE_LOG(LogAzulHilo, Warning, TEXT("[ResolveNiagaraComponent] NiagaraComp es NULL en %s. Intentando recuperarlo..."),
        *GetNameSafe(this));

    // 1) Buscar componente Niagara ya existente en el actor
    NiagaraComp = FindComponentByClass<UNiagaraComponent>();

    if (NiagaraComp)
    {
        UE_LOG(LogAzulHilo, Warning, TEXT("[ResolveNiagaraComponent] Recuperado con FindComponentByClass: %s"),
            *GetNameSafe(NiagaraComp));
        return;
    }

    // 2) Si no existe ninguno, crearlo en runtime
    NiagaraComp = NewObject<UNiagaraComponent>(this, UNiagaraComponent::StaticClass(), TEXT("NiagaraComp_Runtime"));

    if (NiagaraComp)
    {
        NiagaraComp->SetupAttachment(SplineComp ? SplineComp : RootComponent);
        NiagaraComp->RegisterComponent();
        NiagaraComp->SetAutoActivate(false);
        NiagaraComp->SetVisibility(false, true);

        if (NiagaraTemplate)
        {
            NiagaraComp->SetAsset(NiagaraTemplate);
        }

        UE_LOG(LogAzulHilo, Warning, TEXT("[ResolveNiagaraComponent] NiagaraComp creado en runtime: %s"),
            *GetNameSafe(NiagaraComp));
    }
    else
    {
        UE_LOG(LogAzulHilo, Error, TEXT("[ResolveNiagaraComponent] No se pudo recuperar ni crear NiagaraComp en %s"),
            *GetNameSafe(this));
    }
}



void AAzulHiloBase::ApplyInterpolatedSplinePoints(const TArray<FVector>& Points)
{
    ResolveNiagaraComponent();

    if (!SplineComp)
    {
        UE_LOG(LogAzulHilo, Error, TEXT("[ApplyInterpolatedSplinePoints] SplineComp es NULL en %s"),
            *GetNameSafe(this));
        return;
    }

    if (!NiagaraComp)
    {
        UE_LOG(LogAzulHilo, Error, TEXT("[ApplyInterpolatedSplinePoints] NiagaraComp sigue siendo NULL en %s"),
            *GetNameSafe(this));
        return;
    }

    UE_LOG(LogAzulHilo, Warning, TEXT("[ApplyInterpolatedSplinePoints] Hilo=%s | IncomingPoints=%d"),
        *GetNameSafe(this), Points.Num());

    SplineComp->ClearSplinePoints(true);

    for (int32 i = 0; i < Points.Num(); i++)
    {
        SplineComp->AddSplinePoint(Points[i], ESplineCoordinateSpace::World);
    }

    for (int32 i = 0; i < Points.Num(); i++)
    {
        SplineComp->SetSplinePointType(i, ESplinePointType::CurveClamped, false);
    }

    SplineComp->SetClosedLoop(false);
    SplineComp->UpdateSpline();

    UE_LOG(LogAzulHilo, Warning, TEXT("[ApplyInterpolatedSplinePoints] AppliedPoints=%d | SplineLength=%.2f | First=%s | Last=%s"),
        SplineComp->GetNumberOfSplinePoints(),
        SplineComp->GetSplineLength(),
        Points.Num() > 0 ? *Points[0].ToString() : TEXT("None"),
        Points.Num() > 0 ? *Points.Last().ToString() : TEXT("None"));

    static const FName SpawnRateName(TEXT("SpawnRate"));
    NiagaraComp->SetFloatParameter(SpawnRateName, SplineComp->GetSplineLength());
    NiagaraComp->SetVisibility(true, true);
    NiagaraComp->ReinitializeSystem();
    NiagaraComp->ResetSystem();
    NiagaraComp->Activate(true);

    UE_LOG(LogAzulHilo, Warning, TEXT("[ApplyInterpolatedSplinePoints] Niagara visible=%d active=%d"),
        NiagaraComp->IsVisible(),
        NiagaraComp->IsActive());
}

void AAzulHiloBase::RecalculateHiloFromInput()
{
    UE_LOG(LogAzulHilo, Warning, TEXT("[RecalculateHiloFromInput] START | Hilo=%s | CachedPlayer=%s | HijoActor=%s | SplineComp=%s | bHiloVisible=%d"),
        *GetNameSafe(this),
        *GetNameSafe(CachedPlayer),
        *GetNameSafe(HijoActor),
        *GetNameSafe(SplineComp),
        bHiloVisible);

    if (!CachedPlayer || !HijoActor || !SplineComp)
    {
        UE_LOG(LogAzulHilo, Error, TEXT("[RecalculateHiloFromInput] ABORT | CachedPlayer=%d | HijoActor=%d | SplineComp=%d"),
            CachedPlayer != nullptr,
            HijoActor != nullptr,
            SplineComp != nullptr);

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

                UE_LOG(LogAzulHilo, Warning, TEXT("[RecalculateHiloFromInput] Movimiento bloqueado para %s"),
                    *GetNameSafe(Character));
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
        EndForward = HijoActor->HiloEndPoint->GetForwardVector();

        UE_LOG(LogAzulHilo, Warning, TEXT("[RecalculateHiloFromInput] Usando HiloEndPoint | EndPos=%s | EndForward=%s"),
            *EndPos.ToString(), *EndForward.ToString());
    }
    else
    {
        EndPos = HijoActor->GetActorLocation();
        EndForward = HijoActor->GetActorForwardVector();

        UE_LOG(LogAzulHilo, Warning, TEXT("[RecalculateHiloFromInput] HiloEndPoint NULL, usando actor | EndPos=%s | EndForward=%s"),
            *EndPos.ToString(), *EndForward.ToString());
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

    UE_LOG(LogAzulHilo, Warning, TEXT("[RecalculateHiloFromInput] Route built | Previous=%d | Target=%d | Start=%s | Second=%s | PreEnd=%s | End=%s"),
        PreviousPoints.Num(),
        TargetPoints.Num(),
        *StartPos.ToString(),
        *SecondPoint.ToString(),
        *PreEndPoint.ToString(),
        *EndPos.ToString());

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

        UE_LOG(LogAzulHilo, Warning, TEXT("[RecalculateHiloFromInput] Timer_HideHilo lanzado | Duration=%.2f | bHiloVisible=%d"),
            HiloVisibleTime,
            bHiloVisible);
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

    UE_LOG(LogAzulHilo, Verbose, TEXT("[GenerateSmoothCurvedRoute] NumPoints=%d | Start=%s | End=%s | CPA=%s | CPB=%s"),
        Result.Num(),
        *StartPos.ToString(),
        *EndPos.ToString(),
        *ControlPointA.ToString(),
        *ControlPointB.ToString());

    return Result;
}





void AAzulHiloBase::HideHilo()
{
    UE_LOG(LogAzulHilo, Warning, TEXT("[HideHilo] START | Hilo=%s | bHiloVisible=%d"),
        *GetNameSafe(this), bHiloVisible);

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

                    UE_LOG(LogAzulHilo, Warning, TEXT("[HideHilo] Movimiento restaurado en %s"), *GetNameSafe(Character));
                }

                else
                {
                    UE_LOG(LogAzulHilo, Warning, TEXT("[HideHilo] Tutorial sigue bloqueando movimiento en %s"), *GetNameSafe(Character));
                }

            }
        }
    }

    if (!bHiloVisible)
    {
        UE_LOG(LogAzulHilo, Warning, TEXT("[HideHilo] ABORT porque ya estaba oculto"));
        return;
    }

    GetWorld()->GetTimerManager().ClearTimer(Timer_HideHilo);

    // Ocultar spline y VFX
    SplineComp->SetVisibility(false, true);

    NiagaraComp->Deactivate();
    NiagaraComp->SetVisibility(false, true);

    bHiloVisible = false;

    UE_LOG(LogAzulHilo, Warning, TEXT("[HideHilo] DONE | SplineVisible=%d | NiagaraVisible=%d | NiagaraActive=%d | bHiloVisible=%d"),
        SplineComp->IsVisible(),
        NiagaraComp->IsVisible(),
        NiagaraComp->IsActive(),
        bHiloVisible);

    //NOTIFICAR SIEMPRE (timer, input, lo que sea)
    OnHiloHidden.Broadcast();

    UE_LOG(LogAzulHilo, Warning, TEXT("[HideHilo] OnHiloHidden broadcast lanzado"));
}

void AAzulHiloBase::ShowHilo()
{
    UE_LOG(LogAzulHilo, Warning, TEXT("[ShowHilo] Called | Hilo=%s | bHiloVisible=%d"),
        *GetNameSafe(this), bHiloVisible);

    if (bHiloVisible)
    {
        UE_LOG(LogAzulHilo, Warning, TEXT("[ShowHilo] Ignorado porque ya está visible"));
        return;
    }

    RecalculateHiloFromInput();
}

void AAzulHiloBase::ForceHideHilo()
{
    UE_LOG(LogAzulHilo, Warning, TEXT("[ForceHideHilo] Called | Hilo=%s | bHiloVisible=%d"),
        *GetNameSafe(this), bHiloVisible);

    if (!bHiloVisible)
    {
        UE_LOG(LogAzulHilo, Warning, TEXT("[ForceHideHilo] Ignorado porque ya estaba oculto"));
        return;
    }

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

    //LOG
    static float TickLogAccumulator = 0.0f;
    TickLogAccumulator += DeltaTime;
    if (TickLogAccumulator >= 1.0f)
    {
        TickLogAccumulator = 0.0f;

        UE_LOG(LogAzulHilo, Warning, TEXT("[Tick] bHiloVisible=%d | CachedPlayer=%s | Point0=%s | Player=%s | SplineLength=%.2f"),
            bHiloVisible,
            *GetNameSafe(CachedPlayer),
            *Smoothed.ToString(),
            *NewStart.ToString(),
            SplineComp->GetSplineLength());
    }
}
