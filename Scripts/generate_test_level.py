"""
generate_test_level.py
======================
Generates a dedicated test level for verifying every gameplay system.

Run from UE5 Editor Python console:
    File > Execute Python Script > Scripts/generate_test_level.py

Level layout (top-down, all measurements in metres):

  Y=80m  [CREEPING BARRAGE LANE — shells advance L→R during test]
         ──────────────────────────────────────────────────────────
  Y=60m  [VERDUN SEQUENCE TARGET ZONE]     [PLAYER BUNKER / TRENCH]
         ──────────────────────────────────────────────────────────
  Y=40m  [ZONE 3: 75mm×8]  [ZONE 4: 155mm×3]  [ZONE 5: 305mm×1]
         [Accumulation tests — overlapping craters, ground churning]
         ──────────────────────────────────────────────────────────
  Y=20m  [ZONE 0: 75mm×1]  [ZONE 1: 155mm×1]  [ZONE 2: 210mm×1]
         [Calibre comparison — single impacts, side by side]
         ──────────────────────────────────────────────────────────
  Y=0m   [REFERENCE AREA — UNDISTURBED] ←── stays flat, baseline

         X=0m              X=35m             X=70m             X=100m

All UE units = 1cm. Measurements above in metres for readability.

What this tests:
  - Lumen flash: visible from player bunker (70m from zones 0-2)
  - Terrain deformation: before/after visible between reference + zones
  - Calibre scaling: 75mm vs 155mm vs 210mm vs 305mm visually side-by-side
  - Accumulation: zones 3-5 show compounding deformation
  - Shell shock: player in bunker builds shock during long tests
  - Creeping barrage: visible advance from Y=80m left to right
  - Drumfire: smoke accumulation obscures the field during Verdun sequence
"""

import unreal
import math
import random

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

LEVEL_PATH  = "/Game/POC_TrenchArtillery/Maps/TestLevel"
LEVEL_NAME  = "TestLevel"

# Scale — all in cm
M = 100.0  # 1 metre

# Zone centres (see layout diagram above)
ZONE_CENTRES = [
    unreal.Vector( 1*M*10,  2*M*10, 0),   # Zone 0: 75mm  single      (10m, 20m)
    unreal.Vector( 3*M*10,  2*M*10, 0),   # Zone 1: 155mm single      (30m, 20m)  ← 20m gap
    unreal.Vector( 6*M*10,  2*M*10, 0),   # Zone 2: 210mm single      (60m, 20m)  ← 30m gap (needs room)
    unreal.Vector( 1*M*10,  5*M*10, 0),   # Zone 3: 75mm × 8 accum    (10m, 50m)
    unreal.Vector( 4*M*10,  5*M*10, 0),   # Zone 4: 155mm × 3 accum   (40m, 50m)
    unreal.Vector( 7*M*10,  5*M*10, 0),   # Zone 5: 305mm single      (70m, 50m)
]

VERDUN_TARGET     = unreal.Vector(4*M*10, 7*M*10, 0)    # (40m, 70m) — Verdun sequence target
CREEP_START       = unreal.Vector(0,      8*M*10, 0)    # (0m, 80m)
CREEP_END         = unreal.Vector(10*M*10,8*M*10, 0)    # (100m, 80m)
PLAYER_BUNKER_LOC = unreal.Vector(9*M*10, 6*M*10, 0)   # (90m, 60m) — viewing position

# Gun origin — off-map, shells appear from here
GUN_ORIGIN        = unreal.Vector(30*M*10, 5*M*10, 80*M)  # 30m off east edge, high up

# Terrain: covers the whole level + margin
TERRAIN_ORIGIN    = unreal.Vector(-10*M, -10*M, -2*M)  # slight below-ground so trench walls sit on it
TERRAIN_WIDTH     = 12*M*10   # 120m
TERRAIN_DEPTH     = 10*M*10   # 100m

# Reference area (stays clear of all shells — left column, Y=0-15m)
REFERENCE_AREA_CENTER = unreal.Vector(1*M*10, 1*M*10, 0)

