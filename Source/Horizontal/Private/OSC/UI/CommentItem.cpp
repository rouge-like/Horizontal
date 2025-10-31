// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/CommentItem.h"

#include "Components/TextBlock.h"
#include "OSC/UI/CommentObject.h"

void UCommentItem::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	if (const UCommentObject* CommentData = Cast<UCommentObject>(ListItemObject))
	{
		CommentText->SetText(FText::FromString(CommentData->Comment));
	}
}