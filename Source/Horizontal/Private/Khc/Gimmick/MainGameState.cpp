// Fill out your copyright notice in the Description page of Project Settings.

#include "Khc/Gimmick/MainGameState.h"
#include "Net/UnrealNetwork.h"

void AMainGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMainGameState, bVoiceEventCompleted);
    DOREPLIFETIME(AMainGameState, bSleepPlayerSetting);
}
