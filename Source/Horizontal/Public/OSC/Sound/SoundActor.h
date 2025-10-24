// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SoundActor.generated.h"

UCLASS()
class HORIZONTAL_API ASoundActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASoundActor();

	UPROPERTY(ReplicatedUsing=OnRep_Sound)
	TObjectPtr<USoundBase> Sound;
	
	void Init(USoundBase* InitSound);
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditAnywhere)
	UAudioComponent* AudioComponent;
	
	UFUNCTION()
	void OnRep_Sound();
};
 