# Blueprint paths
BP_DEFORMABLE_TERRAIN = "/Game/POC_TrenchArtillery/Blueprints/BP_DeformableTerrain"
BP_BARRAGE_DIRECTOR   = "/Game/POC_TrenchArtillery/Blueprints/BP_BarrageDirector"
BP_TEST_DIRECTOR      = "/Game/POC_TrenchArtillery/Blueprints/BP_TestDirector"
BP_TRENCH_SEGMENT     = "/Game/POC_TrenchArtillery/Blueprints/BP_TrenchSegment"
BP_SHELL_75MM         = "/Game/POC_TrenchArtillery/Blueprints/BP_ArtilleryShell_75mm"
BP_SHELL_155MM        = "/Game/POC_TrenchArtillery/Blueprints/BP_ArtilleryShell_155mm"
BP_SHELL_210MM        = "/Game/POC_TrenchArtillery/Blueprints/BP_ArtilleryShell_210mm"
BP_SHELL_305MM        = "/Game/POC_TrenchArtillery/Blueprints/BP_ArtilleryShell_305mm"

# Zone labels for text renders
ZONE_LABELS = [
    "ZONE 0\n75mm × 1\nBaseline crater",
    "ZONE 1\n155mm × 1\n3× larger",
    "ZONE 2\n210mm × 1\n6× larger",
    "ZONE 3\n75mm × 8\nAccumulation",
    "ZONE 4\n155mm × 3\nOverlapping",
    "ZONE 5\n305mm × 1\nMaximum impact",
]

ZONE_DESCRIPTIONS = [
    "75mm Soixante-Quinze — 5m lethal, 2m crater, 60cm deep",
    "155mm Howitzer — 15m lethal, 6m crater, 115cm deep",
    "210mm Morser — 30m lethal, 12m crater, 200cm deep",
    "8 × 75mm rapid — watch craters accumulate and merge",
    "3 × 155mm spaced — overlapping medium craters",
    "305mm Siege Mortar — 50m lethal, 25m crater, 350cm deep",
]

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def log(msg):        unreal.log(f"[TestLevel] {msg}")
def log_warn(msg):   unreal.log_warning(f"[TestLevel] {msg}")

def get_class(path):
    try:
        cls = unreal.load_class(None, path + "_C")
        return cls
    except Exception:
        return None

def spawn_actor(world, cls_path, location, rotation=None, fallback_class=None):
    """Spawn an actor by Blueprint path, with fallback."""
    rot = rotation or unreal.Rotator(0, 0, 0)
    cls = get_class(cls_path)
    if cls:
        return unreal.EditorLevelLibrary.spawn_actor_from_class(cls, location, rot)
    elif fallback_class:
        return unreal.EditorLevelLibrary.spawn_actor_from_class(fallback_class, location, rot)
    else:
        log_warn(f"Cannot spawn {cls_path} — no fallback, skipping")
        return None

def spawn_text_render(world, location, text, color=None, size=20.0):
    """Spawn a TextRenderActor at location. Visible in editor and PIE."""
    try:
        tr_class = unreal.load_class(None, "/Script/Engine.TextRenderActor")
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            tr_class, location, unreal.Rotator(0, 0, 0))
        if actor:
            text_comp = actor.get_component_by_class(unreal.TextRenderComponent)
            if text_comp:
                text_comp.set_editor_property("text", text)
                text_comp.set_editor_property("world_size", size)
                if color:
                    text_comp.set_editor_property("text_render_color", color)
        return actor
    except Exception as e:
        log_warn(f"Could not spawn text render: {e}")
        return None

# ---------------------------------------------------------------------------
# Level creation
# ---------------------------------------------------------------------------

def create_or_get_level():
    world = unreal.UnrealEditorSubsystem.get_editor_world()
    if world:
        return world
    unreal.EditorLevelLibrary.new_level(LEVEL_PATH)
    return unreal.UnrealEditorSubsystem.get_editor_world()

# ---------------------------------------------------------------------------
# 1. Lighting and post-process
# ---------------------------------------------------------------------------

