// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/LobbyMainUI.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "OSC/LobbyPlayerController.h"
#include "OSC/Game/BaseGameInstance.h"
#include "OSC/Game/LobbyGameMode.h"

void ULobbyMainUI::NativeConstruct()
{
	Super::NativeConstruct();

	StartGameButton->OnClicked.AddDynamic(this, &ULobbyMainUI::OnStartButtonClick);
	ReadyButton->OnClicked.AddDynamic(this, &ULobbyMainUI::OnReadyButtonClick);
	
	StartGameButton->SetVisibility(GetWorld()->GetFirstPlayerController()->HasAuthority()? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	ReadyButton->SetVisibility(GetWorld()->GetFirstPlayerController()->HasAuthority()? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	
	UBaseGameInstance* BGI = Cast<UBaseGameInstance>(GetGameInstance());
	if (IsValid(BGI))
	{
		DisplayName->SetText(FText::FromString(BGI->GetDisplayName()));
	}
}

void ULobbyMainUI::OnStartButtonClick()
{
	ALobbyGameMode* LGM = Cast<ALobbyGameMode>(GetWorld()->GetAuthGameMode());
	LGM->StartGame();
}

void ULobbyMainUI::OnReadyButtonClick()
{
	if (!IsValid(PlayerController))
	{
		PlayerController = Cast<ALobbyPlayerController>(GetWorld()->GetFirstPlayerController());
		if (!IsValid(PlayerController))
			return;
	}
	bIsReady = !bIsReady;
	PlayerController->ServerPlayerReady(bIsReady);
}

void ULobbyMainUI::SetPlayerCount(int32 Count)
{
	if (PlayerCounter)
		PlayerCounter->SetText(FText::FromString(FString::Printf(TEXT("플레이어 : %d / 2"), Count)));
	
}

void ULobbyMainUI::SetStartButtonEnable(bool Enable)
{
	StartGameButton->SetIsEnabled(Enable);
}
