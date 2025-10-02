#include "Public/OSC/PlayerBase.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "Net/UnrealNetwork.h"

APlayerBase::APlayerBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->NavAgentProps.bCanCrouch = true;
		Movement->NavAgentProps.bCanJump = false;
		Movement->MaxWalkSpeed = WalkSpeed;
		Movement->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}

void APlayerBase::BeginPlay()
{
	Super::BeginPlay();

	RefreshMovementSpeed();
}

void APlayerBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void APlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (CrouchAction)
		{
			EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &APlayerBase::HandleCrouchPressed);
			EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Completed, this, &APlayerBase::HandleCrouchReleased);
		}

		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &APlayerBase::HandleSprintPressed);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerBase::HandleSprintReleased);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Canceled, this, &APlayerBase::HandleSprintReleased);
		}
	}
}

void APlayerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerBase, bIsSprinting);
}

void APlayerBase::RefreshMovementSpeed()
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeedCrouched = CrouchSpeed;

		float TargetSpeed = WalkSpeed;
		if (bIsSprinting && !Movement->IsCrouching())
		{
			TargetSpeed = SprintSpeed;
		}

		Movement->MaxWalkSpeed = TargetSpeed;
	}
}

void APlayerBase::SetSprintingInternal(bool bNewSprinting)
{
	if (bIsSprinting == bNewSprinting)
	{
		return;
	}

	bIsSprinting = bNewSprinting;
	RefreshMovementSpeed();
}

void APlayerBase::HandleCrouchPressed()
{
	bWantsToCrouch = true;
	bWantsToSprint = false;

	if (IsLocallyControlled())
	{
		SetSprintingInternal(false);
	}

	ServerSetSprinting(false);
	Crouch();
	RefreshMovementSpeed();
}

void APlayerBase::HandleCrouchReleased()
{
	bWantsToCrouch = false;
	UnCrouch();
	RefreshMovementSpeed();
}

void APlayerBase::HandleSprintPressed()
{
	bWantsToSprint = true;

	if (IsCrouched())
	{
		UnCrouch();
	}

	if (IsLocallyControlled())
	{
		SetSprintingInternal(true);
	}

	ServerSetSprinting(true);
}

void APlayerBase::HandleSprintReleased()
{
	bWantsToSprint = false;

	if (IsLocallyControlled())
	{
		SetSprintingInternal(false);
	}

	ServerSetSprinting(false);
}

void APlayerBase::ServerSetSprinting_Implementation(bool bNewSprinting)
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		if (bNewSprinting && Movement->IsCrouching())
		{
			bNewSprinting = false;
		}
	}

	SetSprintingInternal(bNewSprinting);
}

void APlayerBase::OnRep_IsSprinting()
{
	RefreshMovementSpeed();
}
