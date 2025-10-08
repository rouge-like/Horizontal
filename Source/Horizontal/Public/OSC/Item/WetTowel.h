// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OSC/UsableItemBase.h"
#include "WetTowel.generated.h"

UCLASS()
class HORIZONTAL_API AWetTowel : public AUsableItemBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWetTowel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void HandlePickupAvailabilityChanged() override;

	virtual void HandleStartUse() override;
	virtual void HandleStopUse() override;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* MeshComp;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
