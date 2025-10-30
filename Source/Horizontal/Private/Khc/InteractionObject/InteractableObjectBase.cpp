// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/InteractionObject/InteractableObjectBase.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Khc/InteractionObject/ObjectInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Khc/Player/DialogueManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "OSC/Sound/SoundManager.h"

AInteractableObjectBase::AInteractableObjectBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	InteractionComponent = CreateDefaultSubobject<UObjectInteractionComponent>(TEXT("InteractionComponent"));
	InteractionComponent->SphereComp->SetSphereRadius(100.f);
	RootComponent = InteractionComponent->SphereComp;
	InteractionComponent->InteractionUI->SetupAttachment(RootComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	MeshComponent->SetCollisionObjectType(ECC_WorldStatic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
}
void AInteractableObjectBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInteractableObjectBase, bHasBeenInteractedWith);
}

void AInteractableObjectBase::BindToPlayerController(APlayerController* PC)
{
	if (PC)
	{
		UDialogueManagerComponent* DialogueManager = PC->FindComponentByClass<UDialogueManagerComponent>();
		if (DialogueManager)
		{
			DialogueManager->OnDialogueEvent.AddDynamic(this, &AInteractableObjectBase::OnDialogueEventReceived);
		}
	}
}

void AInteractableObjectBase::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractableObjectBase::OnDialogueEventReceived(FName EventTag, AActor* InteractableActor)
{
	if (InteractableActor != this)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("'%s' received EventTag: %s"), *GetName(), *EventTag.ToString());

	ASoundManager* SoundManager = Cast<ASoundManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ASoundManager::StaticClass()));
	
	// 1. Obstruction 일때 EndGood -> 파괴
	if (EventTag == "DestroyObstacle")
	{
		//Destroy();

		ObstacleTargetLocation = FVector(-252.000000, 54.000000, 368.000000);
		ObstacleTargetRotation = FRotator(90.000000, 90.000000, 0.000001);
       
		bObstacleMove = true; 
       
		OnRep_ObstacleMove();
		SoundManager->SpawnSoundAtLocation(FName(TEXT("Obstacle")), GetActorLocation());
		//(X=-252.000000,Y=237.000000,Z=420.000000)
		//(Pitch=-39.999999,Yaw=90.000000,Roll=0.000001)
		//(X=-252.000000,Y=54.000000,Z=368.000000)
		//(Pitch=90.000000,Yaw=90.000000,Roll=0.000000)
	}
	else if (EventTag == "ResetInteraction")
	{
		if (InteractionComponent)
		{
			InteractionComponent->SetInteractable(true);
		}
	}
	else if (EventTag == "DeactivateTrigger")
	{
		UE_LOG(LogTemp, Log, TEXT("Trigger '%s' has been deactivated."), *GetName());
	}
	else if (EventTag == "InfoReset")
	{
		if (InteractionComponent)
		{
			InteractionComponent->SetInteractable(true);
		}
	}
	else if (EventTag == "OpenDoor" || EventTag == "OpenDoorBad")
	{
		bIsMove = true;
		originRotYaw = GetActorRotation().Yaw;
		SoundManager->SpawnSoundAtLocation(FName(TEXT("DoorOpen")), GetActorLocation());

		OnRep_IsMove();
	}
	else if (EventTag == "OpenMainDoor")
	{
		bMainDoorMove = true;
		SoundManager->SpawnSoundAtLocation(FName(TEXT("MainDoorButton")), GetActorLocation());
		SoundManager->SpawnSoundAtLocation(FName(TEXT("MainDoorOpen")), GetActorLocation());
		
		OnRep_MainDoorMove();
	}
	else if (EventTag == "EmergencyBell")
	{
		USoundBase* BGMSound = LoadObject<USoundBase>(nullptr, TEXT("/Script/Engine.SoundCue'/Game/khc/SFX/Emergency_Cue.Emergency_Cue'"));

		if (BGMSound && SoundManager)
		{
			// 2. [서버] 멀티캐스트 RPC를 호출하여, 로드한 사운드 애셋을 모든 클라이언트에게 전달
			Multicast_PlayEmergencyBGM(BGMSound);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("EmergencyBell BGM asset NOT found at hardcoded path!"));
		}	
	}
}

void AInteractableObjectBase::OnRep_IsMove()
{
	if (bIsMove)
	{
		PrimaryActorTick.bCanEverTick = true;
	}
}

void AInteractableObjectBase::OnRep_ObstacleMove()
{
	if (bObstacleMove)
	{
		PrimaryActorTick.bCanEverTick = true;
	}
}

void AInteractableObjectBase::OnRep_MainDoorMove()
{
	if (bMainDoorMove)
	{
		PrimaryActorTick.bCanEverTick = true;
	}
}

void AInteractableObjectBase::Multicast_PlayEmergencyBGM_Implementation(USoundBase* SoundToPlay)
{
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), SoundToPlay, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true);
	}
}

void AInteractableObjectBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsMove)
	{
		const float TargetYaw = originRotYaw + (-140.0f);
		float CurrentYaw = GetActorRotation().Yaw;
		const float RotationSpeed = 100.0f; 
		float NewYaw = FMath::FInterpConstantTo(CurrentYaw, TargetYaw, DeltaTime, RotationSpeed);

		SetActorRotation(FRotator(0.0f, NewYaw, 0.0f));

		if (FMath::IsNearlyEqual(NewYaw, TargetYaw))
		{
			bIsMove = false; // 이동 완료
		}
	}

	if (bObstacleMove)
	{
		const float InterpSpeed = 5.0f; 
        
		// 액터 전체의 위치와 회전을 부드럽게 보간
		FTransform CurrentTransform = GetActorTransform();
		FVector NewLocation = FMath::VInterpTo(CurrentTransform.GetLocation(), ObstacleTargetLocation, DeltaTime, InterpSpeed);
		FRotator NewRotation = FMath::RInterpTo(CurrentTransform.GetRotation().Rotator(), ObstacleTargetRotation, DeltaTime, InterpSpeed);

		// 액터의 트랜스폼을 업데이트 (충돌 무시)
		SetActorTransform(FTransform(NewRotation, NewLocation), false, nullptr, ETeleportType::TeleportPhysics);

		// 목표 지점에 거의 도달했는지 확인
		if (CurrentTransform.GetLocation().Equals(ObstacleTargetLocation, 1.0f))
		{
			bObstacleMove = false; // 이동 완료
		}
	}

	if (bMainDoorMove)
	{
		if (MainDoor == nullptr)
		{
			bMainDoorMove = false;
			return;
		}
		float CurrentZ = MainDoor->GetActorLocation().Z;
		float TargetZ = 550.f;
		
		float NewZ = FMath::FInterpConstantTo(CurrentZ, TargetZ, DeltaTime, 20);

		MainDoor->SetActorLocation(FVector(374.0, 1544.0, NewZ));

		if (FMath::IsNearlyEqual(NewZ, TargetZ))
		{
			bMainDoorMove = false; // 이동 완료
		}
	}

	if (!bIsMove && !bObstacleMove && !bMainDoorMove)
	{
		PrimaryActorTick.bCanEverTick = false;
	}
}
