#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCAStarMovementComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMovementFinished);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORIZONTAL_API UNPCAStarMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPCAStarMovementComponent();

	void StartMovingTo(const FVector& NewDestination);

	UPROPERTY(BlueprintAssignable)
	FOnMovementFinished OnMovementFinished;

protected:
	virtual void BeginPlay() override;


public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	//UPROPERTY()
	//class AAStarGridManager* GridManager;

	UPROPERTY()
	class AAStarNavigationManager* NavigationManager;

	// 계산된 경로를 저장할 배열
	TArray<FVector> CurrentPath;
	int32 CurrentPathIndex = 0;

	FVector Destination;
	bool bIsMoving = false;
};
