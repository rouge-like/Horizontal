#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDetected_Interaction, AActor*, DetectedPlayer);

// 플레이어가 NPC와 상호작용하는 방식
UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	InformSituation	UMETA(DisplayName = "Inform Situation"),
	ClearObstacle	UMETA(DisplayName = "Clear Obstacle"),
	CalmDown		UMETA(DisplayName = "Calm Down")
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORIZONTAL_API UNPCInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPCInteractionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(VisibleAnywhere, Category="Component")
	class USphereComponent* SphereComp;

	UPROPERTY(EditAnywhere)
	EInteractionType InteractionType = EInteractionType::InformSituation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UAnimMontage* StartAnimation;

	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnPlayerDetected_Interaction OnPlayerDetected;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
