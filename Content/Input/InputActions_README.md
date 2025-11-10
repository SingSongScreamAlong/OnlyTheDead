# Enhanced Input Configuration for Only The Dead

This directory contains the Enhanced Input system configuration for the camera/movement/cover system.

## Input Actions Required

Create these Input Actions in Unreal Editor:

### Camera Actions
- **IA_Look** (Input Action)
  - Value Type: Axis2D (Vector2D)
  - Consumes Input: Yes

- **IA_SwapShoulder** (Input Action)
  - Value Type: Digital (bool)
  - Consumes Input: Yes

- **IA_Aim** (Input Action)
  - Value Type: Digital (bool)
  - Consumes Input: Yes

### Movement Actions
- **IA_Move** (Input Action)
  - Value Type: Axis2D (Vector2D)
  - Consumes Input: Yes

- **IA_Sprint** (Input Action)
  - Value Type: Digital (bool)
  - Consumes Input: Yes

### Posture Actions
- **IA_Crouch** (Input Action)
  - Value Type: Digital (bool)
  - Consumes Input: Yes

- **IA_Prone** (Input Action)
  - Value Type: Digital (bool)
  - Consumes Input: Yes

### Lean Actions
- **IA_LeanLeft** (Input Action)
  - Value Type: Digital (bool)
  - Consumes Input: Yes

- **IA_LeanRight** (Input Action)
  - Value Type: Digital (bool)
  - Consumes Input: Yes

- **IA_Lean** (Input Action)
  - Value Type: Axis1D (float)
  - Consumes Input: Yes
  - Range: -1.0 (left) to 1.0 (right)

## Input Mapping Context

Create **IMC_Character** (Input Mapping Context):

### Keyboard + Mouse Mappings
- **IA_Move**: W/A/S/D keys → 2D Composite
- **IA_Look**: Mouse XY → 2D Composite, multiply by sensitivity
- **IA_Sprint**: Left Shift (hold)
- **IA_Crouch**: C (toggle) or Ctrl (hold)
- **IA_Prone**: X or Z (toggle)
- **IA_Aim**: Right Mouse Button (hold)
- **IA_SwapShoulder**: Middle Mouse Button or V (press)
- **IA_LeanLeft**: Q (hold)
- **IA_LeanRight**: E (hold)

### Gamepad Mappings
- **IA_Move**: Left Thumbstick
- **IA_Look**: Right Thumbstick
- **IA_Sprint**: Left Thumbstick Click (L3)
- **IA_Crouch**: B/Circle (toggle)
- **IA_Prone**: D-Pad Down
- **IA_Aim**: Left Trigger (hold)
- **IA_SwapShoulder**: Right Thumbstick Click (R3)
- **IA_Lean**: Right Bumper (right) / Left Bumper (left)

## Implementation in Character BP

In your Character Blueprint or C++ class:

1. Add Input Mapping Context on BeginPlay:
```cpp
if (APlayerController* PC = Cast<APlayerController>(GetController()))
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(InputMappingContext, 0);
    }
}
```

2. Bind Input Actions in SetupPlayerInputComponent:
```cpp
if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
{
    // Movement
    EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOTDCharacter::Move);
    EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOTDCharacter::Look);
    EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &AOTDCharacter::StartSprint);
    EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &AOTDCharacter::StopSprint);

    // Posture
    EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &AOTDCharacter::ToggleCrouch);
    EnhancedInput->BindAction(ProneAction, ETriggerEvent::Started, this, &AOTDCharacter::ToggleProne);

    // Camera
    EnhancedInput->BindAction(AimAction, ETriggerEvent::Started, this, &AOTDCharacter::StartAiming);
    EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &AOTDCharacter::StopAiming);
    EnhancedInput->BindAction(SwapShoulderAction, ETriggerEvent::Started, this, &AOTDCharacter::SwapShoulder);

    // Lean
    EnhancedInput->BindAction(LeanAction, ETriggerEvent::Triggered, this, &AOTDCharacter::Lean);
    EnhancedInput->BindAction(LeanAction, ETriggerEvent::Completed, this, &AOTDCharacter::StopLean);
}
```

## Input Sensitivity Settings

Recommended starting values:
- **Mouse Look Sensitivity**: 0.3
- **Gamepad Look Sensitivity**: 1.0
- **Mouse Smoothing**: Enabled (slight)
- **Gamepad Dead Zone**: 0.15
- **Aim Assist (Console)**: 0.4 (subtle)

These can be exposed to players in options menu.

## Accessibility Considerations

- All hold-to-activate actions should have toggle alternatives
- Lean can be tap-to-peek or hold-to-lean (configurable)
- Sprint can be toggle or hold (configurable)
- Camera sensitivity should be widely adjustable (0.1x to 3.0x)
- Option to disable set-piece camera locks
- Fixed horizon option for motion sensitivity
