// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HexGrid.generated.h"

UCLASS()
class HEXCARDGAME_API AHexGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHexGrid();
	
	UPROPERTY(EditAnywhere)
	int HexQ;
	UPROPERTY(EditAnywhere)
	int HexR;

};
