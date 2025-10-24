// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Game/LobbyGameMode.h"

#include "OnlineSessionSettings.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "OSC/LobbyPlayerController.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemUtils.h"

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	NewPlayer->GetPawn()->SetActorLocation(FVector(0, 530 * PawnCount, 0));
	PawnCount++;
}

void ALobbyGameMode::StartGame()
{
	GetWorld()->ServerTravel(URL);
}

void ALobbyGameMode::OnJoinedPlayer() const
{
	int32 PlayerCount = GameState->PlayerArray.Num();
	
	for (int i = 0; i < PlayerCount; i++)
	{
		APlayerState* PS = GameState->PlayerArray[i];
		if (PS)
		{
			ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(PS->GetPlayerController());
			if (IsValid(LPC))
			{
				if (LPC->IsLocalController())
					LPC->SetPlayerCount(PlayerCount);
				else
					LPC->ClientSetPlayerCount(PlayerCount);
			}
		}
	}
}
