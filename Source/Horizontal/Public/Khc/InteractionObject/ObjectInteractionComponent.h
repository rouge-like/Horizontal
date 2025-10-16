// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Khc/InteractableComponentBase.h"
#include "ObjectInteractionComponent.generated.h"

UENUM(BlueprintType)
enum class EObjectInteractionType : uint8
{
	Obstruction		UMETA(DisplayName = "Obstruction Object"),
	Trigger			UMETA(DisplayName = "Trigger Object"),
	Information		UMETA(DisplayName = "Information Object")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORIZONTAL_API UObjectInteractionComponent : public UInteractableComponentBase
{
	GENERATED_BODY()

public:
	UObjectInteractionComponent();

	virtual void InitiateInteraction(ACharacter* InteractingCharacter) override;

	UPROPERTY(EditAnywhere)
	EObjectInteractionType InteractionType = EObjectInteractionType::Trigger;
};