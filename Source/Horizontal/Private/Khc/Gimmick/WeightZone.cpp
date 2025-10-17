// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/Gimmick/WeightZone.h"
#include "Components/BoxComponent.h"
#include "Math/Box.h"


// Sets default values
AWeightZone::AWeightZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeightArea = CreateDefaultSubobject<UBoxComponent>(TEXT("WeightArea"));
	RootComponent = WeightArea;
	WeightArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AWeightZone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeightZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FBox AWeightZone::GetWeightBox() const
{
	if (WeightArea)
	{
		return WeightArea->Bounds.GetBox();
	}
	return FBox(ForceInit);
}

