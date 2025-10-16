#pragma once

#include "CoreMinimal.h"
#include "Khc/InteractableComponentBase.h"
#include "NPCInteractionComponent.generated.h"

// 플레이어가 NPC와 상호작용하는 방식
UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	InformSituation	UMETA(DisplayName = "Inform Situation"),
	ClearObstacle	UMETA(DisplayName = "Clear Obstacle"),
	CalmDown		UMETA(DisplayName = "Calm Down")
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORIZONTAL_API UNPCInteractionComponent : public UInteractableComponentBase
{
	GENERATED_BODY()

public:
	UNPCInteractionComponent();

	virtual void InitiateInteraction(ACharacter* InteractingPlayer) override;

public:
	// NPC에만 필요한 고유 속성들
	UPROPERTY(EditAnywhere)
	EInteractionType InteractionType = EInteractionType::InformSituation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UAnimMontage* StartAnimation;
};
