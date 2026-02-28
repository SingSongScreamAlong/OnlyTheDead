"""
setup_and_generate.py — "Only The Dead" : One-shot setup + level generation
============================================================================

Run this ONCE from the UE 5.7 Editor Python console after the C++ has
compiled. It creates every Blueprint asset, the terrain material, and
generates the observer level.

After it finishes: press Play.

    File > Execute Python Script > Scripts/setup_and_generate.py

The user does exactly three things:
    1. Open OnlyTheDead.uproject (compiles C++ automatically)
    2. Run this script
    3. Press Play

Nothing else is needed for the bombardment to start and the terrain to
deform in real time.

What runs without Blueprint asset assignments (pure C++):
    - Ballistic flight (ProjectileMovementComponent)
    - Terrain deformation (FDynamicMesh3 vertex displacement)
    - Gaussian crater profile with raised rim
    - Lumen dynamic GI flash (AExplosionLight)
    - Radial damage, morale damage, shell shock (ConcussionComponent)
    - Fallback physics debris (simple cubes — replace with Chaos GC later)

What this script adds on top:
    - Visible shell in flight (Engine sphere, scaled as a placeholder)
    - Chalk→mud terrain material driven by VertexColor.R
    - All Blueprint wiring so DefaultShellClass is valid
"""

import unreal

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

BP_FOLDER  = "/Game/POC_TrenchArtillery/Blueprints"
MAT_FOLDER = "/Game/POC_TrenchArtillery/Materials"

# Full asset paths
PATH_SHELL_LIGHT    = f"{BP_FOLDER}/BP_ArtilleryShell"
PATH_SHELL_HEAVY    = f"{BP_FOLDER}/BP_ArtilleryShell_Heavy"
PATH_DIRECTOR       = f"{BP_FOLDER}/BP_BarrageDirector"
PATH_TERRAIN        = f"{BP_FOLDER}/BP_DeformableTerrain"
PATH_TRENCH         = f"{BP_FOLDER}/BP_TrenchSegment"
PATH_TEST_DIRECTOR  = f"{BP_FOLDER}/BP_TestDirector"
PATH_TERRAIN_MAT    = f"{MAT_FOLDER}/M_Terrain_ChalkMud"

# Level layout constants (cm)
FIELD_W      = 15000.0   # 150m west→east
FIELD_D      = 10000.0   # 100m north→south
TRENCH_X     = FIELD_W - 800.0               # trench at east edge
FIELD_CENTRE = unreal.Vector(FIELD_W * 0.4, FIELD_D * 0.5, 0.0)

# Gun positions off the west/south-west/north-west edges
# Multiple origins: shells arrive from different bearings, different whistle angles
GUN_ORIGINS = [
    unreal.Vector(-3000.0, FIELD_D * 0.50, 8000.0),   # due west — main battery
    unreal.Vector(-2000.0, -2000.0,         7000.0),   # south-west
    unreal.Vector(-2000.0, FIELD_D + 2000,  6500.0),   # north-west
    unreal.Vector(FIELD_W * 0.3, -3000.0,   7500.0),   # south (enfilading)
]

# ---------------------------------------------------------------------------
# Utilities
# ---------------------------------------------------------------------------

_at = unreal.AssetToolsHelpers.get_asset_tools()
_al = unreal.EditorAssetLibrary
_mel = unreal.MaterialEditingLibrary

def log(msg):   unreal.log(f"[Setup] {msg}")
def warn(msg):  unreal.log_warning(f"[Setup] {msg}")

def ensure_folder(path):
    if not _al.does_directory_exist(path):
        _al.make_directory(path)

def make_blueprint(name, folder, cpp_class_name):
    """
    Create a Blueprint child of a C++ class. Idempotent — if it already
    exists, just load and return it.
    Returns (asset, full_path) or (None, full_path) on failure.
    """
    full_path = f"{folder}/{name}"
    if _al.does_asset_exist(full_path):
        log(f"  {name}: already exists, loading.")
        return _al.load_asset(full_path), full_path

    parent = unreal.find_class(cpp_class_name)
    if not parent:
        # C++ hasn't compiled yet — most likely cause
        warn(f"  {name}: cannot find C++ class '{cpp_class_name}'. "
             f"Has the project compiled?")
        return None, full_path

    factory = unreal.BlueprintFactory()
    factory.parent_class = parent

    bp = _at.create_asset(name, folder, unreal.Blueprint, factory)
    if bp:
        _al.save_asset(full_path)
        log(f"  {name}: created.")
    else:
        warn(f"  {name}: create_asset returned None.")
    return bp, full_path

