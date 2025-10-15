// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/UI/UChoiceButtonWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Khc/Dialogue/DialogueDataTypes.h"
#include "Khc/UI/DialogueWidget.h"

void UUChoiceButtonWidget::SetupButton(const FChoiceData& ChoiceData, UDialogueWidget* InParentWidget)
{
	if (ChoiceText)
	{
		ChoiceText->SetText(ChoiceData.Text);
	}
	JumpToLabel = ChoiceData.JumpToLabel;
	ParentDialogueWidget = InParentWidget;
}

void UUChoiceButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ChoiceButton)
	{
		ChoiceButton->OnClicked.AddDynamic(this, &UUChoiceButtonWidget::OnButtonClicked);
	}
}

void UUChoiceButtonWidget::OnButtonClicked()
{
	if (ParentDialogueWidget.IsValid())
	{
		// 부모 위젯(DialogueWidget)의 함수를 호출하여 클릭 이벤트를 전달
		ParentDialogueWidget->OnChoiceButtonClicked(JumpToLabel);
	}
}
