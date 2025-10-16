// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Horror/HorrorGameMode.h"

#include "Khc/InteractionObject/InteractableObjectBase.h"
#include "Kismet/GameplayStatics.h"

AHorrorGameMode::AHorrorGameMode()
{
	// stub
}

void AHorrorGameMode::BeginPlay()
{
	Super::BeginPlay();
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteractableObjectBase::StaticClass(), FoundActors);
	for(AActor* Actor : FoundActors)
	{
		if(AInteractableObjectBase* Interactable = Cast<AInteractableObjectBase>(Actor))
		{
			AllInteractableObjectsInLevel.Add(Interactable);
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
			// 각 오브젝트에게 "이 새로운 플레이어와 연결해!" 라고 명령합니다.
			Interactable->BindToPlayerController(NewPlayer);
		}
	}
    
	UE_LOG(LogTemp, Log, TEXT("A new player '%s' has logged in. Bound events to %d interactable objects."), *NewPlayer->GetName(), AllInteractableObjectsInLevel.Num());
}
