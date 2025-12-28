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

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FCardStateChangeEvent> FrozenCardStateChangeEvents;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FCardStateChangeEvent> UnhandledCardStateChangeEvents;

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
	
};
