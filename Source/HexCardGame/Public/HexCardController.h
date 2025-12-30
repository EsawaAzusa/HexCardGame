// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VisualManager.h"
#include "HexCardState.h"
#include "HexGrid.h"
#include "HexCardController.generated.h"

UCLASS()
class HEXCARDGAME_API AHexCardController : public APlayerController
{
	GENERATED_BODY()

public:

	AHexCardController();
	
	virtual void BeginPlay() override;

	virtual void OnRep_PlayerState() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* CardLibrary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCardState> OwningDeck;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UVisualManager* VisualManager;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	AHexCardState* HexCardState;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	ACameraActor* CameraActor;
	
	//**************************输入操作*****************************

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	AHexCardModel* CardModel;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	AHexGrid* HexModel;
	
	UFUNCTION(BlueprintCallable)
	void SelectCard();

	UFUNCTION(BlueprintCallable)
	void SelectHex();

	UFUNCTION(Server, Reliable)
	void RequestPlayCard(int CardInstanceID, int HexQ, int HexR);
};
