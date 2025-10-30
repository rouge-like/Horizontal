// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Game/BaseGameInstance.h"
#include "MoviePlayer.h"
#include "OSC/PlayerBaseState.h"

void UBaseGameInstance::Init()
{
	Super::Init();
	
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UBaseGameInstance::BeginLoadingScreen);
}

void UBaseGameInstance::BeginLoadingScreen(const FString& MapName)
{
	if (IsRunningDedicatedServer()) return;

	// MoviePlayer 속성
	FLoadingScreenAttributes Attr;
	Attr.bAutoCompleteWhenLoadingCompletes = true; // 로딩 끝나면 자동 종료
	Attr.MinimumLoadingScreenDisplayTime = 2.f; // 너무 빨리 깜빡임 방지
	Attr.bWaitForManualStop = false; // 수동 종료 여부

	// mainMap 레벨로 전환할 때 특정 로딩 이미지 표시
	Attr.MoviePaths = { TEXT("Loading") };

	GetMoviePlayer()->SetupLoadingScreen(Attr);
	GetMoviePlayer()->PlayMovie();
}

void UBaseGameInstance::EndLoadingScreen(UWorld* LoadedWorld)
{
}

void UBaseGameInstance::AddRank(EValueType Type, int32 Rank)
{
	if (Ranks.Contains(Type))
		Ranks[Type] = Rank;
	else
		Ranks.Add(Type, Rank);
}
