// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartMainUI.generated.h"

class UListView;
class UEditableTextBox;
class UButton;
class AStartGameMode;
/**
 * 
 */
UCLASS()
class HORIZONTAL_API UStartMainUI : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeOnInitialized() override;
	
protected:
	UPROPERTY(meta = (BindWidget))
	UListView* RoomList;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* RoomNameTextBox;
	
	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* CreateRoomButton;

	UPROPERTY()
	AStartGameMode* StartGameMode;
public:
	UFUNCTION(BlueprintCallable)
	void UpdateRoomList(const TArray<FString>& RoomNames);

	UFUNCTION(BlueprintCallable)
	void RefreshRoomList();
	
	UFUNCTION(BlueprintCallable)
	void CreateRoom();
};
