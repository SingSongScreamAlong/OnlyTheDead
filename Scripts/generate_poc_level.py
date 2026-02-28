"""
generate_poc_level.py
=====================
Run this script from the UE5 Editor Python console:
    File > Execute Python Script > Scripts/generate_poc_level.py

OR paste into:
    Tools > Output Log > Python console

What it builds:
    A 200m x 100m POC level called "POC_TrenchArtillery" containing:
    - A flat muddy terrain base (200m x 100m)
    - 12 TrenchSegment actors in an L-shape (historically authentic)
    - Pre-existing shell craters (the battlefield was cratered before February 21)
    - An ArtilleryManager pointed at the trench
    - WWI atmospheric lighting (overcast, filtered grey light)
    - Player Start positioned in the trench
    - Exponential height fog (acrid smoke, low visibility)

Requirements:
    - UnrealEngine 5.3+
    - PythonScriptPlugin enabled
    - EditorScriptingUtilities plugin enabled
    - Project compiled with OnlyTheDead module

Notes on scale:
    - UE units = 1 cm
    - Trench: 200 cm wide, 200 cm deep (2m x 2m, per design doc)
    - Level playable area: ~200m x 100m (20,000 x 10,000 cm)
    - Shell craters: 200-2500 cm radius depending on calibre
"""

import unreal
import math
import random

# ---- Configuration -------------------------------------------------------

LEVEL_PATH   = "/Game/POC_TrenchArtillery/Maps/POC_TrenchArtillery"
LEVEL_NAME   = "POC_TrenchArtillery"

# Trench layout origin (centre of the level, ground level)
TRENCH_ORIGIN = unreal.Vector(0.0, 0.0, 0.0)

# Trench segment length in cm (each actor covers this much trench)
SEGMENT_LENGTH_CM = 300.0   # 3 m per segment

# Number of segments in each arm of the L-shape
MAIN_ARM_COUNT  = 8   # main trench (runs along X axis)
SIDE_ARM_COUNT  = 4   # sap / traverse (runs along Y axis)

# Artillery manager position — "behind German lines" (positive X, off-screen)
ARTILLERY_ORIGIN = unreal.Vector(25000.0, 0.0, 500.0)

# Target zone: the trench area
ARTILLERY_TARGET_CENTER = unreal.Vector(0.0, 0.0, 0.0)
ARTILLERY_HALF_EXTENT   = unreal.Vector(2500.0, 600.0, 0.0)

# Pre-placed craters (historical: the ground at Verdun was already cratered)
NUM_EXISTING_CRATERS = 8

# ---- Blueprint class paths (must match your Content Browser paths) --------

BP_TRENCH_SEGMENT    = "/Game/POC_TrenchArtillery/Blueprints/BP_TrenchSegment"
BP_ARTILLERY_SHELL   = "/Game/POC_TrenchArtillery/Blueprints/BP_ArtilleryShell"
BP_ARTILLERY_MANAGER = "/Game/POC_TrenchArtillery/Blueprints/BP_ArtilleryManager"
BP_CRATER            = "/Game/POC_TrenchArtillery/Blueprints/BP_ShellCrater"

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def log(msg):
    unreal.log(f"[POC Level Gen] {msg}")

def log_warning(msg):
    unreal.log_warning(f"[POC Level Gen] WARNING: {msg}")

def get_actor_class(blueprint_path):
    """Load a Blueprint class from Content Browser path."""
    asset = unreal.load_asset(blueprint_path)
    if asset is None:
        log_warning(f"Could not load asset at: {blueprint_path}")
        return None
    return asset.generated_class() if hasattr(asset, 'generated_class') else None

