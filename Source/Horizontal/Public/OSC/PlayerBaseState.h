// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PlayerBaseState.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EValueType : uint8
{
	PlayTime,
	InFireTime,
	RunningTime,
	NotCrouchedTime,
	NoMaskTime,
	RescueScore,
	ExtinguishScore,
	WrongScore
};

USTRUCT(BlueprintType)
struct HORIZONTAL_API FEvaluationData
{
	GENERATED_BODY()
	FEvaluationData() : ValueType(EValueType::PlayTime), ExcellentValue(0), GoodValue(0), bIsGreater(false)
	{
		
	}
	UPROPERTY(EditAnywhere)
	EValueType ValueType;
	
	UPROPERTY(EditAnywhere)
	float ExcellentValue;

	UPROPERTY(EditAnywhere)
	float GoodValue;

	UPROPERTY(EditAnywhere)
	bool bIsGreater;
	
public:
	FString GetEvaluation(float Value) const
	{
		if (bIsGreater)
		{
			if (Value >= ExcellentValue) return FString(TEXT("우수"));
			if (Value >= GoodValue) return FString(TEXT("양호"));
			return FString(TEXT("나쁨"));
		}
		else
		{
			if (Value <= ExcellentValue) return FString(TEXT("우수"));
			if (Value <= GoodValue) return FString(TEXT("양호"));
			return FString(TEXT("나쁨"));
		}
	};
};
class APlayerBase;
UCLASS()
class HORIZONTAL_API APlayerBaseState : public APlayerState
{
	GENERATED_BODY()
	APlayerBaseState();
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly)
	float Size = 35;

	UPROPERTY()
	APlayerBase* PlayerBase;
	
	UPROPERTY(ReplicatedUsing = OnRep_UpdateValue)
	float RepPlayTime = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_UpdateValue)
	float RepInFireTime = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_UpdateValue)
	float RepRunningTime = 0;

	UPROPERTY(ReplicatedUsing = OnRep_UpdateValue)
	float RepNotCrouchedTime = 0;

	UPROPERTY(ReplicatedUsing = OnRep_UpdateValue)
	float RepNoMaskTime = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_UpdateValue)
	float RepRescueScore = 0;

	UPROPERTY(ReplicatedUsing = OnRep_UpdateValue)
	float RepExtinguishScore = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_UpdateValue)
	float RepWrongScore = 0;
	
	UPROPERTY(EditAnywhere)
	TMap<EValueType, FEvaluationData> EvaluationMap;
	
	UFUNCTION()
	void OnRep_UpdateValue();

	FTimerHandle UpdateValueTimer;
public:
	UPROPERTY(VisibleAnywhere)
	float PlayTime = 0;
	
	UPROPERTY(VisibleAnywhere)
	float InFireTime = 0;
	
	UPROPERTY(VisibleAnywhere)
	float RunningTime = 0;

	UPROPERTY(VisibleAnywhere)
	float NotCrouchedTime = 0;

	UPROPERTY(VisibleAnywhere)
	float NoMaskTime = 0;
	
	UPROPERTY(VisibleAnywhere)
	float RescueScore = 0;

	UPROPERTY(VisibleAnywhere)
	float ExtinguishScore = 0;
	
	UPROPERTY(VisibleAnywhere)
	float WrongScore = 0;

	UFUNCTION()
	void UpdateValue();
	
	void SetPawn(APawn* Pawn);
	float GetSize() const { return Size; };
	float GetSum() const {return InFireTime + RunningTime + NotCrouchedTime + NoMaskTime + PlayTime; };
	void AddFireTime(float DeltaTime);
	void AddRecueScore(float Value);
	void AddExtinguishScore(float Value);
	void AddWrongScore(float Value);
	
	FString GetEvaluation(EValueType EvaluationKey);
};
