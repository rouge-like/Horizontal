// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/VFX/VFXActor.h"

#include "NiagaraComponent.h"


// Sets default values
AVFXActor::AVFXActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	NiagaraComponent->SetupAttachment(SceneComponent);

	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
	ParticleSystemComponent->SetupAttachment(SceneComponent);

	bReplicates = true;
}

// Called when the game starts or when spawned
void AVFXActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVFXActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AVFXActor::StartVFX()
{
	if (NiagaraComponent)
	{
		NiagaraComponent->ResetSystem();
	}

	if (ParticleSystemComponent)
	{
		ParticleSystemComponent->ActivateSystem();
	}
}

void AVFXActor::StopVFX()
{
	if (NiagaraComponent)
	{
		NiagaraComponent->Deactivate();
	}

	if (ParticleSystemComponent)
	{
		ParticleSystemComponent->DeactivateSystem();
	}
}