def bp_class(path):
    """Load the generated _C class for a Blueprint asset path."""
    try:
        return unreal.load_class(None, path + "_C")
    except Exception:
        return None

def bp_cdo(path):
    """Get the Class Default Object for a Blueprint path."""
    cls = bp_class(path)
    if cls:
        try:
            return unreal.get_default_object(cls)
        except Exception:
            return None
    return None

def try_set(obj, prop, value, context=""):
    """set_editor_property with a warning on failure."""
    try:
        obj.set_editor_property(prop, value)
    except Exception as e:
        warn(f"  {context} — could not set '{prop}': {e}")

def spawn_actor(cls_path, loc, rot=None):
    rot = rot or unreal.Rotator(0, 0, 0)
    cls = bp_class(cls_path)
    if cls:
        return unreal.EditorLevelLibrary.spawn_actor_from_class(cls, loc, rot)
    warn(f"  spawn_actor: {cls_path} not found.")
    return None


# ===========================================================================
# PART 1 — Blueprint assets
# ===========================================================================

def create_blueprints():
    log("─── Creating Blueprint assets ───")
    ensure_folder(BP_FOLDER)

    make_blueprint("BP_ArtilleryShell",       BP_FOLDER, "ArtilleryShell")
    make_blueprint("BP_ArtilleryShell_Heavy", BP_FOLDER, "ArtilleryShell")
    make_blueprint("BP_BarrageDirector",      BP_FOLDER, "BarrageDirector")
    make_blueprint("BP_DeformableTerrain",    BP_FOLDER, "DeformableTerrain")
    make_blueprint("BP_TrenchSegment",        BP_FOLDER, "TrenchSegment")
    make_blueprint("BP_TestDirector",         BP_FOLDER, "TestDirector")


# ===========================================================================
# PART 2 — Shell CDO configuration
# ===========================================================================

def configure_shells():
    """
    Light shell (BP_ArtilleryShell):
        - ShellData defaults from C++ constructor = correct 75mm values.
        - Assign Engine sphere mesh as a placeholder so the shell is visible
          in flight. Replace with a proper elongated shell mesh later.

    Heavy shell (BP_ArtilleryShell_Heavy):
        - Override ShellData on CDO to 155mm values (weight 43kg, deeper
          craters, 8s whistle — the howitzer arc is much longer than field gun).
        - Larger sphere scale so you can visually distinguish calibres.
    """
    log("─── Configuring shell Blueprint CDOs ───")

    sphere_mesh = unreal.load_asset("/Engine/BasicShapes/Sphere")

    # --- 75mm ---
    cdo = bp_cdo(PATH_SHELL_LIGHT)
    if cdo:
        try:
            mesh = cdo.get_component_by_class(unreal.StaticMeshComponent)
            if mesh and sphere_mesh:
                mesh.set_static_mesh(sphere_mesh)
                # Small elongated sphere: looks vaguely like a shell from distance
                mesh.set_editor_property(
                    "relative_scale3d", unreal.Vector(0.12, 0.12, 0.28))
                log("  BP_ArtilleryShell: sphere mesh set (75mm placeholder)")
        except Exception as e:
            warn(f"  BP_ArtilleryShell mesh: {e}")
        _al.save_asset(PATH_SHELL_LIGHT)

    # --- 155mm ---
    cdo = bp_cdo(PATH_SHELL_HEAVY)
    if cdo:
        # ShellData struct — get, modify, set back
        try:
            sd = cdo.get_editor_property("shell_data")
            if sd:
                try_set(sd, "shell_weight_kg",           43.0,    "155mm")
                try_set(sd, "lethal_radius",           1500.0,    "155mm")
                try_set(sd, "wound_radius",            4000.0,    "155mm")
                try_set(sd, "shock_radius",            8000.0,    "155mm")
                try_set(sd, "crater_radius",            600.0,    "155mm")
                try_set(sd, "max_damage",               750.0,    "155mm")
                try_set(sd, "morale_damage_lethal",      55.0,    "155mm")
                try_set(sd, "morale_damage_shock",       20.0,    "155mm")
                try_set(sd, "launch_speed",            4500.0,    "155mm")
                try_set(sd, "incoming_whistle_seconds",   8.0,    "155mm")
                try_set(sd, "display_name", "155mm C/1915 Howitzer", "155mm")
                cdo.set_editor_property("shell_data", sd)
                log("  BP_ArtilleryShell_Heavy: 155mm ShellData applied.")
        except Exception as e:
            warn(f"  BP_ArtilleryShell_Heavy ShellData: {e}")

        try:
            mesh = cdo.get_component_by_class(unreal.StaticMeshComponent)
            if mesh and sphere_mesh:
                mesh.set_static_mesh(sphere_mesh)
                mesh.set_editor_property(
                    "relative_scale3d", unreal.Vector(0.22, 0.22, 0.50))
        except Exception as e:
            warn(f"  BP_ArtilleryShell_Heavy mesh: {e}")

        _al.save_asset(PATH_SHELL_HEAVY)


