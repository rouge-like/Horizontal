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
}

void AFireManager::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HasAuthority())
    {
        return;
    }

    for (int32 RootIndex : RootCellIndices)
    {
        if (!Cells.IsValidIndex(RootIndex))
        {
            continue;
        }

        ProcessCellRecursive(RootIndex, DeltaSeconds);
    }
}

AFireManager::FFireCellTickResult AFireManager::ProcessCellRecursive(int32 CellIndex, float DeltaSeconds)
{
    FFireCellTickResult Result;

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

        for (int32 ChildIndex : Cell.ChildIndices)
        {
            if (ChildIndex == INDEX_NONE)
            {
                continue;
            }

            FFireCellTickResult ChildResult = ProcessCellRecursive(ChildIndex, DeltaSeconds);
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

    FFireCell& LeafCell = Cells[CellIndex];

    if (LeafCell.CellExtent.IsNearlyZero())
    {
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
        const FColor DebugColor = LeafCell.State == EFireCellState::Burning ? FColor::Red : (LeafCell.State == EFireCellState::Igniting ? FColor::Yellow : FColor::Green);
        //DrawDebugBox(GetWorld(), LeafCell.WorldCenter, LeafCell.CellExtent, FQuat::Identity, DebugColor, false, 0.0f, 0, 1.0f);
#endif
        Result.bAnyBurning = LeafCell.State == EFireCellState::Burning && LeafCell.Heat > 0.0f;
        return Result;
    }

    if (LeafCell.State == EFireCellState::Igniting)
    {
        LeafCell.IgnitionTimeRemaining = FMath::Max(0.0f, LeafCell.IgnitionTimeRemaining - DeltaSeconds);
        if (LeafCell.IgnitionTimeRemaining <= KINDA_SMALL_NUMBER)
        {
            LeafCell.State = EFireCellState::Burning;
            LeafCell.Heat = DefaultHeatValue;
            ActivateFireVFX(CellIndex);
        }
    }

    if (LeafCell.State == EFireCellState::Burning)
    {
        if (LeafCell.Heat > 0.0f)
        {
            Result.bAnyBurning = true;
            SpreadFireFromCell(CellIndex);
        }
        else
        {
            LeafCell.Heat = 0.0f;
            LeafCell.State = EFireCellState::Extinguished;
            LeafCell.IgnitionTimeRemaining = 0.0f;
            DeactivateFireVFX(CellIndex);
        }
    }

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    const FColor DebugColor = LeafCell.State == EFireCellState::Burning ? FColor::Red : (LeafCell.State == EFireCellState::Igniting ? FColor::Yellow : FColor::Green);
    //DrawDebugBox(GetWorld(), LeafCell.WorldCenter, LeafCell.CellExtent, FQuat::Identity, DebugColor, false, 0.0f, 0, 1.0f);
#endif

    if (!Result.bAnyBurning)
    {
        Result.bAnyBurning = (LeafCell.State == EFireCellState::Burning || LeafCell.State == EFireCellState::Igniting)&& LeafCell.Heat > 0.0f;
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
                const FVector ChildCenter = Cell.WorldCenter + FVector(OffsetX, OffsetY, OffsetZ);
                const int32 ChildIndex = CreateCell(ChildCenter, ChildExtent, Cell.Depth + 1, CellIndex);

                UE_LOG(LogTemp, Warning, TEXT("Created %d %d %d"), CellIndex, ChildCounter, ChildIndex)
                
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
                UE_LOG(LogTemp, Warning, TEXT("Receive %d %d"), CellIndex, ChildIndex);
                
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
        for (int32 ChildIndex : TargetCell.ChildIndices)
        {
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
            for (int32 ChildIndex : Cells[TargetIndex].ChildIndices)
            {
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

    const float SpawnScale = 2.5f;

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
        SpawnedActor = VFXManager->SpawnVFX(VFXName, SpawnLocation, SpawnRotation, FVector(SpawnScale));
    }

    if (!IsValid(SpawnedActor))
    {
        return;
    }

    SpawnedActor->SetVFXName(VFXName);
    ActiveFireVFXActors.Add(CellIndex, SpawnedActor);
    Cell.ActiveEffect = SpawnedActor;
}

void AFireManager::DeactivateFireVFX(int32 CellIndex)
{
    if (!HasAuthority())
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

    if (Cells.IsValidIndex(CellIndex))
    {
        Cells[CellIndex].ActiveEffect = nullptr;
    }
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

void AFireManager::ApplySuppressionInSphere(const FVector& Center, float Radius, float SuppressionAmount)
{
    if (SuppressionAmount <= 0.0f || Radius <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const double RadiusSquared = FMath::Square(static_cast<double>(Radius));
    for (int32 RootIndex : RootCellIndices)
    {
        ApplySuppressionInSphereRecursive(RootIndex, Center, RadiusSquared, SuppressionAmount);
    }
}

void AFireManager::ApplySuppressionInSphereRecursive(int32 CellIndex, const FVector& Center, double RadiusSquared, float SuppressionAmount)
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
                ApplySuppressionInSphereRecursive(ChildIndex, Center, RadiusSquared, SuppressionAmount);
            }
        }
        return;
    }

    if (ApplySuppressionToCell(CellIndex, SuppressionAmount))
    {
        // nothing extra
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
        bModified = true;

        if (Cell.Heat <= KINDA_SMALL_NUMBER)
        {
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
        bModified = true;

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

