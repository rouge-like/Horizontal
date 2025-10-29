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

	// VFX 종료 시 MPC의 Extinguished를 1로 설정하여 emissive 숨김 (bUpdateMPC가 true일 때만)
	if (bUpdateMPC && MPC_Fire)
	{
		if (UWorld* W = GetWorld())
		{
			if (UMaterialParameterCollectionInstance* Inst = W->GetParameterCollectionInstance(MPC_Fire))
			{
				Inst->SetScalarParameterValue(TEXT("Extinguished"), 1.f);  // 불이 꺼진 상태
			}
		}
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
	// bUpdateMPC가 false면 MPC 업데이트 건너뛰기 (여러 VFX가 동시에 MPC를 덮어쓰는 것 방지)
	if (!bUpdateMPC || !MPC_Fire) return;

	UWorld* W = GetWorld();
	if (!W) return;

	if (UMaterialParameterCollectionInstance* Inst = W->GetParameterCollectionInstance(MPC_Fire))
	{
		// 중심 고정(최초 1회)
		if (bLockCenter && !bCenterInit)
		{
			LockedCenter = BurnCenter;
			bCenterInit  = true;
		}

		const FVector CenterToPush = (bLockCenter && bCenterInit) ? LockedCenter : BurnCenter;

		Inst->SetVectorParameterValue(TEXT("BurnCenter"),      CenterToPush);
		Inst->SetScalarParameterValue(TEXT("BurnRadius"),      BurnRadius);
		Inst->SetScalarParameterValue(TEXT("EdgeWidth"),       EdgeWidth);
		Inst->SetScalarParameterValue(TEXT("BurnIntensity_G"), BurnIntensity_G);
		Inst->SetScalarParameterValue(TEXT("Extinguished"),    0.f);  // 불이 붙은 상태
	}
}

