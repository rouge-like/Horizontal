// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/RoomItem.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "OSC/Game/StartGameMode.h"
#include "OSC/UI/RoomObject.h"

void URoomItem::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	
	if (const URoomObject* RoomData = Cast<URoomObject>(ListItemObject))
	{
		DisplayName->SetText(FText::FromString(RoomData->DisplayName));
		RoomIndex = RoomData->RoomIndex;
		JoinButton->OnClicked.AddDynamic(this, &URoomItem::Join);
	}
}

void URoomItem::Join()
{
	AGameModeBase* GM = GetWorld()->GetAuthGameMode();

	if (IsValid(GM))
	{
		AStartGameMode* SGM = Cast<AStartGameMode>(GM);
		SGM->JoinOtherSession(RoomIndex);
	}
}
