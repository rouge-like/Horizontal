// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/CertificateUI.h"

#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "OSC/Game/BaseGameInstance.h"
#include "OSC/UI/CommentObject.h"

class UBaseGameInstance;

void UCertificateUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	UBaseGameInstance* GI = GetGameInstance<UBaseGameInstance>();

	if (GI->TotalScore >= 90) TotalRank->SetText(FText::FromString(TEXT("금상")));
	else if (GI->TotalScore >= 75) TotalRank->SetText(FText::FromString(TEXT("은상")));
	else if (GI->TotalScore >= 60) TotalRank->SetText(FText::FromString(TEXT("동상")));
	else TotalRank->SetText(FText::FromString(TEXT("장려상")));

	ToHomeButton->OnClicked.AddDynamic(this, &UCertificateUI::OnToHomeButtonClicked);
	EndGameButton->OnClicked.AddDynamic(this, &UCertificateUI::OnEndGameButtonClicked);
}

void UCertificateUI::UpdateCommentList(const TArray<FString>& Comments)
{
	for (const FString& Comment : Comments)
	{
		UCommentObject* EntryObject = NewObject<UCommentObject>(this);
		EntryObject->Comment = Comment;

		CommentList->AddItem(EntryObject);
	}
}

void UCertificateUI::OnToHomeButtonClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("/Game/OSC/Map/StartLevel")));
}

void UCertificateUI::OnEndGameButtonClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, true);
}
