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
	void OnDialogueEventReceived(FName EventTag, AActor* InteractableActor);

public:
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void BindToPlayerController(APlayerController* PC);
	
	UPROPERTY(Replicated)
	bool bHasBeenInteractedWith;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	class UNPCFSMComponent* FSMComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	class UNPCAStarMovementComponent* AStarMovementComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Component")
	class UNPCInteractionComponent* InteractionComp;

	void SetReInteractable();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "NPC|AI")
	TObjectPtr<ASafetyZone> TargetSafetyZone;
};
