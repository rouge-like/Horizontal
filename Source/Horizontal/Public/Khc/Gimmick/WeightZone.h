// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeightZone.generated.h"

UCLASS()
class HORIZONTAL_API AWeightZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeightZone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "A* Weight")
	float MovementWeight = 2.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A* Weight")
	class UBoxComponent* WeightArea;

	FBox GetWeightBox() const;
};
