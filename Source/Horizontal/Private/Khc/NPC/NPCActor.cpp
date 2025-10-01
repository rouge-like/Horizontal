// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Khc/NPC/NPCActor.h"


// Sets default values
ANPCActor::ANPCActor()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANPCActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANPCActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANPCActor::SetNPCState(ENPCState NewState)
{
}

void ANPCActor::OnRep_NPCState()
{
}

void ANPCActor::Interact(APlayerController* InteractingPlayer, EInteractionType InteractionType)
{
}

void ANPCActor::Server_ProcessInteraction_Implementation(EInteractionType InteractionType)
{
}

bool ANPCActor::IsProblemSolved(EInteractionType InteractionType)
{
	return false;
}

void ANPCActor::MoveToSafeZone()
{
}
