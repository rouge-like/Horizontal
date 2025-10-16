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
	PrimaryComponentTick.bCanEverTick = false;

	//SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollision"));
	SetIsReplicatedByDefault(true);
}

void UObjectInteractionComponent::InitiateInteraction(ACharacter* InteractingCharacter)
{
	Super::InitiateInteraction(InteractingCharacter);

	Super::InitiateInteraction(InteractingCharacter);

	// 부모 로직 통과 후, 오브젝트 고유의 추가 로직 실행
	if (IsInteractable())
	{
		// 정보(Information) 타입이 아닐 경우, 한 번 상호작용하면 바로 잠금
		if (InteractionType != EObjectInteractionType::Information)
		{
			SetInteractable(false);
		}
	}
}

