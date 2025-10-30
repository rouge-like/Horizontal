// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/CertificateUI.h"

#include "Components/ListView.h"
#include "OSC/UI/CommentObject.h"

void UCertificateUI::UpdateCommentList(const TArray<FString>& Comments)
{
	for (const FString& Comment : Comments)
	{
		UCommentObject* EntryObject = NewObject<UCommentObject>(this);
		EntryObject->Comment = Comment;

		CommentList->AddItem(EntryObject);
	}
}
