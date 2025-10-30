// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MainGameState.generated.h"

/**
 * 
 */
UCLASS()
class HORIZONTAL_API AMainGameState : public AGameStateBase
{
	GENERATED_BODY()
	

public:
    UPROPERTY(Replicated)
    bool bVoiceEventCompleted;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
