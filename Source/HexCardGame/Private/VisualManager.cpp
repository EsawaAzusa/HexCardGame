#include "VisualManager.h"

#include "HexCardController.h"

UVisualManager::UVisualManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVisualManager::Initialize(AHexCardState* InGameState)
{
	return;
}

void UVisualManager::OnCardStateChangeEvent(const FCardStateChangeEvent& NewEvent)
{
	if (NewEvent.StateChangeEventSequenceID <= HandledEventSequenceID) return;
	UnhandledCardStateChangeEvents.Add(NewEvent);
	if(!bIsProcessing)
	{
		ProcessNextEvent();
	}
}

void UVisualManager::SortBySequenceID(TArray<FCardStateChangeEvent>& Events)
{
		Algo::Sort(Events, [](const FCardStateChangeEvent& A, const FCardStateChangeEvent& B){return A.StateChangeEventSequenceID < B.StateChangeEventSequenceID;});
}

void UVisualManager::ProcessNextEvent()
{
	SortBySequenceID(UnhandledCardStateChangeEvents);
	const int32 NextSequenceID = HandledEventSequenceID + 1;
	for (int32 idx = UnhandledCardStateChangeEvents.Num() - 1; idx >= 0; --idx)	//SequenceID较大，采用逆序查找加速
	{
		FCardStateChangeEvent& Event = UnhandledCardStateChangeEvents[idx];
		if (Event.StateChangeEventSequenceID == NextSequenceID)
		{
			HandleEvent(Event);
			break;
		}
	}
	//如果一次Process未命中，则在后续接收事件时，在OnCardStateChangeEvent中触发新一轮Process
}

void UVisualManager::HandleEvent(const FCardStateChangeEvent& Event)
{
	bIsProcessing = true;
	switch (Event.UpdateHint)
	{
	case ECardUpdate::Location:
		HandleLocationEvent(Event);
		break;
	case ECardUpdate::AddBuff:
		HandleAddBuffEvent(Event);
		break;
	case ECardUpdate::RemoveBuff:
		HandleRemoveBuffEvent(Event);
		break;
	}
}

void UVisualManager::HandleLocationEvent(const FCardStateChangeEvent& Event)
{
	if (!IsNetMode(NM_Client)) return;
	if (!GetOwner() || !Cast<APlayerController>(GetOwner())->IsLocalController()) return;

	if(Event.StartCardLocation.Zone == ECardZone::Deck && Event.EndCardLocation.Zone == ECardZone::Hand)
	{
		DemoDrawCard(Event);
		return;
	}
	//************************************************************************************************
}

void UVisualManager::HandleAddBuffEvent(const FCardStateChangeEvent& Event)
{
	return;
}

void UVisualManager::HandleRemoveBuffEvent(const FCardStateChangeEvent& Event)
{
	
}

void UVisualManager::EventFinishCallback(int32 SequenceID)
{
	for (int32 idx = UnhandledCardStateChangeEvents.Num() - 1; idx >= 0; --idx)
	{
		if (SequenceID == UnhandledCardStateChangeEvents[idx].StateChangeEventSequenceID)
		{
			// 从未处理数组移除，添加到冻结数组
			const FCardStateChangeEvent& Event = UnhandledCardStateChangeEvents[idx];
			UnhandledCardStateChangeEvents.RemoveAt(idx);
			FrozenCardStateChangeEvents.Add(Event);
			break;
		}
	}
	
	HandledEventSequenceID = SequenceID;
	bIsProcessing = false;
	ProcessNextEvent();
}

void UVisualManager::OnTurnChanged(int NewTurnPlayerID)
{
	return;
}

void UVisualManager::DemoDrawCard(const FCardStateChangeEvent& Event)
{
	//使用卡牌ID获取CardName
	FName CardName = NAME_None;
	for (const FCardState& idx : Cast<AHexCardController>(GetOwner()) -> HexCardState -> CardStates)
	{
		if (idx.CardInstanceID == Event.CardInstanceID)
		{
			CardName = idx.CardName;
			break;
		}
	}
	
	//获取Card模型结构体
	const FHexCardLibrary* Card =Cast<AHexCardController>(GetOwner()) ->  CardLibrary -> FindRow<FHexCardLibrary>(CardName, TEXT("Invalid Row Name"));
	if (!Card || Card -> CardModelClass.IsNull()) return;

	//解软引用
	UClass* CardModel = Card -> CardModelClass.LoadSynchronous();
	if (!CardModel) return;

	//生成位置（仅供测试）
	FVector Location = FVector(-700.f, 0.f, 1000.f);
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(Location);
	SpawnTransform.SetRotation(FQuat::Identity);
	SpawnTransform.SetScale3D(FVector::OneVector);
	
	//生成！
	GetWorld() -> SpawnActor<AHexCardModel>(CardModel, SpawnTransform);
	EventFinishCallback(Event.StateChangeEventSequenceID);
}

void UVisualManager::DemoPlayCard(const FCardStateChangeEvent& Event)
{
	
}