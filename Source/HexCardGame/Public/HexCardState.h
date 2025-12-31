// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CardType.h"
#include "EffectInterpreter.h"
#include "HexCardState.generated.h"

/*
GameState类是游戏逻辑层的核心。
TArray<FCardState> CardStates是双方共享的卡牌数组。
TArray<FCardStateChangeEvent> CardStateChangeEvents是双方共享的本地效果扳机。
UEffectInterpreter* EffectInterpreter是效果解释器。一切抽牌等游戏逻辑都是效果，进入解释器，修改CardStates同时抛出StateChangeEvents，同步到客户端。
不经过解释器修改CardStates是非法的！！！
RequestDrawCard(）是由客户端调用的函数。
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCardStateChangeEvent, const FCardStateChangeEvent&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPhaseChangeEvent, const FCardStateChangeEvent&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTurnChangeEvent, const FCardStateChangeEvent&);

class AHexCardController;

UCLASS()
class HEXCARDGAME_API AHexCardState : public AGameStateBase
{
	GENERATED_BODY()

	AHexCardState();
	
	virtual void BeginPlay() override;
	
public:
	//***************************************卡牌模块***********************************************
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated)
	TArray<FCardState> CardStates; 

	FOnCardStateChangeEvent OnCardStateChangeEvent; //客户端Visual Manager订阅
	
	UFUNCTION(NetMulticast, Reliable)
	void CardStateChangeEventDispatch(const FCardStateChangeEvent& Event);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UEffectInterpreter* EffectInterpreter;
	
	int32 GlobalStateChangeSequenceID = 0;
	int32 GlobalEffectQueueID = 0;
	
	UFUNCTION()
	static FCardState GetCardInstancebyID(int CardInstanceID, TArray<FCardState>& CardStatez);
	
	//*************************************游戏进程模块*********************************************
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated)
	int TurnNumber = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, ReplicatedUsing=OnRep_CurrentTurnPlayerID)
	int CurrentTurnPlayerID = -1;

	UFUNCTION()
	void OnRep_CurrentTurnPlayerID();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, ReplicatedUsing=OnRep_CurrentGamePhase)
	EGamePhase GamePhase  = EGamePhase::PreGameAwait;

	UFUNCTION()
	void OnRep_CurrentGamePhase();

	UFUNCTION()
	void AdvancedGamePhase();

	int ReadyClient = 0;

	UFUNCTION()
	void AppendDeck(AHexCardController* OwnerPlayer);
	
	//***************************************操作模块***********************************************	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void RequestChangeTurn(int PlayerID);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void RequestPlayCard(int playerID, int CardInstanceID, int HexQ, int HexR );
	
};
