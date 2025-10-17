// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFXManager.generated.h"

class AVFXActor;

USTRUCT()
struct FVFXActorPool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AVFXActor*> VFXs;
};
UCLASS()
class HORIZONTAL_API AVFXManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVFXManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TMap<FName, TSubclassOf<AVFXActor>> VFXActors;

	UPROPERTY(VisibleAnywhere)
	TMap<FName, FVFXActorPool> VFXPools;

	UPROPERTY(EditAnywhere)
	int32 InitialPoolSize = 50;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void InitPool(FName Name);
	
	AVFXActor* SpawnVFX(FName Name, const FVector& Location, const FRotator& Rotation, const FVector& Scale);

	void DespawnVFX(AVFXActor* VFX);
};
