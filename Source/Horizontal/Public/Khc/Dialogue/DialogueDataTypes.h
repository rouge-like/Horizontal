#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DialogueDataTypes.generated.h"

/**
 * @brief 대화의 종류를 정의합니다. (일반, 선택지, 종료)
 */
UENUM(BlueprintType)
enum class EDialogueDataType : uint8
{
	Normal      UMETA(DisplayName = "Normal Dialogue"), 	 // 다음 대사로 바로 이어지는 일반 대사
	Choice      UMETA(DisplayName = "Choice Dialogue"), 	 // 플레이어의 선택이 필요한 대사 (미래 확장용)
	End         UMETA(DisplayName = "End Dialogue"),    	 // 이 대화를 끝으로 대화창을 닫음
	EndGood     UMETA(DisplayName = "Success End Dialogue"),    // 이 대화를 끝으로 대화창을 닫음, 성공 선택지
	EndBad      UMETA(DisplayName = "Fail End Dialogue"),    // 이 대화를 끝으로 대화창을 닫음, 실패 선택지
};

USTRUCT(BlueprintType)
struct FChoiceData
{
	GENERATED_BODY()

	// 선택지 버튼에 표시될 텍스트
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Text;

	// 이 선택지를 골랐을 때 점프할 다음 대사의 Label(행 이름)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName JumpToLabel;
};

/**
 * @brief 데이터 테이블의 한 행(Row)에 해당하는 구조체입니다. CSV 컬럼명과 변수명이 일치해야 합니다.
 */
USTRUCT(BlueprintType)
struct FDialogueRow : public FTableRowBase
{
	GENERATED_BODY()

	// 말하는 대상 (예: "NPC", "Player")
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Speaker;

	// 실제 대사 내용
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DialogueText;

	// 대사의 종류 (Normal, Choice, End)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDialogueDataType DialogueType = EDialogueDataType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FChoiceData> Choices;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EventTag;
};