// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Horror/HorrorGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Khc/InteractionObject/InteractableObjectBase.h"
#include "Kismet/GameplayStatics.h"

AHorrorGameMode::AHorrorGameMode()
{
	// stub
}

void AHorrorGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AHorrorGameMode::StartPlay()
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

	for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
	{
		if (APlayerController* PC = PS->GetPlayerController())
		{
			for (AInteractableObjectBase* Interactable : AllInteractableObjectsInLevel)
			{
				Interactable->BindToPlayerController(PC);
			}
			UE_LOG(LogTemp, Log, TEXT("StartPlay: Bound events for existing player '%s'."), *PC->GetName());
		}
	}
}

void AHorrorGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	for (AInteractableObjectBase* Interactable : AllInteractableObjectsInLevel)
	{
		if (Interactable)
		{
			Interactable->BindToPlayerController(NewPlayer);
		}
	}
    
	UE_LOG(LogTemp, Log, TEXT("A new player '%s' has logged in. Bound events to %d interactable objects."), *NewPlayer->GetName(), AllInteractableObjectsInLevel.Num());
}
