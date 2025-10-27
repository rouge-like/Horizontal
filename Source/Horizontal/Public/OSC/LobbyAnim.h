// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "LobbyAnim.generated.h"

/**
 * 
 */
class ALobbyPreviewPawn;
UCLASS()
class HORIZONTAL_API ULobbyAnim : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY()
	ALobbyPreviewPawn* PreviewPawn;

	UPROPERTY(BlueprintReadOnly)
	bool bIsReady;
};
