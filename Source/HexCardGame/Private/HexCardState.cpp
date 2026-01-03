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
	OnTurnChangeEvent.Broadcast(CurrentTurnPlayerID, TurnNumber); //广播通知Visual Manager
}

void AHexCardState::OnRep_CurrentGamePhase()
{
	OnPhaseChangeEvent.Broadcast(GamePhase); //广播通知Visual Manager
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
			break;
		}
	case EGamePhase::InGame:
		{
			CheckWinner();
			break;
		}
	case EGamePhase::GameEnd:
		{
			break;
		}
	}
}

void AHexCardState::CheckWinner()
{
	const TArray<FIntPoint> Nodes =
		{
		FIntPoint(0,2),
		FIntPoint(0,0),
		FIntPoint(0,-2),
		};
		
			if (TurnNumber == 40)	//回合数满
			{
				int CaptureP0 = 0;
				int CaptureP1 = 0;

				for (FIntPoint Node : Nodes)
				{
					FCardState Find = GetCardInstancebyHex(Node.X, Node.Y, CardStates);
					if (Find.IsValid())
					{
						Find.OwnerPlayerID ? CaptureP1 ++ : CaptureP0 ++;
					}
				}
				if (CaptureP0 > CaptureP1)
				{
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
							FString::Printf(TEXT(" Player 0 Win!")));
					}
				}
				else if(CaptureP0 < CaptureP1)
				{
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
							FString::Printf(TEXT(" Player 1 Win!")));
					}
				}
				else
				{
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
							FString::Printf(TEXT(" Draw Game!")));
					}
				}
				FAnyEffect Effect;
				Effect.EffectQueueId = ++GlobalEffectQueueID; //注册效果唯一ID
				Effect.EffectType = EEffectType::ChangePhase; //效果类型
				Effect.Payload = NewObject<UChangePhasePayload>(EffectInterpreter);
				Cast<UChangePhasePayload>(Effect.Payload) -> GamePhase = EGamePhase::GameEnd; //设置payload内参数
	
				EffectInterpreter -> PushEffect(Effect);
			}
			else
			{
				int CaptureP0 = 0;
				int CaptureP1 = 0;

				for (FIntPoint Node : Nodes)
				{
					FCardState Find = GetCardInstancebyHex(Node.X, Node.Y, CardStates);
					if (Find.IsValid())
					{
						Find.OwnerPlayerID ? CaptureP1 ++ : CaptureP0 ++;
					}
				}
				
				if (CurrentTurnPlayerID == 0 && CaptureP0 == 3)
				{
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
							FString::Printf(TEXT(" Player 0 Win!")));
					}
					FAnyEffect Effect;
					Effect.EffectQueueId = ++GlobalEffectQueueID; //注册效果唯一ID
					Effect.EffectType = EEffectType::ChangePhase; //效果类型
					Effect.Payload = NewObject<UChangePhasePayload>(EffectInterpreter);
					Cast<UChangePhasePayload>(Effect.Payload) -> GamePhase = EGamePhase::GameEnd; //设置payload内参数
	
					EffectInterpreter -> PushEffect(Effect);
				}
				if (CurrentTurnPlayerID == 1 && CaptureP1 == 3)
				{
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
							FString::Printf(TEXT(" Player 1 Win!")));
					}
					FAnyEffect Effect;
					Effect.EffectQueueId = ++GlobalEffectQueueID; //注册效果唯一ID
					Effect.EffectType = EEffectType::ChangePhase; //效果类型
					Effect.Payload = NewObject<UChangePhasePayload>(EffectInterpreter);
					Cast<UChangePhasePayload>(Effect.Payload) -> GamePhase = EGamePhase::GameEnd; //设置payload内参数
	
					EffectInterpreter -> PushEffect(Effect);
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
	
	FAnyEffect Effect;
	Effect.EffectQueueId = ++GlobalEffectQueueID; //注册效果唯一ID
	Effect.EffectType = EEffectType::Play; //效果类型
	Effect.SourcePlayerID = playerID;
	Effect.TargetCardInstanceIDs.Add(CardInstanceID);
	Effect.Payload = NewObject<UPlayPayload>(EffectInterpreter);
	Cast<UPlayPayload>(Effect.Payload) -> HexQ = HexQ; //设置payload内参数
	Cast<UPlayPayload>(Effect.Payload) -> HexR = HexR; //设置payload内参数
	
	EffectInterpreter -> PushEffect(Effect);
}

void AHexCardState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AHexCardState, CardStates);
	DOREPLIFETIME(AHexCardState, CurrentTurnPlayerID);
	DOREPLIFETIME(AHexCardState, TurnNumber);
}
