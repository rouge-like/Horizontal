// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Game/MainGameMode.h"
#include "Khc/InteractionObject/InteractableObjectBase.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Khc/NPC/NPCBase.h"
#include "Kismet/GameplayStatics.h"

AMainGameMode::AMainGameMode()
{
}

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();
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
