#include "OSC/InventoryComponent.h"

#include "EnhancedInputComponent.h"
#include "Net/UnrealNetwork.h"
#include "OSC/PlayerBase.h"
#include "OSC/UsableItemBase.h"
#include "OSC/Item/AimableItemBase.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
    SetIsReplicated(true);
}

void UInventoryComponent::InitializeComponent()
{
    Super::InitializeComponent();

    OwningPlayer = Cast<APlayerBase>(GetOwner());
    OwningPlayer->OnSetUpPlayerInputDelegate.AddDynamic(this, &UInventoryComponent::OnSetUpPlayerInput);

}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UInventoryComponent, SelectedItem);
}

void UInventoryComponent::OnSetUpPlayerInput(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (UseItemAction)
        {
            EnhancedInput->BindAction(UseItemAction, ETriggerEvent::Started, this, &UInventoryComponent::StartUseItem);
            EnhancedInput->BindAction(UseItemAction, ETriggerEvent::Completed, this, &UInventoryComponent::StopUseItem);
        }

        if (AimAction)
        {
            EnhancedInput->BindAction(AimAction, ETriggerEvent::Started, this, &UInventoryComponent::StartAim);
            EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, & UInventoryComponent::StopAim);
        }
    }
}

void UInventoryComponent::OnRep_SelectedItem(AUsableItemBase* PreviousItem)
{
    if (IsValid(PreviousItem))
    {
        PreviousItem->OnUnequip();
    }

    if (IsValid(SelectedItem))
    {
        SelectedItem->OnEquip();
    }
}

void UInventoryComponent::ApplySelectionInternal(int32 Idx)
{
    if (!Inventory.IsValidIndex(Idx))
    {
        return;
    }

    AUsableItemBase* NewItem = Inventory[Idx].Get();
    if (!IsValid(NewItem))
    {
        Inventory.RemoveAt(Idx);
        return;
    }

    if (SelectedItem == NewItem)
    {
        return;
    }

    AUsableItemBase* PreviousItem = SelectedItem;
    SelectedItem = NewItem;

    const bool bHasAuthority = (GetOwner() && GetOwner()->HasAuthority());
    if (bHasAuthority)
    {
        OnRep_SelectedItem(PreviousItem);
    }
}

void UInventoryComponent::ServerSelectItem_Implementation(int32 Idx)
{
    ApplySelectionInternal(Idx);
}

void UInventoryComponent::AddItem(AUsableItemBase* NewItem)
{
    if (!IsValid(NewItem))
    {
        return;
    }

    if (ContainsItem(NewItem))
    {
        return;
    }

    const int32 NewIndex = Inventory.Add(NewItem);

    const bool bHasAuthority = (GetOwner() && GetOwner()->HasAuthority());
    if (bHasAuthority && !IsValid(SelectedItem))
    {
        ApplySelectionInternal(NewIndex);
    }
}

void UInventoryComponent::RemoveItem(int32 ItemIndex)
{
    RemoveItemInternal(ItemIndex, true);
}

bool UInventoryComponent::RemoveSelectedItem()
{
    if (!IsValid(SelectedItem))
    {
        return false;
    }

    for (int32 Idx = 0; Idx < Inventory.Num(); ++Idx)
    {
        if (Inventory[Idx].Get() == SelectedItem)
        {
            RemoveItemInternal(Idx, true);
            SelectedItem = nullptr;
            return true;
        }
    }

    return false;
}

bool UInventoryComponent::ContainsItem(AUsableItemBase* Item) const
{
    if (!IsValid(Item))
    {
        return false;
    }

    for (const TWeakObjectPtr<AUsableItemBase>& Entry : Inventory)
    {
        if (Entry.Get() == Item)
        {
            return true;
        }
    }

    return false;
}

void UInventoryComponent::RemoveItemInternal(int32 ItemIndex, bool bCallOnDrop)
{
    if (!Inventory.IsValidIndex(ItemIndex))
    {
        return;
    }

    AUsableItemBase* Item = Inventory[ItemIndex].Get();
    Inventory.RemoveAt(ItemIndex);

    const bool bWasSelected = (SelectedItem == Item);
    if (bWasSelected)
    {
        AUsableItemBase* PreviousItem = SelectedItem;
        SelectedItem = nullptr;

        const bool bHasAuthority = (GetOwner() && GetOwner()->HasAuthority());
        if (bHasAuthority)
        {
            OnRep_SelectedItem(PreviousItem);
        }
    }

    if (bCallOnDrop && IsValid(Item))
    {
        Item->OnDrop();
    }
}

void UInventoryComponent::StartUseItem()
{
    if (AUsableItemBase* Item = SelectedItem.Get())
    {
        Item->StartUse();
    }
}

void UInventoryComponent::StopUseItem()
{
    if (AUsableItemBase* Item = SelectedItem.Get())
    {
        Item->StopUse();
    }
}

void UInventoryComponent::SelectItem(int32 Idx)
{
    const bool bHasAuthority = (GetOwner() && GetOwner()->HasAuthority());

    if (!bHasAuthority)
    {
        ServerSelectItem(Idx);
        return;
    }

    ApplySelectionInternal(Idx);
}

void UInventoryComponent::StartAim()
{
    if (AAimableItemBase* Item = Cast<AAimableItemBase>(SelectedItem.Get()))
    {
        Item->StartAim();
    }
}

void UInventoryComponent::StopAim()
{
    if (AAimableItemBase* Item = Cast<AAimableItemBase>(SelectedItem.Get()))
    {
        Item->StopAim();
    }
}
