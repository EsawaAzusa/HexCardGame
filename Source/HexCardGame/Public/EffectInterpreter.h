#pragma once

#include "CoreMinimal.h"
#include "EffectInterpreter.generated.h"

class AHexCardState;

/*
 解释器是逻辑层的核心/
EEffectType ：效果类型。一切逻辑操作都是一种效果，不直接作用于CardState，而是进入FIFO等待解释。
UEffectPayload : 效果payload。根据效果类型初始化。payload不包含已经在FAnyEffect中存在的参数（如效果的发起者、接收者）
FAnyEffect ：效果结构体，包括效果唯一ID（用于重播），效果类型，一些必须的参数，payload。
基本函数包括push，process，interpreter。
基于EEffectType调用Execute执行函数，如ExecuteDraw。
 */

UENUM(BlueprintType)
enum class EEffectType : uint8
{
	ChangeTurn,
	ChangePhase,
	Draw,
	Play,
	Attack
};

// ************** Virtual Payload ****************
UCLASS(BlueprintType)
class HEXCARDGAME_API UEffectPayload : public UObject
{
	GENERATED_BODY()
public:
	virtual ~UEffectPayload() override {};
};

// ************ ChangeTurn Payload *************
UCLASS()
class UChangeTurnPayload : public UEffectPayload
{
	GENERATED_BODY()
};

// ************ ChangePhase Payload *************
UCLASS()
class UChangePhasePayload : public UEffectPayload
{
	GENERATED_BODY()

public:

	EGamePhase GamePhase;	//新游戏阶段
	
};

// ***************** Draw Payload ****************
UCLASS()
class UDrawPayload : public UEffectPayload
{
	GENERATED_BODY()

public:
	
	int Count = 1;	//抽牌数量
	
};

// ***************** Play Payload ****************
UCLASS()
class UPlayPayload : public UEffectPayload
{
	GENERATED_BODY()
};

// ***************** Attack Payload **************
UCLASS()
class UAttackPayload : public UEffectPayload
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FAnyEffect
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 EffectQueueId = INT32_MAX;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	EEffectType EffectType;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int SourceCardInstanceID = INT_MAX;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int SourcePlayerID = INT_MAX;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<int> TargetCardInstanceIDs = {};

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<int> TargetPlayerIDs = {};

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UEffectPayload* Payload;
};

UCLASS()

class HEXCARDGAME_API UEffectInterpreter : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY()
	AHexCardState* OwnerHexCardState;

	void Initialize(AHexCardState* InGameState)
	{
		OwnerHexCardState = InGameState;
	}
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FAnyEffect> EffectQueue;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bIsProcessing = false;

	UFUNCTION()
	void PushEffect(const FAnyEffect& AddedEffect);
	
	UFUNCTION()
	void ProcessEffectQueue();

	UFUNCTION()
	void InterpreterEffect(const FAnyEffect& HandleEffect);

	UFUNCTION()
	void ExecuteDraw(const FAnyEffect& HandleEffect, const UDrawPayload* Payload);

	UFUNCTION()
	void ExecuteChangeTurn(const FAnyEffect& HandleEffect, const UChangeTurnPayload* Payload);

	UFUNCTION()
	void ExecuteChangePhase(const FAnyEffect& HandleEffect, const UChangePhasePayload* Payload);
	
};
