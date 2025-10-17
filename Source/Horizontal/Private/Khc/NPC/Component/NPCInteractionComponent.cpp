#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Khc/NPC/NPCBase.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"


UNPCInteractionComponent::UNPCInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollision"));
}


void UNPCInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if(SphereComp)
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &UNPCInteractionComponent::HandleBeginOverlap);
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
	if (OtherActor && OtherActor != GetOwner() && OtherActor->IsA<ACharacter>())
	{
		ANPCBase* MyOwner = Cast<ANPCBase>(GetOwner());
		if (MyOwner && MyOwner->FSMComp)
		{
			if (MyOwner->FSMComp->GetState() == ENPCState::Wait)
			{
				OnPlayerDetected.Broadcast(OtherActor);
			}
		}
	}
}

