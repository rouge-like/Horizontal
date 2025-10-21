// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/PlayerBaseState.h"

#include "OSC/PlayerBase.h"

APlayerBaseState::APlayerBaseState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerBaseState::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerBaseState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!HasAuthority()) return;
	if (!PlayerBase) return;
	if (PlayerBase->IsSprinting()) RunningTime += DeltaTime;
	if (!PlayerBase->IsCoveringMouth()) NotCoveringMouth += DeltaTime;
}

void APlayerBaseState::SetPawn(APawn* Pawn)
{
	PlayerBase = Cast<APlayerBase>(Pawn);
}

void APlayerBaseState::AddFireTime(float DeltaTime)
{
	InFireTime += DeltaTime;
}

void APlayerBaseState::AddWrongCount(int32 Value)
{
	WrongCount += Value;
}
