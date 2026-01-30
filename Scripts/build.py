import os
import sys
import argparse
import subprocess
import shutil
from pathlib import Path

def main():
    parser = argparse.ArgumentParser(description="Source67 Build System")
    parser.add_argument("--project", help="Path to the project directory to build", required=True)
    parser.add_argument("--output", help="Output directory for the standalone build", default="Build/Standalone")
    parser.add_argument("--clean", help="Clean build directory before building", action="store_true")

    args = parser.parse_args()
    
    root_dir = Path(__file__).parent.parent.resolve()
    os.chdir(root_dir) # Ensure we are in root

    project_path = Path(args.project).resolve()
    if not project_path.exists():
        print(f"Error: Project path {project_path} does not exist.")
        return 1

    manifest_path = project_path / "manifest.source"
    if not manifest_path.exists():
         print(f"Error: manifest.source not found in {project_path}. Not a valid project.")
         return 1

    # 0. Find CMake
    cmake_cmd = "cmake"
    try:
        # Check if cmake is in PATH
        subprocess.check_output([cmake_cmd, "--version"], stderr=subprocess.STDOUT)
    except (FileNotFoundError, subprocess.CalledProcessError):
        # Try common locations
        possible_paths = [
            Path("/Users/olik/Applications/CLion.app/Contents/bin/cmake/mac/aarch64/bin/cmake"),
            Path("/Applications/CLion.app/Contents/bin/cmake/mac/aarch64/bin/cmake"), # Fallback standard install
            Path("/opt/homebrew/bin/cmake"),
            Path("/usr/local/bin/cmake")
        ]
        
        found = False
        for p in possible_paths:
            if p.exists():
                print(f"System 'cmake' not found in PATH. Using found: {p}")
                cmake_cmd = str(p)
                found = True
                break
        
        if not found:
            print("Error: 'cmake' not found in PATH or common locations.")
            print("Please add cmake to your PATH or verify installation.")
            return 1
         
    # 1. Configure CMake
    build_dir = root_dir / "build_runtime"
    if args.clean and build_dir.exists():
        shutil.rmtree(build_dir)
        
    print(f"Configuring CMake in {build_dir}...")
    cmd = [cmake_cmd, "-B", str(build_dir), "-DBUILD_RUNTIME=ON", "-DCMAKE_BUILD_TYPE=Release"]
    
    try:
        subprocess.check_call(cmd)
    except subprocess.CalledProcessError:
        print("Error: CMake Configuration Failed.")
        return 1
    
    # 2. Build
    print("Building Project...")
    try:
        subprocess.check_call([cmake_cmd, "--build", str(build_dir), "--config", "Release"])
    except subprocess.CalledProcessError:
        print("Error: Build Failed.")
        return 1
    
    # 3. Package
    output_dir = Path(args.output).resolve()
    if output_dir.exists():
        print(f"Cleaning output directory {output_dir}...")
        shutil.rmtree(output_dir)
    output_dir.mkdir(parents=True)
    
    # Identify Executable Name
    exe_name = "Source67"
    if sys.platform == "win32":
        exe_name += ".exe"
    
    # Find built executable
    # Search recursively in build_dir for the executable
    # Prioritize 'Release' folder if exists
    src_exe = None
    
    potential_paths = [
        build_dir / "Release" / exe_name,
        build_dir / "Debug" / exe_name, # Fallback
        build_dir / exe_name
    ]
    
    for p in potential_paths:
        if p.exists():
            src_exe = p
            break
            
    if not src_exe:
        # Fallback: Find anything executable with that name
        found = list(build_dir.rglob(exe_name))
        if found:
            src_exe = found[0]
            
    if not src_exe or not src_exe.exists():
        print(f"Error: Could not find built executable '{exe_name}' in {build_dir}")
        return 1

    print(f"Copying Executable from {src_exe}...")
    shutil.copy2(src_exe, output_dir / exe_name)
    # Ensure executable permissions on Unix
    if sys.platform != "win32":
        os.chmod(output_dir / exe_name, 0o755)

    
    # Copy Engine Assets
    print("Copying Engine Assets...")
    source_assets = root_dir / "assets"
    if source_assets.exists():
        # Copy engine assets to output/assets
        # shutil.copytree requires destination to NOT exist (prior to Python 3.8). 
        # Source67 runs on modern systems, assuming 3.8+.
        # But to be safe and merge:
        dest_assets = output_dir / "assets"
        shutil.copytree(source_assets, dest_assets, dirs_exist_ok=True)
        
    # Copy Project Assets & Scripts
    print(f"Copying Project resources from {project_path}...")
    
    # We copy: manifest.source, assets, scripts
    shutil.copy2(manifest_path, output_dir / "manifest.source")
    
    for item in project_path.iterdir():
        if item.name == "manifest.source":
            continue
        if item.is_dir():
            # Merge folders (assets, scripts) into output/
            # Example: Project/assets/MyLevel.s67 -> Output/assets/MyLevel.s67
            dest_subdir = output_dir / item.name
            shutil.copytree(item, dest_subdir, dirs_exist_ok=True)
            
    # Copy Dynamic Libraries (Windows)
    if sys.platform == "win32":
         print("Copying DLLs...")
         for dll in build_dir.rglob("*.dll"):
             # Avoid copying DLLs from intermediate folders if possible, but grabbing all is safer
             # filtered by modification time? No.
             try:
                shutil.copy2(dll, output_dir)
             except Exception:
                 pass # Ignore errors if file busy or same

    print("--------------------------------------------------")
    print("Build SUCCESS!")
    print(f"Runtime Executable located at: {output_dir}")
    print("--------------------------------------------------")
    
    # Create a wrapper script for Mac (optional, but requested earlier context implies user likes automation)
    # The build is a raw binary. MacOS might expect an Bundle?
    # For now, raw binary is fine for dev.

    return 0

if __name__ == "__main__":
    sys.exit(main())
