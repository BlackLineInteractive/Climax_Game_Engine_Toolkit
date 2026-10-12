import os
import subprocess
import sys
import glob
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
PROJECT_DIR = Path.home() / "Library/ghidra/projects"
PROJECT_NAME = "SHO_R5900"
PROGRAM_NAME = "SLES_551.47"

GHIDRA_HEADLESS = Path("/usr/local/Cellar/ghidra/12.1.3/libexec/support/analyzeHeadless")
JAVA_HOME = Path("/usr/local/Cellar/openjdk@21/21.0.12.1/libexec/openjdk.jdk/Contents/Home")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 tools/decompile_func.py <address_or_name> [--all/--recursive] [--clean]")
        sys.exit(1)

    addr_or_name = sys.argv[1]
    recursive = "--all" in sys.argv or "--recursive" in sys.argv
    clean = "--clean" in sys.argv

    # Resolve address from known_names if needed
    known_names_path = REPO_ROOT / "docs/generated/known_names.txt"
    addr = addr_or_name
    func_name = addr_or_name

    if not addr.startswith("0x") and not all(c in "0123456789abcdefABCDEF" for c in addr):
        # Name lookup in known_names
        found = False
        if known_names_path.exists():
            for line in known_names_path.read_text().splitlines():
                parts = line.strip().split(maxsplit=1)
                if len(parts) == 2 and parts[1].lower() == addr_or_name.lower():
                    addr = parts[0]
                    func_name = parts[1]
                    found = True
                    break
        if not found:
            print(f"[-] Could not resolve '{addr_or_name}' to an address.")
            sys.exit(1)

    if not addr.startswith("0x"):
        addr = f"0x{addr}"

    tmp_dir = Path("/tmp/ghidra_sho")
    tmp_dir.mkdir(parents=True, exist_ok=True)
    out_dir = tmp_dir / "out"
    if out_dir.exists():
        for f in out_dir.glob("*.c"):
            f.unlink()
    else:
        out_dir.mkdir(parents=True, exist_ok=True)

    seed_file = tmp_dir / "seed.txt"
    seed_file.write_text(f"{addr} {func_name}\n")

    script_name = "GhidraDecompileAll.java" if recursive else "GhidraDecompile.java"
    script_args = [str(seed_file), str(out_dir)]
    if recursive and known_names_path.exists():
        script_args.append(str(known_names_path))

    env = os.environ.copy()
    if JAVA_HOME.exists():
        env["JAVA_HOME"] = str(JAVA_HOME)
        env["PATH"] = f"{JAVA_HOME}/bin:{env.get('PATH', '')}"

    cmd = [
        str(GHIDRA_HEADLESS),
        str(PROJECT_DIR),
        PROJECT_NAME,
        "-process", PROGRAM_NAME,
        "-noanalysis",
        "-readOnly",
        "-scriptPath", str(REPO_ROOT / "tools"),
        "-postScript", script_name,
        *script_args
    ]

    res = subprocess.run(cmd, env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if res.returncode != 0:
        print(f"[-] Ghidra exited with code {res.returncode}")

    c_files = sorted(out_dir.glob("*.c"))
    if not c_files:
        print(f"[-] No decompiled output found for {addr} ({func_name})")
        sys.exit(1)

    if clean:
        try:
            from sho_clean_decomp import (
                load_known_names, load_class_registry, load_attr_map,
                make_replacer, TYPE_RE, TYPE_MAP
            )
            names = {}
            if (REPO_ROOT / "docs/generated/known_names.txt").exists():
                names.update(load_known_names(str(REPO_ROOT / "docs/generated/known_names.txt")))
            if (REPO_ROOT / "docs/generated/sho_class_registry.json").exists():
                names.update(load_class_registry(str(REPO_ROOT / "docs/generated/sho_class_registry.json")))
            if (REPO_ROOT / "docs/generated/sho_attribute_map.json").exists():
                names.update(load_attr_map(str(REPO_ROOT / "docs/generated/sho_attribute_map.json")))

            replacer = make_replacer(names)
            for f in c_files:
                text = f.read_text(errors='replace')
                text = TYPE_RE.sub(lambda m: TYPE_MAP.get(m.group(1), m.group(1)), text)
                text = replacer(text)
                f.write_text(text)
        except Exception as e:
            print(f"[!] Warning: clean decomp failed: {e}")

    for f in c_files:
        content = f.read_text()
        print(f"// === {f.name} ===")
        print(content)

if __name__ == "__main__":
    main()


