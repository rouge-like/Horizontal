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

	UFUNCTION()
	void OnPlayerDetected(AActor* DetectedPlayer);

	UFUNCTION(Server, Reliable)
	void Server_RequestInteraction();
	
public:
	virtual void Tick(float DeltaTime) override;

	//void BillboardUI();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	class UNPCFSMComponent* FSMComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	class UNPCAStarMovementComponent* AStarMovementComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	class UNPCInteractionComponent* InteractionComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|AI")
	TObjectPtr<ASafetyZone> TargetSafetyZone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	class UWidgetComponent* InteractionUI;

};
