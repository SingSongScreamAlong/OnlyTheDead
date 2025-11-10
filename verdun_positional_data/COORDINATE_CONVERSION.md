# Coordinate Conversion Technical Reference

## Overview

This document provides detailed technical information for converting GPS coordinates (WGS84 latitude/longitude) to Unreal Engine 5 world space coordinates.

## Coordinate Systems

### Source: WGS84 (World Geodetic System 1984)

**Description:** Standard GPS coordinate system used worldwide

**Format:** Decimal Degrees (DD)
- **Latitude**: -90° (South Pole) to +90° (North Pole)
- **Longitude**: -180° (West) to +180° (East)
- **Elevation**: Meters above mean sea level

**Verdun Area Bounds:**
- Latitude: 48.77°N to 49.28°N (~56 km N-S)
- Longitude: 5.16°E to 5.47°E (~22 km E-W)
- Elevation: 188m to 392m

**Example:**
```
Fort Douaumont:
  Latitude:  49.216868°N
  Longitude: 5.4383833°E
  Elevation: 392 meters
```

### Target: UE5 World Space

**Description:** 3D Cartesian coordinate system used by Unreal Engine

**Units:** Centimeters (cm)
- 1 UE5 unit = 1 centimeter
- 100 UE5 units = 1 meter
- 100,000 UE5 units = 1 kilometer

**Axes:**
- **X-axis**: Forward/North (positive = north)
- **Y-axis**: Right/East (positive = east)
- **Z-axis**: Up (positive = up)

**Origin:** User-defined (recommended: Verdun city center)
- Latitude: 49.1667°N
- Longitude: 5.3833°E
- Elevation: 200m

**Coordinate Limits:**
- UE5 can handle ±2,097,152 km from origin (21-bit precision)
- Our battlefield is only ~60 km × ~30 km, well within limits

## Conversion Mathematics

### The Problem

GPS coordinates are **spherical** (angles on a curved surface), while UE5 uses **Cartesian** coordinates (flat 3D space).

For small areas (~100 km), we can use a **local tangent plane approximation** - essentially treating the Earth's surface as flat within our battlefield area. This introduces minimal error (< 0.01%) for the Verdun battlefield scale.

### Conversion Constants

#### Meters per Degree Latitude
**Constant:** 111,320 meters per degree

This is nearly constant at all latitudes because lines of latitude are parallel.

**Formula:**
```
meters_per_degree_lat = (Earth's circumference) / 360°
                      = 40,075,017 m / 360°
                      ≈ 111,320 m/degree
```

More precisely (using WGS84 ellipsoid):
```
meters_per_degree_lat = 111132.92 - 559.82 × cos(2φ) + 1.175 × cos(4φ) - 0.0023 × cos(6φ)

Where φ = latitude in radians
At 49.1667°N: ≈ 111,229 meters/degree
```

For simplicity, we use 111,320 m/degree (error < 0.1%).

#### Meters per Degree Longitude
**Variable:** Depends on latitude (lines of longitude converge at poles)

**Formula:**
```
meters_per_degree_lon = 111,320 × cos(latitude)

At 49.1667°N:
  cos(49.1667°) ≈ 0.6527
  meters_per_degree_lon ≈ 111,320 × 0.6527 = 72,675 meters/degree
```

This is why longitude distances shrink as you move toward the poles.

### Conversion Formula (Step by Step)

**Input:**
- Location GPS: (Lat_L, Lon_L, Elev_L)
- Origin GPS: (Lat_O, Lon_O, Elev_O)

**Output:**
- UE5 World: (X, Y, Z) in centimeters

**Steps:**

1. **Calculate deltas (difference from origin)**
   ```
   ΔLat = Lat_L - Lat_O     (degrees)
   ΔLon = Lon_L - Lon_O     (degrees)
   ΔElev = Elev_L - Elev_O  (meters)
   ```

2. **Convert deltas to meters**
   ```
   X_meters = ΔLat × 111,320
   Y_meters = ΔLon × 111,320 × cos(Lat_O)
   Z_meters = ΔElev
   ```

3. **Convert meters to centimeters**
   ```
   X_cm = X_meters × 100
   Y_cm = Y_meters × 100
   Z_cm = Z_meters × 100
   ```

4. **Final UE5 coordinates**
   ```
   X = ΔLat × 11,132,000 cm
   Y = ΔLon × 7,267,500 cm  (at 49.1667°N)
   Z = ΔElev × 100 cm
   ```

### Simplified Formula for Verdun

**Given origin at Verdun (49.1667°N, 5.3833°E, 200m):**

```cpp
// C++ version
float X = (Latitude - 49.1667f) * 11132000.0f;    // cm
float Y = (Longitude - 5.3833f) * 7267500.0f;     // cm
float Z = (Elevation - 200.0f) * 100.0f;          // cm
```

```python
# Python version
X = (latitude - 49.1667) * 11_132_000    # cm
Y = (longitude - 5.3833) * 7_267_500     # cm
Z = (elevation - 200) * 100              # cm
```

