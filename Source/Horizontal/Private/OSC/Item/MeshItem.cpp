// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Item/MeshItem.h"


// Sets default values
AMeshItem::AMeshItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->SetIsReplicated(true);
	AActor::SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AMeshItem::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMeshItem::HandlePickupAvailabilityChanged()
{
	Super::HandlePickupAvailabilityChanged();

	const bool bShouldSimulatePhysics = HasAuthority() && bCanBePickedUp;
	MeshComp->SetSimulatePhysics(bShouldSimulatePhysics);

	if (HasAuthority() && !bShouldSimulatePhysics)
	{
		MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		MeshComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
}

// Called every frame
void AMeshItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

