#pragma once

#include "CoreMinimal.h"
#include "AimableItemBase.h"
#include "Extinguisher.generated.h"

class AFireManager;
class USphereComponent;

UCLASS()
class HORIZONTAL_API AExtinguisher : public AAimableItemBase
{
    GENERATED_BODY()

public:
    AExtinguisher();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;
    virtual void HandleStartUse() override;
    virtual void HandleStopUse() override;
    virtual void HandleStopAim() override;
    virtual bool GatherUseData(FVector& OutStartLocation, FVector& OutDirection) const override;
    virtual void HandlePickupAvailabilityChanged() override;

    FVector GetSprayStartLocation(const APlayerBase& Player) const;
    FVector GetSprayDirection(const APlayerBase& Player) const;

    void UpdateSpray(float DeltaTime);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    USphereComponent* CollisionComponent;

    UPROPERTY(EditDefaultsOnly, Category="Spray")
    FVector LocalSprayOffset = FVector(15.0f, 0.0f, 0.0f);

    UPROPERTY(EditDefaultsOnly, Category="Spray", meta=(ClampMin="0.0"))
    float SprayRange = 600.0f;

    UPROPERTY(EditDefaultsOnly, Category="Spray", meta=(ClampMin="0.0"))
    float SuppressionPerSecond = 40.0f;

    UPROPERTY(EditDefaultsOnly, Category="Spray", meta=(ClampMin="0.0"))
    float SuppressionRadius = 120.0f;

    UPROPERTY(EditDefaultsOnly, Category="Spray", meta=(ClampMin="0.0"))
    float SprayUpdateInterval = 0.05f;

    UPROPERTY()
    AFireManager* FireManager;

    UPROPERTY(Replicated)
    bool bIsSpraying = false;

    UPROPERTY(ReplicatedUsing=OnRep_SprayData)
    FVector_NetQuantize CurrentSprayStart;

    UPROPERTY(ReplicatedUsing=OnRep_SprayData)
    FVector_NetQuantizeNormal CurrentSprayDirection;

    float SprayUpdateAccumulator = 0.0f;

    UFUNCTION()
    void OnRep_SprayData();

    UFUNCTION(Server, Reliable)
    void ServerUpdateSpray(const FVector_NetQuantize& InStart, const FVector_NetQuantizeNormal& InDirection);

public:
    virtual void Tick(float DeltaTime) override;
};