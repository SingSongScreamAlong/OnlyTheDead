"""
generate_test_level.py  —  "Only The Dead" Environmental Horror Observer Level
===============================================================================
Run from UE5.7 Editor Python console:
    File > Execute Python Script > Scripts/generate_test_level.py

WHAT THIS CREATES
-----------------
An atmospheric bombardment environment — not a test grid but a real battlefield
section. The player stands in a fire trench and watches the world get destroyed.

No triggers, no UI sequences. Play begins, 8 seconds of silence, then the first
registration shot. Then the second. Then it escalates.

By the 5-minute mark — Drumfire — individual explosions are indistinguishable.
The ground in front of the trench has been churned from chalk limestone to dark
mud. Craters overlap. Nothing lives on the surface.

LAYOUT
------
The level is 150m × 100m of open ground. The player's trench runs along the
eastern edge. The open field faces west — that's where every shell lands.

    ←── Open field (shell impact area) ──────────────┐  ┌── Trench ──→
                                                       │  │
    Craters accumulate here. Vertex colors shift       │  │  Player starts
    from pale chalk (R=0) to dark churned mud (R=1).   │  │  here, facing
    Smoke columns rise and persist. The Lumen          │  │  west across
    flash from each impact briefly illuminates the     │  │  the field.
    smoke from below — exactly as in photographs.      │  │
                                                       │  │
    ←── 100m+ ──────────────────────────────────→     │  ├── Cover bays
                                                       │  │
    Gun origins: off the west and south map edges.     │  └────────────
    Shells come from multiple directions — whistles    │
    vary in pitch and approach angle.                  │
                                                       └── East edge

WHAT TO OBSERVE
---------------
Ground deformation: watch craters form in real time. Move close to a fresh
impact and look at the rim — the Gaussian lip is raised, the bowl is depressed,
the vertex color around it has shifted toward churned mud. Come back after
10 more impacts — it's been partially filled by a new crater from the left.

Lumen flash: face away from an incoming shell. The flash will still illuminate
you from behind — the GI bounce off the smoke column hits your back. Correct.

Shell shock accumulation: during Phase 4 (Saturation), stand in the open. By
the time Phase 5 (Drumfire) begins you should be Catatonic — unable to move
properly, screen desaturated, movement slow. Duck into the trench: the cover
value reduces incoming shock. Morale partially recovers.

Smoke accumulation: the Niagara smoke columns from each impact don't disappear.
By mid-session, the western field is partially obscured by hanging smoke
interacting with the volumetric fog. The sky appears to darken.

SETUP REQUIRED BEFORE PLAY
---------------------------
1. Create BP_BarrageDirector (parent: ABarrageDirector)
   - Assign DefaultShellClass (BP_ArtilleryShell_75mm)
   - Assign HeavyShellClass   (BP_ArtilleryShell_155mm)
   - bAutoStartOnPlay         = true  (set by this script)
2. Create BP_DeformableTerrain (parent: ADeformableTerrain)
   - Assign a Substrate material: Layer A = chalk limestone, Layer B = wet mud
   - Blend weight = VertexColor.R
3. Project Settings → Rendering:
   - Global Illumination: Lumen
   - Reflections: Lumen
   - Shadows: Virtual Shadow Maps
   - Volumetric Fog: enabled (height fog component below)
"""

import unreal
import math
import random

# ---------------------------------------------------------------------------
# Scale constants
# ---------------------------------------------------------------------------

M = 100.0    # 1 metre in UE units (cm)

# Level dimensions
FIELD_WIDTH  = 15 * M * 10   # 150m west–east  (shell impact area)
FIELD_DEPTH  = 10 * M * 10   # 100m north–south

# Trench runs along the eastern edge, set back 8m from the field edge
TRENCH_X      = FIELD_WIDTH - 8 * M    # ~142m from west edge
TRENCH_LENGTH = FIELD_DEPTH            # Runs full north–south

# Field centre (where most shells will land)
FIELD_CENTRE = unreal.Vector(FIELD_WIDTH * 0.4, FIELD_DEPTH * 0.5, 0)  # Slightly west of centre

# Player start: inside the trench, facing west across the field
PLAYER_START_LOC = unreal.Vector(TRENCH_X + 2*M, FIELD_DEPTH * 0.5, 1*M)
PLAYER_START_ROT = unreal.Rotator(0, 180, 0)  # Facing west (-X)

