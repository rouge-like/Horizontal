#include "OSC/Item/ThrowingExtinguisher.h"

#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "OSC/InventoryComponent.h"
#include "OSC/PlayerBase.h"
#include "OSC/PlayerBaseState.h"
#include "OSC/Fire/FireManager.h"
#include "OSC/VFX/VFXManager.h"

AThrowingExtinguisher::AThrowingExtinguisher()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    AActor::SetReplicateMovement(true);

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    SetRootComponent(CollisionComponent);
    CollisionComponent->SetIsReplicated(true);
    CollisionComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
    CollisionComponent->SetSimulatePhysics(true);
    CollisionComponent->InitSphereRadius(CollisionRadius);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(CollisionComponent);
    MeshComponent->SetIsReplicated(true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComponent->SetSimulatePhysics(false);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->bAutoActivate = false;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = ProjectileGravityScale;
    ProjectileMovement->InitialSpeed = ThrowSpeed;
    ProjectileMovement->MaxSpeed = ThrowSpeed;
    ProjectileMovement->PrimaryComponentTick.bStartWithTickEnabled = false;
    ProjectileMovement->SetComponentTickEnabled(false);

    TrajectorySpline = CreateDefaultSubobject<USplineComponent>(TEXT("TrajectorySpline"));
    TrajectorySpline->SetupAttachment(RootComponent);
    TrajectorySpline->SetMobility(EComponentMobility::Movable);

    HitSphereComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HitSphereComponent"));
    HitSphereComponent->SetupAttachment(CollisionComponent);
    HitSphereComponent->SetIsReplicated(true);
    HitSphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HitSphereComponent->SetSimulatePhysics(false);
    HitSphereComponent->SetVisibility(false);
}

void AThrowingExtinguisher::BeginPlay()
{
    Super::BeginPlay();

    EnsureSplineMeshPool();

    if (ProjectileMovement)
    {
        ProjectileMovement->SetUpdatedComponent(CollisionComponent);
        ProjectileMovement->ProjectileGravityScale = ProjectileGravityScale;
        ProjectileMovement->InitialSpeed = ThrowSpeed;
        ProjectileMovement->MaxSpeed = ThrowSpeed;
        ProjectileMovement->OnProjectileStop.AddDynamic(this, &AThrowingExtinguisher::HandleProjectileStop);
        ProjectileMovement->StopMovementImmediately();
        ProjectileMovement->Deactivate();
        ProjectileMovement->SetComponentTickEnabled(false);
    }
}

void AThrowingExtinguisher::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (ShouldShowTrajectory())
    {
        UpdateTrajectoryVisualization();
    }
    else
    {
        for (USplineMeshComponent* Mesh : ActiveSplineMeshes)
        {
            if (IsValid(Mesh))
            {
                Mesh->SetVisibility(false);
            }
        }
        HitSphereComponent->SetVisibility(false);
    }
}

void AThrowingExtinguisher::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    EnsureSplineMeshPool();
}

