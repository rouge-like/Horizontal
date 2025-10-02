// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/NPC/Component/NPCFSMComponent.h"

#include "AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Khc/Gimmick/SafetyZone.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UNPCFSMComponent::UNPCFSMComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UNPCFSMComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SafeZoneTarget = UGameplayStatics::GetActorOfClass(GetWorld(), ASafetyZone::StaticClass());
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
		if (SafeZoneTarget)
		{
			float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), SafeZoneTarget->GetActorLocation());

			if (DistanceToTarget < ArrivalThreshold)
			{
				SetState(ENPCState::Idle);
			}
		}
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
			// 내비게이션 시스템을 이용해 SafeZone으로 이동을 시작합니다.
			// 이 함수를 사용하려면 NPC가 AIController를 가지고 있어야 합니다.
			UAIBlueprintHelperLibrary::SimpleMoveToActor(GetOwner()->GetInstigatorController(), SafeZoneTarget);
			UE_LOG(LogTemp, Warning, TEXT("State Changed to Move. Moving to SafeZone."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SafeZoneTarget을 찾을 수 없습니다!"));
		}
		break;
	case ENPCState::Idle:
		{
			AController* Controller = GetOwner()->GetInstigatorController();
			if (Controller)
			{
				AAIController* AIController = Cast<AAIController>(Controller);
				if (AIController)
				{
					AIController->StopMovement();
				}
				UE_LOG(LogTemp, Warning, TEXT("Arrived at SafeZone. State Changed to Idle."));
				break;
			}
		}
	default:
		break;
	}
}

ENPCState UNPCFSMComponent::GetState()
{
	return CurrentState;
}