# Gun origins: off the west, south-west, and north-west map edges.
# Multiple origins = shells come from different angles = different whistle pitches
# and approach directions. Historically German guns surrounded Verdun.
GUN_ORIGINS = [
    unreal.Vector(-30*M, FIELD_DEPTH * 0.5, 80*M),    # Due west — main battery
    unreal.Vector(-20*M, -20*M,             70*M),    # South-west
    unreal.Vector(-20*M, FIELD_DEPTH+20*M,  65*M),    # North-west
    unreal.Vector(FIELD_WIDTH*0.3, -30*M,   75*M),    # South (enfilading fire)
]

# Blueprint paths
BP_BARRAGE_DIRECTOR   = "/Game/POC_TrenchArtillery/Blueprints/BP_BarrageDirector"
BP_DEFORMABLE_TERRAIN = "/Game/POC_TrenchArtillery/Blueprints/BP_DeformableTerrain"
BP_TRENCH_SEGMENT     = "/Game/POC_TrenchArtillery/Blueprints/BP_TrenchSegment"
BP_TEST_DIRECTOR      = "/Game/POC_TrenchArtillery/Blueprints/BP_TestDirector"


def log(msg):       unreal.log(f"[ObserverLevel] {msg}")
def warn(msg):      unreal.log_warning(f"[ObserverLevel] {msg}")

def get_class(path):
    try:   return unreal.load_class(None, path + "_C")
    except: return None

def spawn(cls_path, loc, rot=None, fallback=None):
    rot = rot or unreal.Rotator(0, 0, 0)
    cls = get_class(cls_path)
    if cls:
        return unreal.EditorLevelLibrary.spawn_actor_from_class(cls, loc, rot)
    if fallback:
        return unreal.EditorLevelLibrary.spawn_actor_from_class(fallback, loc, rot)
    warn(f"Cannot spawn {cls_path}")
    return None

# ---------------------------------------------------------------------------
# 1. Lighting — WWI overcast winter, Lumen enabled
# ---------------------------------------------------------------------------

def setup_lighting(world):
    log("Placing lighting (overcast winter sky, Lumen)...")

    # Directional light: flat overcast — no harsh shadows, uniform grey light.
    # Historical photographs of Verdun are always grey, low-contrast, featureless sky.
    try:
        dl = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.load_class(None, "/Script/Engine.DirectionalLight"),
            unreal.Vector(0, 0, 8000), unreal.Rotator(-35, 15, 0))
        if dl:
            c = dl.get_component_by_class(unreal.DirectionalLightComponent)
            if c:
                c.set_editor_property("intensity", 2.5)
                c.set_editor_property("light_color", unreal.LinearColor(0.82, 0.87, 1.0, 1.0))
                c.set_editor_property("atmosphere_sun_light", True)
                c.set_editor_property("cast_shadows", True)
            dl.set_actor_label("DirLight_WinterOvercast")
    except Exception as e:
        warn(f"Directional light: {e}")

    # Sky light: real-time capture so Lumen updates as smoke fills the air.
    # As smoke columns accumulate the reflected light colour darkens toward ochre.
    try:
        sl = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.load_class(None, "/Script/Engine.SkyLight"),
            unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
        if sl:
            c = sl.get_component_by_class(unreal.SkyLightComponent)
            if c:
                c.set_editor_property("intensity", 0.4)
                c.set_editor_property("light_color", unreal.LinearColor(0.65, 0.7, 0.85, 1.0))
                c.set_editor_property("real_time_capture", True)
            sl.set_actor_label("SkyLight_Lumen")
    except Exception as e:
        warn(f"Sky light: {e}")

    # Volumetric height fog: the persistent smoke columns from Niagara FX
    # interact with this, building visual density over the session.
    # Starts thin — by Drumfire, the field is partially obscured.
    try:
        fog = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.load_class(None, "/Script/Engine.ExponentialHeightFog"),
            unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
        if fog:
            c = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
            if c:
                c.set_editor_property("fog_density", 0.015)
                c.set_editor_property("fog_height_falloff", 0.15)
                c.set_editor_property("fog_inscattering_color",
                    unreal.LinearColor(0.65, 0.60, 0.50, 1.0))   # Ochre-grey, cordite smoke
                c.set_editor_property("start_distance", 0.0)
                c.set_editor_property("fog_cutoff_distance", 60000.0)
                c.set_editor_property("volumetric_fog", True)
                c.set_editor_property("volumetric_fog_scattering_distribution", 0.15)
                c.set_editor_property("volumetric_fog_albedo",
                    unreal.LinearColor(0.75, 0.70, 0.60, 1.0))
                c.set_editor_property("volumetric_fog_extinction_scale", 1.2)
            fog.set_actor_label("HeightFog_BattlefieldSmoke")
    except Exception as e:
        warn(f"Height fog: {e}")

    # Post-process volume: full scene. Lumen settings configured in Project Settings.
    # Low exposure so shell flashes are dramatically visible.
    try:
        ppv = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.load_class(None, "/Script/Engine.PostProcessVolume"),
            unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
        if ppv:
            ppv.set_editor_property("infinite_extent", True)
            ppv.set_actor_label("PostProcess_Main")
    except Exception as e:
        warn(f"Post-process: {e}")

    log("  Lighting placed.")

