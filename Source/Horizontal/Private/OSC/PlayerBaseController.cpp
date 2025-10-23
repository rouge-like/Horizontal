// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/PlayerBaseController.h"

#include "Khc/Player/DialogueManagerComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HorizontalCameraManager.h"
#include "OSC/PlayerBaseState.h"

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
