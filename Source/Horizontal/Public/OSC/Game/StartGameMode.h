// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "StartGameMode.generated.h"

class UStartMainUI;
/**
 * 
 */
UCLASS()
class HORIZONTAL_API AStartGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	
	IOnlineSessionPtr SessionInterface;
	
	FName CurrentSessionName;

	UFUNCTION(BlueprintCallable)
	void CreateMySession(FString DisplayName, int32 PlayerCount);

	void OnCreateSessionCompleted(FName SessionName, bool Success);

	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
	UFUNCTION(BlueprintCallable)
	void FindOtherSessions();
	void OnFindSessionsCompleted(bool Success);

	UFUNCTION(BlueprintCallable)
	void JoinOtherSession(int32 SessionIndex);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UStartMainUI> MainUIClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStartMainUI* MainUI;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FString> DisplayNames;

	UPROPERTY(EditDefaultsOnly)
	FString URL;
};
