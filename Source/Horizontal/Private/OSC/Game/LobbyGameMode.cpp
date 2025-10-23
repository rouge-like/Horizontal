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

void ALobbyGameMode::StartGame()
{
	GetWorld()->ServerTravel(URL);
}

void ALobbyGameMode::OnJoinedPlayer()
{
	int32 PlayerCount = GameState->PlayerArray.Num();
	for (APlayerState* PS : GameState->PlayerArray)
	{
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
