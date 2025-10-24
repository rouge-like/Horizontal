// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

/**
 * 
 */
class ULobbyMainUI;
UCLASS()
class HORIZONTAL_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULobbyMainUI> MainUIClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ULobbyMainUI* MainUI;
	
	int32 PlayerCount;
	
	void MakeUI();

	UFUNCTION(Server, Reliable)
	void ServerOnJoinComplete();

	UPROPERTY(ReplicatedUsing = OnRep_IsReady)
	bool bIsReady = false;

	UFUNCTION()
	void OnRep_IsReady();
public:
	UFUNCTION(Client, Reliable)
	void ClientSetPlayerCount(int32 Count);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerPlayerReady(bool bSetReady);
	
	void SetPlayerCount(int32 Count);

	UPROPERTY(VisibleAnywhere)
	FVector PawnSpawnLocation;
};
