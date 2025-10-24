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
#include "OSC/PlayerBaseState.h"


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
}

void ANPCBase::BeginPlay()
{
	Super::BeginPlay();
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
			//GetWorld()->GetFirstPlayerController()->GetPlayerState<APlayerBaseState>()->AddRecueScore(1);
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
			DialogueManager->OnDialogueEvent.AddDynamic(this, &ANPCBase::OnDialogueEventReceived);
		}
	}
}

void ANPCBase::SetReInteractable()
{
	if (InteractionComp)
	{
		InteractionComp->SetInteractable(true);
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
