// Fill out your copyright notice in the Description page of Project Settings.


#include "Khc/Gimmick/AStarGridManager.h"
#include "Khc/Gimmick/WeightZone.h"
#include "Khc/Gimmick/SafetyZone.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AAStarGridManager::AAStarGridManager()
{
	PrimaryActorTick.bCanEverTick = true;

}

bool AAStarGridManager::FindPath(FVector StartLocation, FVector TargetLocation, TArray<FVector>& OutPath)
{
    FPathNode* StartNode = GetNodeFromWorldLocation(StartLocation);
    FPathNode* TargetNode = GetNodeFromWorldLocation(TargetLocation);

    if (TargetNode)
    {
        TargetNode->bIsObstacle = false;
    }

    if (!StartNode || !TargetNode)
    {
        return false;
    }

    for (FPathNode& Node : Grid)
    {
        Node.GCost = BIG_NUMBER;
        Node.HCost = 0.f;
        Node.ParentNode = nullptr;
    }
    StartNode->GCost = 0;
    StartNode->HCost = CalculateDistance(StartNode, TargetNode);


    TArray<FPathNode*> OpenList;
    TSet<FPathNode*> ClosedSet;
    OpenList.Add(StartNode);

    while (OpenList.Num() > 0)
    {
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

        if (CurrentNode == TargetNode)
        {
            RetracePath(StartNode, TargetNode, OutPath);
            return true;
        }

        TArray<FPathNode*> Neighbors;
        GetNeighborNodes(CurrentNode, Neighbors);

        for (FPathNode* Neighbor : Neighbors)
        {
            if (Neighbor->bIsObstacle || ClosedSet.Contains(Neighbor))
            {
                continue;
            }

            // 가중치를 사용한 비용 계산
            float NewCostToNeighbor = CurrentNode->GCost + CalculateDistance(CurrentNode, Neighbor) * Neighbor->MovementWeight;

            if (NewCostToNeighbor < Neighbor->GCost || !OpenList.Contains(Neighbor))
            {
                Neighbor->GCost = NewCostToNeighbor;
                Neighbor->HCost = CalculateDistance(Neighbor, TargetNode);
                Neighbor->ParentNode = CurrentNode;

                if (!OpenList.Contains(Neighbor))
                {
                    OpenList.Add(Neighbor);
                }
            }
        }
    }
    return false;
}

void AAStarGridManager::BlurObstaclePenalties(int32 BlurSize)
{
    int32 KernelSize = BlurSize * 2 + 1;
	int32 KernelExtents = BlurSize;

	// 원본 그리드의 가중치 복사
	TArray<float> OriginalWeights;
	OriginalWeights.SetNum(Grid.Num());
	for(int i = 0; i < Grid.Num(); ++i)
	{
		OriginalWeights[i] = Grid[i].bIsObstacle ? 1.0f : 0.0f;
	}

	// 모든 노드 순회하며 주변 장애물에 따라 가중치 증가
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
			// 기존 가중치에 페널티
			Grid[Y * GridSizeX + X].MovementWeight += Penalty * 25.0f;
		}
	}
}

void AAStarGridManager::BeginPlay()
{
	Super::BeginPlay();

    CreateGrid();
}

void AAStarGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (bDebugDrawGrid && bGridReady)
    {
        const float MaxWeight = 10.0f;

        for (const FPathNode& Node : Grid)
        {
            FColor NodeColor;
            if (Node.bIsObstacle)
            {
                NodeColor = FColor(100, 0, 0); // 장애물 어두운 빨간색
            }
            else
            {
                // 가중치에 따라 노란색에서 빨간색
                //  1.0에 가까우면 노란색, MaxWeight에 가까우면 빨간색
                float LerpAlpha = FMath::Clamp((Node.MovementWeight - 1.0f) / FMath::Max(MaxWeight - 1.0f, 1.0f), 0.0f, 1.0f);
                NodeColor = FLinearColor::LerpUsingHSV(FLinearColor::Yellow, FLinearColor::Red, LerpAlpha).ToFColor(true);
            }

            DrawDebugSphere(
                GetWorld(),
                Node.WorldLocation,
                NodeRadius * 0.2f,
                6,
                NodeColor,
                false,
                -1.f,
                0,
                1.f
            );
        }
    }

}

