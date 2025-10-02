// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCAStarMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORIZONTAL_API UNPCAStarMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNPCAStarMovementComponent();

	void SetFollowTarget(AActor* NewTarget);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	AActor* FollowTarget;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
