#include "OSC/Fire/FireManager.h"

#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/GameStateBase.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "OSC/PlayerBaseState.h"
#include "OSC/VFX/VFXManager.h"
#include "OSC/VFX/VFXActor.h"


AFireManager::AFireManager()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicates(true);
}

void AFireManager::BeginPlay()
{
    Super::BeginPlay();
    ActiveFireVFXActors.Reset();

    // MPC 기본값 초기화 (문제의 원인 해결)
    if (MPC_Fire)
    {
        if (UWorld* W = GetWorld())
        {
            if (UMaterialParameterCollectionInstance* Inst = W->GetParameterCollectionInstance(MPC_Fire))
            {
                Inst->SetVectorParameterValue(TEXT("BurnCenter"), FVector::ZeroVector);
                Inst->SetScalarParameterValue(TEXT("BurnRadius"), OnFireBurnRadius);
                Inst->SetScalarParameterValue(TEXT("EdgeWidth"), OnFireEdgeWidth);
                Inst->SetScalarParameterValue(TEXT("BurnIntensity_G"), OnFireBurnIntensity_G);
                
                UE_LOG(LogTemp, Warning, TEXT("FireManager: MPC 초기화 완료 - BurnRadius=%f, BurnIntensity_G=%f"), 
                    OnFireBurnRadius, OnFireBurnIntensity_G);
            }
        }
    }

    if (HasAuthority())
    {
        VFXManager = Cast<AVFXManager>(UGameplayStatics::GetActorOfClass(this, AVFXManager::StaticClass()));
        if (VFXManager.IsValid())
        {
            for (const FName& VFXName : FireVFXs)
            {
                if (!VFXName.IsNone())
                {
                    VFXManager->InitPool(VFXName);
                }
            }
        }
    }

    if (!HasAuthority())
    {
        return;
    }

    Cells.Reset();
    RootCellIndices.Reset();
    FreeCellIndices.Reset();

    TArray<AActor*> CellBoxes;
    UGameplayStatics::GetAllActorsWithTag(this, FName(TEXT("CellBox")), CellBoxes);

    for (AActor* Actor : CellBoxes)
    {
        if (!IsValid(Actor))
        {
            continue;
        }

        GenerateCellsFromActor(*Actor);
    }

    InitializeFireAreas();

    // FTimerHandle TimerHandle;
    // FTimerDelegate TimerDelegate;
    // TimerDelegate.BindLambda([this]
    // {
    //     if (!IsValid(this)) return;
    //     if (!IsValid(GetWorld())) return;
    //     
    //
    // });
    //
    // GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, 1, true);
}

void AFireManager::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HasAuthority())
    {
        return;
    }
   
    NextActiveCells.Empty();
    
    for (int32 CellIndex : ActiveCells)
    {
        if (!Cells.IsValidIndex(CellIndex))
            continue;
    
        FFireCell& Cell = Cells[CellIndex];
    
        if (Cell.State == EFireCellState::Igniting)
        {
            Cell.IgnitionTimeRemaining -= DeltaSeconds;
            
            if (Cell.IgnitionTimeRemaining <= 0)
            {
                Cell.State = EFireCellState::Burning;
                Cell.Heat = DefaultHeatValue;
                ActivateFireVFX(CellIndex);
            }
            NextActiveCells.Add(CellIndex);
        }
        else if (Cell.State == EFireCellState::Burning)
        {
            if (Cell.Heat > 0)
            {
                SpreadFireFromCell(CellIndex);
                NextActiveCells.Add(CellIndex);
            }
            else
            {
                Cell.State = EFireCellState::Extinguished;
                DeactivateFireVFX(CellIndex);
            }
        }
    }
    
    ActiveCells = MoveTemp(NextActiveCells);

    CheckPlayerInFire(DeltaSeconds);
    
    CurrentTime += DeltaSeconds;
    if (CurrentTime > 1.0f)
    {
        CurrentTime = 0.0f;
        
        for (int32 RootIndex : RootCellIndices)
        {
            if (!Cells.IsValidIndex(RootIndex))
            {
                continue;
            }
    
            ProcessCollapseRecursive(RootIndex, 1);
        }
    }
}

void AFireManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearAllTimersForObject(this);
    
    Super::EndPlay(EndPlayReason);
}

void AFireManager::CheckPlayerInFire(float DeltaSeconds)
{
    for (uint64 i = 0; i < GetWorld()->GetGameState()->PlayerArray.Num(); i++)
    {
        APlayerState* PS = GetWorld()->GetGameState()->PlayerArray[i];
        
        if (!PS) continue;

        APlayerController* PC = PS->GetOwner<APlayerController>();

        if (!PC) continue;

        APawn* Pawn = PC->GetPawn();
        if (!Pawn) continue;
        
        APlayerBaseState* PBS = Cast<APlayerBaseState>(PS);
        
        const FVector PlayerLocation = Pawn->GetActorLocation();
        const FVector PlayerExtent = FVector(PBS->GetSize());
        const FBox PlayerBox = FBox::BuildAABB(PlayerLocation, PlayerExtent);

        for (int32 CellIndex : ActiveCells)
        {
            if (!Cells.IsValidIndex(CellIndex))
                continue;

            const FFireCell& Cell = Cells[CellIndex];
            float Value = 1.0f;

            if (Cell.State == EFireCellState::Burning )  Value = 2.0f;
            else if (Cell.State == EFireCellState::Igniting) Value = 1.0f;
            else continue;;

            const FBox FireBox = FBox::BuildAABB(Cell.WorldCenter, Cell.CellExtent);
            
            if (PlayerBox.Intersect(FireBox))
            {
                if (PBS)
                {
                    PBS->AddFireTime(DeltaSeconds * Value);
                }
                break;
            }
        }
    }
}

