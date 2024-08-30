# Script to format all cpp and h files.
# Requires clang-format to be installed.

from pathlib import Path
import subprocess
import glob
import logging

src_dir: Path = Path(__file__).parent / "src"

logging.basicConfig(level=logging.INFO)
log = logging.getLogger(__name__)


def get_src_header_files(target_dir: Path):
    files: list[Path] = []
    files.extend(glob.glob("*.cpp", root_dir=target_dir, recursive=True))
    files.extend(glob.glob("*.h", root_dir=target_dir, recursive=True))
    files = [target_dir / Path(file) for file in files]
    return files


def format_all_files(files: list[Path]):
    format_cmd = ["clang-format", "-i"]
    files_str = [str(file.absolute()) for file in files]
    format_cmd.extend(files_str)
    subprocess.run(
        format_cmd,
        check=True,
    )
    return len(files)


if __name__ == "__main__":
    num_files_formatted = format_all_files(get_src_header_files(src_dir))
    print(f"{num_files_formatted} files formatted.")
