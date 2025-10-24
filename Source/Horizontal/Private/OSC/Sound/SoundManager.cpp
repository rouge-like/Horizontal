// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Sound/SoundManager.h"

#include "OSC/Sound/SoundActor.h"

// Sets default values
ASoundManager::ASoundManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASoundManager::BeginPlay()
{
	Super::BeginPlay();

	InitPool();
}

// Called every frame
void ASoundManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASoundManager::InitPool()
{
	const int32 ActorsToCreate = FMath::Max(0, InitialPoolSize - SoundActors.Num());

	for (int32 i = 0; i < ActorsToCreate; i++)
	{
		ASoundActor* SoundActor = GetWorld()->SpawnActor<ASoundActor>(ASoundActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);

		if (!IsValid(SoundActor)) continue;

		SoundActors.Add(SoundActor);
	}
}

ASoundActor* ASoundManager::SpawnSoundAtLocation(FName Name, const FVector& Location)
{
	if (!Sounds.Contains(Name)) return nullptr;
		
	USoundBase* Sound = Sounds[Name];
	
	if (!IsValid(Sound)) return nullptr;
	
	if (SoundActors.IsEmpty())
	{
		return nullptr;
	}
	else
	{
		ASoundActor* SoundActor = SoundActors.Pop();

		SoundActor->SetActorLocation(Location);
		SoundActor->Init(Sound);

		
		const float Duration = Sound->GetDuration();

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUFunction(this, FName("DespawnSound"), SoundActor);
		GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, false);
		
		return SoundActor;
	}
	
}

void ASoundManager::DespawnSound(ASoundActor* SoundActor)
{
	
}