AFireManager::FFireCellTickResult AFireManager::ProcessCollapseRecursive(int32 CellIndex, float DeltaSeconds)
{
    FFireCellTickResult Result;

    if (!GetWorld()) return Result;
    
    if (!Cells.IsValidIndex(CellIndex))
    {
        return Result;
    }

    FFireCell& Cell = Cells[CellIndex];

    if (!Cell.bIsLeaf)
    {
        bool bAllChildrenLeaf = true;
        bool bAllChildrenExtinguishedOrDormant = true;
        bool bAnyChildBurning = false;

        for (int32 i = 0; i < FFireCell::NumChildren; i++)
        {
            int32 ChildIndex = Cells[CellIndex].ChildIndices[i];
            if (ChildIndex == INDEX_NONE)
            {
                continue;
            }

            FFireCellTickResult ChildResult = ProcessCollapseRecursive(ChildIndex, DeltaSeconds);
            bAnyChildBurning |= ChildResult.bAnyBurning;

            if (!Cells.IsValidIndex(ChildIndex))
            {
                bAllChildrenLeaf = false;
                bAllChildrenExtinguishedOrDormant = false;
                continue;
            }

            const FFireCell& ChildCell = Cells[ChildIndex];
            if (!ChildCell.bIsLeaf)
            {
                bAllChildrenLeaf = false;
                bAllChildrenExtinguishedOrDormant = false;
                continue;
            }

            if (ChildCell.State != EFireCellState::Dormant && ChildCell.State != EFireCellState::Extinguished)
            {
                bAllChildrenExtinguishedOrDormant = false;
            }
        }

        if (bAllChildrenLeaf && bAllChildrenExtinguishedOrDormant)
        {
            //UE_LOG(LogTemp, Warning, TEXT("Collapse"));
            CollapseCell(CellIndex);
            DeactivateFireVFX(CellIndex);
            FFireCell& CollapsedCell = Cells[CellIndex];
            CollapsedCell.State = EFireCellState::Extinguished;
            CollapsedCell.Heat = 0.0f;
            CollapsedCell.IgnitionTimeRemaining = 0.0f;
            CollapsedCell.ActiveEffect = nullptr;
        }
        else
        {
            Result.bIsLeaf = false;
            Result.bAnyBurning = bAnyChildBurning;
            return Result;
        }
    }
    
    if (!Result.bAnyBurning)
    {
        Result.bAnyBurning = (Cells[CellIndex].State == EFireCellState::Burning || Cells[CellIndex].State == EFireCellState::Igniting)&& Cells[CellIndex].Heat > 0.0f;
    }

    return Result;
}

