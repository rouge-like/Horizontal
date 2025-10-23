// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/LobbyPlayerController.h"
#include "OSC/UI/LobbyMainUI.h"
#include "Blueprint/UserWidget.h"
#include "OSC/Game/LobbyGameMode.h"

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

	MakeUI();
	bShowMouseCursor = true;
	
	if (HasAuthority())
	{
		SetPlayerCount(1);
	}
	else
	{
		ServerOnJoinComplete();
	}
}

void ALobbyPlayerController::MakeUI()
{
	if (MainUIClass)
	{
		MainUI = CreateWidget<ULobbyMainUI>(GetWorld(), MainUIClass);
		MainUI->AddToViewport();
	}
}

void ALobbyPlayerController::ServerOnJoinComplete_Implementation()
{
	ALobbyGameMode* LGM = Cast<ALobbyGameMode>(GetWorld()->GetAuthGameMode());
	if (IsValid(LGM))
		LGM->OnJoinedPlayer();
}

void ALobbyPlayerController::ClientSetPlayerCount_Implementation(int32 Count)
{
	SetPlayerCount(Count);
}

void ALobbyPlayerController::SetPlayerCount(int32 Count)
{
	PlayerCount = Count;
	MainUI->SetPlayerCount(PlayerCount);
}