## Worked Examples

### Example 1: Fort Douaumont

**GPS Input:**
```
Latitude:  49.216868°N
Longitude: 5.4383833°E
Elevation: 392 meters
```

**Conversion:**
```
ΔLat = 49.216868 - 49.1667 = 0.050168°
ΔLon = 5.4383833 - 5.3833 = 0.0550833°
ΔElev = 392 - 200 = 192 meters

X = 0.050168 × 11,132,000 = 558,470 cm = 5,584.7 m ≈ 5.58 km North
Y = 0.0550833 × 7,267,500 = 400,260 cm = 4,002.6 m ≈ 4.00 km East
Z = 192 × 100 = 19,200 cm = 192 m Up
```

**UE5 World Coordinates:**
```
X = 558,470 cm (5.58 km north of Verdun)
Y = 400,260 cm (4.00 km east of Verdun)
Z = 19,200 cm (192 m above Verdun)
```

### Example 2: Fort Vaux

**GPS Input:**
```
Latitude:  49.20049°N
Longitude: 5.47016°E
Elevation: 320 meters
```

**Conversion:**
```
ΔLat = 49.20049 - 49.1667 = 0.03379°
ΔLon = 5.47016 - 5.3833 = 0.08686°
ΔElev = 320 - 200 = 120 meters

X = 0.03379 × 11,132,000 = 376,150 cm ≈ 3.76 km North
Y = 0.08686 × 7,267,500 = 631,190 cm ≈ 6.31 km East
Z = 120 × 100 = 12,000 cm = 120 m Up
```

**UE5 World Coordinates:**
```
X = 376,150 cm (3.76 km north of Verdun)
Y = 631,190 cm (6.31 km east of Verdun)
Z = 12,000 cm (120 m above Verdun)
```

**Distance Verification:**
- Fort Douaumont to Fort Vaux:
  ```
  ΔX = 376,150 - 558,470 = -182,320 cm
  ΔY = 631,190 - 400,260 = 230,930 cm
  ΔZ = 12,000 - 19,200 = -7,200 cm

  Distance = √(ΔX² + ΔY² + ΔZ²)
           = √(182,320² + 230,930² + 7,200²)
           = √(86,541,522,400)
           = 294,183 cm
           = 2,941.83 meters
           ≈ 2.94 km
  ```
  *Historically accurate! The forts are ~3 km apart.*

### Example 3: Fleury-devant-Douaumont

**GPS Input:**
```
Latitude:  49.198109°N
Longitude: 5.427765°E
Elevation: 305 meters
```

**Conversion:**
```
ΔLat = 49.198109 - 49.1667 = 0.031409°
ΔLon = 5.427765 - 5.3833 = 0.044465°
ΔElev = 305 - 200 = 105 meters

X = 0.031409 × 11,132,000 = 349,653 cm ≈ 3.50 km North
Y = 0.044465 × 7,267,500 = 323,167 cm ≈ 3.23 km East
Z = 105 × 100 = 10,500 cm = 105 m Up
```

**UE5 World Coordinates:**
```
X = 349,653 cm
Y = 323,167 cm
Z = 10,500 cm
```

## Error Analysis

### Sources of Error

1. **Earth Curvature**
   - Assuming flat plane introduces error
   - Error at 60 km: < 0.01%
   - Negligible for gameplay

2. **Meters per Degree Approximation**
   - Using constant 111,320 instead of precise formula
   - Error: < 0.1%

3. **Longitude Correction**
   - cos(latitude) varies slightly across battlefield
   - At 49°N vs 49.28°N: < 0.01% difference

4. **GPS Data Accuracy**
   - Historical village locations approximate
   - GPS of destroyed villages ±10-50 meters
   - Fort locations ±5 meters (surveyed)

### Total Expected Error

**Horizontal position:** ±10-50 meters (due to historical data uncertainty)
**Elevation:** ±5-10 meters
**Relative positions:** Highly accurate (< 0.1% error)

This is more than sufficient for video game purposes!

## Inverse Conversion (UE5 to GPS)

If you need to convert UE5 coordinates back to GPS:

**Input:** UE5 World (X, Y, Z) in centimeters
**Output:** GPS (Lat, Lon, Elev)

**Formula:**
```
Latitude  = (X / 11,132,000) + 49.1667
Longitude = (Y / 7,267,500) + 5.3833
Elevation = (Z / 100) + 200
```

**Example:** Convert (558,470, 400,260, 19,200) to GPS
```
Lat = (558,470 / 11,132,000) + 49.1667 = 0.05017 + 49.1667 = 49.2169°N
Lon = (400,260 / 7,267,500) + 5.3833 = 0.05508 + 5.3833 = 5.4384°E
Elev = (19,200 / 100) + 200 = 192 + 200 = 392 m
```
This gives us Fort Douaumont coordinates - perfect!

## Alternative: UTM Coordinates

For maximum precision, you can use **UTM (Universal Transverse Mercator)** coordinates instead of the simple formula above.

