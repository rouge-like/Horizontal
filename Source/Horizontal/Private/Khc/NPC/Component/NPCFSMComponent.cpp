#include "Khc/NPC/Component/NPCFSMComponent.h"

#include "AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Khc/Gimmick/SafetyZone.h"
#include "Kismet/GameplayStatics.h"
#include "Khc/NPC/Component/NPCAStarMovementComponent.h"
#include "Khc/NPC/NPCBase.h"
#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "Net/UnrealNetwork.h"


UNPCFSMComponent::UNPCFSMComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UNPCFSMComponent::BeginPlay()
{
	Super::BeginPlay();
	
	

	ANPCBase* OwnerPawn = Cast<ANPCBase>(GetOwner());
	if (OwnerPawn && OwnerPawn->AStarMovementComp)
	{
		OwnerPawn->AStarMovementComp->OnMovementFinished.AddDynamic(this, &UNPCFSMComponent::OnMovementFinished);
	}
}

void UNPCFSMComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UNPCFSMComponent, CurrentState);
}

void UNPCFSMComponent::OnRep_CurrentState()
{
	UE_LOG(LogTemp, Warning, TEXT("Client State Synced: %d"), CurrentState);
}

void UNPCFSMComponent::OnMovementFinished()
{
	if (!GetOwner()->HasAuthority()) return;
	
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
	if (!GetOwner()->HasAuthority()) return;
	if (CurrentState == NewState) return;

	// 상태 변경을 먼저 기록
	CurrentState = NewState;
	OnRep_CurrentState(); 
       

	ANPCBase* OwnerPawn = Cast<ANPCBase>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->InteractionComp) return; // InteractionComp도 확인

	// 상태가 변경될 때마다 InteractionComp의 상태도 함께 변경
	OwnerPawn->InteractionComp->SetInteractable(CurrentState == ENPCState::Wait);
	
	switch (CurrentState)
	{
	case ENPCState::Wait:
		break;
	case ENPCState::Move:
		{
			if (OwnerPawn->TargetSafetyZone && OwnerPawn->AStarMovementComp)
			{
				OwnerPawn->AStarMovementComp->StartMovingTo(OwnerPawn->TargetSafetyZone->GetActorLocation());
				UE_LOG(LogTemp, Warning, TEXT("Server: State Changed to Move. Moving to SafeZone."));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Server: TargetSafetyZone is not set on NPC Actor!"));
				// 실패했으므로 상태를 다시 Wait로 되돌립니다.
				SetState(ENPCState::Wait); 
			}
			break;
		}
	case ENPCState::Idle:
		{
			if (OwnerPawn && OwnerPawn->GetController())
			{
				AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
				if (AIController)
				{
					AIController->StopMovement();
				}
			}
			UE_LOG(LogTemp, Warning, TEXT("Arrived at SafeZone. State Changed to Idle."));
			break;
		}
	default:
		break;
	}
	
}

ENPCState UNPCFSMComponent::GetState()
{
	return CurrentState;
}

