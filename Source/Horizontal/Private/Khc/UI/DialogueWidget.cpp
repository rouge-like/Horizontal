// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/UI/DialogueWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Khc/Dialogue/DialogueDataTypes.h"
#include "Khc/Player/DialogueManagerComponent.h"
#include "Khc/UI/UChoiceButtonWidget.h"
#include "Runtime/MediaAssets/Public/MediaPlayer.h"
#include "Runtime/MediaAssets/Public/MediaSource.h"
#include "Runtime/MediaAssets/Public/MediaTexture.h"


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
		if (CurrentDialogueData.DialogueType != EDialogueDataType::Choice)
		{
			DialogueManager->RequestAdvanceDialogue();
		}
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (MediaPlayer)
	{
		MediaPlayer->OnMediaOpened.AddDynamic(this, &UDialogueWidget::OnMediaOpenSuccess);
	}
}

void UDialogueWidget::OnChoiceButtonClicked(FName JumpToLabel)
{
	if (DialogueManager.IsValid())
	{
		// DialogueManager에게 선택된 JumpToLabel로 진행하라고 요청
		DialogueManager->RequestAdvanceDialogueWithChoice(JumpToLabel);
	}
}

void UDialogueWidget::ShowImage(uint32 imgNum)
{
	if (!ExampleImg) return;

	UObject* AssetToShow = DisplayableAssets[imgNum];
	
	if (UTexture2D* Texture = Cast<UTexture2D>(AssetToShow))
	{
		// 비디오가 재생 중이었다면 멈춤
		if (MediaPlayer && MediaPlayer->IsPlaying())
		{
			MediaPlayer->Close();
		}
        
		ExampleImg->SetBrushResourceObject(Texture);
		ExampleImg->SetVisibility(ESlateVisibility::Visible);
	}
	else if (UMediaSource* MediaSource = Cast<UMediaSource>(AssetToShow))
	{
		if (MediaPlayer && MediaTexture)
		{
			ExampleImg->SetBrushResourceObject(MediaTexture);
			ExampleImg->SetVisibility(ESlateVisibility::Visible);

			// 미디어 플레이어에게 비디오 파일을 열고 재생하라고 명령
			MediaPlayer-> OpenSource(MediaSource);
			//MediaPlayer->Play();
		}
	}
	else
	{
		HideImage();
	}
	
	
}

void UDialogueWidget::HideImage()
{
	if (ExampleImg)
	{
		ExampleImg->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (MediaPlayer && MediaPlayer->IsPlaying())
	{
		MediaPlayer->Close(); // 비디오 멈춤
	}
}

void UDialogueWidget::OnMediaOpenSuccess(FString FilePath)
{
	if (MediaPlayer)
	{
		// 파일 열기에 성공했으므로, '지금' 재생합니다.
		MediaPlayer->Play();
	}
}