void AThrowingExtinguisher::HandleStartUse()
{
    Super::HandleStartUse();

    if (!HasAuthority() || bInFlight || !bIsAiming)
    {
        return;
    }

    APlayerBase* ThrowingPlayer = OwningPlayer.Get();
    if (!IsValid(ThrowingPlayer))
    {
        ThrowingPlayer = Cast<APlayerBase>(GetOwner());
    }

    if (!IsValid(ThrowingPlayer))
    {
        return;
    }

    FVector StartLocation = FVector::ZeroVector;
    FVector ThrowDirection = FVector::ZeroVector;
    const bool bHadPendingData = ConsumeUseData(StartLocation, ThrowDirection);
    if (!bHadPendingData)
    {
        StartLocation = GetThrowStartLocation(*ThrowingPlayer);
        ThrowDirection = GetThrowDirection(*ThrowingPlayer);
    }

    ThrowDirection = ThrowDirection.GetSafeNormal();
    if (ThrowDirection.IsNearlyZero())
    {
        ThrowDirection = ThrowingPlayer->GetActorForwardVector().GetSafeNormal();
    }

    if (ThrowDirection.IsNearlyZero())
    {
        return;
    }

    if (bIsAiming)
    {
        ClientHandleThrow();
        if (ThrowingPlayer->IsLocallyControlled())
        {
            ClientHandleThrow_Implementation();
        }
        HandleStopAim();
    }

    if (UInventoryComponent* Inventory = ThrowingPlayer->GetInventoryComponent())
    {
        if (Inventory->GetSelectedItem() == this)
        {
            Inventory->RemoveSelectedItem();
        }
        else
        {
            OnDrop();
        }
    }
    else
    {
        OnDrop();
    }

    LastThrowingActor = ThrowingPlayer;

    CollisionComponent->IgnoreActorWhenMoving(ThrowingPlayer, true);
    CollisionComponent->SetSimulatePhysics(false);
    CollisionComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
    CollisionComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

    SetActorLocation(StartLocation);
    SetActorRotation(ThrowDirection.Rotation());

    ProjectileMovement->Velocity = ThrowDirection * ThrowSpeed;
    ProjectileMovement->InitialSpeed = ThrowSpeed;
    ProjectileMovement->MaxSpeed = ThrowSpeed;
    ProjectileMovement->ProjectileGravityScale = ProjectileGravityScale;
    ProjectileMovement->SetUpdatedComponent(CollisionComponent);
    ProjectileMovement->SetComponentTickEnabled(true);
    ProjectileMovement->Activate(true);
    ProjectileMovement->UpdateComponentVelocity();

    bInFlight = true;
}

bool AThrowingExtinguisher::GatherUseData(FVector& OutStartLocation, FVector& OutDirection) const
{
    APlayerBase* ThrowingPlayer = OwningPlayer.Get();
    if (!IsValid(ThrowingPlayer))
    {
        ThrowingPlayer = Cast<APlayerBase>(GetOwner());
    }

    if (!IsValid(ThrowingPlayer) || !ThrowingPlayer->IsLocallyControlled())
    {
        OutStartLocation = FVector::ZeroVector;
        OutDirection = FVector::ZeroVector;
        return false;
    }

    OutStartLocation = GetThrowStartLocation(*ThrowingPlayer);
    OutDirection = GetThrowDirection(*ThrowingPlayer).GetSafeNormal();
    return !OutDirection.IsNearlyZero();
}


void AThrowingExtinguisher::HandlePickupAvailabilityChanged()
{
    Super::HandlePickupAvailabilityChanged();

    const bool bShouldSimulate = bCanBePickedUp && !bInFlight;
    CollisionComponent->SetSimulatePhysics(bShouldSimulate);
    if (!bShouldSimulate)
    {
        CollisionComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
        CollisionComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }
}

bool AThrowingExtinguisher::ShouldShowTrajectory() const
{
    if (bInFlight || GetNetMode() == NM_DedicatedServer)
    {
        return false;
    }

    if (!bIsAiming)
    {
        return false;
    }

    const APlayerBase* LocalOwner = OwningPlayer.Get();
    if (!IsValid(LocalOwner))
    {
        LocalOwner = Cast<APlayerBase>(GetOwner());
    }

    return IsValid(LocalOwner) && LocalOwner->IsLocallyControlled();
}

