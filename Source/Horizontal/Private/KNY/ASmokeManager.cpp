// Fill out your copyright notice in the Description page of Project Settings.


#include "KNY/ASmokeManager.h"


// Sets default values
AASmokeManager::AASmokeManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AASmokeManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AASmokeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

