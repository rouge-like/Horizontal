// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Game/StartGameMode.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Blueprint/UserWidget.h"
#include "Online/OnlineSessionNames.h"
#include "OSC/Game/BaseGameInstance.h"
#include "OSC/UI/RoomObject.h"
#include "OSC/UI/StartMainUI.h"

void AStartGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	
	PC->bShowMouseCursor = true;
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;
	
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();
		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &AStartGameMode::OnCreateSessionCompleted);
		SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &AStartGameMode::OnFindSessionsCompleted);
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &AStartGameMode::OnJoinSessionComplete);
	}

	if (MainUIClass)
	{
		MainUI = CreateWidget<UStartMainUI>(GetWorld(), MainUIClass);
		MainUI->AddToViewport();
	}
}

void AStartGameMode::CreateMySession(FString DisplayName, int32 PlayerCount)
{
	UE_LOG(LogTemp, Warning, TEXT("CreateSession"));
	FOnlineSessionSettings SessionSettings;
	
	FName SubSystemName = Online::GetSubsystem(GetWorld())->GetSubsystemName();
	
	SessionSettings.bIsLANMatch = SubSystemName.IsEqual(FName(TEXT("NULL")));
	UE_LOG(LogTemp, Warning, TEXT("%s"), *SubSystemName.ToString());
	// STEAM 필수
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bUsesPresence = true;

	SessionSettings.bShouldAdvertise = true;
	SessionSettings.NumPublicConnections = PlayerCount;
	SessionSettings.Set(FName("DP_NAME"), DisplayName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	UBaseGameInstance* BGI = Cast<UBaseGameInstance>(GetGameInstance());
	if (IsValid(BGI))
		BGI->SetDisplayName(DisplayName);
	
	FUniqueNetIdPtr NetId = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
	
	SessionInterface->CreateSession(*NetId, FName(DisplayName), SessionSettings);
}

void AStartGameMode::OnCreateSessionCompleted(FName SessionName, bool Success)
{
	if (Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] OnCreateSessionComplete"), *SessionName.ToString());

		GetWorld()->ServerTravel(URL);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] OnCreateSessionFail"), *SessionName.ToString());
	}
}

void AStartGameMode::FindOtherSessions()
{
	UE_LOG(LogTemp, Warning, TEXT("FindOtherSessions"));
	
	SessionSearch = MakeShared<FOnlineSessionSearch>();

	FName SubSystemName = Online::GetSubsystem(GetWorld())->GetSubsystemName();

	SessionSearch->bIsLanQuery = SubSystemName.IsEqual(FName(TEXT("NULL")));
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	SessionSearch->MaxSearchResults = 100;
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}


void AStartGameMode::OnFindSessionsCompleted(bool Success)
{
	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionsComplete"));
	if (Success)
	{
		DisplayNames.Reset();
		TArray<FOnlineSessionSearchResult> Results = SessionSearch->SearchResults;
		UE_LOG(LogTemp, Warning, TEXT("Results : %d"), Results.Num());
		for (int32 i = 0; i < Results.Num(); i++)
		{
			FString DisplayName;
			Results[i].Session.SessionSettings.Get(FName("DP_NAME"), DisplayName);
			UE_LOG(LogTemp, Warning, TEXT("Session No.%d %s"),i, *DisplayName);
			DisplayNames.Add(DisplayName);
		}
		MainUI->UpdateRoomList(DisplayNames);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Session No"));
	}
}

void AStartGameMode::JoinOtherSession(int32 SessionIndex)
{
	auto Results = SessionSearch->SearchResults;

	Results[SessionIndex].Session.SessionSettings.bUseLobbiesIfAvailable = true;
	Results[SessionIndex].Session.SessionSettings.bUsesPresence = true;

	FString DisplayName;
	Results[SessionIndex].Session.SessionSettings.Get(FName("DP_NAME"), DisplayName);
	
	SessionInterface->JoinSession(0, FName(DisplayName), Results[SessionIndex]);
}

void AStartGameMode::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		FString SelectedURL;
		SessionInterface->GetResolvedConnectString(SessionName, SelectedURL);
		UE_LOG(LogTemp, Warning, TEXT("URL [%s] %s"), *SelectedURL, *SessionName.ToString());
		UBaseGameInstance* BGI = Cast<UBaseGameInstance>(GetGameInstance());
		if (IsValid(BGI))
			BGI->SetDisplayName(FString(SessionName.ToString()));

		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		PlayerController->ClientTravel(SelectedURL, TRAVEL_Absolute);
	}
}
