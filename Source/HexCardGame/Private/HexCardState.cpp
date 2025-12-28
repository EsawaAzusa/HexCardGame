#include "HexCardState.h"
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
	
	//************************测试用添加五张牌****************************
	for (int idx = 0; idx < 5; ++idx)
	{
		FCardState NewCard;
		NewCard.CardName = FName(TEXT("TestCard_%d"), idx);
		NewCard.CardInstanceID = idx;
		NewCard.OwnerPlayerID = 0;               // 玩家0
		NewCard.CardLocation.Zone = ECardZone::Deck;
		CardStates.Add(NewCard);
	}
	//*****************************测试用**********************************
}

void AHexCardState::CardStateChangeEventDispatch_Implementation(const FCardStateChangeEvent& Event)
{
	OnCardStateChangeEvent.Broadcast(Event); //广播通知Visual Manager
}

void AHexCardState::RequestDrawCard_Implementation(int PlayerID)
{
	//典型的Effect注入请求，可参考
	
	if (!EffectInterpreter) return; //没有解释器
	
	FAnyEffect Effect;
	Effect.EffectQueueId = ++GlobalEffectQueueID; //注册效果唯一ID
	Effect.EffectType = EEffectType::Draw; //效果类型
	Effect.TargetPlayerIDs.Add(PlayerID);	//效果参数
	Effect.Payload = NewObject<UDrawPayload>(EffectInterpreter);
	Cast<UDrawPayload>(Effect.Payload) -> Count = 1; //设置payload内参数
	
	EffectInterpreter -> PushEffect(Effect);
	EffectInterpreter -> ProcessEffectQueue();	//生成并推送
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

void AHexCardState::RequestChangeTurn_Implementation(int PlayerID)
{
	if (!EffectInterpreter) return; //没有解释器

	if (CurrentTurnPlayerID != PlayerID) return; //非法回合结束请求

	FAnyEffect Effect;
	Effect.EffectQueueId = ++GlobalStateChangeSequenceID; //注册效果唯一ID
	Effect.EffectType = EEffectType::ChangeTurn; //效果类型
	Effect.SourcePlayerID = PlayerID; //请求来源
	Effect.Payload = NewObject<UChangeTurnPayload>(EffectInterpreter);
	
	EffectInterpreter -> PushEffect(Effect);
	EffectInterpreter -> ProcessEffectQueue();	//生成并推送
	
}

void AHexCardState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AHexCardState, CardStates);
	DOREPLIFETIME(AHexCardState, CurrentTurnPlayerID);
	DOREPLIFETIME(AHexCardState, TurnNumber);
}
