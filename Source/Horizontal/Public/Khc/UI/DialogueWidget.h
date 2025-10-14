// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Khc/Dialogue/DialogueDataTypes.h"
#include "DialogueWidget.generated.h"

/**
 * 
 */
UCLASS()
class HORIZONTAL_API UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void UpdateDialogue(const FDialogueRow& DialogueData);
	void SetDialogueManager(class UDialogueManagerComponent* InManager);

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	UPROPERTY()
	TWeakObjectPtr<class UDialogueManagerComponent> DialogueManager;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Dialogue;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_DialogueSpeaker;
};
