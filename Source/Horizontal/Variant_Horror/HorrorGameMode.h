// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HorrorGameMode.generated.h"

/**
 *  Simple GameMode for a first person horror game
 */
UCLASS(abstract)
class HORIZONTAL_API AHorrorGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	AHorrorGameMode();

	virtual void BeginPlay() override;
	virtual void StartPlay() override;
protected:
	// 새로운 플레이어가 성공적으로 로그인했을 때 서버에서 호출되는 함수
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	// 성능을 위해 레벨에 있는 모든 상호작용 오브젝트 목록을 미리 저장해 둠
	UPROPERTY()
	TArray<class AInteractableObjectBase*> AllInteractableObjectsInLevel;
};
