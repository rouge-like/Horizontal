// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/ResultUI.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "OSC/PlayerBaseState.h"
#include "OSC/Game/MainGameMode.h"
#include "OSC/UI/EvaluationItem.h"

void UResultUI::NativeConstruct()
{
	Super::NativeConstruct();

	EvaluationItems.Add(PlayTimeItem);
	EvaluationItems.Add(InFireTimeItem);
	EvaluationItems.Add(RunningTimeItem);
	EvaluationItems.Add(NotCrouchedTimeItem);
	EvaluationItems.Add(NoMaskTimeItem);
	EvaluationItems.Add(RescueScoreItem);
	EvaluationItems.Add(ExtinguishScoreItem);
	EvaluationItems.Add(WrongScoreItem);
	
	EndButton->OnClicked.AddDynamic(this, &UResultUI::OnEndButtonClicked);
}

void UResultUI::SetValue(int32 ValueIndex, const FString& ValueString, const FString& TitleString, const FString& EvaluationString)
{
	UEvaluationItem* CurrentItem = EvaluationItems[ValueIndex];

	CurrentItem->Title->SetText(FText::FromString(TitleString));
	CurrentItem->Value->SetText(FText::FromString(ValueString));
	CurrentItem->Evaluation->SetText(FText::FromString(EvaluationString));
}

void UResultUI::SetResult(float TotalScore, int32 Rank)
{
	TotalScoreText->SetText(FText::FromString(FString::Printf(TEXT("%.1f점"), TotalScore)));
}

void UResultUI::OnEndButtonClicked()
{
	AMainGameMode* GM = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode());
	GM->EndSimulation();
}

