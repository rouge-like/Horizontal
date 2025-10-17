// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VFXActor.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class HORIZONTAL_API AVFXActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVFXActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FName VFXName;

	UPROPERTY()
	USceneComponent* SceneComponent;
	
	UPROPERTY(EditAnywhere)
	UNiagaraComponent* NiagaraComponent;

	UPROPERTY(EditAnywhere)
	UParticleSystemComponent* ParticleSystemComponent;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FName GetVFXName() const { return VFXName; };
	void SetVFXName(FName Name) { VFXName = Name; };
	void StartVFX();
	void StopVFX();
};
