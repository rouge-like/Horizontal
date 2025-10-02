// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/NPC/Component/NPCAStarMovementComponent.h"


// Sets default values for this component's properties
UNPCAStarMovementComponent::UNPCAStarMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UNPCAStarMovementComponent::SetFollowTarget(AActor* NewTarget)
{
	FollowTarget = NewTarget;
}


// Called when the game starts
void UNPCAStarMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UNPCAStarMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
	if (FollowTarget)
	{
		// 매 틱마다 목표물의 위치를 목적지로 설정하여 길찾기 및 이동 로직을 수행
		FVector Destination = FollowTarget->GetActorLocation();
        
		// 여기에 직접 구현하신 A* 길찾기 로직을 호출하여
		// 현재 NPC 위치에서 Destination까지의 경로를 계산하고, 그 경로를 따라 이동하는 코드를 작성합니다.

		// 참고: 매 틱마다 A* 경로 계산을 전부 다시 하는 것은 매우 무거울 수 있습니다.
		// 처음에는 이렇게 구현하고, 나중에 0.5초마다 한 번씩 경로를 갱신하는 식으로 최적화하는 것을 추천합니다.
	}
}

