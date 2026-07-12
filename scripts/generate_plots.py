#!/usr/bin/env python3

import argparse
import csv
from pathlib import Path
from collections import defaultdict
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker

ALGORITHM_STYLES = {
    "OptimizedLRU": {"color": "#4C9BE8", "marker": "o", "linestyle": "-",  "linewidth": 2.5, "zorder": 5},
    "SLRU":         {"color": "#9B59B6", "marker": "P", "linestyle": "-",  "linewidth": 2.5, "zorder": 6},
    "NaiveLRU":     {"color": "#F4A261", "marker": "s", "linestyle": "--", "linewidth": 2.0, "zorder": 4},
    "FIFO":         {"color": "#E76F51", "marker": "^", "linestyle": "-.", "linewidth": 2.0, "zorder": 3},
    "Belady":       {"color": "#2EC4B6", "marker": "D", "linestyle": ":",  "linewidth": 2.5, "zorder": 7},
}

SEQUENCE_LABELS = {
    "uniform_random": "Uniform Random Workload",
    "high_locality":  "High Temporal Locality Workload",
}

PLOT_DIR = Path(__file__).parent.parent / "results" / "plots"

def load_csv(path: Path) -> list[dict]:
    rows = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append({
                "Algorithm": row["Algorithm"],
                "Sequence":  row["Sequence"],
                "CacheSize": int(row["CacheSize"]),
                "HitRatio":  float(row["HitRatio"]),
                "TimeMs":    float(row["TimeMs"]),
            })
    return rows

def load_scalability_csv(path: Path) -> dict:
    data = defaultdict(lambda: {"x": [], "y": []})
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            algo = row["Algorithm"]
            data[algo]["x"].append(int(row["RequestCount"]))
            data[algo]["y"].append(float(row["TimeMs"]))
    return data

def group_data(rows: list[dict]) -> dict:
    data = defaultdict(lambda: defaultdict(list))
    for row in rows:
        data[row["Sequence"]][row["Algorithm"]].append(
            (row["CacheSize"], row["HitRatio"], row["TimeMs"])
        )
    for seq in data:
        for algo in data[seq]:
            data[seq][algo].sort(key=lambda t: t[0])
    return data

def _apply_style(ax: plt.Axes, title: str, xlabel: str, ylabel: str) -> None:
    ax.set_title(title, fontsize=14, fontweight="bold", pad=12)
    ax.set_xlabel(xlabel, fontsize=12)
    ax.set_ylabel(ylabel, fontsize=12)
    ax.grid(True, which="major", linestyle="--", alpha=0.4)
    ax.grid(True, which="minor", linestyle=":", alpha=0.2)
    ax.minorticks_on()
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.legend(fontsize=10, framealpha=0.85)

def plot_hit_rate(data: dict, out_dir: Path, dpi: int) -> None:
    for seq_name, algo_data in data.items():
        fig, ax = plt.subplots(figsize=(9, 5))
        for algo, style in ALGORITHM_STYLES.items():
            if algo not in algo_data: continue
            pts = algo_data[algo]
            ax.plot([p[0] for p in pts], [p[1] * 100 for p in pts], label=algo, **style)

        seq_label = SEQUENCE_LABELS.get(seq_name, seq_name)
        _apply_style(ax, f"Hit Rate vs. Cache Size\n{seq_label}", "Cache Size (k)", "Hit Rate (%)")
        ax.yaxis.set_major_formatter(mticker.PercentFormatter(xmax=100, decimals=0))
        ax.set_ylim(0, 105)

        out_path = out_dir / f"hit_rate_{seq_name}.png"
        fig.tight_layout()
        fig.savefig(out_path, dpi=dpi)
        plt.close(fig)
        print(f"  Saved: {out_path.name}")

def plot_execution_time(data: dict, out_dir: Path, dpi: int) -> None:
    for seq_name, algo_data in data.items():
        fig, ax = plt.subplots(figsize=(9, 5))
        for algo, style in ALGORITHM_STYLES.items():
            if algo not in algo_data: continue
            pts = algo_data[algo]
            ax.plot([p[0] for p in pts], [p[2] for p in pts], label=algo, **style)

        seq_label = SEQUENCE_LABELS.get(seq_name, seq_name)
        _apply_style(ax, f"Execution Time vs. Cache Size\n{seq_label}", "Cache Size (k)", "Execution Time (ms)")

        out_path = out_dir / f"exec_time_{seq_name}.png"
        fig.tight_layout()
        fig.savefig(out_path, dpi=dpi)
        plt.close(fig)
        print(f"  Saved: {out_path.name}")

def plot_scalability(scalability_path: Path, out_dir: Path, dpi: int) -> None:
    if not scalability_path.exists():
        print("  Skipping scalability plot (run cache_sim first)")
        return

    data = load_scalability_csv(scalability_path)
    fig, ax = plt.subplots(figsize=(10, 5))

    for algo, style in ALGORITHM_STYLES.items():
        if algo not in data: continue
        xs = data[algo]["x"]
        ys = data[algo]["y"]
        ax.plot(xs, ys, label=algo, **style)

    _apply_style(ax,
        "Execution Time vs. Request Count (cache size k=16)\n"
        "Proves O(1) LRU vs. growing Belady overhead",
        "Number of Requests",
        "Execution Time (ms)"
    )
    ax.set_xscale("log")

    out_path = out_dir / "scalability.png"
    fig.tight_layout()
    fig.savefig(out_path, dpi=dpi)
    plt.close(fig)
    print(f"  Saved: {out_path.name}")

def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--results-dir", type=Path, default=Path(__file__).parent.parent / "results")
    parser.add_argument("--dpi", type=int, default=120)
    args = parser.parse_args()

    hit_path  = args.results_dir / "hit_miss_ratios.csv"
    time_path = args.results_dir / "execution_times.csv"
    scale_path = args.results_dir / "scalability.csv"

    if not hit_path.exists() or not time_path.exists():
        print("Error: Results not found. Run `make run` first.")
        return

    PLOT_DIR.mkdir(parents=True, exist_ok=True)
    print("\nGenerating individual plots:")

    hit_data  = group_data(load_csv(hit_path))
    time_data = group_data(load_csv(time_path))

    plot_hit_rate(hit_data, PLOT_DIR, args.dpi)
    plot_execution_time(time_data, PLOT_DIR, args.dpi)
    plot_scalability(scale_path, PLOT_DIR, args.dpi)

    print("\nDone.")

if __name__ == "__main__":
    main()
