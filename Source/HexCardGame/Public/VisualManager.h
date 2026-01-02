#pragma once

#include "CoreMinimal.h"
#include "CardType.h"
#include "Components/ActorComponent.h"
#include "VisualManager.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HEXCARDGAME_API UVisualManager : public UActorComponent
{
	GENERATED_BODY()

public:	

	UVisualManager();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	//**********************更新表现*****************************
	UFUNCTION()
	void Locate();
	
	UFUNCTION()
	void UpdateHand_P0(TArray<AHexCardModel*> Hand);

	UFUNCTION()
	void UpdateHand_P1(TArray<AHexCardModel*> Hand);

	UFUNCTION()
	void UpdateBoard(TArray<AHexCardModel*> Board);
	//**********************更新表现*****************************
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FCardStateChangeEvent> FrozenCardStateChangeEvents;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FCardStateChangeEvent> UnhandledCardStateChangeEvents;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<AHexCardModel*> HexCardModels;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 HandledEventSequenceID = 0; //初始时事件ID为0

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bIsProcessing = false;

	UFUNCTION()
	void Initialize(AHexCardState* InGameState);

	UFUNCTION()
	void OnCardStateChangeEvent(const FCardStateChangeEvent& NewEvent);

	UFUNCTION()
	static void SortBySequenceID(TArray<FCardStateChangeEvent>& Events);

	UFUNCTION()
	void ProcessNextEvent();

	UFUNCTION()
	void HandleEvent(const FCardStateChangeEvent& Event);

	UFUNCTION()
	void HandleLocationEvent(const FCardStateChangeEvent& Event);

	UFUNCTION()
	void HandleAddBuffEvent(const FCardStateChangeEvent& Event);

	UFUNCTION()
	void HandleRemoveBuffEvent(const FCardStateChangeEvent& Event);
	
	UFUNCTION()
	void EventFinishCallback(int32 SequenceID);

	UFUNCTION()
	void OnTurnChanged(int NewTurnPlayerID);

	UFUNCTION()
	void DemoDrawCard(const FCardStateChangeEvent& Event);

	UFUNCTION()
	void DemoPlayCard(const FCardStateChangeEvent& Event);

	UFUNCTION()
	void DemoRemoveCard(const FCardStateChangeEvent& Event);
	
};
