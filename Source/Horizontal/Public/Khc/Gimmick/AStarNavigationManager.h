// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AStarNavigationManager.generated.h"

class AAStarGridManager;

UCLASS()
class HORIZONTAL_API AAStarNavigationManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAStarNavigationManager();

	AAStarGridManager* GetManagerForLocation(const FVector& Location);

	bool FindPath(FVector StartLocation, FVector TargetLocation, TArray<FVector>& OutPath);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TArray<TObjectPtr<AAStarGridManager>> AllGridManagers;

	
};
