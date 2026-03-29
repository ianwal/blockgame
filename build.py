import argparse
import subprocess
import shutil
from pathlib import Path
import math


def main():
    parser = argparse.ArgumentParser(description="Build and optionally run the game.")

    parser.add_argument(
        "arch",
        choices=["linux64", "win64"],
        help="Target architecture",
    )

    parser.add_argument(
        "--clean",
        action=argparse.BooleanOptionalAction,
        help="Clean the build directory completely before building",
    )

    parser.add_argument(
        "--run",
        action=argparse.BooleanOptionalAction,
        help="Run after building.",
    )

    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Verbose QEMU execution."
    )

    parser.add_argument(
        "-d",
        "--debug",
        action=argparse.BooleanOptionalAction,
        help="Wait for debug for the QEMU execution.",
    )

    parser.add_argument(
        "--build-type",
        choices=["Asan", "RelWithDebInfo", "Release", "Debug"],
        default="Asan",
        type=str,
        help="CMake build type.",
    )

    args = parser.parse_args()

    if args.arch == "win64" and args.build_type == "Asan":
        print("WARNING: Asan is not a valid build type for win64. Switching to Debug.")
        args.build_type = "Debug"

    build_target_map = {
        "linux64": "BUILD_TARGET_LINUX64",
        "win64": "BUILD_TARGET_WIN64",
    }
    assert args.arch in build_target_map.keys()

    cmake_target_map = {
        "linux64": "cmake/linux64.cmake",
        "win64": "cmake/mingw-w64-x86_64.cmake",
    }
    assert args.arch in cmake_target_map.keys()

    build_dir = Path(__file__).parent / f".build-{args.arch}"

    if args.clean:
        print("Cleaning build directory before building.")
        if build_dir.exists() and build_dir.is_dir():
            shutil.rmtree(build_dir)
        else:
            print("Build dir does not exist. Skipping clean.")

    if (build_dir / args.build_type).exists():
        print("Skipping CMake generation. Build dir exists.")
    else:
        print("Running CMake generation.")
        subprocess.run(
            [
                "cmake",
                f"-DCMAKE_TOOLCHAIN_FILE={cmake_target_map[args.arch]}",
                f"-D{build_target_map[args.arch]}=1",
                f"-DCMAKE_BUILD_TYPE={args.build_type}",
                "-S",
                ".",
                f"{build_dir}",
            ],
            check=True,
        )

    print("Building code.")
    subprocess.run(["cmake", "--build", f"{build_dir}", "--parallel"], check=True)

    # Print the filesize
    exe_path = build_dir / args.build_type / "sdl-min"
    size_bytes = exe_path.stat().st_size
    size_kb = math.ceil(size_bytes / 1024)
    print(f"EXE Size: {size_kb} kb")

    if args.run:
        qemu_cmd = [
            exe_path,
        ]

        print("Running...")
        subprocess.run(
            qemu_cmd,
            check=True,
        )


if __name__ == "__main__":
    main()
