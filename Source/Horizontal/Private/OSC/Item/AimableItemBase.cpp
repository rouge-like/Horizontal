#include "OSC/Item/AimableItemBase.h"

#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "OSC/PlayerBase.h"

AAimableItemBase::AAimableItemBase()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AAimableItemBase::BeginPlay()
{
    Super::BeginPlay();
}

void AAimableItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AAimableItemBase, bIsAiming);
}

void AAimableItemBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    APlayerBase* LocalOwner = OwningPlayer.Get();
    if (!IsValid(LocalOwner) || !LocalOwner->IsLocallyControlled())
    {
        return;
    }

    if (bStartLerpOnAim)
    {
        if (UCameraComponent* Camera = LocalOwner->GetFirstPersonCameraComponent())
        {
            if (LerpOnAimAlpha >= 1.0f)
            {
                LerpOnAimAlpha = 1.0f;
                bStartLerpOnAim = false;
                Camera->SetFieldOfView(AimingFOV);
            }
            else
            {
                const float TargetFOV = FMath::Lerp(OriginFOV, AimingFOV, LerpOnAimAlpha);
                Camera->SetFieldOfView(TargetFOV);
                LerpOnAimAlpha += DeltaSeconds * InterpSpeed;
            }
        }
    }

    if (bStartLerpOffAim)
    {
        if (UCameraComponent* Camera = LocalOwner->GetFirstPersonCameraComponent())
        {
            if (LerpOffAimAlpha >= 1.0f)
            {
                LerpOffAimAlpha = 1.0f;
                bStartLerpOffAim = false;
                Camera->SetFieldOfView(OriginFOV);
            }
            else
            {
                const float TargetFOV = FMath::Lerp(AimingFOV, OriginFOV, LerpOffAimAlpha);
                Camera->SetFieldOfView(TargetFOV);
                LerpOffAimAlpha += DeltaSeconds * InterpSpeed;
            }
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
        LerpOnAimAlpha = 0.0f;
    }

    if (HasAuthority())
    {
        HandleStartAim();
    }
    else
    {
        ServerStartAim();
    }
}

void AAimableItemBase::StopAim()
{
    APlayerBase* LocalOwner = OwningPlayer.Get();
    if (IsValid(LocalOwner) && LocalOwner->IsLocallyControlled())
    {
        bStartLerpOffAim = true;
        bStartLerpOnAim = false;
        LerpOffAimAlpha = 0.0f;
    }

    if (HasAuthority())
    {
        HandleStopAim();
    }
    else
    {
        ServerStopAim();
    }
}

void AAimableItemBase::OnRep_Owner()
{
    Super::OnRep_Owner();

    if (APlayerBase* LocalOwner = OwningPlayer.Get())
    {
        if (IsValid(LocalOwner) && LocalOwner->IsLocallyControlled())
        {
            if (UCameraComponent* Camera = LocalOwner->GetFirstPersonCameraComponent())
            {
                OriginFOV = Camera->FieldOfView;
            }
        }
    }
}

void AAimableItemBase::OnEquip()
{
    Super::OnEquip();

    if (APlayerBase* LocalOwner = OwningPlayer.Get())
    {
        if (IsValid(LocalOwner) && LocalOwner->IsLocallyControlled())
        {
            if (UCameraComponent* Camera = LocalOwner->GetFirstPersonCameraComponent())
            {
                OriginFOV = Camera->FieldOfView;
            }
        }
    }
}

void AAimableItemBase::OnUnequip()
{
    Super::OnUnequip();

    if (bIsAiming)
    {
        HandleStopAim();
    }
}

void AAimableItemBase::HandleStartAim()
{
    bIsAiming = true;

    if (APlayerBase* LocalOwner = OwningPlayer.Get())
    {
        LocalOwner->SetHandsState(EHandsState::Aiming);
    }
}

void AAimableItemBase::HandleStopAim()
{
    bIsAiming = false;

    if (APlayerBase* LocalOwner = OwningPlayer.Get())
    {
        LocalOwner->SetHandsState(EHandsState::None);
    }
}

void AAimableItemBase::OnRep_IsAiming(bool Previous)
{
    if (Previous == bIsAiming)
    {
        return;
    }

    APlayerBase* LocalOwner = OwningPlayer.Get();
    if (!IsValid(LocalOwner) || !LocalOwner->IsLocallyControlled())
    {
        return;
    }

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

void AAimableItemBase::StartUse()
{
    FVector StartLocation;
    FVector Direction;
    const bool bHasData = GatherUseData(StartLocation, Direction);

    if (HasAuthority())
    {
        SetPendingUseData(StartLocation, Direction, bHasData);
        Super::StartUse();
        return;
    }

    if (bHasData)
    {
        const FVector NormalizedDirection = Direction.GetSafeNormal();
        if (!NormalizedDirection.IsNearlyZero())
        {
            ServerStartUseWithAimData(FVector_NetQuantize(StartLocation), FVector_NetQuantizeNormal(NormalizedDirection), true);
            return;
        }
    }

    Super::StartUse();
}

bool AAimableItemBase::GatherUseData(FVector& OutStartLocation, FVector& OutDirection) const
{
    OutStartLocation = FVector::ZeroVector;
    OutDirection = FVector::ZeroVector;
    return false;
}

void AAimableItemBase::SetPendingUseData(const FVector& InStartLocation, const FVector& InDirection, bool bIsValid)
{
    const FVector NormalizedDirection = InDirection.GetSafeNormal();
    bHasPendingUseData = bIsValid && !NormalizedDirection.IsNearlyZero();
    PendingUseStartLocation = InStartLocation;
    PendingUseDirection = NormalizedDirection;
}

bool AAimableItemBase::ConsumeUseData(FVector& OutStartLocation, FVector& OutDirection)
{
    if (!bHasPendingUseData)
    {
        OutStartLocation = FVector::ZeroVector;
        OutDirection = FVector::ZeroVector;
        return false;
    }

    OutStartLocation = PendingUseStartLocation;
    OutDirection = PendingUseDirection;
    bHasPendingUseData = false;
    return true;
}

void AAimableItemBase::ServerStartUseWithAimData_Implementation(const FVector_NetQuantize& ClientStartLocation, const FVector_NetQuantizeNormal& ClientDirection, bool bClientProvidedData)
{
    const FVector StartLocation(ClientStartLocation);
    const FVector Direction(ClientDirection);
    const bool bValidData = bClientProvidedData && !Direction.IsNearlyZero();
    SetPendingUseData(StartLocation, Direction, bValidData);

    if (bValidData)
    {
        HandleStartUse();
    }
    else
    {
        Super::StartUse();
    }
}