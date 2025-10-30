#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialParameterCollection.h"
#include "ASmokeManager.generated.h"

UCLASS()
class HORIZONTAL_API ASmokeManager : public AActor
{
	GENERATED_BODY()
public:
	ASmokeManager();

	UPROPERTY(EditAnywhere, Category="Smoke|Config")
	int32 NumFloors = 8;

	UPROPERTY(EditAnywhere, Category="Smoke|Config")
	TArray<float> CeilZ;         

	UPROPERTY(EditAnywhere, Category="Smoke|Config")
	TArray<FVector2D> FlowDirXY;  

	UPROPERTY(EditAnywhere, Category="Smoke|Runtime")
	TArray<float> Density;       

	UPROPERTY(EditAnywhere, Category="Smoke|Runtime")
	TArray<float> SpreadSpeed;    

	UPROPERTY(EditAnywhere, Category="Smoke|Leak")
	TArray<float> LeakToNext;    

	UPROPERTY(EditAnywhere, Category="Smoke|Refs")
	UMaterialParameterCollection* MPC_Smoke = nullptr;

	// 가중치 파라미터
	UPROPERTY(EditAnywhere, Category="Smoke|Tuning")
	float FireAddK = 0.8f;

	UPROPERTY(EditAnywhere, Category="Smoke|Tuning")
	float DecayK = 0.15f;

	UPROPERTY(EditAnywhere, Category="Smoke|Tuning")
	float LeakK = 0.35f;

	// 외부(화재 시스템)에서 층별 강도 세팅
	UFUNCTION(BlueprintCallable, Category="Smoke")
	void SetFireIntensityForFloor(int32 FloorIdx1Based, float Intensity01);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	TArray<float> FireIntensity; // 0~1
	UMaterialParameterCollectionInstance* GetMPCInst() const;

	void PushMPC();
};

