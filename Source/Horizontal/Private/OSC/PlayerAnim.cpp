// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/PlayerAnim.h"

#include "OSC/PlayerBase.h"

void UPlayerAnim::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	OwnerPawn = Cast<APlayerBase>(TryGetPawnOwner());
}

void UPlayerAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwnerPawn)
	{
		Velocity = OwnerPawn->GetVelocity();
		bIsCrouch = OwnerPawn->IsCrouched();
		bIsSprint = OwnerPawn->IsSprinting();

		DirV = FVector::DotProduct(Velocity, OwnerPawn->GetActorForwardVector());
		DirH = FVector::DotProduct(Velocity, OwnerPawn->GetActorRightVector());
	}
}