# ===========================================================================
# PART 3 — Terrain material
# ===========================================================================

def create_terrain_material():
    """
    M_Terrain_ChalkMud — the visual story of the bombardment.

    VertexColor.R drives everything:
        R = 0.0 : undisturbed chalk limestone — pale, dry, rough
        R = 1.0 : churned mud             — dark, wet, slightly smoother

    BaseColor = lerp(chalk, mud, R)
    Roughness = lerp(0.85, 0.72, R)   — wet mud has faint specular

    This is a plain UMaterial (not Substrate) for maximum compatibility.
    Swap for a Substrate version later to get proper wet-mud multi-lobe BRDF.
    """
    log("─── Creating M_Terrain_ChalkMud material ───")
    ensure_folder(MAT_FOLDER)

    if _al.does_asset_exist(PATH_TERRAIN_MAT):
        log("  Material already exists — loading.")
        return _al.load_asset(PATH_TERRAIN_MAT)

    mat = _at.create_asset("M_Terrain_ChalkMud", MAT_FOLDER,
                             unreal.Material, unreal.MaterialFactoryNew())
    if not mat:
        warn("  Failed to create material asset.")
        return None

    try:
        mel = _mel

        # --- VertexColor node ---
        vc = mel.create_material_expression(
            mat, unreal.MaterialExpressionVertexColor, -800, 0)

        # --- ComponentMask: extract R channel as scalar ---
        rm = mel.create_material_expression(
            mat, unreal.MaterialExpressionComponentMask, -600, 0)
        rm.set_editor_property("r", True)
        rm.set_editor_property("g", False)
        rm.set_editor_property("b", False)
        rm.set_editor_property("a", False)
        mel.connect_material_expressions(vc, "", rm, "")

        # ---- BaseColor ----

        # Chalk: warm off-white limestone, slightly yellowed
        chalk_col = mel.create_material_expression(
            mat, unreal.MaterialExpressionConstant3Vector, -900, -160)
        chalk_col.set_editor_property(
            "constant", unreal.LinearColor(0.79, 0.74, 0.60, 1.0))

        # Churned mud: dark, almost black wet earth
        mud_col = mel.create_material_expression(
            mat, unreal.MaterialExpressionConstant3Vector, -900, 160)
        mud_col.set_editor_property(
            "constant", unreal.LinearColor(0.13, 0.09, 0.05, 1.0))

        bc_lerp = mel.create_material_expression(
            mat, unreal.MaterialExpressionLinearInterpolate, -300, 0)
        mel.connect_material_expressions(chalk_col, "", bc_lerp, "A")
        mel.connect_material_expressions(mud_col,   "", bc_lerp, "B")
        mel.connect_material_expressions(rm,        "", bc_lerp, "Alpha")
        mel.connect_material_property(
            bc_lerp, "", unreal.MaterialProperty.MP_BASE_COLOR)

        # ---- Roughness ----

        chalk_r = mel.create_material_expression(
            mat, unreal.MaterialExpressionConstant, -600, 240)
        chalk_r.r = 0.85   # Chalk: rough, powdery

        mud_r = mel.create_material_expression(
            mat, unreal.MaterialExpressionConstant, -600, 290)
        mud_r.r = 0.72     # Wet mud: smoother, slight glint in Lumen

        rgh_lerp = mel.create_material_expression(
            mat, unreal.MaterialExpressionLinearInterpolate, -300, 240)
        mel.connect_material_expressions(chalk_r,  "", rgh_lerp, "A")
        mel.connect_material_expressions(mud_r,    "", rgh_lerp, "B")
        mel.connect_material_expressions(rm,       "", rgh_lerp, "Alpha")
        mel.connect_material_property(
            rgh_lerp, "", unreal.MaterialProperty.MP_ROUGHNESS)

        # ---- Metallic: always 0 (dirt is not metallic) ----
        zero = mel.create_material_expression(
            mat, unreal.MaterialExpressionConstant, -300, 400)
        zero.r = 0.0
        mel.connect_material_property(
            zero, "", unreal.MaterialProperty.MP_METALLIC)

        mel.recompile_material(mat)
        _al.save_asset(PATH_TERRAIN_MAT)
        log("  M_Terrain_ChalkMud: compiled and saved.")

    except Exception as e:
        warn(f"  Material expression setup failed: {e}")
        warn("  Saving blank material — you can wire VertexColor.R manually.")
        _al.save_asset(PATH_TERRAIN_MAT)

    return _al.load_asset(PATH_TERRAIN_MAT)


