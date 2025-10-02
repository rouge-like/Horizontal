// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPCBase.generated.h"

UCLASS()
class HORIZONTAL_API ANPCBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANPCBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnPlayerDetected(AActor* DetectedPlayer);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	class UNPCFSMComponent* FSMComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	class UNPCAStarMovementComponent* AStarMovementComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	class UNPCInteractionComponent* InteractionComp;

};