def setup_environment(world):
    """
    WWI overcast battlefield environment.
    Lumen GI + volumetric fog for smoke accumulation.
    Neutral grey exposure so shell flashes register dramatically.
    """
    log("Setting up environment (Lumen + overcast lighting)...")

    # Directional light — filtered grey overcast, no harsh shadows
    try:
        dl_class = unreal.load_class(None, "/Script/Engine.DirectionalLight")
        dl = unreal.EditorLevelLibrary.spawn_actor_from_class(
            dl_class, unreal.Vector(0, 0, 5000), unreal.Rotator(-45, 45, 0))
        if dl:
            dl_comp = dl.get_component_by_class(unreal.DirectionalLightComponent)
            if dl_comp:
                dl_comp.set_editor_property("intensity", 3.0)
                # Overcast colour: grey-blue, desaturated, WWI winter sky
                dl_comp.set_editor_property("light_color",
                    unreal.LinearColor(0.85, 0.88, 1.0, 1.0))
                dl_comp.set_editor_property("cast_shadows", True)
                # Atmosphere/Lumen
                dl_comp.set_editor_property("atmosphere_sun_light", True)
            dl.set_actor_label("DirectionalLight_OvercastSky")
            log("  Directional light: overcast grey-blue")
    except Exception as e:
        log_warn(f"Could not create directional light: {e}")

    # Sky light — Lumen requires this to pick up environmental bounce
    try:
        sl_class = unreal.load_class(None, "/Script/Engine.SkyLight")
        sl = unreal.EditorLevelLibrary.spawn_actor_from_class(
            sl_class, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
        if sl:
            sl_comp = sl.get_component_by_class(unreal.SkyLightComponent)
            if sl_comp:
                sl_comp.set_editor_property("intensity", 0.5)
                sl_comp.set_editor_property("light_color",
                    unreal.LinearColor(0.7, 0.75, 0.9, 1.0))
                # Real-time capture so Lumen updates as smoke fills the sky
                sl_comp.set_editor_property("source_type",
                    unreal.SkyLightSourceType.SLS_CAPTURED_SCENE)
                sl_comp.set_editor_property("real_time_capture", True)
            sl.set_actor_label("SkyLight_Lumen")
            log("  Sky light: Lumen real-time capture enabled")
    except Exception as e:
        log_warn(f"Could not create sky light: {e}")

    # Exponential height fog — acrid smoke hanging over battlefield
    # Accumulates with shell impacts (Niagara smoke columns feed into it)
    try:
        fog_class = unreal.load_class(None, "/Script/Engine.ExponentialHeightFog")
        fog = unreal.EditorLevelLibrary.spawn_actor_from_class(
            fog_class, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
        if fog:
            fog_comp = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
            if fog_comp:
                fog_comp.set_editor_property("fog_density", 0.02)
                fog_comp.set_editor_property("fog_height_falloff", 0.2)
                fog_comp.set_editor_property("fog_inscattering_color",
                    unreal.LinearColor(0.7, 0.65, 0.55, 1.0))  # Ochre-grey, WWI smoke
                fog_comp.set_editor_property("start_distance", 0.0)
                fog_comp.set_editor_property("fog_cutoff_distance", 50000.0)
                # Volumetric fog — required for Niagara smoke to affect the atmosphere
                fog_comp.set_editor_property("volumetric_fog", True)
                fog_comp.set_editor_property("volumetric_fog_scattering_distribution", 0.2)
                fog_comp.set_editor_property("volumetric_fog_albedo",
                    unreal.LinearColor(0.8, 0.75, 0.65, 1.0))
                fog_comp.set_editor_property("volumetric_fog_extinction_scale", 1.0)
            fog.set_actor_label("ExponentialHeightFog_BattlefieldSmoke")
            log("  Volumetric fog: enabled (Niagara smoke will interact with it)")
    except Exception as e:
        log_warn(f"Could not create height fog: {e}")

    # Post-process volume — full scene, Lumen settings
    try:
        ppv_class = unreal.load_class(None, "/Script/Engine.PostProcessVolume")
        ppv = unreal.EditorLevelLibrary.spawn_actor_from_class(
            ppv_class, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
        if ppv:
            ppv.set_editor_property("infinite_extent", True)
            ppv.set_actor_label("PostProcessVolume_Main")
            # Note: Lumen GI and reflection settings are configured in Project Settings.
            # Enable: Rendering > Global Illumination > Lumen
            #         Rendering > Reflections > Lumen
            # The PPV allows per-scene overrides if needed.
            log("  PostProcessVolume: placed (configure Lumen in Project Settings)")
    except Exception as e:
        log_warn(f"Could not create post-process volume: {e}")

    log("Environment setup complete.")

# ---------------------------------------------------------------------------
# 2. Deformable terrain
# ---------------------------------------------------------------------------

def create_terrain(world):
    """
    ADeformableTerrain covering the entire test area.
    100cm grid resolution: good fidelity for 75mm craters (4 verts across).
    """
    log(f"Creating deformable terrain ({TERRAIN_WIDTH/M:.0f}m × {TERRAIN_DEPTH/M:.0f}m)...")

    terrain = spawn_actor(world, BP_DEFORMABLE_TERRAIN, TERRAIN_ORIGIN,
                          fallback_class=unreal.StaticMeshActor)
    if terrain:
        terrain.set_actor_label("DeformableTerrain")
        try:
            terrain.set_editor_property("terrain_width",      TERRAIN_WIDTH)
            terrain.set_editor_property("terrain_depth",      TERRAIN_DEPTH)
            terrain.set_editor_property("grid_resolution_cm", 100.0)
        except Exception as e:
            log_warn(f"Could not set terrain properties: {e}")
        log(f"  Terrain: {TERRAIN_WIDTH/M:.0f}m × {TERRAIN_DEPTH/M:.0f}m, 100cm grid")
    else:
        log_warn("Terrain not placed. Create BP_DeformableTerrain (parent: ADeformableTerrain).")
    return terrain

# ---------------------------------------------------------------------------
# 3. Zone markers — visual guides for each test zone
# ---------------------------------------------------------------------------

def place_zone_markers(world):
    """
    Each test zone gets:
      - A text render label (zone name + description)
      - A set of scale-reference stakes (at 1m, 5m, 10m from centre)
      - An arrow pointing at the impact point
    These make crater size immediately readable at a glance.
    """
    log("Placing zone markers and scale references...")

    # Shell lethal/wound/crater radii for reference stakes
    zone_radii = [
        (200,  500,  1500),    # Zone 0: 75mm  crater/lethal/wound
        (600,  1500, 4000),    # Zone 1: 155mm
        (1200, 3000, 8000),    # Zone 2: 210mm
        (200,  500,  1500),    # Zone 3: 75mm accum
        (600,  1500, 4000),    # Zone 4: 155mm accum
        (2500, 5000, 12000),   # Zone 5: 305mm
    ]

    zone_colors = [
        unreal.Color(100, 200, 100, 255),   # Zone 0: green (small)
        unreal.Color(200, 200, 50, 255),    # Zone 1: yellow
        unreal.Color(220, 120, 50, 255),    # Zone 2: orange
        unreal.Color(100, 200, 100, 255),   # Zone 3: green (accum)
        unreal.Color(200, 200, 50, 255),    # Zone 4: yellow accum
        unreal.Color(200, 50, 50, 255),     # Zone 5: red (maximum)
    ]

    for i, (centre, label, desc) in enumerate(zip(ZONE_CENTRES, ZONE_LABELS, ZONE_DESCRIPTIONS)):
        color = zone_colors[i]

        # Label above the impact point
        label_loc = unreal.Vector(centre.x, centre.y, 300.0)  # 3m above ground
        text_actor = spawn_text_render(world, label_loc, label, color, size=25.0)
        if text_actor:
            text_actor.set_actor_label(f"ZoneLabel_{i:02d}")

        # Description text — smaller, below label
        desc_loc = unreal.Vector(centre.x, centre.y, 150.0)
        desc_actor = spawn_text_render(world, desc_loc, desc, color, size=12.0)
        if desc_actor:
            desc_actor.set_actor_label(f"ZoneDesc_{i:02d}")

        # Scale reference stakes: thin columns at crater radius + lethal radius
        # (so you can judge crater size against lethal zone after impact)
        crater_r, lethal_r, _ = zone_radii[i]
        _place_stake(world, centre, crater_r, f"Zone{i}_CraterEdge",
                     unreal.Vector(0.1, 0.1, 2.0))   # thin, tall
        _place_stake(world, centre, lethal_r, f"Zone{i}_LethalEdge",
                     unreal.Vector(0.2, 0.2, 1.5))

    # Reference area label
    ref_label = spawn_text_render(world,
        unreal.Vector(REFERENCE_AREA_CENTER.x, REFERENCE_AREA_CENTER.y, 250.0),
        "REFERENCE\nUndisturbed terrain\nStays flat",
        unreal.Color(150, 220, 255, 255), size=20.0)
    if ref_label:
        ref_label.set_actor_label("ReferenceAreaLabel")

    log(f"Zone markers placed ({len(ZONE_CENTRES)} zones).")

def _place_stake(world, zone_centre, radius_cm, label, scale):
    """Place a thin vertical pole at the given radius from zone_centre (N/S/E/W)."""
    offsets = [
        unreal.Vector(radius_cm, 0, 0),
        unreal.Vector(-radius_cm, 0, 0),
        unreal.Vector(0, radius_cm, 0),
        unreal.Vector(0, -radius_cm, 0),
    ]
    for j, offset in enumerate(offsets):
        loc = unreal.Vector(zone_centre.x + offset.x, zone_centre.y + offset.y, 0)
        stake = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor, loc, unreal.Rotator(0, 0, 0))
        if stake:
            stake.set_actor_label(f"{label}_{j}")
            stake.set_actor_scale3d(scale)
            sm_comp = stake.get_component_by_class(unreal.StaticMeshComponent)
            if sm_comp:
                cylinder = unreal.load_asset("/Engine/BasicShapes/Cylinder")
                if cylinder:
                    sm_comp.set_static_mesh(cylinder)

# ---------------------------------------------------------------------------
# 4. Measurement grid — 10m interval markers across reference area
# ---------------------------------------------------------------------------

def place_measurement_grid(world):
    """
    A 10m grid of low markers across the reference area (left column, Y=0-20m).
    Gives a sense of scale when comparing to crater sizes.
    """
    log("Placing 10m measurement grid in reference area...")

    grid_origin_x = 0
    grid_origin_y = 0
    grid_size_x   = 3   # 3 columns (0, 10m, 20m along X — within reference zone)
    grid_size_y   = 2   # 2 rows

    for gx in range(grid_size_x + 1):
        for gy in range(grid_size_y + 1):
            loc = unreal.Vector(
                grid_origin_x + gx * 10 * M,
                grid_origin_y + gy * 10 * M,
                0
            )
            marker = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.StaticMeshActor, loc, unreal.Rotator(0, 0, 0))
            if marker:
                marker.set_actor_label(f"GridMarker_{gx}_{gy}")
                marker.set_actor_scale3d(unreal.Vector(0.3, 0.3, 0.1))  # Flat disc
                sm_comp = marker.get_component_by_class(unreal.StaticMeshComponent)
                if sm_comp:
                    cylinder = unreal.load_asset("/Engine/BasicShapes/Cylinder")
                    if cylinder:
                        sm_comp.set_static_mesh(cylinder)

    log("Measurement grid placed.")

# ---------------------------------------------------------------------------
# 5. Player bunker — L-shaped trench giving cover + viewing position
# ---------------------------------------------------------------------------

def create_player_bunker(world):
    """
    Small L-shaped trench at (90m, 60m).
    Gives the player:
      - Cover from direct blast (important for shell shock accumulation tests)
      - Sightline across all 6 test zones
      - A position 70m from zone 0 — within Lumen flash visibility range

    The bunker is 4 TrenchSegment actors forming an L.
    """
    log("Creating player bunker (L-shaped trench at viewing position)...")

    trench_cls = get_class(BP_TRENCH_SEGMENT)
    segment_len = 3 * M   # 3m per segment

    bunker_segments = []

    # Main arm: 3 segments running along -Y (so player looks toward zones)
    for i in range(3):
        loc = unreal.Vector(
            PLAYER_BUNKER_LOC.x,
            PLAYER_BUNKER_LOC.y - i * segment_len,
            0
        )
        rot = unreal.Rotator(0, 90, 0)  # Facing toward zones (-Y direction)
        if trench_cls:
            seg = unreal.EditorLevelLibrary.spawn_actor_from_class(trench_cls, loc, rot)
        else:
            seg = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.StaticMeshActor, loc, rot)
        if seg:
            seg.set_actor_label(f"BunkerSegment_Main_{i:02d}")
            bunker_segments.append(seg)

    # Side arm: 2 segments running along +X (perpendicular safety exit)
    for i in range(1, 3):
        loc = unreal.Vector(
            PLAYER_BUNKER_LOC.x + i * segment_len,
            PLAYER_BUNKER_LOC.y,
            0
        )
        rot = unreal.Rotator(0, 0, 0)
        if trench_cls:
            seg = unreal.EditorLevelLibrary.spawn_actor_from_class(trench_cls, loc, rot)
        else:
            seg = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.StaticMeshActor, loc, rot)
        if seg:
            seg.set_actor_label(f"BunkerSegment_Side_{i:02d}")
            bunker_segments.append(seg)

    # Label
    label = spawn_text_render(world,
        unreal.Vector(PLAYER_BUNKER_LOC.x, PLAYER_BUNKER_LOC.y + 5*M, 3*M),
        "PLAYER BUNKER\n← Viewing position\nSightline to all zones",
        unreal.Color(100, 200, 255, 255), size=18.0)
    if label:
        label.set_actor_label("BunkerLabel")

    log(f"Player bunker: {len(bunker_segments)} segments placed.")
    return bunker_segments

