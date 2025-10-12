// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OSC/UsableItemBase.h"

struct FVector_NetQuantize;
struct FVector_NetQuantizeNormal;
#include "AimableItemBase.generated.h"

UCLASS()
class HORIZONTAL_API AAimableItemBase : public AUsableItemBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAimableItemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
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

	UPROPERTY(Transient, ReplicatedUsing = OnRep_IsAiming)
	bool bIsAiming = false;

	bool bStartLerpOnAim = false;
	bool bStartLerpOffAim = false;

	float LerpOnAimAlpha = 0;
	float LerpOffAimAlpha = 0;
	
	UPROPERTY(EditAnywhere)
	float InterpSpeed = 1.0f;
		
	
	UPROPERTY(EditAnywhere)
	float AimingFOV;
	UPROPERTY(EditAnywhere)
	float OriginFOV;

	FVector PendingUseStartLocation = FVector::ZeroVector;
	FVector PendingUseDirection = FVector::ZeroVector;
	bool bHasPendingUseData = false;
	
	UFUNCTION()
	void OnRep_IsAiming(bool Previous);
};
