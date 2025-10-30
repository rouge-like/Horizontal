// Fill out your copyright notice in the Description page of Project Settings.


#include "KNY/ASmokeFloorVolume.h"
#include "NiagaraComponent.h"
#include "Components/BoxComponent.h"

ASmokeFloorVolume::ASmokeFloorVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	Bounds = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
	RootComponent = Bounds;
	Bounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NS_CeilingSmoke"));
	NiagaraComp->SetupAttachment(RootComponent);
}

void ASmokeFloorVolume::OnConstruction(const FTransform& Xform)
{
	Super::OnConstruction(Xform);
	PushUserParams();
}

void ASmokeFloorVolume::BeginPlay()
{
	Super::BeginPlay();
	PushUserParams();
}

void ASmokeFloorVolume::PushUserParams()
{
	if (!NiagaraComp) return;

	const FVector Extent = Bounds->GetUnscaledBoxExtent();
	const FVector C = Bounds->GetComponentLocation();
	const FVector Min = C - Extent;
	const FVector Max = C + Extent;

	NiagaraComp->SetVariableVec3(TEXT("User.BoxMin"), Min);
	NiagaraComp->SetVariableVec3(TEXT("User.BoxMax"), Max);
	NiagaraComp->SetVariableFloat(TEXT("User.BaseSpawnRate"), BaseSpawnRate);
	NiagaraComp->SetVariableInt(TEXT("User.FloorIndex"), FloorIndex);
}
