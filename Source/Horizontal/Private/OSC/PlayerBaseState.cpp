// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/PlayerBaseState.h"

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
	
	if (GetPlayerController()->IsLocalController())
	{
		GEngine->AddOnScreenDebugMessage(0, 0, FColor::Blue, FString::FormatAsNumber(InFireTime));
	}
}

void APlayerBaseState::AddFireTime(float DeltaTime)
{
	InFireTime += DeltaTime;
}