void AThrowingExtinguisher::UpdateTrajectoryVisualization()
{
    EnsureSplineMeshPool();

    APlayerBase* ThrowingPlayer = OwningPlayer.Get();
    if (!IsValid(ThrowingPlayer))
    {
        ThrowingPlayer = Cast<APlayerBase>(GetOwner());
    }

    if (!IsValid(ThrowingPlayer) || !GetWorld())
    {
        return;
    }

    const FVector LaunchLocation = GetThrowStartLocation(*ThrowingPlayer);
    const FVector LaunchVelocity = GetThrowDirection(*ThrowingPlayer) * ThrowSpeed;

    FPredictProjectilePathParams PredictParams;
    PredictParams.StartLocation = LaunchLocation;
    PredictParams.LaunchVelocity = LaunchVelocity;
    const float PredictedRadius = CollisionComponent ? CollisionComponent->GetScaledSphereRadius() : TrajectoryRadius;
    PredictParams.ProjectileRadius = PredictedRadius;
    PredictParams.bTraceWithCollision = true;
    PredictParams.SimFrequency = TrajectorySimFrequency;
    PredictParams.MaxSimTime = TrajectorySimTime;
    PredictParams.bTraceWithChannel = true;
    PredictParams.TraceChannel = TrajectoryTraceChannel;
    PredictParams.ActorsToIgnore.Add(ThrowingPlayer);
    PredictParams.ActorsToIgnore.Add(this);
    PredictParams.OverrideGravityZ = GetWorld()->GetGravityZ() * ProjectileGravityScale;

    FPredictProjectilePathResult PredictResult;
    const bool bHitSurface = UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);
    if (bHitSurface)
    {
        HitSphereComponent->SetWorldLocation(PredictResult.HitResult.Location);
        HitSphereComponent->SetVisibility(true);
        //DrawDebugSphere(GetWorld(), PredictResult.HitResult.Location, 10.f, 25, FColor::Green, false, 0.f);
    }
    
    const int32 PathPointCount = PredictResult.PathData.Num();
    if (PathPointCount < 2 || !TrajectorySpline)
    {
        if (TrajectorySpline)
        {
            TrajectorySpline->ClearSplinePoints();
        }

        for (USplineMeshComponent* Mesh : ActiveSplineMeshes)
        {
            if (IsValid(Mesh))
            {
                Mesh->SetVisibility(false);
            }
        }

        return;
    }

    TrajectorySpline->ClearSplinePoints();
    for (int32 PointIndex = 0; PointIndex < PathPointCount; ++PointIndex)
    {
        TrajectorySpline->AddSplinePoint(PredictResult.PathData[PointIndex].Location, ESplineCoordinateSpace::World, false);
        TrajectorySpline->SetSplinePointType(PointIndex, ESplinePointType::Curve, false);
    }
    TrajectorySpline->UpdateSpline();

    const int32 SegmentCount = PathPointCount - 1;
    const int32 VisibleSegmentCount = FMath::Min(SegmentCount, ActiveSplineMeshes.Num());

    for (int32 MeshIndex = 1; MeshIndex < ActiveSplineMeshes.Num(); ++MeshIndex)
    {
        USplineMeshComponent* Mesh = ActiveSplineMeshes[MeshIndex];
        if (!IsValid(Mesh))
        {
            continue;
        }
 
        if (MeshIndex < VisibleSegmentCount)
        {
            const FVector StartLocation = TrajectorySpline->GetLocationAtSplinePoint(MeshIndex, ESplineCoordinateSpace::Local);
            const FVector StartTangent = TrajectorySpline->GetTangentAtSplinePoint(MeshIndex, ESplineCoordinateSpace::Local);
            const FVector EndLocation = TrajectorySpline->GetLocationAtSplinePoint(MeshIndex + 1, ESplineCoordinateSpace::Local);
            const FVector EndTangent = TrajectorySpline->GetTangentAtSplinePoint(MeshIndex + 1, ESplineCoordinateSpace::Local);

            Mesh->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent, true);
            Mesh->SetStartScale(LineMeshScale);
            Mesh->SetEndScale(LineMeshScale);
            Mesh->SetVisibility(true);
        }
        else
        {
            Mesh->SetVisibility(false);
        }
    }
    // const FColor DrawColor = TrajectoryColor.ToFColor(true);
    // for (int32 Index = 0; Index < PredictResult.PathData.Num() - 1; ++Index)
    // {
    //     const FVector& SegmentStart = PredictResult.PathData[Index].Location;
    //     const FVector& SegmentEnd = PredictResult.PathData[Index + 1].Location;
    //     DrawDebugLine(GetWorld(), SegmentStart, SegmentEnd, DrawColor, false, 0, 0, 1.5f);
    // }
}

FVector AThrowingExtinguisher::GetThrowStartLocation(const APlayerBase& ThrowingPlayer) const
{
    if (const UCameraComponent* Camera = ThrowingPlayer.GetFirstPersonCameraComponent())
    {
        const FVector Forward = Camera->GetForwardVector();
        const FVector Right = Camera->GetRightVector();
        const FVector Up = Camera->GetUpVector();
        return Camera->GetComponentLocation()
            + Forward * LocalThrowOffset.X
            + Right * LocalThrowOffset.Y
            + Up * LocalThrowOffset.Z;
    }

    return GetActorLocation();
}

