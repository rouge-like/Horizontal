#pragma once

#include "CoreMinimal.h"
#include "AimableItemBase.h"
#include "Extinguisher.generated.h"

class AFireManager;
class UCapsuleComponent;
class UAudioComponent;

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
    virtual void HandleStartAim() override;
    virtual void HandleStopAim() override;
    virtual bool GatherUseData(FVector& OutStartLocation, FVector& OutDirection) const override;
    virtual void HandlePickupAvailabilityChanged() override;

    FVector GetSprayStartLocation(const APlayerBase& Player) const;
    FVector GetSprayDirection(const APlayerBase& Player) const;

    void UpdateSpray(float DeltaTime);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    UCapsuleComponent* CollisionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components")
    UStaticMeshComponent* Body;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components")
    UStaticMeshComponent* Hose;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components")
    UStaticMeshComponent* HoseForAim;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components")
    UNiagaraComponent* Spray;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components")
    UAudioComponent* AudioComponent;
    
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

    UPROPERTY(ReplicatedUsing=OnRep_Spray)
    bool bIsSpraying = false;

    UPROPERTY(Replicated)
    FVector_NetQuantize CurrentSprayStart;

    UPROPERTY(Replicated)
    FVector_NetQuantizeNormal CurrentSprayDirection;

    float SprayUpdateAccumulator = 0.0f;

    UFUNCTION()
    void OnRep_Spray();
    
    virtual void OnRep_IsAiming(bool Previous) override;

    UFUNCTION(Server, Reliable)
    void ServerUpdateSpray(const FVector_NetQuantize& InStart, const FVector_NetQuantizeNormal& InDirection);

public:
    virtual void Tick(float DeltaTime) override;
};