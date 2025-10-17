#include "Khc/NPC/Component/NPCAStarMovementComponent.h"
#include "Khc/Gimmick/AStarGridManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"

UNPCAStarMovementComponent::UNPCAStarMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNPCAStarMovementComponent::BeginPlay()
{

	GridManager = Cast<AAStarGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarGridManager::StaticClass()));
}

void UNPCAStarMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
    if (!bIsMoving || !GridManager || !GridManager->IsGridReady())
    {
        return; // 아직 준비 안됐으면 아무것도 하지 않음
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
    if (CurrentPath.IsValidIndex(CurrentPathIndex))
    {
        FVector Waypoint = CurrentPath[CurrentPathIndex];
        FVector OwnerLocation = GetOwner()->GetActorLocation();

        if (FVector::Dist2D(OwnerLocation, Waypoint) < 150.f)
        {
            CurrentPathIndex++;
        }
        else
        {
            FVector Direction = (Waypoint - OwnerLocation).GetSafeNormal();
            if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
            {
                OwnerPawn->AddMovementInput(Direction);
            }
        }
    }
    else // 경로의 마지막 지점까지 모두 통과했을 때
    {
        // 도착 판정: 최종 목적지와의 실제 거리를 확인
        if (FVector::Dist2D(GetOwner()->GetActorLocation(), Destination) < 150.f)
        {
            bIsMoving = false; // 이동 중지
            OnMovementFinished.Broadcast(); // 이동 완료 신호 방송
        }
        // 경로가 끝났는데도 목적지와 멀리 떨어져 있는 경우는
        // 경로 자체가 잘못되었을 가능성이 높으므로, 일단 이동을 멈추고 로그
        else
        {
            bIsMoving = false;
            UE_LOG(LogTemp, Warning, TEXT("Path finished, but far from final destination. Stopping movement."));
        }
    }
}

void UNPCAStarMovementComponent::StartMovingTo(const FVector& NewDestination)
{
    if (!GridManager || !GridManager->IsGridReady())
    {
        UE_LOG(LogTemp, Warning, TEXT("AStarMovementComponent: GridManager not ready."));
        return;
    }

    Destination = NewDestination;
    CurrentPathIndex = 0;

    // 이동 시작 시 경로를 한 번만 계산
    bool bPathFound = GridManager->FindPath(GetOwner()->GetActorLocation(), Destination, CurrentPath);

    if (bPathFound)
    {
        bIsMoving = true; // 경로를 찾았을 때만 이동 시작
        UE_LOG(LogTemp, Log, TEXT("New path found with %d waypoints."), CurrentPath.Num());
    }
    else
    {
        bIsMoving = false; // 경로를 못 찾았으면 이동하지 않음
        UE_LOG(LogTemp, Warning, TEXT("Path to destination could not be found!"));
    }
}
