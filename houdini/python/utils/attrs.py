"""
Houdini attribute utilities for the Verdun pipeline.
Defines standard attribute schemas for geometry interchange.
"""

import logging
from typing import Dict, List, Any, Optional
from enum import Enum

logger = logging.getLogger(__name__)


class AttributeType(Enum):
    """Houdini attribute types."""
    FLOAT = "float"
    INT = "int"
    STRING = "string"
    VECTOR = "vector"
    VECTOR4 = "vector4"


class AttributeClass(Enum):
    """Attribute storage class."""
    POINT = "point"
    VERTEX = "vertex"
    PRIMITIVE = "prim"
    DETAIL = "detail"


class VerdunAttributes:
    """
    Standard attribute schemas for Verdun pipeline assets.

    These attributes are used to pass data between HDAs and
    to the engine for material assignment, instancing, etc.
    """

    # Trench attributes
    TRENCH_ATTRS = {
        "trench_width": (AttributeType.FLOAT, AttributeClass.POINT, "Width of trench in meters"),
        "trench_depth": (AttributeType.FLOAT, AttributeClass.POINT, "Depth of trench in meters"),
        "revetment_type": (AttributeType.INT, AttributeClass.PRIMITIVE, "Revetment style index"),
        "has_duckboards": (AttributeType.INT, AttributeClass.PRIMITIVE, "1 if duckboards present"),
        "mudiness": (AttributeType.FLOAT, AttributeClass.POINT, "Mud level 0-1"),
        "side": (AttributeType.STRING, AttributeClass.PRIMITIVE, "french/german/neutral"),
    }

    # Crater/shell attributes
    CRATER_ATTRS = {
        "crater_radius": (AttributeType.FLOAT, AttributeClass.POINT, "Crater radius in meters"),
        "crater_depth": (AttributeType.FLOAT, AttributeClass.POINT, "Crater depth in meters"),
        "has_water": (AttributeType.INT, AttributeClass.POINT, "1 if water-filled"),
        "water_depth": (AttributeType.FLOAT, AttributeClass.POINT, "Water depth in meters"),
        "age_days": (AttributeType.FLOAT, AttributeClass.POINT, "Days since created"),
        "shell_type": (AttributeType.STRING, AttributeClass.POINT, "Type of shell/ordnance"),
    }

    # Debris scatter attributes
    DEBRIS_ATTRS = {
        "asset_path": (AttributeType.STRING, AttributeClass.POINT, "Path to debris asset"),
        "scale_variance": (AttributeType.FLOAT, AttributeClass.POINT, "Scale randomization 0-1"),
        "rot_variance": (AttributeType.VECTOR, AttributeClass.POINT, "Rotation variance XYZ"),
        "burial_depth": (AttributeType.FLOAT, AttributeClass.POINT, "How buried in terrain"),
        "material_override": (AttributeType.STRING, AttributeClass.POINT, "Override material name"),
    }

    # Building/ruin attributes
    BUILDING_ATTRS = {
        "damage_level": (AttributeType.FLOAT, AttributeClass.PRIMITIVE, "Damage 0=intact 1=destroyed"),
        "building_type": (AttributeType.STRING, AttributeClass.PRIMITIVE, "house/church/barn/etc"),
        "hit_direction": (AttributeType.VECTOR, AttributeClass.PRIMITIVE, "Primary damage vector"),
        "has_rubble": (AttributeType.INT, AttributeClass.PRIMITIVE, "1 if rubble field present"),
        "floor_count": (AttributeType.INT, AttributeClass.DETAIL, "Number of stories"),
    }

    # Landscape layer attributes
    LAYER_ATTRS = {
        "layer_mud": (AttributeType.FLOAT, AttributeClass.POINT, "Mud weight 0-1"),
        "layer_water": (AttributeType.FLOAT, AttributeClass.POINT, "Water weight 0-1"),
        "layer_wood": (AttributeType.FLOAT, AttributeClass.POINT, "Wood/duckboard weight 0-1"),
        "layer_metal": (AttributeType.FLOAT, AttributeClass.POINT, "Metal debris weight 0-1"),
        "layer_grass": (AttributeType.FLOAT, AttributeClass.POINT, "Grass weight 0-1"),
        "layer_blood": (AttributeType.FLOAT, AttributeClass.POINT, "Blood stain weight 0-1"),
    }


