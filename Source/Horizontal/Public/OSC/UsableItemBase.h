#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UsableItemBase.generated.h"

class APlayerBase;
class UInventoryComponent;
class FLifetimeProperty;

UCLASS(Abstract)
class HORIZONTAL_API AUsableItemBase : public AActor
{
    GENERATED_BODY()

public:
    AUsableItemBase();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    FORCEINLINE FName GetItemName() const { return ItemName; }
    FORCEINLINE APlayerBase* GetOwningPlayer() const { return OwningPlayer.Get(); }

    virtual void OnPickup(APlayerBase* InOwner);
    virtual void OnDrop();

    virtual void OnEquip();
    virtual void OnUnequip();

    virtual void OnRep_Owner() override;

    void StartUse();
    void StopUse();

    UFUNCTION(BlueprintCallable, Category = "Item")
    bool CanBePickedUp() const { return bCanBePickedUp; }

protected:
    virtual bool TryAttachToOwnerMesh();

    void SetPickupAvailability(bool bNewCanBePickedUp);
    virtual void HandlePickupAvailabilityChanged();

    UFUNCTION()
    void OnRep_CanBePickedUp();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName ItemName;

    UPROPERTY(EditAnywhere, Category = "Item|Equip")
    FString SocketName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Pickup")
    bool bHideWhenPickedUp = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Pickup")
    bool bAttachToOwnerOnPickup = true;

    UPROPERTY(Transient)
    TWeakObjectPtr<APlayerBase> OwningPlayer;

    UPROPERTY(Transient)
    bool bPendingEquipAttach = false;

    UPROPERTY(Transient, ReplicatedUsing = OnRep_CanBePickedUp)
    bool bCanBePickedUp = true;

    UPROPERTY(Transient)
    bool bIsEquipped = false;
    
    virtual void HandleStartUse() {};
    virtual void HandleStopUse() {};
    
    UFUNCTION(Server, Reliable)
    void ServerStartUse();
    UFUNCTION(Server, Reliable)
    void ServerStopUse();
};
