// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInteractionComponent.generated.h"

class UNPCInteractionComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORIZONTAL_API UPlayerInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerInteractionComponent();

	void SetupPlayerInput(class UInputComponent* PlayerInputComponent);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;



public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Client, Reliable)
	void Client_SetInteractableTarget(UNPCInteractionComponent* NewTarget);

protected:
	// 'E' 키를 눌렀을 때 클라이언트에서 실행될 함수
	void OnInteractPressed();

	// [클라이언트가 호출] 서버에 상호작용을 실제로 요청하는 RPC
	UFUNCTION(Server, Reliable)
	void Server_RequestInteraction(UNPCInteractionComponent* TargetToInteractWith);

private:
	// 상호작용 가능한 최대 거리
	UPROPERTY(EditAnywhere, Category = "Interaction")
	float InteractionDistance = 300.0f;

	// 현재 상호작용 가능한 NPC 컴포넌트 (서버가 지정해 줌)
	UPROPERTY()
	TWeakObjectPtr<UNPCInteractionComponent> CurrentInteractable;

	// 현재 플레이어가 바라보고 있는 상호작용 대상 (매 틱마다 갱신)
	UPROPERTY()
	TWeakObjectPtr<UNPCInteractionComponent> FocusedInteractable;
    
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;
};
