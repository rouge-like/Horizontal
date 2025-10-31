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
class UButton;
UCLASS()
class HORIZONTAL_API UCertificateUI : public UUserWidget
{
	GENERATED_BODY()
	virtual void NativeConstruct() override;
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalRank;
	
	UPROPERTY(meta = (BindWidget))
	UListView* CommentList;

	UPROPERTY(meta = (BindWidget))
	UButton* ToHomeButton;

	UPROPERTY(meta = (BindWidget))
	UButton* EndGameButton;
	
	void UpdateCommentList(const TArray<FString>& Comments);

	UFUNCTION()
	void OnToHomeButtonClicked();
	UFUNCTION()
	void OnEndGameButtonClicked();
	
};
