// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/Gimmick/PathLinkZone.h"

// Sets default values
APathLinkZone::APathLinkZone()
{
    PrimaryActorTick.bCanEverTick = false;

    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

