#include "OSC/Item/ThrowingExtinguisher.h"

#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "OSC/PlayerBase.h"

AThrowingExtinguisher::AThrowingExtinguisher()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
    CollisionComponent->SetSimulatePhysics(false);
    CollisionComponent->InitSphereRadius(16.0f);
    SetRootComponent(CollisionComponent);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(CollisionComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->bAutoActivate = false;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
    ProjectileMovement->InitialSpeed = ThrowSpeed;
    ProjectileMovement->MaxSpeed = ThrowSpeed;
}

void AThrowingExtinguisher::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AThrowingExtinguisher, bIsInFlight);
}

void AThrowingExtinguisher::BeginPlay()
{
    Super::BeginPlay();

    if (ProjectileMovement)
    {
        ProjectileMovement->Deactivate();
    }
}

void AThrowingExtinguisher::HandleStartUse()
{
    Super::HandleStartUse();

    FVector StartLocation;
    FVector Direction;
    const bool bHasData = GatherUseData(StartLocation, Direction);

    if (!bHasData)
    {
        if (APlayerBase* Player = OwningPlayer.Get())
        {
            StartLocation = Player->GetActorLocation() + Player->GetActorForwardVector() * 50.0f;
            Direction = Player->GetActorForwardVector();
        }
        else
        {
            StartLocation = GetActorLocation();
            Direction = GetActorForwardVector();
        }
    }

    const FVector NormalizedDirection = Direction.GetSafeNormal();
    if (NormalizedDirection.IsNearlyZero())
    {
        return;
    }

    if (HasAuthority())
    {
        SpawnAndLaunchProjectile(StartLocation, NormalizedDirection);
    }
    else
    {
        ServerThrowProjectile(StartLocation, NormalizedDirection);
    }
}

void AThrowingExtinguisher::HandleStopUse()
{
    Super::HandleStopUse();
}

bool AThrowingExtinguisher::GatherUseData(FVector& OutStartLocation, FVector& OutDirection) const
{
    APlayerBase* Player = OwningPlayer.Get();
    if (!IsValid(Player))
    {
        Player = Cast<APlayerBase>(GetOwner());
    }

    if (!IsValid(Player) || !Player->IsLocallyControlled())
    {
        OutStartLocation = FVector::ZeroVector;
        OutDirection = FVector::ZeroVector;
        return false;
    }

    const UCameraComponent* Camera = Player->GetFirstPersonCameraComponent();
    if (Camera)
    {
        OutStartLocation = Camera->GetComponentLocation();
        OutDirection = Camera->GetForwardVector();
        return true;
    }

    OutStartLocation = Player->GetActorLocation();
    OutDirection = Player->GetActorForwardVector();
    return true;
}

void AThrowingExtinguisher::ServerThrowProjectile_Implementation(const FVector_NetQuantize& StartLocation, const FVector_NetQuantizeNormal& Direction)
{
    SpawnAndLaunchProjectile(StartLocation, FVector(Direction));
}

void AThrowingExtinguisher::SpawnAndLaunchProjectile(const FVector& StartLocation, const FVector& Direction)
{
    if (!ProjectileMovement)
    {
        return;
    }

    SetActorLocation(StartLocation);
    SetActorRotation(Direction.Rotation());

    ProjectileMovement->Velocity = Direction * ThrowSpeed;
    ProjectileMovement->Activate(true);
    bIsInFlight = true;
}