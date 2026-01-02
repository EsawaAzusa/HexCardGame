#include "HexCardModel.h"
#include "CardType.h"

// Sets default values
AHexCardModel::AHexCardModel()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	CardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CardMesh"));
	CardMesh -> SetupAttachment(RootComponent);  
	HexMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeXMesh"));
	HexMesh -> SetupAttachment(RootComponent);
	CardMesh -> SetVisibility(true);
	HexMesh -> SetVisibility(false);
}

void AHexCardModel::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	//位置自动更新
	if (MoveMode == EMoveMode::Interp)
	{
		const FVector Current = GetActorLocation();

		const FVector NewLocation =
			FMath::VInterpTo(Current, TargetLocation, DeltaSeconds, InterpSpeed);
		SetActorLocation(NewLocation);
	}
}

void AHexCardModel::Initialize(int OwnerCardInstanceID)
{
	CardInstanceID = OwnerCardInstanceID;
}