void AFireManager::GenerateCellsFromActor(AActor& SourceActor)
{
    if (LeafCellSize <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    FVector Origin = SourceActor.GetActorLocation();
    FVector Extent = FVector::ZeroVector;

    if (UBoxComponent* Box = SourceActor.FindComponentByClass<UBoxComponent>())
    {
        Origin = Box->GetComponentLocation();
        Extent = Box->GetScaledBoxExtent();
    }
    else
    {
        FVector BoundsOrigin;
        SourceActor.GetActorBounds(false, BoundsOrigin, Extent);
        Origin = BoundsOrigin;
    }

    if (Extent.IsNearlyZero())
    {
        return;
    }

    const FBox Bounds = FBox::BuildAABB(Origin, Extent);
    const int32 RootIndex = CreateCell(Bounds.GetCenter(), Bounds.GetExtent(), 0, INDEX_NONE);
    if (RootIndex != INDEX_NONE)
    {
        RootCellIndices.Add(RootIndex);
    }
}

int32 AFireManager::CreateCell(const FVector& Center, const FVector& Extent, int32 Depth, int32 ParentIndex)
{
    int32 CellIndex = INDEX_NONE;
    if (FreeCellIndices.Num() > 0)
    {
        CellIndex = FreeCellIndices.Pop(EAllowShrinking::No);
        if (!Cells.IsValidIndex(CellIndex))
        {
            CellIndex = INDEX_NONE;
        }
    }

    FFireCell NewCell;
    NewCell.WorldCenter = Center;
    NewCell.CellExtent = Extent;
    NewCell.Depth = Depth;
    NewCell.ParentIndex = ParentIndex;
    NewCell.MaxFuel = DefaultFuelAmount;
    NewCell.Fuel = DefaultFuelAmount;

    const FVector Quantized = Center / FMath::Max(LeafCellSize, 1.0f);
    NewCell.GridIndex = FIntVector(
        FMath::FloorToInt(Quantized.X),
        FMath::FloorToInt(Quantized.Y),
        FMath::FloorToInt(Quantized.Z));

    if (CellIndex != INDEX_NONE)
    {
        Cells[CellIndex] = NewCell;
        return CellIndex;
    }
    
           
    return Cells.Add(NewCell);
}

void AFireManager::SubdivideCell(int32 CellIndex)
{
    if (!Cells.IsValidIndex(CellIndex))
    {
        return;
    }

    FFireCell& Cell = Cells[CellIndex];
    if (!Cell.bIsLeaf)
    {
        return;
    }

    if (Cell.Depth >= MaxOctreeDepth)
    {
        return;
    }

    const FVector ChildExtent = Cell.CellExtent * 0.5f;
    if (ChildExtent.IsNearlyZero())
    {
        return;
    }

    Cell.bIsLeaf = false;
    Cells.Reserve(Cells.Num() + FFireCell::NumChildren);
    
    int32 ChildCounter = 0;
    for (int32 X = 0; X < 2; ++X)
    {
        const float OffsetX = (X == 0 ? -1.0f : 1.0f) * ChildExtent.X;
        for (int32 Y = 0; Y < 2; ++Y)
        {
            const float OffsetY = (Y == 0 ? -1.0f : 1.0f) * ChildExtent.Y;
            for (int32 Z = 0; Z < 2; ++Z)
            {
                const float OffsetZ = (Z == 0 ? -1.0f : 1.0f) * ChildExtent.Z;
                const FVector ChildCenter = Cells[CellIndex].WorldCenter + FVector(OffsetX, OffsetY, OffsetZ);
                
                const int32 ChildIndex = CreateCell(ChildCenter, ChildExtent, Cells[CellIndex].Depth + 1, CellIndex);

                //UE_LOG(LogTemp, Warning, TEXT("Created %d %d %d"), CellIndex, ChildCounter, ChildIndex)
                
                Cells[CellIndex].ChildIndices[ChildCounter++] = ChildIndex;
            }
        }
    }
}

void AFireManager::CollapseCell(int32 CellIndex)
{
    if (!Cells.IsValidIndex(CellIndex))
    {
        return;
    }

    FFireCell& Cell = Cells[CellIndex];
    if (Cell.bIsLeaf)
    {
        return;
    }

    for (int32& ChildIndex : Cell.ChildIndices)
    {
        if (Cells.IsValidIndex(ChildIndex))
        {
            CollapseCell(ChildIndex);
            DeactivateFireVFX(ChildIndex);
            
            FFireCell& ChildCell = Cells[ChildIndex];                                                                            
            ChildCell = FFireCell();        // 기본값으로 초기화                                                                 
            ChildCell.State = EFireCellState::Dormant;                                                                           
            ChildCell.CellExtent = FVector::ZeroVector;                                                                          
            ChildCell.ParentIndex = INDEX_NONE;                                                                    
            ChildCell.AttachedComponent = nullptr;                                                                               
            ChildCell.ActiveEffect = nullptr;
            ChildCell.bIsLeaf = false;
            
            FreeCellIndices.Add(ChildIndex);
        }
        ChildIndex = INDEX_NONE;
    }

    Cell.bIsLeaf = true;
}

void AFireManager::InitializeFireAreas()
{
    TArray<AActor*> FireAreas;
    UGameplayStatics::GetAllActorsWithTag(this, FName(TEXT("FireArea")), FireAreas);

    for (AActor* Actor : FireAreas)
    {
        if (!IsValid(Actor))
        {
            continue;
        }

        FVector Center = Actor->GetActorLocation();
        float Radius = 0.0f;

        if (USphereComponent* Sphere = Actor->FindComponentByClass<USphereComponent>())
        {
            Center = Sphere->GetComponentLocation();
            Radius = Sphere->GetScaledSphereRadius();
        }
        else
        {
            FVector BoundsOrigin, BoundsExtent;
            Actor->GetActorBounds(false, BoundsOrigin, BoundsExtent);
            Center = BoundsOrigin;
            Radius = BoundsExtent.Size();
        }

        if (Radius > KINDA_SMALL_NUMBER)
        {
            IgniteSphere(Center, Radius);
        }
    }
}

void AFireManager::IgniteSphere(const FVector& Center, float Radius)
{
    for (int32 RootIndex : RootCellIndices)
    {
        IgniteSphereRecursive(RootIndex, Center, Radius);
    }
}

void AFireManager::IgniteSphereRecursive(int32 CellIndex, const FVector& Center, float Radius)
{
    if (!Cells.IsValidIndex(CellIndex))
    {
        return;
    }

    FFireCell& Cell = Cells[CellIndex];
    const FBox CellBounds = FBox::BuildAABB(Cell.WorldCenter, Cell.CellExtent);
    const double DistanceSquared = CellBounds.ComputeSquaredDistanceToPoint(Center);
    if (DistanceSquared > FMath::Square(static_cast<double>(Radius)))
    {
        return;
    }

    const float CellSize = Cell.CellExtent.GetMax() * 2.0f;
 
    if (CellSize > LeafCellSize + KINDA_SMALL_NUMBER && Cell.Depth < MaxOctreeDepth)
    {
        SubdivideCell(CellIndex);
        if (!Cells[CellIndex].bIsLeaf)
        {
            for (int32 ChildIndex : Cells[CellIndex].ChildIndices)
            {
                //UE_LOG(LogTemp, Warning, TEXT("Receive %d %d"), CellIndex, ChildIndex);
                
                if (ChildIndex != INDEX_NONE)
                {
                    IgniteSphereRecursive(ChildIndex, Center, Radius);
                }
            }
            return;
        }
    }

    if (!EnsureCombustibleComponent(CellIndex))
    {
        return;
    }

    Cell.State = EFireCellState::Burning;
    Cell.Heat = DefaultHeatValue;
    Cell.IgnitionTimeRemaining = 0.0f;
    ActiveCells.Add(CellIndex);
    ActivateFireVFX(CellIndex);
}

int32 AFireManager::FindCellIndexAtLocation(const FVector& WorldLocation) const
{
    for (int32 RootIndex : RootCellIndices)
    {
        const int32 Result = FindCellIndexRecursive(RootIndex, WorldLocation);
        if (Result != INDEX_NONE)
        {
            return Result;
        }
    }

    return INDEX_NONE;
}

int32 AFireManager::FindCellIndexRecursive(int32 CellIndex, const FVector& WorldLocation) const
{
    if (!Cells.IsValidIndex(CellIndex))
    {
        return INDEX_NONE;
    }

    const FFireCell& Cell = Cells[CellIndex];
    const FBox CellBounds = FBox::BuildAABB(Cell.WorldCenter, Cell.CellExtent);
    if (!CellBounds.IsInsideOrOn(WorldLocation))
    {
        return INDEX_NONE;
    }

    if (Cell.bIsLeaf)
    {
        return CellIndex;
    }

    const FVector Center = Cell.WorldCenter;
    const int32 BitX = WorldLocation.X >= Center.X ? 1 : 0;
    const int32 BitY = WorldLocation.Y >= Center.Y ? 1 : 0;
    const int32 BitZ = WorldLocation.Z >= Center.Z ? 1 : 0;

    const int32 ChildSlot = (BitX << 2) | (BitY << 1) | BitZ;
    if (ChildSlot < 0 || ChildSlot >= FFireCell::NumChildren)
    {
        return CellIndex;
    }

    const int32 ChildIndex = Cell.ChildIndices[ChildSlot];
    if (ChildIndex == INDEX_NONE)
    {
        return CellIndex;
    }

    const int32 Result = FindCellIndexRecursive(ChildIndex, WorldLocation);
    return Result != INDEX_NONE ? Result : CellIndex;
}

void AFireManager::SpreadFireFromCell(int32 CellIndex)
{
    if (!Cells.IsValidIndex(CellIndex))
    {
        return;
    }

    const FFireCell& SourceCell = Cells[CellIndex];
    if (!SourceCell.bIsLeaf || SourceCell.State != EFireCellState::Burning)
    {
        return;
    }

    const FBox SourceBounds = FBox::BuildAABB(SourceCell.WorldCenter, SourceCell.CellExtent);
    for (int32 RootIndex : RootCellIndices)
    {
        SpreadFireRecursive(CellIndex, RootIndex, SourceBounds);
    }
}

void AFireManager::SpreadFireRecursive(int32 SourceIndex, int32 TargetIndex, const FBox& SourceBounds)
{
    if (!Cells.IsValidIndex(TargetIndex) || TargetIndex == SourceIndex)
    {
        return;
    }

    FFireCell& TargetCell = Cells[TargetIndex];
    if (TargetCell.CellExtent.IsNearlyZero())
    {
        return;
    }

    const FBox TargetBounds = FBox::BuildAABB(TargetCell.WorldCenter, TargetCell.CellExtent);
    if (!AreBoxesPotentiallyAdjacent(SourceBounds, TargetBounds))
    {
        return;
    }

    if (!TargetCell.bIsLeaf)
    {
        const int32 ChildNum = FFireCell::NumChildren;
        for (int32 i = 0; i < ChildNum; i++)
        {
            const int32 ChildIndex = Cells[TargetIndex].ChildIndices[i];
            if (ChildIndex != INDEX_NONE)
            {
                SpreadFireRecursive(SourceIndex, ChildIndex, SourceBounds);
            }
        }
        return;
    }

    const float TargetSize = TargetCell.CellExtent.GetMax() * 2.0f;
    if (TargetCell.Depth < MaxOctreeDepth && TargetSize > LeafCellSize + KINDA_SMALL_NUMBER)
    {
        SubdivideCell(TargetIndex);
        if (!Cells[TargetIndex].bIsLeaf)
        {
            for (int32 i = 0; i < FFireCell::NumChildren; i++)
            {
                int32 ChildIndex = Cells[TargetIndex].ChildIndices[i];
                if (ChildIndex != INDEX_NONE)
                {
                    SpreadFireRecursive(SourceIndex, ChildIndex, SourceBounds);
                }
            }
        }
        return;
    }

    if (TargetCell.State == EFireCellState::Burning || TargetCell.State == EFireCellState::Igniting)
    {
        return;
    }

    if (!AreBoxesAdjacent(SourceBounds, TargetBounds))
    {
        return;
    }

    if (!EnsureCombustibleComponent(TargetIndex))
    {
        return;
    }

    NextActiveCells.Add(TargetIndex);
    TargetCell.State = EFireCellState::Igniting;
    TargetCell.IgnitionTimeRemaining = IgnitionDelay;
}

bool AFireManager::AreBoxesPotentiallyAdjacent(const FBox& SourceBounds, const FBox& TargetBounds) const
{
    return SourceBounds.ExpandBy(SpreadSearchTolerance).Intersect(TargetBounds);
}

bool AFireManager::AreBoxesAdjacent(const FBox& SourceBounds, const FBox& TargetBounds) const
{
    return SourceBounds.ExpandBy(AdjacentContactTolerance).Intersect(TargetBounds);
}

bool AFireManager::EnsureCombustibleComponent(int32 CellIndex)
{
    if (!Cells.IsValidIndex(CellIndex))
    {
        return false;
    }

    FFireCell& Cell = Cells[CellIndex];
    if (Cell.AttachedComponent.IsValid())
    {
        return true;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    const FVector Extent = Cell.CellExtent;
    if (Extent.IsNearlyZero())
    {
        return false;
    }

    const FCollisionShape CollisionShape = FCollisionShape::MakeBox(Extent + FVector(1.0f));
    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
    ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FireCellOverlap), false, this);
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(this);

    TArray<FOverlapResult> Overlaps;
    if (!World->OverlapMultiByObjectType(Overlaps, Cell.WorldCenter, FQuat::Identity, ObjectParams, CollisionShape, QueryParams))
    {
        return false;
    }

    for (const FOverlapResult& Result : Overlaps)
    {
        UPrimitiveComponent* Primitive = Result.GetComponent();
        if (!Primitive || !Primitive->IsRegistered() || Primitive->GetOwner() == this)
        {
            continue;
        }

        if (Primitive->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
        {
            continue;
        }

        const bool bIsCombustibleMesh = Primitive->IsA<UStaticMeshComponent>()
            || Primitive->IsA<USkeletalMeshComponent>()
            || Primitive->IsA<UInstancedStaticMeshComponent>()
            || Primitive->IsA<UHierarchicalInstancedStaticMeshComponent>();

        if (!bIsCombustibleMesh)
        {
            continue;
        }

        Cell.AttachedComponent = Primitive;
        Cell.SurfacePoint = Cell.WorldCenter;
        Cell.SurfaceNormal = FVector::UpVector;
        return true;
    }

    return false;
}

void AFireManager::ActivateFireVFX(int32 CellIndex)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!Cells.IsValidIndex(CellIndex) || FireVFXs.Num() == 0)
    {
        return;
    }

    FFireCell& Cell = Cells[CellIndex];
    const FVector SpawnLocation = Cell.SurfacePoint.IsNearlyZero() ? Cell.WorldCenter : Cell.SurfacePoint;
    const FRotator SpawnRotation = Cell.SurfaceNormal.IsNearlyZero() ? FRotator::ZeroRotator : Cell.SurfaceNormal.Rotation();
    const int32 EffectIndex = FMath::RandRange(0, FireVFXs.Num() - 1);

    if (!FireVFXs.IsValidIndex(EffectIndex))
    {
        return;
    }

    if (TWeakObjectPtr<AVFXActor>* ExistingPtr = ActiveFireVFXActors.Find(CellIndex))
    {
        if (AVFXActor* ExistingActor = ExistingPtr->Get())
        {
            if (VFXManager.IsValid())
            {
                VFXManager->DespawnVFX(ExistingActor);
            }
            else
            {
                ExistingActor->Destroy();
            }
        }

        ActiveFireVFXActors.Remove(CellIndex);
    }

    AVFXActor* SpawnedActor = nullptr;
    const FName& VFXName = FireVFXs[EffectIndex];

    if (!VFXName.IsNone() && VFXManager.IsValid())
    {
        SpawnedActor = VFXManager->SpawnVFX(VFXName, SpawnLocation, SpawnRotation, FVector(FireVFXSize));
    }

    if (!IsValid(SpawnedActor))
    {
        return;
    }

    SpawnedActor->SetVFXName(VFXName);
    //전역 파리미터 갱신
    if (MPC_Fire)
    {
        SpawnedActor->SetMPCFire(MPC_Fire); 
    }
    
    ActiveFireVFXActors.Add(CellIndex, SpawnedActor);
    Cell.ActiveEffect = SpawnedActor;

    // 멀티캐스트 RPC로 모든 클라이언트에 머티리얼 변경 전파
    if (Cell.AttachedComponent.IsValid())
    {
        Multicast_UpdateBurnMaterial(Cell.AttachedComponent.Get(), SpawnLocation, CellIndex);
    }

    // 번짐 시작 (AVFXActor가 MPC_Fire를 제어함)
    const float MaxRadius = Cell.CellExtent.Size();
    SpawnedActor->SetBurnParams(
        SpawnLocation,   // Center
        /*InRadius*/ OnFireBurnRadius,  // 기존 파라미터 사용
        /*InEdgeWidth*/ OnFireEdgeWidth,
        /*InIntensityG*/ OnFireBurnIntensity_G);

    //확산시작
    SpawnedActor->StartBurnAt(SpawnLocation, 0.f, MaxRadius);

}

