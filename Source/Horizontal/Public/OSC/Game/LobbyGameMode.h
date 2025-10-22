// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class HORIZONTAL_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	virtual void BeginPlay() override;
protected:
	UPROPERTY(EditDefaultsOnly)
	FString URL;

public:
	UFUNCTION(BlueprintCallable)
	void StartGame();

	UFUNCTION()
	void OnJoinedPlayer();
};
