#include "VisualManager.h"

#include "HexCardController.h"
#include "Kismet/GameplayStatics.h"

UVisualManager::UVisualManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVisualManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Locate();
}

void UVisualManager::Locate()
{
	TArray<AHexCardModel*> Hand_P0;
	TArray<AHexCardModel*> Hand_P1;
	TArray<AHexCardModel*> Board;
	//分选模型，组建数组
	for (AHexCardModel* idx : HexCardModels)
	{
		FCardState Owner = Cast<AHexCardController>(GetOwner()) -> HexCardState -> GetCardInstancebyID(idx -> CardInstanceID,Cast<AHexCardController>(GetOwner()) -> HexCardState -> CardStates);
		if (Owner.IsValid())
		{
			switch (Owner.CardLocation.Zone)
			{
			case ECardZone::Hand:
				if (Owner.OwnerPlayerID == 0)
					Hand_P0.Add(idx);
				else
					Hand_P1.Add(idx);
				break;
			case ECardZone::Board:
				Board.Add(idx);
				break;
			default:
				break;
			}
		}
	}
	//开始更新
	UpdateHand_P0(Hand_P0);
	UpdateHand_P1(Hand_P1);
	UpdateBoard(Board);
}

void UVisualManager::UpdateHand_P0(TArray<AHexCardModel*> Hand)
{
	//临时美术设置
	const FVector Start(-1000.f, -700.f, 1000.f);
	const FVector End  (-1000.f,  700.f, 1000.f);

	//魔术方法，好孩子不要学
	Hand.Insert(nullptr, 0);
	Hand.Add(nullptr);
	const int N = Hand.Num();
	
	for (int idx = 0; idx < N; ++idx)
	{
		const float Alpha = static_cast<float>(idx) / static_cast<float>(N - 1);
		const FVector Pos = FMath::Lerp(Start, End, Alpha);
		if (Hand[idx])
		{
			Hand[idx] -> TargetLocation = Pos;
			Hand[idx] -> MoveMode = EMoveMode::Interp;
		}
	}
}

void UVisualManager::UpdateHand_P1(TArray<AHexCardModel*> Hand)
{
	//临时美术设置
	const FVector Start(1000.f, 700.f, 1000.f);
	const FVector End  (1000.f,  -700.f, 1100.f);

	//魔术方法，好孩子不要学
	Hand.Insert(nullptr, 0);
	Hand.Add(nullptr);
	const int N = Hand.Num();
	
	for (int idx = 0; idx < N; ++idx)
	{
		const float Alpha = static_cast<float>(idx) / static_cast<float>(N - 1);
		const FVector Pos = FMath::Lerp(Start, End, Alpha);
		if (Hand[idx])
		{
			Hand[idx] -> TargetLocation = Pos;
			Hand[idx] -> MoveMode = EMoveMode::Interp;
			Hand[idx]-> InterpSpeed = 15.f;
		}
	}
}

void UVisualManager::UpdateBoard(TArray<AHexCardModel*> Board)
{
	for (AHexCardModel* CardModel : Board)
	{
		FCardState CardState = Cast<AHexCardController>(GetOwner()) -> HexCardState -> GetCardInstancebyID(CardModel -> CardInstanceID,Cast<AHexCardController>(GetOwner()) -> HexCardState -> CardStates);
		if (!CardState.IsValid()) continue;
		const int HexQ = CardState.CardLocation.HexQ;
		const int HexR= CardState.CardLocation.HexR;
		
		TArray<AActor*> HexGrids;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHexGrid::StaticClass(), HexGrids);
		for (AActor* Actor : HexGrids)
		{
			AHexGrid* HexGrid = Cast<AHexGrid>(Actor); 
			if (HexGrid)
			{
				if (HexGrid -> HexQ == HexQ && HexGrid -> HexR == HexR)
				{
					CardModel -> TargetLocation = HexGrid -> GetActorLocation() + FVector (0.f, 0.f, 10.f);
					CardModel -> MoveMode = EMoveMode::Interp;
					CardModel -> InterpSpeed = 30.f;
					break;
				}
			}
		}
	}
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
	if(Event.StartCardLocation.Zone == ECardZone::Hand && Event.EndCardLocation.Zone == ECardZone::Board)
	{
		DemoPlayCard(Event);
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
	FCardState CardState = Cast<AHexCardController>(GetOwner()) -> HexCardState -> GetCardInstancebyID(Event.CardInstanceID,Cast<AHexCardController>(GetOwner()) -> HexCardState -> CardStates);
	if (!CardState.IsValid()) return;
	const FName CardName = CardState.CardName;
	
	//获取Card模型结构体
	const FHexCardLibrary* Card =Cast<AHexCardController>(GetOwner()) ->  CardLibrary -> FindRow<FHexCardLibrary>(CardName, TEXT("Invalid Row Name"));
	if (!Card || Card -> CardModelClass.IsNull()) return;

	//解软引用
	UClass* CardModel = Card -> CardModelClass.LoadSynchronous();
	if (!CardModel) return;

	//生成位置（仅供测试，假设为P0控制）
	FVector Location = CardState.OwnerPlayerID  ?  FVector(1000.f, -800.f, 1000.f) : FVector(-1000.f, 800.f, 1000.f) ;
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(Location);
	SpawnTransform.SetRotation(FQuat::Identity);
	SpawnTransform.SetScale3D(FVector::OneVector);
	
	//生成Model，在Models数组中集中统一管理
	AHexCardModel* NewCardModel = GetWorld() -> SpawnActor<AHexCardModel>(CardModel, SpawnTransform);
	NewCardModel -> Initialize(Event.CardInstanceID);
	HexCardModels.Add(NewCardModel);
	EventFinishCallback(Event.StateChangeEventSequenceID);
}

void UVisualManager::DemoPlayCard(const FCardStateChangeEvent& Event)
{
	for (AHexCardModel* idx : HexCardModels)
	{
		FCardState Owner = Cast<AHexCardController>(GetOwner()) -> HexCardState -> GetCardInstancebyID(idx -> CardInstanceID,Cast<AHexCardController>(GetOwner()) -> HexCardState -> CardStates);
		if (Event.CardInstanceID == Owner.CardInstanceID)
		{
			idx -> CardMesh -> SetVisibility(false);
			idx -> HexMesh -> SetVisibility(true);
		}
	}
		EventFinishCallback(Event.StateChangeEventSequenceID);
}