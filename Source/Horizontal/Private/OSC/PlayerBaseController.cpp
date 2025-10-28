// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/PlayerBaseController.h"

#include "Khc/Player/DialogueManagerComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HorizontalCameraManager.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/SpectatorPawn.h"
#include "OSC/PlayerBaseState.h"
#include "OSC/UI/EvaluationItem.h"
#include "OSC/UI/ResultUI.h"

#include "OnlineSubsystem.h"
#include "Interfaces/VoiceInterface.h"

class UEnhancedInputLocalPlayerSubsystem;

APlayerBaseController::APlayerBaseController()
{
	PlayerCameraManagerClass = AHorizontalCameraManager::StaticClass();

	DialogueManagerComponent = CreateDefaultSubobject<UDialogueManagerComponent>(TEXT("DialogueManagerComponent"));
}

void APlayerBaseController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			IOnlineVoicePtr VoiceInterface = Subsystem->GetVoiceInterface();
			if (VoiceInterface.IsValid())
			{
				VoiceInterface->StartNetworkedVoice(0);
				UE_LOG(LogTemp, Log, TEXT("Local voice capture started."));
			}
		}
	}
}

void APlayerBaseController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	APlayerBaseState* PBS = GetPlayerState<APlayerBaseState>();

	if (PBS)
	{
		PBS->SetPawn(aPawn);
	}
}

void APlayerBaseController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

void APlayerBaseController::ServerChangeToSpectator_Implementation()
{
	APawn* P = GetPawn();
	AGameModeBase* GM = GetWorld()->GetAuthGameMode();
	
	ASpectatorPawn* Spectator = GetWorld()->SpawnActor<ASpectatorPawn>(GM->SpectatorClass, P->GetTransform());
	Possess(Spectator);

	APlayerBaseState* PBS = GetPlayerState<APlayerBaseState>();
	if (IsValid(PBS))
		PBS->UpdateValue();
	
	if (IsLocalPlayerController())
		ClientShowResultUI_Implementation();
	else
		ClientShowResultUI();
}

void APlayerBaseController::ClientShowResultUI_Implementation()
{
	ResultUI = CreateWidget<UResultUI>(GetWorld(), ResultUIClass);
	ResultUI->AddToViewport();
	
	if (IsValid(ResultUI))
	{
		APlayerBaseState* PBS = GetPlayerState<APlayerBaseState>();

		if (IsValid(PBS))
		{
			FString PlayTimeValue = FString::Printf(TEXT("%.1f초"), PBS->PlayTime);
			FString PlayTimeEvaluation = PBS->GetEvaluation(EValueType::PlayTime);
			ResultUI->PlayTimeItem->Title->SetText(FText::FromString(TEXT("탈출")));
			ResultUI->PlayTimeItem->Value->SetText(FText::FromString(PlayTimeValue));
			ResultUI->PlayTimeItem->Evaluation->SetText(FText::FromString(PlayTimeEvaluation));

			FString InFireTime = FString::Printf(TEXT("%.1f초"), PBS->InFireTime);
			FString InFireTimeEvaluation = PBS->GetEvaluation(EValueType::InFireTime);
			ResultUI->InFireTimeItem->Title->SetText(FText::FromString(TEXT("불속 노출")));
			ResultUI->InFireTimeItem->Value->SetText(FText::FromString(InFireTime));
			ResultUI->InFireTimeItem->Evaluation->SetText(FText::FromString(InFireTimeEvaluation));
			
			FString RunningTime = FString::Printf(TEXT("%.1f초"), PBS->RunningTime);
			FString RunningTimeEvaluation = PBS->GetEvaluation(EValueType::RunningTime);
			ResultUI->RunningTimeItem->Title->SetText(FText::FromString(TEXT("달리기")));
			ResultUI->RunningTimeItem->Value->SetText(FText::FromString(RunningTime));
			ResultUI->RunningTimeItem->Evaluation->SetText(FText::FromString(RunningTimeEvaluation));
			
			FString NotCrouchedTime = FString::Printf(TEXT("%.1f초"), PBS->NotCrouchedTime);
			FString NotCrouchedTimeEvaluation = PBS->GetEvaluation(EValueType::NotCrouchedTime);
			ResultUI->NotCrouchedTimeItem->Title->SetText(FText::FromString(TEXT("엎드리지 않음")));
			ResultUI->NotCrouchedTimeItem->Value->SetText(FText::FromString(NotCrouchedTime));
			ResultUI->NotCrouchedTimeItem->Evaluation->SetText(FText::FromString(NotCrouchedTimeEvaluation));

			FString NoMaskTime = FString::Printf(TEXT("%.1f초"), PBS->NoMaskTime);
			FString NoMaskTimeEvaluation = PBS->GetEvaluation(EValueType::NoMaskTime);
			ResultUI->NoMaskTimeItem->Title->SetText(FText::FromString(TEXT("입 미가림")));
			ResultUI->NoMaskTimeItem->Value->SetText(FText::FromString(NoMaskTime));
			ResultUI->NoMaskTimeItem->Evaluation->SetText(FText::FromString(NoMaskTimeEvaluation));

			FString RescueScore = FString::Printf(TEXT("%.1f점"), PBS->RescueScore);
			FString RescueScoreEvaluation = PBS->GetEvaluation(EValueType::RescueScore);
			ResultUI->RescueScoreItem->Title->SetText(FText::FromString(TEXT("구조")));
			ResultUI->RescueScoreItem->Value->SetText(FText::FromString(RescueScore));
			ResultUI->RescueScoreItem->Evaluation->SetText(FText::FromString(RescueScoreEvaluation));

			FString ExtinguishScore = FString::Printf(TEXT("%.1f점"), PBS->ExtinguishScore);
			FString ExtinguishScoreEvaluation = PBS->GetEvaluation(EValueType::ExtinguishScore);
			ResultUI->ExtinguishScoreItem->Title->SetText(FText::FromString(TEXT("초기 진압")));
			ResultUI->ExtinguishScoreItem->Value->SetText(FText::FromString(ExtinguishScore));
			ResultUI->ExtinguishScoreItem->Evaluation->SetText(FText::FromString(ExtinguishScoreEvaluation));
			

			FString WrongScore = FString::Printf(TEXT("%.1f점"), PBS->WrongScore);
			FString WrongScoreEvaluation = PBS->GetEvaluation(EValueType::WrongScore);
			ResultUI->WrongScoreItem->Title->SetText(FText::FromString(TEXT("잘못된 행동")));
			ResultUI->WrongScoreItem->Value->SetText(FText::FromString(WrongScore));
			ResultUI->WrongScoreItem->Evaluation->SetText(FText::FromString(WrongScoreEvaluation));
			
		}
	}
}