# ---------------------------------------------------------------------------
# 6. Verdun sequence target zone marker
# ---------------------------------------------------------------------------

def place_verdun_zone(world):
    """
    Marker for where the Verdun Opening Sequence concentrates its fire.
    Positioned between the accumulation zones and the player bunker.
    This area should be completely unrecognisable after 5 minutes.
    """
    log("Placing Verdun sequence target zone marker...")

    label = spawn_text_render(world,
        unreal.Vector(VERDUN_TARGET.x, VERDUN_TARGET.y, 4*M),
        "VERDUN SEQUENCE\nTARGET ZONE\nReg → Harassing → Sustained\n→ Saturation → DRUMFIRE",
        unreal.Color(255, 80, 80, 255), size=22.0)
    if label:
        label.set_actor_label("VerdunTargetLabel")

    # Visible boundary markers — 4 poles at corners of ~20m × 20m target zone
    corners = [
        unreal.Vector(VERDUN_TARGET.x - 10*M, VERDUN_TARGET.y - 10*M, 0),
        unreal.Vector(VERDUN_TARGET.x + 10*M, VERDUN_TARGET.y - 10*M, 0),
        unreal.Vector(VERDUN_TARGET.x - 10*M, VERDUN_TARGET.y + 10*M, 0),
        unreal.Vector(VERDUN_TARGET.x + 10*M, VERDUN_TARGET.y + 10*M, 0),
    ]
    for j, corner in enumerate(corners):
        pole = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor, corner, unreal.Rotator(0, 0, 0))
        if pole:
            pole.set_actor_label(f"VerdunBoundary_{j}")
            pole.set_actor_scale3d(unreal.Vector(0.2, 0.2, 4.0))
            sm_comp = pole.get_component_by_class(unreal.StaticMeshComponent)
            if sm_comp:
                cyl = unreal.load_asset("/Engine/BasicShapes/Cylinder")
                if cyl:
                    sm_comp.set_static_mesh(cyl)

    log("Verdun target zone marked.")

