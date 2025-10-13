// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Khc/NPC/NPCBase.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Khc/NPC/Component/NPCAStarMovementComponent.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"
#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


ANPCBase::ANPCBase()
{
	PrimaryActorTick.bCanEverTick = true;

	FSMComp           = CreateDefaultSubobject<UNPCFSMComponent>(TEXT("FSMComp"));  
	AStarMovementComp = CreateDefaultSubobject<UNPCAStarMovementComponent>(TEXT("AStarMoveComp"));
	InteractionComp   = CreateDefaultSubobject<UNPCInteractionComponent>(TEXT("InteractionComp"));
	InteractionComp->SphereComp->SetSphereRadius(100.f);
	InteractionComp->SphereComp->SetupAttachment(RootComponent);
	InteractionUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionUI"));
	InteractionUI->SetupAttachment(RootComponent);
}

void ANPCBase::BeginPlay()
{
	Super::BeginPlay();

	if (InteractionComp)
	{
		InteractionComp->OnPlayerDetected.AddDynamic(this, &ANPCBase::OnPlayerDetected);
	}

	if (InteractionUI)
		InteractionUI->SetVisibility(false);
}

void ANPCBase::OnPlayerDetected(AActor* DetectedPlayer)
{
	if (!HasAuthority()) return;

	// FSMComp가 유효한지 여기서 직접 확인하고 상태 변경을 지시합니다.
	if (FSMComp && FSMComp->GetState() == ENPCState::Wait)
	{
		FSMComp->SetState(ENPCState::Move);
		UE_LOG(LogTemp, Warning, TEXT("Server: State change to Move requested."));
	}
}

void ANPCBase::Server_RequestInteraction_Implementation()
{
}


void ANPCBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//BillboardUI();
}

// void ANPCBase::BillboardUI()
// {
// 	AActor* cam = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
// 	FRotator rot = UKismetMathLibrary::MakeRotFromXZ(-cam->GetActorForwardVector(), cam->GetActorUpVector());
//
// 	InteractionUI->SetWorldRotation(rot);
// }
