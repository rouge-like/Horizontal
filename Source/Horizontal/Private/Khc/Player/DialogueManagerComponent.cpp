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
#include "Khc/NPC/Component/NPCAStarMovementComponent.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"
#include "Kismet/GameplayStatics.h"
#include "OSC/PlayerBaseController.h"
#include "OSC/PlayerBaseState.h"
#include <Khc/Player/PlayerInteractionComponent.h>

#include "OSC/Sound/SoundManager.h"

class ASoundManager;
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

			ASoundManager* SoundManager = Cast<ASoundManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ASoundManager::StaticClass()));
			EInteractionType type = NPC->InteractionComp->InteractionType;
	
			if (SoundManager)
			{
				switch (type)
				{
				case EInteractionType::InformSituation:
					SoundManager->SpawnSoundAtLocation(FName(TEXT("What")), NPC->GetActorLocation());
					break;
				case EInteractionType::Attention:
					SoundManager->SpawnSoundAtLocation(FName(TEXT("Attention")), NPC->GetActorLocation());
					break;
				case EInteractionType::CalmDown:
					SoundManager->SpawnSoundAtLocation(FName(TEXT("Help")), NPC->GetActorLocation());
					break;
				}
			}
			
			if (NPC->AStarMovementComp)
			{
				NPC->AStarMovementComp->OnMovementFinished.RemoveDynamic(this, &UDialogueManagerComponent::OnNPCMovementFinished);
				NPC->AStarMovementComp->OnMovementFinished.AddDynamic(this, &UDialogueManagerComponent::OnNPCMovementFinished);
			}
		}
	}
	
	if (DialogueTable && !CurrentDialogueLabel.IsNone())
	{
		FDialogueRow* Row = DialogueTable->FindRow<FDialogueRow>(CurrentDialogueLabel, "");
		if (Row)
		{
			Client_UpdateDialogueUI(CurrentDialogueLabel, *Row);
		}
	}
}

void UDialogueManagerComponent::HandleDialogueEnd(const FDialogueRow* DialogueRow)
{
	if (DialogueRow && !DialogueRow->EventTag.IsNone())
	{
		if (CurrentInteractableComponent.IsValid())
		{
			OnDialogueEvent.Broadcast(DialogueRow->EventTag, CurrentInteractableComponent->GetOwner());
		}
	}
    
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
		}
	}

	APlayerBaseState* PlayerState = (Cast<APlayerBaseController>(GetOwner())->GetPlayerState<APlayerBaseState>());

	if (DialogueRow && PlayerState && !DialogueRow->EventTag.IsNone())
	{
		FString EventTagString = DialogueRow->EventTag.ToString();

		const FString Prefix = TEXT("WrongScore_");

		if (EventTagString.StartsWith(Prefix))
		{
			FString ScoreValueString = EventTagString.RightChop(Prefix.Len());

			int32 ScoreValue = FCString::Atoi(*ScoreValueString);

			if (ScoreValue != 0)
			{
				PlayerState->AddWrongScore(ScoreValue);
				UE_LOG(LogTemp, Warning, TEXT("'%s' : %d"), *EventTagString, ScoreValue);
			}
		}
	}

	if (DialogueRow->EventTag == "OpenDoorBad")
	{
		if (PlayerState)
		{
			PlayerState->AddWrongScore(10);
		}
	}

	Client_EndDialogue();
}

void UDialogueManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDialogueManagerComponent::OnNPCMovementFinished()
{
	APlayerBaseState* PlayerState = (Cast<APlayerBaseController>(GetOwner())->GetPlayerState<APlayerBaseState>());
	if (PlayerState)
	{
		PlayerState->AddRecueScore(1);
		UE_LOG(LogTemp, Warning, TEXT("AddResqueScore + 1"));
	}

	if (CurrentInteractableComponent.IsValid())
	{
		if (UNPCInteractionComponent* NPCComp = Cast<UNPCInteractionComponent>(CurrentInteractableComponent.Get()))
		{
			ANPCBase* NPC = Cast<ANPCBase>(NPCComp->GetOwner());
			if (NPC && NPC->AStarMovementComp)
			{
				NPC->AStarMovementComp->OnMovementFinished.RemoveDynamic(this, &UDialogueManagerComponent::OnNPCMovementFinished);
			}
		}
	}
}

void UDialogueManagerComponent::Server_ProcessChoice_Implementation(FName JumpToLabel)
{
	CurrentDialogueLabel = JumpToLabel;
    
	FDialogueRow* Row = DialogueTable->FindRow<FDialogueRow>(CurrentDialogueLabel, "");
	if (Row)
	{
		if (Row->DialogueType == EDialogueDataType::End || Row->DialogueType == EDialogueDataType::EndGood || Row->DialogueType == EDialogueDataType::EndBad)
		{
			Client_UpdateDialogueUI(CurrentDialogueLabel, *Row);
		}
		else // Normal 또는 Choice 타입이면
		{
			Client_UpdateDialogueUI(CurrentDialogueLabel, *Row);
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

	if (CurrentRow->DialogueType == EDialogueDataType::End || CurrentRow->DialogueType == EDialogueDataType::EndGood || CurrentRow->DialogueType == EDialogueDataType::EndBad)
	{
		HandleDialogueEnd(CurrentRow);
		return;
	}
    
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

	FDialogueRow* NextRow = DialogueTable->FindRow<FDialogueRow>(CurrentDialogueLabel, "");
	if (NextRow)
	{
		Client_UpdateDialogueUI(CurrentDialogueLabel, *NextRow);
	}
	else
	{
		Client_EndDialogue();
	}
}

void UDialogueManagerComponent::Client_UpdateDialogueUI_Implementation(FName DialogueLabel, const FDialogueRow& DialogueData)
{
	if (!DialogueWidgetClass) return;

	if (!DialogueWidgetInstance) // 위젯이 없으면 새로 생성
	{
		APlayerController* PC = GetOwner<APlayerController>();
		if (!PC) return;

		ACharacter* player = PC->GetCharacter();
		
		if (player)
		{
			if (UPlayerInteractionComponent* InteractionComp = player->FindComponentByClass<UPlayerInteractionComponent>())
			{
				InteractionComp->HideCrosshair();
			}
		}

		PC->FlushPressedKeys();
		if (player)
		{
			player->GetCharacterMovement()->StopMovementImmediately();
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

		if (DialogueLabel == "NPC03_7")
		{
			DialogueWidgetInstance->ShowImage(0, 1000, 600);
		}
		else if (DialogueLabel == "Door01_2")
		{
			DialogueWidgetInstance->ShowImage(1, 1000, 600);
		}
		else if (DialogueLabel == "MainDoor1_2")
		{
			DialogueWidgetInstance->ShowImage(2, 600, 700);
		}
		else if (DialogueLabel == "FireExt_6")
		{
			DialogueWidgetInstance->ShowImage(3, 600, 700);
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
			if (ACharacter* PlayerCharacter = PC->GetCharacter())
			{
				if (UPlayerInteractionComponent* InteractionComp = PlayerCharacter->FindComponentByClass<UPlayerInteractionComponent>())
				{
					InteractionComp->ShowCrosshair();
				}
			}

			PC->SetInputMode(FInputModeGameOnly()); // 입력 모드를 다시 게임 전용으로
			PC->bShowMouseCursor = false;
		}
	}
}
