// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Game/LobbyGameMode.h"

ALobbyGameMode::ALobbyGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALobbyGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC)
		{
			UE_LOG(LogTemp, Log, TEXT("Found PlayerController: %s"), *PC->GetName());
		}
	}

}

void ALobbyGameMode::StartGame()
{
	GetWorld()->ServerTravel(URL);
}
