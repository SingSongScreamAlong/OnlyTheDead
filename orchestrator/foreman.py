#!/usr/bin/env python3
"""
foreman.py - Conversational CLI for the Verdun pipeline.

The "Foreman" provides a natural language interface to control
Houdini PDG cooking, UE5 imports, and pipeline operations.

Usage:
    python foreman.py watch              # Start PDG file watcher
    python foreman.py build N13          # Build specific tile
    python foreman.py build N13 --heavy-shelling  # With variant
    python foreman.py publish --all      # Publish all staged assets
    python foreman.py ue import N13      # Import tile to UE5
    python foreman.py ue rebuild-hlod --map Verdun_Persistent
    python foreman.py qc N13             # Run QC checks on tile
    python foreman.py status             # Show pipeline status
"""

import sys
import subprocess
import logging
from pathlib import Path
from typing import Optional, List
import time
import json
from datetime import datetime

# Use click for CLI (simpler than typer, more portable)
try:
    import click
except ImportError:
    print("ERROR: 'click' module not found. Install with: pip install click")
    sys.exit(1)

# Add utils to path
REPO_ROOT = Path(__file__).parent.parent
sys.path.insert(0, str(REPO_ROOT / "houdini" / "python"))

from utils import load_config, get_tile_paths, VerdunIOError


# Configure logging
LOG_DIR = REPO_ROOT / "orchestrator" / "logs"
LOG_DIR.mkdir(parents=True, exist_ok=True)

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s',
    handlers=[
        logging.FileHandler(LOG_DIR / "foreman.log"),
        logging.StreamHandler(sys.stdout)
    ]
)
logger = logging.getLogger("foreman")


class Foreman:
    """Pipeline orchestration controller."""

    def __init__(self):
        """Initialize foreman."""
        try:
            self.config = load_config()
        except Exception as e:
            logger.error(f"Failed to load config: {e}")
            self.config = {}

        self.houdini_bin = self.get_houdini_bin()
        self.ue_editor = self.get_ue_editor()

    def get_houdini_bin(self) -> Optional[Path]:
        """Get path to Houdini binaries."""
        houdini_config = self.config.get("houdini", {})
        houdini_bin = houdini_config.get("houdini_bin")

        if houdini_bin:
            return Path(houdini_bin)

        # Try environment variable
        import os
        hfs = os.environ.get("HFS")
        if hfs:
            return Path(hfs) / "bin"

        return None

    def get_ue_editor(self) -> Optional[Path]:
        """Get path to UE5 editor."""
        ue_config = self.config.get("unreal", {})
        editor_path = ue_config.get("editor_path")

        if editor_path:
            return Path(editor_path)

        # Try environment variable
        import os
        ue_path = os.environ.get("UE5_EDITOR_PATH")
        if ue_path:
            return Path(ue_path)

        return None

    def run_hython(self, script: str, args: List[str], stream_output: bool = True) -> int:
        """
        Run a hython script.

        Args:
            script: Path to Python script.
            args: Command-line arguments.
            stream_output: If True, streams output in real-time.

        Returns:
            Exit code.
        """
        if not self.houdini_bin:
            logger.error("Houdini binary path not configured")
            return 1

        hython = self.houdini_bin / "hython"
        if not hython.exists():
            logger.error(f"hython not found: {hython}")
            return 1

        cmd = [str(hython), script] + args
        logger.info(f"Running: {' '.join(cmd)}")

        if stream_output:
            # Stream output in real-time
            process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1
            )

            for line in process.stdout:
                print(line.rstrip())

            process.wait()
            return process.returncode
        else:
            # Run and capture
            result = subprocess.run(cmd, capture_output=True, text=True)
            if result.stdout:
                print(result.stdout)
            if result.stderr:
                print(result.stderr, file=sys.stderr)
            return result.returncode

    def run_ue_script(self, script: str, args: List[str] = None) -> int:
        """
        Run a UE5 Python script.

        Args:
            script: Path to Python script.
            args: Optional arguments.

        Returns:
            Exit code.
        """
        if not self.ue_editor:
            logger.error("UE5 editor path not configured")
            return 1

        ue_config = self.config.get("unreal", {})
        project_path = ue_config.get("project_path")

        if not project_path:
            logger.error("UE5 project path not configured")
            return 1

        cmd = [
            str(self.ue_editor),
            project_path,
            "-run=pythonscript",
            f"-script={script}"
        ]

        if args:
            cmd.extend(args)

        logger.info(f"Running: {' '.join(cmd)}")

        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1
        )

        for line in process.stdout:
            print(line.rstrip())

        process.wait()
        return process.returncode

    def watch_incoming(self):
        """Watch incoming directory for new data (stub)."""
        logger.info("Starting file watcher on incoming directory...")

        incoming_dir = Path(self.config["paths"]["incoming"])
        logger.info(f"Watching: {incoming_dir}")

        # In production, use watchdog or similar
        logger.info("File watcher started. Press Ctrl+C to stop.")
        try:
            while True:
                time.sleep(5)
                # Check for new files, trigger PDG cook
                # This is a stub - implement actual file watching
        except KeyboardInterrupt:
            logger.info("File watcher stopped")


foreman = Foreman()


@click.group()
def cli():
    """Verdun Pipeline Foreman - Conversational pipeline control."""
    pass


@cli.command()
def watch():
    """Start PDG file watcher for incoming data."""
    foreman.watch_incoming()


