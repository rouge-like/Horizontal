#include "Khc/NPC/Component/NPCAStarMovementComponent.h"
#include "Khc/Gimmick/AStarNavigationManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "Khc/NPC/NPCBase.h"
#include "Khc/NPC/Component/NPCFSMComponent.h"
#include "Khc/NPC/Component/NPCInteractionComponent.h"
#include "Kismet/KismetMathLibrary.h"


UNPCAStarMovementComponent::UNPCAStarMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNPCAStarMovementComponent::BeginPlay()
{
    NavigationManager = Cast<AAStarNavigationManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarNavigationManager::StaticClass()));
	//GridManager = Cast<AAStarGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarGridManager::StaticClass()));
    OwnerPawn = Cast<ANPCBase>(GetOwner());
}

void UNPCAStarMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner()->HasAuthority())
    {
        return;
    }
    
    if (bIsMoving)
    {
        // 1. 실제 목표 지점(SafetyZone)을 빨간색 구로 표시S
        DrawDebugSphere(GetWorld(), Destination, 100.f, 12, FColor::Red, false, -1.f, 0, 10.f);

        if (CurrentPath.Num() > 0)
        {
            // 2. A*가 계산한 경로의 최종 도착 지점을 파란색 구로 표시
            DrawDebugSphere(GetWorld(), CurrentPath.Last(), 120.f, 12, FColor::Blue, false, -1.f, 0, 10.f);

            // 3. 현재 따라가야 할 경로를 초록색 선으로 표시
            for (int32 i = CurrentPathIndex; i < CurrentPath.Num() - 1; ++i)
            {
                DrawDebugLine(GetWorld(), CurrentPath[i], CurrentPath[i + 1], FColor::Green, false, -1.f, 0, 5.f);
            }
        }
    }


    // --- 경로 따라가기 로직 ---
    if (bIsMoving && CurrentPath.IsValidIndex(CurrentPathIndex))
    {

        if (!OwnerPawn) return;

        FVector CurrentLocation = OwnerPawn->GetActorLocation();
        FVector Waypoint = CurrentPath[CurrentPathIndex];

        // 1. 도착 판정: 다음 경유지에 충분히 가까워졌는지 확인
        if (FVector::Dist2D(CurrentLocation, Waypoint) < 60.f)
        {
            CurrentPathIndex++;
            // 경로의 마지막에 도달했는지 다시 한번 체크
            if (!CurrentPath.IsValidIndex(CurrentPathIndex))
            {
                // 최종 목적지에 도착했는지 최종 확인
                if (FVector::Dist2D(CurrentLocation, Destination) < 150.f)
                {
                    bIsMoving = false;
                    OnMovementFinished.Broadcast();
                }
                return; // 이번 틱은 여기서 종료
            }
            // 다음 웨이포인트로 목표 갱신
            Waypoint = CurrentPath[CurrentPathIndex];
        }

        FVector Direction = (Waypoint - CurrentLocation).GetSafeNormal();
        OwnerPawn->AddMovementInput(Direction);
    }
    else if(bIsMoving) // 경로가 끝났지만 아직 이동 중 플래그가 켜져 있다면
    {
        if (FVector::Dist2D(GetOwner()->GetActorLocation(), Destination) < 150.f)
        {
            bIsMoving = false;
            OnMovementFinished.Broadcast();
        }
    }

    if (bIsMoving)
    {
        const float StuckVelocityThreshold = 10.0f; 
        const float StuckDistanceToGoal = 500.0f; 

        float CurrentSpeed = OwnerPawn->GetVelocity().Size2D();
        float DistanceToGoal = FVector::Dist2D(OwnerPawn->GetActorLocation(), Destination);

        if (CurrentSpeed < StuckVelocityThreshold && DistanceToGoal < StuckDistanceToGoal)
        {
            UE_LOG(LogTemp, Warning, TEXT("NPC '%s' is stuck near goal. Forcing arrival."), *OwnerPawn->GetName());
            
            // 강제로 도착한 것으로 판정
            bIsMoving = false;
            OnMovementFinished.Broadcast();
        }
    }
}

void UNPCAStarMovementComponent::StartMovingTo(const FVector& NewDestination)
{
    if (!NavigationManager)
    {
        UE_LOG(LogTemp, Error, TEXT("AStarMovementComponent: AStarNavigationManager not found!"));
        return;
    }

    Destination = NewDestination;
    CurrentPathIndex = 0;

    float bPathFound = NavigationManager->FindPath(GetOwner()->GetActorLocation(), Destination, CurrentPath);

    if (bPathFound > 0)
    {
        bIsMoving = true;
        UE_LOG(LogTemp, Log, TEXT("New path found with %d waypoints."), CurrentPath.Num());
    }
    else
    {
        bIsMoving = false;
        UE_LOG(LogTemp, Warning, TEXT("Path to destination could not be found!"));
        auto owner =Cast<ANPCBase>(GetOwner()); 
        owner->FSMComp->SetState(ENPCState::Idle);
        owner->SetReInteractable();
    }
}