# ---------------------------------------------------------------------------
# 2. Deformable terrain — the field that gets destroyed
# ---------------------------------------------------------------------------

def create_terrain(world):
    log(f"Creating deformable terrain ({FIELD_WIDTH/M:.0f}m × {FIELD_DEPTH/M:.0f}m)...")

    terrain_origin = unreal.Vector(-5*M, -5*M, -2*M)   # Slight below-ground
    terrain = spawn(BP_DEFORMABLE_TERRAIN, terrain_origin,
                    fallback=unreal.StaticMeshActor)
    if terrain:
        terrain.set_actor_label("DeformableTerrain_Field")
        try:
            # Terrain covers the open field + trench area + margin
            terrain.set_editor_property("terrain_width",      FIELD_WIDTH + 10*M)
            terrain.set_editor_property("terrain_depth",      FIELD_DEPTH + 10*M)
            terrain.set_editor_property("grid_resolution_cm", 80.0)   # Tighter than test level
        except Exception as e:
            warn(f"Terrain properties: {e}")
        log(f"  Terrain: {FIELD_WIDTH/M:.0f}m × {FIELD_DEPTH/M:.0f}m, 80cm grid")
    else:
        warn("BP_DeformableTerrain not found — create it (parent: ADeformableTerrain).")
    return terrain

# ---------------------------------------------------------------------------
# 3. The trench — player shelter and observation position
# ---------------------------------------------------------------------------

def create_trench(world):
    """
    L-shaped fire trench with two observer bays facing west.

    Structure:
      - Main run: 40m long, north-south, at TRENCH_X
      - Two forward bays: 4m deep, pointing west — fire positions
      - Firestep (implicit in TrenchSegment mesh): player can stand and look
        over the parapet at the open field

    The trench provides:
      - Cover value: reduces morale damage and shell shock from distant impacts
      - A physically walkable space so the player can move along the trench
        and change their observation angle
      - A sense of scale against the open field — the trench is tiny
    """
    log("Creating fire trench (L-shaped, observer bays)...")

    trench_cls = get_class(BP_TRENCH_SEGMENT)
    placed = []
    seg_len = 3 * M  # 3m per segment

    def place_seg(loc, rot_yaw, label):
        rot = unreal.Rotator(0, rot_yaw, 0)
        if trench_cls:
            seg = unreal.EditorLevelLibrary.spawn_actor_from_class(trench_cls, loc, rot)
        else:
            seg = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.StaticMeshActor, loc, rot)
        if seg:
            seg.set_actor_label(label)
            placed.append(seg)
        return seg

    centre_y = FIELD_DEPTH * 0.5

    # Main trench run — 8 segments north-south (24m total)
    for i in range(8):
        y = centre_y - 4 * seg_len + i * seg_len
        place_seg(unreal.Vector(TRENCH_X, y, 0), 0, f"Trench_Main_{i:02d}")

    # Forward observer bay 1 — pointing west at FIELD_CENTRE
    bay1_y = centre_y - seg_len
    for i in range(2):
        place_seg(unreal.Vector(TRENCH_X - (i+1)*seg_len, bay1_y, 0), 90, f"Trench_Bay1_{i}")

    # Forward observer bay 2 — pointing west, offset north
    bay2_y = centre_y + seg_len
    for i in range(2):
        place_seg(unreal.Vector(TRENCH_X - (i+1)*seg_len, bay2_y, 0), 90, f"Trench_Bay2_{i}")

    # Rear communication trench — escape route
    for i in range(3):
        place_seg(unreal.Vector(TRENCH_X + (i+1)*seg_len, centre_y, 0), 90, f"Trench_Rear_{i}")

    log(f"  Trench: {len(placed)} segments.")
    return placed

# ---------------------------------------------------------------------------
# 4. BarrageDirector — fully configured, auto-starts
# ---------------------------------------------------------------------------

