// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "UnrealWidgetFwd.h"
#include "EditorModeManager.h"
#include "EdMode.h"

DECLARE_DELEGATE_OneParam(FOnCoordinatePicked, const FVector&);
DECLARE_DELEGATE(FOnCoordinatePickCancelled);

/**
 * Editor mode for picking coordinates in the viewport
 * Allows users to click in the 3D viewport to select locations
 * TIER 2.1 Feature
 */
class CLAUDETERMINALEDITOR_API FCoordinatePickerMode : public FEdMode
{
public:
	const static FEditorModeID EM_CoordinatePicker;

	FCoordinatePickerMode();
	virtual ~FCoordinatePickerMode();

	// FEdMode interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual bool UsesToolkits() const override { return false; }
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click) override;
	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	virtual bool GetCursor(EMouseCursor::Type& OutCursor) const override;
	// End of FEdMode interface

	/**
	 * Activate coordinate picking
	 * @param Callback Called when coordinate is picked
	 * @param CancelCallback Called when picking is cancelled
	 */
	void StartPicking(FOnCoordinatePicked Callback, FOnCoordinatePickCancelled CancelCallback);

	/**
	 * Cancel coordinate picking
	 */
	void CancelPicking();

	/**
	 * Check if currently picking
	 */
	bool IsPicking() const { return bIsActive; }

	/**
	 * Get the current hover location
	 */
	FVector GetHoverLocation() const { return HoverLocation; }

private:
	// Update hover location based on mouse position
	bool UpdateHoverLocation(FEditorViewportClient* ViewportClient);

	// State
	bool bIsActive;
	FVector HoverLocation;
	bool bHasValidHover;

	// Delegates
	FOnCoordinatePicked OnCoordinatePicked;
	FOnCoordinatePickCancelled OnCoordinatePickCancelled;
};


/**
 * Helper class to manage coordinate picker mode lifecycle
 */
class CLAUDETERMINALEDITOR_API FCoordinatePickerHelper
{
public:
	/**
	 * Start coordinate picking in the editor viewport
	 * @param OnPicked Called when user clicks a location
	 * @param OnCancelled Called when user cancels (ESC)
	 */
	static void StartPickingCoordinate(FOnCoordinatePicked OnPicked, FOnCoordinatePickCancelled OnCancelled);

	/**
	 * Cancel active coordinate picking
	 */
	static void CancelPickingCoordinate();

	/**
	 * Check if coordinate picking is active
	 */
	static bool IsPickingActive();
};