# ===========================================================================
# PART 4 — BarrageDirector CDO
# ===========================================================================

def configure_barrage_director():
    """
    Wire shell classes into the BarrageDirector CDO and set the auto-start
    defaults. Individual level instances inherit these and can be overridden.
    """
    log("─── Configuring BP_BarrageDirector CDO ───")
    cdo = bp_cdo(PATH_DIRECTOR)
    if not cdo:
        warn("  BP_BarrageDirector CDO not available — check compilation.")
        return

    light = bp_class(PATH_SHELL_LIGHT)
    heavy = bp_class(PATH_SHELL_HEAVY)

    if light:
        try_set(cdo, "default_shell_class", light,  "BarrageDirector")
        log("  DefaultShellClass → BP_ArtilleryShell (75mm)")
    else:
        warn("  DefaultShellClass: BP_ArtilleryShell class not found.")

    if heavy:
        try_set(cdo, "heavy_shell_class", heavy, "BarrageDirector")
        log("  HeavyShellClass → BP_ArtilleryShell_Heavy (155mm)")

    try_set(cdo, "trench_center_location",       FIELD_CENTRE,  "BarrageDirector")
    try_set(cdo, "default_gun_origin",           GUN_ORIGINS[0],"BarrageDirector")
    try_set(cdo, "gun_origins",                  GUN_ORIGINS,   "BarrageDirector")
    try_set(cdo, "b_auto_start_on_play",         True,          "BarrageDirector")
    try_set(cdo, "auto_start_delay_sec",         8.0,           "BarrageDirector")
    try_set(cdo, "b_auto_wide_area_harassment",  True,          "BarrageDirector")
    try_set(cdo, "wide_area_radius_cm",          6000.0,        "BarrageDirector")
    try_set(cdo, "wide_area_shells_per_minute",  4.0,           "BarrageDirector")
    try_set(cdo, "b_loop_sequence",              True,          "BarrageDirector")

    _al.save_asset(PATH_DIRECTOR)
    log("  BP_BarrageDirector CDO: saved.")


# ===========================================================================
# PART 5 — DeformableTerrain CDO
# ===========================================================================

