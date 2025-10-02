// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Khc/NPC/NPCBase.h"

#include "Components/SphereComponent.h"
#include "Khc/NPC/Component/NPCAStarMovementComponent.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"
#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ANPCBase::ANPCBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FSMComp           = CreateDefaultSubobject<UNPCFSMComponent>(TEXT("FSMComp"));  
	AStarMovementComp = CreateDefaultSubobject<UNPCAStarMovementComponent>(TEXT("AStarMoveComp"));
	InteractionComp   = CreateDefaultSubobject<UNPCInteractionComponent>(TEXT("InteractionComp"));
	InteractionComp->SphereComp->SetSphereRadius(100.f);
	InteractionComp->SphereComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
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
		// 1. FSM 상태를 'Move'로 변경
		FSMComp->SetState(ENPCState::Move);

		// 2. Movement 컴포넌트에 따라갈 목표물(감지된 플레이어)을 설정
		AStarMovementComp->SetFollowTarget(DetectedPlayer);

		UE_LOG(LogTemp, Warning, TEXT("%s가 감지되었습니다. 따라가기 시작합니다."), *DetectedPlayer->GetName());
	}
}


// Called every frame
void ANPCBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}