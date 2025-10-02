// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "Khc/NPC/NPCBase.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"


class ANPCBase;
// Sets default values for this component's properties
UNPCInteractionComponent::UNPCInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollision"));
	// ...
}


// Called when the game starts
void UNPCInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
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

	// ...
}

void UNPCInteractionComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetOwner() && OtherActor->IsA<ACharacter>())
	{
		// NPC의 FSM 컴포넌트를 가져옵니다.
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

