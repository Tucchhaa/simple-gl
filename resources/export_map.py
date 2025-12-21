import bpy
import json
import os
from mathutils import Matrix

# --- CONFIGURATION ---
# Set the path to your project's resource folder.
# The script will save the map file here.
# Example: "/home/user/projects/simple-gl/resources/maps/level.json"
PROJECT_RESOURCES_PATH = "/home/franciscobrizuela/Documents/codes1/simple-gl/resources"
MAP_FILENAME = "maps/blender_export.json"

# The name of the collection in Blender that contains your level geometry.
GEOMETRY_COLLECTION_NAME = "Geometry"
# --- END CONFIGURATION ---

# This matrix converts coordinates from Blender's system (X-right, Y-forward, Z-up)
# to our engine's system (X-right, Y-up, Z-forward).
# It swaps the Y and Z axes and inverts the new Z axis.
# (x, y, z)_blender -> (x, z, -y)_engine
CONVERSION_MATRIX = Matrix(
    (
        (1, 0, 0, 0),
        (0, 0, 1, 0),
        (0, -1, 0, 0),
        (0, 0, 0, 1),
    )
)
CONVERSION_MATRIX_INV = CONVERSION_MATRIX.inverted()

def transform_matrix_to_engine_space(blender_matrix: Matrix) -> Matrix:
    """Converts a Blender world matrix to the engine's coordinate system."""
    return CONVERSION_MATRIX @ blender_matrix @ CONVERSION_MATRIX_INV

def vec3_to_list(v):
    """Converts a 3D vector to a list of floats."""
    return [float(v.x), float(v.y), float(v.z)]

def quat_to_list(q):
    """Converts a Blender quaternion (W, X, Y, Z) to a list of floats."""
    return [float(q.w), float(q.x), float(q.y), float(q.z)]

def export_level():
    """Exports scene geometry from the specified collection to a JSON file."""
    
    # Ensure the output directory exists.
    out_path = os.path.join(PROJECT_RESOURCES_PATH, MAP_FILENAME)
    os.makedirs(os.path.dirname(out_path), exist_ok=True)

    # Find the geometry collection.
    collection = bpy.data.collections.get(GEOMETRY_COLLECTION_NAME)
    if not collection:
        raise RuntimeError(
            f"Error: Collection '{GEOMETRY_COLLECTION_NAME}' not found. "
            "Please create it and place your level geometry inside."
        )

    exported_objects = []
    print(f"Exporting objects from collection: '{GEOMETRY_COLLECTION_NAME}'")

    for obj in collection.objects:
        if obj.type != 'MESH':
            print(f"- Skipping non-mesh object: {obj.name}")
            continue

        print(f"+ Processing mesh object: {obj.name}")

        # Apply the coordinate system conversion.
        engine_matrix = transform_matrix_to_engine_space(obj.matrix_world)

        # Decompose the matrix into position, rotation, and scale.
        pos = engine_matrix.to_translation()
        rot = engine_matrix.to_quaternion()
        
        # Blender's `dimensions` property gives the world-space size of the
        # object's bounding box. We need to handle the coordinate swap for dimensions too.
        dims = obj.dimensions
        size = (dims.x, dims.z, dims.y)

        exported_objects.append({
            "name": obj.name,
            "type": "box",  # We'll assume all objects are boxes for now.
            "position": vec3_to_list(pos),
            "rotation": quat_to_list(rot),
            "size": [float(s) for s in size],
        })

    # Prepare the final JSON structure.
    output_data = {
        "version": 1,
        "settings": {
            "axis_system": "x_right_y_up_z_forward",
        },
        "objects": exported_objects,
    }

    # Write the JSON file.
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(output_data, f, indent=2)

    print(f"\nSuccessfully exported {len(exported_objects)} objects to {out_path}")

if __name__ == "__main__":
    export_level()
