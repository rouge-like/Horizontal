// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Game/ResultGameMode.h"

#include "Blueprint/UserWidget.h"
#include "OSC/Game/BaseGameInstance.h"
#include "OSC/UI/CertificateUI.h"


void AResultGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (MainUIClass)
	{
		MainUI = CreateWidget<UCertificateUI>(GetWorld(), MainUIClass);
		MainUI->AddToViewport();
	}

	UBaseGameInstance* GI = GetGameInstance<UBaseGameInstance>();
	TArray<FString> Comments;
	
	for (auto A : GI->Ranks)
	{
		if (A.Value == 0) Comments.Add(GreatCommentMap[A.Key]);
		else if (A.Value == 2) Comments.Add(BadCommentMap[A.Key]);
	}

	MainUI->UpdateCommentList(Comments);
}
