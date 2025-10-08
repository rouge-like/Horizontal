// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Item/AimableItemBase.h"

#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "OSC/PlayerBase.h"


// Sets default values
AAimableItemBase::AAimableItemBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAimableItemBase::BeginPlay()
{
	Super::BeginPlay();
}

void AAimableItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAimableItemBase, bIsAiming);
}

// Called every frame
void AAimableItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlayerBase* LocalOwner = OwningPlayer.Get();
	if (!IsValid(LocalOwner)) return;
	if (!LocalOwner->IsLocallyControlled()) return;
	
	// 에임 Lerp 처리
	if (bStartLerpOnAim)
	{
		if (LerpOnAimAlpha >= 1.0f)
		{
			LerpOnAimAlpha = 1;
			bStartLerpOnAim = false;
			LocalOwner->GetFirstPersonCameraComponent()->FieldOfView = AimingFOV;
		}
		else
		{
			float Target = FMath::Lerp(OriginFOV, AimingFOV, LerpOnAimAlpha);
			LocalOwner->GetFirstPersonCameraComponent()->FieldOfView = Target;
			LerpOnAimAlpha += DeltaTime * InterpSpeed;
		}
	}
	// 에임 해제 Lerp 처리
	if (bStartLerpOffAim)
	{
		if (LerpOffAimAlpha >= 1.0f)
		{
			LerpOffAimAlpha = 1;
			bStartLerpOffAim = false;
			LocalOwner->GetFirstPersonCameraComponent()->FieldOfView = OriginFOV;
		}
		else
		{
			float Target = FMath::Lerp(AimingFOV, OriginFOV, LerpOffAimAlpha);
			LocalOwner->GetFirstPersonCameraComponent()->FieldOfView = Target;
			LerpOffAimAlpha += DeltaTime * InterpSpeed;
		}
	}
}

void AAimableItemBase::StartAim()
{
	APlayerBase* LocalOwner = OwningPlayer.Get();
	if (IsValid(LocalOwner) && LocalOwner->IsLocallyControlled())
	{
		bStartLerpOnAim = true;
		bStartLerpOffAim = false;
		LerpOnAimAlpha = 0;
	}
	if (HasAuthority())
		HandleStartAim();
	else
		ServerStartAim();
}

void AAimableItemBase::StopAim()
{
	APlayerBase* LocalOwner = OwningPlayer.Get();
	if (IsValid(LocalOwner) && LocalOwner->IsLocallyControlled())
	{
		bStartLerpOffAim = true;
		bStartLerpOnAim = false;
		LerpOffAimAlpha = 0;
	}
	if (HasAuthority())
		HandleStopAim();
	else
		ServerStopAim();
}

void AAimableItemBase::OnRep_Owner()
{
	Super::OnRep_Owner();
	
	APlayerBase* LocalOwner = OwningPlayer.Get();
	if (IsValid(LocalOwner) && LocalOwner->IsLocallyControlled())
	{
		UCameraComponent* Camera = LocalOwner->GetFirstPersonCameraComponent();
		if (IsValid(Camera))
			OriginFOV = Camera->FieldOfView;
	}
}

void AAimableItemBase::OnEquip()
{
	Super::OnEquip();

	APlayerBase* LocalOwner = OwningPlayer.Get();
	if (IsValid(LocalOwner) && LocalOwner->IsLocallyControlled())
	{
		UCameraComponent* Camera = LocalOwner->GetFirstPersonCameraComponent();
		if (IsValid(Camera))
			OriginFOV = Camera->FieldOfView;
	}
}

void AAimableItemBase::HandleStartAim()
{
	bIsAiming = true;

	APlayerBase* LocalOwner = OwningPlayer.Get();

	if (IsValid(LocalOwner))
		LocalOwner->SetHandsState(EHandsState::Aiming);
}

void AAimableItemBase::HandleStopAim()
{
	bIsAiming = false;

	APlayerBase* LocalOwner = OwningPlayer.Get();

	if (IsValid(LocalOwner))
		LocalOwner->SetHandsState(EHandsState::None);
}

void AAimableItemBase::OnRep_IsAiming(bool Previous)
{
	if (Previous == bIsAiming)
		return;
	
	APlayerBase* LocalOwner = OwningPlayer.Get();
	
	if (!IsValid(LocalOwner)) return;
	if (!LocalOwner->IsLocallyControlled()) return;

	if (bIsAiming)
	{
		LocalOwner->SetHandsState(EHandsState::Aiming);
	}
	else
	{
		LocalOwner->SetHandsState(EHandsState::None);
	}
}

void AAimableItemBase::ServerStartAim_Implementation()
{
	HandleStartAim();
}

void AAimableItemBase::ServerStopAim_Implementation()
{
	HandleStopAim();
}


