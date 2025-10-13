#pragma once

#include "CoreMinimal.h"
#include "HorizontalCharacter.h"
#include "PlayerBase.generated.h"

class UInventoryComponent;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSetUpPlayerInput, UInputComponent*, PlayerInputComponent);

UENUM(BlueprintType)
enum class EHandsState : uint8
{
	None,
	CoveringMouth,
	TwoHandedGrabbing,
	Aiming
};


UCLASS(Config=Game)
class HORIZONTAL_API APlayerBase : public AHorizontalCharacter
{
	GENERATED_BODY()

public:
	APlayerBase();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool IsSprinting() const { return bIsSprinting; }
	
	EHandsState GetHandsState() const { return HandsState; }
	void SetHandsState(EHandsState NewState) { HandsState = NewState; };
	UInventoryComponent* GetInventoryComponent() const { return InventoryComp; }

	FOnSetUpPlayerInput OnSetUpPlayerInputDelegate;
	
protected:
	virtual void BeginPlay() override;

	// 인벤토리 컴포넌트
	UPROPERTY(EditDefaultsOnly)
	UInventoryComponent* InventoryComp;

	// 양손 상태 (애니메이션용)
	UPROPERTY(Replicated)
	EHandsState HandsState;
	
	// 크라우치 토글에 바인딩할 입력 액션
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> CrouchAction;

	// 스프린트 토글에 바인딩할 입력 액션
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> SprintAction;
	
	// 스프린트 토글에 바인딩할 입력 액션
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> PickupAction;

	// 스프린트 토글에 바인딩할 입력 액션 
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> DropAction;
	
	// 기본 이동 속도
	UPROPERTY(EditDefaultsOnly, Category="Movement|Speed", meta=(ClampMin="0.0"))
	float WalkSpeed = 250.0f;

	// 크라우치 상태 이동 속도
	UPROPERTY(EditDefaultsOnly, Category="Movement|Speed", meta=(ClampMin="0.0"))
	float CrouchSpeed = 150.0f;

	// 스프린트 상태 이동 속도
	UPROPERTY(EditDefaultsOnly, Category="Movement|Speed", meta=(ClampMin="0.0"))
	float SprintSpeed = 600.0f;

	// 아이템 획득 범위
	UPROPERTY(EditDefaultsOnly, Category="Pickup", meta=(ClampMin="0.0"))
	float PickupRadius = 600.0f;
	
	// 로컬 입력으로 결정된 스프린트 의도
	bool bWantsToSprint = false;

	// 서버 권한으로 복제되는 스프린트 상태
	UPROPERTY(ReplicatedUsing=OnRep_IsSprinting)
	bool bIsSprinting = false;

	// 로컬 입력으로 결정된 크라우치 의도
	bool bWantsToCrouch = false;

	// 현재 상태에 맞춰 이동 속도를 갱신
	void RefreshMovementSpeed();

	// 스프린트 상태를 내부적으로 설정
	void SetSprintingInternal(bool bNewSprinting);
	// 픽업 상태를 내부적으로 설정
	void SetPickupInternal(AUsableItemBase* Item);
	// 드랍 상태를 내부적으로 설정
	void SetDropInternal();
	
	// 크라우치 입력 처리
	void HandleCrouchPressed();
	void HandleCrouchReleased();

	// 스프린트 입력 처리
	void HandleSprintPressed();
	void HandleSprintReleased();
	
	// 픽업 입력 처리
	void HandlePickupStarted();
	// 드랍 입력 처리
	void HandleDropStarted();
	
	// 서버에서 스프린트 상태를 갱신
	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprinting);
	// 서버에서 픽업 상태를 갱신
	UFUNCTION(Server, Reliable)
	void ServerSetPickup(AUsableItemBase* Item);
	// 서버에서 드랍 상태를 갱신
	UFUNCTION(Server, Reliable)
	void ServerSetDrop();
	
	// 스프린트 상태 복제 통지를 처리
	UFUNCTION()
	void OnRep_IsSprinting();

	class UPlayerInteractionComponent* InteractionComponent;
};