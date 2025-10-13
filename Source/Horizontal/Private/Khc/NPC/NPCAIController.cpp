// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/NPC/NPCAIController.h"


// Sets default values
ANPCAIController::ANPCAIController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANPCAIController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANPCAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

