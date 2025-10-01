// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPCActor.generated.h"

UENUM(BlueprintType)
enum class ENPCState : uint8
{
	// 도움이 필요한 대기 상태 (플레이어와 상호작용 전)
	E_WaitingForHelp		UMETA(DisplayName = "Waiting For Help"),
	// 안전 장소로 이동 중인 상태
	E_MovingToSafeZone		UMETA(DisplayName = "Moving To Safe Zone"),
	// 안전하게 대피를 완료한 상태
	E_Safe					UMETA(DisplayName = "Safe")
};

// 플레이어가 NPC와 상호작용하는 방식을 정의하는 열거형입니다.
UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	// 주변 상황을 알려주는 상호작용 (상황 전파)
	E_InformSituation		UMETA(DisplayName = "Inform Situation"),
	// 길을 막는 장애물을 제거해주는 상호작용
	E_ClearObstacle			UMETA(DisplayName = "Clear Obstacle"),
	// NPC를 진정시키는 상호작용
	E_CalmDown				UMETA(DisplayName = "Calm Down")
};

UCLASS()
class HORIZONTAL_API ANPCActor : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANPCActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State", ReplicatedUsing = OnRep_NPCState)
	ENPCState CurrentState;


	void SetNPCState(ENPCState NewState);

	UFUNCTION()
	void OnRep_NPCState();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	void Interact(APlayerController* InteractingPlayer, EInteractionType InteractionType);

protected:
	UFUNCTION(Server, Reliable)
	void Server_ProcessInteraction(EInteractionType InteractionType);

	// NPC의 문제가 해결되었는지 판별하는 함수입니다. (서버에서만 호출)
	// 예: '진정시키기' 상호작용이 들어오면 문제가 해결된 것으로 처리
	bool IsProblemSolved(EInteractionType InteractionType);

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "NPC Movement")
	AActor* SafeZoneTarget;

	void MoveToSafeZone();
};
