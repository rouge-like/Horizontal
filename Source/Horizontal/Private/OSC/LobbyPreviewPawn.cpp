// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPreviewPawn.h"

#include "Net/UnrealNetwork.h"
#include "OSC/LobbyPlayerController.h"
#include "OSC/Game/LobbyGameMode.h"


// Sets default values
ALobbyPreviewPawn::ALobbyPreviewPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts or when spawned
void ALobbyPreviewPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALobbyPreviewPawn::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPreviewPawn, bIsReady);
}

// Called every frame
void ALobbyPreviewPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ALobbyPreviewPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ALobbyPreviewPawn::OnRep_IsReady()
{
	
}

