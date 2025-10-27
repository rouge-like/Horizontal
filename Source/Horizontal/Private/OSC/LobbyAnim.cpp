// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/LobbyAnim.h"
#include "OSC/LobbyPreviewPawn.h"

void ULobbyAnim::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PreviewPawn = Cast<ALobbyPreviewPawn>(TryGetPawnOwner());
	
}

void ULobbyAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (PreviewPawn)
	{
		bIsReady = PreviewPawn->bIsReady;
	}
}
