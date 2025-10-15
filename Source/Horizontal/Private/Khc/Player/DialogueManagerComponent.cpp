// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/Player/DialogueManagerComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "Khc/NPC/NPCBase.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"

UDialogueManagerComponent::UDialogueManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDialogueManagerComponent::RequestAdvanceDialogue()
{
	if (DialogueWidgetInstance && DialogueWidgetInstance->IsInViewport())
	{
		Server_AdvanceDialogue();
	}
}

void UDialogueManagerComponent::RequestAdvanceDialogueWithChoice(FName JumpToLabel)
{
	Server_ProcessChoice(JumpToLabel);
}

void UDialogueManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDialogueManagerComponent::Server_ProcessChoice_Implementation(FName JumpToLabel)
{
	CurrentDialogueLabel = JumpToLabel;
    
	FDialogueRow* Row = DialogueTable->FindRow<FDialogueRow>(CurrentDialogueLabel, "");
	if (Row)
	{
		// 공통 로직 호출
		ProcessDialogueRow(Row);
	}
	else
	{
		Client_EndDialogue();
	}
}

void UDialogueManagerComponent::StartDialogue(class UNPCInteractionComponent* TargetNPC, FName StartingLabel)
{
	if (!GetOwner()->HasAuthority()) return; // 서버 전용

	CurrentInteractingNPC = TargetNPC;
	CurrentDialogueLabel = StartingLabel;

	if (DialogueTable && !CurrentDialogueLabel.IsNone())
	{
		FDialogueRow* Row = DialogueTable->FindRow<FDialogueRow>(CurrentDialogueLabel, "");
		if (Row)
		{
			// 2. [서버 -> 클라이언트] 첫 대사 UI를 띄우라고 명령
			Client_UpdateDialogueUI(*Row);
		}
	}
}

void UDialogueManagerComponent::StartObjectDialogue(class UObjectInteractionComponent* TargetObj, FName StartingLabel)
{
	if (!GetOwner()->HasAuthority()) return; // 서버 전용

	CurrentInteractingNPC = nullptr;
	CurrentDialogueLabel = StartingLabel;

	if (DialogueTable && !CurrentDialogueLabel.IsNone())
	{
		FDialogueRow* Row = DialogueTable->FindRow<FDialogueRow>(CurrentDialogueLabel, "");
		if (Row)
		{
			// 2. [서버 -> 클라이언트] 첫 대사 UI를 띄우라고 명령
			Client_UpdateDialogueUI(*Row);
		}
	}
}

void UDialogueManagerComponent::ProcessDialogueRow(const FDialogueRow* Row)
{
	if (!Row) return;

	Client_UpdateDialogueUI(*Row);

	// 만약 다음 대사가 End 타입이라면, 서버에서 미리 게임 로직을 처리합니다.
	if (Row->DialogueType == EDialogueDataType::End)
	{
		if (CurrentInteractingNPC.IsValid())
		{
			ANPCBase* NPC = Cast<ANPCBase>(CurrentInteractingNPC->GetOwner());
			if (NPC && NPC->FSMComp)
			{
				NPC->FSMComp->SetState(ENPCState::Move);
			}
		}
	}
}

void UDialogueManagerComponent::Server_AdvanceDialogue_Implementation()
{
	FString CurrentLabelStr = CurrentDialogueLabel.ToString();
	FString Prefix;
	FString Suffix;

	// 마지막 '_'를 기준으로 문자열을 "NPC01_" (Prefix)와 "1" (Suffix)로 분리
	if (CurrentLabelStr.Split(TEXT("_"), &Prefix, &Suffix, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
	{
		int32 CurrentNumber = FCString::Atoi(*Suffix);
		int32 NextNumber = CurrentNumber + 1;
        
		// "NPC01_" + "2" = "NPC01_2" 형태로 새로운 Label 생성
		FName NextLabel = FName(*FString::Printf(TEXT("%s_%d"), *Prefix, NextNumber));
		CurrentDialogueLabel = NextLabel;
	}
	else
	{
		// _가 없는 형식이면 대화 진행 불가, 그냥 종료
		Client_EndDialogue();
		return;
	}
	FDialogueRow* Row = DialogueTable->FindRow<FDialogueRow>(CurrentDialogueLabel, "");
	if (Row)
	{
		if (Row->DialogueType == EDialogueDataType::End)
		{
			// 대화 종료 로직
			if (CurrentInteractingNPC.IsValid())
			{
				// NPC의 FSM 상태를 Move로 변경
				ANPCBase* NPC = Cast<ANPCBase>(CurrentInteractingNPC->GetOwner());
				if (NPC && NPC->FSMComp)
				{
					NPC->FSMComp->SetState(ENPCState::Move);
				}
			}
			Client_EndDialogue(); // 클라이언트에게 UI를 닫으라고 명령
		}
		else // 일반 대사
		{
			// 6. [서버 -> 클라이언트] 다음 대사 UI를 업데이트하라고 명령
			ProcessDialogueRow(Row);
		}
	}
	else
	{
		// 다음 대사가 없으면 그냥 종료
		Client_EndDialogue();
	}
}

void UDialogueManagerComponent::Client_UpdateDialogueUI_Implementation(const FDialogueRow& DialogueData)
{
	if (!DialogueWidgetClass) return;

	if (!DialogueWidgetInstance) // 위젯이 없으면 새로 생성
	{
		APlayerController* PC = GetOwner<APlayerController>();
		if (!PC) return;

		DialogueWidgetInstance = CreateWidget<UDialogueWidget>(PC, DialogueWidgetClass);
		DialogueWidgetInstance->AddToViewport();
		PC->SetInputMode(FInputModeUIOnly()); // 입력 모드를 UI 전용으로 변경
		PC->bShowMouseCursor = true;
	}

	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->SetDialogueManager(this);
		
		switch (DialogueData.DialogueType)
		{
		case EDialogueDataType::Normal:
		case EDialogueDataType::End:
			DialogueWidgetInstance->UpdateDialogue(DialogueData);
			break;
		case EDialogueDataType::Choice:
			DialogueWidgetInstance->UpdateSelectionDialogue(DialogueData);
			break;
		}
	}
}

void UDialogueManagerComponent::Client_EndDialogue_Implementation()
{
	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->RemoveFromParent();
		DialogueWidgetInstance = nullptr;

		APlayerController* PC = GetOwner<APlayerController>();
		if (PC)
		{
			PC->SetInputMode(FInputModeGameOnly()); // 입력 모드를 다시 게임 전용으로
			PC->bShowMouseCursor = false;
		}
	}
}

