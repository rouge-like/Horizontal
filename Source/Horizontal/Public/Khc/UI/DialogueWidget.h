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

	void ShowImage(uint32 imgNum, float Width = 0.0f, float Height = 0.0f);
	void HideImage();

	UFUNCTION()
	void OnMediaOpenSuccess(FString FilePath);

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeConstruct() override;

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

	UPROPERTY(meta = (BindWidget))
	class UImage* ExampleImg;

	// UPROPERTY(meta = (BindWidget))
	// class USizeBox* ImageSizeBox;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets", meta = (AllowedClasses = "Texture2D, MediaSource"))
	TArray<TObjectPtr<UObject>> DisplayableAssets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Video")
	class UMediaPlayer* MediaPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Video")
	class UMediaTexture* MediaTexture;
};