def configure_terrain(terrain_mat):
    log("─── Configuring BP_DeformableTerrain CDO ───")
    cdo = bp_cdo(PATH_TERRAIN)
    if not cdo:
        warn("  BP_DeformableTerrain CDO not available.")
        return

    try_set(cdo, "terrain_width",      FIELD_W + 1000.0, "Terrain")
    try_set(cdo, "terrain_depth",      FIELD_D + 1000.0, "Terrain")
    try_set(cdo, "grid_resolution_cm", 80.0,             "Terrain")

    if terrain_mat:
        try_set(cdo, "terrain_material", terrain_mat, "Terrain")
        log("  terrain_material: M_Terrain_ChalkMud assigned.")
    else:
        warn("  Terrain material not available — assign M_Terrain_ChalkMud manually.")

    _al.save_asset(PATH_TERRAIN)
    log("  BP_DeformableTerrain CDO: saved.")


# ===========================================================================
# PART 6 — Level generation
# ===========================================================================

def gen_lighting(world):
    log("  Placing lighting (overcast winter, Lumen)...")

    # Flat overcast directional: no harsh shadow cast, featureless grey-white
    # Historical: Verdun photographs are uniformly grey, no sun visible
    try:
        dl = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.load_class(None, "/Script/Engine.DirectionalLight"),
            unreal.Vector(0, 0, 8000), unreal.Rotator(-35, 15, 0))
        if dl:
            c = dl.get_component_by_class(unreal.DirectionalLightComponent)
            if c:
                c.set_editor_property("intensity", 2.5)
                c.set_editor_property(
                    "light_color", unreal.LinearColor(0.82, 0.87, 1.0, 1.0))
                c.set_editor_property("atmosphere_sun_light", True)
                c.set_editor_property("cast_shadows", True)
            dl.set_actor_label("DirLight_WinterOvercast")
    except Exception as e:
        warn(f"  Directional light: {e}")

    # Real-time sky light — Lumen updates as smoke fills the air
    try:
        sl = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.load_class(None, "/Script/Engine.SkyLight"),
            unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
        if sl:
            c = sl.get_component_by_class(unreal.SkyLightComponent)
            if c:
                c.set_editor_property("intensity", 0.4)
                c.set_editor_property(
                    "light_color", unreal.LinearColor(0.65, 0.70, 0.85, 1.0))
                c.set_editor_property("real_time_capture", True)
            sl.set_actor_label("SkyLight_RealtimeLumen")
    except Exception as e:
        warn(f"  Sky light: {e}")

    # Volumetric height fog — smoke columns from Niagara interact with this,
    # building visual density. Ochre-grey: cordite + chalk dust.
    try:
        fog = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.load_class(None, "/Script/Engine.ExponentialHeightFog"),
            unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
        if fog:
            c = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
            if c:
                c.set_editor_property("fog_density",          0.015)
                c.set_editor_property("fog_height_falloff",   0.15)
                c.set_editor_property("fog_inscattering_color",
                    unreal.LinearColor(0.65, 0.60, 0.50, 1.0))
                c.set_editor_property("start_distance",       0.0)
                c.set_editor_property("fog_cutoff_distance",  60000.0)
                c.set_editor_property("volumetric_fog",       True)
                c.set_editor_property(
                    "volumetric_fog_scattering_distribution", 0.15)
                c.set_editor_property("volumetric_fog_albedo",
                    unreal.LinearColor(0.75, 0.70, 0.60, 1.0))
                c.set_editor_property("volumetric_fog_extinction_scale", 1.2)
            fog.set_actor_label("HeightFog_BattlefieldSmoke")
    except Exception as e:
        warn(f"  Height fog: {e}")


def gen_terrain(world):
    log("  Placing deformable terrain...")
    actor = spawn_actor(PATH_TERRAIN,
                        unreal.Vector(-500.0, -500.0, -20.0))
    if actor:
        actor.set_actor_label("DeformableTerrain")
        # Instance-level overrides (belt-and-suspenders over CDO defaults)
        try_set(actor, "terrain_width",      FIELD_W + 1000.0, "Terrain instance")
        try_set(actor, "terrain_depth",      FIELD_D + 1000.0, "Terrain instance")
        try_set(actor, "grid_resolution_cm", 80.0,             "Terrain instance")
    return actor


