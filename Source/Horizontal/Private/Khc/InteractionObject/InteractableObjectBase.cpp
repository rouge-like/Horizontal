// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/InteractionObject/InteractableObjectBase.h"

#include "CborTypes.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Khc/InteractionObject/ObjectInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Khc/Player/DialogueManagerComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AInteractableObjectBase::AInteractableObjectBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	InteractionComponent = CreateDefaultSubobject<UObjectInteractionComponent>(TEXT("InteractionComponent"));
	InteractionComponent->SphereComp->SetSphereRadius(100.f);
	RootComponent = InteractionComponent->SphereComp;
	InteractionComponent->InteractionUI->SetupAttachment(InteractionComponent->SphereComp);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	// 기본적으로 플레이어의 라인 트레이스(Visibility 채널)에 감지되도록 콜리전 설정
	MeshComponent->SetCollisionObjectType(ECC_WorldStatic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

	// 2. 상호작용 컴포넌트 생성 (논리적인 컴포넌트라 Attachment 불필요)

	// 3. UI 위젯 컴포넌트 생성 및 설정
	// InteractionUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionUI"));
	// InteractionUI->SetupAttachment(RootComponent); // 메시 컴포넌트에 부착
	// InteractionUI->SetWidgetSpace(EWidgetSpace::Screen); // 항상 화면을 바라보도록 설정
	// InteractionUI->SetVisibility(false); // 기본적으로 숨겨둠
}

void AInteractableObjectBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInteractableObjectBase, bHasBeenInteractedWith);
}

void AInteractableObjectBase::BindToPlayerController(APlayerController* PC)
{
	if (PC)
	{
		UDialogueManagerComponent* DialogueManager = PC->FindComponentByClass<UDialogueManagerComponent>();
		if (DialogueManager)
		{
			// 이 오브젝트의 OnDialogueEventReceived 함수를 해당 DialogueManager의 델리게이트에 연결합니다.
			DialogueManager->OnDialogueEvent.AddDynamic(this, &AInteractableObjectBase::OnDialogueEventReceived);
		}
	}
}

void AInteractableObjectBase::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractableObjectBase::OnDialogueEventReceived(FName EventTag, AActor* InteractableActor)
{
	if (InteractableActor != this)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("'%s' received EventTag: %s"), *GetName(), *EventTag.ToString());

	// 1. Obstruction 일때 EndGood -> 파괴
	if (EventTag == "DestroyObstacle")
	{
		Destroy();
	}
	// 2. Obstruction 일때 EndBad -> 다시 상호작용 가능
	else if (EventTag == "ResetInteraction")
	{
		if (InteractionComponent)
		{
			InteractionComponent->SetInteractable(true);
		}
	}
	// 3 & 4. Trigger 일때 EndGood/EndBad -> 상호작용 불가능 상태 유지
	else if (EventTag == "DeactivateTrigger")
	{
		// PlayerInteractionComponent에서 SetInteractable(false)로 이미 잠갔으므로,
		// 여기서는 특별한 행동이 필요 없습니다. 만약 문을 열거나 하는 추가 행동이 필요하다면 여기에 구현합니다.
		UE_LOG(LogTemp, Log, TEXT("Trigger '%s' has been deactivated."), *GetName());
	}
	// 5. Information 타입은 대화 종료 후 즉시 다시 상호작용 가능
	else if (EventTag == "InfoReset")
	{
		if (InteractionComponent)
		{
			InteractionComponent->SetInteractable(true);
		}
	}
	else if (EventTag == "OpenDoor")
	{
		bIsMove = true; 
		OnRep_IsMove();
	}
}

void AInteractableObjectBase::OnRep_IsMove()
{
	if (bIsMove)
	{
		// 클라이언트에서도 Tick 로직을 실행할 수 있도록 활성화합니다.
		PrimaryActorTick.bCanEverTick = true;
	}
}

void AInteractableObjectBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsMove)
	{
		const float TargetYaw = -140.0f;
		const float RotationSpeed = 100.0f; 
		float CurrentYaw = GetActorRotation().Yaw;
		float NewYaw = FMath::FInterpConstantTo(CurrentYaw, TargetYaw, DeltaTime, RotationSpeed);

		SetActorRotation(FRotator(0.0f, NewYaw, 0.0f));

		if (FMath::IsNearlyEqual(NewYaw, TargetYaw))
		{
			bIsMove = false; // 이동 완료
		}
	}
}
