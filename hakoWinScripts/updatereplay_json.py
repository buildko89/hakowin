import csv
import json
import argparse
from pathlib import Path

def sec_to_hms(sec: float) -> str:
    """秒(float) -> HH:MM:SS.mmm 形式に変換"""
    msec = int(round((sec - int(sec)) * 1000))
    total_sec = int(sec)
    h, s = divmod(total_sec, 3600)
    m, s = divmod(s, 60)
    return f"{h:02d}:{m:02d}:{s:02d}.{msec:03d}"

def main():
    parser = argparse.ArgumentParser(
        description="CSVのtimestamp(マイクロ秒)からreplay.jsonのrange_sec/range_timestampを更新する"
    )
    parser.add_argument("csv_path", help="CSVファイルへのパス（相対または絶対）")
    parser.add_argument("json_path", help="JSONファイルへのパス（相対または絶対）")
    args = parser.parse_args()

    # 相対パスでも絶対パスでもOK
    csv_path = Path(args.csv_path).resolve()
    json_path = Path(args.json_path).resolve()

    print(f"CSV : {csv_path}")
    print(f"JSON: {json_path}")

    # CSV からtimestamp(μ秒)を読み取り → 秒に変換
    timestamps = []
    with csv_path.open(encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            try:
                t_sec = float(row["timestamp"]) / 1_000_000.0
                timestamps.append(t_sec)
            except (KeyError, ValueError):
                pass

    if not timestamps:
        raise ValueError("CSVのtimestamp列が読み取れませんでした。")

    begin_val = min(timestamps)
    end_val   = max(timestamps)

    begin_str = sec_to_hms(begin_val)
    end_str   = sec_to_hms(end_val)

    # JSON 更新
    with json_path.open(encoding="utf-8") as f:
        data = json.load(f)

    if "range_sec" in data:
        data["range_sec"]["begin"] = round(begin_val, 3)
        data["range_sec"]["end"]   = round(end_val, 3)

    if "range_timestamp" in data:
        data["range_timestamp"]["begin"] = begin_str
        data["range_timestamp"]["end"]   = end_str

    with json_path.open("w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)

    print("更新完了")
    print(f"range_sec      : {begin_val:.3f} ～ {end_val:.3f}")
    print(f"range_timestamp: {begin_str} ～ {end_str}")

if __name__ == "__main__":
    main()
