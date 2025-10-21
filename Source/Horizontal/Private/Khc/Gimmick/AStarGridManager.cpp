// Fill out your copyright notice in the Description page of Project Settings.

#include "Khc/Gimmick/AStarGridManager.h"
#include "Khc/Gimmick/WeightZone.h"
#include "Khc/Gimmick/SafetyZone.h"
#include <Kismet/GameplayStatics.h>

#include "Khc/Gimmick/AStarNavigationManager.h"
#include "Khc/InteractionObject/InteractableObstacleObjectBase.h"

// Sets default values
AAStarGridManager::AAStarGridManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

bool AAStarGridManager::FindPath(FVector StartLocation, FVector TargetLocation, TArray<FVector>& OutPath)
{
    FPathNode* StartNode = GetNodeFromWorldLocation(StartLocation);
    FPathNode* TargetNode = GetNodeFromWorldLocation(TargetLocation);

    // 목표 노드가 유효하면, 장애물이 아니라고 강제 설정
    if (TargetNode)
    {
        TargetNode->bIsObstacle = false;
    }

    // 시작 또는 목표 노드가 그리드 범위를 벗어났으면 길찾기 실패
    if (!StartNode || !TargetNode)
    {
        return false;
    }

    // 모든 노드의 비용 정보 초기화
    for (FPathNode& Node : Grid)
    {
        Node.GCost = BIG_NUMBER;
        Node.HCost = 0.f;
        Node.ParentNode = nullptr;
    }
    StartNode->GCost = 0;
    StartNode->HCost = CalculateDistance(StartNode, TargetNode);

    TArray<FPathNode*> OpenList; // 탐색할 후보 노드 목록
    TSet<FPathNode*> ClosedSet;  // 탐색이 완료된 노드 목록
    OpenList.Add(StartNode);

    while (OpenList.Num() > 0)
    {
        // OpenList에서 F Cost가 가장 낮은 노드를 CurrentNode로 선택
        FPathNode* CurrentNode = OpenList[0];
        for (int32 i = 1; i < OpenList.Num(); ++i)
        {
            if (OpenList[i]->GetFCost() < CurrentNode->GetFCost() || (OpenList[i]->GetFCost() == CurrentNode->GetFCost() && OpenList[i]->HCost < CurrentNode->HCost))
            {
                CurrentNode = OpenList[i];
            }
        }

        OpenList.Remove(CurrentNode);
        ClosedSet.Add(CurrentNode);

        // 현재 노드가 목표 지점이면 경로를 생성하고 성공 반환
        if (CurrentNode == TargetNode)
        {
            RetracePath(StartNode, TargetNode, OutPath);
            return true;
        }

        TArray<FPathNode*> Neighbors;
        GetNeighborNodes(CurrentNode, Neighbors);

        for (FPathNode* Neighbor : Neighbors)
        {
            // 이웃이 장애물이거나 이미 탐색 완료된 노드면 건너뜀
            if (Neighbor->bIsObstacle || ClosedSet.Contains(Neighbor))
            {
                continue;
            }

            // 현재 노드를 거쳐 이웃 노드로 가는 새로운 G Cost 계산 (가중치 적용)
            float NewCostToNeighbor = CurrentNode->GCost + CalculateDistance(CurrentNode, Neighbor) * Neighbor->MovementWeight;

            // 새로운 경로가 더 저렴하거나, 아직 OpenList에 없는 노드라면 정보 갱신
            if (NewCostToNeighbor < Neighbor->GCost || !OpenList.Contains(Neighbor))
            {
                Neighbor->GCost = NewCostToNeighbor;
                Neighbor->HCost = CalculateDistance(Neighbor, TargetNode);
                Neighbor->ParentNode = CurrentNode; // 경로 추적을 위해 부모 노드 기록

                if (!OpenList.Contains(Neighbor))
                {
                    OpenList.Add(Neighbor);
                }
            }
        }
    }
    return false; // OpenList가 비었는데 목표를 못 찾았으면 길찾기 실패
}

