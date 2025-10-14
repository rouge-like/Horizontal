// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/InteractionObject/InteractableObjectBase.h"
#include "Components/WidgetComponent.h"
#include "Khc/InteractionObject/ObjectInteractionComponent.h"
#include "Components/StaticMeshComponent.h"



// Sets default values
AInteractableObjectBase::AInteractableObjectBase()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// 기본적으로 플레이어의 라인 트레이스(Visibility 채널)에 감지되도록 콜리전 설정
	MeshComponent->SetCollisionObjectType(ECC_WorldStatic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

	// 2. 상호작용 컴포넌트 생성 (논리적인 컴포넌트라 Attachment 불필요)
	InteractionComponent = CreateDefaultSubobject<UObjectInteractionComponent>(TEXT("InteractionComponent"));

	// 3. UI 위젯 컴포넌트 생성 및 설정
	InteractionUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionUI"));
	InteractionUI->SetupAttachment(RootComponent); // 메시 컴포넌트에 부착
	InteractionUI->SetWidgetSpace(EWidgetSpace::Screen); // 항상 화면을 바라보도록 설정
	InteractionUI->SetVisibility(false); // 기본적으로 숨겨둠
}

// Called when the game starts or when spawned
void AInteractableObjectBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInteractableObjectBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

