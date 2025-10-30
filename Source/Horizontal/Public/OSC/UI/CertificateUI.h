// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CertificateUI.generated.h"

class UCommentObject;
/**
 * 
 */
class UTextBlock;
class UListView;
UCLASS()
class HORIZONTAL_API UCertificateUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalRank;
	
	UPROPERTY(meta = (BindWidget))
	UListView* CommentList;

	void UpdateCommentList(const TArray<FString>& Comments);
};
