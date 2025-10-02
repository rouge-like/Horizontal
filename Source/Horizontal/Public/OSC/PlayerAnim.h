// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnim.generated.h"

/**
 * 
 */
class APlayerBase;

UCLASS()
class HORIZONTAL_API UPlayerAnim : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	UPROPERTY()
	APlayerBase* OwnerPawn;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector Velocity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
 	float DirH;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
 	float DirV;

 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
 	bool bIsCrouch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsSprint;
};