void AAStarGridManager::BlurObstaclePenalties(int32 BlurSize)
{
    int32 KernelSize = BlurSize * 2 + 1;
    int32 KernelExtents = BlurSize;

    // 원본 그리드의 장애물 정보만 복사
    TArray<float> OriginalWeights;
    OriginalWeights.SetNum(Grid.Num());
    for(int i = 0; i < Grid.Num(); ++i)
    {
       OriginalWeights[i] = Grid[i].bIsObstacle ? 1.0f : 0.0f;
    }

    // 모든 노드를 순회하며 주변 장애물에 따라 가중치 증가
    for (int32 Y = 0; Y < GridSizeY; ++Y)
    {
       for (int32 X = 0; X < GridSizeX; ++X)
       {
          if (Grid[Y * GridSizeX + X].bIsObstacle) continue;

          float Penalty = 0;
          for (int32 BlurY = -KernelExtents; BlurY <= KernelExtents; ++BlurY)
          {
             for (int32 BlurX = -KernelExtents; BlurX <= KernelExtents; ++BlurX)
             {
                int32 SampleX = X + BlurX;
                int32 SampleY = Y + BlurY;

                if (SampleX >= 0 && SampleX < GridSizeX && SampleY >= 0 && SampleY < GridSizeY)
                {
                   if (OriginalWeights[SampleY * GridSizeX + SampleX] > 0) // 주변에 장애물이 있다면
                   {
                      // 거리가 멀수록 페널티를 약하게 부여
                      float Distance = FMath::Sqrt(float(BlurX * BlurX + BlurY * BlurY));
                      Penalty = FMath::Max(Penalty, KernelExtents - Distance);
                   }
                }
             }
          }
          // 기존 가중치에 계산된 페널티 추가
          Grid[Y * GridSizeX + X].MovementWeight += Penalty * 25.0f;
       }
    }
}
//
// void AAStarGridManager::UpdateNodesInBounds(const FBox& BoundsToUpdate)
// {
// }

void AAStarGridManager::RebuildGrid()
{
    if (!HasAuthority()) return;

    CreateGrid();
}

void AAStarGridManager::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;

    CreateGrid();

    // TArray<AActor*> FoundObstacles;
    // UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteractableObstacleObjectBase::StaticClass(), FoundObstacles);
    // for (AActor* Actor : FoundObstacles)
    // {
    //     AInteractableObstacleObjectBase* Obstacle = Cast<AInteractableObstacleObjectBase>(Actor);
    //     if (Obstacle)
    //     {
    //         // 장애물의 상태가 바뀔 때마다 UpdateNodesInBounds 함수를 호출하도록 연결(바인딩)합니다.
    //         Obstacle->OnStateChanged.AddDynamic(this, &AAStarGridManager::UpdateNodesInBounds);
    //     }
    // }
}

void AAStarGridManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDebugDrawGrid && bGridReady)
    {
        const float MaxWeight = 10.0f; // 최대 가중치 값 (색상 보간용)

        for (const FPathNode& Node : Grid)
        {
            FColor NodeColor;
            
            if (Node.LinkedActor) // <-- 이 if문 추가
            {
                NodeColor = FColor::Magenta; // 환승역 노드는 마젠타(자홍색)으로 표시
            }
            else if (Node.bIsObstacle)
            {
                NodeColor = FColor(100, 0, 0); // 장애물 노드는 어두운 빨간색
            }
            else
            {
                // 가중치에 따라 노란색에서 빨간색으로 색상 보간
                // 1.0에 가까우면 노란색, MaxWeight에 가까우면 빨간색
                float LerpAlpha = FMath::Clamp((Node.MovementWeight - 1.0f) / FMath::Max(MaxWeight - 1.0f, 1.0f), 0.0f, 1.0f);
                NodeColor = FLinearColor::LerpUsingHSV(FLinearColor::Yellow, FLinearColor::Red, LerpAlpha).ToFColor(true);
            }

            DrawDebugSphere(GetWorld(), Node.WorldLocation, NodeRadius * 0.2f, 6, NodeColor, false, -1.f, 0, 1.f);
        }
    }
}

