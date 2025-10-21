// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/NPC/NPCAnimInstance.h"

#include "GameFramework/PawnMovementComponent.h"
#include "Khc/NPC/NPCBase.h"
#include "Khc/NPC/Component/NPCInteractionComponent.h"

void UNPCAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	me = Cast<ANPCBase>(TryGetPawnOwner());
}

void UNPCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (me)
	{
		speed = me->GetMovementComponent()->Velocity.Length();
		bPlayStartAnimation = me->InteractionComp->bIsInteractable;
	}
}
