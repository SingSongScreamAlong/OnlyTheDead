"""
Verdun Pipeline Utilities
==========================

Common utilities for I/O, tile management, QC, and attribute handling.
"""

from .io import (
    load_config,
    save_job_ticket,
    load_job_ticket,
    validate_job_ticket,
    load_catalog_csv,
    save_catalog_csv,
    ensure_path_exists,
    get_tile_paths,
    VerdunIOError,
)

from .tiles import (
    TileBounds,
    TileGrid,
    create_tile_manifest,
)

from .qc import (
    QCMetrics,
    QualityControl,
    create_thumbnail_stub,
    check_file_integrity,
)

from .attrs import (
    AttributeType,
    AttributeClass,
    VerdunAttributes,
    create_attribute_dict,
    get_default_value,
    houdini_create_attribute_snippet,
    export_attribute_docs,
)

__all__ = [
    # I/O
    "load_config",
    "save_job_ticket",
    "load_job_ticket",
    "validate_job_ticket",
    "load_catalog_csv",
    "save_catalog_csv",
    "ensure_path_exists",
    "get_tile_paths",
    "VerdunIOError",
    # Tiles
    "TileBounds",
    "TileGrid",
    "create_tile_manifest",
    # QC
    "QCMetrics",
    "QualityControl",
    "create_thumbnail_stub",
    "check_file_integrity",
    # Attributes
    "AttributeType",
    "AttributeClass",
    "VerdunAttributes",
    "create_attribute_dict",
    "get_default_value",
    "houdini_create_attribute_snippet",
    "export_attribute_docs",
]
