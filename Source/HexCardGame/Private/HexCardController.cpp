#include "HexCardController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerState.h"

AHexCardController::AHexCardController()
{
	PlayerCameraManager = nullptr;
}

void AHexCardController::BeginPlay()
{
	Super::BeginPlay();

	SetViewTarget(nullptr);
	
	//设置HexCardState
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState) return;
	HexCardState = Cast<AHexCardState>(GameState);
	
	//设置VisualManager
	VisualManager = NewObject<UVisualManager>(this);
	VisualManager->RegisterComponent();
	VisualManager->Initialize(HexCardState);
	HexCardState->OnCardStateChangeEvent.AddUObject(VisualManager, &UVisualManager::OnCardStateChangeEvent);
}

void AHexCardController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(HasAuthority()) return;

	if (HexCardState && VisualManager && CameraActor)
		
}

void AHexCardController::OnRep_PlayerState()
{
	if (HasAuthority()) return;	//纯本地事件
	
	//设置场景摄像机，旋转第二个玩家的本地摄像机
	if (AActor* CA  = UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass()))
	{
		CameraActor = Cast<ACameraActor>(CA);
	}
	if (PlayerState -> GetPlayerId() == 1 && IsLocalController() && CameraActor)
	{
		CameraActor -> AddActorWorldRotation(FRotator(0.0f, 180.0f , 0.0f));
	}
	SetViewTarget(CameraActor);
	Super::OnRep_PlayerState();	
}

void AHexCardController::SelectCard()
{
	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, HitResult))
	{
		if(AHexCardModel* Model = Cast<AHexCardModel>(HitResult.GetActor()))
		{
			CardModel = Model;
		}
	}
}

void AHexCardController::SelectHex()
{
	if (!CardModel) return; //无卡牌就返回

	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, HitResult))
	{
		if(AHexGrid* Model = Cast<AHexGrid>(HitResult.GetActor()))
		{
			HexModel = Model;
		}
	}
	if (CardModel && HexModel)
	{
		RequestPlayCard(CardModel -> CardInstanceID, HexModel -> HexQ, HexModel -> HexR);
		CardModel = nullptr;
		HexModel = nullptr;
	}
}

void AHexCardController::ClientReady_Implementation()
{
	HexCardState -> ReadyClient ++;
	HexCardState -> AdvancedGamePhase();
}

void AHexCardController::RequestPlayCard_Implementation( int CardInstanceID, int HexQ, int HexR)
{
	HexCardState -> RequestPlayCard(PlayerState->GetPlayerId(), CardInstanceID, HexQ, HexR);
}
