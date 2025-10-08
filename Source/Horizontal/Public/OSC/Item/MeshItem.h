// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OSC/UsableItemBase.h"
#include "MeshItem.generated.h"

UCLASS()
class HORIZONTAL_API AMeshItem : public AUsableItemBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMeshItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void HandlePickupAvailabilityChanged() override;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* MeshComp;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
