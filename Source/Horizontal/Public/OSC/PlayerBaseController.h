// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "PlayerBaseController.generated.h"

class UResultUI;
/**
 * 
 */
class UInputMappingContext;
class UDialogueManagerComponent;
UCLASS()
class HORIZONTAL_API APlayerBaseController : public APlayerController
{
	GENERATED_BODY()
	
	APlayerBaseController();

protected:
	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;
	
	/** Gameplay Initialization */
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	/** Possessed pawn initialization */
	virtual void OnPossess(APawn* aPawn) override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UDialogueManagerComponent* DialogueManagerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Result")
	TSubclassOf<UResultUI> ResultUIClass;

	UPROPERTY()
	UResultUI* ResultUI;
	
public:
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerChangeToSpectator();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void ClientShowResultUI();

	TArray<TSharedRef<const FUniqueNetId>> Talkers;

	UFUNCTION(Server, Reliable)
	void Server_RequestWakeUp();

	bool bVoiceChatInitialized = false;
};