void AFireManager::DeactivateFireVFX(int32 CellIndex)
{
    if (!Cells.IsValidIndex(CellIndex))
    {
        return;
    }
    
    if (!HasAuthority())
    {
        return;
    }
    
    // 멀티캐스트 RPC로 소화 애니메이션 시작 (서버에서만 호출)
    if (Cells[CellIndex].AttachedComponent.IsValid())
    {
        Multicast_StartExtinguishAnimation(Cells[CellIndex].AttachedComponent.Get(), CellIndex);
    }

    if (TWeakObjectPtr<AVFXActor>* ExistingPtr = ActiveFireVFXActors.Find(CellIndex))
    {
        if (AVFXActor* ExistingActor = ExistingPtr->Get())
        {
            if (VFXManager.IsValid())
            {
                VFXManager->DespawnVFX(ExistingActor);
            }
            else
            {
                ExistingActor->Destroy();
            }
        }

        ActiveFireVFXActors.Remove(CellIndex);
    }

    if (Cells.IsValidIndex(CellIndex))
    {
        Cells[CellIndex].ActiveEffect = nullptr;
    }

    //메테리얼 전환
    ClearBurnMIForCell(CellIndex);
}

void AFireManager::ApplySuppressionAtLocation(const FVector& WorldLocation, float SuppressionAmount)
{
    if (SuppressionAmount <= 0.0f)
    {
        return;
    }

    const int32 CellIndex = FindCellIndexAtLocation(WorldLocation);
    ApplySuppressionToCell(CellIndex, SuppressionAmount);
}