# ---------------------------------------------------------------------------
# 7. Creeping barrage lane
# ---------------------------------------------------------------------------

def place_creep_lane(world):
    """
    Visual markers for the creeping barrage lane (Y=80m, runs X=0→100m).
    Numbered stakes every 10m show the advance.
    """
    log("Placing creeping barrage lane markers...")

    for i in range(11):  # 0 through 100m
        x = i * 10 * M
        y = CREEP_START.y
        loc = unreal.Vector(x, y, 0)

        marker = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor, loc, unreal.Rotator(0, 0, 0))
        if marker:
            marker.set_actor_label(f"CreepMarker_{i*10}m")
            marker.set_actor_scale3d(unreal.Vector(0.15, 0.15, 3.0))
            sm_comp = marker.get_component_by_class(unreal.StaticMeshComponent)
            if sm_comp:
                cyl = unreal.load_asset("/Engine/BasicShapes/Cylinder")
                if cyl:
                    sm_comp.set_static_mesh(cyl)

        # Label every 20m
        if i % 2 == 0:
            span_label = spawn_text_render(world,
                unreal.Vector(x, y + 2*M, 3*M),
                f"{i*10}m",
                unreal.Color(255, 200, 100, 255), size=15.0)
            if span_label:
                span_label.set_actor_label(f"CreepLaneLabel_{i*10}m")

    creep_header = spawn_text_render(world,
        unreal.Vector(5*10*M, CREEP_START.y + 5*M, 4*M),
        "CREEPING BARRAGE LANE\nShells advance L→R at 50m/min\nInfantry walking pace",
        unreal.Color(255, 150, 50, 255), size=18.0)
    if creep_header:
        creep_header.set_actor_label("CreepLaneHeader")

    log("Creeping barrage lane marked.")