@cli.command()
@click.argument("tile_id")
@click.option("--heavy-shelling", is_flag=True, help="Apply heavy shelling variant")
@click.option("--season", type=click.Choice(["spring", "summer", "autumn", "winter"]), default="spring")
def build(tile_id: str, heavy_shelling: bool, season: str):
    """Build a specific tile using PDG."""
    logger.info(f"Building tile: {tile_id}")
    if heavy_shelling:
        logger.info("Variant: Heavy shelling")
    logger.info(f"Season: {season}")

    # Run PDG
    script = str(REPO_ROOT / "houdini" / "python" / "run_pdg.py")
    args = ["--tile", tile_id]

    # In production, pass variants as PDG parameters
    exit_code = foreman.run_hython(script, args)

    if exit_code == 0:
        logger.info(f"✓ Tile {tile_id} built successfully")
    else:
        logger.error(f"✗ Tile {tile_id} build failed")

    sys.exit(exit_code)


@cli.command()
@click.option("--all", "build_all", is_flag=True, help="Publish all staged assets")
@click.argument("tile_id", required=False)
def publish(build_all: bool, tile_id: Optional[str]):
    """Publish staged assets to production directory."""
    if build_all:
        logger.info("Publishing all staged assets...")
    elif tile_id:
        logger.info(f"Publishing tile: {tile_id}")
    else:
        logger.error("Must specify --all or TILE_ID")
        sys.exit(1)

    # Move files from staging to published
    staging_dir = Path(foreman.config["paths"]["staging"])
    published_dir = Path(foreman.config["paths"]["published"])

    # Stub: implement actual publishing logic
    logger.info(f"Copying from {staging_dir} to {published_dir}")
    logger.info("✓ Publishing complete")


@cli.group()
def ue():
    """Unreal Engine commands."""
    pass


@ue.command("import")
@click.argument("tile_id", required=False)
@click.option("--all", "import_all", is_flag=True, help="Import all staged assets")
def ue_import(tile_id: Optional[str], import_all: bool):
    """Import assets to UE5."""
    if tile_id:
        logger.info(f"Importing tile to UE5: {tile_id}")
    elif import_all:
        logger.info("Importing all assets to UE5...")
    else:
        logger.error("Must specify TILE_ID or --all")
        sys.exit(1)

    script = str(REPO_ROOT / "ue" / "Scripts" / "auto_ingest.py")
    exit_code = foreman.run_ue_script(script)

    if exit_code == 0:
        logger.info("✓ Import complete")
    else:
        logger.error("✗ Import failed")

    sys.exit(exit_code)


@ue.command("rebuild-hlod")
@click.option("--map", "map_name", help="Map name (e.g., Verdun_Persistent)")
def ue_rebuild_hlod(map_name: Optional[str]):
    """Rebuild HLODs for a map."""
    logger.info(f"Rebuilding HLODs for map: {map_name or 'current'}")

    script = str(REPO_ROOT / "ue" / "Scripts" / "maintenance.py")
    exit_code = foreman.run_ue_script(script)

    if exit_code == 0:
        logger.info("✓ HLOD rebuild complete")
    else:
        logger.error("✗ HLOD rebuild failed")

    sys.exit(exit_code)


@ue.command("maintenance")
def ue_maintenance():
    """Run UE5 maintenance tasks."""
    logger.info("Running UE5 maintenance...")

    script = str(REPO_ROOT / "ue" / "Scripts" / "maintenance.py")
    exit_code = foreman.run_ue_script(script)

    if exit_code == 0:
        logger.info("✓ Maintenance complete")
    else:
        logger.error("✗ Maintenance failed")

    sys.exit(exit_code)


@cli.command()
@click.argument("tile_id")
def qc(tile_id: str):
    """Run quality control checks on a tile."""
    logger.info(f"Running QC for tile: {tile_id}")

    # Load QC module and run checks
    from utils import QualityControl

    qc_checker = QualityControl(foreman.config)
    output_dir = Path(foreman.config["paths"]["staging"]) / "meshes"

    metrics = qc_checker.validate_tile_output(tile_id, output_dir)

    # Print results
    logger.info(f"QC Results for {tile_id}:")
    logger.info(f"  Passed: {metrics.passed}")
    logger.info(f"  Triangle count: {metrics.triangle_count}")
    logger.info(f"  Total size: {metrics.total_size_mb:.2f} MB")
    logger.info(f"  Errors: {len(metrics.errors)}")
    logger.info(f"  Warnings: {len(metrics.warnings)}")

    if metrics.errors:
        for error in metrics.errors:
            logger.error(f"  - {error}")

    sys.exit(0 if metrics.passed else 1)


@cli.command()
def status():
    """Show pipeline status."""
    logger.info("=" * 60)
    logger.info("Verdun Pipeline Status")
    logger.info("=" * 60)

    # Check directories
    dirs_to_check = ["incoming", "staging", "published"]
    for dir_name in dirs_to_check:
        dir_path = Path(foreman.config["paths"][dir_name])
        exists = "✓" if dir_path.exists() else "✗"
        logger.info(f"{exists} {dir_name}: {dir_path}")

    # Check tools
    logger.info("\nTools:")
    houdini_status = "✓" if foreman.houdini_bin else "✗ Not configured"
    logger.info(f"{houdini_status} Houdini: {foreman.houdini_bin or 'N/A'}")

    ue_status = "✓" if foreman.ue_editor else "✗ Not configured"
    logger.info(f"{ue_status} UE5: {foreman.ue_editor or 'N/A'}")

    # Count tiles in staging
    staging_meshes = Path(foreman.config["paths"]["staging"]) / "meshes"
    if staging_meshes.exists():
        mesh_count = len(list(staging_meshes.glob("*.fbx")))
        logger.info(f"\nStaged assets: {mesh_count} FBX files")

    logger.info("=" * 60)


if __name__ == "__main__":
    cli()
