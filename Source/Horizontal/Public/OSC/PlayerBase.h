#pragma once

#include "CoreMinimal.h"
#include "HorizontalCharacter.h"
#include "PlayerBase.generated.h"

class UInputAction;
UCLASS(Config=Game)
class HORIZONTAL_API APlayerBase : public AHorizontalCharacter
{
	GENERATED_BODY()

public:
	APlayerBase();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool IsSprinting() const { return bIsSprinting; };
protected:
	virtual void BeginPlay() override;

	/** 크라우치 토글에 바인딩할 입력 액션 */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> CrouchAction;

	/** 스프린트 토글에 바인딩할 입력 액션 */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> SprintAction;

	/** 기본 이동 속도 */
	UPROPERTY(EditDefaultsOnly, Category="Movement|Speed", meta=(ClampMin="0.0"))
	float WalkSpeed = 250.0f;

	/** 크라우치 상태 이동 속도 */
	UPROPERTY(EditDefaultsOnly, Category="Movement|Speed", meta=(ClampMin="0.0"))
	float CrouchSpeed = 150.0f;

	/** 스프린트 상태 이동 속도 */
	UPROPERTY(EditDefaultsOnly, Category="Movement|Speed", meta=(ClampMin="0.0"))
	float SprintSpeed = 600.0f;

	/** 로컬 입력으로 결정된 스프린트 의도 */
	bool bWantsToSprint = false;

	/** 서버 권한으로 복제되는 스프린트 상태 */
	UPROPERTY(ReplicatedUsing=OnRep_IsSprinting)
	bool bIsSprinting = false;

	/** 로컬 입력으로 결정된 크라우치 의도 */
	bool bWantsToCrouch = false;

	/** 현재 상태에 맞춰 이동 속도를 갱신 */
	void RefreshMovementSpeed();

	/** 스프린트 상태를 내부적으로 설정 */
	void SetSprintingInternal(bool bNewSprinting);

	/** 크라우치 입력 처리 */
	void HandleCrouchPressed();
	void HandleCrouchReleased();

	/** 스프린트 입력 처리 */
	void HandleSprintPressed();
	void HandleSprintReleased();

	/** 서버에서 스프린트 상태를 갱신 */
	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprinting);

	/** 스프린트 상태 복제 통지를 처리 */
	UFUNCTION()
	void OnRep_IsSprinting();
};
