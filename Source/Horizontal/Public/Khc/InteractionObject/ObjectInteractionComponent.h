// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteraction, ACharacter*, InteractingCharacter);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORIZONTAL_API UObjectInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UObjectInteractionComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsInteractable() const { return bIsInteractable; }
    
	// 서버에서만 이 오브젝트의 상호작용 가능 상태를 변경하는 함수
	void SetInteractable(bool bNewState);

protected:
	// UI 가시성 상태가 복제될 때 모든 클라이언트에서 호출될 함수
	UFUNCTION()
	void OnRep_IsInteractable();

public:
	// 플레이어가 상호작용을 시도할 때, 서버의 PlayerInteractionComponent가 호출할 함수
	void InitiateInteraction(ACharacter* InteractingCharacter);

	// 플레이어 감지를 위한 콜리전
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	TObjectPtr<class USphereComponent> SphereComp;

	// 이 오브젝트와 상호작용 시 시작될 대사의 Label
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	FName DialogueStartLabel;

private:
	// 서버에서만 변경되고, 모든 클라이언트로 복제되는 '상호작용 가능 여부' 상태 변수
	UPROPERTY(ReplicatedUsing = OnRep_IsInteractable)
	bool bIsInteractable = true;
};