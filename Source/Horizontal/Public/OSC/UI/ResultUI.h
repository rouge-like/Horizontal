// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResultUI.generated.h"

class UEvaluationItem;
/**
 * 
 */
UCLASS()
class HORIZONTAL_API UResultUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UEvaluationItem* PlayTimeItem;

	UPROPERTY(meta = (BindWidget))
	UEvaluationItem* InFireTimeItem;

	UPROPERTY(meta = (BindWidget))
	UEvaluationItem* RunningTimeItem;

	UPROPERTY(meta = (BindWidget))
	UEvaluationItem* NotCrouchedTimeItem;

	UPROPERTY(meta = (BindWidget))
	UEvaluationItem* NoMaskTimeItem;

	UPROPERTY(meta = (BindWidget))
	UEvaluationItem* RescueScoreItem;

	UPROPERTY(meta = (BindWidget))
	UEvaluationItem* ExtinguishScoreItem;

	UPROPERTY(meta = (BindWidget))
	UEvaluationItem* WrongScoreItem;
};