def create_attribute_dict(attr_schema: Dict[str, tuple]) -> List[Dict[str, Any]]:
    """
    Convert attribute schema to a list of dictionaries for documentation or code generation.

    Args:
        attr_schema: Schema dict from VerdunAttributes.

    Returns:
        List of attribute definitions.
    """
    attrs = []
    for name, (attr_type, attr_class, description) in attr_schema.items():
        attrs.append({
            "name": name,
            "type": attr_type.value,
            "class": attr_class.value,
            "description": description,
        })
    return attrs


def get_default_value(attr_type: AttributeType) -> Any:
    """
    Get default value for an attribute type.

    Args:
        attr_type: Attribute type.

    Returns:
        Default value.
    """
    defaults = {
        AttributeType.FLOAT: 0.0,
        AttributeType.INT: 0,
        AttributeType.STRING: "",
        AttributeType.VECTOR: (0.0, 0.0, 0.0),
        AttributeType.VECTOR4: (0.0, 0.0, 0.0, 0.0),
    }
    return defaults.get(attr_type, None)


# Houdini Python snippet for creating attributes
def houdini_create_attribute_snippet(attr_name: str, attr_type: AttributeType,
                                      attr_class: AttributeClass,
                                      default_value: Any = None) -> str:
    """
    Generate Houdini Python code snippet to create an attribute.

    Args:
        attr_name: Name of attribute.
        attr_type: Type of attribute.
        attr_class: Storage class.
        default_value: Default value.

    Returns:
        Python code string.
    """
    if default_value is None:
        default_value = get_default_value(attr_type)

    # Map to Houdini attribute types
    hou_type_map = {
        AttributeType.FLOAT: "hou.attribType.Float",
        AttributeType.INT: "hou.attribType.Int",
        AttributeType.STRING: "hou.attribType.String",
        AttributeType.VECTOR: "hou.attribType.Float",  # 3-tuple
        AttributeType.VECTOR4: "hou.attribType.Float",  # 4-tuple
    }

    hou_type = hou_type_map[attr_type]

    if attr_type in (AttributeType.VECTOR, AttributeType.VECTOR4):
        size = 3 if attr_type == AttributeType.VECTOR else 4
        snippet = f'geo.addAttrib(hou.attribType.{attr_class.value.title()}, "{attr_name}", {default_value}, size={size})'
    else:
        snippet = f'geo.addAttrib(hou.attribType.{attr_class.value.title()}, "{attr_name}", {repr(default_value)})'

    return snippet


def export_attribute_docs(output_path: Optional[str] = None) -> str:
    """
    Generate markdown documentation for all Verdun attributes.

    Args:
        output_path: If provided, saves to this file.

    Returns:
        Markdown string.
    """
    doc = "# Verdun Pipeline Attribute Reference\n\n"
    doc += "Standard attributes used across HDAs and exports.\n\n"

    schemas = {
        "Trench Attributes": VerdunAttributes.TRENCH_ATTRS,
        "Crater/Shell Attributes": VerdunAttributes.CRATER_ATTRS,
        "Debris Attributes": VerdunAttributes.DEBRIS_ATTRS,
        "Building Attributes": VerdunAttributes.BUILDING_ATTRS,
        "Landscape Layer Attributes": VerdunAttributes.LAYER_ATTRS,
    }

    for section_name, schema in schemas.items():
        doc += f"## {section_name}\n\n"
        doc += "| Attribute | Type | Class | Description |\n"
        doc += "|-----------|------|-------|-------------|\n"

        for name, (attr_type, attr_class, description) in schema.items():
            doc += f"| `{name}` | {attr_type.value} | {attr_class.value} | {description} |\n"

        doc += "\n"

    if output_path:
        with open(output_path, 'w') as f:
            f.write(doc)
        logger.info(f"Saved attribute documentation: {output_path}")

    return doc