def snap_to_ground(world, location, trace_dist=5000.0):
    """Line-trace downward and return the surface hit point."""
    start = unreal.Vector(location.x, location.y, location.z + trace_dist)
    end   = unreal.Vector(location.x, location.y, location.z - trace_dist)
    hit   = unreal.SystemLibrary.line_trace_single(
        world,
        start, end,
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,  # WorldStatic
        False, [], unreal.DrawDebugTrace.NONE, unreal.HitResult(), True
    )
    if hit:
        return unreal.HitResult.impact_point.get_value(unreal.HitResult())
    return location


# ---------------------------------------------------------------------------
# Level creation
# ---------------------------------------------------------------------------

def create_or_open_level():
    """Create the POC level if it doesn't exist, otherwise open it."""
    subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

    # Check if level already exists
    existing = unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH)
    if existing:
        log(f"Level already exists at {LEVEL_PATH}. Opening...")
        subsystem.load_level(LEVEL_PATH)
    else:
        log(f"Creating new level at {LEVEL_PATH}...")
        subsystem.new_level(LEVEL_PATH)

    return unreal.EditorLevelLibrary.get_editor_world()


# ---------------------------------------------------------------------------
# Lighting setup — WWI overcast February sky
# ---------------------------------------------------------------------------

def setup_lighting(world):
    log("Setting up atmospheric WWI lighting...")

    # Find or create directional light
    dir_lights = unreal.GameplayStatics.get_all_actors_of_class(
        world, unreal.DirectionalLight
    )

    if dir_lights:
        sun = dir_lights[0]
    else:
        sun = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.DirectionalLight,
            unreal.Vector(0, 0, 10000),
            unreal.Rotator(-20, 160, 0)   # Low winter sun angle
        )

    # February 1916 Verdun: overcast, cold grey light
    sun_comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
    if sun_comp:
        sun_comp.set_editor_property("intensity", 3.0)          # Dim, overcast
        sun_comp.set_editor_property("light_color",
            unreal.LinearColor(0.82, 0.88, 1.0, 1.0))           # Blue-grey winter
        sun_comp.set_editor_property("cast_shadows", True)
        sun_comp.set_editor_property("atmosphere_sun_light", True)
        sun.set_actor_rotation(unreal.Rotator(-20, 160, 0), False)

    # Exponential height fog — smoke, mud vapour, burned propellant
    fog_actors = unreal.GameplayStatics.get_all_actors_of_class(
        world, unreal.ExponentialHeightFog
    )

    if fog_actors:
        fog = fog_actors[0]
    else:
        fog = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.ExponentialHeightFog,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0)
        )

    fog_comp = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
    if fog_comp:
        fog_comp.set_editor_property("fog_density", 0.04)        # Moderate fog
        fog_comp.set_editor_property("fog_height_falloff", 0.2)
        fog_comp.set_editor_property("fog_inscattering_color",
            unreal.LinearColor(0.6, 0.55, 0.5, 1.0))            # Dirty yellow-grey
        fog_comp.set_editor_property("fog_max_opacity", 0.85)
        fog_comp.set_editor_property("start_distance", 1000.0)   # Fog starts 10m out

    # Sky atmosphere — gives correct horizon colour
    sky_atm = unreal.GameplayStatics.get_all_actors_of_class(
        world, unreal.SkyAtmosphere
    )
    if not sky_atm:
        unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.SkyAtmosphere,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0)
        )

    log("Lighting configured.")


# ---------------------------------------------------------------------------
# Terrain — flat muddy ground (landscape asset created manually in editor,
# this creates a simple BSP plane as placeholder for POC)
# ---------------------------------------------------------------------------

