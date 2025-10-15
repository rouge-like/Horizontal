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
	void UpdateSelectionDialogue(const FDialogueRow& DialogueData);
	void SetDialogueManager(class UDialogueManagerComponent* InManager);

	UFUNCTION(BlueprintCallable)
	void OnChoiceButtonClicked(FName JumpToLabel);
	
protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(EditAnywhere, Category = "Dialogue")
	TSubclassOf<UUserWidget> ChoiceButtonClass;
	
	UPROPERTY()
	TWeakObjectPtr<class UDialogueManagerComponent> DialogueManager;
private:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Dialogue;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_DialogueSpeaker;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* SelectionBox;

	FDialogueRow CurrentDialogueData;
};
