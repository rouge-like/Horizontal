// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/InteractionObject/ObjectInteractionComponent.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Khc/NPC/NPCBase.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UObjectInteractionComponent::UObjectInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // Tick은 필요 없음

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollision"));
	SetIsReplicatedByDefault(true);
}

void UObjectInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UObjectInteractionComponent, bIsInteractable);

	
}

void UObjectInteractionComponent::InitiateInteraction(ACharacter* InteractingPlayer)
{
	if (GetOwner()->HasAuthority())
	{
		OnPlayerDetected.Broadcast(InteractingPlayer);
	}
}

void UObjectInteractionComponent::SetInteractable(bool bNewState)
{
	if (GetOwner()->HasAuthority())
	{
		bIsInteractable = bNewState;
	}
}