void AAStarGridManager::CreateGrid()
{
    NodeDiameter = NodeRadius * 2;
    GridSizeX = FMath::RoundToInt(GridWorldSize.X / NodeDiameter);
    GridSizeY = FMath::RoundToInt(GridWorldSize.Y / NodeDiameter);
    Grid.SetNum(GridSizeX * GridSizeY);

    FVector WorldBottomLeft = GetActorLocation() - FVector::ForwardVector * GridWorldSize.X / 2 - FVector::RightVector * GridWorldSize.Y / 2;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWeightZone::StaticClass(), FoundActors);
    TArray<AWeightZone*> WeightActors;
    for (AActor* Actor : FoundActors)
    {
        WeightActors.Add(Cast<AWeightZone>(Actor));
    }

    for (int32 Y = 0; Y < GridSizeY; ++Y)
    {
        for (int32 X = 0; X < GridSizeX; ++X)
        {
            int32 Index = Y * GridSizeX + X;
            FVector WorldPoint = WorldBottomLeft
                + FVector::ForwardVector * (X * NodeDiameter + NodeRadius)
                + FVector::RightVector * (Y * NodeDiameter + NodeRadius);
            Grid[Index].WorldLocation = WorldPoint;
            Grid[Index].GridIndex = FIntPoint(X, Y);

            // 장애물 검사
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(this);

            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
            if (PlayerPawn)
            {
                QueryParams.AddIgnoredActor(PlayerPawn); // 플레이어 폰을 무시 목록 추가
            }

            AActor* SafeZoneActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASafetyZone::StaticClass());
            if (SafeZoneActor)
            {
                QueryParams.AddIgnoredActor(SafeZoneActor); // SafetyZone 무시 목록 추가
            }

            bool bHit = GetWorld()->OverlapAnyTestByObjectType(WorldPoint, FQuat::Identity, FCollisionObjectQueryParams(ObstacleObjectTypes), FCollisionShape::MakeSphere(NodeRadius), QueryParams);
            Grid[Index].bIsObstacle = bHit;

            if (!bHit)
            {
                // 가중치 적용
                for (AWeightZone* WeightActor : WeightActors)
                {
                    if (WeightActor && WeightActor->GetWeightBox().IsInside(WorldPoint))
                    {
                        Grid[Index].MovementWeight = WeightActor->MovementWeight;
                        break; // 첫 번째로 찾은 가중치를 적용하고 멈춤
                    }
                }
            }
        }
    }

    BlurObstaclePenalties(3);

    bGridReady = true;
    UE_LOG(LogTemp, Log, TEXT("A* Grid is ready."));
}

FPathNode* AAStarGridManager::GetNodeFromWorldLocation(FVector WorldLocation)
{
    // 그리드 월드 중심에서 얼마나 떨어져 있는지 계산
    FVector LocalPos = WorldLocation - GetActorLocation();

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

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            // 자기 자신은 제외
            if (x == 0 && y == 0)
                continue;

            int checkX = Node->GridIndex.X + x;
            int checkY = Node->GridIndex.Y + y;

            // 그리드 범위 안에 있는지 확인
            if (checkX >= 0 && checkX < GridSizeX && checkY >= 0 && checkY < GridSizeY)
            {
                int32 Index = checkY * GridSizeX + checkX;
                OutNeighbors.Add(&Grid[Index]);
            }
        }
    }
}

float AAStarGridManager::CalculateDistance(const FPathNode* A, const FPathNode* B)
{
    int32 DstX = FMath::Abs(A->GridIndex.X - B->GridIndex.X);
    int32 DstY = FMath::Abs(A->GridIndex.Y - B->GridIndex.Y);

    // 직선 이동 비용: 10, 대각선 이동 비용: 14
    if (DstX > DstY)
        return 14 * DstY + 10 * (DstX - DstY);

    return 14 * DstX + 10 * (DstY - DstX);
}

void AAStarGridManager::RetracePath(const FPathNode* StartNode, const FPathNode* EndNode, TArray<FVector>& OutPath)
{
    OutPath.Empty();
    const FPathNode* CurrentNode = EndNode;

    TArray<FVector> TempPath;
    while (CurrentNode != StartNode && CurrentNode != nullptr)
    {
        TempPath.Add(CurrentNode->WorldLocation);
        CurrentNode = CurrentNode->ParentNode;
    }

    // 경로 역순
    Algo::Reverse(TempPath);
    OutPath = TempPath;

    TArray<FVector> RawPath = TempPath; // 뒤집힌 경로 임시 저장
    SimplifyPath(RawPath, OutPath);
}

void AAStarGridManager::SimplifyPath(const TArray<FVector>& InPath, TArray<FVector>& OutPath)
{
    OutPath.Empty();
    if (InPath.Num() < 2)
    {
        OutPath = InPath;
        return;
    }

    OutPath.Add(InPath[0]); // 시작점 항상 포함
    FVector LastDirection = (InPath[1] - InPath[0]).GetSafeNormal();

    for (int32 i = 1; i < InPath.Num() - 1; ++i)
    {
        FVector CurrentDirection = (InPath[i + 1] - InPath[i]).GetSafeNormal();
        // 이전 방향과 현재 방향이 다르면, 해당 지점은 코너이므로 경로 추가
        if (!LastDirection.Equals(CurrentDirection, 0.1f))
        {
            OutPath.Add(InPath[i]);
        }
        LastDirection = CurrentDirection;
    }

    OutPath.Add(InPath.Last()); // 끝점ㄴ 항상 포함
}

