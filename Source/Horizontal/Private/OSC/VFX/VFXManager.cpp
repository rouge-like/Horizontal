// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/VFX/VFXManager.h"

#include "OSC/VFX/VFXActor.h"


// Sets default values
AVFXManager::AVFXManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AVFXManager::BeginPlay()
{
	Super::BeginPlay();

	for (const auto& E : VFXActors)
	{
		InitPool(E.Key);
	}
}

// Called every frame
void AVFXManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AVFXManager::InitPool(FName Name)
{
	if (!VFXActors.Contains(Name)) return;
	if (!IsValid(VFXActors[Name])) return;
	
	FVFXActorPool& Pool = VFXPools.FindOrAdd(Name);

	const int32 ActorsToCreate = FMath::Max(0, InitialPoolSize - Pool.VFXs.Num());

	for (int32 Index = 0; Index < ActorsToCreate; ++Index)
	{
		AVFXActor* VFXActor = GetWorld()->SpawnActor<AVFXActor>(VFXActors[Name], FVector::ZeroVector, FRotator::ZeroRotator);
		if (!IsValid(VFXActor))
		{
			continue;
		}

		VFXActor->SetVFXName(Name);
		VFXActor->SetActorHiddenInGame(true);
		VFXActor->SetActorEnableCollision(false);
		VFXActor->SetActorTickEnabled(false);
		VFXActor->StopVFX();
		Pool.VFXs.Add(VFXActor);
	}
}

AVFXActor* AVFXManager::SpawnVFX(FName Name, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
	if (!VFXActors.Contains(Name)) return nullptr;

	FVFXActorPool& Pool = VFXPools.FindOrAdd(Name);
	bool bAllUsing = true;
	for (int32 Index = 0; Index < Pool.VFXs.Num(); ++Index)
	{
		AVFXActor* VFXActor = Pool.VFXs[Index];
		if (!IsValid(VFXActor))
		{
			continue;
		}

		if (VFXActor->IsHidden())
		{
			VFXActor->SetVFXName(Name);
			VFXActor->SetActorHiddenInGame(false);
			VFXActor->SetActorEnableCollision(false);
			VFXActor->SetActorLocation(Location);
			VFXActor->SetActorRotation(Rotation);
			VFXActor->SetActorScale3D(Scale);
			VFXActor->SetActorTickEnabled(true);
			VFXActor->StartVFX();

			bAllUsing = false;

			return VFXActor;
		}
	}

	if (bAllUsing)
	{
		AVFXActor* VFXActor = GetWorld()->SpawnActor<AVFXActor>(VFXActors[Name], Location, Rotation);
		if (!IsValid(VFXActor))
		{
			return nullptr;
		}

		VFXActor->SetVFXName(Name);
		VFXActor->SetActorScale3D(Scale);
		VFXActor->SetActorHiddenInGame(false);
		VFXActor->SetActorEnableCollision(false);
		VFXActor->SetActorTickEnabled(true);
		VFXActor->StartVFX();

		Pool.VFXs.Add(VFXActor);
		return VFXActor;
	}

	return nullptr;
}

void AVFXManager::DespawnVFX(AVFXActor* VFX)
{
	if (!IsValid(VFX)) return;

	if (VFXActors.Contains(VFX->GetVFXName()))
	{
		VFX->SetActorHiddenInGame(true);
		VFX->SetActorEnableCollision(false);
		VFX->SetActorTickEnabled(false);
		VFX->SetActorScale3D(FVector::OneVector);
		VFX->StopVFX();
	}
	else
	{
		VFX->Destroy();
	}
}