void AAStarGridManager::CreateGrid()
{
    // NodeDiameter = NodeRadius * 2;
    // GridSizeX = FMath::RoundToInt(GridWorldSize.X / NodeDiameter);
    // GridSizeY = FMath::RoundToInt(GridWorldSize.Y / NodeDiameter);
    // Grid.SetNum(GridSizeX * GridSizeY);
    //
    // FVector WorldBottomLeft = GetActorLocation() - FVector::ForwardVector * GridWorldSize.X / 2 - FVector::RightVector * GridWorldSize.Y / 2;
    //
    // TArray<AActor*> FoundActors;
    // UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWeightZone::StaticClass(), FoundActors);
    // TArray<AWeightZone*> WeightActors;
    // for (AActor* Actor : FoundActors)
    // {
    //     WeightActors.Add(Cast<AWeightZone>(Actor));
    // }
    //
    // TArray<AActor*> FoundLinks;
    // UGameplayStatics::GetAllActorsOfClass(GetWorld(), APathLinkZone::StaticClass(), FoundLinks);
    // TArray<APathLinkZone*> NavLinks;
    // for (AActor* Actor : FoundLinks)
    // {
    //     NavLinks.Add(Cast<APathLinkZone>(Actor));
    // }
    //
    // for (int32 Y = 0; Y < GridSizeY; ++Y)
    // {
    //     for (int32 X = 0; X < GridSizeX; ++X)
    //     {
    //         int32 Index = Y * GridSizeX + X;
    //         FVector WorldPoint = WorldBottomLeft
    //             + FVector::ForwardVector * (X * NodeDiameter + NodeRadius)
    //             + FVector::RightVector * (Y * NodeDiameter + NodeRadius);
    //
    //         WorldPoint.Z = GetActorLocation().Z;
    //         
    //         Grid[Index].WorldLocation = WorldPoint;
    //         Grid[Index].GridIndex = FIntPoint(X, Y);
    //
    //         for (APathLinkZone* Link : NavLinks)
    //         {
    //             if (Link && FVector::DistSquared(Grid[Index].WorldLocation, Link->GetActorLocation()) < FMath::Square(NodeRadius))
    //             {
    //                 Grid[Index].LinkedActor = Link;
    //                 Grid[Index].bIsObstacle = false; // 링크 지점은 장애물이 아니라고 보장
    //                 break;
    //             }
    //         }
    //
    //         FCollisionQueryParams QueryParams;
    //         QueryParams.AddIgnoredActor(this);
    //
    //         APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    //         if (PlayerPawn)
    //         {
    //             QueryParams.AddIgnoredActor(PlayerPawn); // 플레이어 폰을 무시 목록에 추가
    //         }
    //
    //         AActor* SafeZoneActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASafetyZone::StaticClass());
    //         if (SafeZoneActor)
    //         {
    //             QueryParams.AddIgnoredActor(SafeZoneActor); // SafetyZone도 무시 목록에 추가
    //         }
    //
    //         // 장애물 검사
    //         bool bHit = GetWorld()->OverlapAnyTestByObjectType(WorldPoint, FQuat::Identity, FCollisionObjectQueryParams(ObstacleObjectTypes), FCollisionShape::MakeSphere(NodeRadius), QueryParams);
    //         Grid[Index].bIsObstacle = bHit;
    //
    //         if (!bHit)
    //         {
    //             // 가중치 적용
    //             for (AWeightZone* WeightActor : WeightActors)
    //             {
    //                 if (WeightActor && WeightActor->GetWeightBox().IsInside(WorldPoint))
    //                 {
    //                     Grid[Index].MovementWeight = WeightActor->MovementWeight;
    //                     break; // 첫 번째로 찾은 가중치를 적용하고 중단
    //                 }
    //             }
    //         }
    //     }
    // }
    //
    // BlurObstaclePenalties(blurSize);
    // bGridReady = true;
    // UE_LOG(LogTemp, Log, TEXT("A* Grid is ready."));

    NodeDiameter = NodeRadius * 2;
    GridSizeX = FMath::RoundToInt(GridWorldSize.X / NodeDiameter);
    GridSizeY = FMath::RoundToInt(GridWorldSize.Y / NodeDiameter);
    Grid.SetNum(GridSizeX * GridSizeY);

    FVector WorldBottomLeft = GetActorLocation() - FVector::ForwardVector * GridWorldSize.X / 2 - FVector::RightVector * GridWorldSize.Y / 2;

    // --- 1. 정보 수집 (루프 시작 전 한 번만 실행) ---
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWeightZone::StaticClass(), FoundActors);
    TArray<AWeightZone*> WeightActors;
    for (AActor* Actor : FoundActors)
    {
        WeightActors.Add(Cast<AWeightZone>(Actor));
    }


    const float MyZ = GetActorLocation().Z; // 이 그리드 매니저의 Z 위치
    const float ZThreshold = 50.0f;
    
    TArray<AActor*> FoundLinks;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APathLinkZone::StaticClass(), FoundLinks);
    TArray<APathLinkZone*> NavLinks;
    for (AActor* Actor : FoundLinks)
    {
        if (FMath::Abs(Actor->GetActorLocation().Z - MyZ) < ZThreshold)
        {
            // 3. 같은 층에 있는 PathLink만 NavLinks 배열에 추가
            NavLinks.Add(Cast<APathLinkZone>(Actor));
        }
    }

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        QueryParams.AddIgnoredActor(PlayerPawn);
    }
    AActor* SafeZoneActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASafetyZone::StaticClass());
    if (SafeZoneActor)
    {
        QueryParams.AddIgnoredActor(SafeZoneActor);
    }
    // ------------------------------------------------

    // --- 2. 그리드 노드 기본 정보 생성 (장애물 & 가중치) ---
    for (int32 Y = 0; Y < GridSizeY; ++Y)
    {
        for (int32 X = 0; X < GridSizeX; ++X)
        {
            int32 Index = Y * GridSizeX + X;
            FPathNode& Node = Grid[Index]; // 참조(&)로 직접 수정

            FVector WorldPoint = WorldBottomLeft
                + FVector::ForwardVector * (X * NodeDiameter + NodeRadius)
                + FVector::RightVector * (Y * NodeDiameter + NodeRadius);

            WorldPoint.Z = GetActorLocation().Z;
            
            Node.WorldLocation = WorldPoint;
            Node.GridIndex = FIntPoint(X, Y);
            Node.LinkedActor = nullptr; // 일단 초기화
            Node.MovementWeight = 1.0f; // 기본 가중치로 초기화

            // 장애물 검사
            bool bHit = GetWorld()->OverlapAnyTestByObjectType(WorldPoint, FQuat::Identity, FCollisionObjectQueryParams(ObstacleObjectTypes), FCollisionShape::MakeSphere(NodeRadius), QueryParams);
            Node.bIsObstacle = bHit;

            // 가중치 적용
            if (!bHit)
            {
                for (AWeightZone* WeightActor : WeightActors)
                {
                    if (WeightActor && WeightActor->GetWeightBox().IsInside(WorldPoint))
                    {
                        Node.MovementWeight = WeightActor->MovementWeight;
                        break;
                    }
                }
            }
        }
    }

    // --- 3. PathLinkZone 연결 (요청하신 부분) ---
    // 모든 노드를 만든 '후에' 각 PathLink와 가장 가까운 노드 딱 1개만 찾습니다.
    for (APathLinkZone* Link : NavLinks)
    {
        if (!Link) continue;

        float MinDistSq = BIG_NUMBER;
        FPathNode* ClosestNode = nullptr;

        for (FPathNode& Node : Grid)
        {
            if (Node.bIsObstacle) continue; // 장애물 노드는 제외

            float DistSq = FVector::DistSquared(Node.WorldLocation, Link->GetActorLocation());
            if (DistSq < MinDistSq)
            {
                MinDistSq = DistSq;
                ClosestNode = &Node;
            }
        }

        // 가장 가까운 노드를 찾았으면, 해당 노드를 '환승역'으로 지정
        if (ClosestNode)
        {
            ClosestNode->LinkedActor = Link;
            ClosestNode->bIsObstacle = false; // 링크 지점은 확실하게 걸을 수 있도록 보장
        }
    }
    // ------------------------------------------

    BlurObstaclePenalties(3);
    bGridReady = true;
    UE_LOG(LogTemp, Log, TEXT("A* Grid is ready."));
}

