"""
Download large scene assets that are excluded from git.

Usage:
    python scripts/download_assets.py           # download all
    python scripts/download_assets.py sponza    # download sponza only
    python scripts/download_assets.py san_miguel
    python scripts/download_assets.py emerald_square
    python scripts/download_assets.py showcase
"""

import argparse
import base64
import concurrent.futures
import io
import json
import math
import os
import struct
import sys
import urllib.request
import zipfile

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

ASSETS = {
    "sponza": {
        "url": "https://cdrdv2.intel.com/v1/dl/getContent/830833",
        "dest": os.path.join(REPO_ROOT, "res", "sponza"),
        "description": "Intel Sponza Scene (glTF)",
    },
    "emerald_square": {
        "url": "https://developer.nvidia.com/emerald-square",
        "dest": os.path.join(REPO_ROOT, "res", "emerald_square"),
        "description": "NVIDIA ORCA Emerald Square City Scene (FBX)",
    },
    "san_miguel": {
        "url": "https://casual-effects.com/g3d/data10/research/model/San_Miguel/San_Miguel.zip",
        "dest": os.path.join(REPO_ROOT, "res", "san_miguel"),
        "description": "San Miguel Scene",
    },
    # Assembled from individual CC0 assets on polyhaven.com instead of a single archive, see download_showcase().
    # res/scenes/pbr_showcase.das places these assets, keep the two in sync.
    "showcase": {
        "dest": os.path.join(REPO_ROOT, "res", "showcase"),
        "description": "PBR showcase props and materials (Poly Haven, CC0)",
        "resolution": "2k",
        "models": [
            "wooden_table_02",
            "marble_bust_01",
            "brass_vase_01",
            "Camera_01",
            "Lantern_01",
            "ArmChair_01",
            "Barrel_01",
            "wine_barrel_01",
            "treasure_chest",
            "street_lamp_01",
        ],
        # Each material is wrapped onto a generated sphere, so it can be placed in a scene as a model.
        "sphere_materials": [
            "rusty_metal_02",
            "marble_01",
            "oak_veneer_01",
            "blue_metal_plate",
            "brown_leather",
            "castle_brick_02_red",
        ],
        "ground_material": "cobblestone_floor_08",
    },
}

POLYHAVEN_API = "https://api.polyhaven.com"


