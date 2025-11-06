// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoordinatePickerMode.h"
#include "EditorModeManager.h"
#include "EditorModes.h"
#include "Editor.h"
#include "LevelEditorViewport.h"
#include "SceneManagement.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

const FEditorModeID FCoordinatePickerMode::EM_CoordinatePicker(TEXT("CoordinatePickerMode"));

FCoordinatePickerMode::FCoordinatePickerMode()
	: bIsActive(false)
	, HoverLocation(FVector::ZeroVector)
	, bHasValidHover(false)
{
}

FCoordinatePickerMode::~FCoordinatePickerMode()
{
}

void FCoordinatePickerMode::Enter()
{
	FEdMode::Enter();
	bIsActive = true;
	bHasValidHover = false;
	UE_LOG(LogTemp, Log, TEXT("Coordinate Picker Mode: ACTIVATED - Click in viewport to select location, ESC to cancel"));
}

void FCoordinatePickerMode::Exit()
{
	bIsActive = false;
	bHasValidHover = false;
	FEdMode::Exit();
	UE_LOG(LogTemp, Log, TEXT("Coordinate Picker Mode: DEACTIVATED"));
}

void FCoordinatePickerMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	FEdMode::Tick(ViewportClient, DeltaTime);

	if (bIsActive && ViewportClient)
	{
		UpdateHoverLocation(ViewportClient);
	}
}

void FCoordinatePickerMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	FEdMode::Render(View, Viewport, PDI);

	if (!bIsActive || !bHasValidHover)
		return;

	// Draw crosshair at hover location
	const float CrosshairSize = 50.0f;
	const FLinearColor CrosshairColor(0.0f, 1.0f, 0.0f, 0.8f); // Green

	// Draw X axis line (red)
	PDI->DrawLine(
		HoverLocation + FVector(CrosshairSize, 0, 0),
		HoverLocation + FVector(-CrosshairSize, 0, 0),
		FLinearColor::Red,
		SDPG_Foreground,
		2.0f
	);

	// Draw Y axis line (green)
	PDI->DrawLine(
		HoverLocation + FVector(0, CrosshairSize, 0),
		HoverLocation + FVector(0, -CrosshairSize, 0),
		FLinearColor::Green,
		SDPG_Foreground,
		2.0f
	);

	// Draw Z axis line (blue)
	PDI->DrawLine(
		HoverLocation + FVector(0, 0, CrosshairSize),
		HoverLocation + FVector(0, 0, -CrosshairSize),
		FLinearColor::Blue,
		SDPG_Foreground,
		2.0f
	);

	// Draw sphere at center
	DrawWireSphere(PDI, HoverLocation, CrosshairColor, 20.0f, 16, SDPG_Foreground, 2.0f);

	// Draw coordinate text
	if (View && Viewport)
	{
		FVector2D PixelLocation;
		if (View->WorldToPixel(HoverLocation, PixelLocation))
		{
			FString CoordText = FString::Printf(TEXT("%.0f, %.0f, %.0f"),
				HoverLocation.X, HoverLocation.Y, HoverLocation.Z);

			// Draw text on canvas (done in DrawHUD)
		}
	}
}

bool FCoordinatePickerMode::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
	if (!bIsActive)
		return false;

	// Left click = select location
	if (Click.GetKey() == EKeys::LeftMouseButton && Click.GetEvent() == IE_Released)
	{
		if (bHasValidHover)
		{
			// Location picked!
			UE_LOG(LogTemp, Log, TEXT("Coordinate Picker: Selected location %.2f, %.2f, %.2f"),
				HoverLocation.X, HoverLocation.Y, HoverLocation.Z);

			if (OnCoordinatePicked.IsBound())
			{
				OnCoordinatePicked.Execute(HoverLocation);
			}

			// Exit mode
			if (GEditor && GLevelEditorModeTools().IsModeActive(EM_CoordinatePicker))
			{
				GLevelEditorModeTools().DeactivateMode(EM_CoordinatePicker);
			}

			return true;
		}
	}

	return false;
}

bool FCoordinatePickerMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	if (!bIsActive)
		return false;

	// ESC = cancel picking
	if (Key == EKeys::Escape && Event == IE_Pressed)
	{
		UE_LOG(LogTemp, Log, TEXT("Coordinate Picker: CANCELLED"));

		if (OnCoordinatePickCancelled.IsBound())
		{
			OnCoordinatePickCancelled.Execute();
		}

		// Exit mode
		if (GEditor && GLevelEditorModeTools().IsModeActive(EM_CoordinatePicker))
		{
			GLevelEditorModeTools().DeactivateMode(EM_CoordinatePicker);
		}

		return true;
	}

	return false;
}

