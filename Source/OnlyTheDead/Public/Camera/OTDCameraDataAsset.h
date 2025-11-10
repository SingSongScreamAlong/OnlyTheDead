// Only The Dead - Camera Data Asset
// Designer-configurable camera parameters for each mode

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OTDCameraManager.h"
#include "OTDCameraDataAsset.generated.h"

/**
 * Data asset containing camera configuration for a specific camera mode
 * Allows designers to tweak camera behavior without code changes
 */
UCLASS(BlueprintType)
class ONLYTHEDEAD_API UOTDCameraDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Complete camera state configuration
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera State")
	FOTDCameraState CameraState;

	/**
	 * Optional post-process settings for this camera mode
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera State")
	FPostProcessSettings PostProcessSettings;

	/**
	 * Whether to apply post-process settings
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera State")
	bool bUsePostProcessSettings = false;

	/**
	 * Camera shake class to apply when entering this mode (optional)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera State")
	TSubclassOf<UCameraShakeBase> EnterModeShake;

	/**
	 * Description of this camera mode for designers
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera State", meta = (MultiLine = true))
	FText Description;
};
