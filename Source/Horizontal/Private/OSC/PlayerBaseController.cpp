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
#include "OnlineSubsystemUtils.h"
#include "GameFramework/GameStateBase.h"
#include "Interfaces/VoiceInterface.h"
#include "OSC/Game/MainGameMode.h"
#include "Khc/Gimmick/MainGameState.h"

class UEnhancedInputLocalPlayerSubsystem;

APlayerBaseController::APlayerBaseController()
{
	PlayerCameraManagerClass = AHorizontalCameraManager::StaticClass();

	DialogueManagerComponent = CreateDefaultSubobject<UDialogueManagerComponent>(TEXT("DialogueManagerComponent"));

}

void APlayerBaseController::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerBaseController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!IsLocalController()) return;

	AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>();
	if (!GS) return;

    APlayerBaseState* PS = GetPlayerState<APlayerBaseState>();
    if (!PS) return;

    IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld());
    if (!OSS) return;
    
    IOnlineVoicePtr VoiceInterface = OSS->GetVoiceInterface();
    if (!VoiceInterface.IsValid()) return;

	if (GS->bVoiceEventCompleted)
	{
		if (bVoiceChatInitialized)
		{
			VoiceInterface->StopNetworkedVoice(0);
			bVoiceChatInitialized = false;
			UE_LOG(LogTemp, Log, TEXT("Player %s (Event End) Mic OFF"), *PS->GetPlayerName());
		}
		return;
	}

    if (PS->bCanMove)
    {
        if (!bVoiceChatInitialized)
        {
            VoiceInterface->StartNetworkedVoice(0);
            bVoiceChatInitialized = true;
            UE_LOG(LogTemp, Log, TEXT("Player %s (1P) Mic ON"), *PS->GetPlayerName());
        }
    }
    else
    {
	    // [2P 로직: 잠들어 있는 플레이어]
    	if (SleepingWidgetInstance == nullptr)
    	{
    		SleepingWidgetInstance = CreateWidget<UUserWidget>(this, SleepingWidgetClass);
    		SleepingWidgetInstance->AddToViewport();
    	}

    	
        // 아직 마이크를 끄지 않았다면 끕니다 (단 한 번만 실행).
        if (bVoiceChatInitialized)
        {
             VoiceInterface->StopNetworkedVoice(0);
             bVoiceChatInitialized = false;
             UE_LOG(LogTemp, Log, TEXT("Player %s (2P) Mic OFF"), *PS->GetPlayerName());
        }

        // 다른 '깨어있는' 플레이어의 소리를 듣습니다.
        for (APlayerState* OtherPS : GS->PlayerArray)
        {
           if (!OtherPS || OtherPS == PS) continue; 
           
           APlayerBaseState* OtherBasePS = Cast<APlayerBaseState>(OtherPS);
           if (OtherBasePS && OtherBasePS->bCanMove) // 1P를 찾음
           {
              FUniqueNetIdPtr UniqueNetId = OtherPS->GetUniqueId().GetUniqueNetId();
              if (UniqueNetId.IsValid())
              {
                 float Amplitude = VoiceInterface->GetAmplitudeOfRemoteTalker(*UniqueNetId);
                 if (Amplitude > WakeUpAmplitude)
                 {
                    Server_RequestWakeUp(); // "깨어남" 요청
                 	SleepingWidgetInstance->RemoveFromParent();
                    return; 
                 }
              }
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

void APlayerBaseController::Server_RequestWakeUp_Implementation()
{
	APlayerBaseState* WakingPlayerPS = GetPlayerState<APlayerBaseState>();
	if (WakingPlayerPS)
	{
		WakingPlayerPS->SetCanMove(true);
	}
    
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;
    
	for (APlayerState* PS : GS->PlayerArray)
	{
		APlayerBaseState* OtherPS = Cast<APlayerBaseState>(PS);
		if (OtherPS && OtherPS != WakingPlayerPS && OtherPS->bCanMove)
		{
			OtherPS->SetCanMove(true);
		}
	}

	AMainGameMode* GameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	if (GameMode)
	{
		GameMode->CompleteVoiceEvent(); // "보이스 이벤트 끝!"
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
