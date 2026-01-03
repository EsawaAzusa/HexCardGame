#include "HexCardState.h"
#include "EffectInterpreter.h"

void UEffectInterpreter::PushEffect(const FAnyEffect& AddedEffect)
{
	EffectQueue.Push(AddedEffect);
	if (!bIsProcessing)
	{
		ProcessEffectQueue();	//生成并推送
	}
}

void UEffectInterpreter::ProcessEffectQueue()
{
	while (!EffectQueue.IsEmpty()) //循环检测LIFO是否为空
	{
		bIsProcessing = true;
		const FAnyEffect HandleEffect = EffectQueue.Pop();
		InterpreterEffect(HandleEffect);
	}
	bIsProcessing = false;
}

void UEffectInterpreter::InterpreterEffect(const FAnyEffect& HandleEffect)
{
	switch (HandleEffect.EffectType)
	{
		
	case EEffectType::ChangeTurn:
		{
			const UChangeTurnPayload* Payload = Cast<UChangeTurnPayload>(HandleEffect.Payload);
			ExecuteChangeTurn(HandleEffect, Payload);
			break;
		}

	case EEffectType::ChangePhase:
		{
			const UChangePhasePayload* Payload = Cast<UChangePhasePayload>(HandleEffect.Payload);
			ExecuteChangePhase(HandleEffect, Payload);
			break;
		}
		
	case EEffectType::Draw:
		{
			const UDrawPayload* Payload = Cast<UDrawPayload>(HandleEffect.Payload);
			ExecuteDraw(HandleEffect, Payload);
			break;
		}
	case EEffectType::Play:
		{
			const UPlayPayload* Payload = Cast<UPlayPayload>(HandleEffect.Payload);
			ExecutePlay(HandleEffect, Payload);
			break;
		}
	
	case EEffectType::Attack:
		{
			const UAttackPayload* Payload = Cast<UAttackPayload>(HandleEffect.Payload);
			ExecuteAttack(HandleEffect, Payload);
			break;
		}
		
	default:
		{
			//Error, damn!
		}
	}
}

void UEffectInterpreter::ExecuteDraw(const FAnyEffect& HandleEffect, const UDrawPayload* Payload)
{
	if (!OwnerHexCardState) return; //检测CardState和payload是否存在
	if (!Payload) return;

	TArray<int> PlayerIDs = HandleEffect.TargetPlayerIDs;
	int32 DrawCount = Payload -> Count;

	// 为每个触发效果的玩家找出count数量的牌
	for (const int PlayerID : PlayerIDs)
	{
		for (int i = 0; i < DrawCount; ++i)
		{
			FCardState* CardToDraw = nullptr;
			for (FCardState& Card : OwnerHexCardState -> CardStates)
			{
				if (Card.OwnerPlayerID == PlayerID && Card.CardLocation.Zone == ECardZone::Deck)
				{
					CardToDraw = &Card;
					break;
				}
			}

			if (!CardToDraw) OwnerHexCardState -> CheckWinner(); //不存在就跳出

			// 调整CardState状态
			CardToDraw->CardLocation.Zone = ECardZone::Hand;

			// 生成相应的的ChangeEvent
			FCardStateChangeEvent Event;
			Event.CardInstanceID = CardToDraw -> CardInstanceID;
			Event.UpdateHint = ECardUpdate::Location;
			Event.StartCardLocation.Zone = ECardZone::Deck;
			Event.EndCardLocation.Zone = ECardZone::Hand;

			// 转发Event
			Event.StateChangeEventSequenceID = ++OwnerHexCardState -> GlobalStateChangeSequenceID;
			OwnerHexCardState -> CardStateChangeEventDispatch(Event);
		}
	}
}

void UEffectInterpreter::ExecutePlay(const FAnyEffect& HandleEffect, const UPlayPayload* Payload)
{
	if (!OwnerHexCardState) return; //检测CardState和payload是否存在
	if (!Payload) return;

	FCardState* CardToPlay = nullptr;
	for (FCardState& Card : OwnerHexCardState -> CardStates)
	{
		if (Card.CardInstanceID == HandleEffect.TargetCardInstanceIDs[0])
		{
			CardToPlay = &Card;
			break;
		}
	}
	
	if (!CardToPlay) return; //不存在就跳出
	
	// 调整CardState状态
	CardToPlay->CardLocation.Zone = ECardZone::Board;
	CardToPlay->CardLocation.HexQ = Payload -> HexQ;
	CardToPlay->CardLocation.HexR = Payload -> HexR;
	
	// 生成相应的的ChangeEvent
	FCardStateChangeEvent Event;
	Event.CardInstanceID = CardToPlay -> CardInstanceID;
	Event.UpdateHint = ECardUpdate::Location;
	Event.StartCardLocation.Zone = ECardZone::Hand;
	Event.EndCardLocation.Zone = ECardZone::Board;

	// 转发Event
	Event.StateChangeEventSequenceID = ++OwnerHexCardState -> GlobalStateChangeSequenceID;
	OwnerHexCardState -> CardStateChangeEventDispatch(Event);

	TArray<FIntPoint> Directions =
			{
			FIntPoint(1, 0),
			FIntPoint(-1, 0),
			FIntPoint(0, 1),
			FIntPoint(0, -1),
			FIntPoint(-1, 1),
			FIntPoint(1, -1)
			} ;

	for (const FIntPoint& Direction : Directions) //检测是否爆发战斗
	{
		const int NeigborHexQ =  Payload -> HexQ + Direction.X;
		const int NeigborHexR =  Payload -> HexR + Direction.Y;
		FCardState Find = OwnerHexCardState -> GetCardInstancebyHex(NeigborHexQ, NeigborHexR, OwnerHexCardState -> CardStates);
	
		if (Find.IsValid() && Find.OwnerPlayerID != CardToPlay -> OwnerPlayerID)  
		{
			FAnyEffect Effect;
			Effect.EffectQueueId = ++ OwnerHexCardState -> GlobalEffectQueueID;
			Effect.EffectType = EEffectType::Attack; 
			Effect.SourceCardInstanceID = CardToPlay ->  CardInstanceID;
			Effect.TargetCardInstanceIDs.Add(Find.CardInstanceID);
			Effect.Payload = NewObject<UAttackPayload>(this);
			PushEffect(Effect);
		}
	}
}