void AFireManager::ApplySuppressionInSphere(APlayerBaseState* PlayerState, const FVector& Center, float Radius, float SuppressionAmount)
{
    if (SuppressionAmount <= 0.0f || Radius <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const double RadiusSquared = FMath::Square(static_cast<double>(Radius));
    for (int32 RootIndex : RootCellIndices)
    {
        ApplySuppressionInSphereRecursive(PlayerState, RootIndex, Center, RadiusSquared, SuppressionAmount);
    }
}

void AFireManager::ApplySuppressionInSphereRecursive(APlayerBaseState* PlayerState, int32 CellIndex, const FVector& Center, double RadiusSquared, float SuppressionAmount)
{
    if (!Cells.IsValidIndex(CellIndex))
    {
        return;
    }

    FFireCell& Cell = Cells[CellIndex];
    const FBox Bounds = FBox::BuildAABB(Cell.WorldCenter, Cell.CellExtent);
    const double DistanceSquared = Bounds.ComputeSquaredDistanceToPoint(Center);
    if (DistanceSquared > RadiusSquared)
    {
        return;
    }

    if (!Cell.bIsLeaf)
    {
        for (int32 ChildIndex : Cell.ChildIndices)
        {
            if (ChildIndex != INDEX_NONE)
            {
                ApplySuppressionInSphereRecursive(PlayerState, ChildIndex, Center, RadiusSquared, SuppressionAmount);
            }
        }
        return;
    }

    if (ApplySuppressionToCell(CellIndex, SuppressionAmount))
    {
        // nothing extra
        PlayerState->AddExtinguishScore(1);
    }
}

bool AFireManager::ApplySuppressionToCell(int32 CellIndex, float SuppressionAmount)
{
    if (!Cells.IsValidIndex(CellIndex) || SuppressionAmount <= 0.0f)
    {
        return false;
    }

    FFireCell& Cell = Cells[CellIndex];
    if (!Cell.bIsLeaf)
    {
        return false;
    }

    if (Cell.State == EFireCellState::Dormant || Cell.State == EFireCellState::Extinguished)
    {
        return false;
    }

    bool bModified = false;

    if (Cell.State == EFireCellState::Burning)
    {
        Cell.Heat = FMath::Max(0.0f, Cell.Heat - SuppressionAmount);
        // UE_LOG(LogTemp, Warning, TEXT("Cell %d : %f / -%f"), CellIndex, Cell.Heat, SuppressionAmount);
        
        if (Cell.Heat <= KINDA_SMALL_NUMBER)
        {
            bModified = true;
            Cell.Heat = 0.0f;
            Cell.State = EFireCellState::Extinguished;
            Cell.IgnitionTimeRemaining = 0.0f;
            DeactivateFireVFX(CellIndex);
        }
    }
    else if (Cell.State == EFireCellState::Igniting)
    {
        Cell.IgnitionTimeRemaining = FMath::Max(0.0f, Cell.IgnitionTimeRemaining - SuppressionAmount);
        Cell.Heat = FMath::Max(0.0f, Cell.Heat - SuppressionAmount);

        if (Cell.IgnitionTimeRemaining <= KINDA_SMALL_NUMBER || Cell.Heat <= KINDA_SMALL_NUMBER)
        {
            Cell.State = EFireCellState::Extinguished;
            Cell.Heat = 0.0f;
            Cell.IgnitionTimeRemaining = 0.0f;
            DeactivateFireVFX(CellIndex);
        }
    }

    return bModified;
}

void AFireManager::FreezeBurnMaterialAtCell(int32 CellIndex)
{
    if (!Cells.IsValidIndex(CellIndex)) return;
    // 점진적 소화 애니메이션 시작
    StartExtinguishAnimation(CellIndex);
}

void AFireManager::StartExtinguishAnimation(int32 CellIndex)
{
    if (!Cells.IsValidIndex(CellIndex)) return;
    
    // 기존 타이머가 있다면 제거
    if (FTimerHandle* ExistingTimer = ExtinguishAnimationTimers.Find(CellIndex))
    {
        if (!GetWorld()) return;
        GetWorldTimerManager().ClearTimer(*ExistingTimer);
    }
    
    // 애니메이션 진행률 초기화
    ExtinguishAnimationProgress.Add(CellIndex, 0.0f);
    
    
    // 타이머 설정 (0.05초마다 업데이트)
    FTimerHandle TimerHandle;
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUFunction(this, FName("UpdateExtinguishAnimation"), CellIndex);
    GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.05f, true);
    
    ExtinguishAnimationTimers.Add(CellIndex, TimerHandle);
    
    //UE_LOG(LogTemp, Warning, TEXT("FireManager: 점진적 소화 애니메이션 시작 - CellIndex=%d"), CellIndex);
}

