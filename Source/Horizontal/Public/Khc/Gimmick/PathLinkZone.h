// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PathLinkZone.generated.h"

UCLASS()
class HORIZONTAL_API APathLinkZone : public AActor
{
	GENERATED_BODY()
	
public:	
	APathLinkZone();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Navigation Link", meta = (MakeEditWidget = true))
	TObjectPtr<APathLinkZone> TargetPoint;
};
