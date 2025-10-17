// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Khc/NPC/NPCBase.h"

#include "Components/SphereComponent.h"
#include "Khc/NPC/Component/NPCAStarMovementComponent.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"
#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "Net/UnrealNetwork.h"


ANPCBase::ANPCBase()
{
	PrimaryActorTick.bCanEverTick = true;

	FSMComp           = CreateDefaultSubobject<UNPCFSMComponent>(TEXT("FSMComp"));  
	AStarMovementComp = CreateDefaultSubobject<UNPCAStarMovementComponent>(TEXT("AStarMoveComp"));
	InteractionComp   = CreateDefaultSubobject<UNPCInteractionComponent>(TEXT("InteractionComp"));
	InteractionComp->SphereComp->SetSphereRadius(100.f);
	InteractionComp->SphereComp->SetupAttachment(RootComponent);
}

void ANPCBase::BeginPlay()
{
	Super::BeginPlay();

	if (InteractionComp)
	{
		InteractionComp->OnPlayerDetected.AddDynamic(this, &ANPCBase::OnPlayerDetected);
	}
}

void ANPCBase::OnPlayerDetected(AActor* DetectedPlayer)
{
	if (FSMComp && AStarMovementComp)
	{
		// FSM 상태 'Move'로 변경
		FSMComp->SetState(ENPCState::Move);

		UE_LOG(LogTemp, Warning, TEXT("이동 시작"));
	}
}


void ANPCBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}