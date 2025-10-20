// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlayerBaseState.generated.h"

/**
 * 
 */
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

	UPROPERTY(EditDefaultsOnly)
	float InFireTime = 0;

public:
	float GetSize() const { return Size; };
	void AddFireTime(float DeltaTime);
};
