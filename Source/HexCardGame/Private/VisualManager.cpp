#include "VisualManager.h"

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
	//********************************************测试用*********************************************
	if (!IsNetMode(NM_Client)) return;
	if (!GetOwner() || !Cast<APlayerController>(GetOwner())->IsLocalController()) return;
	
	FString EventInfo = FString::Printf(
	 TEXT("Event SeqID: %d | CardID: %d | UpdateHint: %s | StartZone: %s | EndZone: %s"),
	 Event.StateChangeEventSequenceID,
	 Event.CardInstanceID,
	 *UEnum::GetValueAsString(Event.UpdateHint),
	 *UEnum::GetValueAsString(Event.StartCardLocation.Zone),
	 *UEnum::GetValueAsString(Event.EndCardLocation.Zone)
	);

	// 打印到屏幕上，持续50秒
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.f, FColor::Yellow, EventInfo);
	}

	// 打到输出日志
	UE_LOG(LogTemp, Log, TEXT("%s"), *EventInfo);
	EventFinishCallback(Event.StateChangeEventSequenceID);
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

