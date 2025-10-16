// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/InteractionObject/InteractableObstacleObjectBase.h"


// Sets default values
AInteractableObstacleObjectBase::AInteractableObstacleObjectBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AInteractableObstacleObjectBase::ClearObstacle()
{
	if (HasAuthority() && ObstacleComponent)
	{
		// 1. 장애물의 콜리전을 비활성화하여 길을 엽니다.
		ObstacleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ObstacleComponent->SetVisibility(false, true); // 메시도 숨김

		// 2. 이 장애물이 차지하던 영역(바운딩 박스) 정보를 담아 델리게이트를 방송합니다.
		OnStateChanged.Broadcast(ObstacleComponent->Bounds.GetBox());
	}
}