def gen_trench(world):
    """
    L-shaped fire trench along the east edge of the field.
    Player stands here looking west across the open impact area.
    """
    log("  Placing trench segments...")

    trench_cls = bp_class(PATH_TRENCH)
    SEG = 300.0   # 3m per segment
    cy  = FIELD_D * 0.5
    placed = 0

    def seg(x, y, yaw):
        nonlocal placed
        loc = unreal.Vector(x, y, 0)
        rot = unreal.Rotator(0, yaw, 0)
        if trench_cls:
            a = unreal.EditorLevelLibrary.spawn_actor_from_class(
                trench_cls, loc, rot)
        else:
            a = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.StaticMeshActor, loc, rot)
        if a:
            a.set_actor_label(f"Trench_{placed:03d}")
            placed += 1

    # Main run: 8 segments N-S
    for i in range(8):
        seg(TRENCH_X, cy - 4*SEG + i*SEG, 0)

    # Forward observer bays: two bays pointing west
    for i in range(2):
        seg(TRENCH_X - (i+1)*SEG, cy - SEG, 90)   # bay south
        seg(TRENCH_X - (i+1)*SEG, cy + SEG, 90)   # bay north

    # Rear communication trench (escape route east)
    for i in range(3):
        seg(TRENCH_X + (i+1)*SEG, cy, 90)

    log(f"  {placed} trench segments placed.")


def gen_director(world):
    """
    Place the BarrageDirector and override per-instance properties
    (gun origins, target, auto-start) so they're visible in the Details panel.
    """
    log("  Placing BarrageDirector...")

    # Place at the primary gun position off the west edge
    actor = spawn_actor(PATH_DIRECTOR,
                        unreal.Vector(-2500.0, FIELD_D * 0.5, 6000.0),
                        unreal.Rotator(0, 90, 0))
    if not actor:
        warn("  BP_BarrageDirector not placed — create it in the Editor.")
        return None

    actor.set_actor_label("BarrageDirector")

    # Per-instance property overrides
    try_set(actor, "trench_center_location",       FIELD_CENTRE,  "Director instance")
    try_set(actor, "gun_origins",                  GUN_ORIGINS,   "Director instance")
    try_set(actor, "default_gun_origin",           GUN_ORIGINS[0],"Director instance")
    try_set(actor, "b_auto_start_on_play",         True,          "Director instance")
    try_set(actor, "auto_start_delay_sec",         8.0,           "Director instance")
    try_set(actor, "b_auto_wide_area_harassment",  True,          "Director instance")
    try_set(actor, "wide_area_radius_cm",          6000.0,        "Director instance")
    try_set(actor, "wide_area_shells_per_minute",  4.0,           "Director instance")
    try_set(actor, "b_loop_sequence",              True,          "Director instance")

    # Wire shell classes on the instance (redundant with CDO but explicit)
    light = bp_class(PATH_SHELL_LIGHT)
    heavy = bp_class(PATH_SHELL_HEAVY)
    if light: try_set(actor, "default_shell_class", light, "Director instance")
    if heavy: try_set(actor, "heavy_shell_class",   heavy, "Director instance")

    log("  BarrageDirector placed and configured.")
    return actor


def gen_test_director(world):
    """Passive HUD readout — no auto sequences."""
    loc = unreal.Vector(TRENCH_X + 100, FIELD_D * 0.5, 100)
    td = spawn_actor(PATH_TEST_DIRECTOR, loc)
    if td:
        td.set_actor_label("TestDirector_HUD")
        try_set(td, "b_auto_run_comparison",               False, "TestDirector")
        try_set(td, "b_auto_start_verdun_after_comparison",False, "TestDirector")


def gen_player_start(world):
    """In the forward observer bay, facing west across the open field."""
    log("  Placing PlayerStart (in trench bay, facing west)...")
    ps = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(TRENCH_X - 300.0, FIELD_D * 0.5, 100.0),
        unreal.Rotator(0, 180, 0))   # Yaw 180 = facing -X (west)
    if ps:
        ps.set_actor_label("PlayerStart_TrenchBay")
    return ps


