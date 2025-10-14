// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/InteractionObject/ObjectInteractionComponent.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UObjectInteractionComponent::UObjectInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // Tick은 필요 없음

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	// SphereComp는 루트 컴포넌트에 직접 붙이지 않습니다. 액터의 블루프린트에서 위치를 자유롭게 조절할 수 있도록 합니다.
	// 이 컴포넌트와 그 안의 변수들이 멀티플레이에서 복제되도록 설정
	SetIsReplicatedByDefault(true);
}

void UObjectInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UObjectInteractionComponent, bIsInteractable);

	
}

void UObjectInteractionComponent::InitiateInteraction(ACharacter* InteractingCharacter)
{

}

void UObjectInteractionComponent::SetInteractable(bool bNewState)
{
	if (GetOwner()->HasAuthority())
	{
		bIsInteractable = bNewState;
        
		// 서버에서는 OnRep 함수가 자동 호출되지 않으므로, 수동으로 호출하여 서버 화면에도 UI가 보이게/안보이게 함
		OnRep_IsInteractable();
	}
}

void UObjectInteractionComponent::OnRep_IsInteractable()
{

}