def create_ground_plane(world):
    log("Creating ground plane (BSP placeholder — replace with Landscape for production)...")

    # Spawn a static mesh plane actor as ground
    # In production: create a Landscape with the terrain from fetch_elevation_data.py
    ground = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0, 0, -5),
        unreal.Rotator(0, 0, 0)
    )
    if ground:
        ground.set_actor_label("Ground_Plane")
        ground.set_actor_scale3d(unreal.Vector(200, 100, 1))  # 200m x 100m

        sm_comp = ground.get_component_by_class(unreal.StaticMeshComponent)
        if sm_comp:
            # Use the engine's built-in 1m plane mesh
            plane_mesh = unreal.load_asset("/Engine/BasicShapes/Plane")
            if plane_mesh:
                sm_comp.set_static_mesh(plane_mesh)
                # Apply a muddy/dark material (assign M_Mud in editor)
                sm_comp.set_editor_property("cast_shadow", True)

    log("Ground plane created.")
    return ground


# ---------------------------------------------------------------------------
# Trench — L-shaped layout
# ---------------------------------------------------------------------------

def place_trench_segments(world):
    """
    Place TrenchSegment actors in an L-shape:

    Y axis (width of map)
    ^
    |   [side arm — 4 segments running along Y]
    |   |
    |   +----[main arm — 8 segments running along X]---->  X axis

    This is historically authentic: WWI trenches were NOT straight lines.
    They were built in bays and traverses to limit blast propagation.
    """
    log(f"Placing trench segments (L-shape: {MAIN_ARM_COUNT} + {SIDE_ARM_COUNT} segments)...")

    trench_class = get_actor_class(BP_TRENCH_SEGMENT)
    if trench_class is None:
        log_warning("BP_TrenchSegment not found. Spawning placeholder actors.")

    segments = []
    segment_index = 0

    # ---- Main arm (X axis) ----
    for i in range(MAIN_ARM_COUNT):
        x = TRENCH_ORIGIN.x + i * SEGMENT_LENGTH_CM
        y = TRENCH_ORIGIN.y
        z = TRENCH_ORIGIN.z

        loc = unreal.Vector(x, y, z)
        rot = unreal.Rotator(0, 0, 0)   # Aligned with X axis

        actor = _spawn_trench_segment(world, trench_class, loc, rot, segment_index)
        if actor:
            segments.append(actor)
        segment_index += 1

    # ---- Side arm (Y axis) — starts at the beginning of the main arm ----
    # This creates the corner of the L at TRENCH_ORIGIN
    for i in range(1, SIDE_ARM_COUNT + 1):   # Start at 1 to avoid origin duplicate
        x = TRENCH_ORIGIN.x
        y = TRENCH_ORIGIN.y + i * SEGMENT_LENGTH_CM
        z = TRENCH_ORIGIN.z

        loc = unreal.Vector(x, y, z)
        rot = unreal.Rotator(0, 90, 0)  # Rotated 90° for Y-axis alignment

        actor = _spawn_trench_segment(world, trench_class, loc, rot, segment_index)
        if actor:
            segments.append(actor)
        segment_index += 1

    # ---- Link adjacency for chain-collapse propagation ----
    _link_adjacent_segments(segments)

    log(f"Placed {len(segments)} trench segments.")
    return segments


def _spawn_trench_segment(world, trench_class, location, rotation, index):
    """Spawn a single TrenchSegment actor."""
    if trench_class:
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            trench_class, location, rotation
        )
    else:
        # Fallback: plain actor as placeholder
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.Actor, location, rotation
        )

    if actor:
        actor.set_actor_label(f"TrenchSegment_{index:02d}")
        try:
            actor.set_editor_property("segment_index", index)
        except Exception:
            pass  # Property not accessible via Python; set in BP defaults instead

    return actor


def _link_adjacent_segments(segments):
    """
    Set adjacent segment references so heavy shell hits propagate to neighbours.
    Simple linear adjacency — each segment knows its immediate neighbours.
    """
    for i, seg in enumerate(segments):
        adjacent = []
        if i > 0:
            adjacent.append(segments[i - 1])
        if i < len(segments) - 1:
            adjacent.append(segments[i + 1])
        try:
            seg.set_editor_property("adjacent_segments", adjacent)
        except Exception:
            pass  # Will be wired in Blueprint if Python property access fails


