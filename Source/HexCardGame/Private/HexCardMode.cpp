// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/PlayerState.h"
#include "HexCardMode.h"

void AHexCardMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (APlayerState* PlayerState = NewPlayer -> GetPlayerState<APlayerState>())
	{
		PlayerState -> SetPlayerId(NextPlayerID);
		UE_LOG(LogTemp, Log, TEXT("Assign PlayerId %d"), NextPlayerID);
		NextPlayerID++;
	}
}
