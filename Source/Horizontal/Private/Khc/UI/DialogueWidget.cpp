// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/UI/DialogueWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Khc/Dialogue/DialogueDataTypes.h"
#include "Khc/Player/DialogueManagerComponent.h"
#include "Khc/UI/UChoiceButtonWidget.h"


void UDialogueWidget::UpdateDialogue(const FDialogueRow& DialogueData)
{
	CurrentDialogueData = DialogueData;
	if (SelectionBox)
	{
		SelectionBox->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (Text_DialogueSpeaker && Text_Dialogue)
	{
		Text_DialogueSpeaker->SetText(FText::FromName(DialogueData.Speaker));
		Text_Dialogue->SetText(DialogueData.DialogueText);
	}
}

void UDialogueWidget::UpdateSelectionDialogue(const FDialogueRow& DialogueData)
{
	CurrentDialogueData = DialogueData;
	if (!SelectionBox || !ChoiceButtonClass) return;

	SelectionBox->ClearChildren();
	for (const FChoiceData& Choice : DialogueData.Choices)
	{
		// UUserWidget 대신 UChoiceButtonWidget으로 생성해야 합니다.
		UUChoiceButtonWidget* ChoiceWidget = CreateWidget<UUChoiceButtonWidget>(this, ChoiceButtonClass);
		if (ChoiceWidget)
		{
			// 버튼에 데이터와 부모 위젯(자기 자신) 정보를 넘겨줌
			ChoiceWidget->SetupButton(Choice, this);

			UVerticalBoxSlot* SelectionSlot = SelectionBox->AddChildToVerticalBox(ChoiceWidget);
			if (SelectionSlot)
			{
				// 위아래로 5픽셀씩 여백을 줍니다. (값은 원하는 대로 조절 가능)
				SelectionSlot->SetPadding(FMargin(0.f, 50.f));
			}
		}
	}
	if (Text_DialogueSpeaker && Text_Dialogue)
	{
		Text_DialogueSpeaker->SetText(FText::FromName(DialogueData.Speaker));
		Text_Dialogue->SetText(DialogueData.DialogueText);
	}
	
	SelectionBox->SetVisibility(ESlateVisibility::Visible);
}

void UDialogueWidget::SetDialogueManager(class UDialogueManagerComponent* InManager)
{
	DialogueManager = InManager;
}

FReply UDialogueWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// DialogueManager가 없으면 아무것도 하지 않음
	if (!DialogueManager.IsValid())
	{
		return FReply::Unhandled();
	}
    
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 현재 대사 타입이 '선택지'가 아닐 때만 다음 대사로 넘어갑니다.
		if (CurrentDialogueData.DialogueType != EDialogueDataType::Choice)
		{
			DialogueManager->RequestAdvanceDialogue();
		}
        
		// 어떤 경우든 클릭 입력을 여기서 '처리했음'으로 간주하여
		// 클릭이 UI 뒤(게임 월드)로 넘어가지 않게 합니다.
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UDialogueWidget::OnChoiceButtonClicked(FName JumpToLabel)
{
	if (DialogueManager.IsValid())
	{
		// DialogueManager에게 선택된 JumpToLabel로 진행하라고 요청
		DialogueManager->RequestAdvanceDialogueWithChoice(JumpToLabel);
	}
}
