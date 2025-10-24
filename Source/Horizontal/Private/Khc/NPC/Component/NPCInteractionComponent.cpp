#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Khc/NPC/NPCBase.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"
#include "Net/UnrealNetwork.h"


UNPCInteractionComponent::UNPCInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UNPCInteractionComponent::InitiateInteraction(ACharacter* InteractingPlayer)
{
	Super::InitiateInteraction(InteractingPlayer);

	ANPCBase* MyOwner = Cast<ANPCBase>(GetOwner());
	if (MyOwner && MyOwner->FSMComp && MyOwner->FSMComp->GetState() == ENPCState::Wait)
	{
		if (InteractionType == EInteractionType::InformSituation)
		{
		}
	}
}