def _progress_hook(label):
    def hook(block_num, block_size, total_size):
        if total_size <= 0:
            downloaded = block_num * block_size
            sys.stdout.write(f"\r{label}: {downloaded // (1024 * 1024)} MB downloaded")
        else:
            downloaded = min(block_num * block_size, total_size)
            pct = downloaded * 100 // total_size
            bar = "#" * (pct // 2) + "-" * (50 - pct // 2)
            sys.stdout.write(f"\r{label}: [{bar}] {pct}%")
        sys.stdout.flush()
    return hook


def _fetch(url, path):
    """Downloads url to path. Files that already exist are kept, so an interrupted download can be resumed."""
    if os.path.exists(path):
        return
    os.makedirs(os.path.dirname(path), exist_ok=True)
    # Write to a temporary name first, a partial file must never be mistaken for a finished one.
    partial = path + ".part"
    with urllib.request.urlopen(url) as response, open(partial, "wb") as out:
        out.write(response.read())
    os.replace(partial, path)


def _polyhaven_files(asset_id):
    with urllib.request.urlopen(f"{POLYHAVEN_API}/files/{asset_id}") as response:
        return json.load(response)


def _polyhaven_model_downloads(asset_id, resolution, dest):
    """Returns (url, path) pairs for the glTF of a Poly Haven model and everything it references."""
    gltf = _polyhaven_files(asset_id)["gltf"][resolution]["gltf"]
    model_dir = os.path.join(dest, "models", asset_id)
    downloads = [(gltf["url"], os.path.join(model_dir, f"{asset_id}.gltf"))]
    for relative_path, entry in gltf["include"].items():
        downloads.append((entry["url"], os.path.join(model_dir, *relative_path.split("/"))))
    return downloads


# glTF texture role -> Poly Haven map name. "arm" packs ambient occlusion, roughness and metallic into R, G and B,
# which is the channel layout glTF expects for its occlusion and metallicRoughness textures.
MATERIAL_MAPS = {"baseColor": "Diffuse", "normal": "nor_gl", "occlusionRoughnessMetallic": "arm"}


def _polyhaven_material_downloads(asset_id, resolution, dest):
    """Returns ((url, path) pairs, {role: file name}) for the texture maps of a Poly Haven material."""
    files = _polyhaven_files(asset_id)
    material_dir = os.path.join(dest, "materials", asset_id)
    downloads = []
    textures = {}
    for role, map_name in MATERIAL_MAPS.items():
        url = files[map_name][resolution]["jpg"]["url"]
        file_name = url.rsplit("/", 1)[1]
        downloads.append((url, os.path.join(material_dir, file_name)))
        textures[role] = file_name
    return downloads, textures


def _sphere_geometry(radius, segments, rings, uv_repeat):
    """UV sphere centered on the origin. uv_repeat is how often the texture wraps around the equator."""
    positions, normals, uvs, indices = [], [], [], []
    for ring in range(rings + 1):
        v = ring / rings
        polar = v * math.pi
        for segment in range(segments + 1):
            u = segment / segments
            azimuth = u * 2.0 * math.pi
            normal = (math.sin(polar) * math.cos(azimuth), math.cos(polar), -math.sin(polar) * math.sin(azimuth))
            normals.append(normal)
            positions.append(tuple(radius * n for n in normal))
            # Half as many repeats from pole to pole as around the equator keeps the texels roughly square.
            uvs.append((u * uv_repeat, v * uv_repeat * 0.5))
    for ring in range(rings):
        for segment in range(segments):
            a = ring * (segments + 1) + segment
            b = a + segments + 1
            indices += [a, b, a + 1, a + 1, b, b + 1]
    return positions, normals, uvs, indices


def _plane_geometry(size, tile_size):
    """Square in the XZ plane facing up, centered on the origin. The texture repeats every tile_size units."""
    half = size * 0.5
    repeat = size / tile_size
    positions = [(-half, 0.0, -half), (half, 0.0, -half), (-half, 0.0, half), (half, 0.0, half)]
    normals = [(0.0, 1.0, 0.0)] * 4
    uvs = [(0.0, 0.0), (repeat, 0.0), (0.0, repeat), (repeat, repeat)]
    indices = [0, 2, 1, 1, 2, 3]
    return positions, normals, uvs, indices


def _write_gltf(path, name, geometry, textures):
    """Writes a single mesh with one textured PBR material as a self-contained .gltf next to its textures."""
    positions, normals, uvs, indices = geometry

    chunks = [
        b"".join(struct.pack("<3f", *p) for p in positions),
        b"".join(struct.pack("<3f", *n) for n in normals),
        b"".join(struct.pack("<2f", *uv) for uv in uvs),
        struct.pack(f"<{len(indices)}I", *indices),
    ]
    buffer_views = []
    offset = 0
    for i, chunk in enumerate(chunks):
        # 34962 = ARRAY_BUFFER (vertex attributes), 34963 = ELEMENT_ARRAY_BUFFER (indices)
        buffer_views.append(
            {"buffer": 0, "byteOffset": offset, "byteLength": len(chunk), "target": 34963 if i == 3 else 34962}
        )
        offset += len(chunk)
    data = b"".join(chunks)

    vertex_count = len(positions)
    gltf = {
        "asset": {"version": "2.0", "generator": "Dodo scripts/download_assets.py"},
        "scene": 0,
        "scenes": [{"nodes": [0]}],
        "nodes": [{"mesh": 0, "name": name}],
        "meshes": [
            {
                "name": name,
                "primitives": [
                    {"attributes": {"POSITION": 0, "NORMAL": 1, "TEXCOORD_0": 2}, "indices": 3, "material": 0}
                ],
            }
        ],
        "materials": [
            {
                "name": name,
                "pbrMetallicRoughness": {
                    "baseColorTexture": {"index": 0},
                    "metallicRoughnessTexture": {"index": 2},
                },
                "normalTexture": {"index": 1},
                "occlusionTexture": {"index": 2},
            }
        ],
        "textures": [{"source": 0, "sampler": 0}, {"source": 1, "sampler": 0}, {"source": 2, "sampler": 0}],
        "images": [
            {"uri": textures["baseColor"]},
            {"uri": textures["normal"]},
            {"uri": textures["occlusionRoughnessMetallic"]},
        ],
        # Linear filtering with mipmaps, repeating in both directions
        "samplers": [{"magFilter": 9729, "minFilter": 9987, "wrapS": 10497, "wrapT": 10497}],
        # 5126 = FLOAT, 5125 = UNSIGNED_INT
        "accessors": [
            {
                "bufferView": 0,
                "componentType": 5126,
                "count": vertex_count,
                "type": "VEC3",
                "min": [min(p[i] for p in positions) for i in range(3)],
                "max": [max(p[i] for p in positions) for i in range(3)],
            },
            {"bufferView": 1, "componentType": 5126, "count": vertex_count, "type": "VEC3"},
            {"bufferView": 2, "componentType": 5126, "count": vertex_count, "type": "VEC2"},
            {"bufferView": 3, "componentType": 5125, "count": len(indices), "type": "SCALAR"},
        ],
        "bufferViews": buffer_views,
        "buffers": [
            {
                "byteLength": len(data),
                "uri": "data:application/octet-stream;base64," + base64.b64encode(data).decode("ascii"),
            }
        ],
    }
    with open(path, "w") as out:
        json.dump(gltf, out, indent=2)


def download_showcase(name):
    asset = ASSETS[name]
    dest = asset["dest"]
    resolution = asset["resolution"]

    print(f"[{name}] {asset['description']}")
    print(f"[{name}] Destination: {dest}")

    # (material id, generated model file, geometry)
    generated = [(m, "sphere.gltf", _sphere_geometry(0.3, 64, 32, 2.0)) for m in asset["sphere_materials"]]
    generated.append((asset["ground_material"], "ground.gltf", _plane_geometry(24.0, 2.0)))

    print(f"[{name}] Querying Poly Haven ...")
    downloads = []
    for model in asset["models"]:
        downloads += _polyhaven_model_downloads(model, resolution, dest)
    material_textures = {}
    for material, _, _ in generated:
        files, material_textures[material] = _polyhaven_material_downloads(material, resolution, dest)
        downloads += files

    done = 0
    with concurrent.futures.ThreadPoolExecutor(max_workers=8) as pool:
        for _ in pool.map(lambda download: _fetch(*download), downloads):
            done += 1
            sys.stdout.write(f"\r[{name}] Downloading {done}/{len(downloads)} ...")
            sys.stdout.flush()
    print()

    for material, file_name, geometry in generated:
        path = os.path.join(dest, "materials", material, file_name)
        _write_gltf(path, material, geometry, material_textures[material])

    with open(os.path.join(dest, "LICENSE.txt"), "w") as out:
        out.write("The assets in this directory were downloaded from https://polyhaven.com\n")
        out.write("and are licensed under CC0 1.0 (https://creativecommons.org/publicdomain/zero/1.0/).\n")

    print(f"[{name}] Done.")


def download(name):
    asset = ASSETS[name]
    if "url" not in asset:
        download_showcase(name)
        return

    url = asset["url"]
    dest = asset["dest"]

    if url is None:
        print(f"[{name}] No URL configured, skipping.")
        return

    print(f"[{name}] {asset['description']}")
    print(f"[{name}] Destination: {dest}")

    os.makedirs(dest, exist_ok=True)

    print(f"[{name}] Downloading from {url} ...")
    zip_path, _ = urllib.request.urlretrieve(url, reporthook=_progress_hook(name))
    print()  # newline after progress bar

    print(f"[{name}] Extracting ...")
    with zipfile.ZipFile(zip_path, "r") as zf:
        members = zf.namelist()
        # Strip common top-level directory if the zip has one
        prefix = ""
        if members and all(m.startswith(members[0].split("/")[0] + "/") for m in members if "/" in m):
            prefix = members[0].split("/")[0] + "/"

        for i, member in enumerate(members):
            sys.stdout.write(f"\r[{name}] Extracting {i + 1}/{len(members)} ...")
            sys.stdout.flush()
            if prefix and member.startswith(prefix):
                target_name = member[len(prefix):]
            else:
                target_name = member
            if not target_name:
                continue
            target_path = os.path.join(dest, target_name)
            if member.endswith("/"):
                os.makedirs(target_path, exist_ok=True)
            else:
                os.makedirs(os.path.dirname(target_path), exist_ok=True)
                with zf.open(member) as src, open(target_path, "wb") as dst:
                    dst.write(src.read())

    print(f"\n[{name}] Done.")


def main():
    # Some hosts reject the default Python-urllib user agent, so identify as this script instead.
    opener = urllib.request.build_opener()
    opener.addheaders = [("User-Agent", "DodoAssetDownloader/1.0")]
    urllib.request.install_opener(opener)

    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("assets", nargs="*", choices=list(ASSETS.keys()) + [[]], help="Assets to download (default: all)")
    args = parser.parse_args()

    targets = args.assets if args.assets else list(ASSETS.keys())
    for name in targets:
        download(name)


if __name__ == "__main__":
    main()
