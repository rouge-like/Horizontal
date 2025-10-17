#include "OSC/Item/Extinguisher.h"

#include "NiagaraComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "OSC/Fire/FireManager.h"
#include "OSC/PlayerBase.h"

AExtinguisher::AExtinguisher()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    AActor::SetReplicateMovement(true);

    CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComponent"));
    SetRootComponent(CollisionComponent);
    CollisionComponent->SetIsReplicated(true);
    CollisionComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
    CollisionComponent->SetSimulatePhysics(true);

    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
    Body->SetupAttachment(CollisionComponent);
    Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    Hose = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hose"));
    Hose->SetupAttachment(CollisionComponent);
    Hose->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    HoseForAim = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ForAim"));
    HoseForAim->SetupAttachment(CollisionComponent);
    HoseForAim->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Spray = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Spray"));
    Spray->SetupAttachment(CollisionComponent);
}

void AExtinguisher::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AExtinguisher, bIsSpraying);
    DOREPLIFETIME(AExtinguisher, CurrentSprayStart);
    DOREPLIFETIME(AExtinguisher, CurrentSprayDirection);
}

void AExtinguisher::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        FireManager = Cast<AFireManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AFireManager::StaticClass()));
    }

    HoseForAim->SetVisibility(false);
    Spray->SetVisibility(false);
}

void AExtinguisher::HandleStartUse()
{
    Super::HandleStartUse();

    if (!bIsAiming) return;
    
    APlayerBase* Player = OwningPlayer.Get();
    if (!IsValid(Player))
    {
        Player = Cast<APlayerBase>(GetOwner());
    }

    if (!IsValid(Player))
    {
        return;
    }
    
    FVector StartLocation;
    FVector Direction;
    const bool bHasData = GatherUseData(StartLocation, Direction);

    if (!bHasData)
    {
        StartLocation = GetSprayStartLocation(*Player);
        Direction = GetSprayDirection(*Player);
    }

    Direction = Direction.GetSafeNormal();
    if (Direction.IsNearlyZero())
    {
        Direction = Player->GetActorForwardVector().GetSafeNormal();
    }

    if (Direction.IsNearlyZero())
    {
        return;
    }

    CurrentSprayStart = StartLocation;
    CurrentSprayDirection = Direction;
    SprayUpdateAccumulator = 0.0f;

    if (!HasAuthority())
    {
        ServerUpdateSpray(CurrentSprayStart, CurrentSprayDirection);
    }
    else
    {
        Spray->SetVisibility(true);
        Spray->ResetSystem();
    }

    bIsSpraying = true;
}

void AExtinguisher::HandleStopUse()
{
    Super::HandleStopUse();

    if (HasAuthority())
    {
        bIsSpraying = false;
        SprayUpdateAccumulator = 0.0f;

        Spray->Deactivate();
    }
}

void AExtinguisher::HandleStartAim()
{
    Super::HandleStartAim();

    if (HasAuthority())
    {
        Hose->SetVisibility(false);
        HoseForAim->SetVisibility(true);
    }
}

void AExtinguisher::HandleStopAim()
{
    Super::HandleStopAim();

    if (HasAuthority())
    {
        bIsSpraying = false;
        SprayUpdateAccumulator = 0.0f;

        Hose->SetVisibility(true);
        HoseForAim->SetVisibility(false);
    }
}

bool AExtinguisher::GatherUseData(FVector& OutStartLocation, FVector& OutDirection) const
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

    OutStartLocation = GetSprayStartLocation(*Player);
    OutDirection = GetSprayDirection(*Player).GetSafeNormal();
    return !OutDirection.IsNearlyZero();
}

void AExtinguisher::HandlePickupAvailabilityChanged()
{
    Super::HandlePickupAvailabilityChanged();

    const bool bShouldSimulate = bCanBePickedUp;
    CollisionComponent->SetSimulatePhysics(bShouldSimulate);
    if (!bShouldSimulate)
    {
        CollisionComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
        CollisionComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }
}

