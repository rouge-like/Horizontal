#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Khc/NPC/NPCBase.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"
#include "Khc/Player/PlayerInteractionComponent.h"


UNPCInteractionComponent::UNPCInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollision"));
}

void UNPCInteractionComponent::InitiateInteraction(ACharacter* InteractingPlayer)
{
	if (GetOwner()->HasAuthority())
	{
		ANPCBase* MyOwner = Cast<ANPCBase>(GetOwner());
		if (MyOwner && MyOwner->FSMComp && MyOwner->FSMComp->GetState() == ENPCState::Wait)
		{
			// InformSituation 타입의 상호작용 처리
			if (InteractionType == EInteractionType::InformSituation)
			{
				// OnPlayerDetected 델리게이트를 호출하여 NPCBase가 상태를 변경하도록 신호를 보냄
				OnPlayerDetected.Broadcast(InteractingPlayer);
			}
			// TODO: 다른 InteractionType에 대한 처리 추가 (전략 패턴 사용)
		}
	}
}


void UNPCInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if(SphereComp)
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &UNPCInteractionComponent::HandleBeginOverlap);
		SphereComp->OnComponentEndOverlap.AddDynamic(this, &UNPCInteractionComponent::HandleEndOverlap);
	}
}

// Called every frame
void UNPCInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UNPCInteractionComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GetOwner()->HasAuthority())
	{
		ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
		if (PlayerChar)
		{
			// 겹친 플레이어에게서 PlayerInteractionComponent를 찾음
			UPlayerInteractionComponent* PlayerComp = PlayerChar->FindComponentByClass<UPlayerInteractionComponent>();
			if (PlayerComp)
			{
				// 해당 플레이어의 클라이언트로 RPC를 보내, 상호작용 대상을 '이 NPC'로 설정하라고 명령
				PlayerComp->Client_SetInteractableTarget(this);
				Cast<ANPCBase>(GetOwner())->InteractionUI->SetVisibility(true);
			}
		}
	}
}

void UNPCInteractionComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (GetOwner()->HasAuthority())
	{
		ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
		if (PlayerChar)
		{
			UPlayerInteractionComponent* PlayerComp = PlayerChar->FindComponentByClass<UPlayerInteractionComponent>();
			if (PlayerComp)
			{
				// 해당 플레이어의 클라이언트로 RPC를 보내, 상호작용 대상을 '없음'으로 설정하라고 명령
				PlayerComp->Client_SetInteractableTarget(nullptr);
				Cast<ANPCBase>(GetOwner())->InteractionUI->SetVisibility(false);
			}
		}
	}
}
