#include "Public/OSC/PlayerBase.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "Net/UnrealNetwork.h"
#include "CollisionShape.h"
#include "NiagaraComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Khc/Player/PlayerInteractionComponent.h"
#include "Math/UnrealMathUtility.h"
#include "OSC/InventoryComponent.h"
#include "OSC/UsableItemBase.h"

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

	InventoryComp = CreateDefaultSubobject<UInventoryComponent>("InventoryComp");
	InteractionComponent = CreateDefaultSubobject<UPlayerInteractionComponent>(TEXT("InteractionComponent"));
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

		if (PickupAction)
		{
			EnhancedInput->BindAction(PickupAction, ETriggerEvent::Started, this, &APlayerBase::HandlePickupStarted);
		}

		if (DropAction)
		{
			EnhancedInput->BindAction(DropAction, ETriggerEvent::Started, this, &APlayerBase::HandleDropStarted);
		}
	}

	if (InteractionComponent)
	{
		InteractionComponent->SetupPlayerInput(PlayerInputComponent);
	}

	OnSetUpPlayerInputDelegate.Broadcast(PlayerInputComponent);
}

void APlayerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerBase, bIsSprinting);
	DOREPLIFETIME(APlayerBase, HandsState);
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

void APlayerBase::SetPickupInternal(AUsableItemBase* Item)
{
	if (!IsValid(Item))
	{
		return;
	}

	Item->OnPickup(this);
}

void APlayerBase::SetDropInternal()
{
	InventoryComp->RemoveSelectedItem();
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
void APlayerBase::HandlePickupStarted()
{
	//
	if (InventoryComp->GetSelectedItem() != nullptr)
	{
		HandleDropStarted();
		return;
	}
	FVector ViewLocation;
	FRotator ViewRotation;
	GetActorEyesViewPoint(ViewLocation, ViewRotation);

	const FVector OverlapCenter = ViewLocation + ViewRotation.Vector() * (PickupRadius * 0.5f);
	const float QueryRadius = PickupRadius;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PlayerPickup), false, this);
	QueryParams.AddIgnoredActor(this);

	TArray<FOverlapResult> Overlaps;
	AUsableItemBase* ClosestItem = nullptr;
	float ClosestDistanceSq = MAX_FLT;

	DrawDebugSphere(GetWorld(), OverlapCenter, QueryRadius, 12, FColor::Purple);
	if (GetWorld()->OverlapMultiByObjectType(Overlaps, OverlapCenter, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(QueryRadius), QueryParams))
	{
		for (const FOverlapResult& Result : Overlaps)
		{
			AUsableItemBase* Candidate = Cast<AUsableItemBase>(Result.GetActor());
			if (!IsValid(Candidate) || !Candidate->CanBePickedUp())
			{
				continue;
			}

			const float DistanceSq = FVector::DistSquared(Candidate->GetActorLocation(), ViewLocation);
			if (DistanceSq < ClosestDistanceSq)
			{
				ClosestDistanceSq = DistanceSq;
				ClosestItem = Candidate;
			}
		}
	}

	if (!IsValid(ClosestItem))
	{
		return;
	}

	if (IsLocallyControlled())
	{
		SetPickupInternal(ClosestItem);
	}

	ServerSetPickup(ClosestItem);
}

void APlayerBase::HandleDropStarted()
{
	if (IsLocallyControlled())
	{
		SetDropInternal();
	}
	ServerSetDrop();
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

void APlayerBase::ServerSetPickup_Implementation(AUsableItemBase* Item)
{
	if (!IsValid(Item))
	{
		return;
	}

	const float DistanceSq = FVector::DistSquared(Item->GetActorLocation(), GetActorLocation());
	if (DistanceSq > FMath::Square(PickupRadius))
	{
		return;
	}

	if (!Item->CanBePickedUp())
	{
		return;
	}

	SetPickupInternal(Item);
}

void APlayerBase::ServerSetDrop_Implementation()
{
	SetDropInternal();
}

void APlayerBase::OnRep_IsSprinting()
{
	RefreshMovementSpeed();
}