# ---------------------------------------------------------------------------
# Pre-existing shell craters
# ---------------------------------------------------------------------------

def place_existing_craters(world):
    """
    Scatter pre-existing craters across the landscape.
    Historically: the ground around Verdun was already heavily cratered
    from months of fighting before the main February 21 offensive.
    """
    log(f"Placing {NUM_EXISTING_CRATERS} pre-existing shell craters...")

    crater_class = get_actor_class(BP_CRATER)

    random.seed(1916)  # Reproducible layout
    craters_placed = 0

    for _ in range(NUM_EXISTING_CRATERS):
        # Random position across the level, but not inside the trench itself
        x = random.uniform(-5000.0, 18000.0)
        y = random.uniform(-4000.0, 4000.0)
        z = 0.0

        # Don't place craters directly on top of the trench segments
        too_close = (abs(y) < 400.0 and 0.0 < x < MAIN_ARM_COUNT * SEGMENT_LENGTH_CM)
        if too_close:
            y = 800.0 * (1 if y >= 0 else -1)

        loc = unreal.Vector(x, y, z)
        rot = unreal.Rotator(0, random.uniform(0, 360), 0)

        # Vary crater size — most are 75mm, some are larger
        roll = random.random()
        if roll < 0.60:
            scale = random.uniform(0.8, 1.2)    # 75mm ~2m radius
        elif roll < 0.85:
            scale = random.uniform(2.0, 3.5)    # 155mm ~6m radius
        else:
            scale = random.uniform(4.0, 7.0)    # 210mm+ ~12m radius

        if crater_class:
            actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
                crater_class, loc, rot
            )
        else:
            actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.StaticMeshActor, loc, rot
            )

        if actor:
            actor.set_actor_label(f"PreExistingCrater_{craters_placed:02d}")
            actor.set_actor_scale3d(unreal.Vector(scale, scale, scale * 0.4))
            craters_placed += 1

    log(f"Placed {craters_placed} pre-existing craters.")


# ---------------------------------------------------------------------------
# Artillery Manager
# ---------------------------------------------------------------------------

def place_artillery_manager(world):
    log("Placing ArtilleryManager...")

    manager_class = get_actor_class(BP_ARTILLERY_MANAGER)

    if manager_class:
        manager = unreal.EditorLevelLibrary.spawn_actor_from_class(
            manager_class,
            ARTILLERY_ORIGIN,
            unreal.Rotator(0, 180, 0)   # Facing toward the trench
        )
    else:
        log_warning("BP_ArtilleryManager not found. Spawning placeholder Actor.")
        manager = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.Actor,
            ARTILLERY_ORIGIN,
            unreal.Rotator(0, 180, 0)
        )

    if manager:
        manager.set_actor_label("ArtilleryManager")
        try:
            manager.set_editor_property("target_zone_center",    ARTILLERY_TARGET_CENTER)
            manager.set_editor_property("target_zone_half_extent", ARTILLERY_HALF_EXTENT)
            manager.set_editor_property("launch_origin",         ARTILLERY_ORIGIN)
            manager.set_editor_property("min_interval_seconds",  2.0)
            manager.set_editor_property("max_interval_seconds",  9.0)
            manager.set_editor_property("barrage_start_delay_seconds", 5.0)
            manager.set_editor_property("barrage_pattern",       0)  # 0 = Random
        except Exception as e:
            log_warning(f"Could not set ArtilleryManager properties via Python: {e}")
            log_warning("Set them manually in the Details panel.")

    log("ArtilleryManager placed.")
    return manager


# ---------------------------------------------------------------------------
# Player Start
# ---------------------------------------------------------------------------

