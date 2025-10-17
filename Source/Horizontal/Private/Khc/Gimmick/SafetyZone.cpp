// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/Gimmick/SafetyZone.h"


// Sets default values
ASafetyZone::ASafetyZone()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASafetyZone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASafetyZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

