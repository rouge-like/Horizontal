// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "RoomItem.generated.h"

class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class HORIZONTAL_API URoomItem : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DisplayName;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	int32 RoomIndex;
	UFUNCTION()
	void Join();
};