FPathNode* AAStarGridManager::GetNodeFromWorldLocation(FVector WorldLocation)
{
    // 그리드의 좌하단 기준 위치 계산
    FVector WorldBottomLeft = GetActorLocation() - FVector::ForwardVector * GridWorldSize.X / 2 - FVector::RightVector * GridWorldSize.Y / 2;

    // 월드 위치가 그리드 범위 밖이면 nullptr 반환
    if (WorldLocation.X < WorldBottomLeft.X || WorldLocation.X >(WorldBottomLeft.X + GridWorldSize.X) ||
        WorldLocation.Y < WorldBottomLeft.Y || WorldLocation.Y >(WorldBottomLeft.Y + GridWorldSize.Y))
    {
        return nullptr;
    }

    // 그리드 내 퍼센트 위치 계산
    float PercentX = (WorldLocation.X - WorldBottomLeft.X) / GridWorldSize.X;
    float PercentY = (WorldLocation.Y - WorldBottomLeft.Y) / GridWorldSize.Y;

    // 퍼센트 위치를 그리드 인덱스로 변환
    int32 X = FMath::Clamp(FMath::FloorToInt(GridSizeX * PercentX), 0, GridSizeX - 1);
    int32 Y = FMath::Clamp(FMath::FloorToInt(GridSizeY * PercentY), 0, GridSizeY - 1);

    int32 Index = Y * GridSizeX + X;
    return &Grid[Index];
}

