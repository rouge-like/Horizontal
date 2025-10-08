// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Item/WetTowel.h"
#include "OSC/PlayerBase.h"


// Sets default values
AWetTowel::AWetTowel()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(FName("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->SetIsReplicated(true);
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
	MeshComp->SetSimulatePhysics(bSimulatePhysics);

	// 물리 수치 초기화
	if (bSimulatePhysics)
	{
		MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		MeshComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
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
		LocalOwner->SetHandsState(EHandsState::None);
	}
}

// Called every frame
void AWetTowel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