def place_barrage_director(world):
    """
    The BarrageDirector with:
      - bAutoStartOnPlay = true: barrage begins 8s after Play
      - bAutoWideAreaHarassment = true: persistent wide scatter (4 shells/min)
      - bLoopSequence = true: restarts the Verdun sequence after ~5min
      - Multiple gun origins spread around the map perimeter
      - TrenchCenterLocation = centre of the open field
    """
    log("Placing BarrageDirector (auto-start, wide coverage, looping)...")

    # Place the actor at the main gun origin position (off-map west edge)
    director = spawn(BP_BARRAGE_DIRECTOR,
                     unreal.Vector(-25*M, FIELD_DEPTH * 0.5, 60*M),
                     unreal.Rotator(0, 90, 0))
    if not director:
        warn("BP_BarrageDirector not found. Create it (parent: ABarrageDirector), assign shell classes.")
        return None

    director.set_actor_label("BarrageDirector")

    try:
        # Target: centre of the open field, weighted slightly west of true centre
        # so more impacts fall in the highly visible mid-field area
        director.set_editor_property("trench_center_location", FIELD_CENTRE)

        # All four gun origins: shells arrive from different bearings
        director.set_editor_property("gun_origins", GUN_ORIGINS)
        director.set_editor_property("default_gun_origin", GUN_ORIGINS[0])

        # Auto-start configuration
        director.set_editor_property("b_auto_start_on_play",          True)
        director.set_editor_property("auto_start_delay_sec",           8.0)
        director.set_editor_property("b_auto_wide_area_harassment",    True)
        director.set_editor_property("wide_area_radius_cm",            6000.0)  # 60m
        director.set_editor_property("wide_area_shells_per_minute",    4.0)
        director.set_editor_property("b_loop_sequence",                True)

    except Exception as e:
        warn(f"Could not set BarrageDirector properties ({e}) — set manually in Details panel.")
        log("  Required settings:")
        log("    TrenchCenterLocation = centre of field")
        log("    bAutoStartOnPlay     = true")
        log("    bAutoWideAreaHarassment = true")
        log("    bLoopSequence        = true")

    log("  BarrageDirector placed. Assign DefaultShellClass + HeavyShellClass in Blueprint.")
    return director

# ---------------------------------------------------------------------------
# 5. TestDirector — HUD only, no auto-sequences
# ---------------------------------------------------------------------------

def place_test_director(world):
    """
    The TestDirector provides the on-screen debug HUD (shell shock level,
    active waves, terrain stats, shells fired) but with bAutoRunComparison
    turned off — it's just a passive readout, not a test sequencer.
    """
    log("Placing TestDirector (HUD readout only)...")

    loc = unreal.Vector(TRENCH_X + 1*M, FIELD_DEPTH * 0.5, 1*M)
    td = spawn(BP_TEST_DIRECTOR, loc)
    if td:
        td.set_actor_label("TestDirector_HUD")
        try:
            td.set_editor_property("b_auto_run_comparison",               False)
            td.set_editor_property("b_auto_start_verdun_after_comparison", False)
            td.set_editor_property("verdun_sequence_target", FIELD_CENTRE)
            td.set_editor_property("gun_origin", GUN_ORIGINS[0])
        except Exception as e:
            warn(f"TestDirector properties: {e}")
    else:
        warn("BP_TestDirector not found. HUD won't show. Create it (parent: ATestDirector).")

    return td

# ---------------------------------------------------------------------------
# 6. Player start — in the trench, facing the open field
# ---------------------------------------------------------------------------

def place_player_start(world):
    log("Placing PlayerStart in fire trench (facing west)...")

    ps = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        PLAYER_START_LOC,
        PLAYER_START_ROT)
    if ps:
        ps.set_actor_label("PlayerStart_Trench")
        log(f"  PlayerStart: ({PLAYER_START_LOC.x/M:.0f}m, {PLAYER_START_LOC.y/M:.0f}m), facing west")
    return ps

# ---------------------------------------------------------------------------
# 7. Distance markers — subtle, so the player can judge crater scale
#    Nothing ostentatious. Small white stakes at 10m, 30m, 50m, 70m from trench.
# ---------------------------------------------------------------------------

def place_distance_markers(world):
    """
    Four rows of low stakes stretching west from the trench at 10m intervals.
    They show scale when a 210mm crater forms — you can see the stakes
    disappear into the rim.
    No labels, no colours. Just white cylinders — like boundary markers in
    a field. They'll be buried or tilted by the end of the session.
    """
    log("Placing distance markers (10m intervals, west from trench)...")

    distances_m  = [10, 20, 30, 50, 70, 100]   # m west of trench
    offsets_y    = [-20*M, 0, 20*M]              # three rows (north, centre, south)

    placed = 0
    for d_m in distances_m:
        for dy in offsets_y:
            x = TRENCH_X - d_m * M
            y = FIELD_DEPTH * 0.5 + dy
            stake = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.StaticMeshActor,
                unreal.Vector(x, y, 0),
                unreal.Rotator(0, 0, 0))
            if stake:
                stake.set_actor_label(f"DistMarker_{d_m}m_{int(dy/M)}m")
                stake.set_actor_scale3d(unreal.Vector(0.12, 0.12, 1.8))  # Thin, 1.8m tall
                sm = stake.get_component_by_class(unreal.StaticMeshComponent)
                if sm:
                    cyl = unreal.load_asset("/Engine/BasicShapes/Cylinder")
                    if cyl:
                        sm.set_static_mesh(cyl)
                placed += 1

    log(f"  {placed} distance markers placed (10-100m from trench).")

