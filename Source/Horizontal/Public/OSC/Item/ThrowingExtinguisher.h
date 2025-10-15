#pragma once

#include "CoreMinimal.h"
#include "OSC/Item/AimableItemBase.h"
#include "ThrowingExtinguisher.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class HORIZONTAL_API AThrowingExtinguisher : public AAimableItemBase
{
    GENERATED_BODY()

public:
    AThrowingExtinguisher();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;
    virtual void HandleStartUse() override;
    virtual void HandleStopUse() override;
    virtual bool GatherUseData(FVector& OutStartLocation, FVector& OutDirection) const override;

    UFUNCTION(Server, Reliable)
    void ServerThrowProjectile(const FVector_NetQuantize& StartLocation, const FVector_NetQuantizeNormal& Direction);

    void SpawnAndLaunchProjectile(const FVector& StartLocation, const FVector& Direction);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    USphereComponent* CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    UProjectileMovementComponent* ProjectileMovement;

    UPROPERTY(EditDefaultsOnly, Category="Throw", meta=(ClampMin="0.0"))
    float ThrowSpeed = 1500.0f;

    UPROPERTY(Replicated)
    bool bIsInFlight = false;
};