#include "Khc/InteractableComponentBase.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"


UInteractableComponentBase::UInteractableComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));

	InteractionUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionUI"));
	InteractionUI->SetWidgetSpace(EWidgetSpace::Screen); // 항상 화면을 바라보도록 설정
	InteractionUI->SetVisibility(false);
}

void UInteractableComponentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractableComponentBase, bIsInteractable);
	DOREPLIFETIME_CONDITION(UInteractableComponentBase, bIsInteractable, COND_None);
}

void UInteractableComponentBase::BeginPlay()
{
	Super::BeginPlay();

	if (InteractionUI && InteractionWidgetClass)
	{
		InteractionUI->SetWidgetClass(InteractionWidgetClass);
		InteractionUI->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void UInteractableComponentBase::InitiateInteraction(ACharacter* InteractingCharacter)
{
	if (GetOwner()->HasAuthority() && bIsInteractable)
	{
		OnInteraction.Broadcast(InteractingCharacter);
	}
}

void UInteractableComponentBase::SetInteractable(bool bNewState)
{
	if (GetOwner()->HasAuthority())
	{
		bIsInteractable = bNewState;
	}
}

void UInteractableComponentBase::OnRep_IsInteractable()
{
	if (InteractionUI)
	{
		InteractionUI->SetVisibility(bIsInteractable);
	}
}
