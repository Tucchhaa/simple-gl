"""
Blender script to export all mesh objects from scene to JSON format for simple-gl engine.

PREREQUISITES (Do these in Blender BEFORE running this script):
1. All objects must be MESH objects (no curves, lights, etc.)
2. Apply all transforms: Select all objects → Ctrl+A → "All Transforms"
3. Ensure all meshes have UV coordinates
4. Ensure objects are NOT parented to each other
5. Save your Blender file

USAGE:
Just paste this entire script into Blender's Scripting workspace and press Run Script (Alt+P).
The script will export ALL mesh objects in your scene to resources/maps/blender_export.json
"""

import bpy
import json
import math
import mathutils
from pathlib import Path

def extract_mesh_data(obj):
    """Extract mesh data from a Blender object.
    
    Returns vertex data in LOCAL space (mesh coordinates)
    and world transform (position, rotation, scale).
    """
    if obj.type != 'MESH':
        return None
    
    mesh = obj.data
    mesh.update()
    
    # Get world transform (position, rotation, scale)
    # This is the object's current transform in the scene
    world_matrix = obj.matrix_world
    position, rotation_quat, scale_vec = world_matrix.decompose()
    
    # Convert position: Blender(X, Y, Z) → OpenGL(X, Z, -Y)
    position_gl = [position.x, position.z, -position.y]
    
    # Convert rotation quaternion: Blender Z-up to OpenGL Y-up
    # Rotate quaternion by -90 degrees around X axis to convert coordinate system
    # mathutils is available in Blender's Python environment
    rot_x_neg90 = mathutils.Euler((math.radians(-90), 0, 0), 'XYZ').to_quaternion()
    rotation_quat_gl = rot_x_neg90 @ rotation_quat
    rotation_quat_gl.normalize()
    rotation = [rotation_quat_gl.w, rotation_quat_gl.x, rotation_quat_gl.y, rotation_quat_gl.z]
    
    # Convert scale: Blender(X, Y, Z) → OpenGL(X, Z, Y)
    scale = [scale_vec.x, scale_vec.z, scale_vec.y]
    
    # Extract mesh data: positions, UVs, normals
    # Vertices are in LOCAL space (mesh coordinates, not world space)
    vertices = []
    indices = []
    vertex_map = {}  # For deduplication
    
    # Get UV layer
    uv_layer = mesh.uv_layers.active
    has_uvs = uv_layer is not None
    
    # Process each face
    for face in mesh.polygons:
        face_verts = []
        
        for loop_idx in face.loop_indices:
            loop = mesh.loops[loop_idx]
            vertex_idx = loop.vertex_index
            
            # Position in LOCAL space (mesh coordinates)
            # Convert Blender (Z-up) to OpenGL (Y-up):
            # Blender(X, Y, Z) → OpenGL(X, Z, -Y)
            pos = mesh.vertices[vertex_idx].co
            pos_tuple = (pos.x, pos.z, -pos.y)
            
            # UV coordinates
            if has_uvs:
                uv = uv_layer.data[loop_idx].uv
                # Blender V coordinate needs to be flipped: V = 1 - V
                uv_tuple = (uv.x, 1.0 - uv.y)
            else:
                # Generate default UVs if missing
                uv_tuple = (0.0, 0.0)
            
            # Normal in LOCAL space
            # Convert normal: Blender(X, Y, Z) → OpenGL(X, Z, -Y)
            normal = loop.normal
            normal_tuple = (normal.x, normal.z, -normal.y)
            
            # Create vertex key for deduplication
            vertex_key = (pos_tuple, uv_tuple, normal_tuple)
            
            if vertex_key not in vertex_map:
                vertex_idx_new = len(vertices)
                vertex_map[vertex_key] = vertex_idx_new
                
                # Vertex format: pos(3) + uv(2) + normal(3) = 8 floats
                vertices.extend([
                    pos_tuple[0], pos_tuple[1], pos_tuple[2],  # position (converted)
                    uv_tuple[0], uv_tuple[1],  # UV (V flipped)
                    normal_tuple[0], normal_tuple[1], normal_tuple[2]  # normal (converted)
                ])
            else:
                vertex_idx_new = vertex_map[vertex_key]
            
            face_verts.append(vertex_idx_new)
        
        # Triangulate faces (quads and n-gons)
        # Note: Blender uses CCW winding, OpenGL with GL_CULL_FACE GL_BACK also expects CCW
        # But after coordinate conversion, we need to ensure correct winding
        if len(face_verts) == 3:
            # Triangle: keep CCW winding (0, 1, 2)
            indices.extend(face_verts)
        elif len(face_verts) == 4:
            # Quad: split into two triangles with correct CCW winding
            # First triangle: 0, 1, 2
            indices.extend([face_verts[0], face_verts[1], face_verts[2]])
            # Second triangle: 0, 2, 3 (maintain CCW order)
            indices.extend([face_verts[0], face_verts[2], face_verts[3]])
        else:
            # N-gon: fan triangulation with CCW winding
            for i in range(1, len(face_verts) - 1):
                indices.extend([face_verts[0], face_verts[i], face_verts[i + 1]])
    
    # Calculate bounding box for collision half-extents (in local space, after conversion)
    # Using converted coordinates: Blender(X, Y, Z) → OpenGL(X, Z, -Y)
    bbox_min = [float('inf'), float('inf'), float('inf')]
    bbox_max = [float('-inf'), float('-inf'), float('-inf')]
    
    for vertex in mesh.vertices:
        local_vertex = vertex.co
        # Convert to OpenGL coordinates
        x_gl = local_vertex.x
        y_gl = local_vertex.z
        z_gl = -local_vertex.y
        
        bbox_min[0] = min(bbox_min[0], x_gl)
        bbox_min[1] = min(bbox_min[1], y_gl)
        bbox_min[2] = min(bbox_min[2], z_gl)
        bbox_max[0] = max(bbox_max[0], x_gl)
        bbox_max[1] = max(bbox_max[1], y_gl)
        bbox_max[2] = max(bbox_max[2], z_gl)
    
    # Half-extents from center (local space, OpenGL coordinates)
    half_extents = [(bbox_max[i] - bbox_min[i]) / 2.0 for i in range(3)]
    
    # Scale half-extents by object's scale transform (already converted)
    half_extents[0] *= abs(scale[0])
    half_extents[1] *= abs(scale[1])
    half_extents[2] *= abs(scale[2])
    
    return {
        "name": obj.name,
        "vertices": vertices,
        "indices": indices,
        "position": position_gl,
        "rotation": rotation,
        "scale": scale,
        "colliderHalfExtents": half_extents
    }

