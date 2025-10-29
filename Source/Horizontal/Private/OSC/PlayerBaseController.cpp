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

void APlayerBaseController::SetValue(EValueType Type, float& OutValue, APlayerBaseState* PBS)
{
	switch (Type)
	{
		case EValueType::PlayTime:
			OutValue = PBS->PlayTime;
			break;
		case EValueType::InFireTime:
			OutValue = PBS->InFireTime;
			break;
		case EValueType::RunningTime:
			OutValue = PBS->RunningTime;
			break;
		case EValueType::NotCrouchedTime:
			OutValue = PBS->NotCrouchedTime;
			break;
		case EValueType::NoMaskTime:
			OutValue = PBS->NoMaskTime;
			break;
		case EValueType::RescueScore:
			OutValue = PBS->RescueScore;
			break;
		case EValueType::ExtinguishScore:
			OutValue = PBS->ExtinguishScore;
			break;
		case EValueType::WrongScore:
			OutValue = PBS->WrongScore;
			break;
		default:
			OutValue = 0;
			break;
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
			for (EValueType Type : TEnumRange<EValueType>())
			{
				float Value;
				SetValue(Type, Value, PBS);
				FString ValueString = FString::Printf(TEXT("%.1f%s"),Value, *PBS->ValueUnitString[Type]);
				FString TileString = PBS->ValueTitleString[Type];
				FString EvaluationString = PBS->GetEvaluation(Type);
				ResultUI->SetValue(static_cast<int>(Type), ValueString,TileString , EvaluationString);
			}

			ResultUI->SetResult(PBS->TotalScore, 0);
		}
	}
}
