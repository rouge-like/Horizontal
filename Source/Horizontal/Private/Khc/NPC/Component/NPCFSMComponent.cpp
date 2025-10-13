#include "Khc/NPC/Component/NPCFSMComponent.h"

#include "AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Khc/Gimmick/SafetyZone.h"
#include "Kismet/GameplayStatics.h"
#include "Khc/NPC/Component/NPCAStarMovementComponent.h"
#include "Khc/NPC/NPCBase.h"


UNPCFSMComponent::UNPCFSMComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UNPCFSMComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SafeZoneTarget = UGameplayStatics::GetActorOfClass(GetWorld(), ASafetyZone::StaticClass());

	ANPCBase* OwnerPawn = Cast<ANPCBase>(GetOwner());
	if (OwnerPawn && OwnerPawn->AStarMovementComp)
	{
		OwnerPawn->AStarMovementComp->OnMovementFinished.AddDynamic(this, &UNPCFSMComponent::OnMovementFinished);
	}
}

void UNPCFSMComponent::OnMovementFinished()
{
	if (CurrentState == ENPCState::Move)
	{
		SetState(ENPCState::Idle);
	}
}


void UNPCFSMComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (CurrentState)
	{
	case ENPCState::Wait:
		break;
	case ENPCState::Move:
		break;
	case ENPCState::Idle:
		break;
	}

}

void UNPCFSMComponent::SetState(ENPCState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	switch (NewState)
	{
	case ENPCState::Wait:
		break;
	case ENPCState::Move:
		if (SafeZoneTarget)
		{
			ANPCBase* OwnerPawn = Cast<ANPCBase>(GetOwner());
			if (OwnerPawn && OwnerPawn->AStarMovementComp)
			{
				// AStarMovementComponent에게 목적지를 알려주고 이동 시작
				OwnerPawn->AStarMovementComp->StartMovingTo(SafeZoneTarget->GetActorLocation());
				UE_LOG(LogTemp, Warning, TEXT("State Changed to Move. Moving to SafeZone."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SafeZoneTarget can't find."));
			SetState(ENPCState::Wait); // 목적지가 없으면 다시 Wait 상태로
		}
		break;
	case ENPCState::Idle:
		{
			ANPCBase* OwnerPawn = Cast<ANPCBase>(GetOwner());
			if (OwnerPawn && OwnerPawn->GetController())
			{
				AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
				if (AIController)
				{
					AIController->StopMovement();
				}
			}
			UE_LOG(LogTemp, Warning, TEXT("Arrived at SafeZone. State Changed to Idle."));
		}
	default:
		break;
	}
}

ENPCState UNPCFSMComponent::GetState()
{
	return CurrentState;
}