void AFireManager::UpdateExtinguishAnimation(int32 CellIndex)
{
    if (!Cells.IsValidIndex(CellIndex)) return;

    float* ProgressPtr = ExtinguishAnimationProgress.Find(CellIndex);
    if (!ProgressPtr) return;
    float& Progress = *ProgressPtr;

    Progress += 0.05f / ExtinguishAnimationDuration;  
    if (Progress >= 1.0f) { Progress = 1.0f; CompleteExtinguishAnimation(CellIndex); return; }

    FFireCell& Cell = Cells[CellIndex];
    UPrimitiveComponent* Prim = Cell.AttachedComponent.Get();
    if (!Prim) return;

    auto UpdateMaterial = [&](UMaterialInstanceDynamic* MI)
    {
        if (!MI) return;

        // 0→1로 '소화' 진행
        const float ExtVal = Progress;
        
        // 강도는 점차 0으로
        const float Intensity = FMath::Lerp(OnFireBurnIntensity_G, 0.0f, Progress);
        
        // 재 효과: 점진적으로 어두워짐 (0→AshDarkenAmount)
        const float AshAmount = FMath::Lerp(0.0f, AshDarkenAmount, Progress);

        MI->SetScalarParameterValue(TEXT("Extinguished"),    ExtVal);
        MI->SetScalarParameterValue(TEXT("BurnIntensity_G"), Intensity);
        MI->SetScalarParameterValue(TEXT("AshAmount"),       AshAmount);

        //UE_LOG(LogTemp, Log, TEXT("FireManager: 소화 진행 중 - CellIndex=%d, Progress=%.2f, Extinguished=%.2f, Intensity=%.2f, AshAmount=%.2f"), 
        //    CellIndex, Progress, ExtVal, Intensity, AshAmount);
    };

    if (UMeshComponent* MC = Cast<UMeshComponent>(Prim))
    {
        const int32 NumSlots = MC->GetNumMaterials();
        for (int32 Slot = 0; Slot < NumSlots; ++Slot)
        {
            const TTuple<TWeakObjectPtr<UMeshComponent>, int32> Key(MC, Slot);
            if (TWeakObjectPtr<UMaterialInstanceDynamic>* FoundPtr = BurnMICacheBySlot.Find(Key))
            {
                if (UMaterialInstanceDynamic* Existing = FoundPtr->Get())
                {
                    UpdateMaterial(Existing);
                }
            }
        }
    }
}

void AFireManager::CompleteExtinguishAnimation(int32 CellIndex)
{
    if (!Cells.IsValidIndex(CellIndex)) return;
    
    // 최종적으로 Extinguished = 1로 확실히 설정
    FFireCell& Cell = Cells[CellIndex];
    if (UMeshComponent* MC = Cast<UMeshComponent>(Cell.AttachedComponent.Get()))
    {
        const int32 NumSlots = MC->GetNumMaterials();
        for (int32 Slot = 0; Slot < NumSlots; ++Slot)
        {
            const TTuple<TWeakObjectPtr<UMeshComponent>, int32> Key(MC, Slot);
            if (TWeakObjectPtr<UMaterialInstanceDynamic>* FoundPtr = BurnMICacheBySlot.Find(Key))
            {
                if (UMaterialInstanceDynamic* DMI = FoundPtr->Get())
                {
                    DMI->SetScalarParameterValue(TEXT("Extinguished"), 1.f);
                    DMI->SetScalarParameterValue(TEXT("BurnIntensity_G"), 0.f);
                    DMI->SetScalarParameterValue(TEXT("AshAmount"), AshDarkenAmount);
                    //UE_LOG(LogTemp, Warning, TEXT("FireManager: 최종 소화 설정 - CellIndex=%d, Slot=%d, Extinguished=1, AshAmount=%.2f"), 
                    //    CellIndex, Slot, AshDarkenAmount);
                }
            }
        }
    }
    
    // 타이머 정리
    if (FTimerHandle* TimerPtr = ExtinguishAnimationTimers.Find(CellIndex))
    {
        GetWorldTimerManager().ClearTimer(*TimerPtr);
        ExtinguishAnimationTimers.Remove(CellIndex);
    }
    
    // 진행률 정리
    ExtinguishAnimationProgress.Remove(CellIndex);
    
    //UE_LOG(LogTemp, Warning, TEXT("FireManager: 점진적 소화 애니메이션 완료 - CellIndex=%d"), CellIndex);
}

