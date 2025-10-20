// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Khc/NPC/NPCBase.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Khc/NPC/Component/NPCAStarMovementComponent.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"
#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"


ANPCBase::ANPCBase()
{
	PrimaryActorTick.bCanEverTick = true;

	FSMComp           = CreateDefaultSubobject<UNPCFSMComponent>(TEXT("FSMComp"));  
	AStarMovementComp = CreateDefaultSubobject<UNPCAStarMovementComponent>(TEXT("AStarMoveComp"));
	
	InteractionComp = CreateDefaultSubobject<UNPCInteractionComponent>(TEXT("InteractionComp"));
	if (InteractionComp && InteractionComp->SphereComp)
	{
		InteractionComp->SphereComp->SetupAttachment(RootComponent);
		InteractionComp->SphereComp->SetSphereRadius(100.f);
	}
	//
	// InteractionUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionUI"));
	// InteractionUI->SetupAttachment(RootComponent);
}

void ANPCBase::BeginPlay()
{
	Super::BeginPlay();
	//
	// if (InteractionUI)
	// 	InteractionUI->SetVisibility(false);
}

void ANPCBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANPCBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANPCBase, bHasBeenInteractedWith);
	DOREPLIFETIME(ANPCBase, TargetSafetyZone);
}
