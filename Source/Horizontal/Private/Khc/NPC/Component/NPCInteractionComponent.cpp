#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Khc/NPC/NPCBase.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"
#include "Net/UnrealNetwork.h"


UNPCInteractionComponent::UNPCInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollision"));
	SetIsReplicatedByDefault(true);
}

void UNPCInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNPCInteractionComponent, bIsInteractable);
}

void UNPCInteractionComponent::SetInteractable(bool bNewState)
{
	if (GetOwner()->HasAuthority())
	{
		bIsInteractable = bNewState;
	}
}

void UNPCInteractionComponent::InitiateInteraction(ACharacter* InteractingPlayer)
{
	//
	if (GetOwner()->HasAuthority())
	{
		ANPCBase* MyOwner = Cast<ANPCBase>(GetOwner());
		if (MyOwner && MyOwner->FSMComp && MyOwner->FSMComp->GetState() == ENPCState::Wait)
		{
			// InformSituation 타입의 상호작용 처리
			if (InteractionType == EInteractionType::InformSituation)
			{
				// OnPlayerDetected 델리게이트를 호출하여 NPCBase가 상태를 변경하도록 신호를 보냄
				OnPlayerDetected.Broadcast(InteractingPlayer);
			}
			// TODO: 다른 InteractionType에 대한 처리 추가 (전략 패턴 사용)
		}
	}
}