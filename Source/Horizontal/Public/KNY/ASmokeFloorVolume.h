// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "ASmokeFloorVolume.generated.h"

UCLASS()
class HORIZONTAL_API ASmokeFloorVolume : public AActor
{
	GENERATED_BODY()
public:
	ASmokeFloorVolume();

	UPROPERTY(EditAnywhere) UBoxComponent* Bounds;
	UPROPERTY(EditAnywhere) UNiagaraComponent* NiagaraComp;

	UPROPERTY(EditAnywhere, Category="Smoke")
	int32 FloorIndex = 1;        

	UPROPERTY(EditAnywhere, Category="Smoke")
	float BaseSpawnRate = 2000.f;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	void PushUserParams();
};