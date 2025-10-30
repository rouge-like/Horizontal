// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableObjectBase.generated.h"

UCLASS()
class HORIZONTAL_API AInteractableObjectBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInteractableObjectBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	bool bHasBeenInteractedWith;

	void BindToPlayerController(APlayerController* PC);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnDialogueEventReceived(FName EventTag, AActor* InteractableActor);

	UPROPERTY(ReplicatedUsing = OnRep_IsMove)
	bool bIsMove;

	UPROPERTY(ReplicatedUsing = OnRep_ObstacleMove)
	bool bObstacleMove; // 장애물 이동용

	UPROPERTY(ReplicatedUsing = OnRep_ObstacleMove)
	bool bMainDoorMove; // 장애물 이동용

	UFUNCTION()
	void OnRep_IsMove();

	UFUNCTION()
	void OnRep_ObstacleMove();

	UFUNCTION()
	void OnRep_MainDoorMove();
	
	FVector ObstacleTargetLocation;
	FRotator ObstacleTargetRotation;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayEmergencyBGM(USoundBase* SoundToPlay);

public:
	virtual void Tick(float DeltaTime) override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UObjectInteractionComponent> InteractionComponent;

	float originRotYaw = 0.f;

	UPROPERTY(EditAnywhere)
	class AStaticMeshActor* MainDoor;

};