void AAStarGridManager::GetNeighborNodes(const FPathNode* Node, TArray<FPathNode*>& OutNeighbors)
{
    OutNeighbors.Empty();

    // 주변 8방향 탐색
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            if (x == 0 && y == 0) continue; // 자기 자신은 제외

            int checkX = Node->GridIndex.X + x;
            int checkY = Node->GridIndex.Y + y;

            if (checkX >= 0 && checkX < GridSizeX && checkY >= 0 && checkY < GridSizeY) // 그리드 범위 안에 있는지 확인
            {
                int32 Index = checkY * GridSizeX + checkX;
                OutNeighbors.Add(&Grid[Index]);
            }
        }
    }

    if (Node->LinkedActor && Node->LinkedActor->TargetPoint)
    {
        // NavigationManager
        AAStarNavigationManager* NavManager = Cast<AAStarNavigationManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarNavigationManager::StaticClass()));
        if (NavManager)
        {
            FVector TargetLocation = Node->LinkedActor->TargetPoint->GetActorLocation();
            
            // 목표 지점에 맞는 다른 GridManager 탐색
            AAStarGridManager* TargetManager = NavManager->GetManagerForLocation(TargetLocation);

            if (TargetManager && TargetManager != this) // 다른 매니저가 확실하다면
            {
                // 다른 매니저에게 목표 위치의 노드를 달라고 요청
                FPathNode* TargetLinkNode = TargetManager->GetNodeFromWorldLocation(TargetLocation);
                if (TargetLinkNode)
                {
                    OutNeighbors.Add(TargetLinkNode);
                }
            }
        }
    }
}

float AAStarGridManager::CalculateDistance(const FPathNode* A, const FPathNode* B)
{
    int32 DstX = FMath::Abs(A->GridIndex.X - B->GridIndex.X);
    int32 DstY = FMath::Abs(A->GridIndex.Y - B->GridIndex.Y);

    // 직선 이동 비용: 10, 대각선 이동 비용: 14 (루트2의 근사치)
    if (DstX > DstY)
        return 14 * DstY + 10 * (DstX - DstY);

    return 14 * DstX + 10 * (DstY - DstX);
}

void AAStarGridManager::RetracePath(const FPathNode* StartNode, const FPathNode* EndNode, TArray<FVector>& OutPath)
{
    OutPath.Empty();
    const FPathNode* CurrentNode = EndNode;

    TArray<FVector> TempPath;
    // 목표 노드부터 부모 노드를 따라가며 경로 저장
    while (CurrentNode != StartNode && CurrentNode != nullptr)
    {
        TempPath.Add(CurrentNode->WorldLocation);
        CurrentNode = CurrentNode->ParentNode;
    }

    Algo::Reverse(TempPath); // 역순으로 저장된 경로 뒤집기

    // SimplifyPath를 호출하기 전에 경로를 OutPath에 먼저 복사
    OutPath = TempPath; 
    
    // 경로 평탄화 (SimplifyPath가 OutPath를 직접 수정하도록 함)
    // SimplifyPath(TempPath, OutPath);
}

void AAStarGridManager::SimplifyPath(const TArray<FVector>& InPath, TArray<FVector>& OutPath)
{
    OutPath.Empty();
    if (InPath.Num() < 2)
    {
        OutPath = InPath;
        return;
    }

    OutPath.Add(InPath[0]); // 시작점은 항상 포함
    FVector LastDirection = (InPath[1] - InPath[0]).GetSafeNormal();

    for (int32 i = 1; i < InPath.Num() - 1; ++i)
    {
        FVector CurrentDirection = (InPath[i + 1] - InPath[i]).GetSafeNormal();
        // 이전 방향과 현재 방향이 다르면 코너로 간주하고 경로에 추가
        if (!LastDirection.Equals(CurrentDirection, 0.1f))
        {
            OutPath.Add(InPath[i]);
        }
        LastDirection = CurrentDirection;
    }

    OutPath.Add(InPath.Last()); // 끝점은 항상 포함
}