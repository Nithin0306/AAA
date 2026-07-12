#!/usr/bin/env python3

import random
import argparse
from pathlib import Path

DEFAULT_N = 10_000
DEFAULT_KEY_RANGE = 100
DEFAULT_SEED = 42
DEFAULT_HOT_KEYS = 10
DEFAULT_HOT_PROB = 0.80

DATA_DIR = Path(__file__).parent.parent / "data"

def generate_uniform_random(n: int, key_range: int, seed: int, output_path: Path) -> None:
    rng = random.Random(seed)
    sequence = [rng.randint(0, key_range - 1) for _ in range(n)]

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with open(output_path, "w") as f:
        f.write("\n".join(map(str, sequence)) + "\n")

    print(f"Generated {n} uniform random requests to {output_path}")

def generate_high_locality(n: int, key_range: int, seed: int, hot_keys: int, hot_prob: float, output_path: Path) -> None:
    assert hot_keys < key_range, "hot_keys must be smaller than key_range"
    rng = random.Random(seed)
    sequence = []

    for _ in range(n):
        if rng.random() < hot_prob:
            sequence.append(rng.randint(0, hot_keys - 1))
        else:
            sequence.append(rng.randint(hot_keys, key_range - 1))

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with open(output_path, "w") as f:
        f.write("\n".join(map(str, sequence)) + "\n")

    hot_count = sum(1 for x in sequence if x < hot_keys)
    print(f"Generated {n} high locality requests to {output_path} (hot: {hot_count/n*100:.1f}%)")

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--n", type=int, default=DEFAULT_N)
    parser.add_argument("--key-range", type=int, default=DEFAULT_KEY_RANGE)
    parser.add_argument("--seed", type=int, default=DEFAULT_SEED)
    parser.add_argument("--hot-keys", type=int, default=DEFAULT_HOT_KEYS)
    parser.add_argument("--hot-prob", type=float, default=DEFAULT_HOT_PROB)
    return parser.parse_args()

def main() -> None:
    args = parse_args()

    generate_uniform_random(
        n = args.n,
        key_range = args.key_range,
        seed = args.seed,
        output_path = DATA_DIR / "uniform_random_seq.txt",
    )

    generate_high_locality(
        n = args.n,
        key_range = args.key_range,
        seed = args.seed,
        hot_keys = args.hot_keys,
        hot_prob = args.hot_prob,
        output_path = DATA_DIR / "high_locality_seq.txt",
    )

if __name__ == "__main__":
    main()