FVector AExtinguisher::GetSprayStartLocation(const APlayerBase& Player) const
{
    if (const UCameraComponent* Camera = Player.GetFirstPersonCameraComponent())
    {
        return Camera->GetComponentLocation()
            + Camera->GetForwardVector() * LocalSprayOffset.X
            + Camera->GetRightVector() * LocalSprayOffset.Y
            + Camera->GetUpVector() * LocalSprayOffset.Z;
    }

    return GetActorLocation();
}

FVector AExtinguisher::GetSprayDirection(const APlayerBase& Player) const
{
    if (const UCameraComponent* Camera = Player.GetFirstPersonCameraComponent())
    {
        return Camera->GetForwardVector().GetSafeNormal();
    }

    return Player.GetActorForwardVector().GetSafeNormal();
}

void AExtinguisher::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateSpray(DeltaTime);
}

void AExtinguisher::UpdateSpray(float DeltaTime)
{
    if (!bIsSpraying)
    {
        return;
    }

    APlayerBase* Player = OwningPlayer.Get();
    if (!IsValid(Player))
    {
        Player = Cast<APlayerBase>(GetOwner());
    }

    if (!IsValid(Player))
    {
        return;
    }

    const bool bLocallyControlled = Player->IsLocallyControlled();
    if (bLocallyControlled)
    {
        FVector NewStart;
        FVector NewDirection;
        if (GatherUseData(NewStart, NewDirection))
        {
            CurrentSprayStart = NewStart;
            CurrentSprayDirection = NewDirection;

            SprayUpdateAccumulator += DeltaTime;
            if (!HasAuthority() && SprayUpdateAccumulator >= SprayUpdateInterval)
            {
                SprayUpdateAccumulator = 0.0f;
                ServerUpdateSpray(CurrentSprayStart, CurrentSprayDirection);
            }
        }
    }

    if (HasAuthority())
    {
        if (!IsValid(FireManager))
        {
            FireManager = Cast<AFireManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AFireManager::StaticClass()));
        }

        if (IsValid(FireManager))
        {
            FVector SprayStart = FVector(CurrentSprayStart);
            FVector SprayDirection = FVector(CurrentSprayDirection).GetSafeNormal();

            if (SprayDirection.IsNearlyZero())
            {
                SprayDirection = GetSprayDirection(*Player);
            }

            if (!SprayDirection.IsNearlyZero())
            {
                const FVector TraceEnd = SprayStart + SprayDirection * SprayRange;
                FHitResult Hit;
                FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ExtinguisherTrace), false, this);
                QueryParams.AddIgnoredActor(this);
                QueryParams.AddIgnoredActor(Player);

                const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, SprayStart, TraceEnd, ECC_Visibility, QueryParams);
                const FVector ImpactPoint = bHit ? Hit.ImpactPoint : TraceEnd;

                FireManager->ApplySuppressionInSphere(ImpactPoint, SuppressionRadius, SuppressionPerSecond * DeltaTime);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
                DrawDebugSphere(GetWorld(), ImpactPoint, SuppressionRadius, 16, FColor::Yellow, false, 0, 0, 1.5f);
#endif
            }
        }
    }
}

void AExtinguisher::OnRep_Spray()
{
    // Hook for client-side VFX if needed.
    if (bIsSpraying)
    {
        Spray->SetVisibility(true);
        Spray->ResetSystem();
    }
    else
    {
        Spray->Deactivate();
    }
}

void AExtinguisher::OnRep_IsAiming(bool Previous)
{
    Super::OnRep_IsAiming(Previous);
    // if (!OwningPlayer->IsLocallyControlled()) return;
    
    if (bIsAiming)
    {
        Hose->SetVisibility(false);
        HoseForAim->SetVisibility(true);
    }
    else
    {
        Hose->SetVisibility(true);
        HoseForAim->SetVisibility(false);
    }
}

void AExtinguisher::ServerUpdateSpray_Implementation(const FVector_NetQuantize& InStart, const FVector_NetQuantizeNormal& InDirection)
{
    CurrentSprayStart = InStart;
    CurrentSprayDirection = InDirection;
}
