#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class AUsableItemBase;
class FLifetimeProperty;
class APlayerBase;
class UInputAction;
class UInputComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORIZONTAL_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

protected:
    virtual void InitializeComponent() override;
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(VisibleAnywhere)
    APlayerBase* OwningPlayer;
    
    UPROPERTY(EditDefaultsOnly, Category="Input")
    TObjectPtr<UInputAction> UseItemAction;
    UPROPERTY(EditDefaultsOnly, Category="Input")
    TObjectPtr<UInputAction> AimAction;
    
    UFUNCTION()
    void OnSetUpPlayerInput(UInputComponent* PlayerInputComponent);
    
    UPROPERTY(VisibleAnywhere)
    TArray<TWeakObjectPtr<AUsableItemBase>> Inventory;

    UPROPERTY(VisibleAnywhere, Transient, ReplicatedUsing=OnRep_SelectedItem)
    TObjectPtr<AUsableItemBase> SelectedItem;

    UFUNCTION()
    void OnRep_SelectedItem(AUsableItemBase* PreviousItem);

    void ApplySelectionInternal(int32 Idx);
    void RemoveItemInternal(int32 ItemIndex, bool bCallOnDrop);

    UFUNCTION(Server, Reliable)
    void ServerSelectItem(int32 Idx);

public:
    UFUNCTION(BlueprintCallable)
    void AddItem(AUsableItemBase* NewItem);

    UFUNCTION(BlueprintCallable)
    void RemoveItem(int32 ItemIndex);

    UFUNCTION(BlueprintCallable)
    bool RemoveSelectedItem();

    UFUNCTION(BlueprintCallable)
    bool ContainsItem(AUsableItemBase* Item) const;

    UFUNCTION(BlueprintCallable)
    void StartUseItem();

    UFUNCTION(BlueprintCallable)
    void StopUseItem();

    UFUNCTION(BlueprintCallable)
    void SelectItem(int32 Idx);

    UFUNCTION(BlueprintCallable)
    void StartAim();

    UFUNCTION(BlueprintCallable)
    void StopAim();

    UFUNCTION(BlueprintCallable)
    AUsableItemBase* GetSelectedItem() const { return SelectedItem.Get(); }
};
