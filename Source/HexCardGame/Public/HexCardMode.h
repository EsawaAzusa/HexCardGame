// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HexCardState.h"
#include "HexCardMode.generated.h"

/*
 登录设置玩家ID
 */

UCLASS()
class HEXCARDGAME_API AHexCardMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	int32 NextPlayerID = 0;
	
};
