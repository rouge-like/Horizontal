// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/NPC/NPCAnimInstance.h"

#include "GameFramework/PawnMovementComponent.h"
#include "Khc/NPC/NPCBase.h"

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
	}
}