# === MAIN EXECUTION (runs directly when pasted) ===
print("=" * 60)
print("Blender Map Export for simple-gl Engine")
print("=" * 60)
print("\nExporting all mesh objects from scene...\n")

# Get ALL objects in scene
all_objects = list(bpy.context.scene.objects)
mesh_objects = [obj for obj in all_objects if obj.type == 'MESH']
non_mesh_objects = [obj for obj in all_objects if obj.type != 'MESH']

# Warn about non-mesh objects
if non_mesh_objects:
    print(f"⚠️  Warning: Found {len(non_mesh_objects)} non-mesh object(s):")
    for obj in non_mesh_objects:
        print(f"     - '{obj.name}' (type: {obj.type})")
    print("   These will be skipped.\n")

if not mesh_objects:
    print("❌ ERROR: No mesh objects found in scene!")
    print("   Make sure you have mesh objects in your scene.")
else:
    print(f"✓ Found {len(mesh_objects)} mesh object(s) to export:\n")
    
    # Extract data from all mesh objects
    objects_data = []
    failed_count = 0
    
    for obj in mesh_objects:
        try:
            mesh_data = extract_mesh_data(obj)
            if mesh_data:
                objects_data.append(mesh_data)
                vertex_count = len(mesh_data['vertices']) // 8
                index_count = len(mesh_data['indices'])
                print(f"  ✓ {obj.name}: {vertex_count} vertices, {index_count} indices")
            else:
                print(f"  ✗ {obj.name}: Failed to extract mesh data")
                failed_count += 1
        except Exception as e:
            print(f"  ✗ {obj.name}: Error - {str(e)}")
            failed_count += 1
    
    if objects_data:
        # Determine export path
        blend_filepath = bpy.data.filepath
        if blend_filepath:
            # If Blender file is saved, try to find resources folder nearby
            blend_dir = Path(blend_filepath).parent
            # Look for resources/maps in common locations
            possible_paths = [
                blend_dir / "resources" / "maps",
                blend_dir.parent / "resources" / "maps",
                blend_dir.parent.parent / "resources" / "maps",
            ]
            
            export_path = None
            for path in possible_paths:
                if path.parent.exists():
                    export_path = path / "blender_export.json"
                    break
            
            if export_path is None:
                # Fallback: use resources folder next to Blender file
                export_path = blend_dir / "resources" / "maps" / "blender_export.json"
        else:
            # Blender file not saved - use hardcoded path
            export_path = Path.home() / "Documents" / "codes1" / "simple-gl" / "resources" / "maps" / "blender_export.json"
            print(f"\n⚠️  Note: Blender file not saved.")
            print(f"   Using default path. Save your .blend file for automatic path detection.\n")
        
        export_path.parent.mkdir(parents=True, exist_ok=True)
        
        # Create JSON structure
        json_data = {
            "version": 1,
            "objects": objects_data
        }
        
        # Write to file
        with open(export_path, 'w') as f:
            json.dump(json_data, f, indent=2)
        
        print("\n" + "=" * 60)
        print(f"✅ SUCCESS! Exported {len(objects_data)} objects")
        if failed_count > 0:
            print(f"⚠️  {failed_count} object(s) failed to export")
        print(f"📁 File saved to: {export_path}")
        print("=" * 60)
        print("\nNext steps:")
        print("  1. Load the map in your engine using MapLoader")
        print("  2. Make sure resources/maps/blender_export.json is accessible")
    else:
        print("\n❌ ERROR: No objects were successfully exported!")
        print("   Check that your meshes have valid geometry.")
