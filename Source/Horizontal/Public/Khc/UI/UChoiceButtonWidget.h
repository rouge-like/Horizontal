// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UChoiceButtonWidget.generated.h"

struct FChoiceData;
/**
 * 
 */
UCLASS()
class HORIZONTAL_API UUChoiceButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 이 버튼에 표시될 선택지 데이터와 부모 위젯을 설정하는 함수
	void SetupButton(const FChoiceData& ChoiceData, class UDialogueWidget* InParentWidget);

protected:
	virtual void NativeConstruct() override;

	// 버튼이 클릭되었을 때 호출될 함수
	UFUNCTION()
	void OnButtonClicked();

	UPROPERTY(meta = (BindWidget))
	class UButton* ChoiceButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ChoiceText;

private:
	FName JumpToLabel;

	UPROPERTY()
	TWeakObjectPtr<class UDialogueWidget> ParentDialogueWidget;
};