void UEffectInterpreter::ExecuteAttack(const FAnyEffect& HandleEffect, const UAttackPayload* Payload)
{
	if (!OwnerHexCardState) return; //检测CardState和payload是否存在
	if (!Payload) return;

	FCardState* CardToAttack = nullptr;
	for (FCardState& Card : OwnerHexCardState -> CardStates)
	{
		if (Card.CardInstanceID == HandleEffect.TargetCardInstanceIDs[0])
		{
			CardToAttack = &Card;
			break;
		}
	}
	
	if (!CardToAttack) return; //不存在就跳出
	
	// 调整CardState状态
	CardToAttack -> CardLocation.Zone = ECardZone::Graveyard;
	CardToAttack -> CardLocation.HexQ = INT_MAX;
	CardToAttack -> CardLocation.HexR = INT_MAX;
	
	// 生成相应的的ChangeEvent
	FCardStateChangeEvent Event;
	Event.CardInstanceID = CardToAttack -> CardInstanceID;
	Event.UpdateHint = ECardUpdate::Location;
	Event.StartCardLocation.Zone = ECardZone::Board;
	Event.EndCardLocation.Zone = ECardZone::Graveyard;

	// 转发Event
	Event.StateChangeEventSequenceID = ++OwnerHexCardState -> GlobalStateChangeSequenceID;
	OwnerHexCardState -> CardStateChangeEventDispatch(Event);
	
}

void UEffectInterpreter::ExecuteChangeTurn(const FAnyEffect& HandleEffect, const UChangeTurnPayload* Payload)
{
	if (!OwnerHexCardState) return;
	if (OwnerHexCardState -> CurrentTurnPlayerID == -1)
	{
		OwnerHexCardState -> CurrentTurnPlayerID = 0;
	}
	else
	{
		FAnyEffect Effect;
		Effect.EffectQueueId = ++ OwnerHexCardState -> GlobalEffectQueueID; //进入自己的回合时触发抽牌
		Effect.EffectType = EEffectType::Draw; 
		Effect.TargetPlayerIDs.Add( HandleEffect.SourcePlayerID ? 0 : 1);
		Effect.Payload = NewObject<UDrawPayload>(this);
		Cast<UDrawPayload>(Effect.Payload) -> Count = 1; 
		PushEffect(Effect);
		OwnerHexCardState -> CurrentTurnPlayerID = HandleEffect.SourcePlayerID ? 0 : 1;
	}
	
	OwnerHexCardState -> TurnNumber++;
	OwnerHexCardState -> AdvancedGamePhase();
}

void UEffectInterpreter::ExecuteChangePhase(const FAnyEffect& HandleEffect, const UChangePhasePayload* Payload)
{
	if (!OwnerHexCardState) return;

	if (Payload->GamePhase == EGamePhase::InGame) //在进入InGame时触发抽牌
	{
		FAnyEffect Effect;
		Effect.EffectQueueId = ++ OwnerHexCardState -> GlobalEffectQueueID; //起始手牌
		Effect.EffectType = EEffectType::Draw; 
		Effect.TargetPlayerIDs.Add(0);
		Effect.TargetPlayerIDs.Add(1);
		Effect.Payload = NewObject<UDrawPayload>(this);
		Cast<UDrawPayload>(Effect.Payload) -> Count = 5; 
		PushEffect(Effect);

		FAnyEffect Effect_2;
		Effect_2.EffectQueueId = ++OwnerHexCardState ->  GlobalEffectQueueID; 
		Effect_2.EffectType = EEffectType::ChangeTurn; 
		Effect_2.SourcePlayerID = 0; 
		Effect_2.Payload = NewObject<UChangeTurnPayload>(this);
		PushEffect(Effect_2);
	}
	
	OwnerHexCardState -> GamePhase = Payload -> GamePhase;
}
