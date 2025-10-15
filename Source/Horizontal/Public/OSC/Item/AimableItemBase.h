#pragma once

#include "CoreMinimal.h"
#include "OSC/UsableItemBase.h"
#include "AimableItemBase.generated.h"

class UNiagaraComponent;
struct FVector_NetQuantize;
struct FVector_NetQuantizeNormal;

UCLASS()
class HORIZONTAL_API AAimableItemBase : public AUsableItemBase
{
    GENERATED_BODY()

public:
    AAimableItemBase();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    virtual void Tick(float DeltaSeconds) override;
    virtual void StartUse() override;

    void StartAim();
    void StopAim();

    virtual void OnRep_Owner() override;

protected:
    virtual void OnEquip() override;
    virtual void OnUnequip() override;

    virtual void HandleStartAim();
    virtual void HandleStopAim();

    virtual bool GatherUseData(FVector& OutStartLocation, FVector& OutDirection) const;
    bool ConsumeUseData(FVector& OutStartLocation, FVector& OutDirection);
    void SetPendingUseData(const FVector& InStartLocation, const FVector& InDirection, bool bIsValid);

    UFUNCTION(Server, Reliable)
    void ServerStartUseWithAimData(const FVector_NetQuantize& ClientStartLocation, const FVector_NetQuantizeNormal& ClientDirection, bool bClientProvidedData);

    UFUNCTION(Server, Reliable)
    void ServerStartAim();

    UFUNCTION(Server, Reliable)
    void ServerStopAim();

    UFUNCTION()
    void OnRep_IsAiming(bool Previous);

protected:
    UPROPERTY(Transient, ReplicatedUsing = OnRep_IsAiming)
    bool bIsAiming = false;

    bool bStartLerpOnAim = false;
    bool bStartLerpOffAim = false;

    float LerpOnAimAlpha = 0.0f;
    float LerpOffAimAlpha = 0.0f;

    UPROPERTY(EditAnywhere, Category="Aim")
    float InterpSpeed = 1.0f;

    UPROPERTY(EditAnywhere, Category="Aim")
    float AimingFOV = 70.0f;

    UPROPERTY(Transient)
    float OriginFOV = 90.0f;

    FVector PendingUseStartLocation = FVector::ZeroVector;
    FVector PendingUseDirection = FVector::ZeroVector;
    bool bHasPendingUseData = false;
};