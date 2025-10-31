// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/PlayerBaseState.h"

#include "Net/UnrealNetwork.h"
#include "OSC/PlayerBase.h"
#include "OSC/Game/BaseGameInstance.h"

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

	GI = GetWorld()->GetGameInstance<UBaseGameInstance>();
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
	DOREPLIFETIME(APlayerBaseState, bCanMove);

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
		GI->AddRank(EvaluationKey, 0);
	}
	else if (Result == TEXT("양호"))
	{
		TotalScore += 8;
		GI->AddRank(EvaluationKey, 1);
	}
	else if (Result == TEXT("미흡"))
	{
		TotalScore += 3.5;
		GI->AddRank(EvaluationKey, 2);
	}

	GI->TotalScore = TotalScore;
	return Result;
}

void APlayerBaseState::SetCanMove(bool bNewState)
{
	if (HasAuthority() && bCanMove != bNewState)
	{
		bCanMove = bNewState;
		OnRep_CanMove(); // 서버
	}
}

void APlayerBaseState::OnRep_CanMove()
{
	APlayerController* PC = GetPlayerController();
	if (PC && PC->IsLocalController())
	{
		if (bCanMove)
		{
			PC->SetInputMode(FInputModeGameOnly());
			UE_LOG(LogTemp, Warning, TEXT("Player %s: 입력 활성화 (깨어남)"), *GetPlayerName());
		}
		else
		{
			PC->SetInputMode(FInputModeUIOnly());
			UE_LOG(LogTemp, Warning, TEXT("Player %s: 입력 비활성화 (잠듦)"), *GetPlayerName());
		}
	}
}