void AFireManager::ClearBurnMIForCell(int32 CellIndex)
{
    if (!Cells.IsValidIndex(CellIndex)) return;
    
    UPrimitiveComponent* Prim = Cells[CellIndex].AttachedComponent.Get();
    if (!Prim) return;
    
    // 캐시에서 제거
    BurnMICache.Remove(Prim);
    
    // BurnMICacheBySlot에서 참조 카운트 감소 및 조건부 제거
    if (UMeshComponent* MC = Cast<UMeshComponent>(Prim))
    {
        const int32 NumSlots = MC->GetNumMaterials();
        for (int32 Slot = 0; Slot < NumSlots; ++Slot)
        {
            const TTuple<TWeakObjectPtr<UMeshComponent>, int32> Key(MC, Slot);
            
            // 참조 카운트 감소
            int32* RefCountPtr = BurnMIRefCount.Find(Key);
            if (RefCountPtr)
            {
                (*RefCountPtr)--;
                
                //UE_LOG(LogTemp, Log, TEXT("FireManager: 참조 카운트 감소 - CellIndex=%d, Slot=%d, Mesh=%s, RefCount=%d"), 
                //    CellIndex, Slot, *MC->GetName(), *RefCountPtr);
                
                // 참조 카운트가 0 이하가 되면 완전히 제거
                if (*RefCountPtr <= 0)
                {
                    if (TWeakObjectPtr<UMaterialInstanceDynamic>* FoundPtr = BurnMICacheBySlot.Find(Key))
                    {
                        if (UMaterialInstanceDynamic* DMI = FoundPtr->Get())
                        {
                            // 모든 참조가 사라졌으므로 완전히 꺼진 상태(재)로 설정
                            DMI->SetScalarParameterValue(TEXT("Extinguished"), 1.f);
                            DMI->SetScalarParameterValue(TEXT("BurnIntensity_G"), 0.f);
                            DMI->SetScalarParameterValue(TEXT("AshAmount"), AshDarkenAmount);
                            
                            UE_LOG(LogTemp, Warning, TEXT("FireManager: 완전 소화 - Mesh=%s, Slot=%d, AshAmount=%.2f"), 
                                *MC->GetName(), Slot, AshDarkenAmount);
                        }
                    }
                    
                    BurnMICacheBySlot.Remove(Key);
                    BurnMIRefCount.Remove(Key);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("FireManager: BurnMI 제거 완료 - CellIndex=%d"), CellIndex);
}

void AFireManager::Multicast_UpdateBurnMaterial_Implementation(UPrimitiveComponent* TargetComponent, const FVector& SpawnLocation, int32 CellIndex)
{
    if (!BurnMaterial || !TargetComponent)
    {
        return;
    }

    UMeshComponent* MC = Cast<UMeshComponent>(TargetComponent);
    if (!MC)
    {
        return;
    }

    // 제외할 액터 태그 확인
    AActor* OwnerActor = MC->GetOwner();
    bool bShouldExclude = false;
    
    if (OwnerActor)
    {
        for (const FName& ExcludedTag : ExcludedActorTags)
        {
            if (OwnerActor->ActorHasTag(ExcludedTag))
            {
                bShouldExclude = true;
                UE_LOG(LogTemp, Log, TEXT("FireManager: 머티리얼 변경 제외 - Actor=%s, Tag=%s"), 
                    *OwnerActor->GetName(), *ExcludedTag.ToString());
                return;
            }
        }
    }
    
    const int32 NumSlots = MC->GetNumMaterials();

    auto InitBurnParams = [&](UMaterialInstanceDynamic* DMI, int32 SlotIndex)
    {
        // 불이 붙었을 때 파라미터 설정
        DMI->SetVectorParameterValue(TEXT("BurnCenter"),      SpawnLocation);  
        DMI->SetScalarParameterValue(TEXT("BurnRadius"),      OnFireBurnRadius); 
        DMI->SetScalarParameterValue(TEXT("EdgeWidth"),       OnFireEdgeWidth);
        DMI->SetScalarParameterValue(TEXT("EdgeFalloffPower"), EdgeFalloffPower);
        DMI->SetScalarParameterValue(TEXT("BurnIntensity_G"), OnFireBurnIntensity_G);
        DMI->SetScalarParameterValue(TEXT("Extinguished"),    0.f);
        DMI->SetScalarParameterValue(TEXT("AshAmount"),       0.f);  // 불이 붙었을 때는 재 효과 없음
        
        UE_LOG(LogTemp, Log, TEXT("FireManager: 불 활성화 - CellIndex=%d, Slot=%d, Mesh=%s, BurnCenter=%s"), 
            CellIndex, SlotIndex, *MC->GetName(), *SpawnLocation.ToString());
    };

    for (int32 Slot = 0; Slot < NumSlots; ++Slot)
    {
        const TTuple<TWeakObjectPtr<UMeshComponent>, int32> Key(MC, Slot);

        // 1) 슬롯 캐시에 이미 MID가 있고 유효하면 건너뛰기 (첫 번째 셀이 이미 제어 중)
        if (TWeakObjectPtr<UMaterialInstanceDynamic>* FoundPtr = BurnMICacheBySlot.Find(Key))
        {
            if (UMaterialInstanceDynamic* Existing = FoundPtr->Get())
            {
                // 참조 카운트 증가
                int32& RefCount = BurnMIRefCount.FindOrAdd(Key, 0);
                RefCount++;
                
                UE_LOG(LogTemp, Log, TEXT("FireManager: 스킵 - CellIndex=%d, Slot=%d, Mesh=%s (이미 다른 셀이 제어 중, RefCount=%d)"), 
                    CellIndex, Slot, *MC->GetName(), RefCount);
                continue;
            }
            else
            {
                BurnMICacheBySlot.Remove(Key);
                BurnMIRefCount.Remove(Key);
            }
        }

        
        UMaterialInterface* CurMat = MC->GetMaterial(Slot);
        UMaterialInstanceDynamic* DMI = Cast<UMaterialInstanceDynamic>(CurMat);

        // DMI가 없거나 BurnMaterial이 아니면 새로 생성
        if (!DMI || DMI->Parent != BurnMaterial)
        {
            DMI = MC->CreateDynamicMaterialInstance(Slot, BurnMaterial);
            if (!DMI) { continue; }
            
            UE_LOG(LogTemp, Log, TEXT("FireManager: 새 DMI 생성 - CellIndex=%d, Slot=%d, Mesh=%s"), 
                CellIndex, Slot, *MC->GetName());
        }
        else
        {
            // 기존 DMI 재사용 (재점화 시 파라미터는 InitBurnParams에서 초기화됨)
            UE_LOG(LogTemp, Log, TEXT("FireManager: 기존 DMI 재사용 - CellIndex=%d, Slot=%d, Mesh=%s"), 
                CellIndex, Slot, *MC->GetName());
        }

        // 캐시에 없었다면 재점화 시나리오이므로 파라미터를 항상 초기화
        InitBurnParams(DMI, Slot);

        // 3) 슬롯 단위 캐시에 저장
        BurnMICacheBySlot.Add(Key, DMI);
        
        // 4) 참조 카운트 초기화
        BurnMIRefCount.Add(Key, 1);
    }
}

void AFireManager::Multicast_StartExtinguishAnimation_Implementation(UPrimitiveComponent* TargetComponent, int32 CellIndex)
{
    if (!TargetComponent)
    {
        return;
    }
    
    // 클라이언트에서도 소화 애니메이션 처리
    UMeshComponent* MC = Cast<UMeshComponent>(TargetComponent);
    if (!MC)
    {
        return;
    }
    
    // 기존 타이머가 있다면 제거
    if (FTimerHandle* ExistingTimer = ExtinguishAnimationTimers.Find(CellIndex))
    {
        GetWorldTimerManager().ClearTimer(*ExistingTimer);
    }
    
    // 애니메이션 진행률 초기화
    ExtinguishAnimationProgress.Add(CellIndex, 0.0f);
    
    // 타이머 설정 (0.05초마다 업데이트)
    FTimerHandle TimerHandle;
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindLambda([this, MC, CellIndex]()
    {
        float* ProgressPtr = ExtinguishAnimationProgress.Find(CellIndex);
        if (!ProgressPtr || !MC) return;
        
        float& Progress = *ProgressPtr;
        Progress += 0.05f / ExtinguishAnimationDuration;
        
        if (Progress >= 1.0f)
        {
            Progress = 1.0f;
            
            // 최종 상태 설정
            const int32 NumSlots = MC->GetNumMaterials();
            for (int32 Slot = 0; Slot < NumSlots; ++Slot)
            {
                const TTuple<TWeakObjectPtr<UMeshComponent>, int32> Key(MC, Slot);
                if (TWeakObjectPtr<UMaterialInstanceDynamic>* FoundPtr = BurnMICacheBySlot.Find(Key))
                {
                    if (UMaterialInstanceDynamic* DMI = FoundPtr->Get())
                    {
                        DMI->SetScalarParameterValue(TEXT("Extinguished"), 1.f);
                        DMI->SetScalarParameterValue(TEXT("BurnIntensity_G"), 0.f);
                        DMI->SetScalarParameterValue(TEXT("AshAmount"), AshDarkenAmount);
                    }
                }
            }
            
            // 타이머 정리
            if (FTimerHandle* TimerPtr = ExtinguishAnimationTimers.Find(CellIndex))
            {
                GetWorldTimerManager().ClearTimer(*TimerPtr);
                ExtinguishAnimationTimers.Remove(CellIndex);
            }
            ExtinguishAnimationProgress.Remove(CellIndex);
            
            UE_LOG(LogTemp, Warning, TEXT("FireManager: 점진적 소화 애니메이션 완료 - CellIndex=%d"), CellIndex);
            return;
        }
        
        // 진행 중 상태 업데이트
        const float ExtVal = Progress;
        const float Intensity = FMath::Lerp(OnFireBurnIntensity_G, 0.0f, Progress);
        const float AshAmount = FMath::Lerp(0.0f, AshDarkenAmount, Progress);
        
        const int32 NumSlots = MC->GetNumMaterials();
        for (int32 Slot = 0; Slot < NumSlots; ++Slot)
        {
            const TTuple<TWeakObjectPtr<UMeshComponent>, int32> Key(MC, Slot);
            if (TWeakObjectPtr<UMaterialInstanceDynamic>* FoundPtr = BurnMICacheBySlot.Find(Key))
            {
                if (UMaterialInstanceDynamic* DMI = FoundPtr->Get())
                {
                    DMI->SetScalarParameterValue(TEXT("Extinguished"), ExtVal);
                    DMI->SetScalarParameterValue(TEXT("BurnIntensity_G"), Intensity);
                    DMI->SetScalarParameterValue(TEXT("AshAmount"), AshAmount);
                }
            }
        }
    });
    
    GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.05f, true);
    ExtinguishAnimationTimers.Add(CellIndex, TimerHandle);
    
    UE_LOG(LogTemp, Warning, TEXT("FireManager: 점진적 소화 애니메이션 시작 - CellIndex=%d"), CellIndex);
}


