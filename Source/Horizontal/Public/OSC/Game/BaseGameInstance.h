// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BaseGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class HORIZONTAL_API UBaseGameInstance : public UGameInstance
{
	GENERATED_BODY()

	virtual void Init() override;
	
protected:
	FString DisplayName;
	/** 로딩 UI 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category="Loading")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

private:
	TSharedPtr<SWidget> LoadingWidgetSlate;
	
public:
	UFUNCTION(BlueprintCallable)
	void SetDisplayName(FString NewDisplayName) {DisplayName = NewDisplayName;};
	
	UFUNCTION(BlueprintCallable)
	FString GetDisplayName() {return DisplayName;};

	/** 로딩 시작 (수동 호출용) */
	UFUNCTION(BlueprintCallable)
	void BeginLoadingScreen(const FString& MapName);

	/** 로딩 종료 (수동 호출용) */
	UFUNCTION(BlueprintCallable)
	void EndLoadingScreen(UWorld* LoadedWorld);

	

	
};
