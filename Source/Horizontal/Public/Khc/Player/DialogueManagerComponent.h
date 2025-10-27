// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Khc/Dialogue/DialogueDataTypes.h"
#include "Khc/UI/DialogueWidget.h"
#include "DialogueManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueEvent, FName, EventTag, AActor*, InteractableActor);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORIZONTAL_API UDialogueManagerComponent : public UActorComponent
{
	GENERATED_BODY()
public:    
	UDialogueManagerComponent();

	void RequestAdvanceDialogue();
	void RequestAdvanceDialogueWithChoice(FName JumpToLabel);


	// [서버 전용] 새로운 대화를 시작하는 함수
	void StartDialogue(class UInteractableComponentBase* TargetComp, FName StartingLabel);
	void HandleDialogueEnd(const FDialogueRow* DialogueRow);

	FOnDialogueEvent OnDialogueEvent;

protected:
	virtual void BeginPlay() override;
	
	//void OnAdvanceDialoguePressed();

	// [서버] 다음 대사로 진행하는 로직
	UFUNCTION(Server, Reliable)
	void Server_AdvanceDialogue();

	// [클라이언트] 서버의 명령을 받아 UI를 업데이트하는 함수
	UFUNCTION(Client, Reliable)
	void Client_UpdateDialogueUI(const FDialogueRow& DialogueData);

	// [클라이언트] 대화 종료를 알리는 함수
	UFUNCTION(Client, Reliable)
	void Client_EndDialogue();

	UFUNCTION(Server, Reliable)
	void Server_ProcessChoice(FName JumpToLabel);

private:
	// 에디터에서 설정할 대사 데이터 테이블
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	TObjectPtr<class UDataTable> DialogueTable;

	// 에디터에서 설정할 대화 UI 위젯 클래스
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	TSubclassOf<UDialogueWidget> DialogueWidgetClass;

	UPROPERTY()
	FName CurrentDialogueLabel;

	UPROPERTY()
	TWeakObjectPtr<UInteractableComponentBase> CurrentInteractableComponent;

	UPROPERTY()
	TObjectPtr<UDialogueWidget> DialogueWidgetInstance;

	UFUNCTION()
	void OnNPCMovementFinished();
	
};
