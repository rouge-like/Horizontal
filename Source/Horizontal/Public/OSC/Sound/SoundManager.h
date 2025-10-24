// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SoundManager.generated.h"

class ASoundActor;
class USoundBase;

UCLASS()
class HORIZONTAL_API ASoundManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASoundManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ASoundActor> SoundActorClass;
	
	UPROPERTY(EditAnywhere)
	TMap<FName, USoundBase*> Sounds;

	UPROPERTY()
	TArray<ASoundActor*> SoundActors;

	UPROPERTY(EditAnywhere)
	int32 InitialPoolSize = 50;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void InitPool();

	UFUNCTION(BlueprintCallable)
	ASoundActor* SpawnSoundAtLocation(FName Name, const FVector& Location);

	UFUNCTION()
	void DespawnSound(ASoundActor* SoundActor);
};