def place_player_start(world):
    log("Placing PlayerStart inside the trench...")

    # Place player at the midpoint of the main arm, at ground level
    player_x = TRENCH_ORIGIN.x + (MAIN_ARM_COUNT // 2) * SEGMENT_LENGTH_CM
    player_y = TRENCH_ORIGIN.y
    player_z = TRENCH_ORIGIN.z + 100.0   # 1m above trench floor

    player_start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(player_x, player_y, player_z),
        unreal.Rotator(0, 180, 0)   # Facing along the trench toward origin
    )

    if player_start:
        player_start.set_actor_label("PlayerStart_TrenchMidpoint")
        log(f"PlayerStart placed at ({player_x:.0f}, {player_y:.0f}, {player_z:.0f})")

    return player_start


# ---------------------------------------------------------------------------
# Debug markers (visible in editor, stripped at runtime)
# ---------------------------------------------------------------------------

def draw_debug_annotations(world):
    """Draw text labels and boxes visible in editor viewport."""
    log("Adding editor annotations...")

    # Target zone box (matches ArtilleryManager.TargetZoneHalfExtent)
    # These are Arrow actors used purely as visual markers
    try:
        arrow = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.Arrow, ARTILLERY_ORIGIN, unreal.Rotator(0, 180, 0)
        )
        if arrow:
            arrow.set_actor_label("DEBUG_ArtilleryDirection")
            arrow.set_actor_scale3d(unreal.Vector(50, 50, 50))
    except Exception:
        pass


# ---------------------------------------------------------------------------
# World settings
# ---------------------------------------------------------------------------

def configure_world_settings(world):
    log("Configuring world settings...")

    settings = world.get_world_settings()
    if settings:
        try:
            settings.set_editor_property("default_game_mode",
                unreal.load_class(None, "/Script/OnlyTheDead.VerdunGameMode"))
        except Exception:
            log_warning("Could not set default game mode — set it manually in World Settings.")

        # Global gravity — standard Earth gravity
        settings.set_editor_property("global_gravity_z", -980.0)

    log("World settings configured.")


# ---------------------------------------------------------------------------
# Main entry point
# ---------------------------------------------------------------------------

def generate_poc_level():
    unreal.log("=" * 60)
    unreal.log("  Only The Dead — POC Level Generator")
    unreal.log("  Battle of Verdun, February 21, 1916")
    unreal.log("=" * 60)

    world = create_or_open_level()
    if world is None:
        unreal.log_error("[POC Level Gen] FAILED: Could not get editor world.")
        return

    with unreal.ScopedEditorTransaction("Generate POC Trench Artillery Level") as transaction:
        setup_lighting(world)
        create_ground_plane(world)
        segments = place_trench_segments(world)
        place_existing_craters(world)
        place_artillery_manager(world)
        place_player_start(world)
        draw_debug_annotations(world)
        configure_world_settings(world)

    # Save the level
    unreal.EditorLevelLibrary.save_current_level()

    unreal.log("=" * 60)
    unreal.log(f"  POC Level generated: {len(segments)} trench segments")
    unreal.log(f"  Level saved to: {LEVEL_PATH}")
    unreal.log("")
    unreal.log("  NEXT STEPS:")
    unreal.log("  1. Create Blueprints in Content/POC_TrenchArtillery/Blueprints/:")
    unreal.log("     - BP_TrenchSegment  (parent: ATrenchSegment)")
    unreal.log("     - BP_ArtilleryShell (parent: AArtilleryShell)")
    unreal.log("     - BP_ArtilleryManager (parent: AArtilleryManager)")
    unreal.log("     - BP_ShellCrater    (simple static mesh actor)")
    unreal.log("     - BP_VerdunSoldier  (parent: AVerdunSoldier)")
    unreal.log("  2. Assign meshes, sounds, and VFX in each Blueprint.")
    unreal.log("  3. Set ShellClass in ArtilleryManager to BP_ArtilleryShell.")
    unreal.log("  4. Press Play In Editor and wait 5 seconds.")
    unreal.log("=" * 60)


# Run immediately when executed via editor
generate_poc_level()
