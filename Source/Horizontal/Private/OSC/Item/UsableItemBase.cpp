#include "OSC/Item/UsableItemBase.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Khc/InteractableComponentBase.h"
#include "Khc/Player/DialogueManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "OSC/InventoryComponent.h"
#include "OSC/PlayerBase.h"
#include "OSC/Sound/SoundManager.h"

AUsableItemBase::AUsableItemBase()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    InteractableComponent = CreateDefaultSubobject<UInteractableComponentBase>(TEXT("InteractableComponent"));
}

void AUsableItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AUsableItemBase, bCanBePickedUp);
}

void AUsableItemBase::OnPickup(APlayerBase* InOwner)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!IsValid(InOwner) || !bCanBePickedUp)
    {
        return;
    }

    UInventoryComponent* Inventory = InOwner->GetInventoryComponent();
    if (!Inventory || Inventory->ContainsItem(this))
    {
        return;
    }

    OwningPlayer = InOwner;
    SetOwner(InOwner);

    if (bAttachToOwnerOnPickup)
    {
        if (USceneComponent* OwnerRoot = InOwner->GetRootComponent())
        {
            AttachToComponent(OwnerRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        }
    }

    bIsEquipped = false;
    SetPickupAvailability(false);

    Inventory->AddItem(this);

    ASoundManager* SoundManager = Cast<ASoundManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ASoundManager::StaticClass()));
    if (IsValid(SoundManager))
        SoundManager->SpawnSoundAtLocation(FName(TEXT("Pickup")), OwningPlayer->GetActorLocation());
}

void AUsableItemBase::OnDrop()
{
    if (!HasAuthority())
    {
        return;
    }

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    SetOwner(nullptr);
    OwningPlayer = nullptr;
    bPendingEquipAttach = false;
    bIsEquipped = false;

    SetPickupAvailability(true);
}

void AUsableItemBase::OnEquip()
{
    bIsEquipped = true;
    HandlePickupAvailabilityChanged();
    bPendingEquipAttach = !TryAttachToOwnerMesh();

    if (!bPendingEquipAttach)
    {
        SetActorHiddenInGame(false);
    }
}

void AUsableItemBase::OnUnequip()
{
    StopUse();
    bIsEquipped = false;
    HandlePickupAvailabilityChanged();
}
void AUsableItemBase::SetPickupAvailability(bool bNewCanBePickedUp)
{
    if (bCanBePickedUp == bNewCanBePickedUp)
    {
        return;
    }

    bCanBePickedUp = bNewCanBePickedUp;
    HandlePickupAvailabilityChanged();
}

void AUsableItemBase::HandlePickupAvailabilityChanged()
{
    if (bHideWhenPickedUp)
    {
        const bool bShouldHide = !bCanBePickedUp && !bIsEquipped;
        SetActorHiddenInGame(bShouldHide);
    }
    else
    {
        SetActorHiddenInGame(false);
    }

    SetActorEnableCollision(bCanBePickedUp);
}

void AUsableItemBase::OnRep_CanBePickedUp()
{
    HandlePickupAvailabilityChanged();
}


void AUsableItemBase::OnRep_Owner()
{
    Super::OnRep_Owner();

    if (APlayerBase* LocalOwner = Cast<APlayerBase>(GetOwner()))
    {
        OwningPlayer = LocalOwner;

        if (bPendingEquipAttach && TryAttachToOwnerMesh())
        {
            bPendingEquipAttach = false;
            HandlePickupAvailabilityChanged();
        }
    }
    else
    {
        OwningPlayer = nullptr;
    }
}

void AUsableItemBase::StartUse()
{
    if (HasAuthority())
        HandleStartUse();
    else
        ServerStartUse();
}

void AUsableItemBase::StopUse()
{
    if (HasAuthority())
        HandleStopUse();
    else
        ServerStopUse();
}

bool AUsableItemBase::TryAttachToOwnerMesh()
{
    APlayerBase* LocalOwner = OwningPlayer.Get();
    if (!IsValid(LocalOwner))
    {
        LocalOwner = Cast<APlayerBase>(GetOwner());
        if (IsValid(LocalOwner))
        {
            OwningPlayer = LocalOwner;
        }
    }

    if (!IsValid(LocalOwner))
    {
        return false;
    }

    APlayerController* PC = Cast<APlayerController>(OwningPlayer->GetController());
    if (IsValid(PC))
    {
        UDialogueManagerComponent* DialogueManager = PC->FindComponentByClass<UDialogueManagerComponent>();
        if (IsValid(DialogueManager))
        {
            // 어떤 타입의 컴포넌트든 상관없이 DialogueStartLabel을 가져와 대화 시작
            DialogueManager->StartDialogue(InteractableComponent, InteractableComponent->DialogueStartLabel);
        }
    }
    
    if (USkeletalMeshComponent* PlayerMesh = LocalOwner->GetFirstPersonMesh())
    {
        if (!SocketName.IsEmpty())
        {
            AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName(*SocketName));
            SetActorRelativeRotation(EquipRotation);
            SetActorRelativeLocation(EquipLocation);
        }
        else
        {
            AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            SetActorRelativeRotation(EquipRotation);
            SetActorRelativeLocation(EquipLocation);
        }

        return true;
    }

    return false;
}

void AUsableItemBase::ServerStartUse_Implementation()
{
    HandleStartUse();
}
void AUsableItemBase::ServerStopUse_Implementation()
{
    HandleStopUse();
}
