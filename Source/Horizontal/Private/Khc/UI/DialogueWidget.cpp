// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/UI/DialogueWidget.h"
#include "Components/TextBlock.h"
#include "Khc/Dialogue/DialogueDataTypes.h"
#include "Khc/Player/DialogueManagerComponent.h"


void UDialogueWidget::UpdateDialogue(const FDialogueRow& DialogueData)
{
	if (Text_DialogueSpeaker && Text_Dialogue)
	{
		// Speaker(FName)를 FText로 변환하여 TextBlock에 설정
		Text_DialogueSpeaker->SetText(FText::FromName(DialogueData.Speaker));

		// DialogueText(FText)를 TextBlock에 설정
		Text_Dialogue->SetText(DialogueData.DialogueText);
	}
}

void UDialogueWidget::SetDialogueManager(class UDialogueManagerComponent* InManager)
{
	DialogueManager = InManager;
}

FReply UDialogueWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// DialogueManager가 유효하다면, 서버에 다음 대사를 요청
		if (DialogueManager.IsValid())
		{
			DialogueManager->RequestAdvanceDialogue(); // 이 함수는 DialogueManager에 새로 만들 겁니다.
		}
        
		// 입력을 처리했음을 시스템에 알림
		return FReply::Handled();
	}

	return FReply::Unhandled();

}
