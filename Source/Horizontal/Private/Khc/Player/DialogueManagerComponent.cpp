// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/Player/DialogueManagerComponent.h"

#include "AIController.h"
#include "MediaPlayer.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Khc/Gimmick/AStarGridManager.h"
#include "Khc/Gimmick/AStarNavigationManager.h"
#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "Khc/NPC/NPCBase.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"
#include "Kismet/GameplayStatics.h"

class AAIController;

UDialogueManagerComponent::UDialogueManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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

void UDialogueManagerComponent::StartDialogue(class UInteractableComponentBase* TargetComp, FName StartingLabel)
{
	if (!GetOwner()->HasAuthority()) return;

	CurrentInteractableComponent = TargetComp; // 통합된 변수에 저장
	CurrentDialogueLabel = StartingLabel;

	if (UNPCInteractionComponent* NPCComp = Cast<UNPCInteractionComponent>(TargetComp))
	{
		ANPCBase* NPC = Cast<ANPCBase>(NPCComp->GetOwner());
		APlayerController* PC = GetOwner<APlayerController>();

		if (NPC && PC)
		{
			AAIController* NPCController = Cast<AAIController>(NPC->GetController());
			// NPC의 AI 컨트롤러에게 대화가 끝날 때까지 플레이어를 바라보도록 설정
			if (NPCController)
			{
				NPCController->SetFocus(PC->GetPawn());
			}
		}
	}
	
	if (DialogueTable && !CurrentDialogueLabel.IsNone())
	{
		FDialogueRow* Row = DialogueTable->FindRow<FDialogueRow>(CurrentDialogueLabel, "");
		if (Row)
		{
			Client_UpdateDialogueUI(*Row);
		}
	}
}

void UDialogueManagerComponent::HandleDialogueEnd(const FDialogueRow* DialogueRow)
{
	if (DialogueRow && !DialogueRow->EventTag.IsNone())
	{
		// 현재 상호작용 중인 대상 액터를 가져옴
		if (CurrentInteractableComponent.IsValid())
		{
			// "이벤트가 발생했다!" 라고 방송. 이 신호를 듣는 액터가 스스로 행동함.
			OnDialogueEvent.Broadcast(DialogueRow->EventTag, CurrentInteractableComponent->GetOwner());
		}
	}
    
	// 2. 만약 상호작용 대상이 NPC였다면, 추가로 FSM 상태 변경
	if (UNPCInteractionComponent* NPCComp = Cast<UNPCInteractionComponent>(CurrentInteractableComponent.Get()))
	{
		ANPCBase* NPC = Cast<ANPCBase>(NPCComp->GetOwner());
		if (NPC && NPC->FSMComp)
		{
			AAIController* NPCController = Cast<AAIController>(NPC->GetController());
			if (NPCController)
			{
				NPCController->ClearFocus(EAIFocusPriority::Gameplay);
			}

			// 이동 전에 그리드를 다시 만들도록 요청
			// AAStarNavigationManager* NavManager = Cast<AAStarNavigationManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarNavigationManager::StaticClass()));
			// if (NavManager)
			// {
			// 	AAStarGridManager* GridManager = NavManager->GetManagerForLocation(NPC->GetActorLocation());
			// 	if (GridManager)
			// 	{
			// 		GridManager->RebuildGrid();
			// 	}
			// }
			// NPC->FSMComp->SetState(ENPCState::Move);
		}
	}

	// 3. 모든 클라이언트에게 대화 종료를 알림
	Client_EndDialogue();
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
		// 선택지로 점프한 결과가 End 타입일 수도 있으므로, 여기서도 확인
		if (Row->DialogueType == EDialogueDataType::End || Row->DialogueType == EDialogueDataType::EndGood || Row->DialogueType == EDialogueDataType::EndBad)
		{
			// End 타입이면 마지막 대사를 일단 보여주고, 다음 클릭 때 종료되도록 함
			Client_UpdateDialogueUI(*Row);
		}
		else // Normal 또는 Choice 타입이면
		{
			Client_UpdateDialogueUI(*Row);
		}
	}
	else
	{
		Client_EndDialogue();
	}
}