# ---------------------------------------------------------------------------
# 8. World settings
# ---------------------------------------------------------------------------

def configure_world(world):
    log("Configuring world settings...")
    settings = world.get_world_settings()
    if settings:
        settings.set_editor_property("global_gravity_z", -980.0)
        try:
            settings.set_editor_property("default_game_mode",
                unreal.load_class(None, "/Script/OnlyTheDead.VerdunGameMode"))
        except Exception:
            warn("Could not set game mode — set to VerdunGameMode manually.")

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def generate_observer_level():
    unreal.log("=" * 65)
    unreal.log("  Only The Dead — Environmental Horror Observer Level")
    unreal.log("  Verdun, February 21, 1916")
    unreal.log("=" * 65)

    world = unreal.UnrealEditorSubsystem.get_editor_world()
    if not world:
        unreal.log_error("[ObserverLevel] FAILED: Could not get world.")
        return

    with unreal.ScopedEditorTransaction("Generate Observer Level") as _:
        setup_lighting(world)
        create_terrain(world)
        create_trench(world)
        place_barrage_director(world)
        place_test_director(world)
        place_player_start(world)
        place_distance_markers(world)
        configure_world(world)

    unreal.EditorLevelLibrary.save_current_level()

    unreal.log("=" * 65)
    unreal.log("  OBSERVER LEVEL GENERATED")
    unreal.log("")
    unreal.log("  LAYOUT:")
    unreal.log(f"  Open field:     {FIELD_WIDTH/M:.0f}m × {FIELD_DEPTH/M:.0f}m")
    unreal.log(f"  Trench:         east edge at X={TRENCH_X/M:.0f}m, runs full N-S")
    unreal.log(f"  Player start:   in trench, facing west across the field")
    unreal.log(f"  Shell origins:  {len(GUN_ORIGINS)} gun positions (W, SW, NW, S)")
    unreal.log(f"  Primary target: field centre ({FIELD_CENTRE.x/M:.0f}m, {FIELD_CENTRE.y/M:.0f}m)")
    unreal.log("")
    unreal.log("  WHAT HAPPENS AUTOMATICALLY:")
    unreal.log("  T+0s:    Persistent wide-area harassing fire begins (4 shells/min)")
    unreal.log("           Shells land anywhere in the 60m radius — no safe ground")
    unreal.log("  T+8s:    Single registration shot (Phase 1 of Verdun sequence)")
    unreal.log("           Then silence. Then the second shot.")
    unreal.log("  T+40s:   Harassing fire starts (8 shells/min, wider scatter)")
    unreal.log("  T+103s:  Sustained barrage begins — 75mm + 155mm mixed")
    unreal.log("  T+198s:  Saturation — 60 shells/min. Ground churning.")
    unreal.log("  T+293s:  DRUMFIRE — 120 shells/min. Individual impacts inaudible.")
    unreal.log("           VertexColor.R across the field approaches 1.0 (fully churned).")
    unreal.log("  T+315s:  Loop restarts (Drumfire continues into Phase 1 of repeat)")
    unreal.log("")
    unreal.log("  SETUP REQUIRED:")
    unreal.log("  1. Create BP_BarrageDirector, assign DefaultShellClass + HeavyShellClass")
    unreal.log("  2. Create BP_DeformableTerrain, assign Substrate mud/chalk material")
    unreal.log("  3. Project Settings → Rendering: Lumen GI, Lumen Reflections, VSM")
    unreal.log("")
    unreal.log("  TO WATCH:")
    unreal.log("  - Stand in trench bay — look west across the field")
    unreal.log("  - Watch the ground change colour (chalk → churned mud)")
    unreal.log("  - Face away from an impact — Lumen GI bounce hits your back")
    unreal.log("  - Stand in the open during Phase 4 — feel shell shock accumulate")
    unreal.log("  - Return to trench — cover reduces shock rate")
    unreal.log("=" * 65)

generate_observer_level()
