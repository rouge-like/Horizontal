#include "Khc/InteractableComponentBase.h"

#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"


UInteractableComponentBase::UInteractableComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
}

void UInteractableComponentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractableComponentBase, bIsInteractable);
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
