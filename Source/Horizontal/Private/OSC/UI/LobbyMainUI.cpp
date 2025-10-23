// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/LobbyMainUI.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "OSC/Game/BaseGameInstance.h"
#include "OSC/Game/LobbyGameMode.h"

void ULobbyMainUI::NativeConstruct()
{
	Super::NativeConstruct();

	StartGameButton->OnClicked.AddDynamic(this, &ULobbyMainUI::OnStartButtonClick);
	StartGameButton->SetVisibility(GetWorld()->GetFirstPlayerController()->HasAuthority()? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	UBaseGameInstance* BGI = Cast<UBaseGameInstance>(GetGameInstance());
	if (IsValid(BGI))
	{
		DisplayName->SetText(FText::FromString(BGI->GetDisplayName()));
		UE_LOG(LogTemp, Warning, TEXT("Display Name : %s"), *BGI->GetDisplayName());
	}
}

void ULobbyMainUI::OnStartButtonClick()
{
	ALobbyGameMode* LGM = Cast<ALobbyGameMode>(GetWorld()->GetAuthGameMode());
	LGM->StartGame();
}

void ULobbyMainUI::SetPlayerCount(int32 Count)
{
	if (PlayerCounter)
		PlayerCounter->SetText(FText::FromString(FString::Printf(TEXT("플레이어 : %d / 2"), Count)));

	if (GetWorld()->GetFirstPlayerController()->HasAuthority() && Count == 2)
		StartGameButton->SetIsEnabled(true);
}
