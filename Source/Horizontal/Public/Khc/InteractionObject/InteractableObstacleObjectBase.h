// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObjectBase.h"
#include "InteractableObstacleObjectBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObstacleStateChanged, const FBox&, BoundsToUpdate);

UCLASS()
class HORIZONTAL_API AInteractableObstacleObjectBase : public AInteractableObjectBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInteractableObstacleObjectBase();

	void ClearObstacle();

	// 상태 변경을 외부에 알리기 위한 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnObstacleStateChanged OnStateChanged;

protected:
	// 장애물의 실제 콜리전/메시 컴포넌트
	UPROPERTY(VisibleAnywhere)
	UPrimitiveComponent* ObstacleComponent;
};
