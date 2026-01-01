#include "HexCardState.h"
#include "HexCardController.h"
#include "Algo/RandomShuffle.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

/*
在BeginPlay里注册解释器，解释器只在服务器端存在。
RequestDrawCard展现了基本的解释器注入逻辑，生成空白Effect，先赋予EffectID，Effect类型，必要的参数值和Payload，EffectID全局自增确保唯一
 */


AHexCardState::AHexCardState()
{
	SetReplicates(true);
}

void AHexCardState::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return; 
	
	EffectInterpreter = NewObject<UEffectInterpreter>(this);
	check(EffectInterpreter);
	EffectInterpreter -> Initialize(this);

	RuleChecker = NewObject<UHexRuleChecker>(this);
	check(RuleChecker);
	RuleChecker -> Initialize(this);
	
}

void AHexCardState::CardStateChangeEventDispatch_Implementation(const FCardStateChangeEvent& Event)
{
	OnCardStateChangeEvent.Broadcast(Event); //广播通知Visual Manager
}

FCardState AHexCardState::GetCardInstancebyID(int CardInstanceID, TArray<FCardState>& CardStatez)
{
	for (FCardState idx : CardStatez)
	{
		if (idx.CardInstanceID == CardInstanceID)
		{
			return idx;
		}
	};
	return FCardState();
}

FCardState AHexCardState::GetCardInstancebyHex(int HexQ, int HexR, TArray<FCardState>& CardStatez)
{
	for (FCardState idx : CardStatez)
	{
		if (idx.CardLocation.HexQ == HexQ && idx.CardLocation.HexR == HexR) 
		{
			return idx;
		}
	};
	return FCardState();
}

void AHexCardState::OnRep_CurrentTurnPlayerID()
{
	//在Visual层表现进入下一个回合
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
			FString::Printf(TEXT("Player %d Turn Start."), CurrentTurnPlayerID));
	}
}

void AHexCardState::OnRep_CurrentGamePhase()
{
	//在Visual层表现进入下一个阶段
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
			FString::Printf(TEXT(" %s  Start."),*UEnum::GetValueAsString(GamePhase)));
	}
}

void AHexCardState::AdvancedGamePhase()
{
	switch (GamePhase)
	{
	case EGamePhase::PreGameAwait:
		{
			if (PlayerArray.Num() == 2)	//登录人数达到2
			{
				for (int idx = 0; idx < CardStates.Num(); ++idx)	//遍历编号
				{
					CardStates[idx].CardInstanceID = idx;
				}
				Algo::RandomShuffle(CardStates);
				Algo::RandomShuffle(CardStates);	//打乱两次
				
				if (!EffectInterpreter) return; //没有解释器
	
				FAnyEffect Effect;
				Effect.EffectQueueId = ++GlobalEffectQueueID; //注册效果唯一ID
				Effect.EffectType = EEffectType::ChangePhase; //效果类型
				Effect.Payload = NewObject<UChangePhasePayload>(EffectInterpreter);
				Cast<UChangePhasePayload>(Effect.Payload) -> GamePhase = EGamePhase::GameStart; //设置payload内参数
	
				EffectInterpreter -> PushEffect(Effect);
			}
			break;
		}
	case EGamePhase::GameStart:
		{
			if (ReadyClient < 2) return;
			if (!EffectInterpreter) return; //没有解释器
			
			FAnyEffect Effect;
			Effect.EffectQueueId = ++GlobalEffectQueueID; //注册效果唯一ID
			Effect.EffectType = EEffectType::ChangePhase; //效果类型
			Effect.Payload = NewObject<UChangePhasePayload>(EffectInterpreter);
			Cast<UChangePhasePayload>(Effect.Payload) -> GamePhase = EGamePhase::InGame; //设置payload内参数
	
			EffectInterpreter -> PushEffect(Effect);
		}
	case EGamePhase::InGame:
		{
			break;
		}
	case EGamePhase::GameEnd:
		{
			break;
		}
	}
}

void AHexCardState::AppendDeck(AHexCardController* OwnerPlayer)
{
	TArray<FCardState> NewDeck = OwnerPlayer -> OwningDeck;
	for (FCardState& idx : NewDeck)
	{
		idx.OwnerPlayerID = OwnerPlayer -> PlayerState -> GetPlayerId();
	}
	CardStates.Append(NewDeck);
}

void AHexCardState::RequestChangeTurn_Implementation(int PlayerID)
{
	if (!EffectInterpreter) return; //没有解释器
	if (CurrentTurnPlayerID != PlayerID) return; //非法回合结束请求

	FAnyEffect Effect;
	Effect.EffectQueueId = ++ GlobalEffectQueueID; //注册效果唯一ID
	Effect.EffectType = EEffectType::ChangeTurn; //效果类型
	Effect.SourcePlayerID = PlayerID; //请求来源
	Effect.Payload = NewObject<UChangeTurnPayload>(EffectInterpreter);
	
	EffectInterpreter -> PushEffect(Effect);
}

void AHexCardState::RequestPlayCard_Implementation(int playerID, int CardInstanceID, int HexQ, int HexR)
{
	if (!EffectInterpreter) return;
	if (!RuleChecker -> PlayCardLegalCheck(playerID, CardInstanceID, HexQ, HexR)) return;	//合法性检验

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			10.0f,
			FColor::Green,
			FString::Printf(
				TEXT("RequestPlayCard received | CardID=%d | HexQ=%d | HexR=%d | Mode = %s"),
				CardInstanceID,
				HexQ,
				HexR,
				GetNetMode() == NM_Client ? TEXT("Client") : TEXT("Server")
			)
		);
	}
}

void AHexCardState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AHexCardState, CardStates);
	DOREPLIFETIME(AHexCardState, CurrentTurnPlayerID);
	DOREPLIFETIME(AHexCardState, TurnNumber);
}