FVector AThrowingExtinguisher::GetThrowDirection(const APlayerBase& ThrowingPlayer) const
{
    if (const UCameraComponent* Camera = ThrowingPlayer.GetFirstPersonCameraComponent())
    {
        return Camera->GetForwardVector().GetSafeNormal();
    }

    return ThrowingPlayer.GetActorForwardVector().GetSafeNormal();
}

void AThrowingExtinguisher::HandleProjectileStop(const FHitResult& ImpactResult)
{
    bInFlight = false;

    if (ProjectileMovement)
    {
        ProjectileMovement->StopMovementImmediately();
        ProjectileMovement->Deactivate();
        ProjectileMovement->SetComponentTickEnabled(false);
    }

    CollisionComponent->SetSimulatePhysics(true);
    CollisionComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
    CollisionComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

    APlayerBase* Thrower = LastThrowingActor.Get();
    if (IsValid(Thrower))
    {
        CollisionComponent->IgnoreActorWhenMoving(Thrower, false);
    }
    LastThrowingActor = nullptr;


    AFireManager* FireManager = Cast<AFireManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AFireManager::StaticClass()));

    APlayerBaseState* PBS = Thrower->GetPlayerState<APlayerBaseState>();

    if (IsValid(PBS) && IsValid(FireManager))
        FireManager->ApplySuppressionInSphere(PBS, GetActorLocation(), HitRadius, 100);
    
    HandlePickupAvailabilityChanged();

    AVFXManager* VFXManager = Cast<AVFXManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AVFXManager::StaticClass()));
    VFXManager->SpawnVFX(VFXName, GetActorLocation(), GetActorRotation(), FVector(HitRadius / 100.f));
}


void AThrowingExtinguisher::ClientHandleThrow_Implementation()
{
    bStartLerpOnAim = false;
    bStartLerpOffAim = true;
    LerpOnAimAlpha = 0.0f;
    LerpOffAimAlpha = 0.0f;
}

void AThrowingExtinguisher::EnsureSplineMeshPool()
{
    if (!TrajectorySpline)
    {
        return;
    }

    for (int32 Index = ActiveSplineMeshes.Num() - 1; Index >= 0; --Index)
    {
        if (!IsValid(ActiveSplineMeshes[Index]))
        {
            ActiveSplineMeshes.RemoveAt(Index);
        }
    }

    const int32 DesiredCount = FMath::Max(MaxSegments, 0);
    while (ActiveSplineMeshes.Num() < DesiredCount)
    {
        USplineMeshComponent* Mesh = NewObject<USplineMeshComponent>(this);
        if (!IsValid(Mesh))
        {
            break;
        }

        Mesh->SetMobility(EComponentMobility::Movable);
        Mesh->AttachToComponent(TrajectorySpline, FAttachmentTransformRules::KeepRelativeTransform);
        Mesh->RegisterComponent();
        Mesh->SetVisibility(false);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        if (LineMesh)
        {
            Mesh->SetStaticMesh(LineMesh);
        }
        if (LineMaterial)
        {
            Mesh->SetMaterial(0, LineMaterial);
        }

        ActiveSplineMeshes.Add(Mesh);
    }

    for (USplineMeshComponent* Mesh : ActiveSplineMeshes)
    {
        if (!IsValid(Mesh))
        {
            continue;
        }

        Mesh->SetMobility(EComponentMobility::Movable);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        if (Mesh->GetStaticMesh() != LineMesh)
        {
            Mesh->SetStaticMesh(LineMesh);
        }

        if (LineMaterial)
        {
            Mesh->SetMaterial(0, LineMaterial);
        }
    }

    if (ActiveSplineMeshes.Num() > DesiredCount)
    {
        for (int32 Index = ActiveSplineMeshes.Num() - 1; Index >= DesiredCount; --Index)
        {
            if (IsValid(ActiveSplineMeshes[Index]))
            {
                ActiveSplineMeshes[Index]->DestroyComponent();
            }
            ActiveSplineMeshes.RemoveAt(Index);
        }
    }
}