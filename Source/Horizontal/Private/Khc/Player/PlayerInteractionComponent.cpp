#include "Khc/Player/PlayerInteractionComponent.h"

#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
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


// void UPlayerInteractionComponent::Client_SetInteractableTarget_Implementation(UNPCInteractionComponent* NewTarget)
// {
// 	CurrentInteractable = NewTarget;
//
// 	// 상호작용 대상이 생기면 Tick을 켜서 시야 추적 시작, 없으면 Tick을 꺼서 성능 절약
// 	SetComponentTickEnabled(NewTarget != nullptr);
// 	if (NewTarget == nullptr)
// 	{
// 		FocusedInteractable = nullptr; // 범위 밖으로 나가면 조준 대상도 해제
// 	}
// }

// Called every frame
void UPlayerInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled()) return; // 로컬 플레이어가 아니면 실행 안함

	if (FocusedInteractable.IsValid())
	{
		if (ANPCBase* OldNPC = Cast<ANPCBase>(FocusedInteractable->GetOwner()))
		{
			if (OldNPC->InteractionUI) OldNPC->InteractionUI->SetVisibility(false);
		}
	}
	
	// 카메라 시점에서 라인 트레이스 발사
	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC) return;

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
		// 부딪힌 액터에서 NPCInteractionComponent를 찾습니다.
		UNPCInteractionComponent* FoundComp = HitResult.GetActor()->FindComponentByClass<UNPCInteractionComponent>();
		if (FoundComp && FoundComp->IsInteractable())
		{
			// 찾았다면, 조준 대상으로 설정하고 해당 NPC의 UI를 켭니다.
			FocusedInteractable = FoundComp;
			if (ANPCBase* NewNPC = Cast<ANPCBase>(FocusedInteractable->GetOwner()))
			{
				if (NewNPC->InteractionUI) NewNPC->InteractionUI->SetVisibility(true);
			}
			return;
		}
	}
	
	// 아무것도 맞지 않았거나, 다른 것을 보고 있다면 조준 해제
	FocusedInteractable = nullptr;
}

void UPlayerInteractionComponent::OnInteractPressed()
{
	if (FocusedInteractable.IsValid())
	{
		Server_RequestInteraction(FocusedInteractable.Get());
	}
}

void UPlayerInteractionComponent::Server_RequestInteraction_Implementation(
	UNPCInteractionComponent* TargetToInteractWith)
{
	if (TargetToInteractWith && OwnerCharacter )
	{
		if (TargetToInteractWith->IsInteractable())
		{
			TargetToInteractWith->SetInteractable(false);
		}
		else
		{
			return; // 이미 다른 사람이 상호작용 중이면 아무것도 하지 않음
		}
		
		APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
		if (PC)
		{
			// 자신의 컨트롤러에 있는 DialogueManager를 찾아 대화 시작을 요청
			UDialogueManagerComponent* DialogueManager = PC->FindComponentByClass<UDialogueManagerComponent>();
			if (DialogueManager)
			{
				//FName StartLabel = TargetToInteractWith->DialogueStartLabel;
				FName StartLabel = "NPC01_1"; // 임시로 첫 대사 라벨 하드코딩
				DialogueManager->StartDialogue(TargetToInteractWith, StartLabel);
			}
		}
	}
}

