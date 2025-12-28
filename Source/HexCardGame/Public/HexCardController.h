// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VisualManager.h"
#include "HexCardState.h"
#include "HexCardController.generated.h"

UCLASS()
class HEXCARDGAME_API AHexCardController : public APlayerController
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UVisualManager* VisualManager;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	AHexCardState* HexCardState;
	
};
