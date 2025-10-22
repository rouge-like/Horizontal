// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlayerBaseState.generated.h"

/**
 * 
 */
class APlayerBase;
UCLASS()
class HORIZONTAL_API APlayerBaseState : public APlayerState
{
	GENERATED_BODY()
	APlayerBaseState();
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly)
	float Size = 35;

	UPROPERTY()
	APlayerBase* PlayerBase;
	
public:
	UPROPERTY(VisibleAnywhere)
	float PlayTime = 0;
	
	UPROPERTY(VisibleAnywhere)
	float InFireTime = 0;
	
	UPROPERTY(VisibleAnywhere)
	float RunningTime = 0;

	UPROPERTY(VisibleAnywhere)
	float NotCrouchingTime = 0;

	UPROPERTY(VisibleAnywhere)
	float NotCoveringMouth = 0;

	UPROPERTY(VisibleAnywhere)
	int32 WrongCount = 0;
public:
	void SetPawn(APawn* Pawn);
	float GetSize() const { return Size; };
	float GetSum() const {return InFireTime + RunningTime + NotCrouchingTime + NotCoveringMouth + WrongCount + PlayTime; };
	void AddFireTime(float DeltaTime);
	void AddWrongCount(int32 Value);
};
