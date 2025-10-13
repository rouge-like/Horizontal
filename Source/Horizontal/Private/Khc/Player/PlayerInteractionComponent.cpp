#include "Khc/Player/PlayerInteractionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Khc/NPC/Component/NPCInteractionComponent.h"


// Sets default values for this component's properties
UPlayerInteractionComponent::UPlayerInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(false);
}


// Called when the game starts
void UPlayerInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UPlayerInteractionComponent::SetupPlayerInput(class UInputComponent* PlayerInputComponent)
{
	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &UPlayerInteractionComponent::OnInteractPressed);
		OwnerCharacter = Cast<ACharacter>(GetOwner());
	}
}


void UPlayerInteractionComponent::Client_SetInteractableTarget_Implementation(UNPCInteractionComponent* NewTarget)
{
	CurrentInteractable = NewTarget;

	// 상호작용 대상이 생기면 Tick을 켜서 시야 추적 시작, 없으면 Tick을 꺼서 성능 절약
	SetComponentTickEnabled(NewTarget != nullptr);
	if (NewTarget == nullptr)
	{
		FocusedInteractable = nullptr; // 범위 밖으로 나가면 조준 대상도 해제
	}
}

// Called every frame
void UPlayerInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled()) return; // 로컬 플레이어가 아니면 실행 안함

	// 카메라 시점에서 라인 트레이스 발사
	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC) return;

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	FVector Start = CamLoc;
	FVector End = Start + (CamRot.Vector() * InteractionDistance);
    
	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
	{
		// 부딪힌 액터가 현재 상호작용 가능한 대상(CurrentInteractable)과 일치하는지 확인
		if (CurrentInteractable.IsValid() && HitResult.GetActor() == CurrentInteractable->GetOwner())
		{
			FocusedInteractable = CurrentInteractable;
			// TODO: 여기에 "E키를 누르세요" 같은 UI를 띄우는 로직 추가
			return;
		}
	}

	// 아무것도 맞지 않았거나, 다른 것을 보고 있다면 조준 해제
	FocusedInteractable = nullptr;
	// TODO: UI를 숨기는 로직 추가
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
	if (TargetToInteractWith)
	{
		TargetToInteractWith->InitiateInteraction(OwnerCharacter);
	}
}

