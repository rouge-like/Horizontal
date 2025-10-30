
#include "KNY/ASmokeManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Materials/MaterialParameterCollectionInstance.h"

ASmokeManager::ASmokeManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ASmokeManager::BeginPlay()
{
    Super::BeginPlay();
    CeilZ.SetNum(NumFloors);
    FlowDirXY.SetNum(NumFloors);
    Density.Init(0.f, NumFloors);
    SpreadSpeed.Init(1.f, NumFloors);
    LeakToNext.Init(0.f, FMath::Max(NumFloors-1, 0));
    FireIntensity.Init(0.f, NumFloors);
}

void ASmokeManager::SetFireIntensityForFloor(int32 FloorIdx1Based, float Intensity01)
{
    int32 i = FloorIdx1Based - 1;
    if (!FireIntensity.IsValidIndex(i)) return;
    FireIntensity[i] = FMath::Clamp(Intensity01, 0.f, 1.f);
}

UMaterialParameterCollectionInstance* ASmokeManager::GetMPCInst() const
{
    if (!GetWorld() || !MPC_Smoke) return nullptr;
    return GetWorld()->GetParameterCollectionInstance(MPC_Smoke);
}

void ASmokeManager::Tick(float dt)
{
    Super::Tick(dt);
    // 1) 시뮬레이션
    for (int i=0; i<NumFloors; ++i)
    {
        // 입력 강도 → 밀도 적분
        Density[i] += FireAddK * FireIntensity[i] * dt;
        Density[i] -= DecayK * Density[i] * dt;
        Density[i] = FMath::Clamp(Density[i], 0.f, 1.f);
    }
    // 2) 상층 유입
    for (int i=0; i<NumFloors-1; ++i)
    {
        float leak = LeakK * Density[i] * FMath::Clamp(LeakToNext[i], 0.f, 1.f);
        Density[i+1] = FMath::Clamp(Density[i+1] + leak * dt, 0.f, 1.f);
    }
    // 3) MPC 반영
    PushMPC();
}

static FORCEINLINE FName PName(const FString& Base, int32 Floor1)
{
    return FName(*FString::Printf(TEXT("%s_F%d"), *Base, Floor1));
}

void ASmokeManager::PushMPC()
{
    if (auto* Inst = GetMPCInst())
    {
        for (int f=1; f<=NumFloors; ++f)
        {
            const int i = f-1;
            Inst->SetScalarParameterValue(PName("SmokeDensity", f), Density[i]);
            Inst->SetScalarParameterValue(PName("CeilHeight",   f), CeilZ.IsValidIndex(i)? CeilZ[i] : 0.f);
            Inst->SetScalarParameterValue(PName("SpreadSpeed",  f), SpreadSpeed.IsValidIndex(i)? SpreadSpeed[i] : 1.f);

            const FVector2D Dir = FlowDirXY.IsValidIndex(i) ? FlowDirXY[i] : FVector2D(1,0);
            const FVector V(Dir.X, Dir.Y, 0.f);
            Inst->SetVectorParameterValue(PName("FlowDir", f), FLinearColor(V.X, V.Y, V.Z));
        }
    }
}

