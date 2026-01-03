#include "HexCardModel.h"
#include "HexCardState.h"
#include "CardType.h"
#include "Kismet/GameplayStatics.h"

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

void AHexCardModel::BreakCardState(FName& OutCardName, int& OutCardInstanceID, int& OutOwnerPlayerID,
	int& OutBasePowerA, int& OutBasePowerB, int& OutBasePowerC, int& OutBaseRange, FCardLocation& OutCardLocation,
	TArray<ECardBuff>& OutCardBuffs) const
{
	AHexCardState* HexCardState = Cast<AHexCardState>(UGameplayStatics::GetGameState(GetWorld()));
	FCardState OwnerCard = HexCardState -> GetCardInstancebyID(CardInstanceID, HexCardState -> CardStates);
	if (OwnerCard.IsValid())
	{
		OutCardName = OwnerCard.CardName;
		OutCardInstanceID = OwnerCard.CardInstanceID;
		OutOwnerPlayerID = OwnerCard.OwnerPlayerID;
		OutBasePowerA = OwnerCard.BasePowerA;
		OutBasePowerB = OwnerCard.BasePowerB;
		OutBasePowerC =OwnerCard.BasePowerC;
		OutBaseRange = OwnerCard.BaseRange;
		OutCardLocation = OwnerCard.CardLocation;
		OutCardBuffs = OwnerCard.CardBuffs;
	}
}

void AHexCardModel::RefreshAttr_Implementation()
{
	
}