# ---------------------------------------------------------------------------
# 8. BarrageDirector
# ---------------------------------------------------------------------------

def place_barrage_director(world):
    log("Placing BarrageDirector...")

    director = spawn_actor(world, BP_BARRAGE_DIRECTOR, GUN_ORIGIN,
                           unreal.Rotator(0, 180, 0))
    if director:
        director.set_actor_label("BarrageDirector")
        try:
            director.set_editor_property("trench_center_location", VERDUN_TARGET)
            director.set_editor_property("default_gun_origin",     GUN_ORIGIN)
        except Exception as e:
            log_warn(f"Could not set BarrageDirector properties: {e}")
    else:
        log_warn("BP_BarrageDirector not found — create it with parent ABarrageDirector.")
    return director

# ---------------------------------------------------------------------------
# 9. TestDirector
# ---------------------------------------------------------------------------

def place_test_director(world):
    """
    ATestDirector — drop in the level, it auto-discovers BarrageDirector
    and DeformableTerrain. Shows on-screen debug HUD during PIE.
    """
    log("Placing TestDirector...")

    td_loc = unreal.Vector(PLAYER_BUNKER_LOC.x + 3*M, PLAYER_BUNKER_LOC.y, 1*M)
    director = spawn_actor(world, BP_TEST_DIRECTOR, td_loc)
    if director:
        director.set_actor_label("TestDirector")
        try:
            director.set_editor_property("verdun_sequence_target", VERDUN_TARGET)
            director.set_editor_property("creep_start",            CREEP_START)
            director.set_editor_property("creep_end",              CREEP_END)
            director.set_editor_property("gun_origin",             GUN_ORIGIN)
        except Exception as e:
            log_warn(f"Could not set TestDirector properties: {e}")

        log("  TestDirector placed. In Blueprint, assign:")
        log("  - ShellClass_75mm, ShellClass_155mm, ShellClass_210mm, ShellClass_305mm")
        log("  - In-game: call RunCalibreComparison() or RunVerdunSequence()")
    else:
        log_warn("BP_TestDirector not found — create it with parent ATestDirector.")
    return director

