import argparse
from pathlib import Path

def create_zero_file(path: Path, size_mb: int):
    """指定パスに size_mb MB のゼロファイルを作成する"""
    path.parent.mkdir(parents=True, exist_ok=True)
    size_bytes = size_mb * 1024 * 1024
    with open(path, "wb") as f:
        f.truncate(size_bytes)
    print(f"[OK] {path} を {size_mb} MB で作成しました。")

def main():
    parser = argparse.ArgumentParser(description="ゼロファイル作成 (truncate版)")
    parser.add_argument("output", help="出力ファイルパス (例: Z:\\mmap\\mmap-0x100.bin)")
    parser.add_argument("size", type=int, help="作成するファイルサイズ(MB)")
    args = parser.parse_args()

    create_zero_file(Path(args.output), args.size)

if __name__ == "__main__":
    main()
