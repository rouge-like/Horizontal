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

	// 부모 로직이 통과되었다는 것은, 이미 상호작용이 가능하고 서버라는 의미.
	// 여기서는 NPC 고유의 추가 조건(FSM 상태)만 검사.
	ANPCBase* MyOwner = Cast<ANPCBase>(GetOwner());
	if (MyOwner && MyOwner->FSMComp && MyOwner->FSMComp->GetState() == ENPCState::Wait)
	{
		if (InteractionType == EInteractionType::InformSituation)
		{
			// 실제 로직 실행. OnInteraction 델리게이트는 이미 부모가 호출했음.
			// (예: FSM 상태 변경 등)
		}
	}
}