// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyMainUI.generated.h"

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
	UPROPERTY(meta=(BindWidget))
	UButton* StartGameButton;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* DisplayName;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* PlayerCounter;

	UFUNCTION()
	void OnStartButtonClick();

public:
	void SetPlayerCount(int32 Count);
};
