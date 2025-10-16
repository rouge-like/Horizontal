#include "Khc/Player/PlayerInteractionComponent.h"

#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Khc/InteractionObject/InteractableObjectBase.h"
#include "Khc/InteractionObject/ObjectInteractionComponent.h"
#include "Khc/NPC/NPCBase.h"
#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "Khc/Player/DialogueManagerComponent.h"


class UDialogueManagerComponent;
// Sets default values for this component's properties
UPlayerInteractionComponent::UPlayerInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UPlayerInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UPlayerInteractionComponent::SetupPlayerInput(class UInputComponent* PlayerInputComponent)
{
	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &UPlayerInteractionComponent::OnInteractPressed);
	}
}

void UPlayerInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled()) return;

	// 이전에 조준했던 대상의 UI를 먼저 끕니다.
	if (FocusedInteractable.IsValid())
	{
		// 이제 어떤 액터든 상관없이 WidgetComponent를 찾아서 끕니다.
		if (UWidgetComponent* InteractionUI = FocusedInteractable->GetOwner()->FindComponentByClass<UWidgetComponent>())
		{
			InteractionUI->SetVisibility(false);
		}
	}
	FocusedInteractable = nullptr;
    
	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC) return;

	// 라인트레이스
	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);
	FVector Start = CamLoc;
	FVector End = Start + (CamRot.Vector() * InteractionDistance);
    
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		UInteractableComponentBase* FoundComp = HitResult.GetActor()->FindComponentByClass<UInteractableComponentBase>();

		if (FoundComp && FoundComp->IsInteractable())
		{
			FocusedInteractable = FoundComp;
			// 해당 액터의 WidgetComponent를 찾아서 UI를 켭니다.
			if (UWidgetComponent* InteractionUI = FoundComp->GetOwner()->FindComponentByClass<UWidgetComponent>())
			{
				InteractionUI->SetVisibility(true);
			}
		}
	}
}

void UPlayerInteractionComponent::OnInteractPressed()
{
	if (FocusedInteractable.IsValid())
	{
		Server_RequestInteraction(FocusedInteractable.Get());
	}
}

void UPlayerInteractionComponent::Server_RequestInteraction_Implementation(
	UInteractableComponentBase* TargetToInteractWith)
{
	if (TargetToInteractWith && OwnerCharacter)
	{
		// 상호작용 시작 전, 서버에서 다시 한번 가능 여부 확인 후 즉시 잠금
		if (TargetToInteractWith->IsInteractable())
		{
			TargetToInteractWith->SetInteractable(false);
		}
		else
		{
			return; // 다른 사람이 먼저 상호작용 시작함
		}
       
		APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
		if (PC)
		{
			UDialogueManagerComponent* DialogueManager = PC->FindComponentByClass<UDialogueManagerComponent>();
			if (DialogueManager)
			{
				// 어떤 타입의 컴포넌트든 상관없이 DialogueStartLabel을 가져와 대화 시작
				DialogueManager->StartDialogue(TargetToInteractWith, TargetToInteractWith->DialogueStartLabel);
			}
		}
	}
}

