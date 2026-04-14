"""
Download large scene assets that are excluded from git.

Usage:
    python scripts/download_assets.py           # download all
    python scripts/download_assets.py sponza    # download sponza only
    python scripts/download_assets.py san_miguel
"""

import argparse
import io
import os
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
    "san_miguel": {
        "url": None,  # TODO: add URL
        "dest": os.path.join(REPO_ROOT, "res", "san_miguel"),
        "description": "San Miguel Scene",
    },
}


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


def download(name):
    asset = ASSETS[name]
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
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("assets", nargs="*", choices=list(ASSETS.keys()) + [[]], help="Assets to download (default: all)")
    args = parser.parse_args()

    targets = args.assets if args.assets else list(ASSETS.keys())
    for name in targets:
        download(name)


if __name__ == "__main__":
    main()