bool FCoordinatePickerMode::GetCursor(EMouseCursor::Type& OutCursor) const
{
	if (bIsActive)
	{
		OutCursor = EMouseCursor::Crosshairs;
		return true;
	}

	return false;
}

void FCoordinatePickerMode::StartPicking(FOnCoordinatePicked Callback, FOnCoordinatePickCancelled CancelCallback)
{
	OnCoordinatePicked = Callback;
	OnCoordinatePickCancelled = CancelCallback;
	bIsActive = true;
	bHasValidHover = false;
}

void FCoordinatePickerMode::CancelPicking()
{
	if (OnCoordinatePickCancelled.IsBound())
	{
		OnCoordinatePickCancelled.Execute();
	}

	bIsActive = false;
}

bool FCoordinatePickerMode::UpdateHoverLocation(FEditorViewportClient* ViewportClient)
{
	if (!ViewportClient || !ViewportClient->Viewport)
		return false;

	// Get mouse position
	FViewport* Viewport = ViewportClient->Viewport;
	int32 MouseX = Viewport->GetMouseX();
	int32 MouseY = Viewport->GetMouseY();

	// Convert to world space ray
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		Viewport,
		ViewportClient->GetScene(),
		ViewportClient->EngineShowFlags)
		.SetRealtimeUpdate(true));

	FSceneView* View = ViewportClient->CalcSceneView(&ViewFamily);

	if (!View)
		return false;

	FViewportCursorLocation Cursor(View, ViewportClient, MouseX, MouseY);
	FVector RayStart = Cursor.GetOrigin();
	FVector RayEnd = RayStart + Cursor.GetDirection() * HALF_WORLD_MAX;

	// Perform line trace
	UWorld* World = ViewportClient->GetWorld();
	if (!World)
		return false;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;

	if (World->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_Visibility, QueryParams))
	{
		HoverLocation = HitResult.Location;
		bHasValidHover = true;
		return true;
	}
	else
	{
		// No hit - place at default distance from camera
		HoverLocation = RayStart + Cursor.GetDirection() * 1000.0f;
		bHasValidHover = true;
		return true;
	}
}

// ===== HELPER CLASS IMPLEMENTATION =====

void FCoordinatePickerHelper::StartPickingCoordinate(FOnCoordinatePicked OnPicked, FOnCoordinatePickCancelled OnCancelled)
{
	if (!GEditor)
		return;

	// Activate coordinate picker mode
	if (!GLevelEditorModeTools().IsModeActive(FCoordinatePickerMode::EM_CoordinatePicker))
	{
		GLevelEditorModeTools().ActivateMode(FCoordinatePickerMode::EM_CoordinatePicker);
	}

	// Get mode and start picking
	FCoordinatePickerMode* PickerMode = static_cast<FCoordinatePickerMode*>(
		GLevelEditorModeTools().GetActiveMode(FCoordinatePickerMode::EM_CoordinatePicker));

	if (PickerMode)
	{
		PickerMode->StartPicking(OnPicked, OnCancelled);
	}
}

void FCoordinatePickerHelper::CancelPickingCoordinate()
{
	if (!GEditor)
		return;

	if (GLevelEditorModeTools().IsModeActive(FCoordinatePickerMode::EM_CoordinatePicker))
	{
		FCoordinatePickerMode* PickerMode = static_cast<FCoordinatePickerMode*>(
			GLevelEditorModeTools().GetActiveMode(FCoordinatePickerMode::EM_CoordinatePicker));

		if (PickerMode)
		{
			PickerMode->CancelPicking();
		}

		GLevelEditorModeTools().DeactivateMode(FCoordinatePickerMode::EM_CoordinatePicker);
	}
}

bool FCoordinatePickerHelper::IsPickingActive()
{
	if (!GEditor)
		return false;

	if (GLevelEditorModeTools().IsModeActive(FCoordinatePickerMode::EM_CoordinatePicker))
	{
		FCoordinatePickerMode* PickerMode = static_cast<FCoordinatePickerMode*>(
			GLevelEditorModeTools().GetActiveMode(FCoordinatePickerMode::EM_CoordinatePicker));

		return PickerMode && PickerMode->IsPicking();
	}

	return false;
}