# ---------------------------------------------------------------------------
# 10. Player Start
# ---------------------------------------------------------------------------

def place_player_start(world):
    """
    Inside the player bunker, facing toward the test zones.
    Standing here at the junction of the L gives sightline to all 6 zones.
    """
    log("Placing PlayerStart in bunker...")

    ps_loc = unreal.Vector(PLAYER_BUNKER_LOC.x, PLAYER_BUNKER_LOC.y, 1*M)
    ps = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart, ps_loc, unreal.Rotator(0, 270, 0))  # Facing -X toward zones
    if ps:
        ps.set_actor_label("PlayerStart_Bunker")
        log(f"  PlayerStart at ({ps_loc.x/M:.0f}m, {ps_loc.y/M:.0f}m) — facing zones")
    return ps

# ---------------------------------------------------------------------------
# 11. World settings
# ---------------------------------------------------------------------------

def configure_world_settings(world):
    log("Configuring world settings...")
    settings = world.get_world_settings()
    if settings:
        settings.set_editor_property("global_gravity_z", -980.0)
        try:
            settings.set_editor_property("default_game_mode",
                unreal.load_class(None, "/Script/OnlyTheDead.VerdunGameMode"))
        except Exception:
            log_warn("Could not set game mode — set manually in World Settings.")
    log("World settings: standard gravity, VerdunGameMode")

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def generate_test_level():
    unreal.log("=" * 65)
    unreal.log("  Only The Dead — Test Level Generator")
    unreal.log("  System verification: terrain, barrage, Lumen, Chaos, shell shock")
    unreal.log("=" * 65)

    world = create_or_get_level()
    if not world:
        unreal.log_error("[TestLevel] FAILED: Could not get world.")
        return

    with unreal.ScopedEditorTransaction("Generate Test Level") as _:
        setup_environment(world)
        create_terrain(world)
        place_zone_markers(world)
        place_measurement_grid(world)
        create_player_bunker(world)
        place_verdun_zone(world)
        place_creep_lane(world)
        place_barrage_director(world)
        place_test_director(world)
        place_player_start(world)
        configure_world_settings(world)

    unreal.EditorLevelLibrary.save_current_level()

    unreal.log("=" * 65)
    unreal.log("  TEST LEVEL GENERATED")
    unreal.log("")
    unreal.log("  Layout (from player bunker at 90m, 60m):")
    unreal.log("    Zone 0 — 75mm × 1    at  10m, 20m   (70m NW of bunker)")
    unreal.log("    Zone 1 — 155mm × 1   at  30m, 20m   (65m W  of bunker)")
    unreal.log("    Zone 2 — 210mm × 1   at  60m, 20m   (50m W  of bunker)")
    unreal.log("    Zone 3 — 75mm × 8    at  10m, 50m   (80m NW of bunker)")
    unreal.log("    Zone 4 — 155mm × 3   at  40m, 50m   (60m W  of bunker)")
    unreal.log("    Zone 5 — 305mm × 1   at  75m, 50m   (20m W  of bunker)")
    unreal.log("    Verdun target         at  40m, 70m   (55m W  of bunker)")
    unreal.log("    Creep lane            at Y=80m, X=0→100m")
    unreal.log("")
    unreal.log("  SETUP REQUIRED BEFORE PLAY:")
    unreal.log("  1. Create BP_TestDirector (parent: ATestDirector)")
    unreal.log("     Assign ShellClass_75mm, _155mm, _210mm, _305mm")
    unreal.log("  2. Create BP_BarrageDirector (parent: ABarrageDirector)")
    unreal.log("     Assign DefaultShellClass + HeavyShellClass")
    unreal.log("  3. Create BP_DeformableTerrain (parent: ADeformableTerrain)")
    unreal.log("     Assign M_DeformableTerrain (VertexColor.R → mud blend)")
    unreal.log("  4. In Project Settings > Rendering:")
    unreal.log("     - Global Illumination: Lumen")
    unreal.log("     - Reflections: Lumen")
    unreal.log("     - Shadows: Virtual Shadow Maps")
    unreal.log("")
    unreal.log("  TEST SEQUENCES (call on BP_TestDirector in PIE):")
    unreal.log("  RunCalibreComparison()  — 75+155+210mm side by side, then accumulation")
    unreal.log("  RunZoneTest(5)          — single 305mm siege mortar, maximum crater")
    unreal.log("  RunCreepingBarrage()    — wall of fire advances across the field")
    unreal.log("  RunSaturationTest()     — 60 shells/min for 30s")
    unreal.log("  RunVerdunSequence()     — 5-phase escalation, ~5min to Drumfire")
    unreal.log("  ResetAll()              — flatten terrain, stop barrages, clear shock")
    unreal.log("")
    unreal.log("  Watch:")
    unreal.log("  - Lumen flash: orange-white burst illuminating smoke from below")
    unreal.log("  - Terrain: vertices deform in real time, collision updates async")
    unreal.log("  - VertexColor.R: ground texture shifts undisturbed→churned")
    unreal.log("  - Shell shock: accumulates toward catatonia during Verdun seq.")
    unreal.log("  - Smoke: Niagara columns accumulate, volumetric fog thickens")
    unreal.log("=" * 65)

generate_test_level()
