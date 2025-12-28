#include "HexCardController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

void AHexCardController::BeginPlay()
{
	Super::BeginPlay();

	//设置HexCardState
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState) return;
	HexCardState = Cast<AHexCardState>(GameState);
	
	//设置VisualManager
	VisualManager = NewObject<UVisualManager>(this);
	VisualManager->RegisterComponent();
	VisualManager->Initialize(HexCardState);
	HexCardState->OnCardStateChangeEvent.AddUObject(VisualManager, &UVisualManager::OnCardStateChangeEvent);

	//设置场景摄像机
	if (AActor* CameraActor  = UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass()))
	{
		SetViewTargetWithBlend(CameraActor, -1.0f, EViewTargetBlendFunction::VTBlend_Cubic);
	}
}
