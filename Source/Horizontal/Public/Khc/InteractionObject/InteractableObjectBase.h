// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableObjectBase.generated.h"

UCLASS()
class HORIZONTAL_API AInteractableObjectBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInteractableObjectBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	bool bHasBeenInteractedWith;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnDialogueEventReceived(FName EventTag, AActor* InteractableActor);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 상호작용 가능 UI를 표시할 위젯 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UWidgetComponent> InteractionUI;

protected:
	// 블루프린트에서 외형을 바꿀 수 있는 스태틱 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> MeshComponent;

	// 상호작용 로직을 담당하는 범용 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UObjectInteractionComponent> InteractionComponent;


};