def gen_distance_markers(world):
    """
    Thin white stakes at 10, 20, 30, 50, 70, 100m west of the trench.
    No labels — just scale references for when craters form near them.
    They'll be partly buried by mid-session.
    """
    log("  Placing distance markers...")
    cyl = unreal.load_asset("/Engine/BasicShapes/Cylinder")
    dists = [10, 20, 30, 50, 70, 100]   # metres from trench
    rows  = [-2000.0, 0.0, 2000.0]      # three rows: south, centre, north

    placed = 0
    for d_m in dists:
        for dy in rows:
            x = TRENCH_X - d_m * 100.0
            y = FIELD_D * 0.5 + dy
            a = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.StaticMeshActor,
                unreal.Vector(x, y, 0),
                unreal.Rotator(0, 0, 0))
            if a:
                a.set_actor_label(f"Marker_{d_m}m")
                a.set_actor_scale3d(unreal.Vector(0.1, 0.1, 2.0))
                sm = a.get_component_by_class(unreal.StaticMeshComponent)
                if sm and cyl:
                    sm.set_static_mesh(cyl)
                placed += 1

    log(f"  {placed} distance markers placed.")


def generate_level():
    log("─── Generating observer level ───")

    world = unreal.UnrealEditorSubsystem.get_editor_world()
    if not world:
        warn("  Cannot get world — is an Editor level open?")
        return

    with unreal.ScopedEditorTransaction("Generate Observer Level") as _:
        gen_lighting(world)
        gen_terrain(world)
        gen_trench(world)
        gen_director(world)
        gen_test_director(world)
        gen_player_start(world)
        gen_distance_markers(world)

        # World settings: gravity + game mode
        try:
            ws = world.get_world_settings()
            if ws:
                ws.set_editor_property("global_gravity_z", -980.0)
                gm = unreal.load_class(None, "/Script/OnlyTheDead.VerdunGameMode")
                if gm:
                    ws.set_editor_property("default_game_mode", gm)
        except Exception as e:
            warn(f"  World settings: {e}")

    unreal.EditorLevelLibrary.save_current_level()


# ===========================================================================
# MAIN
# ===========================================================================

def main():
    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  Only The Dead — Observer Level Setup")
    unreal.log("  Verdun, February 21, 1916")
    unreal.log("=" * 64)

    # -- Setup phase (Blueprints + material) --
    with unreal.ScopedEditorTransaction("Create Blueprint Assets") as _:
        create_blueprints()
        configure_shells()

    mat = create_terrain_material()

    with unreal.ScopedEditorTransaction("Configure CDOs") as _:
        configure_barrage_director()
        configure_terrain(mat)

    # -- Level generation --
    generate_level()

    unreal.log("")
    unreal.log("=" * 64)
    unreal.log("  SETUP COMPLETE")
    unreal.log("")
    unreal.log("  Press Play.")
    unreal.log("")
    unreal.log("  T+ 0s : Wide-area harassing fire starts — 4 shells/min")
    unreal.log("          across a 60m radius. Nowhere is permanently safe.")
    unreal.log("  T+ 8s : Single registration shot. Silence. Second shot.")
    unreal.log("  T+40s : Harassing fire — 8/min, unpredictable scatter.")
    unreal.log("  T+103s: Sustained barrage — 75mm + 155mm mixed. Ground")
    unreal.log("          vertex colors shift chalk→mud around each crater.")
    unreal.log("  T+198s: Saturation — 60 shells/min. Stand in the open")
    unreal.log("          and shell shock accumulates rapidly. Trench cover")
    unreal.log("          reduces the rate — step into a bay to recover.")
    unreal.log("  T+293s: DRUMFIRE. 120 shells/min. Individual impacts")
    unreal.log("          are indistinguishable. The field is mud.")
    unreal.log("  T+315s: Loop restart — Drumfire merges into next cycle.")
    unreal.log("")
    unreal.log("  WHAT TO LOOK AT:")
    unreal.log("  - Crater rims: raised Gaussian lip, each impact persistent")
    unreal.log("  - Overlapping craters: second hit into existing bowl")
    unreal.log("  - VertexColor.R shift: chalk pale → mud dark in real time")
    unreal.log("  - Lumen flash: face away — GI bounce illuminates your back")
    unreal.log("  - Shell shock: stand in open, return to trench to recover")
    unreal.log("  - Distance markers: 10m stakes disappearing into crater rims")
    unreal.log("=" * 64)
    unreal.log("")


main()
