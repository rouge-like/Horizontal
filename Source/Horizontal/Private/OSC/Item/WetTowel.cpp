// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Item/WetTowel.h"

#include "Components/BoxComponent.h"
#include "OSC/PlayerBase.h"
#include "OSC/PlayerBaseState.h"


// Sets default values
AWetTowel::AWetTowel()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	SetRootComponent(BoxComp);
	BoxComp->SetSimulatePhysics(true);
	BoxComp->SetIsReplicated(true);
	BoxComp->SetCollisionProfileName(TEXT("PhysicsActor"));
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(FName("MeshComp"));
	MeshComp->SetupAttachment(BoxComp);
	MeshComp->SetIsReplicated(true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetSimulatePhysics(false);
	
	AActor::SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AWetTowel::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWetTowel::HandlePickupAvailabilityChanged()
{
	Super::HandlePickupAvailabilityChanged();

	// 서버쪽에서만 물리 계산 실행
	const bool bSimulatePhysics = HasAuthority() && bCanBePickedUp;
	BoxComp->SetSimulatePhysics(bSimulatePhysics);

	// 물리 수치 초기화
	if (bSimulatePhysics)
	{
		BoxComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		BoxComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
}


void AWetTowel::HandleStartUse()
{
	Super::HandleStartUse();

	// Player 입 가리기 상태
	APlayerBase* LocalOwner = OwningPlayer.Get();
	
	if (IsValid(LocalOwner))
	{
		LocalOwner->SetHandsState(EHandsState::CoveringMouth);
	}
}

void AWetTowel::HandleStopUse()
{
	Super::HandleStopUse();
	
	// Player 입 가리기 상태 해제
	APlayerBase* LocalOwner = OwningPlayer.Get();
	
	if (IsValid(LocalOwner))
	{
		APlayerBaseState* PBS = LocalOwner->GetPlayerState<APlayerBaseState>();
		if (PBS->bIsInteracting) return;
		LocalOwner->SetHandsState(EHandsState::None);
	}
}

// Called every frame
void AWetTowel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