void UDialogueManagerComponent::Server_AdvanceDialogue_Implementation()
{
	FDialogueRow* CurrentRow = DialogueTable->FindRow<FDialogueRow>(CurrentDialogueLabel, "");
	if (!CurrentRow)
	{
		Client_EndDialogue();
		return;
	}

	// 2. 현재 대사가 End 타입 중 하나인지 확인합니다.
	if (CurrentRow->DialogueType == EDialogueDataType::End || CurrentRow->DialogueType == EDialogueDataType::EndGood || CurrentRow->DialogueType == EDialogueDataType::EndBad)
	{
		// 'End' 타입의 대사를 보고 난 후 클릭한 것이므로, HandleDialogueEnd를 호출하여 대화를 완전히 종료합니다.
		HandleDialogueEnd(CurrentRow);
		return;
	}
    
	// 3. 현재 대사가 End가 아니라면, 다음 대사 Label을 계산합니다.
	FString CurrentLabelStr = CurrentDialogueLabel.ToString();
	FString Prefix;
	FString Suffix;

	if (CurrentLabelStr.Split(TEXT("_"), &Prefix, &Suffix, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
	{
		int32 CurrentNumber = FCString::Atoi(*Suffix);
		int32 NextNumber = CurrentNumber + 1;
		FName NextLabel = FName(*FString::Printf(TEXT("%s_%d"), *Prefix, NextNumber));
		CurrentDialogueLabel = NextLabel;
	}
	else
	{
		Client_EndDialogue();
		return;
	}

	// 4. 계산된 다음 대사 정보를 찾아옵니다.
	FDialogueRow* NextRow = DialogueTable->FindRow<FDialogueRow>(CurrentDialogueLabel, "");
	if (NextRow)
	{
		// 5. 찾은 다음 대사를 클라이언트에게 보여주라고 명령합니다.
		Client_UpdateDialogueUI(*NextRow);
	}
	else
	{
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
		
		PC->FlushPressedKeys();
		if (ACharacter* PlayerCharacter = PC->GetCharacter())
		{
			PlayerCharacter->GetCharacterMovement()->StopMovementImmediately();
		}
		
		DialogueWidgetInstance = CreateWidget<UDialogueWidget>(PC, DialogueWidgetClass);
		DialogueWidgetInstance->AddToViewport();
		PC->SetInputMode(FInputModeUIOnly()); // 입력 모드를 UI 전용으로 변경
		PC->bShowMouseCursor = true;
	}

	if (DialogueWidgetInstance)
	{
		DialogueWidgetInstance->SetDialogueManager(this);
		DialogueWidgetInstance->HideImage();
		
		switch (DialogueData.DialogueType)
		{
		case EDialogueDataType::Normal:
		case EDialogueDataType::End:
			DialogueWidgetInstance->UpdateDialogue(DialogueData);
			break;
		case EDialogueDataType::Choice:
			DialogueWidgetInstance->UpdateSelectionDialogue(DialogueData);
			break;
		case EDialogueDataType::EndGood:
			DialogueWidgetInstance->UpdateDialogue(DialogueData);
			break;
		case EDialogueDataType::EndBad:
			DialogueWidgetInstance->UpdateDialogue(DialogueData);
			break;
		}

		if (CurrentDialogueLabel == "Door01_3")
		{
			DialogueWidgetInstance->ShowImage(0);
		}
		else if (CurrentDialogueLabel == "NPC03_7")
		{
			DialogueWidgetInstance->ShowImage(1);
		}
	}
}

void UDialogueManagerComponent::Client_EndDialogue_Implementation()
{
	if (GetOwner()->HasAuthority())
	{
		if (CurrentInteractableComponent.IsValid())
		{
			ANPCBase* NPC = Cast<ANPCBase>(CurrentInteractableComponent->GetOwner());
			if (NPC)
			{
				AAIController* NPCController = Cast<AAIController>(NPC->GetController());
				if (NPCController)
				{
					NPCController->ClearFocus(EAIFocusPriority::Gameplay);
				}
			}
		}
	}
	
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
