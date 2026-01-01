#pragma once

#include "CoreMinimal.h"
#include "HexRuleChecker.generated.h"

class AHexCardState;

/**
 * 
 */

UCLASS()
class HEXCARDGAME_API UHexRuleChecker : public UObject
{
	GENERATED_BODY()

public:
	
	UPROPERTY()
	AHexCardState* OwnerHexCardState;

	void Initialize(AHexCardState* InGameState)
	{
		OwnerHexCardState = InGameState;
	}

	UFUNCTION()
	bool PlayCardLegalCheck(int PlayerID, int CardInstanceID, int HexQ, int HexR);
	
};
