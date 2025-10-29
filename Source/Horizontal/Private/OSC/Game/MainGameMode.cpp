// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Game/MainGameMode.h"
#include "Khc/InteractionObject/InteractableObjectBase.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Khc/NPC/NPCBase.h"
#include "Kismet/GameplayStatics.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/VoiceInterface.h"
#include "GameFramework/PlayerState.h"

AMainGameMode::AMainGameMode()
{
}

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();

	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (Subsystem)
	{
		// 보이스 인터페이스를 찾아서 멤버 변수 'VoiceInterface'에 저장
		VoiceInterface = Subsystem->GetVoiceInterface();
		if (VoiceInterface.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("AMainGameMode: Voice Interface successfully cached on server."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AMainGameMode: Failed to get Voice Interface!"));
		}
	}
}

void AMainGameMode::StartPlay()
{
	Super::StartPlay();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteractableObjectBase::StaticClass(), FoundActors);
	for(AActor* Actor : FoundActors)
	{
		if(AInteractableObjectBase* Interactable = Cast<AInteractableObjectBase>(Actor))
		{
			AllInteractableObjectsInLevel.Add(Interactable);
		}
	}

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPCBase::StaticClass(), FoundActors);
	for(AActor* Actor : FoundActors)
	{
		if(ANPCBase* NPC = Cast<ANPCBase>(Actor))
		{
			AllNPCsInLevel.Add(NPC); // AllNPCsInLevel 변수를 헤더에 추가해야 함
		}
	}
	
	for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
	{
		if (APlayerController* PC = PS->GetPlayerController())
		{
			for (AInteractableObjectBase* Interactable : AllInteractableObjectsInLevel)
			{
				Interactable->BindToPlayerController(PC);
			}

			for (ANPCBase* NPC : AllNPCsInLevel)
			{
				NPC->BindToPlayerController(PC);
			}
			UE_LOG(LogTemp, Log, TEXT("StartPlay: Bound events for existing player '%s'."), *PC->GetName());
		}
	}
}

void AMainGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer && NewPlayer->PlayerState && VoiceInterface.IsValid())
	{
		FUniqueNetIdPtr UniqueNetId = NewPlayer->PlayerState->GetUniqueId().GetUniqueNetId();
		if (UniqueNetId.IsValid())
		{
			VoiceInterface->RegisterRemoteTalker(*UniqueNetId);
			UE_LOG(LogTemp, Log, TEXT("Player '%s' has been registered as a remote talker."), *NewPlayer->GetName());
		}
	}
	
	for (AInteractableObjectBase* Interactable : AllInteractableObjectsInLevel)
	{
		if (Interactable)
		{
			Interactable->BindToPlayerController(NewPlayer);
		}
	}

	for (ANPCBase* NPC : AllNPCsInLevel)
	{
		if (NPC)
		{
			NPC->BindToPlayerController(NewPlayer);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("A new player '%s' has logged in. Bound events to %d interactable objects."), *NewPlayer->GetName(), AllInteractableObjectsInLevel.Num());
}
