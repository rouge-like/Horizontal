// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/PlayerBaseState.h"

#include "Net/UnrealNetwork.h"
#include "OSC/PlayerBase.h"

APlayerBaseState::APlayerBaseState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerBaseState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(UpdateValueTimer, this, &APlayerBaseState::UpdateValue, 1, true);
	}
}

void APlayerBaseState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerBaseState, RepPlayTime);
	DOREPLIFETIME(APlayerBaseState, RepInFireTime);
	DOREPLIFETIME(APlayerBaseState, RepRunningTime);
	DOREPLIFETIME(APlayerBaseState, RepNotCrouchedTime);
	DOREPLIFETIME(APlayerBaseState, RepNoMaskTime);
	DOREPLIFETIME(APlayerBaseState, RepRescueScore);
	DOREPLIFETIME(APlayerBaseState, RepExtinguishScore);
	DOREPLIFETIME(APlayerBaseState, RepWrongScore);
}

void APlayerBaseState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!HasAuthority()) return;
	if (!PlayerBase) return;
	if (bIsInteracting) return;
	PlayTime += DeltaTime;
	if (PlayerBase->IsSprinting()) RunningTime += DeltaTime;
	if (!PlayerBase->IsCrouched()) NotCrouchedTime += DeltaTime;
	if (!PlayerBase->IsCoveringMouth()) NoMaskTime += DeltaTime;
}

void APlayerBaseState::UpdateValue()
{
	RepPlayTime = PlayTime;
	RepInFireTime = InFireTime;
	RepRunningTime = RunningTime;
	RepNotCrouchedTime = NotCrouchedTime;
	RepNoMaskTime = NoMaskTime;
	RepRescueScore = RescueScore;
	RepExtinguishScore = ExtinguishScore;
	RepWrongScore = WrongScore;
}

void APlayerBaseState::OnRep_UpdateValue()
{
	PlayTime = RepPlayTime;
	InFireTime = RepInFireTime;
	RunningTime = RepRunningTime;
	NotCrouchedTime = RepNotCrouchedTime;
	NoMaskTime = RepNoMaskTime;
	RescueScore = RepRescueScore;
	ExtinguishScore = RepExtinguishScore;
	WrongScore = RepWrongScore;
}

void APlayerBaseState::SetPawn(APawn* Pawn)
{
	PlayerBase = Cast<APlayerBase>(Pawn);
}

void APlayerBaseState::AddFireTime(float DeltaTime)
{
	if (bIsInteracting) return;
		InFireTime += DeltaTime;
}

void APlayerBaseState::AddRecueScore(float Value)
{
	RescueScore += Value;
}

void APlayerBaseState::AddExtinguishScore(float Value)
{
	ExtinguishScore += Value;
}

void APlayerBaseState::AddWrongScore(float Value)
{
	WrongScore += Value;
}

FString APlayerBaseState::GetEvaluation(EValueType EvaluationKey)
{
	FString Result;
	if (EvaluationMap.Contains(EvaluationKey))
	{
		FEvaluationData Data = EvaluationMap[EvaluationKey];
		
		if (Data.ValueType == EValueType::PlayTime)
		{
			Result = Data.GetEvaluation(PlayTime);
		}
		if (Data.ValueType == EValueType::InFireTime)
		{
			Result =  Data.GetEvaluation(InFireTime);
		}
		if (Data.ValueType == EValueType::RunningTime)
		{
			Result =  Data.GetEvaluation(RunningTime);
		}
		if (Data.ValueType == EValueType::NotCrouchedTime)
		{
			Result =  Data.GetEvaluation(NotCrouchedTime);
		}
		if (Data.ValueType == EValueType::NoMaskTime)
		{
			Result =  Data.GetEvaluation(NoMaskTime);
		}
		if (Data.ValueType == EValueType::RescueScore)
		{
			Result =  Data.GetEvaluation(RescueScore);
		}
		if (Data.ValueType == EValueType::ExtinguishScore)
		{
			Result =  Data.GetEvaluation(ExtinguishScore);
		}
		if (Data.ValueType == EValueType::WrongScore)
		{
			Result =  Data.GetEvaluation(WrongScore);
		}
	}
	if (Result == TEXT("우수"))
	{
		TotalScore += 12.5f;
	}
	else if (Result == TEXT("양호"))
	{
		TotalScore += 8;
	}
	else if (Result == TEXT("미흡"))
	{
		TotalScore += 3.5;
	}
	return Result;
}

