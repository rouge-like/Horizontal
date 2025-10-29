// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResultUI.generated.h"

class UEvaluationItem;
/**
 * 
 */
enum class EValueType : uint8;
class UTextBlock;
UCLASS()
class HORIZONTAL_API UResultUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
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

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalScoreText;

	UPROPERTY()
	TArray<UEvaluationItem*> EvaluationItems;

	void SetValue(int32 ValueIndex, const FString& ValueString, const FString& TitleString, const FString& EvaluationString);
	void SetResult(float TotalScore, int32 Rank);
};
