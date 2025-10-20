#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPCBase.generated.h"

class ASafetyZone;

UCLASS()
class HORIZONTAL_API ANPCBase : public ACharacter
{
	GENERATED_BODY()

public:
	ANPCBase();

protected:
	virtual void BeginPlay() override;


public:
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	bool bHasBeenInteractedWith;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	class UNPCFSMComponent* FSMComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	class UNPCAStarMovementComponent* AStarMovementComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	class UNPCInteractionComponent* InteractionComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "NPC|AI")
	TObjectPtr<ASafetyZone> TargetSafetyZone;

	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	// class UWidgetComponent* InteractionUI;

};
