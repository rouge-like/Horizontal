// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/Khc/NPC/NPCBase.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Khc/Gimmick/AStarGridManager.h"
#include "Khc/Gimmick/AStarNavigationManager.h"
#include "Khc/NPC/Component/NPCAStarMovementComponent.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"
#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "Khc/Player/DialogueManagerComponent.h"
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

void ANPCBase::OnDialogueEventReceived(FName EventTag, AActor* InteractableActor)
{
	if (!InteractionComp || InteractableActor != InteractionComp->GetOwner()) 
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("NPC '%s' received EventTag: %s"), *GetName(), *EventTag.ToString());

	if (EventTag == "ResetInteraction")
	{
		if (InteractionComp)
		{
			InteractionComp->SetInteractable(true);
		}
	}
	else if (EventTag == "MoveToSafeZone")
	{
		if(FSMComp)
		{
			AAStarNavigationManager* NavManager = Cast<AAStarNavigationManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarNavigationManager::StaticClass()));
			if (NavManager)
			{
				AAStarGridManager* GridManager = NavManager->GetManagerForLocation(GetActorLocation());
				if (GridManager)
				{
					GridManager->RebuildGrid();
				}
			}
			FSMComp->SetState(ENPCState::Move);
		}
	}
}

void ANPCBase::BindToPlayerController(APlayerController* PC)
{
	if (PC)
	{
		UDialogueManagerComponent* DialogueManager = PC->FindComponentByClass<UDialogueManagerComponent>();
		if (DialogueManager)
		{
			// 이 NPC의 OnDialogueEventReceived 함수를 해당 DialogueManager의 델리게이트에 연결합니다.
			DialogueManager->OnDialogueEvent.AddDynamic(this, &ANPCBase::OnDialogueEventReceived);
		}
	}
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
