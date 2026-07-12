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
    "OptimizedLRU": {"color": "#4C9BE8", "marker": "o", "linestyle": "-",  "linewidth": 2.2},
    "SLRU":         {"color": "#9B59B6", "marker": "P", "linestyle": "-",  "linewidth": 2.2},
    "NaiveLRU":     {"color": "#F4A261", "marker": "s", "linestyle": "--", "linewidth": 1.8},
    "FIFO":         {"color": "#E76F51", "marker": "^", "linestyle": "-.", "linewidth": 1.8},
    "Belady":       {"color": "#2EC4B6", "marker": "D", "linestyle": ":",  "linewidth": 2.2},
}

SEQUENCE_ORDER = ["uniform_random", "high_locality"]
SEQUENCE_LABELS = {
    "uniform_random": "Uniform Random",
    "high_locality":  "High Locality",
}

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
            data[row["Algorithm"]]["x"].append(int(row["RequestCount"]))
            data[row["Algorithm"]]["y"].append(float(row["TimeMs"]))
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

def _style_ax(ax: plt.Axes, title: str, xlabel: str, ylabel: str, percent_y: bool = False) -> None:
    ax.set_title(title, fontsize=11, fontweight="bold")
    ax.set_xlabel(xlabel, fontsize=9)
    ax.set_ylabel(ylabel, fontsize=9)
    ax.grid(True, linestyle="--", alpha=0.35)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    if percent_y:
        ax.yaxis.set_major_formatter(mticker.PercentFormatter(xmax=100, decimals=0))
        ax.set_ylim(0, 105)

def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--results-dir", type=Path, default=Path(__file__).parent.parent / "results")
    parser.add_argument("--dpi", type=int, default=130)
    args = parser.parse_args()

    results_dir = args.results_dir
    plot_dir    = results_dir / "plots"
    plot_dir.mkdir(parents=True, exist_ok=True)

    hit_path    = results_dir / "hit_miss_ratios.csv"
    time_path   = results_dir / "execution_times.csv"
    scale_path  = results_dir / "scalability.csv"

    if not hit_path.exists() or not time_path.exists():
        print("Error: Results not found.")
        return

    hit_data  = group_data(load_csv(hit_path))
    time_data = group_data(load_csv(time_path))

    # 2x2 hit rate + exec time dashboard
    fig, axes = plt.subplots(2, 2, figsize=(14, 9))
    fig.suptitle("Cache Replacement Algorithm Comparison Dashboard", fontsize=15, fontweight="bold", y=0.99)

    for row_idx, seq_name in enumerate(SEQUENCE_ORDER):
        seq_label = SEQUENCE_LABELS.get(seq_name, seq_name)

        ax_hit = axes[row_idx][0]
        for algo, style in ALGORITHM_STYLES.items():
            if algo not in hit_data.get(seq_name, {}): continue
            pts = hit_data[seq_name][algo]
            ax_hit.plot([p[0] for p in pts], [p[1] * 100 for p in pts], label=algo, **style)
        _style_ax(ax_hit, f"Hit Rate - {seq_label}", "Cache Size (k)", "Hit Rate (%)", True)
        ax_hit.legend(fontsize=8, framealpha=0.8)

        ax_time = axes[row_idx][1]
        for algo, style in ALGORITHM_STYLES.items():
            if algo not in time_data.get(seq_name, {}): continue
            pts = time_data[seq_name][algo]
            ax_time.plot([p[0] for p in pts], [p[2] for p in pts], label=algo, **style)
        _style_ax(ax_time, f"Exec Time - {seq_label}", "Cache Size (k)", "Time (ms)")
        ax_time.legend(fontsize=8, framealpha=0.8)

    fig.tight_layout(rect=[0, 0, 1, 0.97])
    dashboard_path = plot_dir / "combined_dashboard.png"
    fig.savefig(dashboard_path, dpi=args.dpi)
    plt.close(fig)
    print(f"Saved: {dashboard_path.name}")

    # Scalability plot (1 extra graph)
    if scale_path.exists():
        scale_data = load_scalability_csv(scale_path)
        fig, ax = plt.subplots(figsize=(10, 5))

        for algo, style in ALGORITHM_STYLES.items():
            if algo not in scale_data: continue
            ax.plot(scale_data[algo]["x"], scale_data[algo]["y"], label=algo, **style)

        _style_ax(ax,
            "Execution Time vs. Request Count (k=16)\n"
            "O(1) LRU stays flat; Belady grows with n",
            "Number of Requests", "Time (ms)"
        )
        ax.set_xscale("log")
        ax.legend(fontsize=9, framealpha=0.8)

        fig.tight_layout()
        scale_out = plot_dir / "scalability.png"
        fig.savefig(scale_out, dpi=args.dpi)
        plt.close(fig)
        print(f"Saved: {scale_out.name}")

    print("All plots generated.")

if __name__ == "__main__":
    main()
