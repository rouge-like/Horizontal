// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Game/LobbyGameMode.h"

#include "OnlineSessionSettings.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "OSC/LobbyPlayerController.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"

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
	int32 PlayerCount = GameState->PlayerArray.Num();
	
	for (int i = 0; i < PlayerCount; i++)
	{
		APlayerState* PS = GameState->PlayerArray[i];
		if (PS)
		{
			ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(PS->GetPlayerController());
			if (IsValid(LPC))
			{
				if (LPC->IsLocalController()) LPC->ClientMakeLoadingUI_Implementation();
				else LPC->ClientMakeLoadingUI();
			}
		}
	}

	FTimerHandle TravelHandle;
	GetWorldTimerManager().SetTimer(
		TravelHandle,
		FTimerDelegate::CreateLambda([this]()
		{
			GetWorld()->ServerTravel(URL);
		}),
		0.25f,  // UI 생성 대기 시간 (0.2~0.5초 추천)
		false
	);
	
	// bUseSeamlessTravel = true;
	
	// 비동기 로드
	// FLatentActionInfo LatentInfo;
	// LatentInfo.CallbackTarget = this;
	// LatentInfo.ExecutionFunction = FName("OnGameLevelLoaded");
	// LatentInfo.Linkage = 0;
	// LatentInfo.UUID = __LINE__; // 고유한 ID
	//
	// UGameplayStatics::LoadStreamLevel(
	// 	this,
	// 	LevelName, 
	// 	true,              
	// 	false,               
	// 	LatentInfo
	// );
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

void ALobbyGameMode::CheckPlayerReady()
{
	int32 PlayerCount = GameState->PlayerArray.Num();
	ALobbyPlayerController* ServerPC = nullptr;
	for (int i = 0; i < PlayerCount; i++)
	{
		APlayerState* PS = GameState->PlayerArray[i];
		if (PS)
		{
			ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(PS->GetPlayerController());
			if (LPC->IsLocalController())
			{
				ServerPC = LPC;
				continue;
			}
			if (!LPC->bIsReady)
			{
				UE_LOG(LogTemp, Warning, TEXT("True"));
				ServerPC->SetStartButtonEnable(false);
				return;
			}
		}
	}

	ServerPC->SetStartButtonEnable(true);
}

void ALobbyGameMode::OnGameLevelLoaded()
{
	UGameplayStatics::OpenLevel(GetWorld(), LevelName);
}
