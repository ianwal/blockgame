import argparse
import subprocess
from pathlib import Path
import platform
import stat
import sys


def format_files(clang_format_bin: Path, files: list[Path]):
    for file in files:
        proc = subprocess.run(
            ["--files", file],
            executable=clang_format_bin,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
        for line in proc.stderr:
            print(line)
        if proc.returncode != 0:
            raise RuntimeError(f"Error running {clang_format_bin} on {f}")

        with open(file, "rb") as f:
            original_file = f.read()
        formatted_file = proc.stdout
        if original_file != formatted_file:
            # WARNING: w+b truncates the file on open, so if anything fails after opening the entire file will be truncated.
            with open(file, "w+b") as f:
                f.write(formatted_file)


def main():
    parser = argparse.ArgumentParser(description="Format C++ source code.")

    parser.add_argument(
        "--check",
        action=argparse.BooleanOptionalAction,
        help="Check what files need formatting. The return code of this program will be: 1 if files need formatting, 0 if no files need formatting",
    )

    args = parser.parse_args()

    root_dir = Path(__file__).parent

    clang_format_bin_dir = root_dir / ".bin"
    clang_format_bin = clang_format_bin_dir / "clang-format"

    if not clang_format_bin.exists():
        print("clang-format not found. Downloading clang-format...")
        clang_format_bin_dir.mkdir(exist_ok=True)
        # TODO: Add arm64 bin?
        if platform.machine() != "x86_64":
            raise RuntimeError("Program only currently supports x86_64.")
        subprocess.run(
            [
                "wget",
                "https://github.com/muttleyxd/clang-tools-static-binaries/releases/download/master-2da3e7b/clang-format-19_linux-amd64",
                "--no-verbose",
                "-O",
                clang_format_bin,
            ],
            timeout=60.0,
            check=True,
        )
        clang_format_bin.chmod(clang_format_bin.stat().st_mode | stat.S_IEXEC)
    else:
        assert clang_format_bin.is_file()

    files_that_need_formatting = []
    print("Scanning files...")
    dirs_to_format = [root_dir / dir_name for dir_name in ["src"]]
    input_files = []
    for dir in dirs_to_format:
        input_files.extend(
            [p for p in dir.rglob("*") if p.suffix in [".cpp", ".c", ".h", ".hpp"] and p.name not in "CImg.h"]
        )
    for file in input_files:
        proc = subprocess.run(
            ["--files", file],
            executable=clang_format_bin,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
        for line in proc.stderr:
            print(line)
        if proc.returncode != 0:
            raise RuntimeError(f"Error running {clang_format_bin} on {file}")

        with open(file, "rb") as f:
            original_file = f.read()
        formatted_file = proc.stdout
        if original_file != formatted_file:
            files_that_need_formatting.append(file)

    print(f"Scanned {len(input_files)} files.")

    if args.check:
        status = 0
        if len(files_that_need_formatting) > 0:
            print("Files that need formatting:")
            [print(f) for f in files_that_need_formatting]
            status = 1
        else:
            print("No files need formatting.")
    else:
        status = 0

        if len(files_that_need_formatting) > 0:
            print(f"Formatting {len(files_that_need_formatting)} files:")
            [print(f) for f in files_that_need_formatting]
            format_files(clang_format_bin, files_that_need_formatting)
            print("Done.")
        else:
            print("No files need formatting.")

    sys.exit(status)


if __name__ == "__main__":
    main()
