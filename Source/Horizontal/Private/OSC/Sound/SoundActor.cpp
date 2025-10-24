// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Sound/SoundActor.h"

#include "Components/AudioComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ASoundActor::ASoundActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->bAutoActivate = false;
	SetRootComponent(AudioComponent);
}

void ASoundActor::Init(USoundBase* InitSound)
{
	if (!HasAuthority()) return;
	
	Sound = InitSound;
	OnRep_Sound();
}

void ASoundActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASoundActor, Sound);
}

void ASoundActor::OnRep_Sound()
{
	if (IsValid(Sound))
	{
		AudioComponent->SetSound(Sound);
		AudioComponent->Play();
	}
}


