// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EvaluationItem.generated.h"

/**
 * 
 */
class UTextBlock;
UCLASS()
class HORIZONTAL_API UEvaluationItem : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Title;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Value;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Evaluation;
};
