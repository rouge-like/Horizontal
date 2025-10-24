// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/VFX/VFXActor.h"

#include "NiagaraComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"


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
	ParticleSystemComponent->bAutoActivate = false;

	bReplicates = true;
}

// Called when the game starts or when spawned
void AVFXActor::BeginPlay()
{
	Super::BeginPlay();

	if (NiagaraSystemAsset)
	{
		NiagaraComponent->SetAsset(NiagaraSystemAsset);
	}

	PushToNiagara();
	PushToMPC();
}

// Called every frame
void AVFXActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bAnimating)
	{
		Elapsed += DeltaTime;

		float Alpha = FMath::Clamp(Elapsed / FMath::Max(RadiusGrowSeconds, 0.001f), 0.f, 1.f);
		BurnRadius = FMath::Lerp(0.f, TargetMaxRadius, Alpha);

		PushToNiagara();
		PushToMPC();

		if (Alpha >= 1.f)
		{
			bAnimating = false;
		}
	}
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

void AVFXActor::StartBurnAt(const FVector& WorldLocation, float StartRadius, float MaxRadius)
{
	BurnCenter = WorldLocation;
	BurnRadius = StartRadius;
	TargetMaxRadius = MaxRadius;

	Elapsed = 0.f;
	bAnimating = true;

	PushToNiagara();
	PushToMPC();
	StartVFX();
}

void AVFXActor::SetBurnParams(const FVector& InCenter, float InRadius, float InEdgeWidth,
	float InIntensityG)
{
	BurnCenter = InCenter;
	BurnRadius = InRadius;
	EdgeWidth  = FMath::Max(0.001f, InEdgeWidth);
	BurnIntensity_G = InIntensityG;

	PushToNiagara();
	PushToMPC();
}

void AVFXActor::PushToNiagara() const
{
	if (!NiagaraComponent) return;
	
	NiagaraComponent->SetVariableVec3(TEXT("User.BurnCenter"),BurnCenter);
	NiagaraComponent->SetVariableFloat(TEXT("User.BurnRadius"),BurnRadius);
	NiagaraComponent->SetVariableFloat(TEXT("User.EdgeWidth"),EdgeWidth);
	NiagaraComponent->SetVariableFloat(TEXT("User.BurnIntensity_G"),BurnIntensity_G);
	
	
	
}

void AVFXActor::PushToMPC() const
{
	if (!MPC_Fire) return;

	if (UWorld* W = GetWorld())
	{
		if (UMaterialParameterCollectionInstance* Inst = W->GetParameterCollectionInstance(MPC_Fire))
		{
			Inst->SetVectorParameterValue(TEXT("BurnCenter"), BurnCenter);
			Inst->SetScalarParameterValue(TEXT("BurnRadius"), BurnRadius);
			Inst->SetScalarParameterValue(TEXT("EdgeWidth"),  EdgeWidth);
			Inst->SetScalarParameterValue(TEXT("BurnIntensity_G"), BurnIntensity_G);
		}
	}
}

