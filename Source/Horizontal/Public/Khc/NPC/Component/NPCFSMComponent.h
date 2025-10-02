// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCFSMComponent.generated.h"

UENUM(BlueprintType)
enum class ENPCState : uint8
{
	Wait		UMETA(DisplayName = "Waiting For Help"),
	Move		UMETA(DisplayName = "Moving To Safe Zone"),
	Idle		UMETA(DisplayName = "Safe")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORIZONTAL_API UNPCFSMComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNPCFSMComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void SetState(ENPCState NewState);
	ENPCState GetState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FSM")
	ENPCState CurrentState = ENPCState::Wait;

	UPROPERTY(EditAnywhere, Category="FSM")
	float ArrivalThreshold = 100.0f;

private:
	UPROPERTY()
	class AActor* SafeZoneTarget;
};
