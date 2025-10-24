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
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
protected:
	UPROPERTY(EditDefaultsOnly)
	FString URL;

	int32 PawnCount;
public:
	UFUNCTION(BlueprintCallable)
	void StartGame();

	UFUNCTION()
	void OnJoinedPlayer() const;
};
