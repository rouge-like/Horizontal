// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/StartMainUI.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ListView.h"
#include "OSC/Game/StartGameMode.h"
#include "OSC/UI/RoomObject.h"

void UStartMainUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	StartGameMode = Cast<AStartGameMode>(GetWorld()->GetAuthGameMode());
	RefreshButton->OnClicked.AddDynamic(this, &UStartMainUI::RefreshRoomList);
	CreateRoomButton->OnClicked.AddDynamic(this, &UStartMainUI::CreateRoom);
}

void UStartMainUI::UpdateRoomList(const TArray<FString>& RoomNames)
{
	RoomList->ClearListItems();

	for (const FString& DisplayName : RoomNames)
	{
		URoomObject* EntryObject = NewObject<URoomObject>(this);
		EntryObject->DisplayName = DisplayName;
		RoomList->AddItem(EntryObject);
	}
}

void UStartMainUI::RefreshRoomList()
{
	if (IsValid(StartGameMode))
		StartGameMode->FindOtherSessions();
}

void UStartMainUI::CreateRoom()
{
	if (IsValid(StartGameMode))
		StartGameMode->CreateMySession(RoomNameTextBox->GetText().ToString(), 2);
}
