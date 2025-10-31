// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CommentObject.generated.h"

/**
 * 
 */
UCLASS()
class HORIZONTAL_API UCommentObject : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	FString Comment;
};