Verdun is in **UTM Zone 31U**:
- Zone: 31 (6° wide longitude zone)
- Band: U (8° wide latitude band)

**Origin:** (0, 0) at intersection of equator and zone's central meridian

This system uses meters and is designed to minimize distortion. However, it's more complex and not necessary for the Verdun battlefield scale.

## Code Implementation

### UE5 Blueprint Function

```
Function Name: ConvertGPSToWorld
Category: Coordinate Conversion

Inputs:
  - Latitude (Float)
  - Longitude (Float)
  - Elevation (Float)

Local Variables:
  - OriginLat = 49.1667 (Float)
  - OriginLon = 5.3833 (Float)
  - OriginElev = 200.0 (Float)

Calculation:
  1. DeltaLat = Latitude - OriginLat
  2. DeltaLon = Longitude - OriginLon
  3. DeltaElev = Elevation - OriginElev

  4. X = DeltaLat * 11132000.0
  5. Y = DeltaLon * 7267500.0
  6. Z = DeltaElev * 100.0

Output:
  - WorldLocation = Make Vector(X, Y, Z)
```

### C++ Function

```cpp
FVector ConvertGPSToWorld(float Latitude, float Longitude, float Elevation)
{
    // Origin coordinates (Verdun city center)
    const float OriginLat = 49.1667f;
    const float OriginLon = 5.3833f;
    const float OriginElev = 200.0f;

    // Conversion constants (cm per degree)
    const float CM_PER_DEGREE_LAT = 11132000.0f;
    const float CM_PER_DEGREE_LON = 7267500.0f;  // At 49.1667°N
    const float CM_PER_METER = 100.0f;

    // Calculate deltas
    float DeltaLat = Latitude - OriginLat;
    float DeltaLon = Longitude - OriginLon;
    float DeltaElev = Elevation - OriginElev;

    // Convert to UE5 world space (cm)
    float X = DeltaLat * CM_PER_DEGREE_LAT;
    float Y = DeltaLon * CM_PER_DEGREE_LON;
    float Z = DeltaElev * CM_PER_METER;

    return FVector(X, Y, Z);
}
```

### Python Script (for preprocessing)

```python
import math
import json

def convert_gps_to_ue5(lat, lon, elev, origin_lat=49.1667, origin_lon=5.3833, origin_elev=200):
    """
    Convert GPS coordinates to UE5 world space.

    Args:
        lat: Latitude in decimal degrees
        lon: Longitude in decimal degrees
        elev: Elevation in meters
        origin_lat: Origin latitude (default: Verdun)
        origin_lon: Origin longitude (default: Verdun)
        origin_elev: Origin elevation (default: Verdun)

    Returns:
        Tuple of (X, Y, Z) in centimeters
    """
    # Calculate deltas
    delta_lat = lat - origin_lat
    delta_lon = lon - origin_lon
    delta_elev = elev - origin_elev

    # Convert to centimeters
    x = delta_lat * 11_132_000  # North
    y = delta_lon * 7_267_500   # East (at ~49°N)
    z = delta_elev * 100        # Up

    return (x, y, z)

# Example usage
fort_douaumont_gps = (49.216868, 5.4383833, 392)
ue5_coords = convert_gps_to_ue5(*fort_douaumont_gps)
print(f"Fort Douaumont UE5: X={ue5_coords[0]:,.0f} Y={ue5_coords[1]:,.0f} Z={ue5_coords[2]:,.0f}")
```

## Validation Tools

### Distance Calculator

To verify your conversions, calculate distances between known points:

**Haversine Formula** (accurate for GPS):
```python
import math

def haversine_distance(lat1, lon1, lat2, lon2):
    """Calculate distance between two GPS points in meters."""
    R = 6371000  # Earth radius in meters

    phi1 = math.radians(lat1)
    phi2 = math.radians(lat2)
    delta_phi = math.radians(lat2 - lat1)
    delta_lambda = math.radians(lon2 - lon1)

    a = (math.sin(delta_phi / 2) ** 2 +
         math.cos(phi1) * math.cos(phi2) * math.sin(delta_lambda / 2) ** 2)
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))

    return R * c  # Distance in meters
```

**Euclidean Distance** (for UE5 coordinates):
```python
def ue5_distance(x1, y1, z1, x2, y2, z2):
    """Calculate distance between two UE5 points in meters."""
    dx = x2 - x1
    dy = y2 - y1
    dz = z2 - z1

    distance_cm = math.sqrt(dx**2 + dy**2 + dz**2)
    return distance_cm / 100  # Convert to meters
```

These should give very similar results if conversion is correct!

## Further Reading

- **WGS84 Specification:** NIMA TR8350.2
- **UE5 Coordinate Systems:** Unreal Engine Documentation
- **UTM Projection:** Wikipedia - Universal Transverse Mercator
- **Geodetic Datums:** [EPSG.org](http://epsg.org)

---

**Summary:** For the Verdun battlefield, our simplified conversion is accurate to within 0.1%, which is far better than the uncertainty in the historical data itself!
