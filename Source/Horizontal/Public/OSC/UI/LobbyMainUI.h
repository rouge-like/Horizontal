// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyMainUI.generated.h"

class ALobbyPlayerController;
/**
 * 
 */
class UButton;
class UTextBlock;
UCLASS()
class HORIZONTAL_API ULobbyMainUI : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

protected:
	UPROPERTY()
	ALobbyPlayerController* PlayerController;

	bool bIsReady = false;
	
	UPROPERTY(meta=(BindWidget)) 
	UButton* StartGameButton;
	
	UPROPERTY(meta=(BindWidget))
	UButton* ReadyButton;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* DisplayName;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* PlayerCounter;

	UFUNCTION()
	void OnStartButtonClick();

	UFUNCTION()
	void OnReadyButtonClick();
public:
	void SetPlayerCount(int32 Count);
	void SetStartButtonEnable(bool Visibility);
};
