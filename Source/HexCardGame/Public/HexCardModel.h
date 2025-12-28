#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HexCardModel.generated.h"

UENUM(BlueprintType)
enum class EMoveMode : uint8
{
	Fixed, //保持不动
	Interp, //插值到TargetLocation
	Teleport, //瞬移到TargetLocation
	Animation //由动画驱动
};

UCLASS()
class HEXCARDGAME_API AHexCardModel : public AActor
{
	GENERATED_BODY()
	
public:	

	AHexCardModel();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int CardInstanceID;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	EMoveMode MoveMode = EMoveMode::Fixed;

	UFUNCTION()
	void Initialize(int OwnerCardInstanceID);
	
};
