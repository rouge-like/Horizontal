// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponentBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionSignature, ACharacter*, InteractingCharacter);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORIZONTAL_API UInteractableComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:    
	UInteractableComponentBase();

	// 이 컴포넌트가 복제할 변수들을 엔진에 등록
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 플레이어가 상호작용을 시도할 때 호출되는 메인 진입 함수. 자식 클래스에서 재정의(override)하여 구체적인 행동을 정의. */
	virtual void InitiateInteraction(ACharacter* InteractingCharacter);

	/** 현재 상호작용이 가능한 상태인지 외부에서 확인하는 함수. */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsInteractable() const { return bIsInteractable; }
    
	/** 서버에서만 이 오브젝트의 상호작용 가능 상태를 변경하는 함수. */
	void SetInteractable(bool bNewState);

public:
	/** 플레이어 감지를 위한 콜리전 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction Components")
	TObjectPtr<class USphereComponent> SphereComp;

	/** 이 오브젝트와 상호작용 시 시작될 대사의 Label. */
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	FName DialogueStartLabel;
    
	/** 상호작용이 성공적으로 시작되었을 때 방송되는 델리게이트. */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractionSignature OnInteraction;

protected:
	/** 서버에서만 변경되고, 모든 클라이언트로 복제되는 '상호작용 가능 여부' 상태 변수. */
	UPROPERTY(Replicated)
	bool bIsInteractable = true;
};
