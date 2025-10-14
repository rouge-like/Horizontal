// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Khc/Dialogue/DialogueDataTypes.h"
#include "Khc/UI/DialogueWidget.h"
#include "DialogueManagerComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORIZONTAL_API UDialogueManagerComponent : public UActorComponent
{
	GENERATED_BODY()
public:    
	UDialogueManagerComponent();

	void RequestAdvanceDialogue();


	// [서버 전용] 새로운 대화를 시작하는 함수
	void StartDialogue(class UNPCInteractionComponent* TargetNPC, FName StartingLabel);

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

private:
	// 에디터에서 설정할 대사 데이터 테이블
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	TObjectPtr<class UDataTable> DialogueTable;

	// 에디터에서 설정할 대화 UI 위젯 클래스
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	TSubclassOf<UDialogueWidget> DialogueWidgetClass;

	// ---- 서버에서만 관리되는 상태 변수들 ----
	UPROPERTY()
	FName CurrentDialogueLabel;

	UPROPERTY()
	TWeakObjectPtr<UNPCInteractionComponent> CurrentInteractingNPC;

	// ---- 클라이언트에서만 존재하는 변수들 ----
	UPROPERTY()
	TObjectPtr<UDialogueWidget> DialogueWidgetInstance;
};
