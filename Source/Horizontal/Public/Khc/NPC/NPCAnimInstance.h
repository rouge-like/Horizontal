// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NPCAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class HORIZONTAL_API UNPCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
public:
	class ANPCBase* me;

	UPROPERTY(BlueprintReadOnly)
	float speed = 0;
};
