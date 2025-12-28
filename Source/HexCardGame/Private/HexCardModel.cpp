#include "HexCardModel.h"
#include "CardType.h"

// Sets default values
AHexCardModel::AHexCardModel()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHexCardModel::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	//位置自动更新
	if (MoveMode == EMoveMode::Interp)
	{
		const FVector Current = GetActorLocation();

		const FVector NewLocation =
			FMath::VInterpTo(Current, TargetLocation, DeltaSeconds, 10.f);
		SetActorLocation(NewLocation);
	}
}

void AHexCardModel::Initialize(int OwnerCardInstanceID)
{
	CardInstanceID = OwnerCardInstanceID;
}

