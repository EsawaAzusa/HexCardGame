#pragma once

#include "CoreMinimal.h"
#include "HexCardModel.h"
#include "CardType.generated.h"

/*
ECardZone : 卡牌区域
ECardBuff ：卡牌BUFF。所有的数值更改都通过BUFF传递。
FCardLocation ：卡牌位置的标记，包括卡牌区域与棋盘格位置。卡牌在手牌的位置不影响逻辑层表现，交给表现层处理。
FCardState ：名称，唯一ID，拥有者，BUFF。在GameState上分发其数组作为状态量。
ECardUpdate : 卡牌状态发生更改的类型
FCardStateChangeEvent ： 从卡牌状态改变中提取的改变事件，相关卡牌的唯一ID，序列号，更改类型，移动位置，增减BUFF。在GameState上分发其数组作为客户端动画扳机。
 */

UENUM(BlueprintType)
enum class ECardZone : uint8
{
	Deck,
	Hand,
	Board,
	Graveyard
};

UENUM(BlueprintType)
enum class ECardBuff : uint8
{
	Placeholder
};

USTRUCT(BlueprintType)
struct HEXCARDGAME_API FCardLocation
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	ECardZone Zone = ECardZone::Deck;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int HexR = INT32_MAX;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int HexQ = INT32_MAX;
};

USTRUCT(BlueprintType)
struct HEXCARDGAME_API FCardState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FName CardName = TEXT("NULL");
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int CardInstanceID = -1;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int OwnerPlayerID = -1;
	
	//***************Attributes******************
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int BasePowerA = 1;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int BasePowerB = 1;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int BasePowerC = 1;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int BaseRange = 1;

	//****************Location*******************
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FCardLocation CardLocation;

	//*****************Buff*********************
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<ECardBuff> CardBuffs = {};
};

UENUM(BlueprintType)
enum class ECardUpdate : uint8
{
	Location,
	AddBuff,
	RemoveBuff
};

USTRUCT(BlueprintType)
struct HEXCARDGAME_API FCardStateChangeEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int CardInstanceID;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 StateChangeEventSequenceID;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	ECardUpdate UpdateHint;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FCardLocation StartCardLocation;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FCardLocation EndCardLocation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<ECardBuff> AddedBuffs;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<ECardBuff> RemovedBuffs;
};

USTRUCT(BlueprintType)
struct FHexCardLibrary : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName CardName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AHexCardModel> CardModelClass;
};