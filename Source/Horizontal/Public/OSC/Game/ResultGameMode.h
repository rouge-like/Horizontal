// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ResultGameMode.generated.h"

class UCertificateUI;
/**
 * 
 */
enum class EValueType : uint8;
UCLASS()
class HORIZONTAL_API AResultGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCertificateUI> MainUIClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCertificateUI* MainUI;

	UPROPERTY(EditDefaultsOnly)
	TMap<EValueType, FString> GreatCommentMap;
	UPROPERTY(EditDefaultsOnly)
	TMap<EValueType, FString> BadCommentMap;
};
